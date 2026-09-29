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
import { makeObserver, equatorialToHorizontal, horizontalToEquatorial, horizontalToVector, vectorToHorizontal, vectorToScreen, screenToVector, magnitudeToRadius } from "./sky-projection.js";
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

export const ZOOM_MIN = 0.5, ZOOM_MAX = 6;
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
// sparse zoomed out or an unreadable smear zoomed in. Major lines (thicker,
// full opacity) sit at 5x the minor spacing (Gilles's own "5°/1°" example,
// applied as a ratio rather than literal degrees so it still makes sense
// once the minor step itself is something other than 1°).
const GRAD_STEPS_DEG = [60, 30, 15, 10, 5, 2, 1, 0.5];
const GRAD_MAJOR_RATIO = 5;
function pickGradStepDeg(rangeDeg, zoom) {
  const maxLines = 12 * Math.max(1, zoom);
  let step = GRAD_STEPS_DEG[0];
  for (const s of GRAD_STEPS_DEG) {
    if (rangeDeg / s <= maxLines) step = s; else break;
  }
  return step;
}
function isMajorLine(value, majorStep) {
  const m = ((value % majorStep) + majorStep) % majorStep;
  return m < 1e-6 || majorStep - m < 1e-6;
}
// Strokes a polyline given as an array of already-projected {x,y,visible}
// points, breaking across any gap where the line leaves the currently
// visible (front) hemisphere and picking back up wherever it re-enters -
// same "moveTo on the first visible point after a gap" pattern the true
// horizon/meridian lines already used before this became a generic grid.
// Returns the first visible point, for a caller that wants to label the
// line (or null if none of it was visible at all).
function strokePolyline(ctx, points) {
  let started = false, first = null;
  ctx.beginPath();
  for (const p of points) {
    if (!p.visible) { started = false; continue; }
    if (!first) first = p;
    if (!started) { ctx.moveTo(p.x, p.y); started = true; } else { ctx.lineTo(p.x, p.y); }
  }
  ctx.stroke();
  return first;
}
// A grid line's coordinate value, drawn once near wherever it happens to
// first become visible - Gilles, 2026-10-01: "tu n'as pas ajoute les
// graduations, coordonnees equatoriales et azimutales comme je te l'avais
// demande": the lines alone aren't the "graduations", the coordinate
// numbers are. Only called for major lines (see render()) - a full grid at
// minor-line density would make every single line's own label unreadable
// clutter.
function drawCornerLabel(ctx, p, text) {
  ctx.fillText(text, p.x + 6, p.y - 6);
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
function fmtDegLabel(deg) {
  return `${Math.round(((deg % 360) + 360) % 360)}°`;
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
function altitudeAtOffset(offX, offY, domeRadius, rotation) {
  const viewVec = screenToVector(offX, offY, domeRadius);
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
function computeSkyPolygon(domeRadius, rotation) {
  const centerAlt = altitudeAtOffset(0, 0, domeRadius, rotation);
  const centerIsSky = centerAlt >= 0;
  let anyGround = !centerIsSky, anySky = centerIsSky;
  const points = [];
  for (let i = 0; i < GROUND_MASK_RAYS; i++) {
    const phi = (2 * Math.PI * i) / GROUND_MASK_RAYS;
    const dx = Math.cos(phi), dy = Math.sin(phi);
    const edgeAlt = altitudeAtOffset(dx * domeRadius, dy * domeRadius, domeRadius, rotation);
    const edgeIsSky = edgeAlt >= 0;
    if (!edgeIsSky) anyGround = true; else anySky = true;
    let r;
    if (edgeIsSky === centerIsSky) {
      r = centerIsSky ? domeRadius : 0; // whole ray agrees - no crossing to find
    } else {
      let lo = 0, hi = domeRadius;
      for (let k = 0; k < GROUND_MASK_BISECT_STEPS; k++) {
        const mid = (lo + hi) / 2;
        const midIsSky = altitudeAtOffset(dx * mid, dy * mid, domeRadius, rotation) >= 0;
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
    this.dayMode = false;
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
    this.onNeedsRender = opts.onNeedsRender || null; // called after wheel-zoom/drag changes, so the caller can re-render immediately instead of waiting for its own timer
    this.onSelect = opts.onSelect || null; // called on a tap/click that isn't a drag - see _handlePointerUp()
    this.onZoomChange = opts.onZoomChange || null; // called after wheel-zoom specifically (not pan/drag) - e.g. to persist it, see templates/navigator.js
    this.catalog = { stars: [], constellationLines: [], messier: [] };
    this._domeRadius = 1; // recomputed every render(), read by the drag/tap handlers to convert screen<->vector
    this._drag = null; // { grabVector (fixed-frame), rotationAtDragStart, startOffset, moved } while a gesture is in progress (mouse or touch)

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
    // Single-touch only for v1: touch-drag pans, a tap (like a click)
    // selects. No pinch-zoom yet (wheel-zoom is desktop-only so far) —
    // { passive: false } since we preventDefault to stop the page itself
    // from scrolling/zooming while panning the dome.
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
    const grabView = screenToVector(off.x, off.y, this._domeRadius);
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
    const cView = screenToVector(off.x, off.y, this._domeRadius);
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
    if (ev.touches.length !== 1) return;
    this._handlePointerDown(ev.touches[0].clientX, ev.touches[0].clientY);
    ev.preventDefault();
  }
  _onTouchMove(ev) {
    if (!this._drag || ev.touches.length !== 1) return;
    this._handlePointerMove(ev.touches[0].clientX, ev.touches[0].clientY);
    ev.preventDefault();
  }
  _onTouchEnd() { this._handlePointerUp(); }

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
      return vectorToScreen(rv, domeRadius);
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
    const view = screenToVector(offX, offY, domeRadius);
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
      ? { dome: "rgba(0,0,0,0.28)", cardinal: "rgba(0,0,0,0.6)", constellation: "rgba(40,80,140,0.45)", star: "#15181c", messier: "rgba(27,94,32,0.85)", horizon: "rgba(180,110,0,0.65)", equator: "rgba(0,121,107,0.6)", azimuthal: "rgba(123,31,162,0.6)", grad: "rgba(0,0,0,0.55)", ground: "#cdbb95" }
      : { dome: "rgba(255,255,255,0.25)", cardinal: "rgba(255,255,255,0.55)", constellation: "rgba(100,160,220,0.35)", star: "#fff", messier: "rgba(129,199,132,0.85)", horizon: "rgba(255,193,7,0.55)", equator: "rgba(77,208,225,0.6)", azimuthal: "rgba(206,147,216,0.6)", grad: "rgba(255,255,255,0.75)", ground: "#241b12" };

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

    const toScreen = (v) => {
      const rv = rotateVector(this.rotation, v);
      const p = vectorToScreen(rv, domeRadius);
      return { x: cx + p.x, y: cy + p.y, visible: p.visible };
    };
    const project = (raDeg, decDeg) => {
      const { azimuth, altitude } = equatorialToHorizontal(raDeg, decDeg, this.observer, date);
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
      const skyPoly = computeSkyPolygon(domeRadius, this.rotation);
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
    // horizontal -> screen chain for every other catalog object. Major
    // lines only get a coordinate label (RA in hours, Dec in degrees) -
    // labelling every minor line too would be unreadable clutter.
    if (this.showEquatorialGrid) {
      const raMinor = pickGradStepDeg(360, this.zoom), raMajor = raMinor * GRAD_MAJOR_RATIO;
      const decMinor = pickGradStepDeg(180, this.zoom), decMajor = decMinor * GRAD_MAJOR_RATIO;
      ctx.fillStyle = fg.grad;
      ctx.font = "10px sans-serif";
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
        const first = strokePolyline(ctx, pts);
        if (major && first) drawCornerLabel(ctx, first, fmtRaHoursLabel(ra));
      }
      for (const dec of rangeSteps(-90, 90, decMinor)) {
        const major = isMajorLine(dec, decMajor);
        ctx.strokeStyle = fg.equator;
        ctx.lineWidth = major ? 1.3 : 0.5;
        ctx.globalAlpha = major ? 1 : 0.55;
        const pts = [];
        for (let i = 0; i <= LINE_SAMPLES; i++) pts.push(project((360 * i) / LINE_SAMPLES, dec));
        const first = strokePolyline(ctx, pts);
        if (major && first) drawCornerLabel(ctx, first, fmtDegLabel(dec));
      }
      ctx.globalAlpha = 1;
    }

    // Azimuthal grid: vertical circles through the zenith (constant
    // azimuth) and circles parallel to the horizon (constant altitude),
    // native to the observer's own horizontal frame. Each azimuth value
    // and its antipode (az/az+180) together trace one full great circle
    // through the zenith - same as two opposite lines of longitude through
    // both poles - so only half the compass (0-180) needs enumerating.
    // az=0 is the local meridian itself: always passes through the
    // celestial pole (at altitude = observer latitude, any hemisphere -
    // see equatorialToHorizontal's own precession-correct math, used here
    // instead of the textbook alt=latitude shortcut) and keeps its own
    // special styling (Gilles, 2026-09-30: "pointille du nord a la
    // polaire, et continu de la polaire au sud") - dashed below the pole,
    // solid from the pole through the zenith to the South point, no matter
    // whether az=0 happens to land on a "major" line at the current zoom.
    // Every other azimuth/altitude line is a plain arc, styled only by
    // major/minor. Altitude lines skip 0 (that's showTrueHorizon's own
    // line, with its own ground-mask semantics, not duplicated here), and
    // neither system draws below the horizon (alt<0) - a "look at the sky"
    // grid, not a full free-floating sphere one.
    if (this.showAzimuthalGrid) {
      const azMinor = pickGradStepDeg(360, this.zoom), azMajor = azMinor * GRAD_MAJOR_RATIO;
      const altMinor = pickGradStepDeg(90, this.zoom), altMajor = altMinor * GRAD_MAJOR_RATIO;
      ctx.fillStyle = fg.grad;
      ctx.font = "10px sans-serif";
      ctx.textAlign = "center";
      ctx.textBaseline = "middle";
      const ARC_SAMPLES = 48;

      for (let az = 0; az < 180; az += azMinor) {
        const major = isMajorLine(az, azMajor);
        ctx.strokeStyle = fg.azimuthal;
        ctx.lineWidth = major ? 1.3 : 0.5;
        ctx.globalAlpha = major ? 1 : 0.55;
        if (az === 0) {
          const ncpAlt = equatorialToHorizontal(0, 90, this.observer, date).altitude;
          const splitAlt = Math.max(0, Math.min(90, ncpAlt));
          const drawMeridianArc = (azz, altFrom, altTo, dashed) => {
            if (altFrom === altTo) return;
            ctx.setLineDash(dashed ? [4, 4] : []);
            const pts = [];
            for (let i = 0; i <= ARC_SAMPLES; i++) pts.push(toScreen(horizontalToVector(azz, altFrom + (altTo - altFrom) * (i / ARC_SAMPLES))));
            strokePolyline(ctx, pts);
          };
          drawMeridianArc(0, 0, splitAlt, true);    // North point -> pole, dashed
          drawMeridianArc(0, splitAlt, 90, false);  // pole -> zenith, solid
          drawMeridianArc(180, 90, 0, false);       // zenith -> South point, solid
          ctx.setLineDash([]);
        } else {
          for (const azz of [az, az + 180]) {
            const pts = [];
            for (let i = 0; i <= ARC_SAMPLES; i++) pts.push(toScreen(horizontalToVector(azz, (90 * i) / ARC_SAMPLES)));
            const first = strokePolyline(ctx, pts);
            if (major && first) drawCornerLabel(ctx, first, fmtDegLabel(azz));
          }
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
        const first = strokePolyline(ctx, pts);
        if (major && first) drawCornerLabel(ctx, first, fmtDegLabel(alt));
      }
      ctx.globalAlpha = 1;
    }

    // Cardinal points. Projected through the same rotation as everything
    // else, so they move/disappear with the pan like any other sky feature.
    if (this.showCardinals) {
      ctx.fillStyle = fg.cardinal;
      ctx.font = "11px sans-serif";
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

    // Stars. Day mode: only the handful bright enough to plausibly still
    // show up against a lit sky (rough, not a real extinction model - see
    // the dayMode comment above render()'s own fillRect).
    ctx.fillStyle = fg.star;
    for (const s of this.catalog.stars) {
      if (this.dayMode && s.mag > 0) continue;
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
    for (const p of PLANETS) {
      const equ = Equator(p.body, date, this.observer, true, true);
      const h = Horizon(date, this.observer, equ.ra, equ.dec, "normal");
      const scr = toScreen(horizontalToVector(h.azimuth, h.altitude));
      if (!scr.visible) continue;
      ctx.fillStyle = p.color;
      ctx.beginPath();
      ctx.arc(scr.x, scr.y, p.radius, 0, 2 * Math.PI);
      ctx.fill();
      ctx.fillText(p.label, scr.x + p.radius + 3, scr.y);
    }

    if (groundClipActive) ctx.restore(); // matches the ctx.save() in the ground-mask block above
  }
}
