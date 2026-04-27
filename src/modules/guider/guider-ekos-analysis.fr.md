---
title: Module - Guider
weight : 20
--- 

# Guider Module Documentation

## Reference: Ekos Internal Autoguiding (KStars)

This analysis describes how Ekos internal autoguiding works, used as a reference for the OST guider module development. Source files are located in `kstars/ekos/guide/`.

---

## Key files in Ekos

| File | Role |
|---|---|
| `calibration.h/cpp` | Data structure, angle computation, rotation matrix, pulse rates |
| `calibrationprocess.cpp` | Calibration sequence state machine |
| `gmath.h/cpp` | Core processing: star detection, drift, corrections |
| `internalguider.h/cpp` | Orchestration: pier flip, target change |
| `vect.h`, `matr.h` | Vector and matrix algebra |

---

## 1. Calibration

### Sequence (`CalibrationProcess` state machine)

5 steps in order:
1. **RA Outward** — pulse RA+ until drift > 15 px (or max iterations)
2. **RA Inward** — pulse RA− to return to starting position
3. **DEC Backlash** (optional) — 5 DEC+ pulses to measure mechanical backlash
4. **DEC Outward** — same as RA+ but for DEC
5. **DEC Inward** — return to starting position

### Calculations in `calculate2D()`

**Pixel to arcsecond conversion:**
```
arcsec_per_pixel = binning × 206264.806 × pixel_size_mm / focal_length_mm
```

**Rotation angles per axis:**
```
phi_RA  = atan2(-dy_RA,  dx_RA)  × (180/π)   [degrees, 0–360°]
phi_DEC = atan2(-dy_DEC, dx_DEC) × (180/π)
```
The DEC axis is oriented perpendicular to RA: both ±90° rotations are tested and the best one is chosen by dot product.

**Final retained angle:**
```
phi = (phi_RA + phi_DEC) / 2
```

**Rotation matrix Z built from phi:**
```
ROT_Z = [ cos(-φ)  -sin(-φ)  0 ]
        [ sin(-φ)   cos(-φ)  0 ]
        [    0         0     1 ]
```

**Pulse rates (fundamental values):**
```
raPulseMsPerArcsecond  = total_RA_pulse_duration  / RA_displacement_in_arcsec
decPulseMsPerArcsecond = total_DEC_pulse_duration / DEC_displacement_in_arcsec
```

### Values persisted after calibration

- Angle φ (`calibrationAngle`)
- `raPulseMsPerArcsecond` and `decPulseMsPerArcsecond`
- `calibrationDecSwap` (whether DEC axis is inverted)
- Mount RA/DEC position at calibration time
- Pier side at calibration time
- Binning used

---

## 2. Correction computation (`gmath.cpp`)

### Per-frame pipeline

**1. Star detection** → pixel position (centroid or SEP)

**2. Raw drift in arcseconds:**
```
raw_drift = (star_position − reticle_position) × arcsec_per_pixel
```

**3. Rotation into RA/DEC frame:**
```
in.x =  raw_drift.x
in.y = −raw_drift.y   ← Y inversion (image frame vs celestial frame)
drift_ra_dec = in × ROT_Z
```

**4. Circular buffer:** the last 50 drifts are stored for the integral term.

**5. Simplified PID correction (per axis):**
```
response_P = current_drift    × gain_P × ms_per_arcsec
response_I = avg(last_50)     × gain_I × ms_per_arcsec
pulse_duration = min(|P + I|, max_pulse_ms)
```
If the correction is below the minimum threshold, no pulse is sent.

**6. GPG (optional for RA):** if enabled, replaces PID with a Gaussian Process predictive model.

---

## 3. Applying corrections

### Normal operation

- Pulses are sent in milliseconds via the mount INDI driver.
- **First frame** after startup or slew: no pulse, reticle is centered on the star (`m_isFirstFrame = true`).

### Target change (slew)

- Calibration is **reused as-is**.
- `m_isFirstFrame = true` → first capture without correction, reticle recentered.
- Guiding resumes normally from the 2nd frame.

### Meridian flip (`guideAfterMeridianFlip()`)

**Option 1 — Full recalibration** (if `resetGuideCalibration` = true):
- Calibration is cleared, a new calibration is required.

**Option 2 — Calibration adaptation** (default behavior):
```
new_angle = calibration_angle + 180°  (modulo 360°)
ROT_Z recomputed with the new angle
```
For DEC swap:
- If `NOT reverseDecOnPierChange` → `decSwap = !calibrationDecSwap`
- Otherwise → decSwap unchanged

### Automatic corrections on restored calibration

**Declination** if current position differs from calibration position:
```
corrected_ms_per_arcsec = original_ms_per_arcsec × cos(current_dec) / cos(calibration_dec)
```

**Binning** if binning has changed since calibration:
```
binFactor = current_subBinX / calibration_subBinX
corrected_arcsec_per_pixel = base_arcsec_per_pixel × binFactor
```

---

## Summary table

| Step | What is computed | What is retained |
|---|---|---|
| **Calibration** | Angular displacement under pulse | Angle φ, `raPulseMsPerArcsec`, `decPulseMsPerArcsec`, `decSwap`, pier side, RA/DEC, binning |
| **Guiding** | Drift pixel → arcsec → RA/DEC frame via ROT_Z → PID | Pulse in ms sent to mount |
| **Pier flip** | angle + 180°, decSwap optionally inverted | Calibration updated in memory |
| **Slew** | No recomputation | Calibration reused, reticle recentered |
