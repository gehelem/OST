// UI3/js/app.js — wires core/ (connection + datastore, unmodified) to the
// render/views/panels/templates layers. Same app-level responsibilities as
// UI2/js/app.js (which screen is active, the module bar, the connect/login
// flow) — see that file's header for why the dispatcher-per-message-type
// pattern is gone: each screen subscribes directly to the core topic(s) it
// needs instead. UI3-specific: a module renders as floating panels on a
// shared canvas instead of a single docked node — see renderContentNow().

import { createClient, deriveUrlsFromHost, cleanHost, normalizeMediaUrl } from "../core/index.js";
import { renderModuleView, buildParamsPanel, setMediaBaseUrl, getMediaBaseUrl, setDialogRoot, setQuickNavRoot, getDisableTemplates } from "./render.js";
import { renderModbar, renderSystemViews, renderHome, renderController, renderFiles, renderLogs, renderLogin, renderLogPanel } from "./views.js";
import { openSetupWizard } from "./wizard.js";
import { openCreditsDialog } from "./credits.js";
import { el, icon, iconButton } from "./dom.js";
import * as Panels from "./panels.js";
import { setBgLayer } from "./imagelayer.js";
import { templates } from "./templates/index.js";
import { setLanguage, t } from "./i18n.js";
import * as zeroconf from "./zeroconf.js";
import { showToast } from "./toast.js";

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
// changes some other way, not just when the quicknav button itself was
// clicked.
panelLayerEl.addEventListener("panelvisibility", (ev) => {
  const btn = quicknavNavEl.querySelector(`[data-panel-id="${ev.detail.id}"]`);
  if (btn) btn.classList.toggle("active", ev.detail.visible);
});
// #panel-layer used to silently swallow every pointer event aimed at empty
// canvas space (no panel there) before it could reach whatever's behind it
// (#bg-image's click-to-viewer, then the sky-dome's own wheel-zoom/drag —
// same root cause, just surfacing under a different name each time: first
// "le clic sur l'image de fond ne marche pas", Gilles 2026-09-29). Fixed
// once, generically, in panels.css instead of forwarding one event type at
// a time (click here, then wheel, then mousedown/mousemove for the sky
// drag...): #panel-layer itself is pointer-events:none, only .panel-float
// (and everything inside it) opts back into pointer-events:auto — see
// panels.css's own comment for why that doesn't cost #panel-layer's
// wheel-scroll capability (togglePanel()'s scrollIntoView() is the actual
// established way to reach an off-screen panel, not manual scrolling).
const connWidgetEl = document.getElementById("conn-widget");
const connToggleEl = document.getElementById("conn-toggle");
const connPanelEl = document.getElementById("conn-panel");
const grantPill = document.getElementById("grant-pill");
const serverHostInput = document.getElementById("server-host");
const serverSslToggleEl = document.getElementById("server-ssl-toggle");
const serverSslLabelEl = document.getElementById("server-ssl-label");
const themeToggle = document.getElementById("theme-toggle");
const loginToggleEl = document.getElementById("login-toggle");
const logToggleEl = document.getElementById("log-toggle");
const logBadgeDotEl = document.getElementById("log-badge-dot");
const paramsToggleEl = document.getElementById("params-toggle");
const dialogRootEl = document.getElementById("dialog-root");
const connectBtnLabelEl = document.getElementById("connect-btn-label");
const disconnectBtnLabelEl = document.getElementById("disconnect-btn-label");
const versionInfoEl = document.getElementById("version-info");
const versionInfoTagEl = document.getElementById("version-info-tag");
const versionInfoMessageEl = document.getElementById("version-info-message");
const clearCacheBtnEl = document.getElementById("clear-cache-btn");
const clearCacheBtnLabelEl = document.getElementById("clear-cache-btn-label");
const discoverRowEl = document.getElementById("discover-row");
const discoverBtnEl = document.getElementById("discover-btn");
const discoverBtnLabelEl = document.getElementById("discover-btn-label");
const discoverResultsEl = document.getElementById("discover-results");
const setupWizardBtnEl = document.getElementById("setup-wizard-btn");
const setupWizardBtnLabelEl = document.getElementById("setup-wizard-btn-label");
const creditsBtnEl = document.getElementById("credits-btn");
const creditsBtnLabelEl = document.getElementById("credits-btn-label");

// Static chrome (this file's own top-right icon chips + the connect bar) is
// never rebuilt from a server dump the way a module's panels are, so a
// language change needs to touch it explicitly — called once at boot and
// again every time the language actually changes (connect, or the
// lang-select's own change handler below). Everything else UI3-local
// (module panels, system views, login...) is rebuilt from client-driven
// events that already run after setLanguage() has updated i18n.js's
// current language, so those pick it up for free.
function applyStaticLabels() {
  paramsToggleEl.title = t("chrome.parameters");
  zoomToggleEl.title = t("chrome.uiSize");
  themeToggle.title = t("chrome.toggleTheme");
  loginToggleEl.title = t("chrome.login");
  logToggleEl.title = t("chrome.logs");
  connectBtnLabelEl.textContent = t("chrome.connect");
  disconnectBtnLabelEl.textContent = t("chrome.disconnect");
  clearCacheBtnLabelEl.textContent = t("chrome.clearCache");
  creditsBtnLabelEl.textContent = t("credits.title");
  discoverBtnLabelEl.textContent = t("chrome.discover");
  connToggleEl.title = t("status." + client.connection.status);
  setupWizardBtnLabelEl.textContent = t("wizard.title");
}

// A button, not a checkbox (render.js's own boolean-element convention,
// Gilles 2026-09-27: "il faut mettre des boutons plutot que des puces ou
// des cases a cocher", reused here for the new SSL toggle - see
// index.html's own comment on replacing the old #ws-url/#media-url pair).
function sslOn() { return serverSslToggleEl.classList.contains("active"); }
function setSsl(on) {
  serverSslToggleEl.classList.toggle("active", on);
  serverSslToggleEl.querySelector(".material-icons").textContent = on ? "lock" : "lock_open";
}
serverSslToggleEl.addEventListener("click", () => setSsl(!sslOn()));

// ---- connection widget: collapsed status pill, click (or connecting/
// disconnected) to fold out the full settings — Gilles, 2026-09-25: "settings
// general et connexion en haut a droite (version minimale si connexion OK,
// clic dessus ca la deplie)".
function setConnPanelOpen(open) {
  connPanelEl.classList.toggle("hidden", !open);
  connWidgetEl.classList.toggle("open", open);
}
connToggleEl.addEventListener("click", () => setConnPanelOpen(connPanelEl.classList.contains("hidden")));
loginToggleEl.addEventListener("click", () => renderLogin(client));
// Gilles, 2026-10-03: "quand on clique depuis un module ca selectionne
// les messages du module" - isModuleView()/activeView defined further
// down (function hoisting - this only runs on an actual click, long after
// the whole module has loaded, so the forward reference is fine).
logToggleEl.addEventListener("click", () => renderLogPanel(client, isModuleView() ? activeView : null));
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
  const hist = loadHistory().filter((h) => h.host !== entry.host);
  hist.unshift(entry);
  try { localStorage.setItem("ostcore-ui3.history", JSON.stringify(hist.slice(0, 10))); } catch (e) { /* ignore */ }
}
// Gilles, 2026-10-06: "ajoute un bouton sur chaque ligne de la liste des
// url connues pour supprimer la ligne si besoin" - renderConnectScreen()'s
// own "Recent servers" list, below.
function removeHistory(host) {
  const hist = loadHistory().filter((h) => h.host !== host);
  try { localStorage.setItem("ostcore-ui3.history", JSON.stringify(hist)); } catch (e) { /* ignore */ }
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
if (hist[0]) {
  serverHostInput.value = hist[0].host;
  setSsl(!!hist[0].ssl);
} else if (!zeroconf.isAvailable()) {
  // Web: auto-init from the browser's own URL (Gilles, 2026-10-06: "l'URL
  // doit etre initialise automatiquement sur la version web, avec l'URL
  // du navigateur"). The Android app has no page URL to derive anything
  // from at all - "pour l'app android on est oblige de saisir ou
  // discover" - left empty there, same as before this change.
  serverHostInput.value = window.location.hostname || "127.0.0.1";
  setSsl(window.location.protocol === "https:");
} else {
  setSsl(false);
}
setMediaBaseUrl(normalizeMediaUrl(deriveUrlsFromHost(serverHostInput.value, sslOn()).mediaUrl));

// ---- theme -----------------------------------------------------------
// Manual light/dark/red cycle, persisted — client-pattern.en.md's dark-mode
// recommendation is honored by *always* having a fully dark default and a
// complete light alternative (css/theme.css), never by forcing one on the
// user; see also base rule carried over from the frontend CLAUDE.md
// ("CSS variables only, never hardcoded colors"). "red" (Gilles,
// 2026-10-05: "identique au sombre, mais avec nuances rouges plutot que
// vertes") is a real night-vision screen for an observing session — same
// dark structure as "dark", red accent instead of cyan/green, see
// css/theme.css's own [data-theme="red"] block.
const THEMES = ["light", "dark", "red"];
const THEME_ICON = { light: "wb_sunny", dark: "brightness_2", red: "nightlight" };
const THEME_NEXT_TITLE = { light: "Switch to dark theme", dark: "Switch to red (night-vision) theme", red: "Switch to light theme" };

function applyTheme(t) {
  document.documentElement.setAttribute("data-theme", t);
  themeToggle.innerHTML = "";
  themeToggle.appendChild(icon(THEME_ICON[t] || THEME_ICON.dark));
  themeToggle.title = THEME_NEXT_TITLE[t] || THEME_NEXT_TITLE.dark;
}
const themeStored = localStorage.getItem("ostcore-ui3.theme") || "dark";
applyTheme(THEMES.includes(themeStored) ? themeStored : "dark");
themeToggle.addEventListener("click", () => {
  const cur = document.documentElement.getAttribute("data-theme");
  const next = THEMES[(THEMES.indexOf(cur) + 1) % THEMES.length];
  applyTheme(next);
  localStorage.setItem("ostcore-ui3.theme", next);
});

// ---- UI zoom -----------------------------------------------------------
// Font size (and, via a real browser-level `zoom` rather than a transform,
// everything else along with it — panel sizes included) is adjustable from
// a slider, persisted like the theme, tucked behind a single toggle chip
// (Gilles, 2026-09-28: "ajuster la taille de police de tous les composants
// ... la taille des fenetres doit s'ajuster en consequence"; 2026-09-29:
// "remplace les boutons +/- par un slider qui s'affiche quand on clic sur
// un bouton unique" — same collapsible pattern as #conn-widget above).
// `zoom` on <body> covers dialogs (#dialog-root, a sibling of #app) too,
// not just #app.
const ZOOM_MIN = 0.7, ZOOM_MAX = 1.6;
function applyZoom(z) {
  z = Math.round(Math.max(ZOOM_MIN, Math.min(ZOOM_MAX, z)) * 100) / 100;
  document.body.style.zoom = z;
  // One exception, the control that changes the zoom itself shouldn't grow/
  // shrink along with it (Gilles, 2026-09-29: "le slider ne devrait pas
  // changer de taille, ce serait la seule exception") — panels.css's
  // #zoom-widget counters this exact value via calc(1 / var(--ui-zoom)),
  // which only works if it's actually published as a real CSS number.
  document.documentElement.style.setProperty("--ui-zoom", z);
  return z;
}
const zoomWidgetEl = document.getElementById("zoom-widget");
const zoomToggleEl = document.getElementById("zoom-toggle");
const zoomPanelEl = document.getElementById("zoom-panel");
const zoomSliderEl = document.getElementById("zoom-slider");

let zoomLevel = applyZoom(parseFloat(localStorage.getItem("ostcore-ui3.zoom")) || 1);
zoomSliderEl.value = zoomLevel;

function setZoomPanelOpen(open) {
  if (open) {
    // #zoom-panel is position:fixed (see panels.css) — no longer flowing
    // relative to the toggle button, so its position is computed here,
    // once per open, from the button's own real (already-zoomed) screen
    // rect, in real viewport px. Recomputed every time rather than cached:
    // cheap, and correct even if the window was resized meanwhile.
    const r = zoomToggleEl.getBoundingClientRect();
    zoomPanelEl.style.top = r.bottom + 6 + "px";
    zoomPanelEl.style.left = r.left + r.width / 2 + "px";
    zoomPanelEl.style.transform = "translateX(-50%)";
  }
  zoomPanelEl.classList.toggle("hidden", !open);
  zoomToggleEl.classList.toggle("active", open);
  if (open) zoomSliderEl.focus();
}
zoomToggleEl.addEventListener("click", () => setZoomPanelOpen(zoomPanelEl.classList.contains("hidden")));
document.addEventListener("click", (ev) => {
  if (!zoomPanelEl.classList.contains("hidden") && !zoomWidgetEl.contains(ev.target)) setZoomPanelOpen(false);
});
zoomSliderEl.addEventListener("input", () => {
  zoomLevel = applyZoom(parseFloat(zoomSliderEl.value));
  try { localStorage.setItem("ostcore-ui3.zoom", zoomLevel); } catch (e) { /* ignore */ }
});

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
  return activeView && activeView !== "__home" && activeView !== "__controller" && activeView !== "__files" && activeView !== "__logs";
}

function updateParamsToggle() {
  const mod = isModuleView() && client.get(client.topics.module(activeView));
  // Gilles, 2026-10-03: the Parameters box's own new "Info" tab (mod.infos
  // - render.js's buildParamsPanel()) means there's now something to show
  // even for a module with zero hasprofile properties (infos always has
  // at least a label) - the gear button needs to unlock for that case too,
  // or that tab would be unreachable on exactly the modules most likely
  // to have nothing else in the box.
  const hasParams = !!(mod && (Object.values(mod.properties).some((p) => p.hasprofile === true) || (mod.infos && Object.keys(mod.infos).length > 0)));
  paramsToggleEl.disabled = !hasParams;
  paramsToggleEl.title = hasParams ? "Parameters" : "Parameters (this module has none)";
  // Small red dot when the active profile has unsaved changes (module.f.changed
  // on the wire) — Gilles, 2026-09-26, so it's visible without opening the
  // modal. Recomputed on every topics.module(name) emission, which already
  // fires on profile.changed flips (see render.js's module doc comment).
  paramsToggleEl.classList.toggle("has-changes", !!(mod && mod.profile && mod.profile.changed));
}

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
    el("h3", { text: t("chrome.parametersPrefix") + (mod.infos.modulelabel || mod.infos.label || moduleName) }),
    iconButton("close", t("common.close"), close),
  ]));
  const body = el("div");
  dialog.appendChild(body);
  buildParamsPanel(client, moduleName, mod, track, body, renderContentNow);

  overlay.appendChild(dialog);
  overlay.addEventListener("click", (ev) => { if (ev.target === overlay) close(); });
  dialogRootEl.appendChild(overlay);
}

paramsToggleEl.addEventListener("click", () => { if (!paramsToggleEl.disabled) openParamsDialog(); });

// ---- connection controls --------------------------------------------------

document.getElementById("connect-btn").addEventListener("click", () => {
  const host = cleanHost(serverHostInput.value);
  serverHostInput.value = host;
  const ssl = sslOn();
  const urls = deriveUrlsFromHost(host, ssl);
  setMediaBaseUrl(normalizeMediaUrl(urls.mediaUrl));
  saveHistory({ host, ssl });
  loginShown = false;
  activeView = null;
  const lang = document.getElementById("lang-select").value;
  setLanguage(lang);
  applyStaticLabels();
  client.connect(urls.wsUrl, { language: lang });
});
document.getElementById("disconnect-btn").addEventListener("click", () => client.disconnect());
// "Self-service recovery" button (see index.html's own comment on why) - a
// stale localStorage value can break rendering in ways a hard reload
// (Ctrl+Shift+R, which only bypasses the HTTP cache, never touches
// localStorage) can't fix. Clears every storage mechanism this app could
// plausibly have written to, then reloads - best-effort on Cache Storage
// (no service worker registered today, but harmless/forward-compatible if
// one ever is).
clearCacheBtnEl.addEventListener("click", async () => {
  if (!confirm(t("chrome.clearCacheConfirm"))) return;
  try { localStorage.clear(); } catch (e) { /* ignore */ }
  try { sessionStorage.clear(); } catch (e) { /* ignore */ }
  try { if (window.caches) { for (const key of await caches.keys()) await caches.delete(key); } } catch (e) { /* ignore */ }
  location.reload();
});

// ---- OST server discovery (Android app only, js/zeroconf.js) ----------
if (zeroconf.isAvailable()) {
  discoverRowEl.classList.remove("hidden");
  discoverBtnEl.addEventListener("click", async () => {
    discoverBtnEl.disabled = true;
    discoverResultsEl.innerHTML = "";
    discoverResultsEl.appendChild(el("div", { class: "muted small", text: t("chrome.discovering") }));
    const servers = await zeroconf.discover();
    discoverBtnEl.disabled = false;
    discoverResultsEl.innerHTML = "";
    if (!servers.length) {
      discoverResultsEl.appendChild(el("div", { class: "muted small", text: t("chrome.discoverNone") }));
      return;
    }
    for (const s of servers) {
      const btn = el("button", { class: "btn btn-sm spacer", text: s.name });
      btn.addEventListener("click", () => {
        serverHostInput.value = s.host;
        // Discovery only ever makes sense unencrypted on a local network
        // (zeroconf.js's own header comment: "cette fonction discover n'a
        // d'usage que si on est en reseau local, donc sans SSL") - always off.
        setSsl(false);
        discoverResultsEl.innerHTML = "";
        // Same as picking a history entry below (serverHostInput.value = ...
        // document.getElementById("connect-btn").click()) - selecting a
        // discovered server used to only fill the host field, the panel
        // then looked like nothing happened until Connect was pressed by
        // hand (Gilles, 2026-10-10: had to reopen the settings panel to
        // find the click still needed).
        document.getElementById("connect-btn").click();
      });
      discoverResultsEl.appendChild(btn);
    }
  });
}
document.getElementById("lang-select").addEventListener("change", (e) => {
  setLanguage(e.target.value);
  applyStaticLabels();
  // Everything server-sourced (property/element labels) follows this same
  // language once the server's own reply comes back, re-triggering each
  // module's own client.subscribe() callback and rebuilding its panels —
  // that rebuild is also what picks up this file's own local i18n.t()
  // calls for free. Views NOT driven by a module subscription (system
  // views, login, the disconnected screen) don't get that free ride, so
  // force them explicitly here too.
  if (client.connection.status === "connected") client.setLanguage(e.target.value);
  renderModbarNow();
  renderContentNow();
});

client.onStatus((s) => {
  connToggleEl.classList.remove("status-connected", "status-connecting", "status-disconnected");
  connToggleEl.classList.add("status-" + s);
  connToggleEl.title = t("status." + s);
  if (s !== "connected") grantPill.textContent = "";
  // Minimal pill once connected; folded open again the moment there's
  // something the user needs to look at or act on.
  setConnPanelOpen(s !== "connected");
  if (s === "connecting" || s === "disconnected") { activeView = null; renderModbarNow(); renderContentNow(); }
});
applyStaticLabels();

// Setup wizard (issue #238) - a modal, opened straight from Settings, not a
// renderSystemViews() entry (those replace the main content area instead -
// see index.html's own comment on this button). Closes the settings
// dropdown first, same as picking a system view already does just below
// (selectView()'s own "__controller"/"__files"/"__logs" handling) - no
// reason to leave it open behind the modal.
setupWizardBtnEl.addEventListener("click", () => {
  setConnPanelOpen(false);
  openSetupWizard(client);
});

// Credits (Gilles, 2026-10-14) - same "close the settings dropdown first"
// convention as the wizard button right above.
creditsBtnEl.addEventListener("click", () => {
  setConnPanelOpen(false);
  openCreditsDialog();
});

// ---- build/version info (Settings, after the connection info) ----------
// Gilles, 2026-10-06: "ajoute un fichier 'gitdata' ... pour afficher la
// version (tag date commit message de commit)" - static per-build data
// (CI/write-gitdata.js), fetched once; no error UI if the file is
// missing (an older/non-CI www/ build, or plain local dev straight off
// index.html without ever running `npm run prepare-www`) - the row just
// stays hidden, same as it starts in index.html.
fetch("gitdata.json").then((r) => (r.ok ? r.json() : null)).then((d) => {
  if (!d) return;
  versionInfoTagEl.textContent = `${d.tag} · ${(d.date || "").slice(0, 10)} · ${d.commit}`;
  // A visible line, not just a title tooltip - Gilles, 2026-10-06: "je ne
  // vois pas le 'commit message' dans ton screenshot" - a tooltip needs
  // hover, which the Android app (touch) has no equivalent of at all.
  versionInfoMessageEl.textContent = d.message || "";
  versionInfoEl.classList.remove("hidden");
}).catch(() => { /* no gitdata.json in this build - leave the row hidden */ });

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
  // #panel-layer stays visible regardless of isModule now (Gilles,
  // 2026-10-06: "sur la home, on doit pouvoir ouvrir les logs (sans
  // selection du module) ==> la il ne se passe rien") - the persistent
  // log panel (views.js's renderLogPanel(), panels.js's own
  // `persistent:true`) lives inside #panel-layer and survives
  // Panels.clear() by design, but hiding its whole ANCESTOR container
  // whenever a docked view (Home/Controller/Files/Logs) was active made
  // it invisible and unclickable there regardless. Safe to leave it
  // always on: #panel-layer itself is pointer-events:none (panels.css),
  // so it doesn't block anything over empty space, and Panels.clear()
  // (above) already wipes every non-persistent (per-module) panel on
  // every call to this function, docked view or not.
  dockedViewEl.classList.toggle("hidden", isModule);
  // #quicknav's own visibility follows the module view directly, not
  // whether it has nav entries to show — the actions zone (params) is
  // still relevant for a module with zero level1 groups.
  quicknavEl.classList.toggle("hidden", !isModule);
  updateParamsToggle();

  if (!activeView) { dockedViewEl.appendChild(renderConnectScreen()); return; }
  if (isModule) {
    // A bespoke template (templates/index.js) wins over the generic,
    // level1-grouped renderer when infos.template names one we have —
    // see templates/index.js's header for the fallback-safety note.
    // getDisableTemplates(activeView) (the Parameters box's own per-module
    // dev-mode checkbox, render.js) forces the generic one for THIS
    // module regardless, Gilles: "je suis certain de pouvoir toucher
    // toutes les proprietes lorsque le template n'est pas encore bien
    // determine".
    const mod = client.get(client.topics.module(activeView));
    const bespoke = !getDisableTemplates(activeView) && mod && templates[mod.infos.template];
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
  if (activeView === "__home") result = renderHome(client, selectView);
  else if (activeView === "__controller") result = renderController(client);
  else if (activeView === "__files") result = renderFiles(client, getMediaBaseUrl());
  else result = renderLogs(client);
  dockedViewEl.appendChild(result.node);
  contentDispose = result.dispose || null;
}

function renderConnectScreen() {
  const div = el("div", { class: "connect-screen" });
  div.appendChild(icon("stars", { small: false }));
  div.querySelector(".material-icons").classList.add("icon-lg");
  div.appendChild(el("h2", { text: "OST — UI3" }));
  div.appendChild(el("p", { class: "lede", text: t("chrome.notConnectedLede") }));
  const panel = el("div", { class: "panel" }, [el("h3", { text: t("chrome.recentServers") })]);
  const ul = el("ul");
  for (const h of loadHistory()) {
    const li = el("li");
    const content = h.ssl ? [icon("lock", { small: true }), el("span", { text: h.host })] : h.host;
    const btn = el("button", { class: "btn spacer" }, content);
    btn.addEventListener("click", () => { serverHostInput.value = h.host; setSsl(!!h.ssl); document.getElementById("connect-btn").click(); });
    li.appendChild(btn);
    li.appendChild(iconButton("close", t("chrome.removeServer"), () => { removeHistory(h.host); renderContentNow(); }, { danger: true }));
    ul.appendChild(li);
  }
  if (!loadHistory().length) ul.appendChild(el("li", { class: "muted small", text: t("chrome.noHistoryYet") }));
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

client.subscribe(client.topics.modules(), (names) => {
  // The module bar always needs the fresh module list.
  renderModbarNow();
  // The active module just disappeared (a normal kill, or a crash under
  // issue #234's process isolation) - navigate back to Home instead of
  // leaving render.js's own "Module not found" fallback panel
  // (moduleLifecycle()) on screen until the user picks something else by
  // hand. Gilles, 2026-10-10, after seeing that fallback for the first
  // time (a crashed module disappearing while its view was open): "Auto
  // vers Home, c'est plus propre".
  if (isModuleView() && !names.includes(activeView)) selectView("__home");
});

// Gilles, 2026-10-03: "la quand je navigue l'onglet chrome indique
// 'OST-UI3' - est-ce qu'il est possible de mettre controllerdata.banner
// issu du backend ?" - same field views.js's own Controller screen
// already shows as a banner div. Falls back to the page's own static
// <title> (read once, before anything overwrites it) once disconnected
// or for a server with no banner configured.
const DEFAULT_TITLE = document.title;
client.subscribe(client.topics.controllerData(), (cd) => {
  document.title = (cd && cd.banner) || DEFAULT_TITLE;
});

// Gilles, 2026-10-03: "le bouton doit avoir le meme type de pastille que
// les boutons de modules selon le niveau le plus eleve de tous les
// messages (jaune=warning orange=error rouge=critical rien sinon)" - the
// WORST level currently in the log buffer (server-messages.en.md's "l"
// field: 0 Debug ... 4 Critical), independent of whatever the log panel's
// own module/level filter happens to be set to right now.
// Toast (js/toast.js) for any genuinely new error/critical-level entry -
// Gilles, 2026-10-10, after a module crash (issue #234) only showed up as
// a log line nobody would necessarily notice: "ça permettra de rendre ce
// type d'évènement plus visible". Error-level included too (orange,
// toast.js's own LEVEL_CLASS/panels.css already had the style ready for
// it from the start - just never triggered until now). `logs` is the
// WHOLE buffer every time (up to 1000 entries, see datastore.js), so only
// the tail past what was already seen last call is actually new.
//
// Bug fixed 2026-10-13 (Gilles: a page refresh replayed every toast in
// the whole history): comparing against the *count* seen last call isn't
// enough on its own - this subscribe() call itself fires once immediately
// with whatever's already in the (empty, freshly reset) datastore, then
// again for real once the connect-time dump lands with the full history,
// which looked exactly like "lots of genuinely new entries" to this
// code. isDumpPending() (datastore.js) tells the two apart - true for
// both of those calls, false for an actual single incremental log push.
let seenLogsCount = -1;
client.subscribe(client.topics.logs(), (logs) => {
  let worst = 0;
  for (const entry of logs || []) if ((entry.l | 0) > worst) worst = entry.l | 0;
  logBadgeDotEl.classList.remove("level-warning", "level-error", "level-critical");
  logBadgeDotEl.classList.toggle("hidden", worst < 2);
  if (worst === 2) logBadgeDotEl.classList.add("level-warning");
  else if (worst === 3) logBadgeDotEl.classList.add("level-error");
  else if (worst >= 4) logBadgeDotEl.classList.add("level-critical");

  const count = (logs || []).length;
  if (seenLogsCount >= 0 && !client.isDumpPending()) {
    for (const entry of (logs || []).slice(seenLogsCount)) {
      if ((entry.l | 0) >= 3) showToast(entry.t, entry.l | 0);
    }
  }
  seenLogsCount = count;
});

client.subscribe(client.topics.session(), (session) => {
  if (session.grantClient === null) { loginShown = false; loginToggleEl.classList.add("hidden"); return; }
  grantPill.textContent = `grant: client=${session.grantClient} server=${session.grantServer}`;
  // Gilles, 2026-10-03: "ajoute un bouton ... si grantserver != 0" - "0"
  // means the server grants full access to everyone (OST/src/
  // wshandler.cpp's own WShandler ctor: mServerGrant "0"/"1"/"2"), no
  // login ever does anything useful there. "1" (read-only by default) and
  // "2" (denied by default) both make a login meaningful - either way,
  // logging in can grant more than the server's own default.
  loginToggleEl.classList.toggle("hidden", session.grantServer === "0");
  // Gilles, 2026-10-03: "change la couleur interne du bouton de login en
  // fonction de grant-client (rouge -1 jaune 0 vert 1)" - same --error/
  // --busy/--ok palette the connection dot already uses (panels.css), not
  // new colours of their own.
  loginToggleEl.classList.remove("grant-denied", "grant-readonly", "grant-full");
  loginToggleEl.classList.add(session.grantClient === "-1" ? "grant-denied" : session.grantClient === "1" ? "grant-full" : "grant-readonly");
  if (session.grantClient === "-1" && !loginShown) { loginShown = true; renderLogin(client); }
  else if (!activeView) {
    // First dump after connecting and no view chosen yet: restore whatever
    // was on screen before the last reload/disconnect, if it still resolves
    // to something real — a saved module name that's no longer loaded (or
    // no saved view at all, e.g. the very first visit) falls back to
    // Controller, same as before this existed.
    const last = loadLastView();
    const stillThere = last === "__home" || last === "__controller" || last === "__files" || last === "__logs" || client.get(client.topics.modules()).includes(last);
    selectView(stillThere ? last : "__home");
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
