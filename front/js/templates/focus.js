// templates/focus.js — bespoke floating-panel arrangement for the "focus"
// module. Started as a scaffolded template (buildScaffoldTemplate) but the
// "values" panel (the V-curve) needed a real chart — Gilles, 2026-10-03,
// on the plain generic table it used to render as: "la 'focus curve' est
// trop minimale, regarde le graphique de rooster, avec la courbe de
// regression et les barres d'erreur". progress stays on the generic
// renderProperty() path (nothing wrong with it as a plain table), grouped
// into one panel together with results (Gilles, 2026-10-03: "on va
// regrouper les fenetres 'progress' et 'final result'"). results' own
// HFR/position got a further bespoke pass (Gilles, same day: "inspire toi
// de l'affichage des valeurs RMS du guider pour afficher ... HFR et
// position") — see buildResultsBoxes below, sharing guider.js's own
// .value-row/.value-box boxed-figure look (renamed from .rms-* once a
// second module wanted it — components.css's own comment on that). "zones"
// (the per-zone best-position grid) briefly got a tile display here too,
// then was dropped outright — Gilles: "toutes ces valeurs numeriques n'ont
// pas d'interet" — in favour of what Rooster actually shows there: 4 tilt
// crops from the image element's own `alternates` array ("sur rooster on
// affiche 4 image supplementaires, de tilt" — see buildImageAlternatesPanel
// in imagelayer.js, generic to any image element, not focus-specific).
// Same "bespoke only where it actually earns its keep" split guider.js
// already settled on.
//
// Real property names confirmed against src/modules/focus/focus.json:
// values (V-curve data — hasGrid, graphType XY, graphParams focpos/
// loopHFRavg, plus imgHFRdev for error bars and fitk0/fitk1/fitk2 for the
// polynomial fit — see buildFocusCurvePanel below), results (hfr/pos
// readout), progress (spinner) — plus parameters (hasprofile:true,
// Parameters modal only, not a panel), actions (abortfocus/autofocus/
// loop) and image, both generic to any IndiModule via
// defineMeAsFocuser()/defineMeAsImager(). "zones" exists too but gets no
// panel — see above.
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
import { addImageBlock, buildImageAlternatesPanel } from "../imagelayer.js";
import { el } from "../dom.js";
import * as Panels from "../panels.js";
import { t } from "../i18n.js";

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
// Exported too — reused as-is (same chart, just placed in a bigger card)
// by views.js's Home dashboard (Gilles, 2026-10-05: "mets les graphes
// correspondants" on the bigger focus/guider home cards).
export function buildFocusCurvePanel(client, moduleName) {
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

// ----------------------------------------------------------------- results
/** "Final result" HFR/position as bordered value boxes, instead of the
 * plain generic label/value table renderProperty() would otherwise give it
 * — Gilles, 2026-10-03: "inspire toi de l'affichage des valeurs RMS du
 * guider pour afficher ... HFR et position" (guider.js's own
 * buildValuesPanel() RMS row). Self-subscribed to the "results" property,
 * same fine-grained-topic pattern as the other bespoke panels here. */
function buildResultsBoxes(client, moduleName) {
  const wrap = el("div", { class: "value-row" });
  const unsub = client.subscribe(client.topics.property(moduleName, "results"), (prop) => {
    wrap.innerHTML = "";
    if (!prop) return;
    const e = prop.elements;
    // Labels come from the server (already translated) - never re-invented
    // locally, same rule guider.js's own RMS row follows.
    for (const key of ["hfr", "pos"]) {
      if (!e[key]) continue;
      wrap.appendChild(el("div", { class: "value-box" }, [
        el("div", { class: "value-box-label", text: e[key].label || key }),
        el("div", { class: "value-box-number", text: (e[key].value || 0).toFixed(2) }),
      ]));
    }
  });
  wrap._dispose = unsub;
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

    // "Final result" + "Progress" grouped into one panel (Gilles,
    // 2026-10-03: "on va regrouper les fenetres 'progress' et 'final
    // result'") — each keeps its own full renderProperty() header/label
    // (same stacking scaffold.js's own panelSpec.extra does for a
    // secondary property sharing a primary one's box).
    if (has("results") || has("progress")) {
      const title = (mod.properties.results && mod.properties.results.label) || "Final result";
      const b = Panels.make({ id: "results", title, x: 20, y: 330, w: 300 });
      if (has("results")) b.appendChild(track(buildResultsBoxes(client, moduleName)));
      if (has("progress")) b.appendChild(track(renderProperty(client, moduleName, "progress")));
      quickNavEntries.push({ id: "results", label: title });
    }
    if (has("values")) {
      const b = Panels.make({ id: "values", title: mod.properties.values.label || "Focus curve", x: 350, y: 92, w: 480 });
      b.appendChild(track(buildFocusCurvePanel(client, moduleName)));
      quickNavEntries.push({ id: "values", label: mod.properties.values.label || "Focus curve" });
    }
    // No small "Image" quick-stats panel/button here (Gilles, 2026-10-03:
    // "supprime le bouton 'image' et la fenetre associee") - the
    // background already shows the picture full-screen, and the "Stats"
    // panel/button (below) already covers the same figures in more depth.
    addImageBlock(client, moduleName, mod, "image", "image", Panels, track, disposers, quickNavEntries, { x: 850, y: 92, w: 280 }, { quickPanel: false });
    if (has("image")) {
      const b = Panels.make({ id: "tilt", title: t("focus.tilt"), x: 1150, y: 92, w: 220 });
      b.appendChild(track(buildImageAlternatesPanel(client, moduleName, "image", "image")));
      quickNavEntries.push({ id: "tilt", label: t("focus.tilt") });
    }

    // values/results(+progress)/tilt/actions/image/parameters account for
    // everything focus.json defines — "zones" gets no panel at all
    // (see this file's own header comment) and no "Other" catch-all is
    // needed either, same reasoning as guider.js's own removal of it
    // (2026-10-03: "enleve le 'other' et la fenetre").
    updateQuickNav(quickNavEntries);
  }

  return moduleLifecycle(client, moduleName, build, dispose);
}
