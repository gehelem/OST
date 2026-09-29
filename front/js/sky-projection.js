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

import { Observer, Horizon } from "../lib/astronomy-engine.js";

/** Builds an astronomy-engine Observer from a site (degrees, degrees, metres). */
export function makeObserver(latDeg, lonDeg, elevM) {
  return new Observer(latDeg, lonDeg, elevM || 0);
}

/**
 * RA (degrees, J2000-ish "of date" per Horizon()'s own contract)/Dec
 * (degrees) -> {azimuth, altitude} (degrees), for one observer at one
 * instant. Horizon() itself wants RA in sidereal hours, hence the /15.
 */
export function equatorialToHorizontal(raDeg, decDeg, observer, date) {
  const h = Horizon(date, observer, raDeg / 15, decDeg, "normal");
  return { azimuth: h.azimuth, altitude: h.altitude };
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
 * Orthographic projection of one unit vector (already rotated to view
 * space by the caller if the dome is panned — see sky-engine.js) onto the
 * dome circle of the given pixel radius. Returns {x, y, visible} —
 * screen-space offsets from the dome's own center (the caller adds its own
 * center point); visible=false below the horizon-equivalent plane (z<0),
 * the caller decides whether to still draw something there.
 */
export function vectorToScreen(v, radiusPx) {
  return { x: -radiusPx * v.x, y: -radiusPx * v.y, visible: v.z >= 0 };
}

/**
 * Inverse of vectorToScreen: a screen point (offset from the dome's own
 * center) -> the unit vector it corresponds to. Used to find what's "under
 * the cursor" for the trackball drag. Points outside the dome circle are
 * clamped onto its rim (z=0) instead of producing NaN — dragging past the
 * edge is common and must stay well-defined.
 */
export function screenToVector(screenX, screenY, radiusPx) {
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
 * vectorToScreen(horizontalToVector(az, alt), radiusPx). Once the dome can
 * be panned (sky-engine.js), everything drawn — including the cardinal
 * labels — goes through the rotated vector instead, so its position on
 * screen reflects the current view like everything else. */
export function horizontalToScreen(azimuthDeg, altitudeDeg, radiusPx) {
  return vectorToScreen(horizontalToVector(azimuthDeg, altitudeDeg), radiusPx);
}

/** Star/DSO magnitude -> dot radius in px, brighter (lower mag) = bigger.
 * Simple linear taper, clamped, tuned by eye against stars.6.json/stars.8.json
 * (mag -1.4 .. 8) rather than derived from any "real" formula. */
export function magnitudeToRadius(mag) {
  const r = 3.2 - mag * 0.34;
  return Math.max(0.4, Math.min(3.2, r));
}
