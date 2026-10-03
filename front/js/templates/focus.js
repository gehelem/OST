// templates/focus.js — bespoke floating-panel arrangement for the "focus"
// module. Started as a scaffolded template (buildScaffoldTemplate) but the
// "values" panel (the V-curve) needed a real chart — Gilles, 2026-10-03,
// on the plain generic table it used to render as: "la 'focus curve' est
// trop minimale, regarde le graphique de rooster, avec la courbe de
// regression et les barres d'erreur". results/progress/zones stay on the
// generic renderProperty() path (same as before, nothing wrong with those
// as plain tables) — same split guider.js already settled on (bespoke only
// where it actually earns its keep).
//
// Real property names confirmed against src/modules/focus/focus.json:
// values (V-curve data — hasGrid, graphType XY, graphParams focpos/
// loopHFRavg, plus imgHFRdev for error bars and fitk0/fitk1/fitk2 for the
// polynomial fit — see buildFocusCurvePanel below), results (hfr/pos
// readout), progress (spinner), zones (zone/bestpos grid) — plus
// parameters (hasprofile:true, Parameters modal only, not a panel),
// actions (abortfocus/autofocus/loop) and image, both generic to any
// IndiModule via defineMeAsFocuser()/defineMeAsImager().
//
// The error-bars + polynomial-fit chart itself ports OstErix/Rooster's own
// focus.component.ts updateFocusChart() (source read directly, not
// guessed): dataPoints {x,y,yMin,yMax} from focpos/loopHFRavg/imgHFRdev,
// a 60-point quadratic curve from the last row with a nonzero fitk2, one
// 'lineWithErrorBars' dataset (HFR) + one 'line' dataset (dashed fit) via
// the chartjs-chart-error-bars plugin — same version Rooster itself pins
// (3.10.0, see lib/chart-error-bars.min.js's own header), vendored
// alongside the already-vendored chart.min.js (Chart.js 3.9.1). Colors
// are ostcore's own dark-theme convention (grey axis/grid, like guider.js's
// guidingChartData) rather than Rooster's own light-theme palette.

import { renderProperty, updateQuickNav, moduleLifecycle, renderActionsPanel } from "../render.js";
import { addImageBlock } from "../imagelayer.js";
import { el } from "../dom.js";
import * as Panels from "../panels.js";

if (window.Chart && window.Chart.registerables) {
  try { window.Chart.register(...window.Chart.registerables); } catch (e) { /* already done */ }
}
// The error-bars controller/point element aren't part of Chart.js's own
// registerables — a separate plugin (lib/chart-error-bars.min.js), must be
// registered on its own.
if (window.Chart && window.ChartErrorBars) {
  try { window.Chart.register(window.ChartErrorBars.LineWithErrorBarsController, window.ChartErrorBars.PointWithErrorBar); } catch (e) { /* already done */ }
}

const num = (v) => (v == null || v === "" ? NaN : Number(v));

// ------------------------------------------------------------- focus curve
/** Builds the {points, curve} data Chart.js needs from the "values" grid's
 * rows (already normalized to {name: value} objects by the datastore — see
 * core/datastore.js's _rowsFromArrayGrid — unlike Rooster's own raw
 * grid/gridheaders arrays, so no header-index lookup is needed here). */
function focusCurveData(prop) {
  const rows = prop.rows || [];
  const points = [];
  let k0 = 0, k1 = 0, k2 = 0, fitAvailable = false;
  for (const r of rows) {
    const focpos = num(r.focpos);
    const hfr = num(r.loopHFRavg);
    if (isFinite(focpos) && isFinite(hfr)) {
      const dev = num(r.imgHFRdev);
      const d = isFinite(dev) ? dev : 0;
      points.push({ x: focpos, y: hfr, yMin: hfr - d, yMax: hfr + d });
    }
    // Last row carrying a nonzero fitk2 wins — the backend only fills the
    // fit coefficients in once enough points exist, same logic Rooster's
    // own updateFocusChart() uses (loop over every row, keep overwriting).
    const rk2 = num(r.fitk2);
    if (isFinite(rk2) && rk2 !== 0) {
      k0 = num(r.fitk0); k1 = num(r.fitk1); k2 = rk2; fitAvailable = true;
    }
  }
  points.sort((a, b) => a.x - b.x);

  const curve = [];
  if (fitAvailable && points.length >= 2) {
    const xMin = points[0].x, xMax = points[points.length - 1].x;
    const steps = 60;
    for (let i = 0; i <= steps; i++) {
      const x = xMin + (xMax - xMin) * i / steps;
      curve.push({ x, y: k0 + k1 * x + k2 * x * x });
    }
  }
  return { points, curve };
}

function focusCurveDatasets(prop) {
  const { points, curve } = focusCurveData(prop);
  const datasets = [{
    type: "lineWithErrorBars",
    label: "HFR",
    data: points,
    borderColor: "#4CAF50", backgroundColor: "rgba(76,175,80,0.1)",
    borderWidth: 2, pointRadius: 4, pointHoverRadius: 6, tension: 0.1,
    errorBarColor: "rgba(76,175,80,0.6)", errorBarWhiskerColor: "rgba(76,175,80,0.8)",
    errorBarLineWidth: 1.5, errorBarWhiskerLineWidth: 1.5, errorBarWhiskerSize: 8,
  }];
  if (curve.length) {
    datasets.push({
      type: "line",
      label: "Polynomial fit",
      data: curve,
      borderColor: "rgba(255,152,0,0.85)", backgroundColor: "transparent",
      borderWidth: 2, borderDash: [5, 4], pointRadius: 0, tension: 0,
    });
  }
  return datasets;
}

/** Builds the focus-curve chart panel's node, self-subscribed to the
 * "values" property (same fine-grained-topic pattern as guider.js's
 * buildGuidingChartPanel/buildDriftChartPanel — grid row updates arrive on
 * this topic, not on topics.module()). Returns a node with `_dispose` set,
 * ready for the caller's track(). */
function buildFocusCurvePanel(client, moduleName) {
  const wrap = el("div", { class: "gd-chart" });
  const canvas = el("canvas");
  wrap.appendChild(canvas);
  let chart = null;
  const grey = "rgba(255,255,255,0.55)";
  const grid = { color: "rgba(255,255,255,0.08)" };

  const unsub = client.subscribe(client.topics.property(moduleName, "values"), (prop) => {
    if (!prop) return;
    const datasets = focusCurveDatasets(prop);
    const xLabel = (prop.elements.focpos && prop.elements.focpos.label) || "Focuser position";
    const yLabel = (prop.elements.loopHFRavg && prop.elements.loopHFRavg.label) || "HFR";
    if (!chart) {
      chart = new window.Chart(canvas.getContext("2d"), {
        type: "lineWithErrorBars",
        data: { datasets },
        options: {
          responsive: true, maintainAspectRatio: false, animation: false,
          plugins: { legend: { display: false } },
          scales: {
            x: {
              type: "linear",
              title: { display: true, text: xLabel, color: grey, font: { size: 10 } },
              ticks: { color: grey, font: { size: 9 } }, grid,
            },
            y: {
              beginAtZero: false,
              title: { display: true, text: yLabel, color: grey, font: { size: 10 } },
              ticks: { color: grey, font: { size: 9 } }, grid,
            },
          },
        },
      });
      return;
    }
    chart.data.datasets = datasets;
    chart.options.scales.x.title.text = xLabel;
    chart.options.scales.y.title.text = yLabel;
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
    const quickNavEntries = [];

    // Profile load/save lives in the dedicated "Parameters" box (app.js's
    // gear button, buildParamsPanel in render.js) — not built here.

    renderActionsPanel(client, moduleName, mod, track, quickNavEntries);

    if (has("results")) {
      const b = Panels.make({ id: "results", title: mod.properties.results.label || "Final result", x: 20, y: 330, w: 300 });
      b.appendChild(track(renderProperty(client, moduleName, "results")));
      quickNavEntries.push({ id: "results", label: mod.properties.results.label || "Final result" });
    }
    if (has("progress")) {
      const b = Panels.make({ id: "progress", title: mod.properties.progress.label || "Progress", x: 20, y: 450, w: 300 });
      b.appendChild(track(renderProperty(client, moduleName, "progress")));
      quickNavEntries.push({ id: "progress", label: mod.properties.progress.label || "Progress" });
    }
    if (has("values")) {
      const b = Panels.make({ id: "values", title: mod.properties.values.label || "Focus curve", x: 350, y: 92, w: 480 });
      b.appendChild(track(buildFocusCurvePanel(client, moduleName)));
      quickNavEntries.push({ id: "values", label: mod.properties.values.label || "Focus curve" });
    }
    if (has("zones")) {
      const b = Panels.make({ id: "zones", title: mod.properties.zones.label || "Zoning results", x: 350, y: 450, w: 300 });
      b.appendChild(track(renderProperty(client, moduleName, "zones")));
      quickNavEntries.push({ id: "zones", label: mod.properties.zones.label || "Zoning results" });
    }
    addImageBlock(client, moduleName, mod, "image", "image", Panels, track, disposers, quickNavEntries, { x: 850, y: 92, w: 280 });

    // Nothing left over: values/results/progress/zones/actions/image/
    // parameters account for every property focus.json actually defines —
    // no "Other" catch-all needed, same reasoning as guider.js's own
    // removal of it (Gilles, 2026-10-03: "enleve le 'other' et la fenetre").
    updateQuickNav(quickNavEntries);
  }

  return moduleLifecycle(client, moduleName, build, dispose);
}
