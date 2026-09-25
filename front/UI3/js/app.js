// UI3/js/app.js — wires core/ (connection + datastore, unmodified) to the
// render/views/panels/templates layers. Same app-level responsibilities as
// UI2/js/app.js (which screen is active, the module bar, the connect/login
// flow) — see that file's header for why the dispatcher-per-message-type
// pattern is gone: each screen subscribes directly to the core topic(s) it
// needs instead. UI3-specific: a module renders as floating panels on a
// shared canvas instead of a single docked node — see renderContentNow().

import { createClient, deriveDefaultUrls } from "../../core/index.js";
import { renderModuleView, setMediaBaseUrl, setDialogRoot } from "./render.js";
import { renderModbar, renderSystemViews, renderController, renderFiles, renderLogs, renderLogin } from "./views.js";
import { el, icon } from "./dom.js";
import * as Panels from "./panels.js";
import { templates } from "./templates/index.js";

const client = createClient();

const modbarEl = document.getElementById("modbar");
const systemViewsEl = document.getElementById("system-views");
const panelLayerEl = document.getElementById("panel-layer");
const dockedViewEl = document.getElementById("docked-view");
Panels.setLayer(panelLayerEl);
const connWidgetEl = document.getElementById("conn-widget");
const connToggleEl = document.getElementById("conn-toggle");
const connDotEl = document.getElementById("conn-dot");
const connSummaryEl = document.getElementById("conn-summary");
const connPanelEl = document.getElementById("conn-panel");
const grantPill = document.getElementById("grant-pill");
const wsUrlInput = document.getElementById("ws-url");
const mediaUrlInput = document.getElementById("media-url");
const themeToggle = document.getElementById("theme-toggle");

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

// ---- derive defaults per structure.en.md's "Finding the server" table ---

function loadHistory() {
  try { return JSON.parse(localStorage.getItem("ostcore-ui3.history") || "[]"); } catch (e) { return []; }
}
function saveHistory(entry) {
  const hist = loadHistory().filter((h) => h.wsUrl !== entry.wsUrl);
  hist.unshift(entry);
  try { localStorage.setItem("ostcore-ui3.history", JSON.stringify(hist.slice(0, 10))); } catch (e) { /* ignore */ }
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
  modbarEl.innerHTML = "";
  modbarEl.appendChild(renderModbar(client, activeView, selectView));
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

  const isModule = activeView && activeView !== "__controller" && activeView !== "__files" && activeView !== "__logs";
  panelLayerEl.classList.toggle("hidden", !isModule);
  dockedViewEl.classList.toggle("hidden", isModule);

  if (!activeView) { dockedViewEl.appendChild(renderConnectScreen()); return; }
  if (isModule) {
    // A bespoke template (templates/index.js) wins over the generic,
    // level1-grouped renderer when infos.template names one we have —
    // see templates/index.js's header for the fallback-safety note.
    const mod = client.get(client.topics.module(activeView));
    const bespoke = mod && templates[mod.infos.template];
    contentDispose = (bespoke ? bespoke.mount(client, activeView) : renderModuleView(client, activeView)).dispose;
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
    // First dump after connecting and no view chosen yet: land on Controller.
    selectView("__controller");
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
