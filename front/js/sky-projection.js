// sky-projection.js — pure math for the sky-dome view: RA/Dec -> horizontal
// (alt/az, via astronomy-engine) -> screen. Written from scratch (not
// ported from skymap/d3), per the planetarium chantier's own decision — see
// notes/NOTES-frontend-ostcore-planetarium.md ("le coeur projection/miroir/
// drag a ete rapiece sur plusieurs sessions"). No drag/mirror handling here
// at all yet (v1 is a fixed, zenith-centered dome, wheel-zoom only) —
// exactly the class of bug that hurt skymap the most, deliberately deferred
// to its own dedicated, tested step rather than bolted on now.
//
// Orientation convention (worth stating explicitly, precisely because this
// is where skymap went wrong repeatedly): azimuth 0=North/90=East/180=
// South/270=West (compass convention, standard for Horizon()). Rendered
// with North at the top of the screen and, since this is a view of the
// INSIDE of the sky dome (looking up, not down at a map), East on the
// LEFT — same convention as any naked-eye planisphere. Verified by hand:
// at az=0 -> (x=0,y=-1) [top/North], az=90 -> (x=-1,y=0) [left/East],
// az=180 -> (x=0,y=1) [bottom/South], az=270 -> (x=1,y=0) [right/West].

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
 * Orthographic projection of one horizontal-coordinate point onto the dome
 * circle of the given pixel radius, centered on the zenith. Returns
 * {x, y, visible} — screen-space offsets from the dome's own center (the
 * caller adds its own center point), visible=false below the horizon (the
 * caller decides whether to still draw something there, e.g. a horizon
 * ring - this function only answers "is it geometrically above the
 * horizon").
 */
export function horizontalToScreen(azimuthDeg, altitudeDeg, radiusPx) {
  const az = azimuthDeg * Math.PI / 180;
  const alt = altitudeDeg * Math.PI / 180;
  const r = radiusPx * Math.cos(alt); // zenith (alt=90) -> r=0 (center); horizon (alt=0) -> r=radiusPx (dome edge)
  return {
    x: -r * Math.sin(az),
    y: -r * Math.cos(az),
    visible: altitudeDeg >= 0,
  };
}

/** Star/DSO magnitude -> dot radius in px, brighter (lower mag) = bigger.
 * Simple linear taper, clamped, tuned by eye against stars.6.json/stars.8.json
 * (mag -1.4 .. 8) rather than derived from any "real" formula. */
export function magnitudeToRadius(mag) {
  const r = 3.2 - mag * 0.34;
  return Math.max(0.4, Math.min(3.2, r));
}
