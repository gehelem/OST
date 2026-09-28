// UI3/js/app.js — wires core/ (connection + datastore, unmodified) to the
// render/views/panels/templates layers. Same app-level responsibilities as
// UI2/js/app.js (which screen is active, the module bar, the connect/login
// flow) — see that file's header for why the dispatcher-per-message-type
// pattern is gone: each screen subscribes directly to the core topic(s) it
// needs instead. UI3-specific: a module renders as floating panels on a
// shared canvas instead of a single docked node — see renderContentNow().

import { createClient, deriveDefaultUrls } from "../core/index.js";
import { renderModuleView, buildParamsPanel, setMediaBaseUrl, setDialogRoot, setQuickNavRoot } from "./render.js";
import { renderModbar, renderSystemViews, renderController, renderFiles, renderLogs, renderLogin } from "./views.js";
import { el, icon, iconButton } from "./dom.js";
import * as Panels from "./panels.js";
import { setBgLayer } from "./imagelayer.js";
import { templates } from "./templates/index.js";

const client = createClient();

const modbarEl = document.getElementById("modbar");
const systemViewsEl = document.getElementById("system-views");
const panelLayerEl = document.getElementById("panel-layer");
const dockedViewEl = document.getElementById("docked-view");
const quicknavEl = document.getElementById("quicknav");
const quicknavNavEl = document.getElementById("quicknav-nav");
const quicknavSepEl = document.getElementById("quicknav-sep");
Panels.setLayer(panelLayerEl);
setBgLayer(document.getElementById("bg-layer"));
setQuickNavRoot(quicknavNavEl, quicknavSepEl);
// One listener, registered once (not per-render, unlike the quicknav
// buttons themselves which get rebuilt on every module change) — keeps a
// quicknav button's "active" state in sync when a panel's visibility
// changes some other way (its own minimize button, or the reorganize
// button), not just when the quicknav button itself was clicked.
panelLayerEl.addEventListener("panelvisibility", (ev) => {
  const btn = quicknavNavEl.querySelector(`[data-panel-id="${ev.detail.id}"]`);
  if (btn) btn.classList.toggle("active", ev.detail.visible);
});
const connWidgetEl = document.getElementById("conn-widget");
const connToggleEl = document.getElementById("conn-toggle");
const connDotEl = document.getElementById("conn-dot");
const connSummaryEl = document.getElementById("conn-summary");
const connPanelEl = document.getElementById("conn-panel");
const grantPill = document.getElementById("grant-pill");
const wsUrlInput = document.getElementById("ws-url");
const mediaUrlInput = document.getElementById("media-url");
const themeToggle = document.getElementById("theme-toggle");
const paramsToggleEl = document.getElementById("params-toggle");
const reorganizeToggleEl = document.getElementById("reorganize-toggle");
const dialogRootEl = document.getElementById("dialog-root");

// ---- connection widget: collapsed status pill, click (or connecting/
// disconnected) to fold out the full settings — Gilles, 2026-09-25: "settings
// general et connexion en haut a droite (version minimale si connexion OK,
// clic dessus ca la deplie)".
function setConnPanelOpen(open) {
  connPanelEl.classList.toggle("hidden", !open);
  connWidgetEl.classList.toggle("open", open);
}
connToggleEl.addEventListener("click", () => setConnPanelOpen(connPanelEl.classList.contains("hidden")));
document.addEventListener("click", (ev) => {
  if (!connPanelEl.classList.contains("hidden") && !connWidgetEl.contains(ev.target)) setConnPanelOpen(false);
});
setConnPanelOpen(true); // nothing connected yet at boot: start open so the fields are reachable

setDialogRoot(document.getElementById("dialog-root"));

let activeView = null; // module name, or "__controller"/"__files"/"__logs", or null
let contentDispose = null; // disposes whatever subscriptions the current content view opened
let loginShown = false;
// Declared up here, not just above renderModbarNow() itself: client.onStatus()
// below fires its callback synchronously on registration (like subscribe()),
// which calls renderModbarNow() immediately for the initial "disconnected"
// status — a `let` declared any later would still be in its temporal dead
// zone at that point, throwing "Cannot access before initialization" and
// aborting the whole script (caught live, 2026-09-27: this silently broke
// the entire app — empty modbar, connect doing nothing — since every
// subsequent status change re-hit the same dead binding, swallowed by
// connection.js's own try/catch around each status listener).
let modbarDispose = null;

// ---- derive defaults per structure.en.md's "Finding the server" table ---

function loadHistory() {
  try { return JSON.parse(localStorage.getItem("ostcore-ui3.history") || "[]"); } catch (e) { return []; }
}
function saveHistory(entry) {
  const hist = loadHistory().filter((h) => h.wsUrl !== entry.wsUrl);
  hist.unshift(entry);
  try { localStorage.setItem("ostcore-ui3.history", JSON.stringify(hist.slice(0, 10))); } catch (e) { /* ignore */ }
}

// The active view survives a reload (Gilles, 2026-09-27: "apres un f5 je ne
// reviens pas sur le module sur lequel j'etais ... il faudrait que ce soit
// remanent") — persisted on every real selectView() call, restored once the
// post-reconnect dump lands (see the session subscription below), but only
// if it still resolves to something real: a module name that's no longer
// loaded falls back to Controller, same as never having a saved view at all.
function loadLastView() {
  try { return localStorage.getItem("ostcore-ui3.lastview") || null; } catch (e) { return null; }
}
function saveLastView(key) {
  try {
    if (key) localStorage.setItem("ostcore-ui3.lastview", key);
    else localStorage.removeItem("ostcore-ui3.lastview");
  } catch (e) { /* ignore */ }
}

const hist = loadHistory();
const defaults = deriveDefaultUrls(window.location);
wsUrlInput.value = (hist[0] && hist[0].wsUrl) || defaults.wsUrl;
mediaUrlInput.value = (hist[0] && hist[0].mediaUrl) || defaults.mediaUrl;
setMediaBaseUrl(mediaUrlInput.value);

// ---- theme -----------------------------------------------------------
// Manual dark/light toggle, persisted — client-pattern.en.md's dark-mode
// recommendation is honored by *always* having a fully dark default and a
// complete light alternative (css/theme.css), never by forcing one on the
// user; see also base rule carried over from the frontend CLAUDE.md
// ("CSS variables only, never hardcoded colors").

function applyTheme(t) {
  document.documentElement.setAttribute("data-theme", t);
  themeToggle.innerHTML = "";
  themeToggle.appendChild(icon(t === "light" ? "wb_sunny" : "brightness_2"));
  themeToggle.title = t === "light" ? "Switch to dark theme" : "Switch to light theme";
}
const themeStored = localStorage.getItem("ostcore-ui3.theme") || "dark";
applyTheme(themeStored);
themeToggle.addEventListener("click", () => {
  const cur = document.documentElement.getAttribute("data-theme") === "light" ? "dark" : "light";
  applyTheme(cur);
  localStorage.setItem("ostcore-ui3.theme", cur);
});

// ---- UI zoom -----------------------------------------------------------
// Font size (and, via a real browser-level `zoom` rather than a transform,
// everything else along with it — panel sizes included) is adjustable from
// two small +/- chips, persisted like the theme — Gilles, 2026-09-28: "deux
// boutons +/- ... ajuster la taille de police de tous les composants ...
// la taille des fenetres doit s'ajuster en consequence". `zoom` on <body>
// covers dialogs (#dialog-root, a sibling of #app) too, not just #app.
const ZOOM_MIN = 0.7, ZOOM_MAX = 1.6, ZOOM_STEP = 0.1;
function applyZoom(z) {
  z = Math.round(Math.max(ZOOM_MIN, Math.min(ZOOM_MAX, z)) * 100) / 100;
  document.body.style.zoom = z;
  return z;
}
let zoomLevel = applyZoom(parseFloat(localStorage.getItem("ostcore-ui3.zoom")) || 1);
function bumpZoom(delta) {
  zoomLevel = applyZoom(zoomLevel + delta);
  try { localStorage.setItem("ostcore-ui3.zoom", zoomLevel); } catch (e) { /* ignore */ }
}
document.getElementById("zoom-in").addEventListener("click", () => bumpZoom(ZOOM_STEP));
document.getElementById("zoom-out").addEventListener("click", () => bumpZoom(-ZOOM_STEP));

// ---- parameters modal --------------------------------------------------
// The profile-centric "Parameters" box (render.js's buildParamsPanel) is
// not one of the module's floating panels — it opens on demand as a
// focused modal instead, per Gilles, 2026-09-26: "cette boite ... ne doit
// pas etre affichee par defaut ... quand on affiche la boite, elle doit
// venir au dessus du reste, elle prend le focus ... et prendre autant de
// place que necessaire". .dialog-overlay (components.css) already sits
// above every floating panel/chip (z-index:50 vs. their 20-30) and closes
// on a backdrop click, same as the grid create/edit dialogs render.js
// already uses — reused here rather than inventing a second modal pattern.

function isModuleView() {
  return activeView && activeView !== "__controller" && activeView !== "__files" && activeView !== "__logs";
}

function updateParamsToggle() {
  const mod = isModuleView() && client.get(client.topics.module(activeView));
  const hasParams = !!(mod && Object.values(mod.properties).some((p) => p.hasprofile === true));
  paramsToggleEl.disabled = !hasParams;
  paramsToggleEl.title = hasParams ? "Parameters" : "Parameters (this module has none)";
  // Small red dot when the active profile has unsaved changes (module.f.changed
  // on the wire) — Gilles, 2026-09-26, so it's visible without opening the
  // modal. Recomputed on every topics.module(name) emission, which already
  // fires on profile.changed flips (see render.js's module doc comment).
  paramsToggleEl.classList.toggle("has-changes", !!(mod && mod.profile && mod.profile.changed));
}

// Only meaningful once a module (with its own floating panels) is active —
// Controller/Files/Logs/the connect screen don't have any.
function updateReorganizeToggle() {
  reorganizeToggleEl.disabled = !isModuleView();
}
reorganizeToggleEl.addEventListener("click", () => { if (!reorganizeToggleEl.disabled) Panels.reorganizeStacked(); });

function openParamsDialog() {
  const moduleName = activeView;
  const mod = isModuleView() && client.get(client.topics.module(moduleName));
  if (!mod) return;

  let disposers = [];
  const track = (node) => { disposers.push(node._dispose); return node; };

  const overlay = el("div", { class: "dialog-overlay" });
  const dialog = el("div", { class: "dialog params-dialog" });
  const close = () => { while (disposers.length) disposers.pop()(); overlay.remove(); };
  dialog.appendChild(el("div", { class: "dialog-header" }, [
    el("h3", { text: "Parameters — " + (mod.infos.modulelabel || mod.infos.label || moduleName) }),
    iconButton("close", "Close", close),
  ]));
  const body = el("div");
  dialog.appendChild(body);
  buildParamsPanel(client, moduleName, mod, track, body);

  overlay.appendChild(dialog);
  overlay.addEventListener("click", (ev) => { if (ev.target === overlay) close(); });
  dialogRootEl.appendChild(overlay);
}

paramsToggleEl.addEventListener("click", () => { if (!paramsToggleEl.disabled) openParamsDialog(); });

// ---- connection controls --------------------------------------------------

document.getElementById("connect-btn").addEventListener("click", () => {
  setMediaBaseUrl(mediaUrlInput.value);
  saveHistory({ wsUrl: wsUrlInput.value, mediaUrl: mediaUrlInput.value });
  loginShown = false;
  activeView = null;
  client.connect(wsUrlInput.value, { language: document.getElementById("lang-select").value });
});
document.getElementById("disconnect-btn").addEventListener("click", () => client.disconnect());
document.getElementById("lang-select").addEventListener("change", (e) => {
  if (client.connection.status === "connected") client.setLanguage(e.target.value);
});

client.onStatus((s) => {
  connDotEl.className = "conn-dot " + s;
  connSummaryEl.textContent = s.charAt(0).toUpperCase() + s.slice(1);
  if (s !== "connected") grantPill.textContent = "";
  // Minimal pill once connected; folded open again the moment there's
  // something the user needs to look at or act on.
  setConnPanelOpen(s !== "connected");
  if (s === "connecting" || s === "disconnected") { activeView = null; renderModbarNow(); renderContentNow(); }
});

// ---- rendering -------------------------------------------------------

function renderModbarNow() {
  if (modbarDispose) { modbarDispose(); modbarDispose = null; }
  modbarEl.innerHTML = "";
  const modbar = renderModbar(client, activeView, selectView);
  modbarEl.appendChild(modbar.node);
  modbarDispose = modbar.dispose;
  systemViewsEl.innerHTML = "";
  systemViewsEl.appendChild(renderSystemViews(activeView, selectView));
}

// Two mutually-exclusive content modes: a module renders as a set of
// floating panels directly on #panel-layer (see render.js/panels.js — no
// single node comes back for those); Controller/Files/Logs/the connect
// screen are conventional "docked" full-width views appended to
// #docked-view. Only one of the two containers is shown at a time.
function renderContentNow() {
  if (contentDispose) { contentDispose(); contentDispose = null; }
  Panels.clear();
  dockedViewEl.innerHTML = "";

  const isModule = isModuleView();
  panelLayerEl.classList.toggle("hidden", !isModule);
  dockedViewEl.classList.toggle("hidden", isModule);
  // #quicknav's own visibility follows the module view directly, not
  // whether it has nav entries to show — the actions zone (reorganize/
  // params) is still relevant for a module with zero level1 groups.
  quicknavEl.classList.toggle("hidden", !isModule);
  updateParamsToggle();
  updateReorganizeToggle();

  if (!activeView) { dockedViewEl.appendChild(renderConnectScreen()); return; }
  if (isModule) {
    // A bespoke template (templates/index.js) wins over the generic,
    // level1-grouped renderer when infos.template names one we have —
    // see templates/index.js's header for the fallback-safety note.
    const mod = client.get(client.topics.module(activeView));
    const bespoke = mod && templates[mod.infos.template];
    const result = bespoke ? bespoke.mount(client, activeView) : renderModuleView(client, activeView);
    // Keeps the gear button's "unsaved changes" dot live while this same
    // module stays active (updateParamsToggle() already ran once above,
    // this covers every subsequent profile.changed flip without switching
    // views) — client.subscribe() fires immediately too, harmless overlap.
    const unsubParamsWatch = client.subscribe(client.topics.module(activeView), updateParamsToggle);
    contentDispose = () => { result.dispose(); unsubParamsWatch(); };
    return;
  }

  let result;
  if (activeView === "__controller") result = renderController(client);
  else if (activeView === "__files") result = renderFiles(client, mediaUrlInput.value);
  else result = renderLogs(client);
  dockedViewEl.appendChild(result.node);
  contentDispose = result.dispose || null;
}

function renderConnectScreen() {
  const div = el("div", { class: "connect-screen" });
  div.appendChild(icon("stars", { small: false }));
  div.querySelector(".material-icons").classList.add("icon-lg");
  div.appendChild(el("h2", { text: "OST — UI3" }));
  div.appendChild(el("p", { class: "lede", text: "Not connected. Set the WebSocket URL above and press Connect." }));
  const panel = el("div", { class: "panel" }, [el("h3", { text: "Recent servers" })]);
  const ul = el("ul");
  for (const h of loadHistory()) {
    const li = el("li");
    const btn = el("button", { class: "btn spacer", text: h.wsUrl });
    btn.addEventListener("click", () => { wsUrlInput.value = h.wsUrl; mediaUrlInput.value = h.mediaUrl; document.getElementById("connect-btn").click(); });
    li.appendChild(btn);
    ul.appendChild(li);
  }
  if (!loadHistory().length) ul.appendChild(el("li", { class: "muted small", text: "No history yet." }));
  panel.appendChild(ul);
  div.appendChild(panel);
  return div;
}

function selectView(key) {
  activeView = key;
  saveLastView(key);
  // Picking Controller/Files/Logs from the settings dropdown should close
  // it back down, same as picking any other destination would.
  if (key === "__controller" || key === "__files" || key === "__logs") setConnPanelOpen(false);
  renderModbarNow();
  renderContentNow();
}

// ---- app-level topic subscriptions --------------------------------------
// Only truly cross-cutting concerns live here: the module bar (needs the
// module list), the login flow (needs session/grant), and "the active
// module just disappeared" (also the module list). Everything else is the
// concern of whichever screen is currently rendered — see render.js/views.js.

client.subscribe(client.topics.modules(), () => {
  // The module bar always needs the fresh module list. The active module
  // view does NOT need a poke here: renderModuleView (render.js) already
  // subscribes to topics.module(name) directly and renders its own
  // "not found" state when a module is removed — see its module doc.
  renderModbarNow();
});

client.subscribe(client.topics.session(), (session) => {
  if (session.grantClient === null) { loginShown = false; return; }
  grantPill.textContent = `grant: client=${session.grantClient} server=${session.grantServer}`;
  if (session.grantClient === "-1" && !loginShown) { loginShown = true; renderLogin(client); }
  else if (!activeView) {
    // First dump after connecting and no view chosen yet: restore whatever
    // was on screen before the last reload/disconnect, if it still resolves
    // to something real — a saved module name that's no longer loaded (or
    // no saved view at all, e.g. the very first visit) falls back to
    // Controller, same as before this existed.
    const last = loadLastView();
    const stillThere = last === "__controller" || last === "__files" || last === "__logs" || client.get(client.topics.modules()).includes(last);
    selectView(stillThere ? last : "__controller");
  }
});

renderModbarNow();
renderContentNow();

// Auto-connect on load/reload, using whatever URL is already filled in
// (last-used from history, or the derived default) — Gilles, 2026-09-25:
// "apres un f5 ou au premier chargement de la page on devrait tenter de se
// connecter". Reuses the connect button's own handler (click()) rather than
// duplicating its logic.
document.getElementById("connect-btn").click();
