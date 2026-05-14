# OST Guider Module — Code Analysis

Analysis of `guider.cpp` / `guider.h`. All issues listed below have been fixed.

---

## Fixes applied

### 1. Signal `abort()` lowercase — state machine hangs

`SMComputeCal()`: when no stars are found during calibration, `emit abort()` was silently ignored
by Qt because the actual signal is `Abort()` (capital A). The state machine would hang indefinitely.

```cpp
// before
emit abort();
// after
emit Abort();
```

---

### 2. Division by zero in `matchIndexes()`

When no star matches between two frames, `pairs` is empty and dividing by `pairs.size()` is
undefined behavior in C++.

```cpp
// added before the division
if (pairs.isEmpty()) { dx = 0; dy = 0; return; }
```

---

### 3. Biased average in calibration (~50% error with default settings)

The averaging loop was starting at `i = 1`, skipping the first valid measurement, but still
dividing by the total size. With `calsteps=2` this caused a 50% systematic underestimate of
the pulse rates.

```cpp
// before
for (unsigned int i = 1; i < _dxvector.size(); i++)
// after
for (unsigned int i = 0; i < _dxvector.size(); i++)
```

---

### 4. Stale drift used when no stars are detected

When `_trigCurrent` was empty (no stars detected in a frame), `_dxFirst`/`_dyFirst` retained
their values from the previous frame and incorrect pulses were sent. The frame is now skipped.

```cpp
else
{
    logWarning("No stars detected in current frame - skipping corrections");
    emit ComputeGuideDone();
    return;
}
```

---

### 5. Wrong DEC rotation formula

The original formula `dx·sin + dy·cos` is not a valid projection onto the DEC axis.
Comparison with Ekos (`gmath.cpp` + `matr.cpp`) shows the correct formula is `dx·sin - dy·cos`.

The error was hidden for small CCD rotation angles (sin ≈ 0) but introduced RA↔DEC
cross-coupling for significant rotations.

```cpp
// before
double _driftDE = _dxFirst * sin(_calCcdOrientation) + _dyFirst * cos(_calCcdOrientation);
// after
double _driftDE = _dxFirst * sin(_calCcdOrientation) + _dyFirst * -cos(_calCcdOrientation);
```

Applied in both `SMComputeCal()` (drift display) and `SMComputeGuide()` (active corrections).

---

### 6. `atan` instead of `atan2` for CCD orientation angle

`atan(ddy/ddx)` has two problems: division by zero when `ddx == 0`, and the result is limited
to `[-π/2, π/2]`. When the West-pulse displacement vector points into the 2nd or 3rd quadrant
(i.e. `ddx < 0`, which is the typical case for a pier-West mount), the computed angle is wrong
by 180°.

```cpp
// before
double a = atan(ddy / ddx);
// after
double a = atan2(ddy, ddx);
```

---

### 7. RA correction direction inverted after atan2 fix

The original `atan` bug always returned an angle near 0 (regardless of the sign of `ddx`),
which made `cos(θ) ≈ +1`. The pulse direction mapping was designed for this wrong angle.

After the `atan2` fix, `θ ≈ π` for the typical pier-West setup, making `cos(θ) = -1` and
flipping the sign of `drift_RA`. The mapping `drift_RA > 0 → pulseW` was therefore inverted.

The two bugs had been cancelling each other. Fixing the angle required fixing the mapping:

```cpp
// before: drift > 0 → West,  drift < 0 → East
// after:  drift > 0 → East,  drift < 0 → West
if (revRA * _driftRA > 0 && !disRAO)
    _pulseE = getFloat("guideParams", "raAgr") * revRA * _driftRA * calPulseECompensated;
if (revRA * _driftRA < 0 && !disRAE)
    _pulseW = -getFloat("guideParams", "raAgr") * revRA * _driftRA * calPulseWCompensated;
```

---

### 8. `_calPulseN/S/E/W` declared as `int` — precision loss

The four calibration rates were declared as `int` in `guider.h` and stored as `int` in the JSON.
They are computed as `double` (pulse_ms / sqrt(dx²+dy²)) and the truncation caused up to ~10%
systematic error in pulse durations.

- `guider.h`: changed `int` → `double` for `_calPulseN/S/E/W`
- `guider.json`: changed `"type":"int"` → `"type":"float"` for the four `calPulse*` properties
- `guider.cpp`: replaced all `getEltInt` / `getInt` with `getEltFloat` / `getFloat` for these values

---

---

## Dithering

Dithering shifts the telescope to a random nearby position between science exposures, which
helps average out fixed-pattern sensor noise across the image stack.

### Parameters

- `guideParams / ditherpixel` (int, default 5, range 1–50): maximum random displacement in pixels.

### Trigger

The `dither` button in the `actions` group. It is silently ignored if `_SMGuide` is not running.

### State machine flow

Dithering is integrated as a branch of `_SMGuide`. When `_doDither` is set and the current
guide frame completes, `SMComputeGuide()` emits `DitherNow` instead of `ComputeGuideDone`:

```
ComputeGuide  --DitherNow-->  RequestDitherPulses  (SMRequestPulses)
                              WaitDitherPulses      (waits PulsesDone)
                              RequestRefExposure    (SMRequestExposure)
                              WaitRefExposure       (waits ExposureDone)
                              FindStarsRef          (SMFindStars)
                              ComputeNewRef         (SMComputeFirst — rebuilds _trigFirst)
                              InitGuide             (fresh start, drift history cleared)
                              RequestGuideExposure  (normal guiding resumes)
```

`SMComputeFirst` is reused as-is: it calls `buildIndexes(_solver, _trigFirst)` which sets the
new reference from the stars detected after the mount has moved.

### Pulse calculation

Random displacements `randRA` and `randDEC` are drawn uniformly from `[-ditherpixel, +ditherpixel]`.

```
// RA: calPulseE/W are equatorial (DEC-normalized), apply current DEC compensation
if (randRA > 0)  pulseW = randRA  * calPulseW * cos(mountDEC)
else             pulseE = -randRA * calPulseE * cos(mountDEC)

// DEC: calPulseN/S are raw ms/pixel, no compensation needed
if (randDEC > 0) pulseN = randDEC  * calPulseN
else             pulseS = -randDEC * calPulseS
```

---

## Notes

**Pier-side flip**: when guiding is started with calibration data from the opposite pier side,
the `enablepiersidereverse` option in `guideParams` automatically inverts `revRA` and `revDE`.
It is disabled by default.

**RA/DEC inversion flags** (`revCorrections`): the `revRA` / `revDE` flags remain the primary
mechanism for handling non-standard camera orientations. The pier-side logic always reloads
the saved calibration flags before applying the inversion, so starting guiding multiple times
does not compound the effect.
