// render.js — generic module/property/element rendering on top of core/'s
// `client` facade. Functionally this is the same coverage as
// ostcore/UI2/js/render.js (verified against a real server — see that
// file's header and ostcore/README.md), re-skinned for UI3's design and
// with the DOM helpers factored into dom.js instead of duplicated.
//
// One rendering path is used for every module (no per-module-type
// screens) — see client-pattern.en.md "Templates: per-module-type screens
// with a generic fallback": this generic screen is allowed to be the only
// screen some module types ever get, so it must not drop information.
//
// The structural rule this file follows throughout (client-pattern.en.md
// "Rendering: scope updates to the property that changed", enforced by
// core/datastore.js's per-topic subscribe()): every property/module screen
// subscribes directly to the exact core topic it renders and disposes that
// subscription itself when discarded — never a global re-render.

import { renderGraph } from "./chart.js";
import { mediaUrl as buildMediaUrl } from "../../core/index.js";
import { el, icon, statusDot, iconButton } from "./dom.js";
import * as Panels from "./panels.js";

const DISPLAY_ONLY_TYPES = new Set(["img", "video", "light", "prg", "message"]);

let mediaBaseUrl = "";
export function setMediaBaseUrl(url) { mediaBaseUrl = url; }
export function getMediaBaseUrl() { return mediaBaseUrl; }

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
      const range = el("input", { type: "range", min: el_.min, max: el_.max, step: el_.step || (type === "float" ? 0.1 : 1), value: buffer[elName] });
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
    if (stats.length) wrap.appendChild(el("div", { class: "img-stats muted mono", text: stats.join(" · ") }));
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
  return el("div", { class: "readonly-val", text: formatScalar(type, value) });
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
    tr.appendChild(el("td", { class: "elt-label", text: def.label || name }));
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
    actions.appendChild(iconButton("delete", "Delete row", () => { if (confirm("Delete this row?")) client.gridDelete(moduleName, propName, idx); }, { danger: true }));
    tr.appendChild(actions);
    tr.addEventListener("click", (ev) => { if (ev.target.closest(".grid-actions")) return; client.gridFetch(moduleName, propName, idx); });
    tbody.appendChild(tr);
  });
  table.appendChild(tbody);
  wrap.appendChild(el("div", { class: "grid-table-wrap" }, [table]));

  const toolbar = el("div", { class: "grid-toolbar" });
  toolbar.appendChild(el("button", { class: "btn btn-sm", onclick: () => openGridDialog(client, moduleName, propName, prop, null, null) }, [icon("add", { small: true }), "Add row"]));
  toolbar.appendChild(el("button", { class: "btn btn-sm btn-danger", onclick: () => { if (confirm("Clear the whole grid?")) client.gridReset(moduleName, propName); } }, "Reset grid"));
  if (prop.gridLimit) toolbar.appendChild(el("span", { class: "muted small mono", text: `${(prop.rows || []).length}/${prop.gridLimit} rows` }));
  wrap.appendChild(toolbar);

  if (prop.hasGraph) {
    const canvas = el("canvas", { class: "graph-canvas", width: "640", height: "240" });
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
  overlay.addEventListener("click", (ev) => { if (ev.target === overlay) overlay.remove(); });
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
      const radio = el("input", { type: "radio", class: "field-checkbox", name: `grp-${moduleName}-${propName}`, style: "border-radius:50%" });
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
    body.appendChild(el("button", { class: "btn btn-primary", style: "margin-top:10px", onclick: () => client.setAll(moduleName, propName, buffer) }, [icon("save", { small: true }), "Save"]));
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

// Default panel positions for the first-ever render of a module (no saved
// layout yet in localStorage — see panels.js). Two columns below the header
// row; once the user drags a panel, its own position persists from then on
// and this allocator is never consulted again for that (scope, id).
function autoPos(index) {
  const col = index % 2, row = Math.floor(index / 2);
  return { x: 20 + col * 440, y: 230 + row * 260, w: 420 };
}

/**
 * The dedicated "Parameters" box — ostnoctua's OstWidgets.params(), see
 * widgets.js: content-only (profile load/save toolbar + the module's
 * `hasprofile:true` properties, grouped into tabs by level1), appended
 * into whatever `container` the caller provides. Not shown by default and
 * not one of the draggable floating panels — Gilles, 2026-09-26: it opens
 * on demand from a dedicated gear button (app.js's openParamsDialog) as a
 * focused modal, "au dessus du reste ... prend autant de place que
 * necessaire", not left sitting open alongside everything else.
 *
 * Those properties still also appear in their normal General/level1 panel
 * (ostnoctua's own default.js duplicates them the same way — that flat
 * panel is a complete read/write overview, this box is the profile-centric
 * editing surface). When nothing has a level1 (the common case — e.g.
 * guider's own hasprofile properties all have level1:"") there's nothing
 * to tab between, so no tab bar is shown, matching ostnoctua's own
 * `tabsEl.style.display = groups.length > 1 ? '' : 'none'`.
 *
 * Returns true if it rendered anything (the module has at least one
 * `hasprofile:true` property), false otherwise — the caller decides what
 * "nothing to show" means for its own UI (app.js disables the gear
 * button; nothing else currently calls this without checking first).
 */
export function buildParamsPanel(client, moduleName, mod, track, container) {
  const names = Object.keys(mod.properties).filter((n) => mod.properties[n].hasprofile === true);
  if (!names.length) return false;
  const props = Object.fromEntries(names.map((n) => [n, mod.properties[n]]));
  const { noLevel1, tabs } = buildLayout(props);
  const allTabs = noLevel1.length ? [{ name: "", direct: noLevel1, groups: [] }, ...tabs] : tabs;

  container.appendChild(renderProfileBar(client, moduleName, mod));

  function fillGroup(target, tab) {
    for (const { name } of tab.direct) target.appendChild(track(renderProperty(client, moduleName, name)));
    for (const grp of tab.groups) {
      const details = el("details", { class: "prop-group", open: true }, [el("summary", { text: grp.name })]);
      for (const { name } of grp.items) details.appendChild(track(renderProperty(client, moduleName, name)));
      target.appendChild(details);
    }
  }

  if (allTabs.length <= 1) {
    if (allTabs[0]) fillGroup(container, allTabs[0]);
    return true;
  }
  const tabBar = el("div", { class: "tab-bar" });
  const tabPanels = el("div", { class: "tab-panels" });
  allTabs.forEach((tab, i) => {
    const btn = el("button", { class: "tab-btn" + (i === 0 ? " active" : ""), text: tab.name || "General" });
    const panel = el("div", { class: "tab-panel" + (i === 0 ? "" : " hidden") });
    fillGroup(panel, tab);
    btn.addEventListener("click", () => {
      tabBar.querySelectorAll(".tab-btn").forEach((b) => b.classList.remove("active"));
      tabPanels.querySelectorAll(".tab-panel").forEach((p) => p.classList.add("hidden"));
      btn.classList.add("active");
      panel.classList.remove("hidden");
    });
    tabBar.appendChild(btn);
    tabPanels.appendChild(panel);
  });
  container.appendChild(tabBar);
  container.appendChild(tabPanels);
  return true;
}

/**
 * Renders a module as a set of floating panels (js/panels.js): one "header"
 * panel (icon/label/description), one "actions" panel when the module has a
 * bool-only `actions` property, one panel per level1 group (level2
 * sub-groups as <details>), and a "General" panel for properties with no
 * level1. The profile-centric "Parameters" box (buildParamsPanel above) is
 * NOT one of these — it opens on demand as a modal instead, see app.js.
 * Each panel is independently draggable/resizable/minimizable and
 * remembers its own layout per module (Gilles, 2026-09-25: "des fenêtres
 * volantes déplaçables, regroupées par level1").
 *
 * Subscribes to `client.topics.module(name)` for the module-level layout
 * (property set + profile — this topic does NOT fire on ordinary ea/ee/ps
 * property traffic, see datastore.js) and rebuilds all panels on every
 * emission; panel position/size/collapsed state survives the rebuild
 * because it lives in localStorage, not in the DOM (see panels.js). Each
 * `renderProperty()` call owns its own fine-grained subscription, disposed
 * on rebuild/unmount. Returns {dispose} (panels attach directly to the
 * shared layer set via Panels.setLayer() — there is no single "screen"
 * node to return, unlike the other views in this file).
 */
export function renderModuleView(client, moduleName) {
  Panels.setScope(moduleName);
  Panels.clear();

  let propertyDisposers = [];
  function disposeProperties() { while (propertyDisposers.length) propertyDisposers.pop()(); }
  function track(node) { propertyDisposers.push(node._dispose); return node; }

  function build(mod) {
    disposeProperties();
    Panels.clear();

    const header = Panels.make({ id: "header", title: mod.infos.modulelabel || mod.infos.label || moduleName, x: 20, y: 60, w: 320 });
    header.appendChild(el("div", { class: "muted small", text: mod.infos.description || "" }));
    // Profile load/save moved out to its own dedicated "Parameters" panel
    // below — see buildParamsPanel() — not duplicated here (Gilles,
    // 2026-09-26: pointing out ostnoctua's dedicated OstWidgets.params box).

    const properties = mod.properties;
    const hasActions = properties.actions && Object.values(properties.actions.elements || {}).some((e) => e.type === "bool");
    if (hasActions) {
      const abody = Panels.make({ id: "actions", title: properties.actions.label || "Actions", x: 360, y: 60, w: 280 });
      abody.appendChild(track(renderProperty(client, moduleName, "actions")));
    }
    const layoutProps = hasActions ? Object.fromEntries(Object.entries(properties).filter(([n]) => n !== "actions")) : properties;
    const { noLevel1, tabs } = buildLayout(layoutProps);

    let i = 0;
    if (noLevel1.length) {
      const pos = autoPos(i++);
      const gbody = Panels.make({ id: "general", title: "General", x: pos.x, y: pos.y, w: pos.w });
      for (const { name } of noLevel1) gbody.appendChild(track(renderProperty(client, moduleName, name)));
    }
    for (const tab of tabs) {
      const pos = autoPos(i++);
      const tbody = Panels.make({ id: "lvl-" + tab.name, title: tab.name || "(ungrouped)", x: pos.x, y: pos.y, w: pos.w });
      for (const { name } of tab.direct) tbody.appendChild(track(renderProperty(client, moduleName, name)));
      for (const grp of tab.groups) {
        const details = el("details", { class: "prop-group", open: true }, [el("summary", { text: grp.name })]);
        for (const { name } of grp.items) details.appendChild(track(renderProperty(client, moduleName, name)));
        tbody.appendChild(details);
      }
    }
    // Parameters box: not built here — see buildParamsPanel's doc, it
    // opens on demand from the gear button (app.js) instead.
  }

  const unsubModule = client.subscribe(client.topics.module(moduleName), (mod) => {
    if (!mod) {
      disposeProperties();
      Panels.clear();
      const body = Panels.make({ id: "missing", title: moduleName, x: 20, y: 60, w: 320 });
      body.appendChild(el("div", { class: "muted", text: "Module not found (unloaded?)" }));
      return;
    }
    build(mod);
  });

  return { dispose: () => { unsubModule(); disposeProperties(); Panels.clear(); } };
}

export function renderProfileBar(client, moduleName, mod) {
  const bar = el("div", { class: "profile-bar" });
  bar.appendChild(el("span", { class: "muted", text: "Profile" }));
  const nameSpan = el("span", { class: "profile-name" + (mod.profile.changed ? " changed" : ""), text: mod.profile.name || "(none)" });
  bar.appendChild(nameSpan);

  // Finding the right profile list for this module instance is unreliable
  // from the dump alone - see ostcore/FINDINGS.md ("profiles-<template> LOV
  // is never actually sent" and "infos.template does not always match the
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
  const select = el("select", { class: "field-input small" });
  select.appendChild(el("option", { value: "", text: options.length ? "(choose)" : "(type name)" }));
  for (const name of options) select.appendChild(el("option", { value: name, text: name }));
  const nameInput = el("input", { type: "text", class: "field-input small", placeholder: "profile name" });
  select.addEventListener("change", () => { if (select.value) nameInput.value = select.value; });
  bar.appendChild(select);
  bar.appendChild(nameInput);
  bar.appendChild(el("button", { class: "btn btn-sm", onclick: () => { const n = nameInput.value || select.value; if (n) client.loadProfile(moduleName, n); } }, [icon("folder_open", { small: true }), "Load"]));
  bar.appendChild(el("button", { class: "btn btn-sm", onclick: () => { const n = nameInput.value || select.value; if (n) client.saveProfile(moduleName, n); } }, [icon("save", { small: true }), "Save"]));
  return bar;
}
