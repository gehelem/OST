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
// panel (see make()'s own "click" bubble-phase listener below, and the
// drag bar/resize handle's own "mousedown" listeners), not just the
// quicknav button's own show/hide.
function raisePanel(panel, id, sc) {
  const z = nextZ();
  panel.dataset.z = String(z);
  const s = loadState(sc, id);
  s.z = z;
  saveState(sc, id, s);
  // `appendChild` on a node already in the tree is a real remove-then-
  // reinsert, even when it's already `layer`'s own child just moving to
  // the end - and that blurs a focused descendant regardless, confirmed
  // in isolation (a focused <input> loses focus the instant its own
  // ancestor panel gets reordered this way, even among the SAME
  // siblings). Caught live, Gilles 2026-10-06, navigator's Search panel:
  // "impossible de saisir ... rien du tout au toucher" - a tap focuses
  // the input (mousedown), then this file's own "click" handler fires
  // raisePanel() on the SAME tap, silently blurring it again before any
  // typing - or even the on-screen keyboard - has a chance to happen.
  // Restored explicitly right after the move: every panel with an
  // editable field in its body (not just Search) was equally exposed,
  // the DOM move itself is what breaks it, nothing specific to this one
  // panel's layout.
  const focused = document.activeElement;
  const hadFocus = panel.contains(focused);
  layer.appendChild(panel);
  if (hadFocus && document.activeElement !== focused) focused.focus();
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
// (.mod-btn, 64px - grew from 56px on 2026-10-16 to fit the loaded-profile
// name under the label, see panels.css's own .mod-btn-profile) plus its
// 8px top+bottom padding makes it 80px tall, so its bottom edge sits at
// 12+80=92px in viewport (== layer) coordinates. A panel dragged with its
// title bar above this line ends up visually AND pointer-wise behind that
// fixed, higher-z-index chrome (clicks in that screen region hit the bar,
// not the panel) - "impossible de les deplacer" once stuck there. Enforced
// here, once, in the panel engine itself (drag clamp + make()'s own clamp,
// so an already-bad saved position self-heals on next render) rather than
// by tuning a handful of per-template default y values again, which is
// what led to this being a recurring bug across several rounds (60, then
// 70, both still not quite enough) - Gilles, 2026-09-27: "probleme qu'on a
// depuis le debut". Kept a steady ~10px buffer below that edge across the
// .mod-btn height change above (92 -> 100).
import { t } from "./i18n.js";
import { icon } from "./dom.js";

const SAFE_TOP = 100;

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

// Gilles, 2026-10-16: "les fenetres ... arrivent souvent a se coincer sous
// [le] menu de chaque module en bas" - clampBottomIntoView() below already
// kept a freshly-made or just-reopened panel's bottom edge above #quicknav,
// but startDrag()'s own live move handler never knew about #quicknav at
// all (its Y clamp only ever used `lr.height - 36`, the raw layer bottom
// minus a small fixed margin) - dragging a panel down let its body slide
// most of the way UNDER the bar before that clamp even engaged, since it
// only bounds where the panel's TOP can go, never its bottom edge against
// the bar's own position. One shared "forbidden zone" height instead of
// duplicating the quicknav lookup in three places (make()'s initial clamp
// doesn't need it - SAFE_TOP already keeps a fresh panel well clear of the
// bottom bar at its own small default height). 1.05x the bar's own
// measured height (not a hardcoded px constant like SAFE_TOP) - Gilles's
// own margin, and #quicknav's height varies with however many icons
// wrapped onto it.
const BOTTOM_MARGIN_FACTOR = 1.05;
function bottomSafeLimit() {
  const quicknav = document.getElementById("quicknav");
  if (quicknav && !quicknav.classList.contains("hidden")) {
    const r = quicknav.getBoundingClientRect();
    return r.top - r.height * (BOTTOM_MARGIN_FACTOR - 1);
  }
  return layerRect().bottom;
}

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

// app.js's own applyZoom() scales the whole app via the real CSS `zoom`
// property (document.body.style.zoom), not a transform - which means
// layout reads (getBoundingClientRect(), offsetWidth, MouseEvent's own
// clientX/clientY) come back in real VIEWPORT pixels (already scaled),
// while style.left/top/width/height WRITES on an element inside that
// zoomed subtree are interpreted as LOCAL (pre-zoom) pixels and get
// re-multiplied by the zoom factor at render time. Mixing the two
// without converting was exactly Gilles's bug report, 2026-10-03: "des
// que je commence a deplacer ma souris la fenetre fait un saut ... puis
// se deplace a cote du curseur, decale - ca n'arrive pas au niveau de
// zoom par defaut" - a 1:1 (z=1) zoom hides the bug since both unit
// systems coincide there. Read fresh on every drag/resize start (not
// cached) since the zoom slider can change between one drag and the
// next. Same `--ui-zoom` custom property templates/navigator.js's own
// readUiZoom() already reads, for the same reason (that file needs it
// for label sizing, not duplicated as a shared import since it's two
// lines and this module stays dependency-free otherwise).
function uiZoom() {
  const raw = parseFloat(document.documentElement.style.getPropertyValue("--ui-zoom"));
  return raw > 0 ? raw : 1;
}

// Snap threshold in viewport pixels, independent of zoom (consistent with
// the `m`/SAFE_TOP margins in startDrag()'s own move handler, which are
// viewport-space too before their final /z conversion). Shared by both
// axes - left/right edges and top/bottom edges snap at the same distance.
const SNAP_THRESHOLD = 24;

// Picks, among the 4 ways one of [aStart, aEnd] could align with one of
// [bStart, bEnd], the smallest offset under `threshold` - or null if none
// of the 4 qualifies.
function bestSnapDelta(aStart, aEnd, bStart, bEnd, threshold) {
  let best = null;
  for (const d of [bStart - aStart, bEnd - aStart, bStart - aEnd, bEnd - aEnd]) {
    if (Math.abs(d) <= threshold && (best === null || Math.abs(d) < Math.abs(best))) best = d;
  }
  return best;
}

// Snaps a dragged panel's candidate position (vx, vy, viewport space) to
// the nearest edge of any other visible panel, on each axis independently.
function snapToOtherPanels(panel, vx, vy, w, h) {
  let bestDx = null, bestDy = null;
  for (const other of layer.querySelectorAll(".panel-float")) {
    if (other === panel || other.classList.contains("collapsed")) continue;
    const orect = other.getBoundingClientRect();
    const dx = bestSnapDelta(vx, vx + w, orect.left, orect.right, SNAP_THRESHOLD);
    if (dx != null && (bestDx == null || Math.abs(dx) < Math.abs(bestDx))) bestDx = dx;
    const dy = bestSnapDelta(vy, vy + h, orect.top, orect.bottom, SNAP_THRESHOLD);
    if (dy != null && (bestDy == null || Math.abs(dy) < Math.abs(bestDy))) bestDy = dy;
  }
  return { dx: bestDx || 0, dy: bestDy || 0 };
}

function startDrag(panel, bar, id, ev, sc) {
  ev.preventDefault();
  const z = uiZoom();
  const lr = layerRect();
  const r = panel.getBoundingClientRect();
  const bottomLimit = bottomSafeLimit(); // #quicknav doesn't move mid-drag, same one-shot capture as `lr`/`r`
  const start = pointXY(ev);
  const ox = start.x - r.left, oy = start.y - r.top;
  panel.classList.add("dragging");
  const move = (e) => {
    const p = pointXY(e);
    const m = 6;
    // Clamped in viewport space (lr/r are already real, post-zoom
    // pixels, consistent with each other) - only the FINAL result is
    // converted to local/pre-zoom space (uiZoom()'s own doc comment),
    // right before it's written into style.left/top.
    let vx = Math.max(m, Math.min(p.x - lr.left - ox, lr.width - r.width - m));
    // Clamps the panel's BOTTOM edge against bottomLimit (bottomLimit -
    // r.height), not just its top against the raw layer edge like before -
    // see bottomSafeLimit()'s own doc comment for why the old `lr.height -
    // 36` let a dragged panel's body slide most of the way under
    // #quicknav before this ever engaged.
    let vy = Math.max(SAFE_TOP, Math.min(p.y - lr.top - oy, bottomLimit - r.height));
    const snap = snapToOtherPanels(panel, vx, vy, r.width, r.height);
    vx += snap.dx;
    vy += snap.dy;
    const x = vx / z;
    const y = vy / z;
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
  const z = uiZoom();
  const start = pointXY(ev);
  const sx = start.x, sy = start.y;
  // getBoundingClientRect(), not offsetWidth/Height - stays consistently
  // in viewport space alongside sx/sy/lr below (same reasoning as
  // startDrag() above; uiZoom()'s own doc comment has the full story).
  const r0 = panel.getBoundingClientRect();
  const sw = r0.width, sh = r0.height;
  const lr = layerRect();
  // Same #quicknav-aware limit as startDrag() above, anchored to this
  // panel's own (fixed, during a pure resize) top edge instead of its
  // current bottom - growing a panel downward by dragging its resize
  // handle had the exact same "slides under the bar" gap as dragging its
  // title bar did (bottomSafeLimit()'s own doc comment).
  const maxHeight = bottomSafeLimit() - r0.top;
  panel.classList.add("resized", "resizing");
  const move = (e) => {
    const p = pointXY(e);
    // The 220/120 minimums are meant as LOCAL (CSS) pixels regardless of
    // zoom (same minimum panel size whatever the UI scale), so they're
    // scaled up to viewport space for this clamp, then the whole result
    // converted back to local space (/ z) right before the style write.
    panel.style.width = (Math.max(220 * z, Math.min(sw + (p.x - sx), lr.width - 12)) / z) + "px";
    panel.style.height = (Math.max(120 * z, Math.min(sh + (p.y - sy), maxHeight)) / z) + "px";
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
  // X clamped too, not just Y below - a position saved from a wide/desktop
  // session (e.g. localStorage synced via a browser account, or just the
  // same person switching screens) stayed exactly where it was saved
  // regardless of how narrow the *current* layer actually is, pushing the
  // panel - and whatever control inside it - off the right edge on a
  // phone with no obvious way back short of scrolling #panel-layer itself
  // (overflow:auto, but no visible affordance hinting that's needed).
  // Caught live, Gilles 2026-10-06: "sur le planetarium/navigator avec
  // l'application android ou en navigation web sur mon smartphone je
  // n'arrive pas a modifier le 'star catalog'" - the Settings panel's own
  // saved x (desktop session) left its star-catalog <select> unreachable
  // on a narrow viewport. Width guesses 220px when neither a saved nor a
  // requested width is known (this file's own smallest commonly-used
  // panel width) - better an approximate clamp than none.
  const panelW = st.w != null ? st.w : (opts.w || 220);
  const clampedX = Math.max(6, Math.min(st.x != null ? st.x : (opts.x != null ? opts.x : 20), layerRect().width - panelW - 6));
  panel.style.left = clampedX + "px";
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

  // Hides the panel exactly like clicking its own quicknav button would
  // (togglePanel(), same "collapsed" state and persistence) - a second,
  // closer-at-hand way to reach the one show/hide mechanism this app
  // already has, not a separate minimize concept.
  const closeBtn = document.createElement("button");
  closeBtn.type = "button";
  closeBtn.className = "panel-float-close";
  closeBtn.title = t("chrome.hide");
  closeBtn.appendChild(icon("close", { small: true }));
  closeBtn.addEventListener("mousedown", (e) => e.stopPropagation());
  closeBtn.addEventListener("touchstart", (e) => e.stopPropagation(), { passive: true });
  closeBtn.addEventListener("click", (e) => { e.stopPropagation(); togglePanel(id); });
  bar.appendChild(closeBtn);

  panel.appendChild(bar);

  const body = document.createElement("div");
  body.className = "panel-float-body";
  panel.appendChild(body);

  const rz = document.createElement("div");
  rz.className = "panel-float-resize";
  rz.title = t("chrome.resize");
  rz.addEventListener("mousedown", (e) => { raisePanel(panel, id, sc); startResize(panel, id, e, sc); });
  rz.addEventListener("touchstart", (e) => { raisePanel(panel, id, sc); startResize(panel, id, e, sc); }, { passive: false });
  panel.appendChild(rz);

  // No minimize button on the panel itself any more (Gilles, 2026-10-03:
  // "plus de passage en mode reduit des fenetres ... c'est le bouton de la
  // barre de menu qui affiche/masque la fenetre") - showing/hiding a panel
  // only ever happens through its own quicknav button (togglePanel(), same
  // "collapsed" class this used to toggle directly) from here on.
  bar.addEventListener("mousedown", (e) => { raisePanel(panel, id, sc); startDrag(panel, bar, id, e, sc); });
  bar.addEventListener("touchstart", (e) => { raisePanel(panel, id, sc); startDrag(panel, bar, id, e, sc); }, { passive: false });

  // Any interaction anywhere on the panel brings it to front - Gilles,
  // 2026-10-0X: "celle que je selectionne doit passer au premier plan (la
  // elle reste derriere si elle a ete creee en premier)". On "click"
  // (bubble phase, the default - fires AFTER an inner button/input's own
  // click handler already ran, target-then-ancestor order), NOT on
  // "mousedown" - a previous version raised on mousedown, which calls
  // layer.appendChild() (a real DOM reparent) before the button's own
  // click could even fire. Verified live (Playwright + a real dump) that
  // this silently broke EVERY button inside EVERY panel: moving the
  // pressed element out from under the cursor mid-gesture stops the
  // browser from ever synthesizing the "click" at all, not just this
  // one - the actual cause of "j'appuis sur le bouton shoot ... il ne se
  // passe rien" (2026-10-0X), a regression from this same feature, not
  // the panel-overlap bug fixed alongside it in templates/inspector.js.
  // The drag bar/resize handle above still raise instantly on their own
  // mousedown instead - neither is a button with its own click handler
  // to protect, so there's nothing to break.
  // A native <select>'s own dropdown is a one-shot OS-level action tied to
  // this exact click - raising right now would reparent the panel
  // (raisePanel's appendChild) while that's still pending and silently
  // cancel it on Android WebView, confirmed live: the select never opens.
  // Unlike the focused-<input> case above, there's no after-the-fact fix
  // (focus can be restored, a cancelled native popup can't) - so for a
  // select the raise itself is deferred until it settles (change or
  // blur), once reparenting is safe again.
  panel.addEventListener("click", (ev) => {
    const sel = ev.target.closest && ev.target.closest("select");
    if (sel) {
      const raiseOnce = () => {
        sel.removeEventListener("change", raiseOnce);
        sel.removeEventListener("blur", raiseOnce);
        raisePanel(panel, id, sc);
      };
      sel.addEventListener("change", raiseOnce);
      sel.addEventListener("blur", raiseOnce);
      return;
    }
    raisePanel(panel, id, sc);
  });

  insertSorted(panel, z);
  if (!opts.persistent) live.push(panel);

  // Y is only floored above (SAFE_TOP), never capped to the bottom edge -
  // a panel whose own natural height pushes it past the bottom of a short
  // viewport stays completely unreachable below the fold, with
  // #panel-layer's own overflow:auto giving no visible hint that
  // scrolling would reveal it. Can't be clamped synchronously like X
  // above: the caller appends its own content into `body` AFTER make()
  // returns, so the panel's real height isn't known yet here, AND a
  // panel that starts `collapsed` has no real height at all yet either
  // (display:none) - clampBottomIntoView() runs again from
  // togglePanel() below for exactly that second case, once the panel
  // actually becomes visible and has a real height to measure. Caught
  // live, Gilles 2026-10-06, navigator's new Search panel (y:580) on a
  // phone: "impossible de saisir ... rien du tout au toucher" - exactly
  // what tapping empty space below the fold (or a `collapsed` panel's
  // own zero-height rect) looks like.
  requestAnimationFrame(() => clampBottomIntoView(panel));

  return body;
}

/**
 * Nudges `panel` up just enough that its bottom edge stays reachable -
 * above `#quicknav` (position:fixed, z-index 20 - ABOVE #panel-layer's
 * own 5, so a panel merely clamped to the layer's raw bottom edge can
 * still end up genuinely unclickable UNDER it, confirmed live: the
 * layer-edge-only version of this fix correctly moved a panel's bottom
 * onto the viewport edge, but tapping where its now-visible input sat
 * actually focused a quicknav button instead) when it's shown, or
 * #panel-layer's own bottom edge otherwise. Measured live every time
 * (not a hardcoded constant alongside SAFE_TOP), since #quicknav's own
 * height varies with however many icons wrapped onto it, and is hidden
 * entirely on non-module screens. Same SAFE_TOP floor as the initial
 * X/Y clamp in make() so this never pushes a panel back up above it
 * either. Called once from make() itself (via requestAnimationFrame,
 * once every panel this build() pass appended its own content has had a
 * chance to), and again from togglePanel() whenever a `collapsed` panel
 * is revealed (make()'s own call measured nothing useful then - a
 * collapsed panel has no real height at all). Shares bottomSafeLimit()
 * with startDrag()/startResize() above - the live-drag/resize gap that
 * was letting a panel's body slide under #quicknav despite this
 * function already existing, see that helper's own doc comment.
 */
function clampBottomIntoView(panel) {
  const bottomLimit = bottomSafeLimit();
  const overflow = panel.getBoundingClientRect().bottom - bottomLimit;
  if (overflow > 0) {
    const top = parseFloat(panel.style.top) || 0;
    panel.style.top = Math.max(SAFE_TOP, top - overflow) + "px";
  }
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
    // make()'s own clampBottomIntoView() call ran while this panel was
    // still `collapsed` (no real height to measure yet) - redo it now
    // that it actually has one. See that function's own doc comment.
    requestAnimationFrame(() => clampBottomIntoView(panel));
  }
  notifyVisibility(id, willShow);
  return willShow;
}

