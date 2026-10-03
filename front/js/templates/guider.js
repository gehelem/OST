// templates/guider.js — bespoke floating-panel arrangement for the
// "guider" module, adapted from ostnoctua's templates/guider.js (see that
// file's own header) onto core/'s client API and UI3's generic property
// renderer instead of hand-written graph code.
//
// The generic renderer's own graph rendering (render.js's renderGridSection
// -> js/chart.js) stays dependency-free and generic (any graphType), but
// structure.en.md itself warns PHD "needs a dedicated implementation", and
// this module's own drift/guiding graphs are exactly that case — a real
// scatter with reference circles, a real PHD2-style multi-axis chart with
// corrections as bars. Ostnoctua already built exactly this (Chart.js,
// vendored — see lib/chart.min.js) — this ports that same configuration
// rather than reinventing it (Gilles, 2026-09-27: "tu as deja fait
// strictement la meme chose dans ostnoctua"). "actions" stays on the
// generic renderActionsPanel() path; "values" got its own bespoke
// treatment too on 2026-10-03 (RMS boxes + pulse bars, see
// buildValuesPanel() below) after Gilles found the plain generic table
// "pas assez graphique" — still "a bespoke template only ever adds
// presentation", just more of this module ended up needing it than
// first assumed.
//
// `image` is the other ostnoctua-ported piece (Gilles, 2026-09-29:
// "affiche l'image derriere en ecran de fond, comme dans ostnoctua"): the
// guide frame fills the shared #bg-layer (js/imagelayer.js) instead of
// showing as a floating panel's own thumbnail — showing the same image
// twice (full background + a small embedded copy) would be exactly the
// kind of redundant duplication Gilles has already flagged elsewhere this
// project ("tu as laisse les libelles des boutons !!! c'est ridicule"). The
// whole thing (background + quick-stats line + the fuller stats/histogram
// panel, one addImageBlock() call) lives in imagelayer.js (Gilles,
// 2026-09-29, having asked "est-ce que ce truc d'image sera reutilisable
// sur d'autres templates ?": "Go") — reused (minus its own quick-stats
// panel/button here, see the opts.quickPanel:false call below — Gilles,
// 2026-10-03: "supprime le bouton 'image' et la fenetre associee") by
// templates/scaffold.js (inspector/polar/parkmanager/sequencer) too, so
// this file just wires it to guider's own "image" property/element.
//
// Real property names confirmed against src/modules/guider/guider.json /
// guider.cpp: actions, values, calibrationvalues, drift, guiding,
// calParams, guideParams, revCorrections, disCorrections, backlash — plus
// `image`, defined generically by IndiModule::defineMeAsImager() (see
// libs/indimodule.cpp) for any imaging module, guider included.

import { updateQuickNav, moduleLifecycle, renderActionsPanel } from "../render.js";
import { addImageBlock } from "../imagelayer.js";
import { el, statusDot } from "../dom.js";
import * as Panels from "../panels.js";
import { t } from "../i18n.js";

if (window.Chart && window.Chart.registerables) {
  try { window.Chart.register(...window.Chart.registerables); } catch (e) { /* already done */ }
}

const num = (v) => (v == null ? NaN : Number(v));

// ---------------------------------------------------------------- guiding
// PHD2-style: RA/DE drift + RMS as lines (left axis "yDrive"), the RA/DE
// correction pulses as bars ("colonnes", right axis "yPulse"), SNR as its
// own line (right axis "ySNR") — Gilles, 2026-09-27: "phd2 pour le
// guidage, avec les corrections en colonnes, le RMS et le SNR". Field
// names are resolved through the property's own graphParams (role -> real
// element name) rather than hardcoded, per structure.en.md's PHD role
// table — guider.json happens to map most roles to their own name, but
// this stays correct if that ever changes.
function guidingFields(prop) {
  const gp = (prop && prop.graphParams) || {};
  return {
    D: gp.D || "time", RA: gp.RA || "RA", DE: gp.DE || "DE",
    pRA: gp.pRA || "pRA", pDE: gp.pDE || "pDE",
    RMS: gp.RMS || "RMS", SNR: gp.RSB || "SNR",
  };
}

function alignedYLimits(rows, f) {
  const driveVals = rows.flatMap((r) => [r[f.DE], r[f.RA], r[f.RMS]]).map(num).filter(isFinite);
  const pulseVals = rows.flatMap((r) => [r[f.pDE], r[f.pRA]]).map(num).filter(isFinite);

  let dMax = Math.max(0, ...driveVals), dMin = Math.min(0, ...driveVals);
  const dPad = (dMax - dMin) * 0.1 || 0.5;
  dMax += dPad; dMin -= dPad;
  const alpha = dMax / (dMax - dMin);

  let pMax = Math.max(0, ...pulseVals), pMin = Math.min(0, ...pulseVals);
  const pPad = (pMax - pMin) * 0.1 || 1;
  pMax += pPad; pMin -= pPad;
  const pMaxAxis = Math.max(pMax, pMin * alpha / (alpha - 1));
  const pMinAxis = pMaxAxis * (alpha - 1) / alpha;

  return { drive: { min: dMin, max: dMax }, pulse: { min: pMinAxis, max: pMaxAxis } };
}

function guidingChartData(prop) {
  const rows = prop.rows || [];
  const f = guidingFields(prop);
  if (!rows.length) return { labels: [], datasets: [] };
  const labels = rows.map((r) => {
    const d = new Date(r[f.D]);
    return isNaN(d) ? "" : d.toLocaleTimeString(undefined, { hour: "2-digit", minute: "2-digit", second: "2-digit", hour12: false });
  });
  const col = (name) => rows.map((r) => num(r[name]));
  const line = (label, data, color, axis, w) => ({
    label, data, borderColor: color, backgroundColor: color, borderWidth: w || 2,
    tension: 0, yAxisID: axis, pointRadius: 2, pointHoverRadius: 4,
  });
  const bar = (label, data, color) => ({
    label, data, borderColor: color, backgroundColor: color, borderWidth: 1,
    yAxisID: "yPulse", type: "bar", borderRadius: 3, barThickness: 6,
  });
  const lbl = (name, dflt) => (prop.elements[name] && prop.elements[name].label) || dflt;
  return {
    labels,
    datasets: [
      line(lbl(f.RA, "Drift RA"), col(f.RA), "#4CAF50", "yDrive"),
      line(lbl(f.DE, "Drift DE"), col(f.DE), "#2196F3", "yDrive"),
      line(lbl(f.RMS, "RMS"), col(f.RMS), "#FFC107", "yDrive", 2.5),
      bar(lbl(f.pRA, "Pulse RA"), col(f.pRA), "rgba(76,175,80,0.25)"),
      bar(lbl(f.pDE, "Pulse DE"), col(f.pDE), "rgba(33,150,243,0.25)"),
      line(lbl(f.SNR, "SNR"), col(f.SNR), "#F44336", "ySNR", 2.5),
    ],
  };
}

/** Builds the guiding chart panel's node, self-subscribed to the "guiding"
 * property (fine-grained topic — grid updates arrive as gu/gc/gr traffic
 * on this topic, never on topics.module(), so this must not depend on the
 * template's own per-module rebuild to stay live). Returns a node with
 * `_dispose` set, ready for the caller's track(). */
function buildGuidingChartPanel(client, moduleName) {
  const wrap = el("div", { class: "gd-chart" });
  const canvas = el("canvas");
  wrap.appendChild(canvas);
  let chart = null;
  const grey = "rgba(255,255,255,0.55)";
  const grid = { color: "rgba(255,255,255,0.08)" };

  const unsub = client.subscribe(client.topics.property(moduleName, "guiding"), (prop) => {
    if (!prop) return;
    const rows = prop.rows || [];
    const f = guidingFields(prop);
    if (!chart) {
      const lim = rows.length ? alignedYLimits(rows, f) : null;
      chart = new window.Chart(canvas.getContext("2d"), {
        type: "line",
        data: guidingChartData(prop),
        options: {
          responsive: true, maintainAspectRatio: false, animation: false,
          interaction: { mode: "index", intersect: false },
          plugins: {
            legend: { position: "bottom", labels: { color: grey, boxWidth: 14, boxHeight: 10, font: { size: 10 } } },
            tooltip: { mode: "index", intersect: false },
          },
          scales: {
            x: { ticks: { color: grey, maxRotation: 0, autoSkip: true, maxTicksLimit: 10, font: { size: 9 } }, grid },
            yDrive: {
              type: "linear", position: "left", min: lim && lim.drive.min, max: lim && lim.drive.max,
              title: { display: true, text: t("guider.drift"), color: grey, font: { size: 10 } },
              ticks: { color: grey, font: { size: 9 }, maxTicksLimit: 8 }, grid,
            },
            yPulse: {
              type: "linear", position: "right", min: lim && lim.pulse.min, max: lim && lim.pulse.max,
              title: { display: true, text: t("guider.pulse"), color: grey, font: { size: 10 } },
              ticks: { color: grey, font: { size: 9 }, maxTicksLimit: 6 }, grid: { drawOnChartArea: false },
            },
            ySNR: {
              type: "linear", position: "right", offset: true, min: 0,
              title: { display: true, text: t("guider.snr"), color: "#F44336", font: { size: 10 } },
              ticks: { color: "#F44336", font: { size: 9 }, maxTicksLimit: 5 }, grid: { drawOnChartArea: false },
            },
          },
        },
      });
      return;
    }
    const d = guidingChartData(prop);
    chart.data.labels = d.labels;
    d.datasets.forEach((ds, i) => { if (chart.data.datasets[i]) chart.data.datasets[i].data = ds.data; });
    if (rows.length) {
      const lim = alignedYLimits(rows, f);
      chart.options.scales.yDrive.min = lim.drive.min;
      chart.options.scales.yDrive.max = lim.drive.max;
      chart.options.scales.yPulse.min = lim.pulse.min;
      chart.options.scales.yPulse.max = lim.pulse.max;
    }
    chart.update("none");
  });

  wrap._dispose = () => { unsub(); if (chart) { chart.destroy(); chart = null; } };
  return wrap;
}

// ------------------------------------------------------------------ drift
// Scatter (RA vs DEC), auto-scaled to the data's own extent, with dashed
// reference circles at 1"/2"/5" (Gilles, 2026-09-27: "drift en scatter
// avec echelles automatiques en cercles").
const refCircles = {
  id: "refCircles",
  afterDatasetsDraw(chart) {
    const { ctx, scales: { x, y } } = chart;
    if (!x || !y) return;
    const cx = x.getPixelForValue(0), cy = y.getPixelForValue(0);
    [[0.5, "rgba(0,255,0,0.35)", '1"'], [1, "rgba(255,165,0,0.35)", '2"'], [2.5, "rgba(255,0,0,0.35)", '5"']]
      .forEach(([rad, color, label]) => {
        const rpx = Math.abs(x.getPixelForValue(rad) - x.getPixelForValue(0));
        ctx.save();
        ctx.strokeStyle = color; ctx.lineWidth = 1.5; ctx.setLineDash([5, 5]);
        ctx.beginPath(); ctx.arc(cx, cy, rpx, 0, 2 * Math.PI); ctx.stroke();
        ctx.fillStyle = color.replace("0.35", "0.9"); ctx.font = "11px sans-serif"; ctx.textAlign = "center";
        ctx.fillText(label, cx + rpx, cy - 4);
        ctx.restore();
      });
  },
};

function driftPoints(prop) {
  const rows = prop.rows || [];
  const gp = (prop && prop.graphParams) || { X: "RA", Y: "DEC" };
  return rows.map((r) => ({ x: num(r[gp.X]), y: num(r[gp.Y]) })).filter((pt) => isFinite(pt.x) && isFinite(pt.y));
}
function driftLimit(points) {
  let m = 0;
  for (const pt of points) m = Math.max(m, Math.abs(pt.x), Math.abs(pt.y));
  return Math.max(1, Math.ceil(m * 1.2 * 10) / 10);
}

function buildDriftChartPanel(client, moduleName) {
  const wrap = el("div", { class: "gd-chart gd-chart-square" });
  const canvas = el("canvas");
  wrap.appendChild(canvas);
  let chart = null;
  const grey = "rgba(255,255,255,0.55)";
  const grid = { color: "rgba(255,255,255,0.08)" };

  const unsub = client.subscribe(client.topics.property(moduleName, "drift"), (prop) => {
    if (!prop) return;
    const pts = driftPoints(prop);
    const L = driftLimit(pts);
    const gp = prop.graphParams || {};
    if (!chart) {
      chart = new window.Chart(canvas.getContext("2d"), {
        type: "scatter",
        data: { datasets: [{ label: "drift", data: pts, backgroundColor: "rgba(33,150,243,0.7)", pointRadius: 4 }] },
        options: {
          responsive: true, maintainAspectRatio: true, aspectRatio: 1, animation: false,
          plugins: { legend: { display: false } },
          scales: {
            x: { type: "linear", min: -L, max: L, title: { display: true, text: (prop.elements[gp.X] && prop.elements[gp.X].label) || gp.X || "RA", color: grey, font: { size: 10 } }, ticks: { color: grey, font: { size: 9 } }, grid },
            y: { min: -L, max: L, title: { display: true, text: (prop.elements[gp.Y] && prop.elements[gp.Y].label) || gp.Y || "DEC", color: grey, font: { size: 10 } }, ticks: { color: grey, font: { size: 9 } }, grid },
          },
        },
        plugins: [refCircles],
      });
      return;
    }
    chart.data.datasets[0].data = pts;
    chart.options.scales.x.min = chart.options.scales.y.min = -L;
    chart.options.scales.x.max = chart.options.scales.y.max = L;
    chart.update("none");
  });

  wrap._dispose = () => { unsub(); if (chart) { chart.destroy(); chart = null; } };
  return wrap;
}

// --------------------------------------------------------------- values
// Bespoke "values" panel (Gilles, 2026-10-03, on the plain generic table
// it used to be: "rends-la plus graphique") - matches OstErix/Rooster's
// own RMS presentation (guider.component.html's own "values-card": a
// bordered, green-accented box per RMS figure instead of a label/value
// table row - source read directly, not guessed) and adds something
// Rooster doesn't have, Gilles's own idea: a small centered bar per axis
// showing the current correction pulse, "0 au milieu, min a gauche (Est)
// et max a droite (Ouest)" for RA - DEC mirrors that (South left/North
// right - Gilles only specified RA explicitly, "idem" for DEC; flag it
// if that pairing turns out backwards too, same iterate-on-feedback spirit
// as the FOV frame fix). Scaled against guideParams.pulsemax (hasprofile,
// subscribed here JUST for that number - guideParams itself still only
// reachable from the Parameters modal, same as any other hasprofile
// property this template doesn't give its own panel).
// One line per axis now (Gilles, 2026-10-03: "fais tenir les curseurs et
// leurs libelles sur deux lignes en tout" - RA and DEC together, not two
// each): axis label, E/S, the bar itself (its current value shown INSIDE
// it, not as separate text below - "affiche la valeur a l'interieur du
// curseur"), W/N.
function pulseAxisRow(axisLabel, leftVal, rightVal, leftName, rightName, maxVal) {
  const row = el("div", { class: "pulse-axis" });
  row.appendChild(el("div", { class: "pulse-axis-label", text: axisLabel }));
  row.appendChild(el("div", { class: "pulse-dir-label", text: leftName }));
  const bar = el("div", { class: "pulse-bar" });
  const fill = el("div", { class: "pulse-bar-fill" });
  // Exactly one of leftVal/rightVal is ever nonzero at once (guider.cpp's
  // own pulse logic always zeroes the opposite direction) - Math.max()
  // picks whichever that is, 0 when neither is active.
  const value = Math.max(leftVal || 0, rightVal || 0);
  bar.appendChild(fill);
  bar.appendChild(el("div", { class: "pulse-bar-center" }));
  bar.appendChild(el("div", { class: "pulse-bar-value mono", text: String(Math.round(value)) }));
  row.appendChild(bar);
  row.appendChild(el("div", { class: "pulse-dir-label", text: rightName }));
  const signed = (rightVal || 0) - (leftVal || 0);
  const frac = Math.max(-1, Math.min(1, maxVal ? signed / maxVal : 0));
  if (frac >= 0) { fill.style.left = "50%"; fill.style.width = (frac * 50) + "%"; }
  else { fill.style.left = (50 + frac * 50) + "%"; fill.style.width = (-frac * 50) + "%"; }
  return row;
}

function buildValuesPanel(client, moduleName) {
  const wrap = el("div", { class: "guider-values" });
  let pulsemax = 2000; // guideParams.json's own default, until the real dump arrives
  let valuesProp = null;
  let calQualEl = null;

  function render() {
    wrap.innerHTML = "";
    if (!valuesProp) return;
    const e = valuesProp.elements;
    // Labels come from the server (already translated, structure.en.md -
    // "property/element labels ... DO come translated already") - never
    // re-invented locally, per js/i18n.js's own header rule.
    const rmsRow = el("div", { class: "value-row" });
    for (const key of ["rmsRA", "rmsDEC", "rmsTotal"]) {
      if (!e[key]) continue;
      rmsRow.appendChild(el("div", { class: "value-box" }, [
        el("div", { class: "value-box-label", text: e[key].label || key }),
        el("div", { class: "value-box-number", text: (e[key].value || 0).toFixed(2) }),
      ]));
    }
    wrap.appendChild(rmsRow);
    if (e.pulseE && e.pulseW) wrap.appendChild(pulseAxisRow("RA", e.pulseE.value, e.pulseW.value, "E", "W", pulsemax));
    if (e.pulseS && e.pulseN) wrap.appendChild(pulseAxisRow("DEC", e.pulseS.value, e.pulseN.value, "S", "N", pulsemax));
    // Just the quality dot, not the rest of "calibrationvalues" (the
    // actual calibration numbers are already reachable from the generic
    // Parameters modal) - Gilles, 2026-10-03: "pas besoin de ce niveau de
    // detail a cet endroit ... laisse juste le calibration quality en
    // petit sous les pulses".
    if (calQualEl) {
      wrap.appendChild(el("div", { class: "cal-quality-row" }, [
        statusDot(calQualEl.value),
        el("span", { class: "muted small", text: calQualEl.label || "Calibration quality" }),
      ]));
    }
  }

  const unsubParams = client.subscribe(client.topics.property(moduleName, "guideParams"), (prop) => {
    const v = prop && prop.elements.pulsemax && prop.elements.pulsemax.value;
    if (typeof v === "number" && v > 0) { pulsemax = v; render(); }
  });
  const unsub = client.subscribe(client.topics.property(moduleName, "values"), (prop) => { valuesProp = prop; render(); });
  const unsubCal = client.subscribe(client.topics.property(moduleName, "calibrationvalues"), (prop) => {
    calQualEl = prop && prop.elements.calqual;
    render();
  });
  wrap._dispose = () => { unsub(); unsubParams(); unsubCal(); };
  return wrap;
}

export function mount(client, moduleName) {
  Panels.setScope(moduleName);
  Panels.clear();

  let disposers = [];
  function track(node) { disposers.push(node._dispose); return node; }
  function dispose() { while (disposers.length) disposers.pop()(); }

  function build(mod) {
    dispose();
    Panels.clear();
    const has = (name) => name in mod.properties;
    // Curated quicknav list — Gilles, 2026-09-27: "actions/image/devices/
    // control et peut-etre d'autres plus tard, qu'on devra pouvoir adapter
    // selon le template". "devices"/"control" don't exist as their own
    // panels yet (guider.json has no such property) - using what's
    // actually built today (actions/image/guiding/drift); add devices/
    // control here once those panels are actually created.
    const quickNavEntries = [];

    // Profile load/save lives in the dedicated "Parameters" box below
    // (buildParamsPanel, shared with the generic renderer) — not here. No
    // "header" panel either any more (render.js's own renderModuleView
    // doc comment: Gilles, 2026-10-03, "la supprimer completement").

    renderActionsPanel(client, moduleName, mod, track, quickNavEntries);
    if (has("values")) {
      const b = Panels.make({ id: "values", title: mod.properties.values.label || "Session values", x: 20, y: 470, w: 300 });
      b.appendChild(track(buildValuesPanel(client, moduleName)));
      quickNavEntries.push({ id: "values", label: mod.properties.values.label || "Session values" });
    }
    // No small "Image" quick-stats panel/button here (Gilles, 2026-10-03:
    // "supprime le bouton 'image' et la fenetre associee") - the
    // background already shows the picture full-screen, and the "Stats"
    // panel/button (below) already covers the same figures in more depth.
    addImageBlock(client, moduleName, mod, "image", "image", Panels, track, disposers, quickNavEntries, { x: 350, y: 92, w: 260 }, { quickPanel: false });
    if (has("guiding")) {
      const b = Panels.make({ id: "guiding", title: mod.properties.guiding.label || "Guiding", x: 350, y: 300, w: 620 });
      b.appendChild(track(buildGuidingChartPanel(client, moduleName)));
      quickNavEntries.push({ id: "guiding", label: mod.properties.guiding.label || "Guiding" });
    }
    if (has("drift")) {
      const b = Panels.make({ id: "drift", title: mod.properties.drift.label || "Drift", x: 990, y: 92, w: 340 });
      b.appendChild(track(buildDriftChartPanel(client, moduleName)));
      quickNavEntries.push({ id: "drift", label: mod.properties.drift.label || "Drift" });
    }
    // The profile-centric "Parameters" box (buildParamsPanel, render.js) is
    // not built here — it opens on demand from the gear button (app.js),
    // same as the generic renderer. Its properties (hasprofile:true, from
    // the module's own metadata) cover every one of the real guider
    // module's remaining properties (calParams/guideParams/
    // revCorrections/disCorrections/backlash, confirmed against
    // src/modules/guider/guider.json) — no "Other" catch-all here, unlike
    // every scaffolded template: there's nothing left to fall into one
    // (Gilles, 2026-10-03, doing some cleanup: "enleve le 'other' et la
    // fenetre" after confirming that).
    updateQuickNav(quickNavEntries);
  }

  return moduleLifecycle(client, moduleName, build, dispose);
}
