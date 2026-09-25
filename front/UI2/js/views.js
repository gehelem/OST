// UI2/js/views.js — the non-module screens: sidebar, controller/global
// settings, file browser, log panel, login dialog. Adapted from
// ostharfang4/js/views.js to consume core/ (a `client` object) instead of
// ostharfang4's own datastore.js/ws.js. Screens that stay on-screen and
// need live data (controller/files/logs) now subscribe themselves and
// return {node, dispose}; the sidebar and login dialog don't hold any
// subscription open, so they just return a node.

function el(tag, attrs, children) {
  const node = document.createElement(tag);
  if (attrs) for (const [k, v] of Object.entries(attrs)) {
    if (k === "class") node.className = v;
    else if (k === "text") node.textContent = v;
    else if (k.startsWith("on") && typeof v === "function") node.addEventListener(k.slice(2), v);
    else if (v !== undefined && v !== null) node.setAttribute(k, v);
  }
  if (children) for (const c of [].concat(children)) {
    if (c === null || c === undefined) continue;
    node.appendChild(typeof c === "string" ? document.createTextNode(c) : c);
  }
  return node;
}

export function renderSidebar(client, activeKey, onSelect) {
  const nav = el("nav", { class: "sidebar" });
  const staticItems = [
    { key: "__controller", label: "Controller", icon: "settings" },
    { key: "__files", label: "Files", icon: "folder" },
    { key: "__logs", label: "Logs", icon: "receipt_long" },
  ];
  for (const item of staticItems) {
    const btn = el("button", { class: "nav-item" + (activeKey === item.key ? " active" : ""), onclick: () => onSelect(item.key) }, [
      el("span", { class: "material-icons icon-sm" }, item.icon),
      el("span", { text: item.label }),
    ]);
    nav.appendChild(btn);
  }
  nav.appendChild(el("div", { class: "nav-sep" }));
  const names = client.get(client.topics.modules());
  for (const name of names) {
    const mod = client.get(client.topics.module(name));
    const label = (mod && (mod.infos.modulelabel || mod.infos.label)) || name;
    const btn = el("button", { class: "nav-item" + (activeKey === name ? " active" : ""), onclick: () => onSelect(name) }, [
      el("span", { text: label }),
    ]);
    nav.appendChild(btn);
  }
  return nav;
}

export function renderLogin(client, onDone) {
  const overlay = el("div", { class: "dialog-overlay" });
  const user = el("input", { type: "text", class: "field-input", placeholder: "user" });
  const pw = el("input", { type: "password", class: "field-input", placeholder: "password" });
  const dialog = el("div", { class: "dialog" }, [
    el("h3", { text: "Login required" }),
    el("div", { class: "muted", text: "This server requires authentication (grant-client = -1)." }),
    el("div", { class: "form-row" }, [el("label", { text: "User" }), user]),
    el("div", { class: "form-row" }, [el("label", { text: "Password" }), pw]),
    el("div", { class: "dialog-actions" }, [
      el("button", { class: "btn btn-primary", onclick: () => { client.login(user.value, pw.value); overlay.remove(); if (onDone) onDone(); } }, "Login"),
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
    root.appendChild(el("h2", { text: "Controller" }));
    if (cd.banner) root.appendChild(el("div", { class: "banner", text: cd.banner }));
    if (cd.git) root.appendChild(el("div", { class: "muted small", text: `${cd.git.Gittag || ""} ${cd.git.Githash || ""} — ${cd.git.Gitdate || ""}` }));

    const sw = cd.systemwatcher;
    if (sw) {
      const sec = el("section", { class: "panel" }, [el("h3", { text: "Server load" })]);
      if (sw.cpu) sec.appendChild(el("div", { text: `CPU load (1/5/15m): ${sw.cpu.load_1m} / ${sw.cpu.load_5m} / ${sw.cpu.load_15m}` }));
      if (sw.ram) sec.appendChild(el("div", { text: `RAM: ${fmtBytes(sw.ram.used_mb)} / ${fmtBytes(sw.ram.total_mb)}` }));
      if (sw.disks && sw.disks.length) {
        const ul = el("ul");
        for (const d of sw.disks) ul.appendChild(el("li", { text: `${d.mountpoint} (${d.device}): ${d.used_gb.toFixed(1)} / ${d.total_gb.toFixed(1)} GB` }));
        sec.appendChild(ul);
      }
      if (sw.network && sw.network.length) {
        const ul = el("ul");
        for (const n of sw.network) ul.appendChild(el("li", { text: `${n.iface}: ↓${n.rx_kbps.toFixed(1)} kbps ↑${n.tx_kbps.toFixed(1)} kbps` }));
        sec.appendChild(ul);
      }
      if (sw.temperatures && sw.temperatures.length) {
        const ul = el("ul");
        for (const t of sw.temperatures) ul.appendChild(el("li", { text: `${t.type}: ${t.temp_c.toFixed(1)} °C` }));
        sec.appendChild(ul);
      }
      root.appendChild(sec);
    }

    const confSec = el("section", { class: "panel" }, [el("h3", { text: "Configurations" })]);
    const confs = cd.availableconfs || {};
    const confList = el("ul");
    for (const [name, modlist] of Object.entries(confs)) {
      const li = el("li");
      li.appendChild(el("span", { text: `${name}${cd.currentconf === name ? " (current)" : ""} — ${(modlist || []).length} module(s) ` }));
      li.appendChild(el("button", { class: "btn", onclick: () => client.loadConfiguration(name) }, "Load"));
      confList.appendChild(li);
    }
    confSec.appendChild(confList);
    const nameInput = el("input", { type: "text", class: "field-input", placeholder: "configuration name" });
    confSec.appendChild(el("div", { class: "form-row" }, [nameInput, el("button", { class: "btn btn-primary", onclick: () => { if (nameInput.value) client.saveConfiguration(nameInput.value); } }, "Save current as…")]));
    root.appendChild(confSec);

    const modSec = el("section", { class: "panel" }, [el("h3", { text: "Loaded modules" })]);
    const modList = el("ul");
    for (const name of client.get(client.topics.modules())) {
      const li = el("li", { text: name + " " });
      if (name !== "GlobalDatastore") li.appendChild(el("button", { class: "btn btn-danger", onclick: () => { if (confirm(`Unload ${name}?`)) client.killModule(name); } }, "Kill"));
      modList.appendChild(li);
    }
    modSec.appendChild(modList);
    root.appendChild(modSec);

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
      el("button", { class: "btn btn-primary", onclick: () => { if (labelInput.value) client.loadModule(libSelect.value, labelInput.value, profileInput.value || undefined); } }, "Load"),
    ]));
    root.appendChild(libSec);

    const indiSec = el("section", { class: "panel" }, [el("h3", { text: `INDI server (${cd.indiserver === "Y" ? "enabled" : "disabled"})` })]);
    indiSec.appendChild(el("div", { class: "form-row" }, [
      el("button", { class: "btn", onclick: () => client.indiStart() }, "Start server"),
      el("button", { class: "btn", onclick: () => client.indiStop() }, "Stop server"),
    ]));
    const active = new Set((cd.indiactivedrivers || []).map((d) => d.binary));
    const driversList = el("ul");
    for (const d of cd.indidrivers || []) {
      const li = el("li", { text: `${d.label} (${d.family}) ${active.has(d.binary) ? "● running" : ""} ` });
      li.appendChild(el("button", { class: "btn", onclick: () => client.indiLoadDriver(d.binary) }, "Load"));
      li.appendChild(el("button", { class: "btn", onclick: () => client.indiReloadDriver(d.binary) }, "Reload"));
      li.appendChild(el("button", { class: "btn btn-danger", onclick: () => client.indiStopDriver(d.binary) }, "Stop"));
      driversList.appendChild(li);
    }
    indiSec.appendChild(driversList);
    root.appendChild(indiSec);
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
  const btn = el("button", { class: "folder-btn" }, [
    el("span", { class: "material-icons icon-sm" }, "folder"),
    el("span", { text: node.name || "(root)" }),
  ]);
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
    root.appendChild(el("h2", { text: "Files" }));
    const layout = el("div", { class: "files-layout" });
    const tree = buildFolderTree(files.folders || []);
    const treeWrap = el("div", { class: "folder-tree" });
    // The selected folder in the dump is an absolute server-side path, not
    // the relative path used for navigation/FS - we cannot reliably
    // highlight the active node from it alone (see FINDINGS.md), so no
    // node is marked "active" here.
    treeWrap.appendChild(renderFolderNode(tree, client, 0));
    layout.appendChild(treeWrap);

    const listWrap = el("div", { class: "file-list-wrap" });
    listWrap.appendChild(el("div", { class: "muted small", text: `Selected: ${files.selectedfolder || "(none)"}` }));
    const grid = el("div", { class: "file-grid" });
    for (const f of files.files || []) {
      const url = mediaBaseUrl + (f.startsWith("/") ? f.slice(1) : f);
      const lower = f.toLowerCase();
      const isImg = /\.(jpe?g|png|gif|webp)$/.test(lower);
      const card = el("a", { href: url, target: "_blank", class: "file-card" });
      if (isImg) card.appendChild(el("img", { src: url, class: "file-thumb" }));
      else card.appendChild(el("span", { class: "material-icons" }, "description"));
      card.appendChild(el("div", { class: "file-name small", text: f.split("/").pop() }));
      grid.appendChild(card);
    }
    listWrap.appendChild(grid);
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
    root.appendChild(el("h2", { text: "Logs" }));
    const table = el("table", { class: "log-table" });
    for (const entry of (logs || []).slice().reverse()) {
      const lvl = LEVELS[entry.l] || "info";
      table.appendChild(el("tr", { class: "log-row log-" + lvl }, [
        el("td", { class: "small muted", text: entry.d }),
        el("td", { class: "small", text: entry.c }),
        el("td", { text: entry.t }),
      ]));
    }
    root.appendChild(table);
  });

  return { node: root, dispose: scope.dispose };
}
