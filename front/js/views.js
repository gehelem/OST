// views.js — the non-module screens: sidebar, controller/global settings,
// file browser, log panel, login dialog. Same functional coverage as
// ostcore/UI2/js/views.js (verified against a real server), re-skinned for
// UI3's design. Screens that stay on-screen and need live data
// (controller/files/logs) subscribe themselves through client.scope() and
// return {node, dispose}; the sidebar and login dialog don't hold any
// subscription open, so they just return a node.

import { el, icon, iconButton, moduleIcon, statusDot, updateStatusDot } from "./dom.js";
import { togglePanel, isPanelVisible, make as makePanel } from "./panels.js";
import { t } from "./i18n.js";

// Shared by renderLogs() (the full-page Logs system view) and
// renderLogPanel() (the always-on floating one, below) - index == the
// wire's own `l` field (server-messages.en.md's "l" message: 0 Debug, 1
// Info, 2 Warning, 3 Error, 4 Critical).
const LOG_LEVELS = ["debug", "info", "warning", "error", "critical"];

// Horizontal module bar (ostnoctua's .on-modbar/.on-mod, see panels.css) —
// only the actually-loaded OST modules. Controller/Files/Logs live in the
// settings dropdown instead (see renderSystemViews) — Gilles, 2026-09-25:
// "deplace les boutons controller/files/logs dans les settings".
//
// Returns {node, dispose}: node is a DocumentFragment, not a wrapping <div>
// — #modbar itself is the flex row (panels.css), and .mod-btn is
// `display:flex` for its own icon-over-label stacking — that makes each
// button a block-level box, so wrapping them in an unstyled <div> stacked
// them vertically instead of letting #modbar's flex row lay them out
// (caught live, 2026-09-25: "la barre de menu est a nouveau verticale"). A
// fragment leaves no such wrapper behind: the buttons become #modbar's
// direct children. `dispose` must be called before the caller rebuilds the
// bar (app.js's renderModbarNow()) — see the per-module subscription below.
export function renderModbar(client, activeKey, onSelect) {
  const bar = document.createDocumentFragment();
  const names = client.get(client.topics.modules());
  const disposers = [];
  for (const name of names) {
    const mod = client.get(client.topics.module(name));
    const label = (mod && (mod.infos.modulelabel || mod.infos.label)) || name;
    // Global-state dot (Gilles, 2026-09-27): the same "signals.state" light
    // element the module's own header panel already shows (see render.js's
    // renderProperty()), so a module in trouble is visible without opening
    // it. Not every module declares "signals" (a handful, e.g.
    // GlobalDatastore, don't call Basemodule::setStateEvent() at all) — for
    // those the dot just stays absent (.hidden) rather than showing a fake
    // standby color. Subscribes directly to the "signals" *property*, not
    // topics.module(name) (Gilles, 2026-09-27: "cette bubulle devrait
    // s'abonner a nomDuModule.signals(.state)") — topics.module() only
    // fires on module add/remove/profile events (see datastore.js's own
    // doc comment on it), never on an element value change (ea/ee), which
    // is exactly how "state" itself updates; only topics.property() covers
    // that.
    const dot = statusDot(0);
    dot.classList.add("mod-btn-dot", "hidden");
    // client.subscribe() fires immediately with the current value (same
    // convention as renderProperty()'s own subscription), so no separate
    // initial call is needed here.
    disposers.push(client.subscribe(client.topics.property(name, "signals"), (prop) => {
      const state = prop && prop.elements && prop.elements.state;
      dot.classList.toggle("hidden", !state);
      if (state) updateStatusDot(dot, state.value);
    }));
    const btn = el("button", { class: "mod-btn" + (activeKey === name ? " active" : ""), title: label, onclick: () => onSelect(name) }, [
      dot,
      icon(moduleIcon(mod)),
      el("span", { class: "mod-btn-label", text: label }),
    ]);
    bar.appendChild(btn);
  }
  return { node: bar, dispose: () => disposers.forEach((d) => d()) };
}

/**
 * The per-module "quick nav" bar (#quicknav, bottom-center): one button per
 * {id, label, icon?} entry, showing/hiding that panel and reflecting
 * whether it's currently visible (Gilles, 2026-09-27: "l'action du bouton
 * doit masquer/demasquer la fenetre et l'etat du bouton doit montrer si la
 * fenetre est visible ou pas") — Panels.togglePanel()/isPanelVisible() are
 * the *only* shared piece (render.js/templates own the entry list; see
 * render.js's setQuickNavRoot()). `data-panel-id` lets app.js's single
 * "panelvisibility" listener find and update this same button if the
 * panel's visibility is ever changed some other way than this button's
 * own click - decoupled on purpose, even with no such caller today.
 * Same DocumentFragment pattern as renderModbar, for the same reason (no
 * unstyled wrapper element). `icon` (a material-icons name) is optional —
 * most entries are text-only; a template can ask for an icon instead when
 * the button represents an action rather than "jump to this panel" (e.g.
 * navigator's "Settings" button, Gilles, 2026-09-30: "l'icone avec 3
 * sliders").
 */
export function renderQuickNav(entries) {
  const bar = document.createDocumentFragment();
  for (const { id, label, icon: iconName, iconOnly } of entries) {
    // `iconOnly` (render.js's own renderActionsPanel(): the actions
    // window's button, Gilles, 2026-10-03: "bouton = juste une icone
    // 'dashboard_customize'") drops the text span entirely - `title`
    // still carries the label as a tooltip either way. Every other entry
    // keeps its previous icon+label (icon given) or plain-text (no icon)
    // shape, unchanged.
    const content = iconOnly ? [icon(iconName)] : iconName ? [icon(iconName), el("span", { text: label })] : label;
    const btn = el("button", {
      class: "quicknav-btn" + (iconOnly ? " quicknav-btn-icon-only" : "") + (isPanelVisible(id) ? " active" : ""),
      title: label, "data-panel-id": id,
      onclick: () => btn.classList.toggle("active", togglePanel(id)),
    }, content);
    bar.appendChild(btn);
  }
  return bar;
}

// The three system/utility screens, rendered as a small vertical list inside
// the settings dropdown (#conn-panel) instead of the module bar — they
// aren't OST modules, they're this client's own chrome, so grouping them
// with the other app-level settings reads better than mixing them into the
// live module list.
const SYSTEM_VIEWS = [
  { key: "__controller", get label() { return t("views.controller"); }, icon: "tune" },
  { key: "__files", get label() { return t("views.files"); }, icon: "folder" },
  { key: "__logs", get label() { return t("views.logs"); }, icon: "list" },
];
export function renderSystemViews(activeKey, onSelect) {
  const nav = el("div", { class: "settings-nav" });
  for (const item of SYSTEM_VIEWS) {
    const btn = el("button", { class: "settings-nav-item" + (activeKey === item.key ? " active" : ""), onclick: () => onSelect(item.key) }, [
      icon(item.icon, { small: true }),
      el("span", { text: item.label }),
    ]);
    nav.appendChild(btn);
  }
  return nav;
}

export function renderLogin(client, onDone) {
  const overlay = el("div", { class: "dialog-overlay" });
  const user = el("input", { type: "text", class: "field-input", placeholder: t("views.userPlaceholder"), style: "width:100%" });
  const pw = el("input", { type: "password", class: "field-input", placeholder: t("views.passwordPlaceholder"), style: "width:100%" });
  // Gilles, 2026-10-03: "on ne peut pas fermer la fenetre de login si on
  // veut" - same close convention as render.js's own grid create/edit
  // dialog (backdrop click + an explicit button, no onclick-only way out).
  // Dismissing it without logging in is fine even for the auto-popup case
  // (grant-client "-1") - it just means staying denied, not a dead end.
  const close = () => overlay.remove();
  const submit = () => { client.login(user.value, pw.value); close(); if (onDone) onDone(); };
  pw.addEventListener("keydown", (ev) => { if (ev.key === "Enter") submit(); });
  const dialog = el("div", { class: "dialog" }, [
    el("h3", { text: t("views.loginRequired") }),
    el("div", { class: "muted small", text: t("views.loginRequiresAuth") }),
    el("div", { class: "form-row" }, [el("label", { text: t("views.user") }), user]),
    el("div", { class: "form-row" }, [el("label", { text: t("views.password") }), pw]),
    el("div", { class: "dialog-actions" }, [
      el("button", { class: "btn", onclick: close }, t("common.close")),
      el("button", { class: "btn btn-primary", onclick: submit }, t("views.login")),
    ]),
  ]);
  overlay.appendChild(dialog);
  overlay.addEventListener("click", (ev) => { if (ev.target === overlay) close(); });
  document.body.appendChild(overlay);
}

function fmtBytes(mb) {
  if (mb === undefined) return "—";
  if (mb > 1024) return (mb / 1024).toFixed(1) + " GB";
  return mb.toFixed(0) + " MB";
}

export function renderController(client) {
  const root = el("div", { class: "controller-view" });
  const scope = client.scope();

  // Two independent topics feed this one screen (controllerdata for most
  // sections, the module list for "Loaded modules"/"Kill") — re-render
  // from a single function driven by either, rather than reaching for a
  // broader "everything" topic that doesn't exist in core by design.
  function render() {
    const cd = client.get(client.topics.controllerData()) || {};
    root.innerHTML = "";
    root.appendChild(el("h2", null, [icon("tune"), el("span", { text: " " + t("views.controller") })]));
    if (cd.banner) root.appendChild(el("div", { class: "banner", text: cd.banner }));
    if (cd.git) root.appendChild(el("div", { class: "muted small mono", text: `${cd.git.Gittag || ""} ${cd.git.Githash || ""} — ${cd.git.Gitdate || ""}` }));

    const grid = el("div", { class: "panel-grid" });

    const sw = cd.systemwatcher;
    if (sw) {
      const sec = el("section", { class: "panel" }, [el("h3", { text: t("views.serverLoad") })]);
      if (sw.cpu) sec.appendChild(el("div", { class: "metric-row" }, [el("span", { class: "label", text: t("views.cpuLabel") }), el("span", { class: "value", text: `${sw.cpu.load_1m} / ${sw.cpu.load_5m} / ${sw.cpu.load_15m}` })]));
      if (sw.ram) sec.appendChild(el("div", { class: "metric-row" }, [el("span", { class: "label", text: t("views.ramLabel") }), el("span", { class: "value", text: `${fmtBytes(sw.ram.used_mb)} / ${fmtBytes(sw.ram.total_mb)}` })]));
      for (const d of sw.disks || []) sec.appendChild(el("div", { class: "metric-row" }, [el("span", { class: "label", text: `${d.mountpoint} (${d.device})` }), el("span", { class: "value", text: `${d.used_gb.toFixed(1)} / ${d.total_gb.toFixed(1)} GB` })]));
      for (const n of sw.network || []) sec.appendChild(el("div", { class: "metric-row" }, [el("span", { class: "label", text: n.iface }), el("span", { class: "value", text: `↓${n.rx_kbps.toFixed(1)} ↑${n.tx_kbps.toFixed(1)} kbps` })]));
      for (const t of sw.temperatures || []) sec.appendChild(el("div", { class: "metric-row" }, [el("span", { class: "label", text: t.type }), el("span", { class: "value", text: `${t.temp_c.toFixed(1)} °C` })]));
      grid.appendChild(sec);
    }

    const confSec = el("section", { class: "panel" }, [el("h3", { text: t("views.configurations") })]);
    const confs = cd.availableconfs || {};
    const confList = el("ul");
    for (const [name, modlist] of Object.entries(confs)) {
      const li = el("li");
      li.appendChild(el("span", { class: "spacer", text: `${name}${cd.currentconf === name ? t("views.current") : ""} — ${(modlist || []).length} ${t("views.moduleCountSuffix")}` }));
      li.appendChild(el("button", { class: "btn btn-sm", onclick: () => client.loadConfiguration(name) }, t("views.load")));
      confList.appendChild(li);
    }
    confSec.appendChild(confList);
    const nameInput = el("input", { type: "text", class: "field-input", placeholder: t("views.configNamePlaceholder") });
    confSec.appendChild(el("div", { class: "form-row" }, [nameInput, el("button", { class: "btn btn-primary btn-sm", onclick: () => { if (nameInput.value) client.saveConfiguration(nameInput.value); } }, t("views.saveCurrentAs"))]));
    grid.appendChild(confSec);

    const modSec = el("section", { class: "panel" }, [el("h3", { text: t("views.loadedModules") })]);
    const modList = el("ul");
    for (const name of client.get(client.topics.modules())) {
      const li = el("li");
      li.appendChild(el("span", { class: "spacer mono small", text: name }));
      if (name !== "GlobalDatastore") li.appendChild(el("button", { class: "btn btn-sm btn-danger", onclick: () => { if (confirm(t("views.unloadConfirm", { name }))) client.killModule(name); } }, t("views.kill")));
      modList.appendChild(li);
    }
    modSec.appendChild(modList);
    grid.appendChild(modSec);

    const libSec = el("section", { class: "panel" }, [el("h3", { text: t("views.loadAModule") })]);
    const libs = cd.libraries || {};
    const libSelect = el("select", { class: "field-input" });
    for (const key of Object.keys(libs)) {
      const stripped = key.startsWith("libost") ? key.slice(6) : key;
      libSelect.appendChild(el("option", { value: stripped, text: key }));
    }
    const labelInput = el("input", { type: "text", class: "field-input", placeholder: t("views.instanceLabelPlaceholder") });
    const profileInput = el("input", { type: "text", class: "field-input", placeholder: t("views.profileOptionalPlaceholder") });
    libSec.appendChild(el("div", { class: "form-row" }, [libSelect, labelInput, profileInput,
      el("button", { class: "btn btn-primary btn-sm", onclick: () => { if (labelInput.value) client.loadModule(libSelect.value, labelInput.value, profileInput.value || undefined); } }, t("views.load")),
    ]));
    grid.appendChild(libSec);

    const indiSec = el("section", { class: "panel" }, [el("h3", { text: t("views.indiServerPrefix") + (cd.indiserver === "Y" ? t("views.enabled") : t("views.disabled")) })]);
    indiSec.appendChild(el("div", { class: "form-row" }, [
      el("button", { class: "btn btn-sm", onclick: () => client.indiStart() }, t("views.startServer")),
      el("button", { class: "btn btn-sm", onclick: () => client.indiStop() }, t("views.stopServer")),
    ]));
    const active = new Set((cd.indiactivedrivers || []).map((d) => d.binary));
    const driversList = el("ul");
    for (const d of cd.indidrivers || []) {
      const li = el("li");
      li.appendChild(el("span", { class: "spacer small", text: `${d.label} (${d.family})${active.has(d.binary) ? t("views.runningSuffix") : ""}` }));
      li.appendChild(el("button", { class: "btn btn-sm", onclick: () => client.indiLoadDriver(d.binary) }, t("views.load")));
      li.appendChild(el("button", { class: "btn btn-sm", onclick: () => client.indiReloadDriver(d.binary) }, t("views.reload")));
      li.appendChild(el("button", { class: "btn btn-sm btn-danger", onclick: () => client.indiStopDriver(d.binary) }, t("views.stop")));
      driversList.appendChild(li);
    }
    indiSec.appendChild(driversList);
    grid.appendChild(indiSec);

    root.appendChild(grid);
  }

  scope.subscribe(client.topics.controllerData(), render);
  scope.subscribe(client.topics.modules(), render);

  return { node: root, dispose: scope.dispose };
}

function buildFolderTree(folders) {
  const root = { name: "", path: "", children: new Map() };
  for (const path of folders) {
    if (path === "") continue;
    const parts = path.split("/").filter(Boolean);
    let node = root;
    let acc = "";
    for (const part of parts) {
      acc += "/" + part;
      if (!node.children.has(part)) node.children.set(part, { name: part, path: acc, children: new Map() });
      node = node.children.get(part);
    }
  }
  return root;
}

function renderFolderNode(node, client, depth) {
  const wrap = el("div", { class: "folder-node", style: `padding-left:${depth * 14}px` });
  const btn = el("button", { class: "folder-btn" }, [icon("folder", { small: true }), el("span", { text: node.name || t("views.rootFolder") })]);
  btn.addEventListener("click", () => client.selectFolder(node.path));
  wrap.appendChild(btn);
  const children = Array.from(node.children.values()).sort((a, b) => a.name.localeCompare(b.name));
  for (const child of children) wrap.appendChild(renderFolderNode(child, client, depth + 1));
  return wrap;
}

export function renderFiles(client, mediaBaseUrl) {
  const root = el("div", { class: "files-view" });
  const scope = client.scope();

  scope.subscribe(client.topics.files(), (files) => {
    files = files || { folders: [], files: [], selectedfolder: "" };
    root.innerHTML = "";
    root.appendChild(el("h2", null, [icon("folder"), el("span", { text: " " + t("views.files") })]));
    const layout = el("div", { class: "files-layout" });
    const tree = buildFolderTree(files.folders || []);
    const treeWrap = el("div", { class: "folder-tree" });
    // The selected folder in the dump is an absolute server-side path, not
    // the relative path used for navigation/FS — we cannot reliably
    // highlight the active node from it alone (see ostcore/FINDINGS.md), so
    // no node is marked "active" here.
    treeWrap.appendChild(renderFolderNode(tree, client, 0));
    layout.appendChild(treeWrap);

    const listWrap = el("div", { class: "file-list-wrap" });
    listWrap.appendChild(el("div", { class: "muted small mono", text: t("views.selectedPrefix") + (files.selectedfolder || t("views.none")) }));
    const fgrid = el("div", { class: "file-grid" });
    for (const f of files.files || []) {
      const url = mediaBaseUrl + (f.startsWith("/") ? f.slice(1) : f);
      const lower = f.toLowerCase();
      const isImg = /\.(jpe?g|png|gif|webp)$/.test(lower);
      const card = el("a", { href: url, target: "_blank", class: "file-card" });
      if (isImg) card.appendChild(el("img", { src: url, class: "file-thumb" }));
      else card.appendChild(icon("description", { small: false }));
      card.appendChild(el("div", { class: "file-name", text: f.split("/").pop() }));
      fgrid.appendChild(card);
    }
    if (!(files.files || []).length) fgrid.appendChild(el("div", { class: "muted small", text: t("views.noFilesInFolder") }));
    listWrap.appendChild(fgrid);
    layout.appendChild(listWrap);
    root.appendChild(layout);
  });

  return { node: root, dispose: scope.dispose };
}

export function renderLogs(client) {
  const root = el("div", { class: "logs-view" });
  const scope = client.scope();

  scope.subscribe(client.topics.logs(), (logs) => {
    root.innerHTML = "";
    root.appendChild(el("h2", null, [icon("list"), el("span", { text: " " + t("views.logs") })]));
    root.appendChild(el("div", { class: "log-toolbar muted small", text: `${(logs || []).length} ${t("views.entriesCappedSuffix")}` }));
    const table = el("table", { class: "log-table" });
    for (const entry of (logs || []).slice().reverse()) {
      const lvl = LOG_LEVELS[entry.l] || "info";
      table.appendChild(el("tr", { class: "log-row log-" + lvl }, [
        el("td", { class: "small muted mono", text: entry.d }),
        el("td", { class: "small mono", text: entry.c }),
        el("td", { text: entry.t }),
      ]));
    }
    root.appendChild(table);
  });

  return { node: root, dispose: scope.dispose };
}

// ---- always-on floating log panel ----------------------------------------
// Gilles, 2026-10-03: "en haut a droite ... ajoute ... un bouton pour la
// gestion de logs - inspire de ostnoctua pour la forme de la fenetre a
// afficher: selection du module / selection du niveau du message / quand
// on clique depuis un module ca selectionne les messages du module /
// couleur selon niveau du message". Shape mirrors ostnoctua's own
// web/log.js (OstLog): a persistent floating panel (Panels.make()'s own
// `persistent: true`, survives module switches, same as ostnoctua's own
// "always-on" framing) with a module + minimum-level filter toolbar above
// a scrollable, colour-coded list - but the module filter here is a real
// per-module dropdown (every module that has actually logged something,
// built from the log entries' own `c` field) rather than ostnoctua's
// simpler this-module/all-modules toggle, since Gilles asked for
// "selection du module" specifically.
const LOG_FILTER_KEY = "ostcore-ui3:logs:filter";
function loadLogFilter() {
  try { return JSON.parse(localStorage.getItem(LOG_FILTER_KEY)) || {}; } catch (e) { return {}; }
}
function saveLogFilter(f) {
  try { localStorage.setItem(LOG_FILTER_KEY, JSON.stringify(f)); } catch (e) { /* ignore */ }
}
const LOG_MODULE_ALL = "__all";

export function renderLogPanel(client, initialModule) {
  const existing = document.querySelector('.panel-float[data-panel="logs"]');
  if (existing) {
    // Already built (this is a toggle, not a re-create - the panel is
    // persistent, it never gets torn down by a module switch). Re-apply
    // the module filter if the button was clicked from a module's own
    // view this time, same as a fresh creation would below, then just
    // show/hide it like any other panel.
    if (initialModule) {
      const sel = existing.querySelector(".log-filter-module");
      if (sel && [...sel.options].some((o) => o.value === initialModule)) {
        sel.value = initialModule;
        sel.dispatchEvent(new Event("change"));
      }
    }
    togglePanel("logs");
    return;
  }

  const filter = loadLogFilter();
  if (initialModule) filter.module = initialModule;
  if (!filter.level) filter.level = "0";

  const body = makePanel({
    id: "logs", title: t("views.logs"), persistent: true,
    x: Math.max(20, window.innerWidth - 460), y: Math.max(80, window.innerHeight - 340), w: 420,
  });
  body.classList.add("log-panel-body");

  const toolbar = el("div", { class: "log-toolbar" });
  const moduleSel = el("select", { class: "field-input log-filter-module" });
  const levelSel = el("select", { class: "field-input log-filter-level" });
  for (const [key, label] of [
    ["0", t("views.logLevelAll")], ["1", t("views.logLevelInfo")], ["2", t("views.logLevelWarning")],
    ["3", t("views.logLevelError")], ["4", t("views.logLevelCritical")],
  ]) {
    const opt = el("option", { value: key }, label);
    levelSel.appendChild(opt);
  }
  levelSel.value = filter.level;
  toolbar.appendChild(moduleSel);
  toolbar.appendChild(levelSel);
  body.appendChild(toolbar);

  const list = el("div", { class: "log-panel-list" });
  body.appendChild(list);

  function passes(entry) {
    if ((entry.l | 0) < (filter.level | 0)) return false;
    return !filter.module || filter.module === LOG_MODULE_ALL || entry.c === filter.module;
  }
  function rebuildRows(logs) {
    const atBottom = list.scrollHeight - list.scrollTop - list.clientHeight < 24;
    list.innerHTML = "";
    for (const entry of (logs || []).filter(passes)) {
      list.appendChild(el("div", { class: "log-panel-row log-" + (LOG_LEVELS[entry.l] || "info") }, [
        el("span", { class: "log-panel-time", text: String(entry.d || "").replace(/^.*[ T]/, "").slice(0, 8) }),
        el("span", { class: "log-panel-src", text: entry.c || "" }),
        el("span", { class: "log-panel-msg", text: entry.t || "" }),
      ]));
    }
    if (atBottom) list.scrollTop = list.scrollHeight;
  }

  // Module options: every module that has actually logged something (built
  // from the log entries' own `c` field, not the live module list - a
  // module that's since been unloaded, or the fixed non-module "WS"
  // source, keeps its own past entries reachable this way), plus "All".
  // Rebuilt on every log update; the current selection (including one not
  // presently in the list, e.g. just set via initialModule) is preserved.
  client.subscribe(client.topics.logs(), (logs) => {
    const sources = [...new Set((logs || []).map((e) => e.c).filter(Boolean))].sort();
    if (filter.module && filter.module !== LOG_MODULE_ALL && !sources.includes(filter.module)) sources.push(filter.module);
    const current = moduleSel.value || filter.module || LOG_MODULE_ALL;
    moduleSel.innerHTML = "";
    moduleSel.appendChild(el("option", { value: LOG_MODULE_ALL }, t("views.logModuleAll")));
    for (const src of sources) moduleSel.appendChild(el("option", { value: src }, src));
    moduleSel.value = [...moduleSel.options].some((o) => o.value === current) ? current : LOG_MODULE_ALL;
    rebuildRows(logs);
  });

  moduleSel.addEventListener("change", () => {
    filter.module = moduleSel.value;
    saveLogFilter(filter);
    rebuildRows(client.get(client.topics.logs()));
  });
  levelSel.addEventListener("change", () => {
    filter.level = levelSel.value;
    saveLogFilter(filter);
    rebuildRows(client.get(client.topics.logs()));
  });
}
