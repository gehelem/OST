// templates/polar.js — scaffolded template for the "polar" module (see
// scaffold.js's own header). Real property names confirmed against
// src/modules/polar/polar.json / libs/indimodule.cpp's defineMeAsImager()
// (called directly by polar.cpp, no giveMeATarget()-style extras beyond
// that): values (ra0/de0/t0 + ra1/de1/t1 + ra2/de2/t2, the 3 measurement
// points), errors (erraz/erralt/errtot + their arcmin counterparts),
// states (idle/moving/shooting/solving/compute) — plus the generic
// imager's own parameters (hasprofile:true, Parameters modal only) and
// image, both generic to any IndiModule via defineMeAsImager().
//
// Rooster's own polar.component.html hand-formats states/errors as light
// dots and split degree/arcmin values rather than generic property
// panels, and never surfaces "values" at all — not reproduced here (no
// onlyElements-style partial rendering in ostcore yet, see inspector.js's
// own note on the same tradeoff); "values" just falls into the generic
// catch-all "Other" panel scaffold.js already provides for anything a
// template doesn't explicitly list.

import { buildScaffoldTemplate } from "./scaffold.js";
import { el, statusDot } from "../dom.js";

// Small canvas arrow, tip at (x1,y1) - shared by the azimuth/altitude
// arrows below, same simple "line + triangular head" look used elsewhere
// in this file's own module family (see sky-engine.js's own mount-frame
// marker for the same construction). A zero-length arrow (no error at
// all) just draws nothing.
function drawArrow(ctx, x0, y0, x1, y1, color) {
  if (x0 === x1 && y0 === y1) return;
  const angle = Math.atan2(y1 - y0, x1 - x0);
  const headLen = 7;
  ctx.strokeStyle = color;
  ctx.fillStyle = color;
  ctx.lineWidth = 2;
  ctx.beginPath(); ctx.moveTo(x0, y0); ctx.lineTo(x1, y1); ctx.stroke();
  ctx.beginPath();
  ctx.moveTo(x1, y1);
  ctx.lineTo(x1 - headLen * Math.cos(angle - Math.PI / 6), y1 - headLen * Math.sin(angle - Math.PI / 6));
  ctx.lineTo(x1 - headLen * Math.cos(angle + Math.PI / 6), y1 - headLen * Math.sin(angle + Math.PI / 6));
  ctx.closePath();
  ctx.fill();
}

// Below 0.5' a polar alignment is good, past 10' it's bad enough to be
// worth fixing before imaging - same three-band read the status dots
// below use, on the arcmin figure (the finer-grained of the two units
// "errors" carries, same value as the degree one, just rescaled).
// Returns a statusDot()-style index (dom.js: 1 ok/green, 2 busy/amber, 3
// error/red).
function errorStatus(arcmin) {
  const a = Math.abs(arcmin);
  if (a < 0.5) return 1;
  if (a <= 10) return 2;
  return 3;
}

// Server labels end in a bare "°" (structure.en.md: labels come already
// translated) - dropped here since the degree VALUE already carries its
// own ° right next to it; keeping the label's own trailing one too would
// just print it twice.
function stripDegSuffix(label, fallback) {
  return label ? label.replace(/\s*°\s*$/, "") : fallback;
}

// Graphical read of "errors" (Polar error) - a horizontal arrow for
// erraz (azimuth) and a vertical one for erralt (altitude), both scaled
// together off whichever of the two is currently larger, so the two
// arrows' relative lengths always reflect their relative magnitudes
// regardless of how big either one actually is. Sign sets direction:
// positive azimuth error points right, positive altitude error points
// up (screen-space y grows downward, hence the unary minus). The actual
// figures go in three status-dot rows below the canvas (dom.js's own
// statusDot(), same look as every other status line in this app) rather
// than printed on top of the arrows themselves (Gilles, 2026-10-22:
// illegible there).
function buildPolarErrorArrows(client, moduleName) {
  const w = 160, h = 160;
  const canvas = el("canvas", { class: "polar-error-canvas", width: String(w), height: String(h) });
  const ctx = canvas.getContext("2d");
  const cx = w / 2, cy = h / 2;
  const maxLen = Math.min(w, h) / 2 - 16;
  const rows = el("div");
  const fmtSigned = (deg) => `${deg >= 0 ? "+" : ""}${deg.toFixed(2)}°`;
  const unsub = client.subscribe(client.topics.property(moduleName, "errors"), (prop) => {
    ctx.clearRect(0, 0, w, h);
    rows.innerHTML = "";
    if (!prop) return;
    const e = prop.elements;
    const az = (e.erraz && e.erraz.value) || 0;
    const alt = (e.erralt && e.erralt.value) || 0;
    const tot = (e.errtot && e.errtot.value) || 0;
    const azmn = (e.errazmn && e.errazmn.value) || 0;
    const altmn = (e.erraltmn && e.erraltmn.value) || 0;
    const totmn = (e.errtotmn && e.errtotmn.value) || 0;
    const scale = maxLen / Math.max(Math.abs(az), Math.abs(alt), 1e-6);

    ctx.strokeStyle = "rgba(255,255,255,0.2)";
    ctx.lineWidth = 1;
    ctx.beginPath(); ctx.moveTo(cx - maxLen, cy); ctx.lineTo(cx + maxLen, cy); ctx.stroke();
    ctx.beginPath(); ctx.moveTo(cx, cy - maxLen); ctx.lineTo(cx, cy + maxLen); ctx.stroke();

    drawArrow(ctx, cx, cy, cx + az * scale, cy, "#4fc3f7");
    drawArrow(ctx, cx, cy, cx, cy - alt * scale, "#ffb74d");

    rows.appendChild(el("div", { class: "status-row" }, [
      statusDot(errorStatus(altmn)),
      el("span", { class: "muted small", text: `${stripDegSuffix(e.erralt && e.erralt.label, "Altitude error")} ${fmtSigned(alt)} ${altmn.toFixed(2)}'` }),
    ]));
    rows.appendChild(el("div", { class: "status-row" }, [
      statusDot(errorStatus(azmn)),
      el("span", { class: "muted small", text: `${stripDegSuffix(e.erraz && e.erraz.label, "Azimuth error")} ${fmtSigned(az)} ${azmn.toFixed(2)}'` }),
    ]));
    rows.appendChild(el("div", { class: "status-row" }, [
      statusDot(errorStatus(totmn)),
      el("span", { class: "muted small", text: `${stripDegSuffix(e.errtot && e.errtot.label, "Total error")} ${tot.toFixed(2)}° ${totmn.toFixed(2)}'` }),
    ]));
  });
  const wrap = el("div");
  wrap.append(canvas, rows);
  wrap._dispose = unsub;
  return wrap;
}

export const mount = buildScaffoldTemplate([
  // Folded into the Actions panel's own optional slot instead of its own
  // standalone window (Gilles, 2026-10-22) - same move as inspector's
  // Analysis selection/planner's progress.
  { id: "states", foldIntoActions: true },
  // Below "states" above, in the same optional slot (Gilles, 2026-10-22) -
  // replaces "errors"'s own former standalone panel entirely, now
  // redundant with this graphical+numeric read of the same property.
  { id: "polarErrorArrows", foldIntoActions: true, customProps: ["errors"], custom: buildPolarErrorArrows },
]);
