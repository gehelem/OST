# Polar alignment — analysis and implementation notes

Reference implementations: KStars/Ekos (`polaralign.cpp`, `poleaxis.cpp`, `rotations.cpp`)
OST implementation: `polar.cpp`

---

## 1. Geometric principle

Three images are taken with a RA rotation of the mount between each one.
The center of each image traces an arc on the celestial sphere.
The plane of that arc is perpendicular to the mount's RA axis.
The normal to that plane is therefore the direction of the RA axis.

A perfectly aligned mount has its RA axis pointing toward the celestial pole,
which coincides with the geographic pole direction: azimuth = 0°, altitude = latitude.

---

## 2. Coordinate conversion chain

For each image, the correct path is:

```
Image pixel
  → WCS (astrometry / StellarSolver)        returns J2000 degrees
  → RA/DEC apparent (J2000toObserved)        precession + nutation
  → Hour angle  (HA = LST - RA_apparent)     needs observer longitude + JD
  → Azimuth / Altitude                       needs observer latitude
  → Unit vector xyz  [azAlt2xyz]
```

Working in J2000 RA/DEC is **wrong** because that frame does not rotate with the Earth.
The mount's RA axis is fixed in the horizontal frame (Az/Alt), not in J2000.

### `solverToAzAlt(ra_j2000_deg, dec_j2000_deg, jd)`

```cpp
// J2000 -> apparent
INDI::J2000toObserved(&j2000, jd, &apparent);

// Hour angle
double gst = ln_get_apparent_sidereal_time(jd);
double lst = gst + _observerLon / 15.0;
double ha  = lst - apparent.rightascension;

// Equatorial -> horizontal (Az=0 North, 90 East)
double sin_alt = sin(lat)*sin(dec) + cos(lat)*cos(dec)*cos(ha_rad);
double az = acos((sin(dec) - sin(alt)*sin(lat)) / (cos(alt)*cos(lat)));
if (sin(ha_rad) > 0) az = 2*PI - az;
```

Observer position (`_observerLat`, `_observerLon`) is read from the GPS device at startup
via `GEOGRAPHIC_COORD / LAT` and `GEOGRAPHIC_COORD / LONG`.

The JD for each calibration image uses the **mid-exposure** timestamp:
```cpp
double t = QDateTime::currentMSecsSinceEpoch() + _exposure * 500.0;
```
This is more accurate than using the start-of-exposure time.

---

## 3. Axis and error calculation

```
p0, p1, p2 = three xyz unit vectors (from Az/Alt of each image)
axis = getAxis(p0, p1, p2)   → plane normal = RA axis direction

azAlt_axis = xyz2azAlt(axis)
  → x() = azimuth of the axis  (should be 0°)
  → y() = altitude of the axis (should equal observer latitude)

error_az  = azAlt_axis.x()
error_alt = azAlt_axis.y() - _observerLat
```

### Hemisphere handling

```cpp
bool northernHemisphere = (_observerLat >= 0.0);
if ((northernHemisphere && axis.x() < 0) || (!northernHemisphere && axis.x() > 0))
    axis = -axis;
```

---

## 4. State machine

### Phase 1 — calibration (3 shots)

```
InitInit
  → RequestFrameReset → WaitFrameReset
  → RequestExposure → WaitExposure → FindStars ──[FindStarsDone]──→ Compute
                                               └─[FindStarsFailed]─→ Abort
  → RequestMove → WaitMove
  → (repeat ×3)
  → FinalCompute
```

**Mount rotation between shots:** 0.5 h (7.5°) in RA.

**Move detection (`_slewing` flag):** `MoveDone` is only emitted after the mount
transitions `IPS_BUSY → IPS_OK` on `EQUATORIAL_EOD_COORD`. This prevents spurious
triggers from position-update messages received before the move starts.

```cpp
// in SMRequestMove:
_slewing = false;  // arm the detector

// in updateProperty:
if (state == IPS_BUSY) _slewing = true;
if (state == IPS_OK && _slewing) { _slewing = false; emit MoveDone(); }
```

**Solver position hint:** the mount reports EOD coordinates, but the solver expects J2000.
The hint is now correctly converted:
```cpp
INDI::ObservedToJ2000(&mountEod, jdHint, &mountJ2000);
_solver.stellarSolver.setSearchPositionInDegrees(mountJ2000.ra * 15.0, mountJ2000.dec);
```

**FOV hint:** uses the horizontal FOV (`_ccdX * _ccdSampling`) instead of the diagonal,
with `minwidth = 0.5 * fovWidthDeg` and `search_radius = 3°`.

**Connections:** `Qt::UniqueConnection` is used on all solver signal connections to
prevent duplicate slots when `SMFindStars` is called multiple times in the loop.

`SMCompute` accumulates `_ra0/_de0`, `_ra1/_de1`, `_ra2/_de2` (J2000 degrees) and
`_t0`, `_t1`, `_t2` (mid-exposure timestamps in ms). After 3 shots it emits `PolarDone`.

`SMComputeFinal`:
1. Converts each `(_raN, _deN, jdN)` to Az/Alt via `solverToAzAlt()`
2. Feeds the 3 Az/Alt unit vectors into `getAxis()`
3. Converts the axis to Az/Alt → computes `_erraz`, `_erralt`, `_errtot`
4. Stores the Az/Alt of image 3 as the correction reference (`_refAz`, `_refAlt`)
5. Draws the error overlay (with image orientation) on the last image
6. Emits `ComputeFinalDone` → enters correction loop

### Phase 2 — correction loop (continuous, no mount moves)

```
CorrExposure → CorrWaitExposure → CorrFindStars ──[FindStarsDone]──→ CorrCompute → CorrExposure
                                               └─[FindStarsFailed]─→ CorrExposure (retry silently)
```

The loop runs until the user triggers `Abort`. Solver failures during correction are
**not fatal**: the state machine loops back to `CorrExposure` and retries.

`SMCorrCompute`:
1. Gets current J2000 from solver
2. Converts to Az/Alt **using the same JD as image 3** (`_t2`):
   ```cpp
   double jdRef = 2440587.5 + _t2 / (86400.0 * 1000.0);
   QPointF currentAzAlt = solverToAzAlt(currentRA, currentDEC, jdRef);
   ```
   Using the same epoch cancels out sidereal rotation — the Az/Alt delta is then
   purely due to the mechanical adjustment made by the user.
3. Displacement from reference: `dAz = currentAz - _refAz`, `dAlt = currentAlt - _refAlt`
4. Remaining error = initial error − correction applied:
   - `remainAz  = _erraz  - dAz`
   - `remainAlt = _erralt - dAlt`
5. Updates the error elements and draws the overlay with the remaining error.

---

## 5. Image overlay (`drawErrorOverlay`)

The overlay triangle is **oriented with celestial North**, using the position angle
returned by the solver (`getSolution().orientation`):

```cpp
painter.translate(cx, cy);
painter.rotate(-orientation);  // align drawing frame with the sky
```

Triangle vertices in the rotated frame (origin = image centre, up = North):
- **P1** (white) = current polar axis position
- **P2** (yellow) = P1 + altitude correction (northward)
- **P3** (green)  = P2 + azimuth correction (eastward) = target (geographic pole)
- Yellow P1→P2 = altitude error
- Green  P2→P3 = azimuth error
- Red    P1→P3 = total error

Labels are placed in screen coordinates (transform reset after drawing) to keep text
horizontal regardless of orientation.

Scale: total error spans 30% of image height, minimum 50 px/degree.

---

## 6. `syncMount()` — mount model sync after solve

```cpp
void Polar::syncMount(double ra_j2000_deg, double dec_j2000_deg)
```

Sends a SYNC command to the mount with the solved J2000 position (converted to EOD):
1. Switch mount to SYNC mode (`ON_COORD_SET / SYNC`)
2. Send `EQUATORIAL_EOD_COORD` with observed RA/DEC
3. Restore SLEW mode

**Status:** implemented but not yet wired into the solve callback. Intended to be called
from `OnSucessSolve` or `SMCompute` to keep the mount's pointing model accurate.

---

## 7. Error elements published to the frontend

Property `errors`:

| Element    | Unit    | Description                  |
|------------|---------|------------------------------|
| `erraz`    | degrees | Azimuth error                |
| `erralt`   | degrees | Altitude error               |
| `errtot`   | degrees | Total error                  |
| `errazmn`  | arcmin  | Azimuth error (arcminutes)   |
| `erraltmn` | arcmin  | Altitude error (arcminutes)  |
| `errtotmn` | arcmin  | Total error (arcminutes)     |

---

## 8. Known limitations

- **Displacement approximation**: the correction loop equates Az/Alt image displacement
  with polar axis displacement. Valid for small corrections; degrades for large
  misalignments or at low altitude.

- **Tracking assumption**: the same-epoch trick in `SMCorrCompute` assumes the mount
  is tracking. If it is not, residual sidereal drift will contaminate the displacement
  measurement.

- **`syncMount` not yet called**: the method exists but is not integrated into the
  solve flow yet.
