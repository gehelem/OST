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
// strictement la meme chose dans ostnoctua"). Everything else (actions,
// values, catch-all) stays on the generic renderProperty() path — still
// "a bespoke template only ever adds presentation" for those.
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
// sur d'autres templates ?": "Go") — reused as-is by templates/scaffold.js
// (focus/sequencer) too, so this file just wires it to guider's own
// "image" property/element.
//
// Real property names confirmed against src/modules/guider/guider.json /
// guider.cpp: actions, values, calibrationvalues, drift, guiding,
// calParams, guideParams, revCorrections, disCorrections, backlash — plus
// `image`, defined generically by IndiModule::defineMeAsImager() (see
// libs/indimodule.cpp) for any imaging module, guider included.

import { renderProperty, updateQuickNav, moduleLifecycle, renderActionsPanel } from "../render.js";
import { addImageBlock } from "../imagelayer.js";
import { el } from "../dom.js";
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
      b.appendChild(track(renderProperty(client, moduleName, "values")));
      if (has("calibrationvalues")) b.appendChild(track(renderProperty(client, moduleName, "calibrationvalues")));
      quickNavEntries.push({ id: "values", label: mod.properties.values.label || "Session values" });
    }
    addImageBlock(client, moduleName, mod, "image", "image", Panels, track, disposers, quickNavEntries, { x: 350, y: 92, w: 260 });
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
