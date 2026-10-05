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
//   addImageBlock(client, moduleName, mod, propName, elName,
//                 panelsApi, track, disposers, quickNavEntries, pos)  the whole thing, one call

import { el, icon, buildTabs } from "./dom.js";
import { getMediaBaseUrl, imageStatsLine } from "./render.js";
import { mediaUrl as buildMediaUrl } from "../core/index.js";
import { t } from "./i18n.js";

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
    if (src) { img.src = src; lastSrc = src; refreshImageViewer(src); }
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
  const close = el("button", { class: "img-viewer-close", type: "button", title: t("common.close") }, [icon("close")]);
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
  // A live refresh() swap below (new frame, same image) must NOT reset the
  // user's own zoom/pan the way opening a genuinely different image
  // should — suppressFit skips exactly one upcoming `load` for that case.
  let suppressFit = false;
  img.addEventListener("load", () => {
    if (suppressFit) { suppressFit = false; return; }
    fit();
  });

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

  // Identity used by refresh() below to decide "is this the same image,
  // just a newer frame" — the path without the cache-busting `?t=` query,
  // same convention every subscribed src in this file already follows.
  let currentBase = null;
  return {
    open(src) {
      root.hidden = false;
      document.addEventListener("keydown", onKey);
      currentBase = src.split("?")[0];
      if (img.src !== src) img.src = src; else fit();
    },
    refresh(src) {
      if (root.hidden || src.split("?")[0] !== currentBase) return;
      suppressFit = true;
      img.src = src;
    },
  };
}

export function openImageViewer(src) {
  if (!src) return;
  if (!viewerCtl) viewerCtl = buildViewer();
  viewerCtl.open(src);
}

/** Pushes a freshly-updated src to the shared fullscreen viewer IF it's
 * currently open and showing this same image (same path, ignoring the
 * cache-busting `?t=`) — a no-op otherwise (viewer closed, or open on a
 * different image). Every live-subscribed caller in this file (the module
 * background, the map/alternates grids below) calls this right after
 * updating its own thumbnail/background `src`, so a viewer left open
 * during a running Loop keeps showing the latest frame instead of
 * freezing on whichever one it was opened on — Gilles, 2026-10-05: "ces
 * images sont prevues pour s'actualiser ... il faut qu'elles se
 * rechargent automatiquement, et aussi celles qui sont en plein ecran". */
export function refreshImageViewer(src) {
  if (!viewerCtl || !src) return;
  viewerCtl.refresh(src);
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
// Section title + its own (single, already-merged) value on one line - a
// further density pass on statRow()/stat-sec above (Gilles, 2026-10-03,
// after a first cut still put the title on its own line above the value:
// "ça prend encore trop de place ... Dimensions RGB 1024x768" - title and
// value inline, not stacked). "Focus : HFR avg ..." (Gilles, same thread,
// one more round): colon separator, same pattern applied to the
// "Dimensions" line too for a consistent look between the two.
// `value` is either a plain string (Dimensions' case) or an array mixing
// plain mono strings with {tag, value} sub-labels (Focus's "HFR avg X''
// dev Y''" - Gilles, one more round yet: "mets aussi le 'dev' avec cette
// meme police", same small/dim/uppercase treatment as the section label
// itself, not baked into the plain mono text).
function statSecLine(label, value) {
  const row = el("div", { class: "stat-sec-line" }, [el("span", { class: "stat-sec-inline", text: label + " :" })]);
  for (const p of Array.isArray(value) ? value : [value]) {
    if (p && typeof p === "object") {
      row.appendChild(el("span", { class: "stat-sec-inline", text: p.tag + " :" }));
      row.appendChild(el("span", { class: "mono", text: String(p.value) }));
    } else {
      row.appendChild(el("span", { class: "mono", text: p }));
    }
  }
  return row;
}
// "Label : value" pairs on one row, for the Stars/SNR line below the Focus
// one - no longer bare numbers (Gilles: "Stars : 93 · SNR : 2.60"), and
// each label in the same small/dim/uppercase font as "FOCUS"/"DIMENSIONS"
// above (Gilles, one more round: "garde la meme police sur 'Stars' que
// pour 'FOCUS'") - so each label is its own .stat-sec-inline span, not
// plain text baked into one mono string like statSecLine()'s value side.
function labeledPairsRow(pairs) {
  const row = el("div", { class: "stat-inline-row" });
  pairs.forEach(([label, value], i) => {
    if (i > 0) row.appendChild(el("span", { class: "stat-sec-inline", text: "·" }));
    row.appendChild(el("span", { class: "stat-sec-inline", text: label + " :" }));
    row.appendChild(el("span", { class: "mono", text: String(value) }));
  });
  return row;
}

function renderImageStatsTable(v) {
  const wrap = el("div", { class: "img-stats-full" });
  const ch = v.channels || 1;

  // "×" matches render.js's own imageStatsLine() dimension formatting
  // (same quick-stats line, kept visually consistent rather than a plain
  // "x").
  wrap.appendChild(statSecLine(t("imagelayer.dimensions"), `${ch === 3 ? "RGB" : "Grayscale"} ${v.width || "N/A"}×${v.height || "N/A"}`));

  if (v.hfravg != null || v.stars != null || v.snr != null) {
    // HFR avg + its own deviation on the section's title line - "''" (two
    // straight quotes) is the arcsec shorthand focus.json's own element
    // labels already use (e.g. "Average HFR ('')"), not guessed.
    const hfrParts = [];
    if (v.hfravg != null) hfrParts.push(`HFR avg ${fmtNum(v.hfravg)}''`);
    if (v.hfravgdev != null) hfrParts.push({ tag: "dev", value: fmtNum(v.hfravgdev) + "''" });
    wrap.appendChild(statSecLine(t("imagelayer.focus"), hfrParts));
    // Stars/SNR merged underneath, each with its own "Label : value" pair
    // this time (Gilles: "Stars : 93 · SNR : 2.60") - same " · " join
    // style as render.js's own imageStatsLine().
    const pairs = [];
    if (v.stars != null) pairs.push(["Stars", v.stars]);
    if (v.snr != null) pairs.push(["SNR", fmtNum(v.snr)]);
    if (pairs.length) wrap.appendChild(labeledPairsRow(pairs));
  }

  wrap.appendChild(el("div", { class: "stat-sec", text: t("imagelayer.statistics") }));
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
    wrap.appendChild(el("div", { class: "stat-sec", text: t("imagelayer.astrometry") }));
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
      { label: t("imagelayer.channelR"), data: flat(h[0]), borderColor: "#ef5350", backgroundColor: "rgba(239,83,80,0.3)", borderWidth: 1, fill: true },
      { label: t("imagelayer.channelG"), data: flat(h[1]), borderColor: "#66bb6a", backgroundColor: "rgba(102,187,106,0.3)", borderWidth: 1, fill: true },
      { label: t("imagelayer.channelB"), data: flat(h[2]), borderColor: "#42a5f5", backgroundColor: "rgba(66,165,245,0.3)", borderWidth: 1, fill: true },
    ]
    : [{ label: t("imagelayer.channelValue"), data: flat(h[0]), borderColor: "rgba(200,200,200,0.9)", backgroundColor: "rgba(200,200,200,0.35)", borderWidth: 1, fill: true }];

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
 * not shown by default, see Panels.make()'s `collapsed` option. Split into
 * a "Stats"/"Histogram" tab pair (dom.js's own buildTabs()) rather than
 * stacked one above the other — Gilles, 2026-10-03: "modifie la fenetre
 * 'image stats' pour qu'elle soit plus dense, par exemple un onglet
 * histogramme et un onglet stats". */
export function buildImageStatsHistogramPanel(client, moduleName, propName, elName) {
  const info = el("div");
  const histoWrap = el("div", { class: "gd-chart" });
  const canvas = el("canvas");
  histoWrap.appendChild(canvas);
  const wrap = buildTabs([
    { label: t("imagelayer.stats"), node: info },
    { label: t("imagelayer.histogram"), node: histoWrap },
  ]);
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

// --------------------------------------------------------- alternate crops
/** Small strip of an image element's own "alternates" (secondary crops
 * alongside the main `urljpeg` — the focus module's own tilt quadrants:
 * one crop per sensor corner, used to diagnose sensor tilt), ported from
 * OstErix/Rooster's own image-element.component's alternates-strip (source
 * read directly, not guessed — `alternates?: string[]` on its ImageElement
 * model, same relative-media-path shape as `urljpeg` itself) — Gilles,
 * 2026-10-03: "sur rooster on affiche 4 image supplementaires, de tilt".
 * Generic to any image element carrying an `alternates` array (not
 * focus-specific itself, same "image handling lives here, any template can
 * reuse it" reasoning as the rest of this file); clicking a thumbnail opens
 * it in the same fullscreen viewer as the main image. */
export function buildImageAlternatesPanel(client, moduleName, propName, elName) {
  const wrap = el("div", { class: "img-alternates" });
  const unsub = client.subscribe(client.topics.property(moduleName, propName), (prop) => {
    wrap.innerHTML = "";
    const v = elValue(prop, elName);
    const alts = v.alternates || [];
    if (!alts.length) { wrap.appendChild(el("div", { class: "muted small", text: t("imagelayer.noAlternates") })); return; }
    const ts = Date.now();
    for (const path of alts) {
      const src = buildMediaUrl(getMediaBaseUrl(), path) + "?t=" + ts;
      const img = el("img", { class: "img-alternate-thumb", src, alt: "" });
      img.addEventListener("click", () => openImageViewer(src));
      wrap.appendChild(img);
      refreshImageViewer(src);
    }
  });
  wrap._dispose = unsub;
  return wrap;
}

// ------------------------------------------------------- multi-image grid
/** Grid of labeled thumbnails, one per *separate* image property (not one
 * element's own `alternates` — see buildImageAlternatesPanel() above for
 * that case) — e.g. inspector's own HFR/shape/collimation/corners maps,
 * each its own "img" property. One panel instead of N small ones, each
 * thumbnail click opens the shared fullscreen viewer — Gilles, 2026-10-05,
 * on the previous one-tiny-floating-panel-per-map layout: "je cherche une
 * facon ergonomique d'afficher les 4 autres, autrement que dans ces
 * petites fenetres", then picking the 2x2 grid-of-clickable-thumbnails
 * option over a tabbed single-image-at-a-time alternative — a grid keeps
 * all 4 comparable at a glance, which matters for correlating e.g. a
 * hotspot in the HFR map with the shape map at the same field position.
 *
 * `items`: `[{propName, elName, label}, ...]` — `label` is a fallback,
 * the server's own property label (once the dump arrives) always wins.
 */
export function buildImageGridPanel(client, moduleName, items) {
  const wrap = el("div", { class: "img-grid" });
  const tiles = items.map((item) => {
    const label = el("div", { class: "img-grid-label", text: item.label || item.propName });
    const img = el("img", { class: "img-grid-thumb", alt: "" });
    let src = null;
    img.addEventListener("click", () => { if (src) openImageViewer(src); });
    const tile = el("div", { class: "img-grid-tile" }, [img, label]);
    const unsub = client.subscribe(client.topics.property(moduleName, item.propName), (prop) => {
      if (prop && prop.label) label.textContent = prop.label;
      const v = elValue(prop, item.elName);
      const path = v.urljpeg || v.urlthumbnail;
      src = path ? buildMediaUrl(getMediaBaseUrl(), path) + "?t=" + Date.now() : null;
      img.src = src || "";
      tile.classList.toggle("img-grid-tile-empty", !src);
      if (src) refreshImageViewer(src);
    });
    wrap.appendChild(tile);
    return unsub;
  });
  wrap._dispose = () => { for (const unsub of tiles) unsub(); };
  return wrap;
}

/**
 * The full "a module has an image" block a template wires in one call:
 * background + a small quick-stats panel + a deeper stats/histogram panel
 * (hidden until its own quicknav button is clicked), plus the matching
 * quicknav entries — shared by templates/scaffold.js and templates/
 * guider.js so the two can't drift apart. No-op if `mod` doesn't actually
 * have `propName`. `panelsApi` is js/panels.js (passed in rather than
 * imported here, same as every template already does, to keep this file
 * itself DOM-shell-agnostic); `track`/`disposers`/`quickNavEntries` are the
 * caller's own bookkeeping, same shapes every template already uses.
 *
 * `opts.quickPanel: false` skips the small quick-stats panel + its own
 * "Image" quicknav button (background image and the deeper Stats panel
 * stay) — Gilles, 2026-10-03, on focus/guider specifically: "supprime le
 * bouton 'image' et la fenetre associee" (the background already carries
 * the picture full-screen, and the Stats panel/button already covers the
 * same figures in more depth — the small panel was pure redundancy once
 * both of those existed). Scaffolded templates (inspector/polar/
 * parkmanager/sequencer, via templates/scaffold.js) keep the default.
 */
export function addImageBlock(client, moduleName, mod, propName, elName, panelsApi, track, disposers, quickNavEntries, pos, opts) {
  if (!(propName in mod.properties)) return;
  const p = { x: 660, y: 92, w: 280, ...pos };
  disposers.push(showBackgroundImage(client, moduleName, propName, elName).dispose);

  if (!opts || opts.quickPanel !== false) {
    const b = panelsApi.make({ id: propName, title: mod.properties[propName].label || "Image", x: p.x, y: p.y, w: p.w });
    b.appendChild(track(buildImageQuickStats(client, moduleName, propName, elName)));
    quickNavEntries.push({ id: propName, label: mod.properties[propName].label || "Image" });
  }

  const sb = panelsApi.make({ id: propName + "-stats", title: t("imagelayer.imageStats"), x: p.x, y: p.y + 100, w: p.w + 100, collapsed: true });
  sb.appendChild(track(buildImageStatsHistogramPanel(client, moduleName, propName, elName)));
  // Icon-only (render.js's own renderActionsPanel() set the precedent for
  // this field pair) - Gilles, 2026-10-03: "modifie l'apparence du bouton
  // 'Stats' pour n'afficher qu'une icone 'bar_chart'" - ligature verified
  // against the vendored Material Symbols font before use (base.css's own
  // header comment on why that check always comes first).
  quickNavEntries.push({ id: propName + "-stats", label: t("imagelayer.stats"), icon: "bar_chart", iconOnly: true });
}
