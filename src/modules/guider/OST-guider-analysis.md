# OST Guider Module — Code Analysis

Analysis of `guider.cpp` / `guider.h`. The module works but has the following bugs.

---

## 1. Calibration reuse issues

### 1.1 Precision loss: `_calPulseN/S/E/W` declared as `int`

`guider.h:111-114` declares the four calibration values as `int`. They are computed as floating-point in `SMComputeCal()` and silently truncated on assignment:

```cpp
_calPulseN = getInt("calParams", "pulse") / sqrt(square(ddx) + square(ddy));
// e.g. 1000 / 114.3 = 8.75 → stored as 8 → ~10% error
```

The truncation error propagates directly into guiding pulse durations.

**Fix:** declare as `double` in the header; replace `getEltInt` / `getInt` with `getEltFloat` / `getFloat` for persistence.

---

### 1.2 CCD angle: `atan` instead of `atan2`

`SMComputeCal()` line 819:
```cpp
double a = atan(ddy / ddx);
```

Two problems: division by zero when `ddx == 0`, and result limited to `[-π/2, π/2]` instead of `[-π, π]`. If the drift vector points into the 2nd or 3rd quadrant, the angle is off by 180°.

**Fix:**
```cpp
double a = atan2(ddy, ddx);
```

---

### 1.3 Wrong CCD → RA/DEC rotation matrix

`SMComputeCal()` line 926 and `SMComputeGuide()` line 951:
```cpp
double _driftRA = _dxFirst * cos(_calCcdOrientation) + _dyFirst * sin(_calCcdOrientation);
double _driftDE = _dxFirst * sin(_calCcdOrientation) + _dyFirst * cos(_calCcdOrientation);
```

The implicit matrix is:
```
[cos  sin]
[sin  cos]
```

This is **not a rotation matrix**. The correct projection onto the RA axis and its perpendicular DEC axis is:
```
[cos   sin]    ← projection onto RA axis
[-sin  cos]    ← projection onto perpendicular DEC axis
```

The sign of `sin` in the DEC line is wrong. The error is hidden for small angles (cos ≈ 1, sin ≈ 0) but introduces RA↔DEC cross-coupling for significant rotation angles.

**Fix** (same change needed in both `SMComputeCal()` and `SMComputeGuide()`):
```cpp
double _driftRA = _dxFirst *  cos(_calCcdOrientation) + _dyFirst * sin(_calCcdOrientation);
double _driftDE = _dxFirst * -sin(_calCcdOrientation) + _dyFirst * cos(_calCcdOrientation);
```

---

## 2. Error handling issues

### 2.1 Signal `abort()` lowercase — never emitted

`SMComputeCal()` line 782:
```cpp
emit abort();
```

The signal declared in `guider.h:74` is `Abort()` with a capital A. `abort()` does not exist as a Qt signal: Qt silently ignores the call. When no stars are found during calibration, the state machine hangs indefinitely instead of stopping.

**Fix:**
```cpp
emit Abort();
```

---

### 2.2 Division by zero in `matchIndexes()`

`matchIndexes()` lines 1238-1239:
```cpp
dx = dx / pairs.size();
dy = dy / pairs.size();
```

If no star matches between the two frames, `pairs` is empty and this is a division by zero (undefined behavior in C++).

**Fix:**
```cpp
if (pairs.isEmpty()) { dx = 0; dy = 0; return; }
dx /= pairs.size();
dy /= pairs.size();
```

---

### 2.3 Stale drift used when no stars are detected

`SMComputeGuide()` lines 944-953:
```cpp
if (_trigCurrent.size() > 0)
{
    matchIndexes(_trigFirst, _trigCurrent, _matchedCurFirst, _dxFirst, _dyFirst);
}
// _driftRA and _driftDE are computed unconditionally
double _driftRA = _dxFirst * cos(...) + ...
```

If `_trigCurrent` is empty (no stars in the frame), `_dxFirst`/`_dyFirst` retain their values from the previous frame and incorrect pulses are sent. Corrections should be skipped, or guiding aborted after N consecutive frames without stars.

---

### 2.4 Biased average in calibration

`SMComputeCal()` lines 812-817:
```cpp
for (unsigned int i = 1; i < _dxvector.size(); i++)  // starts at 1, skips first measurement
{
    ddx += _dxvector[i];
    ddy += _dyvector[i];
}
ddx = ddx / (_dxvector.size());   // divides by total size, not (size - 1)
```

The loop skips `_dxvector[0]` (the first valid measurement after the first pulse) but divides by `size` instead of `size - 1`. The computed rates are systematically underestimated by a factor of `(N-1)/N`. With `calsteps=2` this is a 50% error.

**Fix:** start loop at `i = 0`, or divide by `_dxvector.size() - 1` if the intent is to skip the first sample.

---

## Priority summary

| Priority | Issue | Location |
|---|---|---|
| **Blocking** | `emit abort()` lowercase → state machine hangs | `SMComputeCal()` L.782 |
| **Blocking** | Division by zero when no stars match | `matchIndexes()` L.1238 |
| **Major** | ~50% bias in calibration average | `SMComputeCal()` L.812 |
| **Major** | Pulses sent using stale drift (frame without stars) | `SMComputeGuide()` L.944 |
| **Major** | Wrong DEC rotation formula (missing minus sign) | L.927, L.952 |
| **Minor** | `atan` → `atan2` for CCD angle | `SMComputeCal()` L.819 |
| **Minor** | `_calPulseN/S/E/W` as `int` → precision loss | `guider.h` L.111 |
