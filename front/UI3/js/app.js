// UI3/js/app.js — wires core/ (connection + datastore, unmodified) to the
// render/views layer. Same app-level responsibilities as UI2/js/app.js
// (which screen is active, the sidebar, the connect/login flow) — see that
// file's header for why the dispatcher-per-message-type pattern is gone:
// each screen subscribes directly to the core topic(s) it needs instead.

import { createClient, deriveDefaultUrls } from "../../core/index.js";
import { renderModuleScreen, setMediaBaseUrl, setDialogRoot } from "./render.js";
import { renderSidebar, renderController, renderFiles, renderLogs, renderLogin } from "./views.js";
import { el, icon } from "./dom.js";

const client = createClient();

const sidebarEl = document.getElementById("sidebar");
const contentInner = document.getElementById("content-inner");
const statusPill = document.getElementById("status-pill");
const grantPill = document.getElementById("grant-pill");
const wsUrlInput = document.getElementById("ws-url");
const mediaUrlInput = document.getElementById("media-url");
const themeToggle = document.getElementById("theme-toggle");

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
  themeToggle.appendChild(icon(t === "light" ? "light_mode" : "dark_mode"));
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
  statusPill.textContent = s;
  statusPill.className = "status-pill " + s;
  if (s !== "connected") grantPill.textContent = "";
  if (s === "connecting" || s === "disconnected") { activeView = null; renderSidebarNow(); renderContentNow(); }
});

// ---- rendering -------------------------------------------------------

function renderSidebarNow() {
  sidebarEl.innerHTML = "";
  sidebarEl.appendChild(renderSidebar(client, activeView, selectView));
}

function renderContentNow() {
  if (contentDispose) { contentDispose(); contentDispose = null; }
  contentInner.innerHTML = "";
  if (!activeView) { contentInner.appendChild(renderConnectScreen()); return; }
  let result;
  if (activeView === "__controller") result = renderController(client);
  else if (activeView === "__files") result = renderFiles(client, mediaUrlInput.value);
  else if (activeView === "__logs") result = renderLogs(client);
  else result = renderModuleScreen(client, activeView);
  contentInner.appendChild(result.node);
  contentDispose = result.dispose || null;
}

function renderConnectScreen() {
  const div = el("div", { class: "connect-screen" });
  div.appendChild(icon("nights_stay", { small: false }));
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
  renderSidebarNow();
  renderContentNow();
}

// ---- app-level topic subscriptions --------------------------------------
// Only truly cross-cutting concerns live here: the sidebar (needs the
// module list), the login flow (needs session/grant), and "the active
// module just disappeared" (also the module list). Everything else is the
// concern of whichever screen is currently rendered — see render.js/views.js.

client.subscribe(client.topics.modules(), () => {
  // Sidebar always needs the fresh module list. The active module screen
  // does NOT need a poke here: renderModuleScreen (render.js) already
  // subscribes to topics.module(name) directly and renders its own
  // "not found" state when a module is removed — see its module doc.
  renderSidebarNow();
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

renderSidebarNow();
renderContentNow();

// Auto-connect only if the page is actually served by an HTTP(S) origin
// (the derived default is meaningful there); left as a manual action
// (press Connect) so the user can adjust the URL first if needed.
