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

## 2. Coordinate conversion chain (critical)

For each image, the correct path is:

```
Image pixel
  → WCS (astrometry / StellarSolver)
  → RA/DEC J2000 (degrees, inertial frame)
  → RA/DEC apparent (current epoch, via J2000toObserved: precession + nutation)
  → Hour angle (HA = LST - RA_apparent)
  → Azimuth / Altitude (horizontal frame, Earth-fixed)
  → Unit vector xyz  [azAlt2xyz]
```

Working in J2000 RA/DEC is **wrong** because that frame does not rotate with the Earth.
The mount's RA axis is fixed in the horizontal frame (Az/Alt), not in J2000.

### Implemented in `solverToAzAlt()`

```cpp
// Step 1: J2000 -> apparent
INDI::J2000toObserved(&j2000, jd, &apparent);

// Step 2: hour angle
double gst = ln_get_apparent_sidereal_time(jd);
double lst = gst + _observerLon / 15.0;
double ha  = lst - apparent.rightascension;

// Step 3: equatorial -> horizontal (standard trig formula)
// Convention: Az = 0 North, 90 East
double sin_alt = sin(lat)*sin(dec) + cos(lat)*cos(dec)*cos(ha_rad);
double az = acos((sin(dec) - sin(alt)*sin(lat)) / (cos(alt)*cos(lat)));
if (sin(ha_rad) > 0) az = 2*PI - az;  // quadrant correction
```

The JD for each image uses the capture timestamp (`_t0`, `_t1`, `_t2` in ms since epoch):
```cpp
double jd = 2440587.5 + _t0 / (86400.0 * 1000.0);
```

Observer position (`_observerLat`, `_observerLon`) is read from the GPS device
at startup via `GEOGRAPHIC_COORD / LAT` and `GEOGRAPHIC_COORD / LONG`.

---

## 3. Axis and error calculation

```
p0, p1, p2 = three xyz unit vectors (from Az/Alt of each image)
axis = getAxis(p0, p1, p2)   → plane normal = RA axis direction
     = normalised cross product

azAlt_axis = xyz2azAlt(axis)
  → azAlt_axis.x() = azimuth of the axis  (should be 0°)
  → azAlt_axis.y() = altitude of the axis (should equal observer latitude)

error_az  = azAlt_axis.x()
error_alt = azAlt_axis.y() - _observerLat
```

### Hemisphere handling

```cpp
bool northernHemisphere = (_observerLat >= 0.0);
if ((northernHemisphere && axis.x() < 0) || (!northernHemisphere && axis.x() > 0))
    axis = -axis;
```

In the southern hemisphere the RA axis points toward the south pole, whose x component
in the `azAlt2xyz` frame is positive (x points northward), so the flip condition is inverted.

---

## 4. State machine

### Phase 1 — calibration (3 shots)

```
InitInit
  → RequestFrameReset → WaitFrameReset
  → RequestExposure → WaitExposure → FindStars → Compute
  → RequestMove → WaitMove
  → RequestExposure → ... (×3 total)
  → FinalCompute
```

`SMCompute` accumulates `_ra0/_de0`, `_ra1/_de1`, `_ra2/_de2` (J2000 degrees) and
`_t0`, `_t1`, `_t2` (timestamps in ms). After 3 shots it emits `PolarDone`.

`SMComputeFinal`:
1. Converts each `(_raN, _deN, jdN)` to Az/Alt via `solverToAzAlt()`
2. Feeds the 3 Az/Alt unit vectors into `getAxis()`
3. Converts the axis to Az/Alt → computes `_erraz`, `_erralt`, `_errtot`
4. Stores the Az/Alt of image 3 as the correction reference (`_refAz`, `_refAlt`)
5. Draws the error overlay on the last image
6. Emits `ComputeFinalDone` → enters correction loop

### Phase 2 — correction loop (continuous, no mount moves)

```
CorrExposure → CorrWaitExposure → CorrFindStars → CorrCompute → CorrExposure → ...
```

The loop runs until the user triggers `Abort`. The mount is not commanded to move —
the user physically adjusts the altitude and azimuth knobs.

`SMCorrCompute`:
1. Gets current J2000 from solver
2. Converts to Az/Alt at current time
3. Computes displacement from the reference: `dAz = currentAz - _refAz`, `dAlt = currentAlt - _refAlt`
4. Remaining error = initial error − correction applied:
   - `remainAz  = _erraz  - dAz`
   - `remainAlt = _erralt - dAlt`
5. Updates the error elements (degrees and arcminutes)
6. Draws the overlay with the remaining error
7. Emits `CorrComputeDone` → loops back to `CorrExposure`

#### Principle of the displacement measurement

When the user turns the altitude or azimuth adjustment screws, the whole mount tilts.
This moves the telescope's pointing direction, which is observable in the solved Az/Alt.
The Az/Alt displacement from the reference ≈ the polar axis correction already applied
(valid approximation for small corrections).

---

## 5. Image overlay (`drawErrorOverlay`)

A triangle is drawn centred on the image:

- **P1** (white dot) = image centre = current polar axis position
- **P2** (yellow dot) = P1 + altitude error (vertical direction)
- **P3** (green dot) = P2 + azimuth error (horizontal direction) = target position
- Yellow line P1→P2 = altitude error
- Green line P2→P3 = azimuth error
- Red line P1→P3 = total error (hypotenuse)

Labels show values in arcminutes. Scale is adaptive: total error spans 30% of the image
height, with a minimum of 50 px/degree so small errors remain visible.

---

## 6. Error elements published to the frontend

Property `errors`:

| Element    | Unit    | Description                     |
|------------|---------|---------------------------------|
| `erraz`    | degrees | Azimuth error                   |
| `erralt`   | degrees | Altitude error                  |
| `errtot`   | degrees | Total error                     |
| `errazmn`  | arcmin  | Azimuth error (arcminutes)      |
| `erraltmn` | arcmin  | Altitude error (arcminutes)     |
| `errtotmn` | arcmin  | Total error (arcminutes)        |

---

## 7. Known limitations and future work

- **Displacement approximation**: the correction loop equates Az/Alt image displacement
  with polar axis displacement. This is valid for small errors and when the telescope
  is pointing roughly toward the meridian. For large misalignments or at low altitude
  the approximation degrades.

- **Tracking assumption**: the computation assumes the mount is tracking during the
  correction loop. If it is not, Earth's rotation drifts the pointing and the measured
  displacement conflates sidereal drift with user correction.

- **No WCS pixel mapping**: the overlay triangle uses a fixed orientation (up = altitude,
  right = azimuth) independent of camera rotation. A future improvement would use the
  solver's position angle to orient the triangle correctly in the image.
