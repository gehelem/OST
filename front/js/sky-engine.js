// sky-engine.js — the sky-dome view: canvas render + wheel-zoom + trackball
// drag/pan. New, clean implementation (not ported from skymap) — see
// sky-projection.js's own header for why, and
// notes/NOTES-frontend-ostcore-planetarium.md for the full chantier
// context. Stars/constellations/Messier/Sun/Moon/planets, all computed
// live via astronomy-engine for whatever instant `getTime()` returns
// (never hardcoded to "now" — see the note's "barre de visibilite"
// requirement: this engine already supports an arbitrary displayed time,
// the UI to scrub it is a separate follow-up).
//
// Panning: a unit quaternion (js/quaternion.js) rotates each point's fixed-
// frame vector before projecting, instead of accumulating az/alt deltas
// directly — the latter is exactly what made skymap's old drag "blow up
// near the zenith" (see the note). The drag algorithm itself ("the
// grabbed point tracks the cursor exactly") is the standard arcball/
// trackball technique; verified against its own precise contract
// (dragged point lands exactly under the new cursor, for a range of
// drags including ones crossing the zenith) with a standalone script
// before this file used it at all.
//
// Selection: a tap/click (not a drag — distinguished by total pointer
// movement, see CLICK_THRESHOLD_PX) hit-tests the catalog in screen space
// and calls onSelect(). Deliberately not a right-click/long-press context
// menu (Gilles, 2026-09-29: "point epineux : la selection d'un objet, on
// fait comment ? ... attention = future app android pour qui ca va poser
// probleme") — works identically for mouse and (single-)touch, no
// platform-specific gesture at all; the caller (templates/navigator.js)
// shows the selection as a regular panel with action buttons, same as
// everything else in this app, not a menu at the tap position.

import { Body, Horizon, Equator } from "../lib/astronomy-engine.js";
import { makeObserver, equatorialToHorizontal, equatorialHorizontalRotation, horizontalToEquatorial, horizontalToVector, vectorToHorizontal, vectorToScreen, screenToVector, magnitudeToRadius, starMagnitudeLimit } from "./sky-projection.js";
import { identity, multiply, conjugate, rotateVector, fromTwoVectors } from "./quaternion.js";
import { loadCatalog } from "./sky-catalog.js";

// Below this many CSS px of total movement between pointerdown and
// pointerup, a gesture counts as a tap/click (selection) rather than a
// drag (pan) — Gilles, 2026-09-29: "clic droit... attention = future app
// android pour qui ca va poser probleme". Works identically for mouse and
// touch, no platform-specific gesture (no right-click, no long-press).
const CLICK_THRESHOLD_PX = 6;
// How close (CSS px) a tap needs to land to a catalog object to select it,
// rather than just recording the raw RA/Dec of the tap itself.
const HIT_RADIUS_PX = 12;

// Distance (CSS px) between two Touch points - the only thing pinch-to-zoom
// (_onTouchStart/_onTouchMove) needs from a TouchList's own two entries.
function touchDistance(touches) {
  return Math.hypot(touches[0].clientX - touches[1].clientX, touches[0].clientY - touches[1].clientY);
}

// ZOOM_MAX raised from 6 (Gilles, 2026-10-03: "le zoom max n'est pas
// assez fin, augmente la limite") - purely a magnification ceiling by
// this point (starMagnitudeLimit() below already reaches every star any
// catalog here contains well before ZOOM_MIN*5=2.5), useful for precisely
// judging position against nearby grid lines/objects (e.g. polar
// alignment against the meridian) rather than revealing more stars.
export const ZOOM_MIN = 0.5, ZOOM_MAX = 20;
// How many points to sample around the true horizon circle when
// showTrueHorizon is on — see render()'s own comment on why it's not just
// the dome edge once panned.
const HORIZON_SAMPLES = 72;

// Equatorial/azimuthal grid graduations (Gilles, 2026-09-30: "regles
// graduees dynamiquement en fonction du niveau de zoom", then 2026-10-01,
// correcting an earlier single-line-per-system pass: "ce que je veux
// c'est un cadrillage ! ... avec des epaisseurs differentes dynamiques (5°
// gras et 1° fin par exemple, mais qui s'ajuste en fonction de
// l'echelle)") — pickGradStepDeg() picks the finest of these "usual"
// intervals that keeps the line count under a zoom-scaled budget, so the
// grid gets denser as you zoom in instead of a fixed spacing that's either
// sparse zoomed out or an unreadable smear zoomed in.
const GRAD_STEPS_DEG = [60, 30, 15, 10, 5, 2, 1, 0.5];
function pickGradStepDeg(rangeDeg, zoom) {
  const maxLines = 12 * Math.max(1, zoom);
  let step = GRAD_STEPS_DEG[0];
  for (const s of GRAD_STEPS_DEG) {
    if (rangeDeg / s <= maxLines) step = s; else break;
  }
  return step;
}
// Major lines (thicker, full opacity - every line gets its own coordinate
// label regardless, see drawCornerLabel()) sit at 5x the minor spacing
// (Gilles's own "5°/1°" example) - but capped at 10°, never coarser
// (Gilles, 2026-10-02: "la determination du 'pas majeur' est curieuse
// lorsqu'on est completement dezoome, on va se limiter a 10° max, pas
// plus" - fully zoomed out, minorStep can be 60° and 5x that (300°) is
// nonsensical against a 360° range, leaving only 1-2 major lines total).
// If the minor step alone already exceeds 10° there's no room for a
// distinct major tier below the cap, so every drawn line just becomes
// major instead (nothing coarser to fall back to, and there are few
// enough lines at that zoom for that to look reasonable anyway).
const GRAD_MAJOR_RATIO = 5;
const GRAD_MAJOR_CAP_DEG = 10;
function pickMajorStepDeg(minorStep) {
  const ratioStep = minorStep * GRAD_MAJOR_RATIO;
  if (ratioStep <= GRAD_MAJOR_CAP_DEG) return ratioStep;
  const capped = Math.floor(GRAD_MAJOR_CAP_DEG / minorStep) * minorStep;
  return capped > 0 ? capped : minorStep;
}
function isMajorLine(value, majorStep) {
  const m = ((value % majorStep) + majorStep) % majorStep;
  return m < 1e-6 || majorStep - m < 1e-6;
}
// Picks a grid line's label anchor among its (already-projected)
// {x,y,visible} sample points: prefers the bottom-most one that's
// actually within the canvas rectangle, INSET by `margin` on every side
// (Gilles, 2026-10-02: "le tour de l'ecran, idealement en bas") -
// hemisphere-"visible" alone isn't enough, an anchor picked from
// hemisphere-visible-but-off-canvas points (e.g. once zoomed in past
// domeRadius > canvas size - see this file's own header) left labels
// nowhere on screen at any real zoom ("toujours KO"). `margin` needs to
// cover both drawCornerLabel()'s own offset (the text is drawn a bit
// *above* the anchor) and roughly half a label's own width/height, or a
// clamped anchor right at the canvas edge would still place the actual
// text outside it (caught the same way: re-verifying by actually checking
// the drawn fillText() coordinates, not just "is there an anchor at all").
// If none of the (necessarily finite) sample points land exactly inside
// the inset rectangle - common once zoomed in, where the on-canvas slice
// of a line can be a sliver much narrower than the sampling step - falls
// back to whichever sample is closest to it, clamped just inside. Only
// needs a modest, fixed sample count either way: "closest point" doesn't
// require actually hitting a narrow window the way "did we sample a point
// strictly inside it" would.
function pickLabelAnchor(points, w, h, margin) {
  let onCanvas = null;
  let near = null, nearDistSq = Infinity;
  const lo = margin, hiX = w - margin, hiY = h - margin;
  for (const p of points) {
    if (!p.visible) continue;
    if (p.x >= lo && p.x <= hiX && p.y >= lo && p.y <= hiY) {
      if (!onCanvas || p.y > onCanvas.y) onCanvas = p;
    } else {
      const dx = p.x < lo ? lo - p.x : (p.x > hiX ? p.x - hiX : 0);
      const dy = p.y < lo ? lo - p.y : (p.y > hiY ? p.y - hiY : 0);
      const d = dx * dx + dy * dy;
      if (d < nearDistSq) { nearDistSq = d; near = p; }
    }
  }
  if (onCanvas) return onCanvas;
  if (!near) return null;
  return { x: Math.min(Math.max(near.x, lo), hiX), y: Math.min(Math.max(near.y, lo), hiY) };
}
// A "bottom-most on-canvas" anchor (pickLabelAnchor() above) breaks down
// for a family of lines that all converge on one shared point at ONE end
// (e.g. every azimuth line meets every other one at the zenith): dropping
// just that exact shared sample (as every azimuth-line call site here
// does) isn't enough, since the very next sample in from it is still
// close enough that several different lines' "bottom-most" pick still
// lands in essentially the same spot on screen, near that shared point,
// rather than spread out (Gilles, 2026-10-02: "regroupent en haut,
// horizontal" - trimming only the exact duplicate wasn't the real fix).
// This instead walks `points` in the order given and returns the first
// one that's actually on-canvas - so passing them ordered away from the
// shared end (e.g. an azimuth line's own horizon-to-zenith sweep, in that
// order) anchors the label at each line's own distinguishing end instead.
// Falls back to pickLabelAnchor()'s nearest-point rule if nothing in
// `points` was on-canvas at all.
function pickOrderedLabelAnchor(points, w, h, margin) {
  const lo = margin, hiX = w - margin, hiY = h - margin;
  for (const p of points) {
    if (p.visible && p.x >= lo && p.x <= hiX && p.y >= lo && p.y <= hiY) return p;
  }
  return pickLabelAnchor(points, w, h, margin);
}
// Grid lines converge toward a pole/zenith, so once panned/zoomed near one
// (Gilles, 2026-10-02: "lorsque je m'approche du pole les graduations se
// resserrent autour du pole - normal / les libelles font pareil, et du
// coup ... tous les libelles se retrouvent presque au meme endroit") each
// individual line's own anchor is still correctly ON that line, but
// several different lines' anchors can end up only a few pixels apart -
// not the earlier "several lines picked the exact same shared point" bug
// (pickOrderedLabelAnchor() above), a distinct one where every anchor is
// genuinely distinct but too close together to read once the actual text
// is drawn. Labels are queued as {anchor, text, major} candidates instead
// of drawn immediately, major ones placed first (skipping fewer of them
// under crowding matters more - there are always far fewer to begin
// with), each checked with a real measureText()-based bounding box against
// every box already placed (any two grid labels can be different lengths,
// e.g. "0h" vs "12h40", so a fixed pixel radius would either false-negative
// on long labels or over-cull short ones) - overlapping candidates are
// simply skipped rather than drawn on top of each other. `placedBoxes` is
// shared across every label family in a single render() (RA/Dec/Az/Alt/
// meridian all queue into the same array) so none of them overlap each
// other either, not just within their own family.
function placeDeclutteredLabels(ctx, candidates, placedBoxes, fontPx, offsetPx) {
  const sorted = candidates.slice().sort((a, b) => (b.major ? 1 : 0) - (a.major ? 1 : 0));
  const padX = 3, padY = 2;
  for (const c of sorted) {
    const textWidth = ctx.measureText(c.text).width;
    const cx = c.anchor.x, cy = c.anchor.y - offsetPx;
    const box = { left: cx - textWidth / 2 - padX, right: cx + textWidth / 2 + padX, top: cy - fontPx / 2 - padY, bottom: cy + fontPx / 2 + padY };
    const overlaps = placedBoxes.some((b) => box.left < b.right && box.right > b.left && box.top < b.bottom && box.bottom > b.top);
    if (overlaps) continue;
    placedBoxes.push(box);
    ctx.globalAlpha = c.major ? 1 : 0.55;
    drawCornerLabel(ctx, c.anchor, c.text, offsetPx);
  }
  ctx.globalAlpha = 1;
}
// Strokes a polyline given as an array of already-projected {x,y,visible}
// points, breaking across any gap where the line leaves the currently
// visible (front) hemisphere and picking back up wherever it re-enters -
// and returns its label anchor (see pickLabelAnchor() above).
function strokePolyline(ctx, points, w, h, margin) {
  let started = false;
  ctx.beginPath();
  for (const p of points) {
    if (!p.visible) { started = false; continue; }
    if (!started) { ctx.moveTo(p.x, p.y); started = true; } else { ctx.lineTo(p.x, p.y); }
  }
  ctx.stroke();
  return pickLabelAnchor(points, w, h, margin);
}
// A grid line's coordinate value, anchored at strokePolyline()'s own
// bottom-most-on-canvas point - Gilles, 2026-10-01: "tu n'as pas ajoute
// les graduations, coordonnees equatoriales et azimutales comme je te
// l'avais demande": the lines alone aren't the "graduations", the
// coordinate numbers are. Called for EVERY visible-on-canvas line, major
// or minor (Gilles, 2026-10-02, after an earlier major-only pass:
// "chaque graduation doit porter un libelle de valeur"). `offsetPx` is in
// device pixels (render() scales it by the current devicePixelRatio, same
// as the font size itself - see render()'s own `uiFont`/`dpr` - a fixed
// CSS-pixel-sized offset would otherwise shrink to almost nothing on a
// high-DPI screen).
function drawCornerLabel(ctx, p, text, offsetPx) {
  ctx.fillText(text, p.x, p.y - offsetPx);
}
// RA graduation label, in hours - the "usual" unit for equatorial
// coordinates (as opposed to plain degrees) in every telescope-control
// context this app already uses (see templates/navigator.js's own
// fmtHours() for target RA). Compact ("2h"/"2h30", not "2h00m00s") since
// this is a quick on-sky reference, not a precision readout.
function fmtRaHoursLabel(raDeg) {
  const h = ((raDeg / 15) % 24 + 24) % 24;
  const hh = Math.floor(h + 1e-9);
  const mm = Math.round((h - hh) * 60);
  return mm === 0 ? `${hh}h` : `${hh}h${String(mm).padStart(2, "0")}`;
}
// Azimuth graduation label - wraps to [0,360), the correct convention for
// azimuth (and the only place this file used to wrap-to-positive Dec/Alt
// values too, which broke them - see fmtSignedDegLabel() below).
function fmtDegLabel(deg) {
  return `${Math.round(((deg % 360) + 360) % 360)}°`;
}
// Declination/altitude graduation label - both are signed, -90° to +90°,
// never wrapping (Gilles, 2026-10-02: "probleme sur l'affichage des
// declinaisons (en equatorial) et des altitude (en altaz) ... rien a voir
// avec les axes qu'ils sont censes graduer" - fmtDegLabel()'s own [0,360)
// wrap wasn't just visually off for these, it silently turned e.g. dec=-30
// into "330°", not a smaller mislabelling but a different, wrong value).
function fmtSignedDegLabel(deg) {
  return `${Math.round(deg)}°`;
}
// Multiples of `step` within [min, max], aligned through zero rather than
// from `min` itself - e.g. rangeSteps(-90, 90, 60) is [-60, 0, 60], not
// [-90, -30, 30, 90], so 0 (the equator/horizon itself) is always one of
// the lines regardless of step.
function rangeSteps(min, max, step) {
  const start = Math.ceil(min / step - 1e-9) * step;
  const out = [];
  for (let v = start; v <= max + 1e-9; v += step) out.push(Math.round(v * 1e6) / 1e6);
  return out;
}

// Ground mask (Gilles, 2026-10-01: "si horizon : la partie inferieure doit
// etre masquee ! (on ne voit quasiment pas le trait)"): when showTrueHorizon
// is on and the view has been panned enough to look toward/below the real
// horizon, the below-horizon portion of the current view gets filled with a
// distinct "ground" colour and everything normally drawn there (stars,
// lines...) gets clipped away, instead of relying on the thin horizon line
// alone to convey it.
//
// The real (fixed-frame) altitude at a given screen offset from the dome's
// centre - screenToVector() already clamps to the visible (front) hemisphere,
// so this is only meaningful for offsets within the dome disk.
function altitudeAtOffset(offX, offY, domeRadius, rotation, projection) {
  const viewVec = screenToVector(offX, offY, domeRadius, projection);
  const fixedVec = rotateVector(conjugate(rotation), viewVec);
  return vectorToHorizontal(fixedVec).altitude;
}

const GROUND_MASK_RAYS = 120;
const GROUND_MASK_BISECT_STEPS = 10;

/**
 * Returns the boundary of the *sky* portion of the currently visible dome,
 * as an array of {x,y} points (offsets from the dome centre) - or `null`
 * if the whole visible dome is real sky (nothing to mask, the common case
 * at the default zenith-centred view), or the string "none" if the whole
 * visible dome is below the real horizon (fully masked).
 *
 * The true horizon is a great circle (alt=0 is always 90° from the
 * zenith), so under an arbitrary rotation + orthographic projection it
 * divides the visible hemisphere into two regions separated by a single
 * curve. Rather than tracing that curve analytically (messy once it also
 * has to be closed off along the dome's own edge), this walks a ray from
 * the dome's centre to its edge at each of GROUND_MASK_RAYS angles and
 * binary-searches each ray for the altitude=0 crossing - naturally
 * produces a simple polygon (every point already expressed in its own
 * polar form) that handles every case uniformly: a ray with no crossing
 * just collapses to the centre (fully ground) or extends fully to the
 * edge (fully sky).
 */
function computeSkyPolygon(domeRadius, rotation, projection) {
  const centerAlt = altitudeAtOffset(0, 0, domeRadius, rotation, projection);
  const centerIsSky = centerAlt >= 0;
  let anyGround = !centerIsSky, anySky = centerIsSky;
  const points = [];
  for (let i = 0; i < GROUND_MASK_RAYS; i++) {
    const phi = (2 * Math.PI * i) / GROUND_MASK_RAYS;
    const dx = Math.cos(phi), dy = Math.sin(phi);
    const edgeAlt = altitudeAtOffset(dx * domeRadius, dy * domeRadius, domeRadius, rotation, projection);
    const edgeIsSky = edgeAlt >= 0;
    if (!edgeIsSky) anyGround = true; else anySky = true;
    let r;
    if (edgeIsSky === centerIsSky) {
      r = centerIsSky ? domeRadius : 0; // whole ray agrees - no crossing to find
    } else {
      let lo = 0, hi = domeRadius;
      for (let k = 0; k < GROUND_MASK_BISECT_STEPS; k++) {
        const mid = (lo + hi) / 2;
        const midIsSky = altitudeAtOffset(dx * mid, dy * mid, domeRadius, rotation, projection) >= 0;
        if (midIsSky === centerIsSky) lo = mid; else hi = mid;
      }
      r = (lo + hi) / 2;
    }
    points.push({ x: dx * r, y: dy * r });
  }
  if (!anyGround) return null;
  if (!anySky) return "none";
  return points;
}

const PLANETS = [
  { body: Body.Sun, color: "#ffd54f", radius: 5, label: "Sun" },
  { body: Body.Moon, color: "#e0e0e0", radius: 4.5, label: "Moon" },
  { body: Body.Mercury, color: "#b0bec5", radius: 2, label: "Mercury" },
  { body: Body.Venus, color: "#fff59d", radius: 2.5, label: "Venus" },
  { body: Body.Mars, color: "#ef5350", radius: 2, label: "Mars" },
  { body: Body.Jupiter, color: "#ffcc80", radius: 3, label: "Jupiter" },
  { body: Body.Saturn, color: "#fff9c4", radius: 2.5, label: "Saturn" },
];

export class SkyEngine {
  /**
   * @param {HTMLCanvasElement} canvas
   * @param {object} opts
   * @param {number} opts.lat / opts.lon / opts.elev - observer site (degrees, degrees, metres)
   * @param {() => Date} [opts.getTime] - the instant to render; defaults to "now". The
   *        caller can swap this out later (e.g. a time-scrub control) without touching the engine.
   */
  constructor(canvas, opts) {
    this.canvas = canvas;
    this.ctx = canvas.getContext("2d");
    this.observer = makeObserver(opts.lat || 0, opts.lon || 0, opts.elev || 0);
    this.getTime = opts.getTime || (() => new Date());
    this.zoom = 1;
    this.rotation = identity(); // view pan state - see this file's own header

    // Display settings (templates/navigator.js's "Settings" panel, Gilles,
    // 2026-09-30: "personnaliser l'affichage (reperes, horizon, niveau de
    // zoom, jour/nuit)") - plain public fields, the caller just sets them
    // and calls render() again, same as `zoom`/`rotation` already work.
    this.showConstellations = true;
    this.showCardinals = true;
    this.showMessier = true;
    this.showTrueHorizon = false;
    this.showEquatorialGrid = false;
    this.showAzimuthalGrid = false;
    // Independent of showAzimuthalGrid - Gilles, 2026-10-03: "je ne vois
    // toujours pas le meridien, d'ailleurs son affichage n'est pas
    // conditionne par un toggle dans les settings" - it used to only ever
    // draw as part of the full azimuthal grid's own az=0 special case,
    // with no way to show just it on its own.
    this.showMeridian = false;
    this.dayMode = false;
    // "orthographic" (default) or "stereographic" - see sky-projection.js's
    // own vectorToScreen()/screenToVector() doc comment for the actual
    // math and what each trades off (Gilles, 2026-10-02: "dans les
    // settings d'affichage prevois un toggle pour afficher en
    // stereographique/orthographique"). Passed through to every
    // vectorToScreen/screenToVector call in this file (rendering, drag,
    // tap-to-select, the ground mask's own ray-casting) - inconsistent
    // between them would mean "what's under the cursor" and "what's drawn
    // there" disagree.
    this.projection = opts.projection === "stereographic" ? "stereographic" : "orthographic";
    // Whether the *app's own* light/dark theme is currently light — set
    // and kept in sync by the caller (templates/navigator.js watches
    // <html data-theme>). Independent of dayMode (that's the astronomical
    // simulation, this is just "does the light/dark palette below need to
    // flip so it's still visible against whatever background ends up on
    // screen") but both end up driving the exact same palette choice below
    // (Gilles, 2026-09-30, having switched the app to light theme: "en
    // mode clair le fond reste noir ... il faut aussi inverser les
    // couleurs (etoiles noires par exemple)").
    this.lightUI = !!opts.lightUI;
    // The app's own global "UI size" scale (Gilles, 2026-10-02: "il
    // faudrait que les libelles ... tiennent compte du niveau d'affichage
    // Tt" - "Tt" being that "UI size" slider, app.js's own applyZoom(),
    // 0.7x-1.6x). Independent of `zoom` above (this engine's own sky-dome
    // zoom): body.style.zoom already makes the whole canvas element (and
    // so domeRadius, and everything drawn relative to it) bigger/smaller
    // for free via resize()'s own getBoundingClientRect() call, but a
    // label's own font size/offsets are absolute device-pixel constants
    // that don't automatically follow that - see uiFont()/labelOffsetPx
    // below, the same way `dpr` already has to be applied explicitly.
    // Kept in sync by the caller the same way lightUI is (watches
    // <html>'s own style attribute for app.js's --ui-zoom custom property
    // changing, alongside its existing data-theme watch).
    this.uiZoom = opts.uiZoom || 1;
    this.onNeedsRender = opts.onNeedsRender || null; // called after wheel-zoom/drag changes, so the caller can re-render immediately instead of waiting for its own timer
    this.onSelect = opts.onSelect || null; // called on a tap/click that isn't a drag - see _handlePointerUp()
    this.onZoomChange = opts.onZoomChange || null; // called after wheel-zoom specifically (not pan/drag) - e.g. to persist it, see templates/navigator.js
    this.catalog = { stars: [], constellationLines: [], messier: [] };
    this._domeRadius = 1; // recomputed every render(), read by the drag/tap handlers to convert screen<->vector
    this._drag = null; // { grabVector (fixed-frame), rotationAtDragStart, startOffset, moved } while a gesture is in progress (mouse or touch)
    this._pinch = null; // { startDist (px), startZoom } while a two-finger pinch-to-zoom is in progress - see _onTouchStart()

    this._onMouseDown = this._onMouseDown.bind(this);
    this._onMouseMove = this._onMouseMove.bind(this);
    this._onMouseUp = this._onMouseUp.bind(this);
    this._onTouchStart = this._onTouchStart.bind(this);
    this._onTouchMove = this._onTouchMove.bind(this);
    this._onTouchEnd = this._onTouchEnd.bind(this);
    this._onWheel = this._onWheel.bind(this);
    canvas.addEventListener("wheel", this._onWheel, { passive: false });
    canvas.addEventListener("mousedown", this._onMouseDown);
    window.addEventListener("mousemove", this._onMouseMove);
    window.addEventListener("mouseup", this._onMouseUp);
    // Single-touch drags/pans, a tap (like a click) selects, two fingers
    // pinch-zoom (Gilles, 2026-10-03: "le zoom ... ne marche pas sur
    // tablette/smartphone (zoom/pinch)" - wheel-zoom was desktop-only) -
    // { passive: false } since we preventDefault to stop the page itself
    // from scrolling/zooming while panning/pinching the dome.
    canvas.addEventListener("touchstart", this._onTouchStart, { passive: false });
    canvas.addEventListener("touchmove", this._onTouchMove, { passive: false });
    canvas.addEventListener("touchend", this._onTouchEnd);
    canvas.addEventListener("touchcancel", this._onTouchEnd);
  }

  setSite(lat, lon, elev) {
    this.observer = makeObserver(lat, lon, elev || 0);
  }

  async loadCatalog(depth) {
    this.catalog = await loadCatalog(depth);
    // Bookkeeping only (sky-catalog.js's own loadCatalog() already
    // defaults an unrecognised depth to "light" internally) - lets a
    // caller (templates/navigator.js's settings panel) read back which
    // depth is actually loaded, e.g. to persist it, without keeping its
    // own separate copy of the same state.
    this.catalogDepth = depth;
  }

  _onWheel(ev) {
    ev.preventDefault();
    const f = ev.deltaY < 0 ? 1.1 : 1 / 1.1;
    this.zoom = Math.max(ZOOM_MIN, Math.min(ZOOM_MAX, this.zoom * f));
    if (this.onNeedsRender) this.onNeedsRender();
    if (this.onZoomChange) this.onZoomChange();
  }

  _screenCenter() {
    const r = this.canvas.getBoundingClientRect();
    return { cx: r.width / 2, cy: r.height / 2, rect: r };
  }

  /** CSS-pixel offset from the canvas's own center, for a {clientX,clientY}
   * point (a MouseEvent or one Touch both have these) — screenToVector/
   * vectorToScreen both work in that same offset space (see
   * sky-projection.js), not raw client/page coordinates. */
  _pointOffset(clientX, clientY) {
    const { cx, cy, rect } = this._screenCenter();
    return { x: clientX - rect.left - cx, y: clientY - rect.top - cy };
  }

  _handlePointerDown(clientX, clientY) {
    const off = this._pointOffset(clientX, clientY);
    const grabView = screenToVector(off.x, off.y, this._domeRadius, this.projection);
    // The point currently under the cursor, expressed in the FIXED
    // (unrotated) frame: view = rotation * fixed, so fixed = rotation^-1 *
    // view = conjugate(rotation) * view (unit quaternions: inverse = conjugate).
    const grabVector = rotateVector(conjugate(this.rotation), grabView);
    this._drag = { grabVector, rotationAtDragStart: this.rotation, startOffset: off, moved: false };
    this.canvas.classList.add("dragging");
  }

  _handlePointerMove(clientX, clientY) {
    if (!this._drag) return;
    const off = this._pointOffset(clientX, clientY);
    if (!this._drag.moved) {
      const d = Math.hypot(off.x - this._drag.startOffset.x, off.y - this._drag.startOffset.y);
      if (d > CLICK_THRESHOLD_PX) this._drag.moved = true;
    }
    const cView = screenToVector(off.x, off.y, this._domeRadius, this.projection);
    // The single minimal rotation taking the originally-grabbed view-space
    // point to the current cursor, composed onto the rotation that was
    // active at dragstart — see this file's header + quaternion.js's own
    // fromTwoVectors() doc for why this exact composition (verified with a
    // standalone script before this code existed at all).
    const grabView = rotateVector(this._drag.rotationAtDragStart, this._drag.grabVector);
    const delta = fromTwoVectors(grabView, cView);
    this.rotation = multiply(delta, this._drag.rotationAtDragStart);
    if (this.onNeedsRender) this.onNeedsRender();
  }

  _handlePointerUp() {
    if (!this._drag) return;
    const { startOffset, moved } = this._drag;
    this._drag = null;
    this.canvas.classList.remove("dragging");
    if (!moved && this.onSelect) this._select(startOffset.x, startOffset.y);
  }

  _onMouseDown(ev) { if (ev.button === 0) { this._handlePointerDown(ev.clientX, ev.clientY); ev.preventDefault(); } }
  _onMouseMove(ev) { this._handlePointerMove(ev.clientX, ev.clientY); }
  _onMouseUp() { this._handlePointerUp(); }

  _onTouchStart(ev) {
    if (ev.touches.length === 1) {
      this._pinch = null;
      this._handlePointerDown(ev.touches[0].clientX, ev.touches[0].clientY);
      ev.preventDefault();
    } else if (ev.touches.length === 2) {
      // Pinch-to-zoom (Gilles, 2026-10-03: "le zoom (molette souris) ne
      // marche pas sur tablette/smartphone (zoom/pinch)") - mirrors
      // _onWheel()'s own zoom clamp + onNeedsRender/onZoomChange calls,
      // just driven by the ratio of the current two-finger distance to
      // the distance when the pinch started instead of a discrete wheel
      // tick. A second finger landing mid-drag cancels that drag first -
      // the gesture just became a pinch, not a pan.
      this._drag = null;
      this.canvas.classList.remove("dragging");
      this._pinch = { startDist: touchDistance(ev.touches), startZoom: this.zoom };
      ev.preventDefault();
    }
  }
  _onTouchMove(ev) {
    if (this._pinch) {
      if (ev.touches.length !== 2) return; // a third finger, or one lifted without a touchend yet - ignore until back to exactly two
      const scale = touchDistance(ev.touches) / this._pinch.startDist;
      this.zoom = Math.max(ZOOM_MIN, Math.min(ZOOM_MAX, this._pinch.startZoom * scale));
      if (this.onNeedsRender) this.onNeedsRender();
      if (this.onZoomChange) this.onZoomChange();
      ev.preventDefault();
      return;
    }
    if (!this._drag || ev.touches.length !== 1) return;
    this._handlePointerMove(ev.touches[0].clientX, ev.touches[0].clientY);
    ev.preventDefault();
  }
  _onTouchEnd(ev) {
    if (this._pinch) {
      // ev.touches lists whatever's still down after this touch ended -
      // below two fingers, the pinch is over; don't fall through to
      // _handlePointerUp()'s own tap-to-select, a pinch is never a tap.
      if (ev.touches.length < 2) this._pinch = null;
      return;
    }
    this._handlePointerUp();
  }

  /** Finds the catalog object nearest a tap/click, within HIT_RADIUS_PX —
   * re-projects the catalog fresh (cheap: only runs once per tap, not per
   * frame) rather than caching render()'s own per-frame positions, so it
   * can never drift out of sync with what's actually on screen. Falls back
   * to the raw RA/Dec under the tap when nothing catalogued is close
   * enough (e.g. centering an arbitrary point, not tied to any object).
   * Calls onSelect({kind, name, ra, dec, mag}) — kind is "star"/"messier"/
   * null (null = the raw-point fallback, no name/mag). */
  _select(offX, offY) {
    const date = this.getTime();
    const domeRadius = this._domeRadius;
    const toOffset = (v) => {
      const rv = rotateVector(this.rotation, v);
      return vectorToScreen(rv, domeRadius, this.projection);
    };
    let best = null, bestDist = HIT_RADIUS_PX;
    const consider = (kind, obj) => {
      const { azimuth, altitude } = equatorialToHorizontal(obj.ra, obj.dec, this.observer, date);
      const p = toOffset(horizontalToVector(azimuth, altitude));
      if (!p.visible) return;
      const d = Math.hypot(p.x - offX, p.y - offY);
      if (d < bestDist) { bestDist = d; best = { kind, ...obj }; }
    };
    for (const s of this.catalog.stars) consider("star", s);
    for (const m of this.catalog.messier) consider("messier", m);

    if (best) { this.onSelect(best); return; }

    // Nothing catalogued close enough - report the raw point instead.
    const view = screenToVector(offX, offY, domeRadius, this.projection);
    const fixed = rotateVector(conjugate(this.rotation), view);
    const { azimuth, altitude } = vectorToHorizontal(fixed);
    if (altitude < 0) return; // tapped past the dome edge - not a real sky point
    const { ra, dec } = horizontalToEquatorial(azimuth, altitude, this.observer, date);
    this.onSelect({ kind: null, ra, dec });
  }

  /** Matches the canvas's drawing-buffer size to its CSS size (device-pixel
   * aware) - the caller wires this to a ResizeObserver, since sizing is a
   * DOM/layout concern, not this engine's own. */
  resize(cssWidth, cssHeight) {
    const dpr = window.devicePixelRatio || 1;
    this.canvas.width = Math.max(1, Math.round(cssWidth * dpr));
    this.canvas.height = Math.max(1, Math.round(cssHeight * dpr));
  }

  destroy() {
    this.canvas.removeEventListener("wheel", this._onWheel);
    this.canvas.removeEventListener("mousedown", this._onMouseDown);
    window.removeEventListener("mousemove", this._onMouseMove);
    window.removeEventListener("mouseup", this._onMouseUp);
    this.canvas.removeEventListener("touchstart", this._onTouchStart);
    this.canvas.removeEventListener("touchmove", this._onTouchMove);
    this.canvas.removeEventListener("touchend", this._onTouchEnd);
    this.canvas.removeEventListener("touchcancel", this._onTouchEnd);
  }

  render() {
    const { ctx, canvas } = this;
    const w = canvas.width, h = canvas.height;
    ctx.clearRect(0, 0, w, h);

    // Background + foreground palette. Two independent things can make the
    // background light — dayMode (simulating an actual daytime sky) and
    // lightUI (the app's own theme happens to be light) — either one needs
    // every foreground colour (stars first and foremost) to invert too, or
    // they're invisible against it (Gilles, 2026-09-30, having switched the
    // app to light theme: "en mode clair le fond reste noir ... il faut
    // aussi inverser les couleurs (etoiles noires par exemple)"). Drawn
    // explicitly here (ctx.fillRect) rather than left to #bg-layer's own
    // CSS background showing through the transparent canvas, so this stays
    // the one, authoritative place that decides the sky's own colours,
    // independent of any CSS/canvas timing.
    const isLightBg = this.dayMode || this.lightUI;
    const bgColor = this.dayMode ? "#8ec8e8" : (this.lightUI ? "#e4e9ee" : "#05070c");
    ctx.fillStyle = bgColor;
    ctx.fillRect(0, 0, w, h);
    const fg = isLightBg
      ? { dome: "rgba(0,0,0,0.28)", cardinal: "rgba(0,0,0,0.6)", constellation: "rgba(40,80,140,0.45)", star: "#15181c", messier: "rgba(27,94,32,0.85)", horizon: "rgba(180,110,0,0.65)", equator: "rgba(0,121,107,0.6)", azimuthal: "rgba(123,31,162,0.6)", meridian: "rgba(13,71,161,0.95)", grad: "rgba(0,0,0,0.55)", ground: "#cdbb95" }
      : { dome: "rgba(255,255,255,0.25)", cardinal: "rgba(255,255,255,0.55)", constellation: "rgba(100,160,220,0.35)", star: "#fff", messier: "rgba(129,199,132,0.85)", horizon: "rgba(255,193,7,0.55)", equator: "rgba(77,208,225,0.6)", azimuthal: "rgba(206,147,216,0.6)", meridian: "rgba(41,121,255,0.95)", grad: "rgba(255,255,255,0.75)", ground: "#241b12" };

    const cx = w / 2, cy = h / 2;
    const domeRadius = Math.min(w, h) / 2 * 0.96 * this.zoom;
    // Drag math (_onPointerDown/_onPointerMove) reads this via CSS pixels
    // (getBoundingClientRect), render() draws in device pixels (canvas.width/
    // height, dpr-scaled) - keep the radius used by both in sync by storing
    // it in CSS pixels here (matching _screenCenter()'s own units) and
    // dividing out the device-pixel ratio.
    const dpr = window.devicePixelRatio || 1;
    this._domeRadius = domeRadius / dpr;
    const date = this.getTime();
    // All drawing below happens in DEVICE pixels (canvas.width/height are
    // already dpr-scaled - see resize()). A literal "10px" font would then
    // render at 10 *device* pixels, i.e. as little as 3-5 CSS pixels tall
    // on a common 2x/3x-DPI screen - technically drawn, but small enough
    // to be effectively invisible (Gilles, 2026-10-02, after the previous
    // fix still left him seeing "strictement rien, absolument nulle part":
    // turned out to be exactly this - not a missing-label bug but an
    // illegible-label one). uiFont() keeps every label's actual on-screen
    // (CSS-pixel) size constant regardless of DPI, the same way
    // `_domeRadius` above already does for geometry. Also folds in
    // `uiZoom` (the app's own global "UI size", see this.uiZoom's own doc
    // comment) - every other piece of on-screen text in this app (panels,
    // buttons...) already grows/shrinks with it via body.style.zoom;
    // canvas text needs the same factor applied explicitly since it isn't
    // real DOM/CSS text.
    const labelScale = dpr * this.uiZoom;
    const uiFont = (px) => `${px * labelScale}px sans-serif`;
    const labelOffsetPx = 6 * labelScale;
    // Keeps a clamped label anchor (pickLabelAnchor()'s fallback path) far
    // enough from the canvas edge that the actual text - drawn offset
    // *above* the anchor, and centred, so it extends further still to
    // either side - never gets clipped off despite the anchor itself being
    // technically inside the rectangle.
    const labelMargin = labelOffsetPx + 24 * labelScale;

    const toScreen = (v) => {
      const rv = rotateVector(this.rotation, v);
      const p = vectorToScreen(rv, domeRadius, this.projection);
      return { x: cx + p.x, y: cy + p.y, visible: p.visible };
    };
    // Computed once per frame, not per catalog object - see
    // equatorialHorizontalRotation()'s own doc comment on why that matters
    // once the star count gets into the tens of thousands (Gilles,
    // 2026-10-03: "le grand saut ... on passe la taille au dessus sur les
    // catalogues" - the ~118k-star "full" depth this made possible).
    const eqjHorRotation = equatorialHorizontalRotation(date, this.observer);
    const project = (raDeg, decDeg) => {
      const { azimuth, altitude } = equatorialToHorizontal(raDeg, decDeg, this.observer, date, eqjHorRotation);
      return toScreen(horizontalToVector(azimuth, altitude));
    };

    // Dome edge. Once panned, this is just the current view's own field-of-
    // view boundary, not the true horizon (which would generally no longer
    // be a circle on screen at all) - see sky-projection.js's
    // horizontalToScreen doc, and showTrueHorizon below for the actual
    // horizon reference.
    ctx.strokeStyle = fg.dome;
    ctx.lineWidth = 1;
    ctx.beginPath();
    ctx.arc(cx, cy, domeRadius, 0, 2 * Math.PI);
    ctx.stroke();

    // True horizon (alt=0 in the FIXED frame, sampled and projected through
    // the current rotation like everything else) - distinct from the dome
    // edge above once panned. Off by default: mostly redundant with the
    // dome edge at the default (unpanned, zenith-centered) view, only
    // earns its own line once you've actually looked away from straight up.
    if (this.showTrueHorizon) {
      ctx.strokeStyle = fg.horizon;
      ctx.setLineDash([4, 4]);
      ctx.beginPath();
      let started = false;
      for (let i = 0; i <= HORIZON_SAMPLES; i++) {
        const az = (360 * i) / HORIZON_SAMPLES;
        const p = toScreen(horizontalToVector(az, 0));
        if (!p.visible) { started = false; continue; }
        if (!started) { ctx.moveTo(p.x, p.y); started = true; } else { ctx.lineTo(p.x, p.y); }
      }
      ctx.stroke();
      ctx.setLineDash([]);
    }

    // Ground mask: fills the below-horizon portion of the current view
    // with a distinct colour and clips everything drawn from here on
    // (equator/meridian/cardinals/constellations/stars/messier/planets) to
    // the real-sky region only - Gilles, 2026-10-01: "si horizon : la
    // partie inferieure doit etre masquee ! (on ne voit quasiment pas le
    // trait)": the thin dashed line above was easy to miss against a
    // starfield with nothing distinguishing "above" from "below" it.
    // groundClipActive tracks whether render() needs its own matching
    // ctx.restore() at the very end.
    let groundClipActive = false;
    if (this.showTrueHorizon) {
      const skyPoly = computeSkyPolygon(domeRadius, this.rotation, this.projection);
      if (skyPoly === "none") {
        ctx.fillStyle = fg.ground;
        ctx.beginPath();
        ctx.arc(cx, cy, domeRadius, 0, 2 * Math.PI);
        ctx.fill();
        ctx.save();
        ctx.beginPath(); // empty path -> clip to nothing, hides everything after
        ctx.clip();
        groundClipActive = true;
      } else if (skyPoly) {
        ctx.fillStyle = fg.ground;
        ctx.beginPath();
        ctx.arc(cx, cy, domeRadius, 0, 2 * Math.PI);
        ctx.fill();
        ctx.save();
        ctx.beginPath();
        skyPoly.forEach((p, i) => {
          if (i === 0) ctx.moveTo(cx + p.x, cy + p.y); else ctx.lineTo(cx + p.x, cy + p.y);
        });
        ctx.closePath();
        ctx.clip();
        // Restore the normal sky background within the clipped (sky-only)
        // region - the ground fill above covered the whole disk, this
        // repaints just the sky portion back over it.
        ctx.fillStyle = bgColor;
        ctx.fillRect(0, 0, w, h);
        groundClipActive = true;
      }
      // skyPoly === null: whole visible dome is real sky, nothing to mask.
    }

    // Collects {anchor, text, major} across BOTH grids below instead of
    // drawing each label immediately - see placeDeclutteredLabels()'s own
    // doc comment for why (labels from lines that converge near a pole/
    // zenith need to be checked against each other, and against every
    // other label already on screen, not just drawn independently).
    const labelCandidates = [];

    // Equatorial grid: lines of constant RA (pole to pole, like meridians
    // of longitude) and constant Dec (parallels, dec=0 being the celestial
    // equator itself) - Gilles, 2026-10-01, correcting an earlier
    // single-line-per-system pass: "ce que je veux c'est un cadrillage !
    // (pas uniquement les valeurs sur le meridien et l'horizon) - avec des
    // epaisseurs differentes dynamiques (5° gras et 1° fin par exemple,
    // mais qui s'ajuste en fonction de l'echelle)". Sampled/projected in
    // equatorial coordinates (unlike the azimuthal grid below, native to
    // the observer's own horizontal frame) since RA/Dec are fixed on the
    // sky regardless of where the observer is or what time it is -
    // project() (RA/Dec -> screen) already does the equatorial ->
    // horizontal -> screen chain for every other catalog object. Every
    // line carries its own coordinate label (RA in hours, Dec in degrees).
    if (this.showEquatorialGrid) {
      const raMinor = pickGradStepDeg(360, this.zoom), raMajor = pickMajorStepDeg(raMinor);
      const decMinor = pickGradStepDeg(180, this.zoom), decMajor = pickMajorStepDeg(decMinor);
      ctx.fillStyle = fg.grad;
      ctx.font = uiFont(11);
      ctx.textAlign = "center";
      ctx.textBaseline = "middle";
      const LINE_SAMPLES = 64;

      for (let ra = 0; ra < 360; ra += raMinor) {
        const major = isMajorLine(ra, raMajor);
        ctx.strokeStyle = fg.equator;
        ctx.lineWidth = major ? 1.3 : 0.5;
        ctx.globalAlpha = major ? 1 : 0.55;
        const pts = [];
        for (let i = 0; i <= LINE_SAMPLES; i++) pts.push(project(ra, -90 + (180 * i) / LINE_SAMPLES));
        strokePolyline(ctx, pts, w, h, labelMargin);
        // Anchor picked from pts[1..-1] specifically, not strokePolyline()'s
        // own full-array anchor - pts[0]/pts[-1] are the celestial poles
        // (dec=-90/+90), a single point shared by every RA line. Left in,
        // "bottom-most on-canvas" could pick that same shared point for
        // several different RA lines whenever it happens to be their own
        // extremal one, bunching their labels together there instead of
        // along each line's own path (Gilles, 2026-10-02: "regroupent en
        // haut" - the analogous azimuth/zenith case below, but the same
        // failure mode applies here too). Queued, not drawn immediately -
        // see placeDeclutteredLabels()'s own doc comment for why (lines
        // converging near a pole/zenith still each get their own correct,
        // but nearly-coincident, anchor).
        const anchor = pickLabelAnchor(pts.slice(1, -1), w, h, labelMargin);
        if (anchor) labelCandidates.push({ anchor, text: fmtRaHoursLabel(ra), major });
      }
      for (const dec of rangeSteps(-90, 90, decMinor)) {
        const major = isMajorLine(dec, decMajor);
        ctx.strokeStyle = fg.equator;
        ctx.lineWidth = major ? 1.3 : 0.5;
        ctx.globalAlpha = major ? 1 : 0.55;
        const pts = [];
        for (let i = 0; i <= LINE_SAMPLES; i++) pts.push(project((360 * i) / LINE_SAMPLES, dec));
        const anchor = strokePolyline(ctx, pts, w, h, labelMargin);
        if (anchor) labelCandidates.push({ anchor, text: fmtSignedDegLabel(dec), major });
      }
      ctx.globalAlpha = 1;
    }

    // Local meridian: its OWN toggle (Gilles, 2026-10-03, after it was
    // only ever drawn as part of showAzimuthalGrid's own az=0 special
    // case: "je ne vois toujours pas le meridien, d'ailleurs son
    // affichage n'est pas conditionne par un toggle dans les settings" -
    // there was no way to show just the meridian without the whole
    // azimuthal grid). Always passes through the celestial pole (at
    // altitude = observer latitude, any hemisphere - see
    // equatorialToHorizontal's own precession-correct math, used here
    // instead of the textbook alt=latitude shortcut). Bold, dedicated blue
    // (Gilles, 2026-10-02: "affiche le en bleu gras"), not the plain
    // azimuthal-grid colour/major-minor thickness, and split in two
    // styles right at the pole (Gilles, 2026-09-30: "pointille du nord a
    // la polaire, et continu de la polaire au sud") - dashed below the
    // pole, solid from the pole through the zenith to the South point.
    const ARC_SAMPLES = 48;
    if (this.showMeridian) {
      ctx.strokeStyle = fg.meridian;
      ctx.lineWidth = 2.2;
      ctx.globalAlpha = 1;
      ctx.fillStyle = fg.grad;
      ctx.font = uiFont(11);
      ctx.textAlign = "center";
      ctx.textBaseline = "middle";
      const ncpAlt = equatorialToHorizontal(0, 90, this.observer, date, eqjHorRotation).altitude;
      const splitAlt = Math.max(0, Math.min(90, ncpAlt));
      const drawMeridianArc = (azz, altFrom, altTo, dashed) => {
        if (altFrom === altTo) return;
        ctx.setLineDash(dashed ? [4, 4] : []);
        const pts = [];
        for (let i = 0; i <= ARC_SAMPLES; i++) pts.push(toScreen(horizontalToVector(azz, altFrom + (altTo - altFrom) * (i / ARC_SAMPLES))));
        strokePolyline(ctx, pts, w, h, labelMargin);
      };
      drawMeridianArc(0, 0, splitAlt, true);    // North point -> pole, dashed
      drawMeridianArc(0, splitAlt, 90, false);  // pole -> zenith, solid
      drawMeridianArc(180, 90, 0, false);       // zenith -> South point, solid
      ctx.setLineDash([]);
      // Every graduation gets its own value label (Gilles, 2026-10-02:
      // "chaque graduation doit porter un libellé de valeur"), the
      // meridian included - re-walk just its North-point-to-zenith half
      // (cheap, ARC_SAMPLES points) purely to find its own anchor. Drops
      // the last sample (alt=90, the zenith) the same way the regular
      // az-line branch below does - see its own comment on why.
      const northPts = [];
      for (let i = 0; i <= ARC_SAMPLES; i++) northPts.push(toScreen(horizontalToVector(0, (90 * i) / ARC_SAMPLES)));
      const northAnchor = pickOrderedLabelAnchor(northPts.slice(0, -1), w, h, labelMargin);
      if (northAnchor) labelCandidates.push({ anchor: northAnchor, text: fmtDegLabel(0), major: true });
    }

    // Azimuthal grid: vertical circles through the zenith (constant
    // azimuth) and circles parallel to the horizon (constant altitude),
    // native to the observer's own horizontal frame. Each azimuth value
    // and its antipode (az/az+180) together trace one full great circle
    // through the zenith - same as two opposite lines of longitude through
    // both poles - so only half the compass (0-180) needs enumerating.
    // az=0 is the local meridian itself, handled above by its own toggle
    // regardless of whether the rest of this grid is even on - skipped
    // here so it's never drawn twice when both are. Every other azimuth/
    // altitude line is a plain arc, styled only by major/minor. Altitude
    // lines skip 0 (that's showTrueHorizon's own line, with its own
    // ground-mask semantics, not duplicated here), and neither system
    // draws below the horizon (alt<0) - a "look at the sky" grid, not a
    // full free-floating sphere one.
    if (this.showAzimuthalGrid) {
      const azMinor = pickGradStepDeg(360, this.zoom), azMajor = pickMajorStepDeg(azMinor);
      const altMinor = pickGradStepDeg(90, this.zoom), altMajor = pickMajorStepDeg(altMinor);
      ctx.fillStyle = fg.grad;
      ctx.font = uiFont(11);
      ctx.textAlign = "center";
      ctx.textBaseline = "middle";

      for (let az = 0; az < 180; az += azMinor) {
        if (az === 0) continue; // the meridian - see its own toggle/block above
        const major = isMajorLine(az, azMajor);
        ctx.strokeStyle = fg.azimuthal;
        ctx.lineWidth = major ? 1.3 : 0.5;
        ctx.globalAlpha = major ? 1 : 0.55;
        for (const azz of [az, az + 180]) {
          const pts = [];
          for (let i = 0; i <= ARC_SAMPLES; i++) pts.push(toScreen(horizontalToVector(azz, (90 * i) / ARC_SAMPLES)));
          strokePolyline(ctx, pts, w, h, labelMargin);
          // Anchor picked from pts[0..-2], not strokePolyline()'s own
          // full-array anchor - pts's last sample is alt=90, the zenith,
          // a single point shared by every azimuth line (every vertical
          // circle passes through it). Left in, "bottom-most on-canvas"
          // could pick that shared point for several different azimuths
          // at once whenever it happens to be their own extremal one
          // (typically the "northern" ones, where the zenith's y is
          // larger than the arc's own near-horizon end) - caught by
          // dumping the actual drawn label positions, which showed many
          // different azimuths' labels landing on the exact same point
          // (Gilles, 2026-10-02: "regroupent en haut, horizontal").
          const anchor = pickOrderedLabelAnchor(pts.slice(0, -1), w, h, labelMargin);
          if (anchor) labelCandidates.push({ anchor, text: fmtDegLabel(azz), major });
        }
      }
      for (let alt = altMinor; alt < 90; alt += altMinor) {
        const major = isMajorLine(alt, altMajor);
        ctx.strokeStyle = fg.azimuthal;
        ctx.lineWidth = major ? 1.3 : 0.5;
        ctx.globalAlpha = major ? 1 : 0.55;
        const pts = [];
        const CIRCLE_SAMPLES = 96;
        for (let i = 0; i <= CIRCLE_SAMPLES; i++) pts.push(toScreen(horizontalToVector((360 * i) / CIRCLE_SAMPLES, alt)));
        const anchor = strokePolyline(ctx, pts, w, h, labelMargin);
        if (anchor) labelCandidates.push({ anchor, text: fmtSignedDegLabel(alt), major });
      }
      ctx.globalAlpha = 1;
    }

    if (labelCandidates.length) {
      ctx.fillStyle = fg.grad;
      ctx.font = uiFont(11);
      ctx.textAlign = "center";
      ctx.textBaseline = "middle";
      placeDeclutteredLabels(ctx, labelCandidates, [], 11 * labelScale, labelOffsetPx);
    }

    // Cardinal points. Projected through the same rotation as everything
    // else, so they move/disappear with the pan like any other sky feature.
    if (this.showCardinals) {
      ctx.fillStyle = fg.cardinal;
      ctx.font = uiFont(12);
      ctx.textAlign = "center";
      ctx.textBaseline = "middle";
      for (const [az, label] of [[0, "N"], [90, "E"], [180, "S"], [270, "O"]]) {
        const p = toScreen(horizontalToVector(az, 0));
        if (p.visible) ctx.fillText(label, p.x, p.y);
      }
    }

    // Constellation lines.
    if (this.showConstellations) {
      ctx.strokeStyle = fg.constellation;
      ctx.lineWidth = 1;
      for (const [a, b] of this.catalog.constellationLines) {
        const pa = project(a.ra, a.dec), pb = project(b.ra, b.dec);
        if (!pa.visible && !pb.visible) continue;
        ctx.beginPath();
        ctx.moveTo(pa.x, pa.y);
        ctx.lineTo(pb.x, pb.y);
        ctx.stroke();
      }
    }

    // Stars. Day mode keeps its own fixed, stricter cutoff (only the
    // handful bright enough to plausibly still show up against a lit sky -
    // rough, not a real extinction model, see the dayMode comment above
    // render()'s own fillRect); otherwise the limit tracks zoom (Gilles,
    // 2026-10-03: "a niveau de zoom faible c'est tout a fait inutile
    // d'afficher toutes les etoiles, ca fait un gros nuage de points
    // illisible et ca ralentit l'affichage" - see starMagnitudeLimit()'s
    // own doc comment). Checked BEFORE project() (the expensive part, one
    // equatorial->horizontal rotation per star), not just before drawing -
    // the whole point is to also skip the work, not only the pixels, once
    // the "full" ~118k-star depth is loaded at a wide-open, dezoomed view.
    const starMagLimit = this.dayMode ? 0 : starMagnitudeLimit(this.zoom, ZOOM_MIN);
    ctx.fillStyle = fg.star;
    for (const s of this.catalog.stars) {
      if (s.mag > starMagLimit) continue;
      const p = project(s.ra, s.dec);
      if (!p.visible) continue;
      const r = magnitudeToRadius(s.mag) * Math.sqrt(this.zoom);
      ctx.beginPath();
      ctx.arc(p.x, p.y, r, 0, 2 * Math.PI);
      ctx.fill();
    }

    // Messier objects: small open circles, distinct from stars.
    if (this.showMessier) {
      ctx.strokeStyle = fg.messier;
      ctx.lineWidth = 1;
      for (const m of this.catalog.messier) {
        const p = project(m.ra, m.dec);
        if (!p.visible) continue;
        ctx.beginPath();
        ctx.arc(p.x, p.y, 3.5, 0, 2 * Math.PI);
        ctx.stroke();
      }
    }

    // Sun/Moon/planets - live position for `date`, not from any static file
    // (a fixed file would drift the instant the displayed time isn't "now").
    ctx.textAlign = "left";
    ctx.font = uiFont(10);
    for (const p of PLANETS) {
      const equ = Equator(p.body, date, this.observer, true, true);
      const h = Horizon(date, this.observer, equ.ra, equ.dec, "normal");
      const scr = toScreen(horizontalToVector(h.azimuth, h.altitude));
      if (!scr.visible) continue;
      ctx.fillStyle = p.color;
      ctx.beginPath();
      ctx.arc(scr.x, scr.y, p.radius, 0, 2 * Math.PI);
      ctx.fill();
      ctx.fillText(p.label, scr.x + p.radius + 3 * labelScale, scr.y);
    }

    if (groundClipActive) ctx.restore(); // matches the ctx.save() in the ground-mask block above
  }
}
