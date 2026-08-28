# Guider module — critical analysis

Read-only review of `guider.{cpp,h,json}` (+ `polynomialfit.*`, cross-checked
against `sequencer.cpp`, `indimodule.cpp`, `solver.h` and the two existing
analysis notes). Nothing changed. Findings are reasoning from the code, not
from a run — the ones marked **(bench)** need a simulator check at two
declinations / pier sides before acting.

---

## A. Correctness — candidate bugs

### A1. DEC-transfer compensation looks inverted **(bench)**
> **Done — confirmed and fixed.** Sim test: calibrated at DEC 5.7 deg, guiding
> at 61.6 deg used 128 ms/px vs the correct 565 (`code/expected = 0.228 =
> cos^2(cur)/cos^2(cal)`); RA ran away until the integral saturated. Both signs
> flipped: store `R_meas * cos(calDEC)` (equatorial), use `/ cos(curDEC)` at
> guide time (+ dither + logs), near-pole guard. Re-test held RA to ~0.2 px.
The RA calibration rate is stored "normalised to the equator" and re-scaled to
the current declination when guiding. Both steps have the wrong sign; they
cancel when you guide at the calibration declination (the usual case — which is
why it isn't obviously broken), but diverge otherwise.

- `_calPulseE/W` = `pulse_ms / drift_px` → **ms per pixel**.
  Physics: `ms/px(δ) = C / cos(δ)` (an RA pulse moves the star by
  `rate·pulse·cos(δ)` on the sensor). So `ms/px(cur) = ms/px(cal)·cos(cal)/cos(cur)`.
- Store (`SMComputeCal`, ~line 949): `_calPulseE /= cos(calDEC)`   → should be `*=`
- Use (`SMComputeGuide`, ~line 1077): `calPulseECompensated = _calPulseE * cos(curDEC)` → should be `/`

Net error vs correct when `cal != cur`: factor `[cos(cur)/cos(cal)]²`.
Calibrate at δ=0, guide at δ=60 → RA pulses come out **4× too short**
(under-correction); calibrate north, guide near equator → 4× too long
(oscillation). Near the pole the guiding-time factor `*cos(curDEC)` drives
`_pulseE → 0`, so RA silently stops correcting instead of saturating.

The `SMInitGuide` header comment works a numeric example that is *correct* — but
it treats `_calPulseE` as "px/sec", the opposite of the code's actual `ms/px`.
The comment describes the right algorithm; the code implements it in the wrong
units without flipping the compensation.

### A2. `SMInitInit` — pier-side read failure doesn't abort
> **Done.** Now `logWarning` + continue (pier side is optional; fork/alt-az
> mounts don't expose it). The stray `;` + misleading error-state + `return`
> that hung init are gone.
`guider.cpp` ~549-554:
```cpp
if (!getModSwitch(..., "TELESCOPE_PIER_SIDE", "PIER_WEST", _mountPointingWest))
{
    logError("Failed to read mount pier side");
    setStateEvent(OST::Error, "error", "devicefailed", "mount failed");
    ;                       // <-- stray semicolon; no emit Abort(); no return;
}
```
The two branches just above (DEC, RA read failures) do `emit Abort(); return;`.
This one falls through and continues initialising with whatever
`_mountPointingWest` happened to contain.

### A3. No watchdog on pulse completion — state machine can hang forever
> **Done.** Single-shot `_watchdog` armed at each hardware wait (exposure,
> pulse, frame reset, SEP), disarmed on the matching callback / on abort;
> timeout -> `emit Abort()`. Configurable via `guideParams/watchdog` (s, 0 = off,
> default 30).
`_pulseRAfinished` / `_pulseDECfinished` are cleared when a pulse is sent and
only set back on `TELESCOPE_TIMED_GUIDE_*` → `IPS_IDLE`. If that INDI update
never arrives (driver quirk, disconnect, dropped event) `PulsesDone` is never
emitted and `_SMGuide` / `_SMCalibration` stalls with no timeout. The only
backstop is the *sequencer's* external `GuiderTimeout` (120 s), and only when
the guider is driven by the sequencer. Stand-alone guiding just freezes.
Same class of risk on the exposure / frame-reset waits.

### A4. `newBLOB` has no ownership / state guard
> **Done.** `_expectingFrame` gate: set in `SMRequestExposure`, cleared on the
> consumed BLOB and on abort. Stray/late BLOBs are ignored.
```cpp
void Guider::newBLOB(INDI::PropertyBlob pblob)
{
    if (QString(pblob.getDeviceName()) == getString("devices", "camera")) { ... emit ExposureDone(); }
}
```
Any `CCD1` BLOB from the configured camera fires `ExposureDone`, which advances
whichever state machine is currently in a `WaitExposure` state — including a
stale/late BLOB after an abort, or one triggered by another module sharing the
camera. There is no `mState`, no "expecting a frame" flag.

---

## B. Algorithm weaknesses

### B1. CCD orientation from a single vector
> **Done (with B3).** Orientation now averages two estimates - `atan2(dRA)` and
> the DEC axis rotated into the same convention - with the 180-deg ambiguity
> resolved against the RA axis. `dRA = (West - East)/2`, `dDE = (North - South)/2`.
`_calCcdOrientation = atan2(mean_dx, mean_dy)` of the **West pass only**
(`SMComputeCal`, `_calState == 0`). Ekos averages an independent RA and DEC
angle estimate, and disambiguates the DEC axis by testing both ±90° and picking
the better dot product. Here the N/S drift vectors *are* collected
(`_dxvector` / `_dyvector` during `_calState 2,3`) but only their **magnitude**
is used (for the N/S rate); the direction is thrown away. Half the calibration
information is unused, and the orientation rests on one noisy 2-sample mean.

### B2. The rotation "matrix" is a reflection
> **Checked (non-mirror case OK).** Calibration now logs the axis-decoupling:
> feeding dRA / dDE through the transform gives ~2% cross-leak and a -92 deg
> axis separation on the sim (the 2.3 deg non-orthogonality accounts for the
> leak). So theta / handedness are right for a non-mirrored train. The
> mirrored-optics failure mode (star diagonal / Newtonian) is still untested -
> would need real optics or a sim-mirror toggle.
```
driftRA =  dx·cos θ + dy·sin θ
driftDE =  dx·sin θ + dy·(−cos θ)
```
`det = −1` — rotation composed with a Y-flip. Probably intentional (image Y is
inverted vs sky, like Ekos' `in.y = −raw_drift.y`), and note `OST-guider-analysis.md`
§5 claims it was checked against Ekos. But its correctness depends entirely on
θ's sign convention, which comes from the fragile single-vector estimate in B1.

### B3. Calibration walks the star and never returns
> **Partly done.** Instead of return-to-origin, the rates and orientation are
> now taken from the antisymmetric combinations `(West-East)/2` and
> `(North-South)/2`, which cancel the constant sidereal / periodic-error drift
> that was biasing each one-directional pass (seen as a 78% N/S and 29% E/W
> rate asymmetry on the sim - gone after the fix). Guiding uses one RA rate
> and one DEC rate (`calPulseRA` / `calPulseDE`); the raw per-direction values
> are kept for diagnostics. Still not done: bounding the star excursion / a
> minimum-drift gate / a max-iteration safety.
`calsteps` (default **2**) pulses per direction, W→E→N→S, no return-to-origin,
no "pulse until drift > N px" gate, no max-excursion / off-sensor check, no
per-sample consistency check. Ekos pulses until drift > 15 px (capped) *and*
returns to start. Consequences:
- 2 samples/axis → noisy rate.
- Small `pulse` → the "drift" is dominated by seeing + periodic error, not the pulse.
- Large `pulse` / bad mount → star can leave the frame mid-calibration
  (only caught afterwards as "no stars, abort").

### B4. No first-frame settle
Ekos skips the correction on the first frame after start/slew and just
re-centres the reticle. Here the first `SMComputeGuide` immediately sends a real
pulse for the full offset between the post-calibration reference and the current
star position. Because the N and S rates differ (backlash, flexure) the star
does not return exactly to origin after the N/S passes, so that first pulse can
be a sizeable kick.

### B5. Triangle matching is brittle
- Hardcoded `±0.1 %` tolerance on **5** quantities (3 side lengths + perimeter +
  area; the last two are algebraically redundant with the sides). Very tight —
  sub-pixel centroid jitter drops matches → fewer pairs → noisier mean drift.
- `buildIndexes` takes the **first 10** stars in solver order, not the 10
  best (brightest / roundest). A star flickering near the SEP threshold
  reshuffles the whole triangle set between frames, so reference triangles lose
  their counterparts.
- `matchIndexes` dedups matched pairs with a linear scan → O(pairs²) per triangle
  pair; with C(10,3)=120 triangles each side that's ~120×120×(3 scans) inner
  work per match, run 2–3×/frame. Works, but heavier than it needs to be.

### B6. Control law is P-only, and the history buffer isn't used for control
> **Done.** Added a true integral term per axis (clamped accumulator, frozen on
> pulse saturation, reset at guide start / dither). `guideParams/raintgain` +
> `deintgain` (def 0.15, 0 = pure P), `guideParams/intmax` (px, anti-windup).
> Nulls the steady-state offset a P-only law leaves against a constant drift.
`_pulseX = agr · drift · rate`, clamped to `[pulsemin, pulsemax]`. No integral
term, no dead-band beyond `pulsemin`. `_dRAvector` / `_dDEvector` (last `rmsOver`
drifts) exist but feed only the RMS *display* — Ekos uses its 50-sample buffer
for the I term. Fine for a stiff mount, weak on drift-dominated ones.

### B7. RMS is a single-frame-fragile metric with a control role
`rmsTotal = sqrt(mean(driftRA² + driftDE²))` over the last `rmsOver` residuals,
about zero (correct for guiding). But it's computed on the *post-correction
residual* each frame and also drives the sequencer's exposure-abort threshold
(`rmsthreshold`), so one bad frame (lost star, cosmic ray, cloud) spikes it and
can abort a sub + trigger a full recalibration.

---

## C. Dead / stale code

| Item | Status |
|---|---|
| `src/modules/guider/polynomialfit.{cpp,h}` | **Not compiled** (only `focus/polynomialfit.*` is in CMakeLists), `#include` commented (line 36), sole call site inside a `/* */` block (line 840). Fully dead — delete. |
| `guider.h` `_machine` | declared "unused currently", never referenced |
| `guider.h` `_ccdSampling = 206*5.2/800` | never read; everything uses `getSampling()` |
| `guider.h` `_calPulseRA`, `_calPulseDEC` | never referenced |
| `calguide` action | handled in `onExternalEvent` (the "full workflow" path) but **no button creates it** — `defineMeAsGuider()` makes `guide`/`calibrate`/`abortguider`, `guider.json` has no `actions`. Dead unless the Angular template hardcodes the key. |
| Commented-out suspend/resume-guiding handlers | `guider.cpp` 124-142 |
| `#define PI 3.14159265` | 8 digits; use `M_PI` |

### C1. Inconsistent action-button guard
`calguide` → `setValue(true, true)`; `calibrate` and `guide` → `setValue(false, true)`.
`ElementBool::setValue` always returns true (in range), so all three "work", but
`calibrate` / `guide` immediately switch their own button back off while
`calguide` leaves it lit. Looks accidental, not designed.

---

## D. Architecture

### D1. Three runtime-wired state machines
`_SMInit`, `_SMCalibration`, `_SMGuide` are separate `QStateMachine`s stitched
together per button press in `onExternalEvent` with
`disconnect(&_SMInit, &finished, nullptr, nullptr)` (wipes *all* finished
connections) followed by fresh `connect`s. Easy to end up with a dangling or
doubled wire; hard to reason about. The project already uses SCXML for the
sequencer, and the git log has `5a7b02155 "first step so scxml, seems ok"` —
a migration that seems to have stalled for the guider. One hierarchical
machine (or the `.scxml` route) would be far more inspectable.

### D2. `updateProperty` overridden privately
Bypasses the `onUpdateProperty` hook the post-refactor modules use
(inspector, etc.). Works, but inconsistent.

### D3. Suspend-during-focus is a full abort + restart
Implemented sequencer-side as `abortguider` then later `guide` → full re-init,
re-acquire reference, re-calibrate if the values were cleared. Heavy vs a real
pause; the lighter in-module path is the commented-out block in D-C.

### D4. `matchIndexes(QVector<Trig> ref, QVector<Trig> act, …)` takes both by value
~120-element copies, 2–3×/frame, plus `foreach (Trig r, ref)` copying each
element. Should be `const &`.

---

## E. What's good (for balance)

- Separate **E/W** and **N/S** rates — captures axis asymmetry, more than Ekos'
  single RA rate.
- Adaptive **DEC backlash** compensation (learn from each reversal, nudge the
  estimate by measured over/undershoot) and the calibration-time backlash
  **kick** are thoughtful and well-commented.
- **Dithering** is cleanly integrated as a `_SMGuide` branch with a proper
  reference rebuild afterwards.
- Guide parameters are re-read **live** every frame → tune without restart.
- Verbose, useful logging; `atomicSaveJpeg` for the preview; star overlay
  (green = reference, red = current).
- `OST-guider-analysis.md` already caught and fixed several real bugs
  (biased calibration average, `atan`→`atan2`, `int`→`double` rates, DEC
  projection formula).

---

## F. The `_matchedCurFirst.size() < 2` abort during guiding — diagnosis

> **Status: addressed.** The triangle matcher (`Trig` / `buildIndexes` /
> `matchIndexes` / `_trig*`) is removed. New `startracker.{h,cpp}`:
> vote for the dominant translation between the two star lists, then a
> two-stage-gate + sigma-clip + flux-weighted refit → sub-pixel drift,
> immune to flux-rank reshuffle. `SMComputeGuide` now skips the frame on a
> lost correlation and only aborts after `guideParams/maxmatchfail` (default 5)
> in a row. Not yet live-tested.

Symptom (live): guiding runs, then intermittently hits `SMComputeGuide` line
~1036 → "Can't compare current image with reference - Abort", while the frame
on screen looks perfectly clean.

**The image display is not the problem.** `OnSucessSEP()` writes the JPEG and
calls `getEltImg("image","image")->setValue(dta, true)` at lines ~1359-1362 —
*before* `FindStarsDone` fires, i.e. before `SMComputeGuide` even runs. The
frame is published every time. If it's not visible in the UI after an abort,
that's the Angular guider component gating the image on "guiding active", not
the backend. So: **the frame you see is the frame the matcher choked on** —
the stars are fine, `matchIndexes()` is what failed.

### Why `matchIndexes()` is fragile

1. **Side lengths are compared by position, not as a set.**
   `buildIndexes` builds each triangle from stars in `_solver.stars` order
   (`i<j<k`), so `d12 = |stars[i]-stars[j]|`, `d13 = |stars[i]-stars[k]|`,
   `d23 = |stars[j]-stars[k]|`. `matchIndexes` then requires
   `r.d12≈a.d12 && r.d13≈a.d13 && r.d23≈a.d23` **positionally**.
   `_solver.stars` comes from `StellarSolver::getStarList()`, which is
   **flux-sorted (brightest first)**. The moment seeing/transparency makes two
   similar-brightness stars swap rank between frames, every triangle involving
   that pair gets its `d13`/`d23` labels swapped → the positional test fails
   even though it is the *same* triangle. `r.s` and `r.p` still match (they're
   symmetric), which is why it's not a total blackout — just a sudden collapse
   in matched pairs. **Intermittent, field looks clean → exactly this.**

2. **`±0.1 %` absolute tolerance is far below centroid noise.**
   For a 30 px side, 0.1 % = 0.03 px; real centroid repeatability is
   ~0.1-0.3 px even on good stars. So any triangle with a short side is
   essentially unmatchable; only all-long-sided triangles have a fighting
   chance. Few stars in the frame → few long triangles → `< 2` pairs.

3. **`buildIndexes` keeps "the first 10" (brightest 10), not a stable set.**
   A star flickering across the keep-10 boundary shifts every index → a
   different triangle set vs `_trigFirst`.

4. **No outlier rejection on the drift.** `dx/dy` is a plain mean of every
   matched pair. One spurious triangle match (more likely precisely when
   sides are short and the 0.1 % window is noise-sized) injects a bogus pair
   and yanks the mean.

5. **`emit Abort()` is too harsh a response.** The "no stars" branch just
   above does `logWarning(...) + emit ComputeGuideDone(); return;` (skip the
   frame). A transient match failure should do the same and only abort after
   *N consecutive* failures — that alone turns this bug into a non-event.

### Recommended fix (increasing effort)

- **Quick:** on `_matchedCurFirst.size() < 2`, skip the frame like the
  no-stars case; count consecutive misses; abort only after e.g. 5. Widen the
  tolerance to `max(1.0 px, 1 % · side)` and compare **sorted** sides
  (`sort d12,d13,d23` in both `buildIndexes` output and the test), keeping a
  sorted-side → opposite-vertex map so point correspondence is still
  recoverable. Add a median-based outlier reject on the pairs before averaging.
- **Better:** triangles are overkill for a *guiding loop*. Use triangle
  matching **once** at guide start (establish correspondence + orientation),
  then track frame-to-frame by nearest-neighbour: for each reference star look
  for the current star nearest to `(xr + predictedDrift)` within a small
  radius, predictedDrift seeded from the previous frame. Robust, O(n), immune
  to flux-rank shuffle.
- Raise the star cap (10 → ~20) and/or pick the *sharpest* stars, not just the
  brightest.

---

## G. Pier-side / correction inversion — advice

Your instinct is right: **calibrate, then guide immediately at the same pier
side → the transform must be the identity** (no inversion, orientation used
as-measured). Any code path that can flip a sign in that case is a bug.

### What the flip actually is

On a GEM, a meridian flip rotates the field 180° (camera bolted to the OTA).
For this module's reflection matrix `(cosθ, sinθ ; sinθ, −cosθ)`, adding 180°
to θ gives `cos→−cos, sin→−sin`, i.e. `driftRA→−driftRA` **and**
`driftDE→−driftDE`. So "flip both `revRA` and `revDE`" *is* the correct
meridian-flip handling here — the current `enablepiersidereverse` branch is
right in principle. The only extra real-world case is mounts where the DEC
motor sense does *not* invert on flip; Ekos exposes that as
`reverseDecOnPierChange`. Leave that as a future escape hatch, don't build it
now.

### The bookkeeping is the weak point

The same "reverse RA" bit lives in three places:
`revCorrections.revRA` (live/manual), `calibrationvalues.revRA` (snapshot
written by `SMComputeCal`), and it gets *mutated in place* by the pier-flip
logic in `SMInitGuide`. It happens not to compound across restarts (each start
reloads the snapshot first), but it's very hard to follow.

**Suggestion:** make it derived, not stored-and-mutated.
- `calibrationvalues` holds only what was true at calibration:
  `calCcdOrientation`, `calPier`, the rates, `calMountDEC`.
- At guide start compute, without persisting:
  `flip = enablepiersidereverse && (currentPier != calPier)`
  `effectiveRevRA = calManualRevRA XOR flip` (same for DEC)
  where `calManualRevRA` is a pure user override for odd camera wiring.
- Never write back into `revCorrections` from `SMInitGuide`.

That removes the "which `revRA` is the real one" question entirely.

### Testing without a rotating simulator
> **Point 3 done:** `guideParams/simflip` — rotates `starsFirst` 180 deg about
> the frame centre at guide start, so the flip / rev-bit math is testable
> against the CCD Simulator. Points 1-2 need no code; point 4 (sim patch) not
> done. The rev-bit simplification (derive, don't store-and-mutate) is still TODO.

The INDI CCD Simulator does **not** rotate its field with `TELESCOPE_PIER_SIDE`,
so you can't validate flip handling end-to-end with it. Workarounds:

1. **Test the sign plumbing directly:** calibrate, then manually toggle
   `revCorrections.revRA` / `revDE` and confirm the star *runs away* instead of
   being held. Validates the inversion path with no flip needed.
2. **Fire the auto-detect branch:** the Telescope Simulator *does* report and
   change `TELESCOPE_PIER_SIDE` (slew across the meridian, or set the property
   directly). With `enablepiersidereverse` on you'll see the
   `currentPier != calPier` branch execute — the sim will then guide *away*
   (expected, since its field didn't actually rotate), which at least proves
   the branch triggers on the right condition.
3. **Add an OST-side "simulate meridian flip" debug toggle** (hidden
   `guideParams` bool): when set, rotate the stored `_trigFirst` coordinates
   by 180° about the frame centre at guide start. Now the *whole* flip math
   (orientation + rev bits + DEC transfer) is regression-testable against the
   CCD Simulator, entirely inside OST. This is the cleanest option and doubles
   as a permanent test hook.
4. Longer term: a small INDI CCD-sim patch that rotates the generated field
   180° on `PIER_EAST` would give a true end-to-end test, but (3) gets you
   90 % of the value for 10 % of the work.

### Also fix A1 while you're in here

The DEC-declination *transfer* of the RA rate (`/cos(calDEC)` on store,
`*cos(curDEC)` on use) cancels for same-declination guiding — so it won't
show up in your "calibrate then guide now" tests — but it's inverted for
cross-declination. Easiest bench check: calibrate at DEC≈0 on the sim, slew
the sim mount to DEC≈60 without recalibrating, guide, and watch whether RA
corrections are ~4× too small (current code) or sane (fixed).

---

## Suggested priority

1. **F** — the live abort. Quick win first: skip-frame + N-consecutive-failure
   counter instead of `emit Abort()`. Then fix `matchIndexes` properly (sorted
   sides, sane tolerance, outlier reject) or move the loop to nearest-neighbour
   tracking. This is what's biting right now.
2. **A1** — confirm the DEC-transfer sign on the simulator at two declinations
   (calibrate at δ≈0, guide at δ≈60 without recal). Cancels for same-δ so your
   "calibrate then guide" tests won't reveal it.
3. **A2 / A3 / A4** — small, safe robustness fixes (abort on pier-side failure,
   pulse/exposure watchdog, BLOB guard).
4. **G** — simplify the rev-bit bookkeeping (derive, don't store-and-mutate)
   and add the "simulate meridian flip" debug toggle so pier handling is
   testable against the CCD Simulator.
5. **C** — delete the dead `polynomialfit.*`, dead members, dead `calguide`
   path; unify the action-button guards.
6. **B3 / B4** — calibration return-to-origin + drift-magnitude gate, and a
   first-frame settle. Medium effort, meaningful accuracy gain.
7. **B1 / B2** — use the N/S vectors to refine orientation (or port Ekos'
   two-angle average + ±90° disambiguation).
8. **D1** — consider consolidating the three SMs (larger refactor).
