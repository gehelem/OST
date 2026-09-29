// sky-engine.js — the sky-dome view: canvas render + wheel-zoom. New,
// clean implementation (not ported from skymap) — see sky-projection.js's
// own header for why, and notes/NOTES-frontend-ostcore-planetarium.md for
// the full chantier context.
//
// v1 scope, deliberately: a fixed, zenith-centered dome (no drag/pan yet -
// that's its own follow-up step, on purpose, to keep the riskiest part
// isolated and testable on its own rather than bolted on here) + wheel-
// zoom + stars/constellations/Messier/Sun/Moon/planets, all computed live
// via astronomy-engine for whatever instant `getTime()` returns (never
// hardcoded to "now" — see the note's "barre de visibilite" requirement:
// this engine already supports an arbitrary displayed time, the UI to
// scrub it is a separate follow-up).

import { Body, Horizon, Equator } from "../lib/astronomy-engine.js";
import { makeObserver, equatorialToHorizontal, horizontalToScreen, magnitudeToRadius } from "./sky-projection.js";
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
    this.onNeedsRender = opts.onNeedsRender || null; // called after wheel-zoom changes, so the caller can re-render immediately instead of waiting for its own timer
    this.catalog = { stars: [], constellationLines: [], messier: [] };
    this._onWheel = this._onWheel.bind(this);
    canvas.addEventListener("wheel", this._onWheel, { passive: false });
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
  }

  render() {
    const { ctx, canvas } = this;
    const w = canvas.width, h = canvas.height;
    ctx.clearRect(0, 0, w, h);
    const cx = w / 2, cy = h / 2;
    const domeRadius = Math.min(w, h) / 2 * 0.96 * this.zoom;
    const date = this.getTime();

    // Dome edge (horizon circle) + cardinal points.
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
      const p = horizontalToScreen(az, 0, domeRadius * 1.035);
      ctx.fillText(label, cx + p.x, cy + p.y);
    }

    const project = (raDeg, decDeg) => {
      const { azimuth, altitude } = equatorialToHorizontal(raDeg, decDeg, this.observer, date);
      const p = horizontalToScreen(azimuth, altitude, domeRadius);
      return { x: cx + p.x, y: cy + p.y, visible: p.visible };
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
    // (structure.en.md-equivalent reasoning: a fixed file would drift the
    // instant the displayed time isn't "now" - see the note's "barre de
    // visibilite" requirement, this is exactly why it matters here already).
    ctx.textAlign = "left";
    for (const p of PLANETS) {
      const equ = Equator(p.body, date, this.observer, true, true);
      const h = Horizon(date, this.observer, equ.ra, equ.dec, "normal");
      const scr = horizontalToScreen(h.azimuth, h.altitude, domeRadius);
      if (!scr.visible) continue;
      const x = cx + scr.x, y = cy + scr.y;
      ctx.fillStyle = p.color;
      ctx.beginPath();
      ctx.arc(x, y, p.radius, 0, 2 * Math.PI);
      ctx.fill();
      ctx.fillText(p.label, x + p.radius + 3, y);
    }
  }
}
