// views.js — the non-module screens: sidebar, controller/global settings,
// file browser, log panel, login dialog. Same functional coverage as
// ostcore/UI2/js/views.js (verified against a real server), re-skinned for
// UI3's design. Screens that stay on-screen and need live data
// (controller/files/logs) subscribe themselves through client.scope() and
// return {node, dispose}; the sidebar and login dialog don't hold any
// subscription open, so they just return a node.

import { el, icon, iconButton, moduleIcon } from "./dom.js";

export function renderSidebar(client, activeKey, onSelect) {
  const nav = el("nav");
  const staticItems = [
    { key: "__controller", label: "Controller", icon: "tune" },
    { key: "__files", label: "Files", icon: "folder" },
    { key: "__logs", label: "Logs", icon: "receipt_long" },
  ];
  for (const item of staticItems) {
    const btn = el("button", { class: "nav-item" + (activeKey === item.key ? " active" : ""), onclick: () => onSelect(item.key) }, [
      icon(item.icon),
      el("span", { class: "nav-label", text: item.label }),
    ]);
    nav.appendChild(btn);
  }
  const names = client.get(client.topics.modules());
  if (names.length) {
    nav.appendChild(el("div", { class: "nav-sep" }));
    nav.appendChild(el("div", { class: "nav-section-label", text: "Modules" }));
    for (const name of names) {
      const mod = client.get(client.topics.module(name));
      const label = (mod && (mod.infos.modulelabel || mod.infos.label)) || name;
      const btn = el("button", { class: "nav-item" + (activeKey === name ? " active" : ""), onclick: () => onSelect(name) }, [
        icon(moduleIcon(mod, name)),
        el("span", { class: "nav-label", text: label }),
      ]);
      nav.appendChild(btn);
    }
  }
  return nav;
}

export function renderLogin(client, onDone) {
  const overlay = el("div", { class: "dialog-overlay" });
  const user = el("input", { type: "text", class: "field-input", placeholder: "user", style: "width:100%" });
  const pw = el("input", { type: "password", class: "field-input", placeholder: "password", style: "width:100%" });
  const submit = () => { client.login(user.value, pw.value); overlay.remove(); if (onDone) onDone(); };
  pw.addEventListener("keydown", (ev) => { if (ev.key === "Enter") submit(); });
  const dialog = el("div", { class: "dialog" }, [
    el("h3", { text: "Login required" }),
    el("div", { class: "muted small", text: "This server requires authentication (grant-client = -1)." }),
    el("div", { class: "form-row" }, [el("label", { text: "User" }), user]),
    el("div", { class: "form-row" }, [el("label", { text: "Password" }), pw]),
    el("div", { class: "dialog-actions" }, [
      el("button", { class: "btn btn-primary", onclick: submit }, "Login"),
    ]),
  ]);
  overlay.appendChild(dialog);
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
    root.appendChild(el("h2", null, [icon("tune"), el("span", { text: " Controller" })]));
    if (cd.banner) root.appendChild(el("div", { class: "banner", text: cd.banner }));
    if (cd.git) root.appendChild(el("div", { class: "muted small mono", text: `${cd.git.Gittag || ""} ${cd.git.Githash || ""} — ${cd.git.Gitdate || ""}` }));

    const grid = el("div", { class: "panel-grid" });

    const sw = cd.systemwatcher;
    if (sw) {
      const sec = el("section", { class: "panel" }, [el("h3", { text: "Server load" })]);
      if (sw.cpu) sec.appendChild(el("div", { class: "metric-row" }, [el("span", { class: "label", text: "CPU (1/5/15m)" }), el("span", { class: "value", text: `${sw.cpu.load_1m} / ${sw.cpu.load_5m} / ${sw.cpu.load_15m}` })]));
      if (sw.ram) sec.appendChild(el("div", { class: "metric-row" }, [el("span", { class: "label", text: "RAM" }), el("span", { class: "value", text: `${fmtBytes(sw.ram.used_mb)} / ${fmtBytes(sw.ram.total_mb)}` })]));
      for (const d of sw.disks || []) sec.appendChild(el("div", { class: "metric-row" }, [el("span", { class: "label", text: `${d.mountpoint} (${d.device})` }), el("span", { class: "value", text: `${d.used_gb.toFixed(1)} / ${d.total_gb.toFixed(1)} GB` })]));
      for (const n of sw.network || []) sec.appendChild(el("div", { class: "metric-row" }, [el("span", { class: "label", text: n.iface }), el("span", { class: "value", text: `↓${n.rx_kbps.toFixed(1)} ↑${n.tx_kbps.toFixed(1)} kbps` })]));
      for (const t of sw.temperatures || []) sec.appendChild(el("div", { class: "metric-row" }, [el("span", { class: "label", text: t.type }), el("span", { class: "value", text: `${t.temp_c.toFixed(1)} °C` })]));
      grid.appendChild(sec);
    }

    const confSec = el("section", { class: "panel" }, [el("h3", { text: "Configurations" })]);
    const confs = cd.availableconfs || {};
    const confList = el("ul");
    for (const [name, modlist] of Object.entries(confs)) {
      const li = el("li");
      li.appendChild(el("span", { class: "spacer", text: `${name}${cd.currentconf === name ? " (current)" : ""} — ${(modlist || []).length} module(s)` }));
      li.appendChild(el("button", { class: "btn btn-sm", onclick: () => client.loadConfiguration(name) }, "Load"));
      confList.appendChild(li);
    }
    confSec.appendChild(confList);
    const nameInput = el("input", { type: "text", class: "field-input", placeholder: "configuration name" });
    confSec.appendChild(el("div", { class: "form-row" }, [nameInput, el("button", { class: "btn btn-primary btn-sm", onclick: () => { if (nameInput.value) client.saveConfiguration(nameInput.value); } }, "Save current as…")]));
    grid.appendChild(confSec);

    const modSec = el("section", { class: "panel" }, [el("h3", { text: "Loaded modules" })]);
    const modList = el("ul");
    for (const name of client.get(client.topics.modules())) {
      const li = el("li");
      li.appendChild(el("span", { class: "spacer mono small", text: name }));
      if (name !== "GlobalDatastore") li.appendChild(el("button", { class: "btn btn-sm btn-danger", onclick: () => { if (confirm(`Unload ${name}?`)) client.killModule(name); } }, "Kill"));
      modList.appendChild(li);
    }
    modSec.appendChild(modList);
    grid.appendChild(modSec);

    const libSec = el("section", { class: "panel" }, [el("h3", { text: "Load a module" })]);
    const libs = cd.libraries || {};
    const libSelect = el("select", { class: "field-input" });
    for (const key of Object.keys(libs)) {
      const stripped = key.startsWith("libost") ? key.slice(6) : key;
      libSelect.appendChild(el("option", { value: stripped, text: key }));
    }
    const labelInput = el("input", { type: "text", class: "field-input", placeholder: "instance label" });
    const profileInput = el("input", { type: "text", class: "field-input", placeholder: "profile (optional)" });
    libSec.appendChild(el("div", { class: "form-row" }, [libSelect, labelInput, profileInput,
      el("button", { class: "btn btn-primary btn-sm", onclick: () => { if (labelInput.value) client.loadModule(libSelect.value, labelInput.value, profileInput.value || undefined); } }, "Load"),
    ]));
    grid.appendChild(libSec);

    const indiSec = el("section", { class: "panel" }, [el("h3", { text: `INDI server — ${cd.indiserver === "Y" ? "enabled" : "disabled"}` })]);
    indiSec.appendChild(el("div", { class: "form-row" }, [
      el("button", { class: "btn btn-sm", onclick: () => client.indiStart() }, "Start server"),
      el("button", { class: "btn btn-sm", onclick: () => client.indiStop() }, "Stop server"),
    ]));
    const active = new Set((cd.indiactivedrivers || []).map((d) => d.binary));
    const driversList = el("ul");
    for (const d of cd.indidrivers || []) {
      const li = el("li");
      li.appendChild(el("span", { class: "spacer small", text: `${d.label} (${d.family})${active.has(d.binary) ? " ● running" : ""}` }));
      li.appendChild(el("button", { class: "btn btn-sm", onclick: () => client.indiLoadDriver(d.binary) }, "Load"));
      li.appendChild(el("button", { class: "btn btn-sm", onclick: () => client.indiReloadDriver(d.binary) }, "Reload"));
      li.appendChild(el("button", { class: "btn btn-sm btn-danger", onclick: () => client.indiStopDriver(d.binary) }, "Stop"));
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
  const btn = el("button", { class: "folder-btn" }, [icon("folder", { small: true }), el("span", { text: node.name || "(root)" })]);
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
    root.appendChild(el("h2", null, [icon("folder"), el("span", { text: " Files" })]));
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
    listWrap.appendChild(el("div", { class: "muted small mono", text: `Selected: ${files.selectedfolder || "(none)"}` }));
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
    if (!(files.files || []).length) fgrid.appendChild(el("div", { class: "muted small", text: "No files in this folder." }));
    listWrap.appendChild(fgrid);
    layout.appendChild(listWrap);
    root.appendChild(layout);
  });

  return { node: root, dispose: scope.dispose };
}

export function renderLogs(client) {
  const root = el("div", { class: "logs-view" });
  const scope = client.scope();
  const LEVELS = ["debug", "info", "warning", "error", "critical"];

  scope.subscribe(client.topics.logs(), (logs) => {
    root.innerHTML = "";
    root.appendChild(el("h2", null, [icon("receipt_long"), el("span", { text: " Logs" })]));
    root.appendChild(el("div", { class: "log-toolbar muted small", text: `${(logs || []).length} entries (capped at 1000)` }));
    const table = el("table", { class: "log-table" });
    for (const entry of (logs || []).slice().reverse()) {
      const lvl = LEVELS[entry.l] || "info";
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
