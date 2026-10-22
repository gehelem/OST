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
import { mediaUrl as buildMediaUrl } from "../core/index.js";
import { el, icon, statusDot, iconButton } from "./dom.js";
import * as Panels from "./panels.js";
import { renderQuickNav } from "./views.js";
import { buildImageStatsHistogramPanel } from "./imagelayer.js";
import { t } from "./i18n.js";

const DISPLAY_ONLY_TYPES = new Set(["img", "video", "light", "prg", "message"]);

let mediaBaseUrl = "";
export function setMediaBaseUrl(url) { mediaBaseUrl = url; }
export function getMediaBaseUrl() { return mediaBaseUrl; }

// Dev-mode escape hatch (Gilles, 2026-10-03: "j'ai fait ca sur
// Osterix/Rooster, en phase de dev ca fait une bouee de sauvetage : on
// force l'utilisation du template de base ... je suis certain de pouvoir
// toucher toutes les proprietes lorsque le template n'est pas encore bien
// determine") - when on for a given module, app.js's own template dispatch
// (renderContentNow) always uses the generic, level1-grouped fallback
// (renderModuleView) instead of that module's bespoke templates/*.js
// (infos.template), so every property stays reachable while that
// particular template is still being built out. Per-module, not global
// (Gilles, having first gotten the global version: "j'aurais bien aime
// que la portee soit au niveau du module") - persisted as the set of
// module names it's currently on for, under one flat
// "ostcore-ui3.<name>" key like every other cross-cutting UI setting
// already is (app.js's own theme/zoom), rather than one key per module.
const DISABLE_TEMPLATES_KEY = "ostcore-ui3.disableTemplates";
const disabledTemplateModules = new Set((() => {
  try { return JSON.parse(localStorage.getItem(DISABLE_TEMPLATES_KEY)) || []; } catch (e) { return []; }
})());
export function getDisableTemplates(moduleName) { return disabledTemplateModules.has(moduleName); }
export function setDisableTemplates(moduleName, v) {
  if (v) disabledTemplateModules.add(moduleName);
  else disabledTemplateModules.delete(moduleName);
  try { localStorage.setItem(DISABLE_TEMPLATES_KEY, JSON.stringify([...disabledTemplateModules])); } catch (e) { /* ignore */ }
}

// #quicknav-nav / #quicknav-sep (app.js sets both once at boot) — each
// module view (this file's generic renderer, or a bespoke templates/*.js)
// fills the nav zone with its own {id,label} entries every time it
// (re)builds panels; there is no other coordination between them, per
// Panels.togglePanel()'s doc comment. This only owns the *navigation* zone
// and its separator — #quicknav itself (the params actions zone too) is
// shown/hidden by app.js from isModuleView(), since the actions
// stay relevant even for a module with zero level1 groups (Gilles,
// 2026-09-27: "le bouton params ... devrait y figurer" regardless).
let quickNavRoot = null;
let quickNavSep = null;
export function setQuickNavRoot(node, sepNode) { quickNavRoot = node; quickNavSep = sepNode; }
export function updateQuickNav(entries) {
  if (!quickNavRoot) return;
  quickNavRoot.innerHTML = "";
  quickNavRoot.appendChild(renderQuickNav(entries));
  const show = entries.length > 0;
  quickNavRoot.classList.toggle("hidden", !show);
  if (quickNavSep) quickNavSep.classList.toggle("hidden", !show);
}

/** Finds a quicknav button rendered by the last updateQuickNav() call, by
 * its entry `id` - for a template that needs to flip a button's enabled
 * state later from outside the build() that created it (e.g. navigator's
 * "center on mount" button, enabled/disabled from the mountposition
 * property subscription, which doesn't trigger a full module rebuild -
 * see templates/navigator.js's own comment on why not). Returns null if
 * the quicknav isn't mounted or has no such entry right now. */
export function getQuickNavButton(id) {
  return quickNavRoot ? quickNavRoot.querySelector(`[data-panel-id="${id}"]`) : null;
}

// ---- shared "root template" pieces ---------------------------------------
// Every template (this file's own generic renderModuleView below,
// templates/scaffold.js, templates/guider.js, templates/navigator.js) is
// built around the same module subscribe/missing/dispose lifecycle and the
// same "actions" panel - Gilles, 2026-10-0X: "Template de module racine
// [...] c'est le Template a partir duquel tous les autres sont construits,
// un heritage comme en C++ - pour ne pas repeter le code quand je te
// demanderai d'ajouter des trucs sur tous les templates". render.js is
// already the shared import every template uses (renderProperty,
// updateQuickNav, ...), so these live here rather than a new file.

/** One missing-module fallback panel, same shape every template already
 * built independently. */
export function missingModulePanel(moduleName) {
  const b = Panels.make({ id: "missing", title: moduleName, x: 20, y: 92, w: 320 });
  b.appendChild(el("div", { class: "muted", text: t("common.moduleNotFound") }));
}

/**
 * The subscribe/missing-module/dispose boilerplate every template's own
 * `mount(client, moduleName)` otherwise re-implements nearly verbatim.
 * `build(mod)` is called on every module dump/profile event; `extraDispose`
 * (a template's own per-property subscriptions, e.g. render.js's own
 * `disposeProperties` or a template's own `dispose`), if given, runs before
 * Panels.clear() both when the module disappears and on final unmount.
 * Returns the `{dispose}` every template's own `mount()` already returns.
 */
export function moduleLifecycle(client, moduleName, build, extraDispose) {
  const unsub = client.subscribe(client.topics.module(moduleName), (mod) => {
    if (!mod) {
      if (extraDispose) extraDispose();
      Panels.clear();
      updateQuickNav([]);
      missingModulePanel(moduleName);
      return;
    }
    build(mod);
  });
  return { dispose: () => { unsub(); if (extraDispose) extraDispose(); Panels.clear(); updateQuickNav([]); } };
}

/**
 * The "actions" panel (the module's own bool-only command buttons) + its
 * quicknav entry - a no-op if the module has no `actions` property. The
 * quicknav button itself is icon-only (views.js's renderQuickNav() own
 * `iconOnly` field) - Gilles, 2026-10-03: "Fenetre action (deja
 * existante): bouton = juste une icone 'dashboard_customize'", but after
 * seeing "dashboard" (tried first, for a since-fixed font-gap reason - see
 * base.css's header comment) he actually wanted something else regardless
 * of availability: "je veux un truc qui symbolise des boutons multiples" -
 * "apps" (the classic multi-square grid) is a deliberate choice, not a
 * stand-in for "dashboard_customize" (which does resolve fine now). The
 * panel's own title bar is unaffected, it keeps its normal text label.
 *
 * `extra` lets a template fold its own module-specific content into this
 * same panel instead of keeping it in a separate one (Gilles, 2026-10-22,
 * on navigator.js's "Actions" vs "Selection": "la partie optionnelle
 * devrait inclure les choses non redondantes de selection ... ce qui me
 * permettrait de supprimer la fenetre selection"):
 *   - excludeElements: action names to skip from the generic bool-button
 *     render below, because the caller's own `node` already exposes that
 *     same command through a richer control (e.g. navigator.js's
 *     "addtoplanner" goes through a Sequencer-profile picker there, not a
 *     bare toggle).
 *   - node: appended right after the generic buttons, before the last-log
 *     line footer - a single node, or an array of them (each in its own
 *     slot, in order) when more than one piece of content is folded in.
 */
export function renderActionsPanel(client, moduleName, mod, track, quickNavEntries, pos, extra) {
  if (!("actions" in mod.properties)) return;
  const p = { x: 20, y: 210, w: 300, ...pos };
  const label = mod.properties.actions.label || t("common.actions");
  const b = Panels.make({ id: "actions", title: label, x: p.x, y: p.y, w: p.w });
  b.appendChild(track(renderProperty(client, moduleName, "actions", { framed: false, excludeElements: extra && extra.excludeElements })));
  const extraNodes = extra && extra.node ? [].concat(extra.node) : [];
  for (const n of extraNodes) b.appendChild(track(n));
  b.appendChild(track(renderLastLogLine(client, moduleName)));
  quickNavEntries.push({ id: "actions", label, icon: "apps", iconOnly: true });
}

/**
 * This module's own last log line, shown right under its "actions" button
 * row (Gilles, 2026-10-14: "en dessous des boutons on affiche le dernier
 * message de log du module"). Same {d,c,t,l} entries the floating log
 * panel/full Logs view already read (core/datastore.js's `logs` array),
 * filtered to this module's own `c` (source) field, keeping just the
 * latest one.
 */
function renderLastLogLine(client, moduleName) {
  const line = el("div", { class: "actions-last-log muted small" });
  function refresh(logs) {
    const entry = (logs || []).filter((e) => e.c === moduleName).pop();
    line.textContent = entry ? entry.t || "" : "";
    line.title = entry ? String(entry.d || "") : "";
  }
  const unsubscribe = client.subscribe(client.topics.logs(), refresh);
  line._dispose = unsubscribe;
  return line;
}

// ---- element value formatting / editing --------------------------------

// `format` is the element's own display mask ("99.99", ".9999", ...) - a
// count of "9"s after the "." gives the number of decimals to round to.
// Only int/float use it; every other type ignores it.
function formatScalar(type, value, format) {
  if (value === undefined || value === null) return "—";
  switch (type) {
    case "date": return value.year !== undefined ? `${value.year}-${String(value.month).padStart(2, "0")}-${String(value.day).padStart(2, "0")}` : "—";
    case "time": return value.hh !== undefined ? `${String(value.hh).padStart(2, "0")}:${String(value.mm).padStart(2, "0")}:${String(value.ss).padStart(2, "0")}` : "—";
    case "datetime": return value.year !== undefined ? `${value.year}-${String(value.month).padStart(2, "0")}-${String(value.day).padStart(2, "0")} ${String(value.hh).padStart(2, "0")}:${String(value.mm).padStart(2, "0")}:${String(value.ss).padStart(2, "0")}` : "—";
    case "bool": return value ? "true" : "false";
    case "int":
    case "float": {
      if (typeof format === "string" && format.includes(".")) {
        const decimals = format.split(".")[1].length;
        return Number(value).toFixed(decimals);
      }
      return String(value);
    }
    default: return String(value);
  }
}

// Same as formatScalar(), but resolves a LOV-backed value (int, float or
// string alike - lov keys/values round-trip through JSON as strings
// either way, so a plain property lookup works regardless of the
// element's real type) to its label instead of showing the raw key.
// Falls back to formatScalar() when there's no LOV, or the value isn't
// one of its keys (e.g. a stale slot - still shown raw rather than
// hidden, same spirit as buildEditControl's own handling of that case).
function formatScalarWithLov(client, moduleName, def, value) {
  const lov = client.resolveLov(moduleName, def);
  if (lov && value !== undefined && value !== null && lov[value] !== undefined) return lov[value];
  return formatScalar(def.type, value, def.format);
}

// Shallow value-equality covering both scalars and the compound
// date/time/datetime shape ({year,month,...}) - used only to decide
// whether a directedit field is currently dirty (see renderPropertyBody's
// own directedit branch), nothing protocol-facing.
function valuesEqual(a, b) {
  if (a && b && typeof a === "object" && typeof b === "object") {
    const keys = new Set([...Object.keys(a), ...Object.keys(b)]);
    for (const k of keys) if (a[k] !== b[k]) return false;
    return true;
  }
  return a === b;
}

// A bool toggle button's own content: its icon (if it has one — e.g. the
// "stop"/"play_arrow"/"shuffle" preicons Focus/Guider's action elements
// declare) + its label, never a literal "On"/"Off" (Gilles, 2026-09-27:
// "dans le bouton : icone et libellé, pas de on/off, l'apparence doit
// refleter l'etat on/off") — the on/off state itself is shown by the
// button's own .toggle-btn.active styling instead of by its text, matching
// ostnoctua's own actions widget (OstWidgets.actions() in widgets.js).
function boolButtonContent(e, name) {
  const parts = [];
  if (e.preicon) parts.push(icon(e.preicon, { small: true }));
  parts.push(el("span", { text: e.label || name }));
  return parts;
}

// Builds the input control for one element, wired to a shared buffer object
// (buffer[elName] holds the live edit value). Returns the DOM node.
function buildEditControl(client, moduleName, el_, elName, buffer, opts) {
  const type = el_.type;
  const lov = client.resolveLov(moduleName, el_);
  if (lov && el_.lovConstrained) {
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
  if (lov) {
    // lovConstrained: false (structure.en.md: "whether value must belong
    // to the LOV" - this is the default, elementbase.h's own
    // mLovConstrained) - free text, the LOV only offered as suggestions.
    // A native <input list> + <datalist> gives exactly that with no
    // custom combobox widget. Gilles, 2026-10-09: every LOV used to get
    // the constrained <select> above regardless, including
    // devices.camera/.mount/... (indimodule.cpp's giveMeADevice(), shared
    // by nearly every module) - a device not yet in the catalog still has
    // to be typeable by hand, by design, not a bug to route around.
    const listId = "lov-opts-" + (++lovDatalistSeq);
    const datalist = el("datalist", { id: listId });
    for (const label of new Set(Object.values(lov))) datalist.appendChild(el("option", { value: label }));
    const input = el("input", { type: "text", class: "field-input", list: listId, value: buffer[elName] ?? "" });
    // The browser filters the datalist's own suggestion dropdown against
    // whatever text is already in the field, so an existing value hides
    // every other option until it's deleted by hand. Clearing the field
    // on focus restores the full suggestion list; if nothing new was
    // typed or picked before the field is left, the blur handler below
    // puts the original text back rather than committing an empty value.
    let shownValue = input.value;
    input.addEventListener("focus", () => {
      shownValue = input.value;
      input.value = "";
    });
    input.addEventListener("change", () => {
      if (input.value === "") return; // restored on blur below, nothing picked/typed
      buffer[elName] = input.value;
      if (opts && opts.onChange) opts.onChange();
    });
    input.addEventListener("blur", () => {
      if (input.value === "") input.value = shownValue;
    });
    const wrap = el("span", { class: "compound-input" });
    wrap.append(input, datalist);
    return wrap;
  }
  if (type === "int" || type === "float") {
    if (el_.slider === 1 || el_.slider === 2) {
      const wrap = el("span", { class: "slider-wrap" });
      const range = el("input", { type: "range", min: el_.min, max: el_.max, step: el_.step || (type === "float" ? 0.1 : 1), value: buffer[elName] });
      // SliderAndValue (common.h: "Slider with adjacent numeric input") — a
      // real editable field, not a read-only number display next to the
      // slider, so a value the slider's own step can't reach (e.g. a 0.05s
      // exposure on a slider stepping by 0.1) can still be typed by hand —
      // Gilles, 2026-10-05: "parms.exposure je n'ai que le slider et si je
      // veux mettre 0.05 ce n'est pas possible". Synced both ways with the
      // range below; its own step stays as permissive as the plain
      // (non-slider) number input further down, independent of the
      // range's own coarser dragging granularity.
      let numInput = null;
      range.addEventListener("input", () => {
        const v = type === "int" ? parseInt(range.value, 10) : parseFloat(range.value);
        buffer[elName] = v;
        if (numInput) numInput.value = v;
        if (opts && opts.onChange) opts.onChange();
      });
      wrap.appendChild(range);
      if (el_.slider === 2) {
        numInput = el("input", { type: "number", class: "slider-val-input", min: el_.min, max: el_.max, step: el_.step || (type === "float" ? "any" : 1), value: buffer[elName] });
        numInput.addEventListener("change", () => {
          const v = numInput.value === "" ? null : (type === "int" ? parseInt(numInput.value, 10) : parseFloat(numInput.value));
          buffer[elName] = v;
          if (v != null) range.value = v;
          if (opts && opts.onChange) opts.onChange();
        });
        wrap.appendChild(numInput);
      }
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
    // Button, not a checkbox (Gilles, 2026-09-27: "il faut mettre des
    // boutons plutot que des puces ou des cases a cocher") — same
    // .toggle-btn look as the directedit case below, but this one's value
    // only lands in `buffer` (sent on the property's own Save button, see
    // renderPropertyBody's "formable" list), so its own active class is
    // toggled by hand on click, same pattern as the slider control above
    // (no live re-render to refresh from until it's actually sent). Icon
    // and label are static (boolButtonContent()), only the state changes.
    const btn = el("button", { type: "button", class: "btn toggle-btn" + (buffer[elName] ? " active" : "") }, boolButtonContent(el_, elName));
    btn.addEventListener("click", () => {
      buffer[elName] = !buffer[elName];
      btn.classList.toggle("active", buffer[elName]);
      if (opts && opts.onChange) opts.onChange();
    });
    return btn;
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

// Shared by renderDisplayOnly's own "img" case and templates/guider.js's
// bespoke background-image stats panel (which shows this line but not the
// thumbnail itself, since the image is already the full background there
// — see imagelayer.js) — one place for the field list/formatting instead
// of two copies drifting apart.
export function imageStatsLine(v) {
  const stats = [];
  if (v.width) stats.push(`${v.width}×${v.height}`);
  // stars/hfravg default to 0 server-side whether or not star detection
  // ever actually ran on this image (ImgData has no "not computed" sentinel,
  // see OST::ImgData, libs/model/element/common.h) - a module that never
  // runs it (indipanel, showing an arbitrary device's BLOB) would otherwise
  // always show a meaningless "0 stars · HFR 0.00". snr is always a real
  // pixel-stats computation regardless, so it has no such gate.
  if (v.stars > 0) stats.push(`${v.stars} stars`);
  if (v.hfravg > 0) stats.push(`HFR ${v.hfravg.toFixed?.(2) ?? v.hfravg}`);
  if (v.snr !== undefined) stats.push(`SNR ${v.snr.toFixed?.(2) ?? v.snr}`);
  if (v.issolved) stats.push(`solved (${v.solverra?.toFixed?.(3)}, ${v.solverde?.toFixed?.(3)})`);
  return stats;
}

// Opens the same full per-channel stats table + histogram chart
// imagelayer.js's own addImageBlock() shows in a persistent floating panel
// - this generic fallback has no per-property quicknav slot to toggle one
// from, so a plain modal dialog instead, same dialog-overlay pattern as
// openPropertyEditDialog() above. Reuses buildImageStatsHistogramPanel()
// rather than duplicating its table/histogram here.
function openImageStatsDialog(client, moduleName, propName, elName, label) {
  const panel = buildImageStatsHistogramPanel(client, moduleName, propName, elName);
  const overlay = el("div", { class: "dialog-overlay" });
  const close = () => { if (panel._dispose) panel._dispose(); overlay.remove(); };
  const dialog = el("div", { class: "dialog" }, [
    el("h3", { text: label || t("imagelayer.imageStats") }),
    panel,
    el("div", { class: "dialog-actions" }, [
      el("button", { class: "btn", onclick: close }, t("common.close")),
    ]),
  ]);
  overlay.appendChild(dialog);
  overlay.addEventListener("click", (ev) => { if (ev.target === overlay) close(); });
  (dialogRoot || document.body).appendChild(overlay);
}

function renderDisplayOnly(elName, el_, ctx) {
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
      wrap.appendChild(el("div", { class: "muted", text: t("common.noImage") }));
    }
    const statsRow = el("div", { class: "img-stats-row" });
    const stats = imageStatsLine(v);
    if (stats.length) statsRow.appendChild(el("div", { class: "img-stats muted mono", text: stats.join(" · ") }));
    if (ctx) {
      statsRow.appendChild(iconButton("bar_chart", t("imagelayer.stats"), () => openImageStatsDialog(ctx.client, ctx.moduleName, ctx.propName, elName, el_.label)));
    }
    if (statsRow.childNodes.length) wrap.appendChild(statsRow);
    return wrap;
  }
  if (type === "video") {
    const v = value || {};
    const url = v.url;
    if (!url) return el("div", { class: "muted", text: t("common.noVideo") });
    return el("video", { src: buildMediaUrl(mediaBaseUrl, url), controls: "true", class: "preview-video" });
  }
  if (type === "light") return statusDot(value);
  if (type === "prg") {
    const v = value || { value: 0, dynlabel: "" };
    const pct = Math.max(0, Math.min(100, v.value || 0));
    const wrap = el("div", { class: "prg" });
    if (el_.prgtype === "spinner") {
      // A real determinate ring (OstErix/Rooster's own mat-progress-spinner
      // mode="determinate"), not an indeterminate always-spinning circle —
      // see components.css's own header comment on this.
      wrap.appendChild(el("div", { class: "prg-ring-wrap" }, [
        el("div", { class: "prg-ring", style: `--pct:${pct}` }),
        el("span", { class: "prg-ring-value", text: Math.round(pct) + "%" }),
      ]));
    } else {
      wrap.appendChild(el("div", { class: "prg-bar-outer" }, [el("div", { class: "prg-bar-inner", style: `width:${pct}%` })]));
    }
    if (v.dynlabel) wrap.appendChild(el("span", { class: "prg-label", text: v.dynlabel }));
    return wrap;
  }
  if (type === "message") return el("div", { class: "muted", text: "(message elements are not implemented by the current backend)" });
  return el("div", { class: "readonly-val", text: formatScalar(type, value, el_.format) });
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

// An element's own "order" (string, same lexicographic comparison every
// level1/level2/property sort in this file already uses) - NOT the
// object's own key order, which is not reliable for this: Qt's
// QJsonObject serializes object keys alphabetically, confirmed against a
// live dump of inspector's own "actions" property (8 buttons, order
// 10-90) coming back as "goextra/gohome/gointra/loop/reload/sethome/
// shoot/stop" - alphabetical by key name, regardless of their real
// "order" field (Gilles, 2026-10-0X, chasing an unrelated "Shoot button
// does nothing" report that turned out to be a panel overlap instead -
// this ordering bug was still real and worth fixing on its own).
function orderedElementNames(elements) {
  return Object.keys(elements).sort((a, b) => (String(elements[a].order) > String(elements[b].order) ? 1 : -1));
}

function editableElementNames(prop) {
  if (prop.permission === 0) return [];
  return orderedElementNames(prop.elements).filter((n) => !DISPLAY_ONLY_TYPES.has(prop.elements[n].type));
}

// `showGrid` (default false — PropertyMulti's own default, see
// propertymulti.h/propertyfactory.cpp) gates the raw table+toolbar below,
// independent of the graph canvas: a graph property is always also a grid
// property (hasGrid:true, structure.en.md), but typically ships
// showGrid:false — e.g. guider.json's own drift/guiding properties — since
// only the chart is meant to show by default, not an editable table of the
// same data (Gilles, 2026-09-27: "pour une propriete avec grille tu ne
// tiens pas compte de showgrid/showelts" — this was rendering both
// unconditionally).
function renderGridSection(client, moduleName, propName, prop) {
  const wrap = el("div", { class: "grid-section" });

  if (prop.showGrid) {
    // gridheaders comes back alphabetical by element key (PropertyMulti::
    // getGridHeaders() returns mElts.keys(), a QMap - same ordering bug
    // orderedElementNames() above was already written to work around for
    // non-grid properties) - sort by each element's own "order" instead.
    const headers = (prop.gridheaders || []).slice().sort((a, b) => (String(prop.elements[a].order) > String(prop.elements[b].order) ? 1 : -1));
    const table = el("table", { class: "grid-table" });
    const thead = el("tr");
    for (const h of headers) {
      const def = prop.elements[h];
      thead.appendChild(el("th", { text: def ? def.label || h : h }));
    }
    thead.appendChild(el("th", { text: "" }));
    table.appendChild(el("thead", null, [thead]));
    const tbody = el("tbody");
    // permission:0 (ReadOnly, e.g. sequencer's "livesequence" - a run-time
    // snapshot the backend itself writes, issue #241) also rejects every
    // grid mutation server-side (Basemodule::onExternalEventBase) - don't
    // show controls that would just fail.
    const editable = prop.permission !== 0;
    (prop.rows || []).forEach((row, idx) => {
      const tr = el("tr", { class: "grid-row" });
      for (const h of headers) {
        const def = prop.elements[h] || { type: "string" };
        if (def.type === "prg" || def.type === "light") tr.appendChild(el("td", null, [renderDisplayOnly(h, Object.assign({}, def, { value: row[h] }))]));
        else tr.appendChild(el("td", { text: formatScalarWithLov(client, moduleName, def, row[h]) }));
      }
      if (editable) {
        const actions = el("td", { class: "grid-actions" });
        actions.appendChild(iconButton("edit", t("common.editRow"), () => openGridDialog(client, moduleName, propName, prop, idx, row)));
        actions.appendChild(iconButton("arrow_upward", t("common.moveUp"), () => client.gridMoveUp(moduleName, propName, idx)));
        actions.appendChild(iconButton("arrow_downward", t("common.moveDown"), () => client.gridMoveDown(moduleName, propName, idx)));
        actions.appendChild(iconButton("delete", t("common.deleteRow"), () => { if (confirm(t("common.deleteRowConfirm"))) client.gridDelete(moduleName, propName, idx); }, { danger: true }));
        tr.appendChild(actions);
        tr.addEventListener("click", (ev) => { if (ev.target.closest(".grid-actions")) return; client.gridFetch(moduleName, propName, idx); });
      }
      tbody.appendChild(tr);
    });
    table.appendChild(tbody);
    wrap.appendChild(el("div", { class: "grid-table-wrap" }, [table]));

    if (editable) {
      const toolbar = el("div", { class: "grid-toolbar" });
      toolbar.appendChild(el("button", { class: "btn btn-sm", onclick: () => openGridDialog(client, moduleName, propName, prop, null, null) }, [icon("add", { small: true }), "Add row"]));
      toolbar.appendChild(el("button", { class: "btn btn-sm btn-danger", onclick: () => { if (confirm("Clear the whole grid?")) client.gridReset(moduleName, propName); } }, "Reset grid"));
      if (prop.gridLimit) toolbar.appendChild(el("span", { class: "muted small mono", text: `${(prop.rows || []).length}/${prop.gridLimit} rows` }));
      wrap.appendChild(toolbar);
    }
  }

  if (prop.hasGraph) {
    const canvas = el("canvas", { class: "graph-canvas", width: "640", height: "240" });
    wrap.appendChild(canvas);
    requestAnimationFrame(() => renderGraph(canvas, prop));
  }
  return wrap;
}

let dialogRoot = null;
export function setDialogRoot(node) { dialogRoot = node; }

// Unique <datalist> id per unconstrained-LOV control built (buildEditControl
// above) - several can be on screen in the same form at once.
let lovDatalistSeq = 0;

function openGridDialog(client, moduleName, propName, prop, index, row) {
  // Same ordering fix as renderGridSection() above.
  const headers = (prop.gridheaders || []).slice().sort((a, b) => (String(prop.elements[a].order) > String(prop.elements[b].order) ? 1 : -1));
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

// Excludes directedit elements (Gilles, 2026-09-27, having read a real "SA"
// dump for a directedit-only "actions" property: "tu dois avoir le droit
// d'envoyer juste un SV/SE juste sur autofocus:true") — client-messages.en.md
// allows SV per-element whenever directedit is true, regardless of the
// property's own rule (OneOfMany/AtMostOne are how e.g. Focus's own
// abortfocus/autofocus/loop actions are declared, purely so the backend
// keeps them mutually exclusive server-side; that's independent of how the
// value gets there). A "boolGroup" needing the batched, all-elements SA
// send is only the elements that have no directedit path of their own.
function boolGroupNames(prop) {
  return orderedElementNames(prop.elements).filter((n) => prop.elements[n].type === "bool" && !prop.elements[n].directedit);
}

// Elements only reachable through the "Edit property" dialog (the pencil
// button in the property's own header), never inline - Gilles, 2026-10-0X,
// rejecting the previous design (inline-editable fields + a bottom bulk
// "Save" button, each directedit field *also* getting its own send arrow):
// "la seule vraie facon de gerer cette mise a jour 'par lot' des elements
// d'une propriete est de passer par un formulaire - fonctionnellement c'est
// exactement ca : on soumet plusieurs valeurs a mettre a jour en meme
// temps [...] les elements qui ont directedit=false ne peuvent etre mis a
// jour que via le formulaire" - mirroring OstErix/Rooster's own
// property.component.ts (directedit elements edit inline/immediately,
// confirmed read against its source) + DatastoreEditDialogComponent (one
// detached buffer, one SA on submit). The pre-existing exclusive bool-group
// mechanic (rule 0/1, its own immediate-send buttons, see boolGroupNames()
// above) is a separate, already-approved exception and stays out of this
// list too, same as before.
export function formableElementNames(prop) {
  const boolGroup = (prop.rule === 0 || prop.rule === 1) ? boolGroupNames(prop) : [];
  return editableElementNames(prop).filter((n) => !boolGroup.includes(n) && prop.elements[n].directedit !== true);
}

/**
 * The "Edit property" dialog itself - same detached-snapshot pattern as
 * openGridDialog() above (the form's buffer is copied from the live values
 * once, up front, then never reads the datastore again until Save), just
 * for a property's own `formable` elements instead of one grid row.
 *
 * The SA payload sent on Save must carry *every* editable element of the
 * property, not just the ones this form shows - client-messages.en.md's
 * own "Always send every element" warning: "If any is missing, the
 * backend applies nothing at all — not even the elements that were
 * present". The elements this form doesn't show (directedit ones, the
 * exclusive bool-group's own current choice) ride along unchanged, read
 * fresh from `prop` at submit time.
 */
function openPropertyEditDialog(client, moduleName, propName, prop, formable) {
  const initial = {};
  for (const n of formable) initial[n] = prop.elements[n].value;
  const form = buildElementsForm(client, moduleName, prop.elements, formable, initial);
  const overlay = el("div", { class: "dialog-overlay" });
  const dialog = el("div", { class: "dialog" }, [
    el("h3", { text: prop.label || propName }),
    form.node,
    el("div", { class: "dialog-actions" }, [
      el("button", { class: "btn", onclick: () => overlay.remove() }, t("common.cancel")),
      el("button", { class: "btn btn-primary", onclick: () => {
        const values = form.getValues();
        const payload = {};
        for (const n of editableElementNames(prop)) payload[n] = n in values ? values[n] : prop.elements[n].value;
        client.setAll(moduleName, propName, payload);
        overlay.remove();
      } }, t("common.save")),
    ]),
  ]);
  overlay.appendChild(dialog);
  overlay.addEventListener("click", (ev) => { if (ev.target === overlay) overlay.remove(); });
  (dialogRoot || document.body).appendChild(overlay);
}

// excludeElements: element names skipped entirely (e.g. a caller showing
// the same action through its own richer control elsewhere - see
// renderActionsPanel()'s own doc comment).
function renderPropertyBody(client, moduleName, propName, prop, editTracker, excludeElements) {
  const body = el("div", { class: "prop-body" });

  // freevalue is documented as a generic display field (structure.en.md),
  // but in practice the only module that ever populates it is indipanel -
  // and purely as internal plumbing (indipanel.cpp: "we keep original
  // device name to avoid unwanted level1 device translations"), not a
  // real annotation meant for the user. Showing it repeated the INDI
  // device's own name under every single property's label - Gilles,
  // 2026-10-09, on a QHY camera panel: "ne l'affiche pas".

  if (prop.hasGrid && (prop.showGrid || prop.hasGraph)) body.appendChild(renderGridSection(client, moduleName, propName, prop));
  // showElts (default true — same source as showGrid above) gates the
  // per-element table below regardless of hasGrid: previously only checked
  // inside the hasGrid branch, so a plain (non-grid) property with
  // showElts:false still showed its elt-table unconditionally.
  if (prop.showElts === false) return body;

  // Current values only - not a two-way edit buffer anymore (see
  // formableElementNames()'s own doc comment on why): the only thing left
  // reading this is sendGroupChoice() below, which needs every OTHER
  // element's current value to complete its own SA payload.
  const editableNames = editableElementNames(prop);
  const currentValues = {};
  for (const name of editableNames) currentValues[name] = prop.elements[name].value;

  const boolGroup = (prop.rule === 0 || prop.rule === 1) ? boolGroupNames(prop) : [];

  function sendGroupChoice(chosen) {
    const payload = {};
    for (const n of editableNames) payload[n] = boolGroup.includes(n) ? (n === chosen) : currentValues[n];
    client.setAll(moduleName, propName, payload);
  }

  const table = el("table", { class: "elt-table" });
  // A run of consecutive self-descriptive buttons (isGroupBtn/isDirectBtn,
  // below) shares ONE row, laid out horizontally - Gilles, 2026-10-03: "si
  // on a une suite de boutons il sont affiches les uns au dessus des
  // autres verticalement, en particulier pour 'actions' - c'est moche".
  // Flushed (made into an actual table row) as soon as a non-button
  // element breaks the run, or at the very end - so buttons interspersed
  // with other elements still land in their own correctly-ordered spot,
  // only genuinely adjacent ones group together.
  let buttonRun = null;
  function flushButtonRun() {
    if (!buttonRun) return;
    const tr = el("tr", { class: "elt-row elt-row-buttons" });
    const td = el("td", { class: "elt-value", colspan: "2" });
    td.appendChild(buttonRun);
    tr.appendChild(td);
    table.appendChild(tr);
    buttonRun = null;
  }
  for (const name of orderedElementNames(prop.elements)) {
    if (excludeElements && excludeElements.includes(name)) continue;
    const e = prop.elements[name];
    const permOk = (prop.permission === 1 || prop.permission === 2) && prop.enabled !== false;
    const isGroupBtn = boolGroup.includes(name) && permOk;
    const isDirectBtn = !isGroupBtn && permOk && e.type === "bool" && e.directedit;

    if (isGroupBtn || isDirectBtn) {
      // Self-descriptive button (icon + label already inside it, see
      // boolButtonContent()) — no separate label cell alongside it, or the
      // same icon/label shows up twice on the same row (Gilles, 2026-09-27,
      // having seen exactly that: "tu as laisse les libelles des boutons
      // !!! c'est ridicule"). sendGroupChoice() sends immediately via SA
      // (the whole group); the directedit case sends immediately via SV
      // (setValue) — the behavior Gilles asked to keep either way.
      if (!buttonRun) buttonRun = el("div", { class: "elt-button-row" });
      const send = isGroupBtn ? () => sendGroupChoice(name) : () => client.setValue(moduleName, propName, name, !e.value);
      buttonRun.appendChild(el("button", { class: "btn toggle-btn" + (e.value ? " active" : ""), onclick: send }, boolButtonContent(e, name)));
      continue;
    }
    flushButtonRun();

    const tr = el("tr", { class: "elt-row" });
    const labelCell = el("td", { class: "elt-label" });
    if (e.preicon) labelCell.appendChild(iconSlotButtons(moduleName, propName, e, client, name));
    const labelSpan = el("span", { text: e.label || name, title: e.hint || "" });
    labelCell.appendChild(labelSpan);
    if (e.badge) labelCell.appendChild(el("span", { class: "badge-star", text: "★" }));
    tr.appendChild(labelCell);

    const valueCell = el("td", { class: "elt-value" });

    if (DISPLAY_ONLY_TYPES.has(e.type)) {
      valueCell.appendChild(renderDisplayOnly(name, e, e.type === "img" ? { client, moduleName, propName } : null));
    } else if (permOk && e.directedit) {
      // Only a directedit element gets an inline control here - anything
      // else (directedit:false) is display-only in this table, editable
      // only through the property's own "Edit property" dialog (the
      // pencil button, see formableElementNames()'s doc comment on why).
      // A local one-field buffer is enough - there's no bulk Save left to
      // share it with.
      //
      // A constrained LOV (native <select>) is already a complete,
      // deliberate choice the instant it changes, so it sends
      // immediately, same as the bool buttons above. Everything else -
      // free-typed fields (number/text/date/time) and a non-constrained
      // LOV (free-text input with a <datalist> of suggestions, still
      // typeable by hand) - behaves like free text and instead commits on
      // leaving the field (focusout), not on every keystroke: see the
      // else branch below.
      const fieldBuffer = { [name]: e.value };
      const lov = client.resolveLov(moduleName, e);
      if (lov && e.lovConstrained) {
        const ctrl = buildEditControl(client, moduleName, e, name, fieldBuffer, {
          onChange: () => client.setValue(moduleName, propName, name, fieldBuffer[name]),
        });
        valueCell.appendChild(ctrl);
      } else {
        // Commits on focusout (leaving the field), not per keystroke, and
        // while focused this property stops redrawing from backend
        // updates (editTracker.begin()/end()) so a live push never
        // overwrites what's being typed. A yellow icon + cancel cross only
        // appear once the value actually differs from the server's; the
        // cross cancels (reverts, no send) instead of committing - its
        // pointerdown is intercepted with preventDefault so the browser
        // never shifts focus away from the field first, which would
        // otherwise fire focusout (and commit) before the cross's own
        // click is handled. Escape does the same cancel. On commit the
        // icon turns green for a moment before the row is redrawn.
        let began = false;
        let dirty = false;
        const statusIcon = el("span", { class: "field-edit-icon" }, [icon("check", { small: true })]);
        function finish(send) {
          if (!began) return;
          began = false;
          statusIcon.classList.remove("editing", "sent");
          cancelBtn.classList.remove("visible");
          if (send) {
            client.setValue(moduleName, propName, name, fieldBuffer[name]);
            statusIcon.classList.add("sent");
            setTimeout(() => editTracker.end(), 500);
          } else {
            editTracker.end();
          }
        }
        const cancelBtn = iconButton("close", t("common.cancel"), () => finish(false));
        cancelBtn.classList.add("field-cancel-btn");
        cancelBtn.addEventListener("pointerdown", (ev) => ev.preventDefault());
        const ctrl = buildEditControl(client, moduleName, e, name, fieldBuffer, {
          onChange: () => {
            dirty = !valuesEqual(fieldBuffer[name], e.value);
            statusIcon.classList.toggle("editing", dirty);
            cancelBtn.classList.toggle("visible", dirty);
          },
        });
        valueCell.addEventListener("focusin", () => {
          if (began) return;
          began = true;
          editTracker.begin();
        });
        valueCell.addEventListener("focusout", (ev) => {
          if (valueCell.contains(ev.relatedTarget)) return;
          finish(dirty);
        });
        valueCell.addEventListener("keydown", (ev) => {
          if (ev.key === "Escape") { ev.preventDefault(); finish(false); }
        });
        valueCell.appendChild(ctrl);
        valueCell.appendChild(statusIcon);
        valueCell.appendChild(cancelBtn);
      }
    } else {
      valueCell.appendChild(el("span", { class: "readonly-val", text: formatScalarWithLov(client, moduleName, e, e.value) }));
    }
    if (e.posticon) valueCell.appendChild(iconSlotButtons(moduleName, propName, e, client, name));
    tr.appendChild(valueCell);
    table.appendChild(tr);
  }
  flushButtonRun(); // a run ending at the very last element (actions' own common case) still needs flushing
  body.appendChild(table);

  return body;
}

/**
 * Renders one property and keeps itself in sync by subscribing directly to
 * `client.topics.property(moduleName, propName)` — the fine-grained, only
 * source of updates for this node (see core/datastore.js). Returns
 * {node, dispose}; the caller (renderModuleScreen) must call dispose() when
 * the node is discarded, or the subscription leaks.
 *
 * `opts.framed` (default true) - whether to "encadrer" the property's own
 * elements in their own bordered/background box with a labelled header
 * (statusDot/preicons/label/badge/disabled-flag/posticons) above them.
 * Gilles, 2026-10-03, having noticed the actions window showing its own
 * label twice (once as the window's own title, once again here), then
 * "les elements de la propriete restent tout de meme entoures par un
 * cadre, ca mange de la place pour rien": "il faut lui passer un
 * parametre pour dire si on doit encadrer les elements dans une boite
 * avec le libelle, ou pas ... je veux garder un socle commun pour
 * afficher des propriétés" - one shared function for every caller (this
 * file's own generic fallback, every template) rather than a bespoke
 * fork, so a caller whose OWN window chrome already frames the property
 * (render.js's own renderActionsPanel(), currently the only one) can ask
 * for the bare element list instead of a second, redundant frame around it.
 */
export function renderProperty(client, moduleName, propName, opts) {
  const framed = !opts || opts.framed !== false;
  const container = el("div", { class: framed ? "property" : "property property-bare" });
  container.dataset.module = moduleName;
  container.dataset.prop = propName;

  // While a free-typed directedit field is being edited (see
  // renderPropertyBody's own else branch), redraws are frozen so a
  // backend push never overwrites what's being typed - editingCount
  // tracks how many fields are currently mid-edit (focused), and the
  // latest prop received meanwhile is drawn as soon as it drops back to
  // zero, whether that's from a commit or a cancel.
  let editingCount = 0;
  let currentProp = null;

  function draw(prop) {
    container.innerHTML = "";
    if (!prop) { container.classList.add("hidden"); return; }
    container.classList.remove("hidden");
    if (framed) {
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
      // Pencil button - the only entry point for the property's own
      // "Edit property" dialog (formableElementNames()'s own doc comment
      // has the full story); last in the header, like OstErix/Rooster's
      // own property-edit-button placement after the posticons.
      const formable = formableElementNames(prop);
      if (formable.length && (prop.permission === 1 || prop.permission === 2) && prop.enabled !== false) {
        header.appendChild(iconButton("edit", t("common.editProperty"), () => openPropertyEditDialog(client, moduleName, propName, prop, formable)));
      }
      container.appendChild(header);
    }
    container.appendChild(renderPropertyBody(client, moduleName, propName, prop, editTracker, opts && opts.excludeElements));
  }

  const editTracker = {
    begin() { editingCount++; },
    end() {
      editingCount = Math.max(0, editingCount - 1);
      if (editingCount === 0) draw(currentProp);
    },
  };

  const unsubscribe = client.subscribe(client.topics.property(moduleName, propName), (prop) => {
    currentProp = prop;
    if (editingCount === 0) draw(prop);
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
 * Fills a level1 panel's body: level2 sub-groups as tabs (Gilles,
 * 2026-09-27: "essaie de me presenter les level2 sous forme de tab s'il y
 * en a") — replaces an earlier always-open <details> accordion for the
 * same groups. `tab.direct` (properties with this level1 but no level2)
 * becomes its own leading, unnamed "General" tab when there's anything
 * else to tab against; no tab bar at all when there's only one group to
 * show (nothing to tab between), same rule buildParamsPanel's own level1
 * tabs already follow.
 */
function fillLevel1Panel(container, client, moduleName, track, tab) {
  const groups = tab.direct.length ? [{ name: "", items: tab.direct }, ...tab.groups] : tab.groups;
  if (groups.length <= 1) {
    for (const { name } of (groups[0] ? groups[0].items : [])) container.appendChild(track(renderProperty(client, moduleName, name)));
    return;
  }
  const tabBar = el("div", { class: "tab-bar" });
  const tabPanels = el("div", { class: "tab-panels" });
  groups.forEach((grp, i) => {
    const btn = el("button", { class: "tab-btn" + (i === 0 ? " active" : ""), text: grp.name || "General" });
    const panel = el("div", { class: "tab-panel" + (i === 0 ? "" : " hidden") });
    for (const { name } of grp.items) panel.appendChild(track(renderProperty(client, moduleName, name)));
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
}

// Default panel positions for the first-ever render of a module (no saved
// layout yet in localStorage — see panels.js). Two columns below the header
// row; once the user drags a panel, its own position persists from then on
// and this allocator is never consulted again for that (scope, id).
function autoPos(index) {
  const col = index % 2, row = Math.floor(index / 2);
  return { x: 20 + col * 440, y: 230 + row * 260, w: 420 };
}

// Plain key/value listing of mod.infos (modulelabel/description/template/
// library/classname/families, plus undocumented internal build/version
// fields - baseGithash, thisversion, startdatetime, "and others",
// structure.en.md's own words - never individually listed there, so shown
// generically here rather than hardcoding which ones exist). Gilles,
// 2026-10-03: "ajoute un onglet qui affiche les infos du module
// (startdatetime baseGitdate etc etc) sans fioritures, juste les couples
// clef/valeur" - reuses .metric-row (views.js's own Controller screen:
// cpu/ram/disk load), the existing plain label/value row style, rather
// than inventing a new one.
function renderInfosTab(mod) {
  const wrap = el("div");
  for (const [key, value] of Object.entries(mod.infos || {})) {
    const text = Array.isArray(value) ? value.join(", ") : typeof value === "object" && value !== null ? JSON.stringify(value) : String(value);
    wrap.appendChild(el("div", { class: "metric-row" }, [el("span", { class: "label", text: key }), el("span", { class: "value", text })]));
  }
  return wrap;
}

/**
 * The dedicated "Parameters" box — ostnoctua's OstWidgets.params(), see
 * widgets.js: content-only (profile load/save toolbar + the module's
 * `hasprofile:true` properties, grouped into tabs by level1, plus a
 * plain-key/value "Info" tab for mod.infos — Gilles, 2026-10-03), appended
 * into whatever `container` the caller provides. Not shown by default and
 * not one of the draggable floating panels — Gilles, 2026-09-26: it opens
 * on demand from a dedicated gear button (app.js's openParamsDialog) as a
 * focused modal, "au dessus du reste ... prend autant de place que
 * necessaire", not left sitting open alongside everything else.
 *
 * Those properties still also appear in their normal General/level1 panel
 * (ostnoctua's own default.js duplicates them the same way — that flat
 * panel is a complete read/write overview, this box is the profile-centric
 * editing surface). When there's only one tab to show (the common case —
 * e.g. guider's own hasprofile properties all have level1:"", and it has
 * no other tab either) there's nothing to tab between, so no tab bar is
 * shown, matching ostnoctua's own
 * `tabsEl.style.display = groups.length > 1 ? '' : 'none'`. (This box's own
 * level2 groups still show as <details> — see fillLevel1Panel() below for
 * the level1 panel's own level2-as-tabs treatment.)
 *
 * Returns true if it rendered anything (the module has at least one
 * `hasprofile:true` property, or any infos at all - virtually always, so
 * in practice this is never false), false otherwise — the caller decides
 * what "nothing to show" means for its own UI (app.js disables the gear
 * button; nothing else currently calls this without checking first).
 *
 * `onTemplateToggle`, if given, is called whenever the profile bar's own
 * dev-mode checkbox (getDisableTemplates(), see its own doc comment)
 * flips - app.js passes its own renderContentNow so the change is visible
 * right away, without closing this modal.
 */
export function buildParamsPanel(client, moduleName, mod, track, container, onTemplateToggle) {
  const names = Object.keys(mod.properties).filter((n) => mod.properties[n].hasprofile === true);
  const hasInfos = mod.infos && Object.keys(mod.infos).length > 0;
  if (!names.length && !hasInfos) return false;
  const props = Object.fromEntries(names.map((n) => [n, mod.properties[n]]));
  const { noLevel1, tabs } = buildLayout(props);
  const allTabs = noLevel1.length ? [{ name: "", direct: noLevel1, groups: [] }, ...tabs] : tabs;
  if (hasInfos) allTabs.push({ name: t("common.moduleInfo"), infosTab: true });

  // Nothing to load/save without a hasprofile property - a bare Info tab
  // (no profile-backed settings at all) skips the profile bar instead of
  // showing one with nothing underneath it to act on.
  if (names.length) container.appendChild(track(renderProfileBar(client, moduleName, mod, onTemplateToggle)));

  function fillGroup(target, tab) {
    if (tab.infosTab) { target.appendChild(renderInfosTab(mod)); return; }
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
 * Renders a module as a set of floating panels (js/panels.js): one
 * "actions" panel when the module has a bool-only `actions` property, one
 * panel per level1 group (level2 sub-groups as tabs when there are any —
 * see fillLevel1Panel()), and a "General" panel for properties with no
 * level1. The profile-centric "Parameters" box (buildParamsPanel above) is
 * NOT one of these — it opens on demand as a modal instead, see app.js.
 * Each panel is independently draggable/resizable and remembers its own
 * layout per module (Gilles, 2026-09-25: "des fenêtres volantes
 * déplaçables, regroupées par level1") — shown/hidden only through its own
 * quicknav button, no minimize of its own any more (Gilles, 2026-10-03).
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
    // Profile load/save lives in its own dedicated "Parameters" panel
    // instead — see buildParamsPanel() — not duplicated here (Gilles,
    // 2026-09-26: pointing out ostnoctua's dedicated OstWidgets.params box).
    // No "header" panel either (name/description) any more - Gilles,
    // 2026-10-03: "la supprimer completement", the module's own name is
    // already shown in the module bar above, this was a window with no
    // quicknav button of its own to hide it.

    const properties = mod.properties;
    const quickNavEntries = [];
    // Bool-only check kept here rather than folded into
    // renderActionsPanel()'s own simpler "has the property at all" rule
    // (every other template uses that simpler one) - this generic fallback
    // is the one screen that also has to keep a non-bool-only `actions`
    // property OUT of its own panel below, laying it out with the rest of
    // the level1/level2 grid like any other property instead.
    const hasActions = properties.actions && Object.values(properties.actions.elements || {}).some((e) => e.type === "bool");
    if (hasActions) renderActionsPanel(client, moduleName, mod, track, quickNavEntries, { x: 360, y: 92, w: 280 });
    const layoutProps = hasActions ? Object.fromEntries(Object.entries(properties).filter(([n]) => n !== "actions")) : properties;
    const { noLevel1, tabs } = buildLayout(layoutProps);

    let i = 0;
    // One quicknav button per panel built below (Gilles, 2026-09-27: "un
    // bouton par level1 et un pour level1='' si necessaire") — reuses the
    // exact same noLevel1/tabs this loop already computed, no separate pass.
    if (noLevel1.length) {
      const pos = autoPos(i++);
      const gbody = Panels.make({ id: "general", title: t("common.general"), x: pos.x, y: pos.y, w: pos.w });
      for (const { name } of noLevel1) gbody.appendChild(track(renderProperty(client, moduleName, name)));
      quickNavEntries.push({ id: "general", label: t("common.general") });
    }
    for (const tab of tabs) {
      const pos = autoPos(i++);
      const tbody = Panels.make({ id: "lvl-" + tab.name, title: tab.name || "(ungrouped)", x: pos.x, y: pos.y, w: pos.w });
      fillLevel1Panel(tbody, client, moduleName, track, tab);
      quickNavEntries.push({ id: "lvl-" + tab.name, label: tab.name || "(ungrouped)" });
    }
    updateQuickNav(quickNavEntries);
    // Parameters box: not built here — see buildParamsPanel's doc, it
    // opens on demand from the gear button (app.js) instead.
  }

  return moduleLifecycle(client, moduleName, build, disposeProperties);
}

/**
 * Renders the profile load/save bar. Self-subscribes to `client.topics.lovs()`
 * (the controller-scope lov map) so the profile dropdown stays live — it used
 * to read client.get(...) once at build time from a snapshot, so it never
 * refreshed after the very first render, and separately keyed off
 * infos.template (fragile, see the same class of bug already found on
 * Sequencer's "sequence"/"sequencer" mismatch). Fixed 2026-09-26, live with
 * Gilles right after finding the equivalent bug in OstErix: profiles are
 * per module TYPE, exposed as the controller lov "profiles-<Type>" keyed by
 * the exact-case infos.classname (new field, added server-side the same
 * day) - not infos.template, and not a one-shot read.
 */
// Load/save-as both need the user to pick/type a name - rather than
// cramming a <select> and a text field into the bar itself (Gilles,
// 2026-10-15, on the previous 3-row version: "ça prend trop de place"),
// each opens its own small dialog, same overlay/dialog pattern as
// openGridDialog() above. `client.topics.lovs()` is only subscribed while
// the Load dialog is actually open - unsubscribed on close either way
// (picked a profile or cancelled).
function openLoadProfileDialog(client, moduleName, mod) {
  const classname = mod.infos.classname || "";
  const select = el("select", { class: "field-input" });
  const unsubscribe = client.subscribe(client.topics.lovs(), (lovs) => {
    const lov = (classname && lovs && lovs["profiles-" + classname]) || null;
    const options = (lov && Object.keys(lov.values || {})) || [];
    const current = select.value;
    select.innerHTML = "";
    select.appendChild(el("option", { value: "", text: options.length ? t("common.chooseProfile") : t("common.noProfilesYet") }));
    for (const name of options) select.appendChild(el("option", { value: name, text: name }));
    if (options.includes(current)) select.value = current;
  });
  const overlay = el("div", { class: "dialog-overlay" });
  const close = () => { unsubscribe(); overlay.remove(); };
  const dialog = el("div", { class: "dialog" }, [
    el("h3", { text: t("common.loadAProfile") }),
    select,
    el("div", { class: "dialog-actions" }, [
      el("button", { class: "btn", onclick: close }, t("common.cancel")),
      el("button", { class: "btn btn-primary", onclick: () => { if (select.value) client.loadProfile(moduleName, select.value); close(); } }, t("common.loadProfile")),
    ]),
  ]);
  overlay.appendChild(dialog);
  overlay.addEventListener("click", (ev) => { if (ev.target === overlay) close(); });
  (dialogRoot || document.body).appendChild(overlay);
}

function openSaveAsProfileDialog(client, moduleName) {
  const nameInput = el("input", { type: "text", class: "field-input", placeholder: t("common.profileNamePlaceholder") });
  const overlay = el("div", { class: "dialog-overlay" });
  const dialog = el("div", { class: "dialog" }, [
    el("h3", { text: t("common.saveAsNewProfile") }),
    nameInput,
    el("div", { class: "dialog-actions" }, [
      el("button", { class: "btn", onclick: () => overlay.remove() }, t("common.cancel")),
      el("button", { class: "btn btn-primary", onclick: () => { if (nameInput.value) client.saveProfile(moduleName, nameInput.value); overlay.remove(); } }, t("common.save")),
    ]),
  ]);
  overlay.appendChild(dialog);
  overlay.addEventListener("click", (ev) => { if (ev.target === overlay) overlay.remove(); });
  (dialogRoot || document.body).appendChild(overlay);
}

export function renderProfileBar(client, moduleName, mod, onTemplateToggle) {
  const bar = el("div", { class: "profile-bar" });
  bar.appendChild(el("span", { class: "muted", text: t("common.profile") }));
  const nameSpan = el("span", { class: "profile-name", text: mod.profile.name || "(none)" });
  bar.appendChild(nameSpan);
  // "Just save what I'm on" needs no selection/typing at all (Gilles,
  // 2026-10-15: previously had to re-pick it from a dropdown first).
  const saveCurrentBtn = el("button", {
    class: "btn btn-sm", disabled: !mod.profile.name,
    onclick: () => client.saveProfile(moduleName, currentMod.profile.name),
  }, [icon("save", { small: true }), t("common.save")]);
  bar.appendChild(saveCurrentBtn);
  bar.appendChild(el("button", { class: "btn btn-sm", onclick: () => openLoadProfileDialog(client, moduleName, currentMod) }, [icon("folder_open", { small: true }), t("common.loadProfile")]));
  bar.appendChild(el("button", { class: "btn btn-sm", onclick: () => openSaveAsProfileDialog(client, moduleName) }, [icon("save", { small: true }), t("common.saveAsProfile")]));

  // Dev-mode "force the generic template" escape hatch - see
  // getDisableTemplates()'s own doc comment above. Gilles: "ajoute un
  // petit toggle discret a droite du bouton save" - plain small checkbox
  // + label, same clickable-label shape as navigator.js's own settings
  // panel checkboxes, not a dedicated switch widget of its own.
  const templateCb = el("input", { type: "checkbox" });
  templateCb.checked = getDisableTemplates(moduleName);
  templateCb.addEventListener("change", () => {
    setDisableTemplates(moduleName, templateCb.checked);
    if (onTemplateToggle) onTemplateToggle();
  });
  bar.appendChild(el("label", { class: "profile-bar-dev-toggle muted small", title: t("common.disableTemplatesHint"), style: "cursor:pointer" }, [
    templateCb, el("span", { text: t("common.disableTemplates") }),
  ]));

  // The dialog this bar lives in is built once from a snapshot `mod`
  // (app.js's openParamsDialog(), one client.get() call, no subscription)
  // - without this, the name/dot/Save button stayed frozen on whatever
  // profile was current when the dialog opened, never reflecting a
  // Load/Save-as done from the very buttons right here (Gilles,
  // 2026-10-15: "le nom du profil ... ne se met pas à jour"). topics.module()
  // fires on profile events (fs/fl/fc - see this file's own module doc
  // comment on it), so this alone is enough to stay live.
  let currentMod = mod;
  const unsubscribe = client.subscribe(client.topics.module(moduleName), (m) => {
    if (!m) return;
    currentMod = m;
    nameSpan.textContent = m.profile.name || "(none)";
    nameSpan.classList.toggle("changed", !!m.profile.changed);
    saveCurrentBtn.disabled = !m.profile.name;
  });
  bar._dispose = unsubscribe;
  return bar;
}
