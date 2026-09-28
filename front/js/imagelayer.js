// imagelayer.js — full-viewport background image, fullscreen viewer, and
// stats/histogram panels for an OST "img" element — ported from ostnoctua's
// own imagelayer.js (OstImage.background/viewer/statsPanel) onto core/'s
// client API — Gilles, 2026-09-29: "sur le template guider affiche l'image
// derriere en ecran de fond, comme dans ostnoctua ... / ajoute un bouton
// pour les stats de l'image ... piloter une fenetre avec histogramme et
// statistiques ... regarde osterix" then, once guider's own version of that
// existed, "Est-ce que ce truc d'image sera reutilisable sur d'autres
// templates ?" -> "Go": moved here from templates/guider.js and
// generalized (moduleName/propName/elName params instead of guider/"image"
// hardcoded) so any future template can reuse all of it, not just the
// background/viewer pair. Field list cross-checked against OstErix's own
// statistics-dialog/histogram-dialog components (dimensions, focus/stars,
// per-channel min/max/mean/median/stddev, astrometry) — combined into one
// panel here, same as ostnoctua, rather than split into two dialogs like
// OstErix does.
//
//   setBgLayer(node)                                              once, at boot
//   showBackgroundImage(client, moduleName, propName, elName)     -> {dispose}
//   openImageViewer(src)                                          standalone use
//   buildImageQuickStats(client, moduleName, propName, elName)    -> node (one live line)
//   buildImageStatsHistogramPanel(client, moduleName, propName, elName) -> node

import { el, icon } from "./dom.js";
import { getMediaBaseUrl, imageStatsLine } from "./render.js";
import { mediaUrl as buildMediaUrl } from "../core/index.js";

if (window.Chart && window.Chart.registerables) {
  try { window.Chart.register(...window.Chart.registerables); } catch (e) { /* already done */ }
}

let bgLayer = null;
export function setBgLayer(node) { bgLayer = node; }

function jpegSrc(elDef) {
  const v = elDef && elDef.value;
  const path = v && (v.urljpeg || v.urlthumbnail);
  if (!path) return null;
  return buildMediaUrl(getMediaBaseUrl(), path) + "?t=" + Date.now();
}

/**
 * Fills the shared #bg-layer with one module's image element, letterboxed
 * (CSS object-fit: contain — resized, aspect ratio always preserved, see
 * panels.css's #bg-image). A click opens the fullscreen viewer. Self-
 * subscribes to the element's own property (topics.property — the
 * fine-grained topic ea/ee/ps/ap traffic actually fires on) so it stays
 * live independent of whatever else is rebuilding around it. Returns
 * {dispose}; the caller must call it when the view changes away (same
 * convention as every other subscription in this codebase).
 */
export function showBackgroundImage(client, moduleName, propName, elName) {
  if (!bgLayer) return { dispose: () => {} };
  bgLayer.classList.add("img-active");
  bgLayer.innerHTML = "";
  const img = el("img", { id: "bg-image", alt: "" });
  bgLayer.appendChild(img);
  let lastSrc = null;
  const onClick = () => { if (lastSrc) openImageViewer(lastSrc); };
  img.addEventListener("click", onClick);

  const unsub = client.subscribe(client.topics.property(moduleName, propName), (prop) => {
    const src = jpegSrc(prop && prop.elements && prop.elements[elName]);
    if (src) { img.src = src; lastSrc = src; }
  });

  return {
    dispose: () => {
      unsub();
      img.removeEventListener("click", onClick);
      bgLayer.innerHTML = "";
      bgLayer.classList.remove("img-active");
    },
  };
}

// ------------------------------------------------------------------ viewer
// One shared overlay (built lazily, reused across opens) — wheel zooms
// toward the cursor, drag pans, Esc/backdrop click/close button closes.
let viewerCtl = null;
function buildViewer() {
  const root = el("div", { class: "img-viewer" });
  root.hidden = true;
  const stage = el("div", { class: "img-viewer-stage" });
  const img = el("img", { class: "img-viewer-img" });
  stage.appendChild(img);
  const close = el("button", { class: "img-viewer-close", type: "button", title: "Close" }, [icon("close")]);
  root.appendChild(stage);
  root.appendChild(close);
  document.body.appendChild(root);

  let scale = 1, tx = 0, ty = 0;
  const apply = () => { img.style.transform = `translate(${tx}px,${ty}px) scale(${scale})`; };

  function fit() {
    const vw = root.clientWidth, vh = root.clientHeight;
    const iw = img.naturalWidth || vw, ih = img.naturalHeight || vh;
    scale = (Math.min(vw / iw, vh / ih) * 0.95) || 1;
    tx = (vw - iw * scale) / 2;
    ty = (vh - ih * scale) / 2;
    apply();
  }
  img.addEventListener("load", fit);

  stage.addEventListener("wheel", (ev) => {
    ev.preventDefault();
    const r = img.getBoundingClientRect();
    const cx = ev.clientX - r.left, cy = ev.clientY - r.top; // point under the cursor, in the (scaled) image's own coordinates
    const f = ev.deltaY < 0 ? 1.15 : 1 / 1.15;
    const ns = Math.max(0.1, Math.min(40, scale * f));
    const k = ns / scale;
    tx -= cx * (k - 1); // keep that same point fixed under the cursor
    ty -= cy * (k - 1);
    scale = ns;
    apply();
  }, { passive: false });

  let drag = null;
  stage.addEventListener("mousedown", (ev) => {
    if (ev.button !== 0) return;
    drag = { x: ev.clientX, y: ev.clientY, tx, ty };
    stage.classList.add("grabbing");
    ev.preventDefault();
  });
  window.addEventListener("mousemove", (ev) => {
    if (!drag) return;
    tx = drag.tx + (ev.clientX - drag.x);
    ty = drag.ty + (ev.clientY - drag.y);
    apply();
  });
  window.addEventListener("mouseup", () => { drag = null; stage.classList.remove("grabbing"); });

  const onKey = (ev) => { if (ev.key === "Escape") hide(); };
  function hide() {
    root.hidden = true;
    document.removeEventListener("keydown", onKey);
  }
  close.addEventListener("click", hide);
  root.addEventListener("mousedown", (ev) => { if (ev.target === root) hide(); });

  return {
    open(src) {
      root.hidden = false;
      document.addEventListener("keydown", onKey);
      if (img.src !== src) img.src = src; else fit();
    },
  };
}

export function openImageViewer(src) {
  if (!src) return;
  if (!viewerCtl) viewerCtl = buildViewer();
  viewerCtl.open(src);
}

function elValue(prop, elName) {
  return (prop && prop.elements && prop.elements[elName] && prop.elements[elName].value) || {};
}

// -------------------------------------------------------------- quick stats
// One live text line (dimensions/HFR/stars/SNR/solved — render.js's
// imageStatsLine()), no thumbnail — meant to sit in a small panel next to
// a showBackgroundImage() background, where the image itself is already on
// screen.
export function buildImageQuickStats(client, moduleName, propName, elName) {
  const wrap = el("div", { class: "muted small mono" });
  const unsub = client.subscribe(client.topics.property(moduleName, propName), (prop) => {
    const stats = imageStatsLine(elValue(prop, elName));
    wrap.textContent = stats.length ? stats.join(" · ") : "No image yet";
  });
  wrap._dispose = unsub;
  return wrap;
}

// --------------------------------------------------- full stats + histogram
function fmtNum(v, d) { return (v == null || isNaN(v)) ? "N/A" : Number(v).toFixed(d == null ? 2 : d); }
function chAt(v, i) { return Array.isArray(v) ? (i < v.length ? fmtNum(v[i]) : "N/A") : (i === 0 ? fmtNum(v) : "N/A"); }
function statRow(label, value) {
  return el("div", { class: "stat-row" }, [el("span", { class: "stat-label", text: label }), el("span", { class: "stat-value mono", text: value })]);
}

function renderImageStatsTable(v) {
  const wrap = el("div", { class: "img-stats-full" });
  const ch = v.channels || 1;

  wrap.appendChild(el("div", { class: "stat-sec", text: "Dimensions" }));
  wrap.appendChild(statRow("Width", (v.width || "N/A") + " px"));
  wrap.appendChild(statRow("Height", (v.height || "N/A") + " px"));
  wrap.appendChild(statRow("Type", ch === 3 ? "RGB" : "Grayscale"));

  if (v.hfravg != null || v.stars != null || v.snr != null) {
    wrap.appendChild(el("div", { class: "stat-sec", text: "Focus & stars" }));
    if (v.hfravg != null) wrap.appendChild(statRow("HFR avg", fmtNum(v.hfravg)));
    if (v.hfravgdev != null) wrap.appendChild(statRow("HFR avg dev", fmtNum(v.hfravgdev)));
    if (v.stars != null) wrap.appendChild(statRow("Stars", String(v.stars)));
    if (v.snr != null) wrap.appendChild(statRow("SNR", fmtNum(v.snr)));
  }

  wrap.appendChild(el("div", { class: "stat-sec", text: "Statistics" }));
  const table = el("table", { class: "img-stats-table" });
  const head = el("tr", null, [el("th", null, "")]);
  (ch === 3 ? ["R", "G", "B"] : ["Value"]).forEach((h) => head.appendChild(el("th", null, h)));
  table.appendChild(el("thead", null, [head]));
  const tbody = el("tbody");
  [["Min", v.min], ["Max", v.max], ["Mean", v.mean], ["Median", v.median], ["Std dev", v.stddev]].forEach(([label, val]) => {
    const row = el("tr", null, [el("td", { class: "stat-label" }, label)]);
    const n = ch === 3 ? 3 : 1;
    for (let i = 0; i < n; i++) row.appendChild(el("td", null, chAt(val, i)));
    tbody.appendChild(row);
  });
  table.appendChild(tbody);
  wrap.appendChild(table);

  if (v.issolved) {
    wrap.appendChild(el("div", { class: "stat-sec", text: "Astrometry" }));
    wrap.appendChild(statRow("RA", fmtNum(v.solverra, 3) + "°"));
    wrap.appendChild(statRow("DEC", fmtNum(v.solverde, 3) + "°"));
  }

  return wrap;
}

function updateImageHistogram(v, canvas, chartRef) {
  const h = v.histogram;
  if (!window.Chart || !h || !h.length) {
    if (chartRef.chart) { chartRef.chart.destroy(); chartRef.chart = null; }
    canvas.parentElement.classList.add("hidden");
    return;
  }
  canvas.parentElement.classList.remove("hidden");
  const bins = h[0].length;
  const labels = Array.from({ length: bins }, (_, i) => String(i));
  const flat = (arr) => arr.map((b) => (Array.isArray(b) ? b[0] : b));
  const ch = v.channels || 1;
  const ds = ch === 3
    ? [
      { label: "R", data: flat(h[0]), borderColor: "#ef5350", backgroundColor: "rgba(239,83,80,0.3)", borderWidth: 1, fill: true },
      { label: "G", data: flat(h[1]), borderColor: "#66bb6a", backgroundColor: "rgba(102,187,106,0.3)", borderWidth: 1, fill: true },
      { label: "B", data: flat(h[2]), borderColor: "#42a5f5", backgroundColor: "rgba(66,165,245,0.3)", borderWidth: 1, fill: true },
    ]
    : [{ label: "Value", data: flat(h[0]), borderColor: "rgba(200,200,200,0.9)", backgroundColor: "rgba(200,200,200,0.35)", borderWidth: 1, fill: true }];

  if (chartRef.chart) {
    chartRef.chart.data.labels = labels;
    ds.forEach((d, i) => { if (chartRef.chart.data.datasets[i]) chartRef.chart.data.datasets[i].data = d.data; });
    chartRef.chart.update("none");
    return;
  }
  const grey = "rgba(255,255,255,0.5)";
  chartRef.chart = new window.Chart(canvas.getContext("2d"), {
    type: "bar",
    data: { labels, datasets: ds },
    options: {
      responsive: true, maintainAspectRatio: false, animation: false,
      plugins: { legend: { display: ch === 3, labels: { color: grey, boxWidth: 12, font: { size: 9 } } } },
      scales: {
        x: { display: true, ticks: { color: grey, maxTicksLimit: 12, font: { size: 8 } }, grid: { display: false } },
        y: { display: true, beginAtZero: true, ticks: { color: grey, font: { size: 8 } }, grid: { color: "rgba(255,255,255,0.06)" } },
      },
      interaction: { intersect: false, mode: "index" },
    },
  });
}

/** Full per-channel stats table + histogram chart for one image element —
 * meant to be opened on demand (e.g. a template's own quicknav button),
 * not shown by default, see Panels.make()'s `collapsed` option. */
export function buildImageStatsHistogramPanel(client, moduleName, propName, elName) {
  const wrap = el("div", { class: "img-stats-panel" });
  const info = el("div");
  const histoWrap = el("div", { class: "gd-chart" });
  const canvas = el("canvas");
  histoWrap.appendChild(canvas);
  wrap.appendChild(info);
  wrap.appendChild(histoWrap);
  const chartRef = { chart: null };

  const unsub = client.subscribe(client.topics.property(moduleName, propName), (prop) => {
    const v = elValue(prop, elName);
    info.innerHTML = "";
    info.appendChild(renderImageStatsTable(v));
    updateImageHistogram(v, canvas, chartRef);
  });

  wrap._dispose = () => { unsub(); if (chartRef.chart) { chartRef.chart.destroy(); chartRef.chart = null; } };
  return wrap;
}
