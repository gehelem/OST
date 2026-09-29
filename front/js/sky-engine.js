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

import { Body, Horizon, Equator } from "../lib/astronomy-engine.js";
import { makeObserver, equatorialToHorizontal, horizontalToVector, vectorToScreen, screenToVector, magnitudeToRadius } from "./sky-projection.js";
import { identity, multiply, conjugate, rotateVector, fromTwoVectors } from "./quaternion.js";
import { loadCatalog } from "./sky-catalog.js";

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
    this.onNeedsRender = opts.onNeedsRender || null; // called after wheel-zoom/drag changes, so the caller can re-render immediately instead of waiting for its own timer
    this.catalog = { stars: [], constellationLines: [], messier: [] };
    this._domeRadius = 1; // recomputed every render(), read by the drag handlers to convert screen<->vector
    this._drag = null; // { grabVector (fixed-frame), rotationAtDragStart } while a drag is in progress

    this._onWheel = this._onWheel.bind(this);
    this._onPointerDown = this._onPointerDown.bind(this);
    this._onPointerMove = this._onPointerMove.bind(this);
    this._onPointerUp = this._onPointerUp.bind(this);
    canvas.addEventListener("wheel", this._onWheel, { passive: false });
    canvas.addEventListener("mousedown", this._onPointerDown);
    window.addEventListener("mousemove", this._onPointerMove);
    window.addEventListener("mouseup", this._onPointerUp);
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
    this.zoom = Math.max(0.5, Math.min(6, this.zoom * f));
    if (this.onNeedsRender) this.onNeedsRender();
  }

  _screenCenter() {
    const r = this.canvas.getBoundingClientRect();
    return { cx: r.width / 2, cy: r.height / 2, rect: r };
  }

  /** CSS-pixel offset from the canvas's own center, for a mouse event —
   * screenToVector/vectorToScreen both work in that same offset space
   * (see sky-projection.js), not raw client/page coordinates. */
  _eventOffset(ev) {
    const { cx, cy, rect } = this._screenCenter();
    return { x: ev.clientX - rect.left - cx, y: ev.clientY - rect.top - cy };
  }

  _onPointerDown(ev) {
    if (ev.button !== 0) return;
    const off = this._eventOffset(ev);
    const grabView = screenToVector(off.x, off.y, this._domeRadius);
    // The point currently under the cursor, expressed in the FIXED
    // (unrotated) frame: view = rotation * fixed, so fixed = rotation^-1 *
    // view = conjugate(rotation) * view (unit quaternions: inverse = conjugate).
    const grabVector = rotateVector(conjugate(this.rotation), grabView);
    this._drag = { grabVector, rotationAtDragStart: this.rotation };
    this.canvas.classList.add("dragging");
    ev.preventDefault();
  }

  _onPointerMove(ev) {
    if (!this._drag) return;
    const off = this._eventOffset(ev);
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

  _onPointerUp() {
    if (!this._drag) return;
    this._drag = null;
    this.canvas.classList.remove("dragging");
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
    this.canvas.removeEventListener("mousedown", this._onPointerDown);
    window.removeEventListener("mousemove", this._onPointerMove);
    window.removeEventListener("mouseup", this._onPointerUp);
  }

  render() {
    const { ctx, canvas } = this;
    const w = canvas.width, h = canvas.height;
    ctx.clearRect(0, 0, w, h);
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

    // Dome edge + cardinal points. Once panned, "the dome edge" is just the
    // current view's own field-of-view boundary, not the true horizon
    // (which would generally no longer be a circle on screen at all) - see
    // sky-projection.js's horizontalToScreen doc. Cardinal labels are
    // projected through the same rotation as everything else, so they
    // move/disappear with the pan like any other sky feature.
    ctx.strokeStyle = "rgba(255,255,255,0.25)";
    ctx.lineWidth = 1;
    ctx.beginPath();
    ctx.arc(cx, cy, domeRadius, 0, 2 * Math.PI);
    ctx.stroke();
    ctx.fillStyle = "rgba(255,255,255,0.55)";
    ctx.font = "11px sans-serif";
    ctx.textAlign = "center";
    ctx.textBaseline = "middle";
    for (const [az, label] of [[0, "N"], [90, "E"], [180, "S"], [270, "O"]]) {
      const p = toScreen(horizontalToVector(az, 0));
      if (p.visible) ctx.fillText(label, p.x, p.y);
    }

    const project = (raDeg, decDeg) => {
      const { azimuth, altitude } = equatorialToHorizontal(raDeg, decDeg, this.observer, date);
      return toScreen(horizontalToVector(azimuth, altitude));
    };

    // Constellation lines.
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

    // Stars.
    ctx.fillStyle = "#fff";
    for (const s of this.catalog.stars) {
      const p = project(s.ra, s.dec);
      if (!p.visible) continue;
      const r = magnitudeToRadius(s.mag) * Math.sqrt(this.zoom);
      ctx.beginPath();
      ctx.arc(p.x, p.y, r, 0, 2 * Math.PI);
      ctx.fill();
    }

    // Messier objects: small open circles, distinct from stars.
    ctx.strokeStyle = "rgba(129,199,132,0.85)";
    ctx.lineWidth = 1;
    for (const m of this.catalog.messier) {
      const p = project(m.ra, m.dec);
      if (!p.visible) continue;
      ctx.beginPath();
      ctx.arc(p.x, p.y, 3.5, 0, 2 * Math.PI);
      ctx.stroke();
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
