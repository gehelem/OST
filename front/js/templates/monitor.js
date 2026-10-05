// templates/monitor.js — scaffolded template for the "monitor" module (see
// scaffold.js's own header), plus one bespoke panel for its "events" grid —
// ported from OstErix/Rooster's own src/app/pages/monitor/
// monitor.component.ts/.html and monitor-graph.component.ts (read
// directly), onto core/'s client API and Chart.js the same way guider.js/
// focus.js's own charts already do.
//
// Real property names confirmed against src/modules/monitor/monitor.json /
// monitor.cpp: parms (autostart/monitor_sampling/maxrows, hasprofile:true,
// stays Parameters-modal-only like any hasprofile property this template
// doesn't list), slaves (which guider/sequencer/focus/navigator/planner/
// parkmanager/polar/inspector *instance* to watch, hasprofile:true, same -
// not shown directly, matching Rooster's own monitor page, which has no
// panel for it either), filter (hasprofile:false, no elements of its own
// in the json - whatever it turns out to carry at runtime, the generic
// renderer already handles it, same as Rooster's own generic
// <app-property> for it), archive (hasGrid/showGrid, one row per saved
// session file: date/filename/size - the generic grid renderer already
// covers this), events (hasGrid/showGrid, one row per recorded sample:
// ts/module/type/key/val_num1-8/val_int1-3/val_str1-3 - too wide/numeric
// to read as a table, same "needs a dedicated implementation" case
// guider's PHD2 graph and focus's V-curve already are, see
// buildMonitorGraphPanel below). monitor never calls defineMeAsImager()
// (no "image" property, matching Rooster's own page, which has no image
// card either) - no imagePos override needed, same as parkmanager.js's
// own call.
//
// A small "status" panel too, reading signals.statedescription/event/
// eventdescription/val_num1/val_int1 - same 5 fields Rooster's own
// status-card shows (event/val_* aren't in monitor.json itself, but
// confirmed live against a real loaded instance, 2026-10-06: "signals"
// does carry them at runtime, generic to every IndiModule, not
// monitor-specific). The base state/statedescription every module gets is
// already covered by the generic status dot (header/modbar) elsewhere in
// this app (no per-module status panel, "la supprimer completement",
// render.js's own renderModuleView doc comment) - this one earns its keep
// anyway since "last event" is genuinely monitor-specific information
// (Rooster's own reason for showing it), not a duplicate of the dot.

import { buildScaffoldTemplate } from "./scaffold.js";
import { el } from "../dom.js";
import { t } from "../i18n.js";

if (window.Chart && window.Chart.registerables) {
  try { window.Chart.register(...window.Chart.registerables); } catch (e) { /* already done */ }
}
// Same error-bars plugin focus.js's own V-curve chart registers (see that
// file's header for why this isn't part of Chart.js's own registerables).
if (window.Chart && window.ChartErrorBars) {
  try { window.Chart.register(window.ChartErrorBars.LineWithErrorBarsController, window.ChartErrorBars.PointWithErrorBar); } catch (e) { /* already done */ }
}

const num = (v) => (typeof v === "number" ? v : NaN);

// events.ts is a "datetime" element (monitor.json) - {year,month,day,hh,
// mm,ss,[ms]}, same shape render.js's own formatScalar() already handles
// for that type (confirmed there, not guessed) - just turned into an
// epoch ms number here instead of a display string, for Chart.js's own
// linear time axis.
function parseTs(v) {
  if (!v || v.year === undefined) return 0;
  return new Date(v.year, v.month - 1, v.day, v.hh, v.mm, v.ss, v.ms || 0).getTime();
}

// Groups the "events" grid's rows by their own `key` column into the same
// 4 series Rooster's own monitor-graph.component.ts does (guideRMS/
// guideSNR as error-bar lines, imagehfr/focusdone as plain scatter
// points) - val_num1/val_num5 are the grid's own "Mean"/"Std dev"
// columns (monitor.json), read by name here (ostcore's datastore already
// normalizes grid rows into {colName: value} objects - no index-based
// column lookup needed, unlike Rooster's own raw grid array).
function monitorChartData(rows) {
  const rms = [], snr = [], imgHfr = [], focHfr = [];
  for (const r of rows) {
    const ts = parseTs(r.ts);
    const mean = num(r.val_num1);
    if (!ts || !isFinite(mean)) continue;
    const stddev = isFinite(num(r.val_num5)) ? r.val_num5 : 0;
    if (r.key === "guideRMS" && mean > 0) rms.push({ x: ts, y: mean, yMin: Math.max(0, mean - stddev), yMax: mean + stddev });
    else if (r.key === "guideSNR" && mean > 0) snr.push({ x: ts, y: mean, yMin: Math.max(0, mean - stddev), yMax: mean + stddev });
    else if (r.key === "imagehfr" && mean > 0) imgHfr.push({ x: ts, y: mean });
    else if (r.key === "focusdone" && mean > 0) focHfr.push({ x: ts, y: mean });
  }
  const allY = [...rms.map((p) => p.yMax), ...imgHfr.map((p) => p.y), ...focHfr.map((p) => p.y)];
  const yMax = allY.length ? Math.max(...allY) * 1.25 : 5;
  const snrMax = snr.length ? Math.max(...snr.map((p) => p.yMax)) * 1.25 : 100;
  return { rms, snr, imgHfr, focHfr, yMax, snrMax };
}

function monitorDatasets(d) {
  return [
    {
      type: "lineWithErrorBars", label: t("monitor.rms"), data: d.rms, yAxisID: "yMain",
      borderColor: "#FF9800", backgroundColor: "rgba(255,152,0,0.08)", borderWidth: 1.5,
      pointRadius: 2, tension: 0, fill: false,
      errorBarColor: "rgba(255,152,0,0.5)", errorBarWhiskerColor: "rgba(255,152,0,0.8)",
      errorBarLineWidth: 1, errorBarWhiskerLineWidth: 1, errorBarWhiskerSize: 6,
    },
    { label: t("monitor.imageHfr"), data: d.imgHfr, yAxisID: "yMain", borderColor: "#2196F3", backgroundColor: "#2196F3", pointRadius: 4, pointHoverRadius: 6 },
    { label: t("monitor.focusHfr"), data: d.focHfr, yAxisID: "yMain", borderColor: "#4CAF50", backgroundColor: "#4CAF50", pointRadius: 6, pointStyle: "triangle", pointHoverRadius: 8 },
    {
      type: "lineWithErrorBars", label: t("monitor.snr"), data: d.snr, yAxisID: "ySNR",
      borderColor: "rgba(244,67,54,0.7)", backgroundColor: "transparent", borderWidth: 1,
      pointRadius: 2, tension: 0, fill: false, borderDash: [4, 4],
      errorBarColor: "rgba(244,67,54,0.4)", errorBarWhiskerColor: "rgba(244,67,54,0.7)",
      errorBarLineWidth: 1, errorBarWhiskerLineWidth: 1, errorBarWhiskerSize: 6,
    },
  ];
}

/** Compact status readout: statedescription + the last recorded event
 * (event/eventdescription) + its value (val_num/val_int, only when either
 * is actually nonzero - same guard Rooster's own status-card uses), all
 * from the generic "signals" property. Real element keys confirmed live
 * against a loaded instance (2026-10-06) - "val_num"/"val_int", NOT
 * Rooster's own "val_num1"/"val_int1" (its TS source is stale here, this
 * is the real wire). Labels come from the server, never re-invented
 * locally, same rule every other bespoke panel in this app follows.
 * Self-subscribed, same fine-grained-topic pattern as every panel here. */
function buildMonitorStatusPanel(client, moduleName) {
  const wrap = el("div");
  const unsub = client.subscribe(client.topics.property(moduleName, "signals"), (prop) => {
    wrap.innerHTML = "";
    const e = prop && prop.elements;
    if (!e) return;
    const row = (label, value) => wrap.appendChild(el("div", { class: "metric-row" }, [el("span", { class: "label", text: label }), el("span", { class: "value", text: value })]));
    if (e.statedescription && e.statedescription.value) row(e.statedescription.label, e.statedescription.value);
    if (e.event && e.event.value) {
      row(e.event.label, e.event.value);
      if (e.eventdescription && e.eventdescription.value) row(e.eventdescription.label, e.eventdescription.value);
    }
    const valNum = e.val_num && e.val_num.value;
    const valInt = e.val_int && e.val_int.value;
    if (valNum) row(e.val_num.label, valNum.toFixed(2));
    if (valInt) row(e.val_int.label, String(valInt));
  });
  wrap._dispose = unsub;
  return wrap;
}

/** Builds the session graph panel's node, self-subscribed to the "events"
 * property (same fine-grained-topic pattern as guider.js's/focus.js's own
 * charts — grid row updates arrive on this topic, not on topics.module()).
 * Returns a node with `_dispose` set, ready for scaffold.js's own
 * `track()`. */
function buildMonitorGraphPanel(client, moduleName) {
  const wrap = el("div", { class: "gd-chart" });
  const canvas = el("canvas");
  wrap.appendChild(canvas);
  let chart = null;
  const grey = "rgba(255,255,255,0.55)";
  const grid = { color: "rgba(255,255,255,0.08)" };

  const unsub = client.subscribe(client.topics.property(moduleName, "events"), (prop) => {
    const rows = (prop && prop.rows) || [];
    const d = monitorChartData(rows);
    if (!chart) {
      chart = new window.Chart(canvas.getContext("2d"), {
        type: "scatter",
        data: { datasets: monitorDatasets(d) },
        options: {
          responsive: true, maintainAspectRatio: false, animation: false,
          interaction: { mode: "nearest", intersect: false, axis: "x" },
          plugins: {
            legend: { position: "bottom", labels: { color: grey, boxWidth: 14, boxHeight: 10, font: { size: 10 } } },
            tooltip: {
              mode: "nearest", intersect: false,
              callbacks: { title: (items) => items.length ? new Date(items[0].parsed.x).toLocaleTimeString(undefined, { hour: "2-digit", minute: "2-digit", second: "2-digit", hour12: false }) : "" },
            },
          },
          scales: {
            // x is a real epoch-ms scale (events span real wall-clock time,
            // not one shared row index like guider's own chart), so its
            // ticks need formatting - caught live: raw epoch numbers
            // showed on first render (Rooster's own formatTime() tick
            // callback, missed on the first port).
            x: {
              type: "linear",
              ticks: { color: grey, maxRotation: 0, autoSkip: true, maxTicksLimit: 8, font: { size: 9 }, callback: (v) => new Date(v).toLocaleTimeString(undefined, { hour: "2-digit", minute: "2-digit", second: "2-digit", hour12: false }) },
              grid,
            },
            yMain: {
              type: "linear", position: "left", min: 0, max: d.yMax,
              title: { display: true, text: "RMS / HFR", color: grey, font: { size: 10 } },
              ticks: { color: grey, font: { size: 9 }, maxTicksLimit: 6 }, grid,
            },
            ySNR: {
              type: "linear", position: "right", min: 0, max: d.snrMax,
              title: { display: true, text: t("monitor.snr"), color: "#F44336", font: { size: 10 } },
              ticks: { color: "#F44336", font: { size: 9 }, maxTicksLimit: 5 }, grid: { drawOnChartArea: false },
            },
          },
        },
      });
      return;
    }
    const datasets = monitorDatasets(d);
    datasets.forEach((ds, i) => { if (chart.data.datasets[i]) chart.data.datasets[i].data = ds.data; });
    chart.options.scales.yMain.max = d.yMax;
    chart.options.scales.ySNR.max = d.snrMax;
    chart.update("none");
  });

  wrap._dispose = () => { unsub(); if (chart) { chart.destroy(); chart = null; } };
  return wrap;
}

export const mount = buildScaffoldTemplate([
  {
    id: "events", title: "monitor.graph", x: 20, y: 92, w: 620,
    custom: (client, moduleName) => buildMonitorGraphPanel(client, moduleName),
    customProps: ["events"],
  },
  { id: "archive", x: 660, y: 92, w: 280 },
  { id: "filter", x: 660, y: 330, w: 280 },
  {
    id: "status", title: "monitor.status", x: 660, y: 470, w: 280,
    custom: (client, moduleName) => buildMonitorStatusPanel(client, moduleName),
    customProps: ["signals"],
  },
]);
