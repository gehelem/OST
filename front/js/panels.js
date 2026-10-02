// panels.js — floating, draggable, minimizable, resizable panels.
//
// Direct adaptation of ostnoctua's panel.js (see ostnoctua/web/panel.js) to
// core/'s client API and UI3's design language. The layer (#panel-layer,
// see css/panels.css) fills #canvas edge to edge exactly like ostnoctua's
// own #on-layer does the viewport — #modbar/#conn-widget float on top of it
// (position:fixed) rather than living in a separate chrome strip above it,
// so drag/resize math here is relative to the layer's own bounding box
// (which is the same as the viewport minus nothing, in practice) rather
// than hard-coding `window.innerWidth/innerHeight` the way ostnoctua's
// original does.
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

// Stacking order is plain DOM order (no CSS z-index, see togglePanel()'s
// own comment on "default-stacking siblings") - `nextZ()` hands out a
// value guaranteed higher than anything persisted in a previous session
// (seeded from Date.now(), which only moves forward), so a freshly raised
// or freshly created panel always sorts above whatever was saved before,
// while relative order among untouched panels stays whatever it was.
let zCounter = Date.now();
function nextZ() { return ++zCounter; }

// Moves `panel` to the end of the layer's DOM (= front, see above) and
// persists the new order under (sc, id) so it survives a reload - Gilles,
// 2026-10-0X: "celle que je selectionne doit passer au premier plan ...
// + a conserver dans le localstorage". Called on ANY interaction with the
// panel (see make()'s own capture-phase listener below), not just the
// quicknav button's own show/hide.
function raisePanel(panel, id, sc) {
  const z = nextZ();
  panel.dataset.z = String(z);
  const s = loadState(sc, id);
  s.z = z;
  saveState(sc, id, s);
  layer.appendChild(panel);
}

// Inserts a freshly-made panel at its correct sorted position among
// already-live siblings instead of always appending last - otherwise a
// template's own Panels.make() call order would silently override
// whatever stacking the user had previously set and saved, every time the
// module's panels get rebuilt (e.g. on every property update).
function insertSorted(panel, z) {
  const next = [...layer.children].find((el) => el.classList.contains("panel-float") && Number(el.dataset.z) > z);
  if (next) layer.insertBefore(panel, next);
  else layer.appendChild(panel);
}

// #modbar/#top-right float at top:12px, and #modbar's own tallest content
// (.mod-btn, 54px) plus its 8px top+bottom padding makes it 70px tall, so
// its bottom edge sits at 12+70=82px in viewport (== layer) coordinates.
// A panel dragged with its title bar above this line ends up visually
// AND pointer-wise behind that fixed, higher-z-index chrome (clicks in
// that screen region hit the bar, not the panel) - "impossible de les
// deplacer" once stuck there. Enforced here, once, in the panel engine
// itself (drag clamp + make()'s own clamp, so an already-bad saved
// position self-heals on next render) rather than by tuning a handful of
// per-template default y values again, which is what led to this being a
// recurring bug across several rounds (60, then 70, both still not quite
// enough) - Gilles, 2026-09-27: "probleme qu'on a depuis le debut".
import { t } from "./i18n.js";

const SAFE_TOP = 92;

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

// Whoever cares whether a given panel is shown or hidden (views.js's
// renderQuickNav() buttons, via app.js's single listener) hears about it
// here, whenever Panels.togglePanel() changes it - decoupled from that
// caller rather than every interested party having to know about it
// directly.
function notifyVisibility(id, visible) {
  layer.dispatchEvent(new CustomEvent("panelvisibility", { detail: { id, visible } }));
}

// Gilles, 2026-10-03: "sur smartphone je ne peux pas deplacer les fenetres"
// - drag/resize only ever listened for mouse events; a touch's own
// mousedown/mousemove aren't reliably synthesized by mobile browsers (and
// even where tap registers, the drag itself never tracked). Same fix as
// sky-engine.js's own mouse+touch split (its _onMouseDown/_onTouchStart,
// see that file's own header) rather than switching to the Pointer Events
// API wholesale - one input model per file, consistent with the rest of
// this app. `touches[0]` on a TouchEvent, `clientX/clientY` directly on a
// MouseEvent - both land here, under one name the move/up handlers below
// read from without caring which kind of event it came from.
function pointXY(ev) {
  const t = ev.touches && ev.touches[0];
  return t ? { x: t.clientX, y: t.clientY } : { x: ev.clientX, y: ev.clientY };
}

function startDrag(panel, bar, id, ev, sc) {
  ev.preventDefault();
  const lr = layerRect();
  const r = panel.getBoundingClientRect();
  const start = pointXY(ev);
  const ox = start.x - r.left, oy = start.y - r.top;
  panel.classList.add("dragging");
  const move = (e) => {
    const p = pointXY(e);
    const m = 6;
    const x = Math.max(m, Math.min(p.x - lr.left - ox, lr.width - r.width - m));
    const y = Math.max(SAFE_TOP, Math.min(p.y - lr.top - oy, lr.height - 36));
    panel.style.left = x + "px";
    panel.style.top = y + "px";
    if (e.touches) e.preventDefault(); // stop the page itself from scrolling under the drag
  };
  const up = () => {
    document.removeEventListener("mousemove", move);
    document.removeEventListener("mouseup", up);
    document.removeEventListener("touchmove", move);
    document.removeEventListener("touchend", up);
    document.removeEventListener("touchcancel", up);
    panel.classList.remove("dragging");
    const s = loadState(sc, id);
    s.x = parseInt(panel.style.left, 10);
    s.y = parseInt(panel.style.top, 10);
    saveState(sc, id, s);
  };
  document.addEventListener("mousemove", move);
  document.addEventListener("mouseup", up);
  document.addEventListener("touchmove", move, { passive: false });
  document.addEventListener("touchend", up);
  document.addEventListener("touchcancel", up);
}

function startResize(panel, id, ev, sc) {
  ev.preventDefault();
  ev.stopPropagation();
  const start = pointXY(ev);
  const sx = start.x, sy = start.y;
  const sw = panel.offsetWidth, sh = panel.offsetHeight;
  const lr = layerRect();
  panel.classList.add("resized", "resizing");
  const move = (e) => {
    const p = pointXY(e);
    panel.style.width = Math.max(220, Math.min(sw + (p.x - sx), lr.width - 12)) + "px";
    panel.style.height = Math.max(120, Math.min(sh + (p.y - sy), lr.height - 12)) + "px";
    if (e.touches) e.preventDefault();
  };
  const up = () => {
    document.removeEventListener("mousemove", move);
    document.removeEventListener("mouseup", up);
    document.removeEventListener("touchmove", move);
    document.removeEventListener("touchend", up);
    document.removeEventListener("touchcancel", up);
    panel.classList.remove("resizing");
    const s = loadState(sc, id);
    s.w = parseInt(panel.style.width, 10);
    s.h = parseInt(panel.style.height, 10);
    saveState(sc, id, s);
  };
  document.addEventListener("mousemove", move);
  document.addEventListener("mouseup", up);
  document.addEventListener("touchmove", move, { passive: false });
  document.addEventListener("touchend", up);
  document.addEventListener("touchcancel", up);
}

/**
 * Creates one floating panel and returns its body element (append content
 * to that, not to the panel itself). opts: { id, title, x, y, w, statusEl,
 * collapsed }. `collapsed: true` is only the first-ever-render default
 * (e.g. a detail panel meant to start hidden behind its own quicknav
 * button) — the user's own later choice always overrides it afterwards.
 * `persistent: true` panels survive clear() - the always-on log panel
 * (views.js's renderLogPanel(), ostnoctua's own OstLog) is the first one.
 */
export function make(opts) {
  const id = opts.id;
  const sc = opts.persistent ? (opts.scope || "__app") : scope;
  const st = loadState(sc, id);

  const panel = document.createElement("div");
  panel.className = "panel-float";
  panel.dataset.panel = id;
  panel.dataset.scope = sc; // togglePanel()'s own lookup below needs this panel's own scope, not whatever scope happens to be active when it's later toggled
  panel.style.left = (st.x != null ? st.x : (opts.x != null ? opts.x : 20)) + "px";
  // Clamped even for an already-saved position: a layout saved from before
  // SAFE_TOP existed (or from before it was raised) would otherwise stay
  // stuck under the chrome forever - see this file's SAFE_TOP comment.
  panel.style.top = Math.max(SAFE_TOP, st.y != null ? st.y : (opts.y != null ? opts.y : 16)) + "px";
  if (st.w != null || opts.w) panel.style.width = (st.w != null ? st.w : opts.w) + "px";
  if (st.h != null) { panel.style.height = st.h + "px"; panel.classList.add("resized"); }
  // `opts.collapsed` is only the template's own first-ever-render default
  // (e.g. a detail panel that should start hidden until its quicknav
  // button is clicked) — once the user has actually toggled it, their own
  // choice (persisted as st.collapsed) always wins from then on.
  if (st.collapsed != null ? st.collapsed : opts.collapsed) panel.classList.add("collapsed");

  // Every panel always has a z, persisted the first time it's assigned
  // (same "stabilize once known" pattern x/y already follow) - see
  // nextZ()'s own comment on why a never-touched panel still needs one,
  // not just the ones the user has actually brought to front.
  const z = st.z != null ? st.z : nextZ();
  panel.dataset.z = String(z);
  if (st.z == null) { st.z = z; saveState(sc, id, st); }

  const bar = document.createElement("div");
  bar.className = "panel-float-bar";
  const grip = document.createElement("span");
  grip.className = "panel-float-grip";
  grip.textContent = "⣿";
  const title = document.createElement("span");
  title.className = "panel-float-title";
  title.textContent = opts.title || id;
  bar.appendChild(grip);
  bar.appendChild(title);
  panel.appendChild(bar);

  const body = document.createElement("div");
  body.className = "panel-float-body";
  panel.appendChild(body);

  const rz = document.createElement("div");
  rz.className = "panel-float-resize";
  rz.title = t("chrome.resize");
  rz.addEventListener("mousedown", (e) => startResize(panel, id, e, sc));
  rz.addEventListener("touchstart", (e) => startResize(panel, id, e, sc), { passive: false });
  panel.appendChild(rz);

  // No minimize button on the panel itself any more (Gilles, 2026-10-03:
  // "plus de passage en mode reduit des fenetres ... c'est le bouton de la
  // barre de menu qui affiche/masque la fenetre") - showing/hiding a panel
  // only ever happens through its own quicknav button (togglePanel(), same
  // "collapsed" class this used to toggle directly) from here on.
  bar.addEventListener("mousedown", (e) => startDrag(panel, bar, id, e, sc));
  bar.addEventListener("touchstart", (e) => startDrag(panel, bar, id, e, sc), { passive: false });

  // Any interaction anywhere on the panel brings it to front - Gilles,
  // 2026-10-0X: "celle que je selectionne doit passer au premier plan (la
  // elle reste derriere si elle a ete creee en premier)". Capture phase so
  // it fires regardless of what an inner control (a button, an input...)
  // does with the event afterwards.
  panel.addEventListener("mousedown", () => raisePanel(panel, id, sc), { capture: true });
  panel.addEventListener("touchstart", () => raisePanel(panel, id, sc), { capture: true, passive: true });

  insertSorted(panel, z);
  if (!opts.persistent) live.push(panel);
  return body;
}

// Looks a panel up by id directly in the layer's own DOM, not the `live`
// array - `live` only ever holds non-persistent panels (make()'s own
// `if (!opts.persistent) live.push(panel)`), so a lookup through it alone
// would never find the persistent log panel (views.js's renderLogPanel()).
function findPanel(id) {
  return layer.querySelector('.panel-float[data-panel="' + id + '"]');
}

/** True if the panel exists and isn't collapsed — the quicknav button's own
 * initial on/off state (views.js's renderQuickNav()) comes from this. */
export function isPanelVisible(id) {
  const panel = findPanel(id);
  return !!panel && !panel.classList.contains("collapsed");
}

/**
 * Shows or hides one panel by id (Gilles, 2026-09-27: the quicknav button
 * "doit masquer/demasquer la fenetre" and reflect which state it's in;
 * 2026-10-03: "c'est le bouton de la barre de menu qui affiche/masque la
 * fenetre" - now the ONLY way, a panel no longer has its own minimize
 * button) — toggles the panel's own "collapsed" class. Showing it also
 * brings it to front (raisePanel() - persisted, not just a one-off DOM
 * move) and scrolls it into view, with a brief flash so it's easy to
 * spot; hiding it is just the collapse, no extra effect needed. Returns
 * the new visible state. This is the one
 * piece of behavior every quicknav bar shares regardless of who built the
 * panel list — the generic level1-grouped renderer or a bespoke template —
 * so it lives here, once, in core (Gilles: "il est important de conserver
 * l'heritage core").
 */
export function togglePanel(id) {
  const panel = findPanel(id);
  if (!panel) return false;
  const willShow = panel.classList.contains("collapsed");
  panel.classList.toggle("collapsed", !willShow);
  // This panel's OWN scope (make()'s own dataset.scope, above) - not
  // whatever scope happens to be active right now. For a persistent panel
  // (always "__app") those can differ: toggling it while some module
  // happens to be on screen must not save its collapsed state under that
  // module's own scope, or it stops persisting consistently.
  const panelScope = panel.dataset.scope || scope;
  const s = loadState(panelScope, id);
  s.collapsed = !willShow;
  saveState(panelScope, id, s);
  if (willShow) {
    raisePanel(panel, id, panelScope);
    panel.scrollIntoView({ behavior: "smooth", block: "nearest", inline: "nearest" });
    panel.classList.remove("flash");
    void panel.offsetWidth; // restart the animation if toggled again quickly
    panel.classList.add("flash");
  }
  notifyVisibility(id, willShow);
  return willShow;
}

