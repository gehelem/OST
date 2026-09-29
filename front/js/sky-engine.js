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
    this.dayMode = false;
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
    // Day mode: a plain sky-blue fill instead of the transparent canvas
    // (which lets #bg-layer's own near-black CSS background show through -
    // see panels.css). No real atmospheric simulation (extinction, twilight
    // gradient...) - that's real future work (astro2001-style, see the
    // note), this is deliberately just a flat on/off "is it meant to look
    // like daytime" toggle, cheap and unambiguous.
    if (this.dayMode) {
      ctx.fillStyle = "#8ec8e8";
      ctx.fillRect(0, 0, w, h);
    }
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

    // Dome edge. Once panned, this is just the current view's own field-of-
    // view boundary, not the true horizon (which would generally no longer
    // be a circle on screen at all) - see sky-projection.js's
    // horizontalToScreen doc, and showTrueHorizon below for the actual
    // horizon reference.
    ctx.strokeStyle = "rgba(255,255,255,0.25)";
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
      ctx.strokeStyle = "rgba(255,193,7,0.55)";
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

    // Cardinal points. Projected through the same rotation as everything
    // else, so they move/disappear with the pan like any other sky feature.
    if (this.showCardinals) {
      ctx.fillStyle = "rgba(255,255,255,0.55)";
      ctx.font = "11px sans-serif";
      ctx.textAlign = "center";
      ctx.textBaseline = "middle";
      for (const [az, label] of [[0, "N"], [90, "E"], [180, "S"], [270, "O"]]) {
        const p = toScreen(horizontalToVector(az, 0));
        if (p.visible) ctx.fillText(label, p.x, p.y);
      }
    }

    const project = (raDeg, decDeg) => {
      const { azimuth, altitude } = equatorialToHorizontal(raDeg, decDeg, this.observer, date);
      return toScreen(horizontalToVector(azimuth, altitude));
    };

    // Constellation lines.
    if (this.showConstellations) {
      ctx.strokeStyle = "rgba(100,160,220,0.35)";
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
    ctx.fillStyle = "#fff";
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
      ctx.strokeStyle = "rgba(129,199,132,0.85)";
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
  }
}
