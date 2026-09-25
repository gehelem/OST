// panels.js — floating, draggable, minimizable, resizable panels.
//
// Direct adaptation of ostnoctua's panel.js (see ostnoctua/web/panel.js) to
// core/'s client API and UI3's design language, with one structural change:
// ostnoctua's layer is `position:fixed; inset:0` over the whole viewport
// (it has no persistent page chrome below the connection pill). UI3 keeps a
// topbar + module bar as permanent chrome, so the layer here is a normal
// flex child (see css/panels.css `#panel-layer`) and all drag/resize math is
// relative to the layer's own bounding box, not the window.
//
// Usage (mirrors ostnoctua's OstPanels exactly):
//   Panels.setLayer(layerEl);              // once, at boot
//   Panels.setScope(moduleName);           // layout persists per scope
//   Panels.clear();                        // drop the previous view's panels
//   const body = Panels.make({ id, title, x, y, w });
//   body.appendChild(...);
//
// Per-panel position/size/collapsed state persists to localStorage under
// (scope, id), so each module remembers its own layout across reloads.

let layer = null;
let scope = "";
const live = [];

const keyFor = (sc, id) => "ostcore-ui3:layout:" + sc + ":" + id;
function loadState(sc, id) {
  try { return JSON.parse(localStorage.getItem(keyFor(sc, id))) || {}; } catch (e) { return {}; }
}
function saveState(sc, id, st) {
  try { localStorage.setItem(keyFor(sc, id), JSON.stringify(st)); } catch (e) { /* ignore */ }
}

export function setLayer(el) { layer = el; }
export function setScope(s) { scope = s || ""; }

export function clear() {
  while (live.length) {
    const p = live.pop();
    try { if (typeof p._onClose === "function") p._onClose(); } catch (e) { /* ignore */ }
    p.remove();
  }
}

function layerRect() { return layer.getBoundingClientRect(); }

function startDrag(panel, bar, id, ev, sc) {
  ev.preventDefault();
  const lr = layerRect();
  const r = panel.getBoundingClientRect();
  const ox = ev.clientX - r.left, oy = ev.clientY - r.top;
  panel.classList.add("dragging");
  const move = (e) => {
    const m = 6;
    const x = Math.max(m, Math.min(e.clientX - lr.left - ox, lr.width - r.width - m));
    const y = Math.max(m, Math.min(e.clientY - lr.top - oy, lr.height - 36));
    panel.style.left = x + "px";
    panel.style.top = y + "px";
  };
  const up = () => {
    document.removeEventListener("mousemove", move);
    document.removeEventListener("mouseup", up);
    panel.classList.remove("dragging");
    const s = loadState(sc, id);
    s.x = parseInt(panel.style.left, 10);
    s.y = parseInt(panel.style.top, 10);
    saveState(sc, id, s);
  };
  document.addEventListener("mousemove", move);
  document.addEventListener("mouseup", up);
}

function startResize(panel, id, ev, sc) {
  ev.preventDefault();
  ev.stopPropagation();
  const sx = ev.clientX, sy = ev.clientY;
  const sw = panel.offsetWidth, sh = panel.offsetHeight;
  const lr = layerRect();
  panel.classList.add("resized", "resizing");
  const move = (e) => {
    panel.style.width = Math.max(220, Math.min(sw + (e.clientX - sx), lr.width - 12)) + "px";
    panel.style.height = Math.max(120, Math.min(sh + (e.clientY - sy), lr.height - 12)) + "px";
  };
  const up = () => {
    document.removeEventListener("mousemove", move);
    document.removeEventListener("mouseup", up);
    panel.classList.remove("resizing");
    const s = loadState(sc, id);
    s.w = parseInt(panel.style.width, 10);
    s.h = parseInt(panel.style.height, 10);
    saveState(sc, id, s);
  };
  document.addEventListener("mousemove", move);
  document.addEventListener("mouseup", up);
}

/**
 * Creates one floating panel and returns its body element (append content
 * to that, not to the panel itself). opts: { id, title, x, y, w, statusEl }.
 * `persistent: true` panels survive clear() (none used yet in UI3, kept for
 * parity with ostnoctua's always-on log panel — see README if one is added).
 */
export function make(opts) {
  const id = opts.id;
  const sc = opts.persistent ? (opts.scope || "__app") : scope;
  const st = loadState(sc, id);

  const panel = document.createElement("div");
  panel.className = "panel-float";
  panel.dataset.panel = id;
  panel.style.left = (st.x != null ? st.x : (opts.x != null ? opts.x : 20)) + "px";
  panel.style.top = (st.y != null ? st.y : (opts.y != null ? opts.y : 16)) + "px";
  if (st.w != null || opts.w) panel.style.width = (st.w != null ? st.w : opts.w) + "px";
  if (st.h != null) { panel.style.height = st.h + "px"; panel.classList.add("resized"); }
  if (st.collapsed) panel.classList.add("collapsed");

  const bar = document.createElement("div");
  bar.className = "panel-float-bar";
  const grip = document.createElement("span");
  grip.className = "panel-float-grip";
  grip.textContent = "⣿";
  const title = document.createElement("span");
  title.className = "panel-float-title";
  title.textContent = opts.title || id;
  const min = document.createElement("button");
  min.className = "panel-float-min";
  min.type = "button";
  min.textContent = "–";
  min.title = "Minimize";
  bar.appendChild(grip);
  bar.appendChild(title);
  bar.appendChild(min);
  panel.appendChild(bar);

  const body = document.createElement("div");
  body.className = "panel-float-body";
  panel.appendChild(body);

  const rz = document.createElement("div");
  rz.className = "panel-float-resize";
  rz.title = "Resize";
  rz.addEventListener("mousedown", (e) => startResize(panel, id, e, sc));
  panel.appendChild(rz);

  min.addEventListener("click", (e) => {
    e.stopPropagation();
    panel.classList.toggle("collapsed");
    const s = loadState(sc, id);
    s.collapsed = panel.classList.contains("collapsed");
    saveState(sc, id, s);
  });
  bar.addEventListener("mousedown", (e) => {
    if (e.target === min) return;
    startDrag(panel, bar, id, e, sc);
  });

  layer.appendChild(panel);
  if (!opts.persistent) live.push(panel);
  return body;
}
