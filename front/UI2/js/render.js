// UI2/js/render.js — generic module/property/element rendering. Adapted
// from ostharfang4/js/render.js to consume core/ (a `client` object, see
// ../../core/index.js) instead of ostharfang4's own datastore.js/ws.js.
//
// The key structural change from the original: instead of app.js pushing
// updates down into property nodes via a global onChange + node lookup
// (`container._update()` called externally), every property/module screen
// now subscribes *directly* to exactly the core topic it renders and
// disposes that subscription itself. See README.md "UI2: adapting
// ostharfang4" for the full comparison.
//
// One rendering path is used for every module (no per-template screens):
// see client-pattern.en.md "Templates: per-module-type screens with a
// generic fallback" — this is the "default" screen, which is allowed to be
// the only screen some module types ever get, so it must not drop info.

import { renderGraph } from "./chart.js";
import { mediaUrl as buildMediaUrl } from "../../core/index.js";

const DISPLAY_ONLY_TYPES = new Set(["img", "video", "light", "prg", "message"]);
const STATUS_CLASS = ["standby", "ok", "busy", "error"];
const STATUS_LABEL = ["Standby", "OK", "Busy", "Error"];

let mediaBaseUrl = "";
export function setMediaBaseUrl(url) { mediaBaseUrl = url; }
export function getMediaBaseUrl() { return mediaBaseUrl; }

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

function icon(name, opts) {
  const span = el("span", { class: "material-icons" + (opts && opts.small ? " icon-sm" : "") });
  span.textContent = name;
  return span;
}

function statusDot(status) {
  const s = typeof status === "number" ? status : 0;
  return el("span", { class: "status-dot status-" + STATUS_CLASS[s], title: STATUS_LABEL[s] });
}

function iconButton(name, title, onClick) {
  return el("button", { class: "icon-btn", title, onclick: onClick }, [icon(name)]);
}

// ---- element value formatting / editing --------------------------------

function formatScalar(type, value) {
  if (value === undefined || value === null) return "—";
  switch (type) {
    case "date": return value.year !== undefined ? `${value.year}-${String(value.month).padStart(2, "0")}-${String(value.day).padStart(2, "0")}` : "—";
    case "time": return value.hh !== undefined ? `${String(value.hh).padStart(2, "0")}:${String(value.mm).padStart(2, "0")}:${String(value.ss).padStart(2, "0")}` : "—";
    case "datetime": return value.year !== undefined ? `${value.year}-${String(value.month).padStart(2, "0")}-${String(value.day).padStart(2, "0")} ${String(value.hh).padStart(2, "0")}:${String(value.mm).padStart(2, "0")}:${String(value.ss).padStart(2, "0")}` : "—";
    case "bool": return value ? "true" : "false";
    default: return String(value);
  }
}

// Builds the input control for one element, wired to a shared buffer object
// (buffer[elName] holds the live edit value). Returns the DOM node.
function buildEditControl(client, moduleName, el_, elName, buffer, opts) {
  const type = el_.type;
  const lov = client.resolveLov(moduleName, el_);
  if (lov) {
    const sel = el("select", { class: "field-input" });
    if (el_.nullable) sel.appendChild(el("option", { value: "" }, "—"));
    for (const [k, label] of Object.entries(lov)) {
      const opt = el("option", { value: k }, label);
      if (String(buffer[elName]) === String(k)) opt.selected = true;
      sel.appendChild(opt);
    }
    sel.addEventListener("change", () => {
      buffer[elName] = sel.value === "" ? null : (type === "int" ? parseInt(sel.value, 10) : type === "float" ? parseFloat(sel.value) : sel.value);
      if (opts && opts.onChange) opts.onChange();
    });
    return sel;
  }
  if (type === "int" || type === "float") {
    if (el_.slider === 1 || el_.slider === 2) {
      const wrap = el("span", { class: "slider-wrap" });
      const range = el("input", { type: "range", class: "field-input", min: el_.min, max: el_.max, step: el_.step || (type === "float" ? 0.1 : 1), value: buffer[elName] });
      const out = el("span", { class: "slider-val", text: String(buffer[elName]) });
      range.addEventListener("input", () => {
        const v = type === "int" ? parseInt(range.value, 10) : parseFloat(range.value);
        buffer[elName] = v;
        out.textContent = String(v);
        if (opts && opts.onChange) opts.onChange();
      });
      wrap.appendChild(range);
      if (el_.slider === 2) wrap.appendChild(out);
      return wrap;
    }
    const input = el("input", { type: "number", class: "field-input", min: el_.min, max: el_.max, step: el_.step || (type === "float" ? "any" : 1), value: buffer[elName] });
    input.addEventListener("change", () => {
      const v = input.value === "" ? null : (type === "int" ? parseInt(input.value, 10) : parseFloat(input.value));
      buffer[elName] = v;
      if (opts && opts.onChange) opts.onChange();
    });
    return input;
  }
  if (type === "bool") {
    const cb = el("input", { type: "checkbox", class: "field-checkbox" });
    cb.checked = !!buffer[elName];
    cb.addEventListener("change", () => { buffer[elName] = cb.checked; if (opts && opts.onChange) opts.onChange(); });
    return cb;
  }
  if (type === "string") {
    const input = el("input", { type: "text", class: "field-input", value: buffer[elName] ?? "" });
    input.addEventListener("change", () => { buffer[elName] = input.value; if (opts && opts.onChange) opts.onChange(); });
    return input;
  }
  if (type === "date") {
    const v = buffer[elName] || {};
    const wrap = el("span", { class: "compound-input" });
    const y = el("input", { type: "number", class: "field-input tiny", value: v.year ?? "", placeholder: "Y" });
    const mo = el("input", { type: "number", class: "field-input tiny", value: v.month ?? "", placeholder: "M" });
    const d = el("input", { type: "number", class: "field-input tiny", value: v.day ?? "", placeholder: "D" });
    function sync() { buffer[elName] = { year: parseInt(y.value, 10), month: parseInt(mo.value, 10), day: parseInt(d.value, 10) }; if (opts && opts.onChange) opts.onChange(); }
    [y, mo, d].forEach((i) => i.addEventListener("change", sync));
    wrap.append(y, mo, d);
    return wrap;
  }
  if (type === "time") {
    const v = buffer[elName] || {};
    const wrap = el("span", { class: "compound-input" });
    const hh = el("input", { type: "number", class: "field-input tiny", value: v.hh ?? "", placeholder: "hh" });
    const mm = el("input", { type: "number", class: "field-input tiny", value: v.mm ?? "", placeholder: "mm" });
    const ss = el("input", { type: "number", class: "field-input tiny", value: v.ss ?? "", placeholder: "ss" });
    function sync() { buffer[elName] = { hh: parseInt(hh.value, 10) || 0, mm: parseInt(mm.value, 10) || 0, ss: parseInt(ss.value, 10) || 0, ms: v.ms || 0 }; if (opts && opts.onChange) opts.onChange(); }
    [hh, mm, ss].forEach((i) => i.addEventListener("change", sync));
    wrap.append(hh, mm, ss);
    return wrap;
  }
  if (type === "datetime") {
    const v = buffer[elName] || {};
    const wrap = el("span", { class: "compound-input" });
    const y = el("input", { type: "number", class: "field-input tiny", value: v.year ?? "", placeholder: "Y" });
    const mo = el("input", { type: "number", class: "field-input tiny", value: v.month ?? "", placeholder: "M" });
    const d = el("input", { type: "number", class: "field-input tiny", value: v.day ?? "", placeholder: "D" });
    const hh = el("input", { type: "number", class: "field-input tiny", value: v.hh ?? "", placeholder: "hh" });
    const mm = el("input", { type: "number", class: "field-input tiny", value: v.mm ?? "", placeholder: "mm" });
    const ss = el("input", { type: "number", class: "field-input tiny", value: v.ss ?? "", placeholder: "ss" });
    function sync() {
      buffer[elName] = { year: parseInt(y.value, 10) || 0, month: parseInt(mo.value, 10) || 0, day: parseInt(d.value, 10) || 0, hh: parseInt(hh.value, 10) || 0, mm: parseInt(mm.value, 10) || 0, ss: parseInt(ss.value, 10) || 0, ms: v.ms || 0 };
      if (opts && opts.onChange) opts.onChange();
    }
    [y, mo, d, hh, mm, ss].forEach((i) => i.addEventListener("change", sync));
    wrap.append(y, mo, d, hh, mm, ss);
    return wrap;
  }
  // fallback
  return el("input", { type: "text", class: "field-input", value: JSON.stringify(buffer[elName]) });
}

function renderDisplayOnly(elName, el_) {
  const type = el_.type;
  const value = el_.value;
  if (type === "img") {
    const v = value || {};
    const wrap = el("div", { class: "img-elt" });
    if (v.urljpeg || v.urlthumbnail) {
      const path = v.urljpeg || v.urlthumbnail;
      const src = buildMediaUrl(mediaBaseUrl, path) + "?t=" + Date.now();
      const a = el("a", { href: buildMediaUrl(mediaBaseUrl, path), target: "_blank" }, [
        el("img", { src, class: "preview-img" }),
      ]);
      wrap.appendChild(a);
    } else {
      wrap.appendChild(el("div", { class: "muted", text: "No image" }));
    }
    const stats = [];
    if (v.width) stats.push(`${v.width}×${v.height}`);
    if (v.stars !== undefined) stats.push(`${v.stars} stars`);
    if (v.hfravg !== undefined) stats.push(`HFR ${v.hfravg.toFixed?.(2) ?? v.hfravg}`);
    if (v.snr !== undefined) stats.push(`SNR ${v.snr.toFixed?.(2) ?? v.snr}`);
    if (v.issolved) stats.push(`solved (${v.solverra?.toFixed?.(3)}, ${v.solverde?.toFixed?.(3)})`);
    if (stats.length) wrap.appendChild(el("div", { class: "img-stats muted", text: stats.join(" · ") }));
    return wrap;
  }
  if (type === "video") {
    const v = value || {};
    const url = v.url;
    if (!url) return el("div", { class: "muted", text: "No video" });
    return el("video", { src: buildMediaUrl(mediaBaseUrl, url), controls: "true", class: "preview-video" });
  }
  if (type === "light") return statusDot(value);
  if (type === "prg") {
    const v = value || { value: 0, dynlabel: "" };
    const wrap = el("div", { class: "prg" });
    if (el_.prgtype === "spinner") wrap.appendChild(el("span", { class: "spinner" }));
    else wrap.appendChild(el("div", { class: "prg-bar-outer" }, [el("div", { class: "prg-bar-inner", style: `width:${Math.max(0, Math.min(100, v.value || 0))}%` })]));
    if (v.dynlabel) wrap.appendChild(el("span", { class: "prg-label", text: v.dynlabel }));
    return wrap;
  }
  if (type === "message") return el("div", { class: "muted", text: "(message elements are not implemented by the current backend)" });
  return el("div", { text: formatScalar(type, value) });
}

function iconSlotButtons(moduleName, propName, obj, client, elName) {
  const row = el("span", { class: "icon-slots" });
  const slots = elName
    ? [["preicon", 1], ["posticon", 2]]
    : [["preicon1", 1], ["preicon2", 2], ["posticon1", 3], ["posticon2", 4]];
  for (const [field, slotNum] of slots) {
    const name = obj[field];
    if (!name) continue;
    row.appendChild(iconButton(name, field, () => {
      if (elName) client.clickElementIcon(slotNum, moduleName, propName, elName);
      else client.clickPropertyIcon(slotNum, moduleName, propName);
    }));
  }
  return row;
}

// ---- generic elements form (reused by property SA form, grid create/edit) --

export function buildElementsForm(client, moduleName, elementDefs, order, initialValues) {
  const buffer = {};
  for (const name of order) {
    const def = elementDefs[name] || { type: "string" };
    buffer[name] = initialValues && name in initialValues ? initialValues[name] : def.value;
  }
  const table = el("table", { class: "form-table" });
  for (const name of order) {
    const def = elementDefs[name] || { type: "string" };
    const tr = el("tr");
    tr.appendChild(el("td", { class: "form-label", text: def.label || name }));
    const td = el("td");
    td.appendChild(buildEditControl(client, moduleName, def, name, buffer, {}));
    tr.appendChild(td);
    table.appendChild(tr);
  }
  return { node: table, getValues: () => Object.assign({}, buffer) };
}

// ---- property rendering -------------------------------------------------

function editableElementNames(prop) {
  if (prop.permission === 0) return [];
  return Object.keys(prop.elements).filter((n) => !DISPLAY_ONLY_TYPES.has(prop.elements[n].type));
}

function renderGridSection(client, moduleName, propName, prop) {
  const wrap = el("div", { class: "grid-section" });
  const headers = prop.gridheaders || [];
  const table = el("table", { class: "grid-table" });
  const thead = el("tr");
  for (const h of headers) {
    const def = prop.elements[h];
    thead.appendChild(el("th", { text: def ? def.label || h : h }));
  }
  thead.appendChild(el("th", { text: "" }));
  table.appendChild(el("thead", null, [thead]));
  const tbody = el("tbody");
  (prop.rows || []).forEach((row, idx) => {
    const tr = el("tr", { class: "grid-row" });
    for (const h of headers) {
      const def = prop.elements[h] || { type: "string" };
      tr.appendChild(el("td", { text: formatScalar(def.type, row[h]) }));
    }
    const actions = el("td", { class: "grid-actions" });
    actions.appendChild(iconButton("edit", "Edit row", () => openGridDialog(client, moduleName, propName, prop, idx, row)));
    actions.appendChild(iconButton("arrow_upward", "Move up", () => client.gridMoveUp(moduleName, propName, idx)));
    actions.appendChild(iconButton("arrow_downward", "Move down", () => client.gridMoveDown(moduleName, propName, idx)));
    actions.appendChild(iconButton("delete", "Delete row", () => { if (confirm("Delete this row?")) client.gridDelete(moduleName, propName, idx); }));
    tr.appendChild(actions);
    tr.addEventListener("click", (ev) => { if (ev.target.closest(".grid-actions")) return; client.gridFetch(moduleName, propName, idx); });
    tbody.appendChild(tr);
  });
  table.appendChild(tbody);
  wrap.appendChild(table);

  const toolbar = el("div", { class: "grid-toolbar" });
  toolbar.appendChild(el("button", { class: "btn", onclick: () => openGridDialog(client, moduleName, propName, prop, null, null) }, "Add row"));
  toolbar.appendChild(el("button", { class: "btn btn-danger", onclick: () => { if (confirm("Clear the whole grid?")) client.gridReset(moduleName, propName); } }, "Reset grid"));
  if (prop.gridLimit) toolbar.appendChild(el("span", { class: "muted", text: ` (${(prop.rows || []).length}/${prop.gridLimit} rows)` }));
  wrap.appendChild(toolbar);

  if (prop.hasGraph) {
    const canvas = el("canvas", { class: "graph-canvas", width: "640", height: "260" });
    wrap.appendChild(canvas);
    requestAnimationFrame(() => renderGraph(canvas, prop));
  }
  return wrap;
}

let dialogRoot = null;
export function setDialogRoot(node) { dialogRoot = node; }

function openGridDialog(client, moduleName, propName, prop, index, row) {
  const headers = prop.gridheaders || [];
  const form = buildElementsForm(client, moduleName, prop.elements, headers, row || {});
  const overlay = el("div", { class: "dialog-overlay" });
  const dialog = el("div", { class: "dialog" }, [
    el("h3", { text: index === null ? "Add row" : `Edit row ${index}` }),
    form.node,
    el("div", { class: "dialog-actions" }, [
      el("button", { class: "btn", onclick: () => overlay.remove() }, "Cancel"),
      el("button", { class: "btn btn-primary", onclick: () => {
        const values = form.getValues();
        if (index === null) client.gridCreate(moduleName, propName, values);
        else client.gridUpdate(moduleName, propName, index, values);
        overlay.remove();
      } }, index === null ? "Add" : "Save"),
    ]),
  ]);
  overlay.appendChild(dialog);
  (dialogRoot || document.body).appendChild(overlay);
}

function boolGroupNames(prop) {
  return Object.entries(prop.elements).filter(([, e]) => e.type === "bool").map(([n]) => n);
}

function renderPropertyBody(client, moduleName, propName, prop) {
  const body = el("div", { class: "prop-body" });

  if (prop.freevalue) body.appendChild(el("div", { class: "freevalue", text: prop.freevalue }));

  if (prop.hasGrid) {
    body.appendChild(renderGridSection(client, moduleName, propName, prop));
    if (!prop.showElts) return body;
  }

  const editableNames = editableElementNames(prop);
  const buffer = {};
  for (const name of editableNames) buffer[name] = prop.elements[name].value;

  const boolGroup = (prop.rule === 0 || prop.rule === 1) ? boolGroupNames(prop) : [];

  function sendGroupChoice(chosen) {
    const payload = {};
    for (const n of editableNames) payload[n] = boolGroup.includes(n) ? (n === chosen) : buffer[n];
    client.setAll(moduleName, propName, payload);
  }

  const table = el("table", { class: "elt-table" });
  for (const [name, e] of Object.entries(prop.elements)) {
    const tr = el("tr", { class: "elt-row" });
    const labelCell = el("td", { class: "elt-label" });
    if (e.preicon) labelCell.appendChild(iconSlotButtons(moduleName, propName, e, client, name));
    const labelSpan = el("span", { text: e.label || name, title: e.hint || "" });
    labelCell.appendChild(labelSpan);
    if (e.badge) labelCell.appendChild(el("span", { class: "badge-star", text: "★" }));
    tr.appendChild(labelCell);

    const valueCell = el("td", { class: "elt-value" });

    if (DISPLAY_ONLY_TYPES.has(e.type)) {
      valueCell.appendChild(renderDisplayOnly(name, e));
    } else if (boolGroup.includes(name) && (prop.permission === 1 || prop.permission === 2) && prop.enabled !== false) {
      const radio = el("input", { type: "radio", name: `grp-${moduleName}-${propName}` });
      radio.checked = !!e.value;
      radio.addEventListener("change", () => sendGroupChoice(name));
      valueCell.appendChild(radio);
    } else if ((prop.permission === 1 || prop.permission === 2) && prop.enabled !== false) {
      if (e.type === "bool" && e.directedit) {
        const btn = el("button", { class: "btn toggle-btn" + (e.value ? " active" : ""), onclick: () => client.setValue(moduleName, propName, name, !e.value) }, e.value ? "On" : "Off");
        valueCell.appendChild(btn);
      } else {
        const ctrl = buildEditControl(client, moduleName, e, name, buffer, {});
        valueCell.appendChild(ctrl);
        if (e.directedit) valueCell.appendChild(iconButton("send", "Send now (SV)", () => client.setValue(moduleName, propName, name, buffer[name])));
      }
    } else {
      valueCell.appendChild(el("span", { class: "readonly-val", text: formatScalar(e.type, e.value) }));
    }
    if (e.posticon) valueCell.appendChild(iconSlotButtons(moduleName, propName, e, client, name));
    tr.appendChild(valueCell);
    table.appendChild(tr);
  }
  body.appendChild(table);

  const formable = editableNames.filter((n) => !boolGroup.includes(n) && !(prop.elements[n].type === "bool" && prop.elements[n].directedit));
  if (formable.length && (prop.permission === 1 || prop.permission === 2) && prop.enabled !== false) {
    body.appendChild(el("button", { class: "btn btn-primary", onclick: () => client.setAll(moduleName, propName, buffer) }, "Save (SA)"));
  }

  return body;
}

/**
 * Renders one property and keeps itself in sync by subscribing directly to
 * `client.topics.property(moduleName, propName)` — the fine-grained, only
 * source of updates for this node (see core/datastore.js). Returns
 * {node, dispose}; the caller (renderModuleScreen) must call dispose() when
 * the node is discarded, or the subscription leaks.
 */
export function renderProperty(client, moduleName, propName) {
  const container = el("div", { class: "property" });
  container.dataset.module = moduleName;
  container.dataset.prop = propName;

  const unsubscribe = client.subscribe(client.topics.property(moduleName, propName), (prop) => {
    container.innerHTML = "";
    if (!prop) { container.classList.add("hidden"); return; }
    container.classList.remove("hidden");
    const header = el("div", { class: "prop-header" });
    header.appendChild(statusDot(prop.status));
    if (prop.preicon1 || prop.preicon2) header.appendChild(iconSlotButtons(moduleName, propName, prop, client));
    header.appendChild(el("span", { class: "prop-label", text: prop.label || propName }));
    if (prop.badge) header.appendChild(el("span", { class: "badge-star", text: "★" }));
    if (prop.enabled === false) header.appendChild(el("span", { class: "muted small", text: "(disabled)" }));
    header.appendChild(el("span", { class: "spacer" }));
    if (prop.posticon1 || prop.posticon2) {
      const post = el("span", { class: "icon-slots" });
      for (const [field, slot] of [["posticon1", 3], ["posticon2", 4]]) {
        if (prop[field]) post.appendChild(iconButton(prop[field], field, () => client.clickPropertyIcon(slot, moduleName, propName)));
      }
      header.appendChild(post);
    }
    container.appendChild(header);
    container.appendChild(renderPropertyBody(client, moduleName, propName, prop));
  });

  container._dispose = unsubscribe;
  return container;
}

// ---- module layout: tabs by level1, groups by level2, ordered -----------

function buildLayout(properties) {
  const noLevel1 = [];
  const tabsMap = new Map();
  for (const [name, p] of Object.entries(properties)) {
    const level1 = p.level1 || "";
    const level2 = p.level2 || "";
    const order = p.order || "0";
    if (!level1) { noLevel1.push({ name, order }); continue; }
    if (!tabsMap.has(level1)) tabsMap.set(level1, { name: level1, order, groupsMap: new Map(), direct: [] });
    const tab = tabsMap.get(level1);
    if (order < tab.order) tab.order = order;
    if (!level2) { tab.direct.push({ name, order }); continue; }
    if (!tab.groupsMap.has(level2)) tab.groupsMap.set(level2, { name: level2, order, items: [] });
    const grp = tab.groupsMap.get(level2);
    if (order < grp.order) grp.order = order;
    grp.items.push({ name, order });
  }
  noLevel1.sort((a, b) => (a.order > b.order ? 1 : -1));
  const tabs = Array.from(tabsMap.values());
  for (const t of tabs) {
    t.direct.sort((a, b) => (a.order > b.order ? 1 : -1));
    t.groups = Array.from(t.groupsMap.values());
    for (const g of t.groups) g.items.sort((a, b) => (a.order > b.order ? 1 : -1));
    t.groups.sort((a, b) => (a.order > b.order ? 1 : -1));
  }
  tabs.sort((a, b) => (a.order > b.order ? 1 : -1));
  return { noLevel1, tabs };
}

/**
 * Renders a full module screen. Subscribes to `client.topics.module(name)`
 * once for the module-level layout (property set + profile bar — this
 * topic does NOT fire on ordinary ea/ee/ps property traffic, see
 * datastore.js), and lets each `renderProperty()` call own its own
 * fine-grained subscription. Returns {node, dispose}.
 */
export function renderModuleScreen(client, moduleName) {
  const root = el("div", { class: "module-screen" });
  const header = el("div", { class: "module-header" });
  const profileBarSlot = el("div");
  header.appendChild(profileBarSlot);
  root.appendChild(header);
  const layoutSlot = el("div");
  root.appendChild(layoutSlot);

  let propertyDisposers = [];
  let lastPropKey = null;

  function disposeProperties() {
    while (propertyDisposers.length) propertyDisposers.pop()();
  }

  const unsubModule = client.subscribe(client.topics.module(moduleName), (mod) => {
    if (!mod) {
      header.innerHTML = "";
      header.appendChild(el("h2", { text: moduleName }));
      layoutSlot.innerHTML = "";
      layoutSlot.appendChild(el("div", { class: "muted", text: "Module not found (unloaded?)" }));
      disposeProperties();
      lastPropKey = null;
      return;
    }
    header.innerHTML = "";
    header.appendChild(el("h2", { text: mod.infos.modulelabel || mod.infos.label || moduleName }));
    if (mod.infos.description) header.appendChild(el("div", { class: "muted", text: mod.infos.description }));
    header.appendChild(renderProfileBar(client, moduleName, mod));

    const propKey = Object.keys(mod.properties).sort().join(",");
    if (propKey === lastPropKey) return; // property set unchanged: leave existing property nodes (and their own live subscriptions) alone
    lastPropKey = propKey;
    disposeProperties();
    layoutSlot.innerHTML = "";

    const { noLevel1, tabs } = buildLayout(mod.properties);

    if (noLevel1.length) {
      const sec = el("div", { class: "prop-list" });
      for (const { name } of noLevel1) { const node = renderProperty(client, moduleName, name); propertyDisposers.push(node._dispose); sec.appendChild(node); }
      layoutSlot.appendChild(sec);
    }

    if (tabs.length) {
      const tabBar = el("div", { class: "tab-bar" });
      const tabPanels = el("div", { class: "tab-panels" });
      tabs.forEach((tab, i) => {
        const btn = el("button", { class: "tab-btn" + (i === 0 ? " active" : ""), text: tab.name });
        const panel = el("div", { class: "tab-panel" + (i === 0 ? "" : " hidden") });
        for (const { name } of tab.direct) { const node = renderProperty(client, moduleName, name); propertyDisposers.push(node._dispose); panel.appendChild(node); }
        for (const grp of tab.groups) {
          const details = el("details", { class: "prop-group", open: "true" });
          details.appendChild(el("summary", { text: grp.name }));
          for (const { name } of grp.items) { const node = renderProperty(client, moduleName, name); propertyDisposers.push(node._dispose); details.appendChild(node); }
          panel.appendChild(details);
        }
        btn.addEventListener("click", () => {
          tabBar.querySelectorAll(".tab-btn").forEach((b) => b.classList.remove("active"));
          tabPanels.querySelectorAll(".tab-panel").forEach((p) => p.classList.add("hidden"));
          btn.classList.add("active");
          panel.classList.remove("hidden");
        });
        tabBar.appendChild(btn);
        tabPanels.appendChild(panel);
      });
      layoutSlot.appendChild(tabBar);
      layoutSlot.appendChild(tabPanels);
    }
  });

  return { node: root, dispose: () => { unsubModule(); disposeProperties(); } };
}

function renderProfileBar(client, moduleName, mod) {
  const bar = el("div", { class: "profile-bar" });
  bar.appendChild(el("span", { class: "muted", text: "Profile: " }));
  const nameSpan = el("span", { class: "profile-name", text: mod.profile.name || "(none)" });
  bar.appendChild(nameSpan);
  if (mod.profile.changed) bar.appendChild(el("span", { class: "muted small", text: " (unsaved changes)" }));

  // Finding the right profile list for this module instance is unreliable
  // from the dump alone - see FINDINGS.md ("profiles-<template> LOV is
  // never actually sent" and "infos.template does not always match the
  // controllerdata.profiles key, e.g. Sequencer's template is 'sequence'").
  // Best effort: try the documented transform of infos.template, then a
  // case-insensitive/prefix match against controllerdata.profiles' keys,
  // then fall back to letting the user type a profile name by hand.
  const template = mod.infos.template || "";
  let options = [];
  const profilesMap = client.get(client.topics.controllerData("profiles")) || {};
  const transformed = template ? template[0].toUpperCase() + template.slice(1).toLowerCase() : "";
  if (transformed && profilesMap[transformed]) options = profilesMap[transformed];
  else if (template) {
    const tl = template.toLowerCase();
    const matchKey = Object.keys(profilesMap).find((k) => k.toLowerCase() === tl || k.toLowerCase().startsWith(tl) || tl.startsWith(k.toLowerCase()));
    if (matchKey) options = profilesMap[matchKey];
  }
  const select = el("select", { class: "field-input" });
  select.appendChild(el("option", { value: "", text: options.length ? "(choose)" : "(no list resolved - type name)" }));
  for (const name of options) select.appendChild(el("option", { value: name, text: name }));
  const nameInput = el("input", { type: "text", class: "field-input small", placeholder: "profile name" });
  select.addEventListener("change", () => { if (select.value) nameInput.value = select.value; });
  bar.appendChild(select);
  bar.appendChild(nameInput);
  bar.appendChild(el("button", { class: "btn", onclick: () => { const n = nameInput.value || select.value; if (n) client.loadProfile(moduleName, n); } }, "Load"));
  bar.appendChild(el("button", { class: "btn", onclick: () => { const n = nameInput.value || select.value; if (n) client.saveProfile(moduleName, n); } }, "Save"));
  return bar;
}
