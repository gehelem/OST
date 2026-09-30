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
  Rotation_EQJ_HOR, Rotation_HOR_EQJ, Rotation_EQD_HOR, Rotation_ECL_EQJ,
} from "../lib/astronomy-engine.js";

/** Builds an astronomy-engine Observer from a site (degrees, degrees, metres). */
export function makeObserver(latDeg, lonDeg, elevM) {
  return new Observer(latDeg, lonDeg, elevM || 0);
}

/**
 * Rotation_EQJ_HOR(date, observer) on its own - depends only on the
 * current instant + site, not on any individual star, but
 * equatorialToHorizontal() below used to recompute it from scratch on
 * every single call. Fine at a few hundred catalog objects; at the
 * ~118k-star "full" depth (Gilles, 2026-10-03: "le grand saut ... on
 * passe la taille au dessus sur les catalogues") that repeated,
 * per-star sidereal-time/precession/nutation computation is real
 * measured cost - ~96ms/frame, unusable for interactive dragging.
 * Callers with many points to project in the same frame (sky-engine.js's
 * own project()) compute this ONCE and pass it into
 * equatorialToHorizontal()'s own optional 5th argument instead.
 */
export function equatorialHorizontalRotation(date, observer) {
  return Rotation_EQJ_HOR(date, observer);
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
 * navigator.js). `rotation`, if given (equatorialHorizontalRotation()'s
 * own return value), replaces the internal Rotation_EQJ_HOR(date,
 * observer) call - see that function's own doc comment for why a caller
 * projecting many points in the same frame would want to pass this in.
 */
export function equatorialToHorizontal(raDeg, decDeg, observer, date, rotation) {
  const eqjVec = VectorFromSphere(new Spherical(decDeg, raDeg, 1), date);
  const horVec = RotateVector(rotation || Rotation_EQJ_HOR(date, observer), eqjVec);
  const sphere = HorizonFromVector(horVec, "normal"); // {lat: altitude, lon: azimuth}
  return { azimuth: sphere.lon, altitude: sphere.lat };
}

/**
 * RA/Dec (degrees, OF DATE - e.g. navigator.cpp's own mountposition, fed
 * straight from the mount's INDI EQUATORIAL_EOD_COORD property, "EOD"
 * being exactly "epoch of date") -> {azimuth, altitude} (degrees), for one
 * observer at one instant. Deliberately a SEPARATE function from
 * equatorialToHorizontal() above rather than the same one reused - that
 * one treats its input as J2000 and applies its own J2000->of-date
 * precession/nutation step internally (Rotation_EQJ_HOR); feeding it
 * already-of-date coordinates would silently apply that correction a
 * second time, reintroducing the same class of "small but real" position
 * error earlier work here specifically fixed (see that function's own doc
 * comment) - this one uses Rotation_EQD_HOR instead, which works directly
 * from sidereal time with no separate precession step, matching what the
 * input already is.
 */
export function equatorialOfDateToHorizontal(raDeg, decDeg, observer, date) {
  const eqdVec = VectorFromSphere(new Spherical(decDeg, raDeg, 1), date);
  const horVec = RotateVector(Rotation_EQD_HOR(date, observer), eqdVec);
  const sphere = HorizonFromVector(horVec, "normal");
  return { azimuth: sphere.lon, altitude: sphere.lat };
}

// A fixed, arbitrary instant - VectorFromSphere() takes a `time` argument
// but never actually uses it in the numeric conversion (plain spherical ->
// Cartesian), it's only there to timestamp the resulting Vector object for
// whatever rotation gets applied to it next. eclipticToEquatorial() below
// applies Rotation_ECL_EQJ(), itself also date-independent (J2000 mean
// ecliptic obliquity, a fixed constant) - so nothing here actually depends
// on which instant this is, unlike every other date-taking function in
// this file.
const ECLIPTIC_TIME_PLACEHOLDER = new Date("2000-01-01T00:00:00Z");

/**
 * Ecliptic longitude (degrees, latitude always 0 - the ecliptic PLANE
 * itself, not any specific body's actual position on a given date) ->
 * {ra, dec} (degrees, J2000 equatorial) - Gilles, 2026-10-01: "ajoute le
 * trait de l'ecliptique en option". A fixed, date-independent shape (the
 * J2000 mean ecliptic doesn't move on any timescale this app cares about)
 * via Rotation_ECL_EQJ() - feeds straight into the same J2000-equatorial
 * project() pipeline every other static reference line here already uses
 * (grids, constellation lines), rather than a separate ecliptic-of-
 * date/observer path.
 */
export function eclipticToEquatorial(lonDeg) {
  const eclVec = VectorFromSphere(new Spherical(0, lonDeg, 1), ECLIPTIC_TIME_PLACEHOLDER);
  const eqjVec = RotateVector(Rotation_ECL_EQJ(), eclVec);
  const eq = EquatorFromVector(eqjVec); // {ra: hours, dec: degrees}
  return { ra: eq.ra * 15, dec: eq.dec };
}

// Tiny local vec3 helpers for sensorFovCorners() below - not exported,
// this file already has plenty of its own vector conventions (unit
// vectors from horizontalToVector, quaternions in js/quaternion.js) and
// doesn't need a fourth.
function v3sub(a, b) { return { x: a.x - b.x, y: a.y - b.y, z: a.z - b.z }; }
function v3add(a, b) { return { x: a.x + b.x, y: a.y + b.y, z: a.z + b.z }; }
function v3scale(a, s) { return { x: a.x * s, y: a.y * s, z: a.z * s }; }
function v3dot(a, b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
function v3cross(a, b) { return { x: a.y * b.z - a.z * b.y, y: a.z * b.x - a.x * b.z, z: a.x * b.y - a.y * b.x }; }
function v3norm(a) { const m = Math.hypot(a.x, a.y, a.z) || 1; return { x: a.x / m, y: a.y / m, z: a.z / m }; }
const FOV_TANGENT_EPS_DEG = 1e-4;

/**
 * The 4 corners of a camera sensor's field of view, as {azimuth, altitude}
 * (degrees, ready for horizontalToVector()+toScreen()) - a genuine
 * gnomonic (tangent-plane/rectilinear) rectangle around the pointing
 * direction (raDeg/decDeg, OF DATE - see equatorialOfDateToHorizontal()'s
 * own doc comment), rotated by `orientationDeg` (StellarSolver's own
 * Solution::orientation, "the orientation angle of the image from North
 * in degrees" - applied here as the standard astrometric convention,
 * rotating from North through East; not independently verified against a
 * real solved frame in this environment, check against one once
 * available and flip the sign below if a real frame comes out mirrored).
 *
 * Deliberately NOT built from RA/Dec coordinate offsets (a "RA ±
 * dRA/cos(dec), Dec ± dDec" box, this function's own earlier version) -
 * Gilles, 2026-10-0X: "proche des poles elle suit les axes et se
 * transforme peu a peu en parallelogramme a mesure qu'on s'approche : ca
 * devrait rester un rectangle de dimensions fixes". A coordinate-offset
 * box is only a flat-sky approximation of what a real sensor images
 * (valid far from the pole, where RA/Dec locally looks flat, increasingly
 * wrong as RA lines converge toward a pole); a real sensor images a
 * genuine gnomonic projection of the sky. This instead builds an
 * orthonormal tangent-plane basis (toward celestial North, and
 * perpendicular to it/East) at the pointing direction itself, derived
 * NUMERICALLY from VectorFromSphere (rather than a hand-derived formula
 * for what "North"/"East" mean in whatever x/y/z convention it happens to
 * use internally) so it stays correct and self-consistent everywhere
 * except the exact pole itself - an inherent singularity, position angle
 * is itself undefined there, exactly like azimuth is undefined at the
 * zenith (and just as irrelevant in practice: no telescope points at the
 * pole to the arcsecond).
 */
export function sensorFovCorners(raDeg, decDeg, fovWidthDeg, fovHeightDeg, orientationDeg, observer, date) {
  const center = VectorFromSphere(new Spherical(decDeg, raDeg, 1), date);
  // Centered difference (samples on both sides of center, not just one) -
  // halves the truncation error of the tangent-direction estimate versus
  // a forward difference, for the same epsilon and no extra cost.
  const eastPlus = VectorFromSphere(new Spherical(decDeg, raDeg + FOV_TANGENT_EPS_DEG, 1), date);
  const eastMinus = VectorFromSphere(new Spherical(decDeg, raDeg - FOV_TANGENT_EPS_DEG, 1), date);
  const northPlus = VectorFromSphere(new Spherical(Math.min(89.999, decDeg + FOV_TANGENT_EPS_DEG), raDeg, 1), date);
  const northMinus = VectorFromSphere(new Spherical(Math.max(-89.999, decDeg - FOV_TANGENT_EPS_DEG), raDeg, 1), date);
  const eastDiff = v3sub(eastPlus, eastMinus);
  const northDiff = v3sub(northPlus, northMinus);
  const east = v3norm(v3sub(eastDiff, v3scale(center, v3dot(eastDiff, center))));
  const north = v3norm(v3sub(northDiff, v3scale(center, v3dot(northDiff, center))));

  const theta = ((orientationDeg || 0) * Math.PI) / 180;
  const cosT = Math.cos(theta), sinT = Math.sin(theta);
  // Rodrigues' rotation formula around `center` (already a unit vector).
  function rotateAroundCenter(v) {
    const kCrossV = v3cross(center, v);
    const kDotV = v3dot(center, v);
    return v3add(v3add(v3scale(v, cosT), v3scale(kCrossV, sinT)), v3scale(center, kDotV * (1 - cosT)));
  }
  const imageUp = rotateAroundCenter(north);
  const imageRight = rotateAroundCenter(east);

  const tanW = Math.tan(((fovWidthDeg / 2) * Math.PI) / 180);
  const tanH = Math.tan(((fovHeightDeg / 2) * Math.PI) / 180);
  return [[-1, -1], [1, -1], [1, 1], [-1, 1]].map(([sx, sy]) => {
    const offset = v3add(v3scale(imageRight, tanW * sx), v3scale(imageUp, tanH * sy));
    const cornerVec = v3norm(v3add(center, offset));
    const horVec = RotateVector(Rotation_EQD_HOR(date, observer), cornerVec);
    const sphere = HorizonFromVector(horVec, "normal");
    return { azimuth: sphere.lon, altitude: sphere.lat };
  });
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

/**
 * The dimmest star magnitude worth drawing at a given zoom level - Gilles,
 * 2026-10-03: "a niveau de zoom faible c'est tout a fait inutile
 * d'afficher toutes les etoiles, ca fait un gros nuage de points
 * illisible et ca ralentit l'affichage" (right after the "full" ~118k-star
 * depth made that failure mode obvious for the first time - the two
 * lighter depths were always small enough that it didn't matter). Matches
 * astro2001's own dynamic-limit approach rather than a fixed cutoff
 * independent of zoom (see notes/ASTRO2001_ANALYSE_COMPLETE.md: "Dezoom:
 * magnitude limite ~3-4 / zoom normal: ~7-9 / zoom serre: +11") -
 * logarithmic in zoom (each doubling reveals ~3 more magnitudes), clamped
 * to a sane range: by about 5x `zoomMin` the limit already reaches every
 * star any of this app's own catalogs contain (mag ~14), so zooming in
 * further past that point is pure magnification, not "revealing more
 * stars" - which is also the actual reason a deeper max zoom needed its
 * own separate fix (ZOOM_MAX, sky-engine.js) rather than this alone.
 */
export function starMagnitudeLimit(zoom, zoomMin) {
  const limit = 4 + 3 * Math.log2(zoom / zoomMin);
  return Math.max(2, Math.min(14, limit));
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
