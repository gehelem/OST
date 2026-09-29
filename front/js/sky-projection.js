// sky-projection.js — pure math for the sky-dome view: RA/Dec -> horizontal
// (alt/az, via astronomy-engine) -> a unit vector -> screen. Written from
// scratch (not ported from skymap/d3), per the planetarium chantier's own
// decision — see notes/NOTES-frontend-ostcore-planetarium.md ("le coeur
// projection/miroir/drag a ete rapiece sur plusieurs sessions").
//
// Orientation convention (worth stating explicitly, precisely because this
// is where skymap went wrong repeatedly): azimuth 0=North/90=East/180=
// South/270=West (compass convention, standard for Horizon()). Rendered
// with North at the top of the screen and, since this is a view of the
// INSIDE of the sky dome (looking up, not down at a map), East on the
// LEFT — same convention as any naked-eye planisphere. Verified by hand:
// at az=0 -> (x=0,y=-1) [top/North], az=90 -> (x=-1,y=0) [left/East],
// az=180 -> (x=0,y=1) [bottom/South], az=270 -> (x=1,y=0) [right/West].
//
// The vector form (horizontalToVector/vectorToScreen/screenToVector) is
// what makes panning possible without az/alt's own singularities near the
// poles: sky-engine.js rotates the vector (via js/quaternion.js) BEFORE
// projecting, rather than accumulating az/alt offsets directly (exactly
// the naive approach that made skymap's old drag "blow up near the
// zenith" — see the note).

import {
  Observer, Spherical, VectorFromSphere, RotateVector,
  HorizonFromVector, VectorFromHorizon, EquatorFromVector,
  Rotation_EQJ_HOR, Rotation_HOR_EQJ,
} from "../lib/astronomy-engine.js";

/** Builds an astronomy-engine Observer from a site (degrees, degrees, metres). */
export function makeObserver(latDeg, lonDeg, elevM) {
  return new Observer(latDeg, lonDeg, elevM || 0);
}

/**
 * RA/Dec (degrees, J2000 — the epoch every star/DSO catalog here uses) ->
 * {azimuth, altitude} (degrees, of date), for one observer at one instant.
 * Built entirely from astronomy-engine's own official rotation-matrix
 * primitives (Rotation_EQJ_HOR et al.) rather than a hand-derived formula,
 * specifically so the J2000->of-date precession/nutation step is handled
 * correctly and automatically — Gilles, 2026-09-29, having watched a GOTO
 * land in a "weird position": "tu dois avoir des problemes de conversion
 * degres/heures et/ou de jnow/j2000". An earlier version of this function
 * (Horizon()-based) skipped that step entirely, off by ~0.2-0.4deg in 2026
 * — small, but real, and worth fixing now that the right tool is known,
 * even though the actual "weird position" turned out to be the separate,
 * much larger targetra-in-degrees-not-hours bug (see templates/
 * navigator.js).
 */
export function equatorialToHorizontal(raDeg, decDeg, observer, date) {
  const eqjVec = VectorFromSphere(new Spherical(decDeg, raDeg, 1), date);
  const horVec = RotateVector(Rotation_EQJ_HOR(date, observer), eqjVec);
  const sphere = HorizonFromVector(horVec, "normal"); // {lat: altitude, lon: azimuth}
  return { azimuth: sphere.lon, altitude: sphere.lat };
}

/**
 * Horizontal coordinates -> unit vector {x,y,z}, in the SAME frame the
 * screen-space formulas below assume (z=up/zenith, and vectorToScreen's
 * -x/-y already bakes in the "North up, East left" convention — see this
 * file's own header). Always a genuine unit vector:
 * x^2+y^2+z^2 = cos^2(alt)*sin^2(az) + cos^2(alt)*cos^2(az) + sin^2(alt)
 *             = cos^2(alt) + sin^2(alt) = 1.
 */
export function horizontalToVector(azimuthDeg, altitudeDeg) {
  const az = azimuthDeg * Math.PI / 180;
  const alt = altitudeDeg * Math.PI / 180;
  const cosAlt = Math.cos(alt);
  return { x: cosAlt * Math.sin(az), y: cosAlt * Math.cos(az), z: Math.sin(alt) };
}

/**
 * Projects one unit vector (already rotated to view space by the caller if
 * the dome is panned — see sky-engine.js) onto the dome circle of the
 * given pixel radius. Returns {x, y, visible} — screen-space offsets from
 * the dome's own center (the caller adds its own center point);
 * visible=false below the horizon-equivalent plane (z<0), the caller
 * decides whether to still draw something there.
 *
 * `projection` picks between the two this engine supports (Gilles,
 * 2026-10-02: "dans les settings d'affichage prevois un toggle pour
 * afficher en stereographique/orthographique"), both parametrised the
 * same way — by θ, the angular distance from the view's own center (the
 * zenith when unpanned): z=cos(θ), and (v.x,v.y) already has magnitude
 * sin(θ) (unit vector) — so both reduce to picking a radial scale factor:
 *
 * - "orthographic" (the default so far, and the one every existing test
 *   assumes unless it says otherwise): r = radiusPx*sin(θ) - the "look
 *   down the barrel" projection, matching the naked eye's own apparent
 *   foreshortening near the horizon (compressed) relative to overhead
 *   (spread out). This is exactly `-radiusPx*(v.x,v.y)` already, no
 *   separate scale needed.
 * - "stereographic": r = radiusPx*tan(θ/2) = radiusPx*(v.x,v.y)/(1+v.z) -
 *   conformal (preserves angles/shapes locally, unlike orthographic),
 *   and trades the opposite way: more room near the horizon, more
 *   compression near the zenith. Both conventions are picked so θ=90°
 *   (the horizon, v.z=0) lands at r=radiusPx either way - sin(90°)=1 and
 *   tan(45°)=1 - so the dome's own edge means the same thing in both
 *   modes and toggling between them doesn't change domeRadius/zoom math
 *   at all, only how everything inside that edge is distributed.
 */
export function vectorToScreen(v, radiusPx, projection) {
  if (projection === "stereographic") {
    const scale = radiusPx / (1 + v.z);
    return { x: -scale * v.x, y: -scale * v.y, visible: v.z >= 0 };
  }
  return { x: -radiusPx * v.x, y: -radiusPx * v.y, visible: v.z >= 0 };
}

/**
 * Inverse of vectorToScreen (same `projection` convention - must match
 * whatever vectorToScreen() was actually rendered with, or "what's under
 * the cursor" and what's on screen would disagree): a screen point (offset
 * from the dome's own center) -> the unit vector it corresponds to. Used
 * to find what's "under the cursor" for the trackball drag. Points
 * outside the dome circle are clamped onto its rim (z=0) instead of
 * producing NaN — dragging past the edge is common and must stay
 * well-defined, in either projection (stereographic could mathematically
 * represent points past the front hemisphere too, but every OTHER piece
 * of this engine - ground masking, the dome edge itself - assumes "front
 * hemisphere only", so this stays consistent with that rather than
 * exploiting the wider range).
 */
export function screenToVector(screenX, screenY, radiusPx, projection) {
  if (projection === "stereographic") {
    let X = -screenX / radiusPx, Y = -screenY / radiusPx;
    const rho2 = X * X + Y * Y;
    if (rho2 > 1) {
      const k = 1 / Math.sqrt(rho2);
      return { x: X * k, y: Y * k, z: 0 };
    }
    const denom = 1 + rho2;
    return { x: (2 * X) / denom, y: (2 * Y) / denom, z: (1 - rho2) / denom };
  }
  let x = -screenX / radiusPx, y = -screenY / radiusPx;
  const r2 = x * x + y * y;
  if (r2 > 1) {
    const k = 1 / Math.sqrt(r2);
    x *= k; y *= k;
    return { x, y, z: 0 };
  }
  return { x, y, z: Math.sqrt(1 - r2) };
}

/** Convenience for the no-pan case (rotation = identity): equivalent to
 * vectorToScreen(horizontalToVector(az, alt), radiusPx, projection). Once
 * the dome can be panned (sky-engine.js), everything drawn — including the
 * cardinal labels — goes through the rotated vector instead, so its
 * position on screen reflects the current view like everything else. */
export function horizontalToScreen(azimuthDeg, altitudeDeg, radiusPx, projection) {
  return vectorToScreen(horizontalToVector(azimuthDeg, altitudeDeg), radiusPx, projection);
}

/** Star/DSO magnitude -> dot radius in px, brighter (lower mag) = bigger.
 * Simple linear taper, clamped, tuned by eye against stars.6.json/stars.8.json
 * (mag -1.4 .. 8) rather than derived from any "real" formula. */
export function magnitudeToRadius(mag) {
  const r = 3.2 - mag * 0.34;
  return Math.max(0.4, Math.min(3.2, r));
}

/** Inverse of horizontalToVector. */
export function vectorToHorizontal(v) {
  return { azimuth: Math.atan2(v.x, v.y) * 180 / Math.PI, altitude: Math.asin(Math.max(-1, Math.min(1, v.z))) * 180 / Math.PI };
}

/**
 * Horizontal (of date) coordinates -> RA/Dec (degrees, J2000) — the exact
 * inverse of equatorialToHorizontal(), same rotation-matrix primitives.
 * Used for "what RA/Dec did the user click on" (sky-engine.js's
 * selection), including setting a GOTO target (navigator.cpp's own
 * OnSucessSolve()/actions.gototarget both treat target.targetra/targetde
 * as J2000, converting to of-date internally via INDI::J2000toObserved) —
 * so returning J2000 here, not of-date, is required for that value to be
 * usable as-is.
 */
export function horizontalToEquatorial(azimuthDeg, altitudeDeg, observer, date) {
  const horVec = VectorFromHorizon(new Spherical(altitudeDeg, azimuthDeg, 1), date, "normal");
  const eqjVec = RotateVector(Rotation_HOR_EQJ(date, observer), horVec);
  const equ = EquatorFromVector(eqjVec); // {ra: hours, dec: degrees}
  return { ra: equ.ra * 15, dec: equ.dec };
}
