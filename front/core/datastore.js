// core/datastore.js
//
// Normalized, single-source-of-truth store for everything an OST server
// sends over the WebSocket protocol (see
// OST/documentation/content/dev/dev-client/). Zero DOM manipulation here —
// this file only knows about plain JS objects and a topic-based pub/sub.
//
// The structural rule this file exists to enforce (see
// client-pattern.en.md "Rendering: scope updates to the property that
// changed"): consumers subscribe to the *exact* slice of state they render
// (one property, one module, the file list, ...) and are only notified when
// that slice actually changes — never a single "everything changed" signal
// fired on every incoming message. The one legitimate exception is the
// full dump (`d`)/reconnection case, where *everything* genuinely did
// change (see client-pattern.en.md "Reconnection: always start again from
// a full dump") — see `reset()`/`applyMessage()` for the full-dump path.
//
// Internal state shape (all plain objects/arrays, never exposed as classes):
//   modules: { [name]: { infos, profile:{name,changed}, lovs:{}, properties:{ [name]: Property } } }
//   Property: { label, order, level1, level2, status, permission, enabled,
//               showElts, hasGrid, hasGraph, gridheaders, gridLimit,
//               graphType, graphParams, rule, badge, preicon1..posticon2,
//               hasprofile, freevalue, elements:{ [name]: Element }, rows:[] }
//   files: { folders:[], files:[], selectedfolder:"" }
//   logs: [ {d,c,t,l}, ... ]
//   controllerdata: { ... see controller.en.md }
//   lovs: controller-scope LOVs ({ key: {label,type,values} }, wire key "lovs")
//   grantClient / grantServer / serverLng

// ---- wire message key classification --------------------------------
// Server keys are matched on their first 1 (for "d"/"l") or 2 characters,
// per server-messages.en.md ("the description part can change").
const ONE_CHAR_CODES = new Set(["d", "l"]);
const TWO_CHAR_CODES = new Set([
  "aa", "ap", "ea", "ee", "ev", "ps", "dm", "dp",
  "gc", "gu", "gd", "gr",
  "lc", "lu", "ld",
  "fs", "fl", "fc",
  "af", "df", "ad", "dd",
  "qa", "xx", "uc",
]);

function classifyKey(key) {
  if (key.length === 1 && ONE_CHAR_CODES.has(key)) return key;
  const two = key.slice(0, 2);
  if (TWO_CHAR_CODES.has(two)) return two;
  return null;
}

// ---- topic keys ---------------------------------------------------------
// Topics are opaque strings built only through the `topics` helpers below;
// never construct one by hand (the separator is deliberately not a
// character that can appear in a module/property/controllerdata name).
const SEP = "\u0000";
function key(...parts) { return parts.join(SEP); }

export const topics = {
  /** Connection/session info: {grantClient, grantServer, serverLng}. */
  session: () => key("session"),
  /** Sorted array of loaded module names. Fires only when a module is added/removed (aa/dm/d), never on ordinary property updates. */
  modules: () => key("modules"),
  /** One module's {infos, profile, lovs, properties}. Fires on aa/dm/d and on profile events (fs/fl/fc) — never on a sibling property's ea/ee. */
  module: (moduleName) => key("modules", moduleName),
  /** One module's own (non-controller) LOVs map. */
  moduleLovs: (moduleName) => key("modules", moduleName, "lovs"),
  /** One property object (elements + grid rows). This is the fine-grained, high-frequency topic — ea/ee/ev/ps/gc/gu/gd/gr/ap/dp all resolve to exactly this. */
  property: (moduleName, propName) => key("modules", moduleName, "properties", propName),
  /** Controller-scope LOVs map (wire key "lovs", see FINDINGS.md #2). */
  lovs: () => key("lovs"),
  /** Whole controllerdata object, or a single key's value when `dataKey` is given (a `uc` message only ever carries one key — see controller.en.md). */
  controllerData: (dataKey) => (dataKey ? key("controllerdata", dataKey) : key("controllerdata")),
  /** {folders, files, selectedfolder}. */
  files: () => key("files"),
  /** The full log array. */
  logs: () => key("logs"),
};

export class Datastore {
  constructor() {
    this._listeners = new Map(); // topicKey -> Set<fn>
    this._queryAnswerListeners = new Set();
    this._rawListeners = new Set(); // low-level debug hook, see subscribeRaw()
    this.reset();
  }

  reset() {
    this.grantClient = null;
    this.grantServer = null;
    this.serverLng = null;
    this.modules = {};
    this.files = { folders: [], files: [], selectedfolder: "" };
    this.logs = [];
    this.controllerdata = {};
    this.lovs = {};
    // Set here, cleared at the end of _applyFullDump() (the only other
    // writer) - lets a consumer with a one-shot side effect on a topic's
    // *change* (e.g. app.js's log-level toast) tell "this update is the
    // dump replaying history/state that already existed" apart from "this
    // is a genuinely new change" - both look identical otherwise (one
    // subscribe() call growing a list is indistinguishable from another
    // without this). See isDumpPending().
    this._dumpPending = true;
    // A reset is the one moment where "everything changed" is actually
    // true (nothing survives a disconnect, per client-pattern.en.md) —
    // tell every currently-subscribed topic so nothing renders stale data.
    this._emitAllSubscribed();
  }

  /**
   * True from reset() (disconnect, or the very start of connect() per
   * connection.js) until the matching full dump has actually been applied
   * - i.e. true for the whole window where state is either stale/cleared
   * or in the middle of being replaced wholesale, never for a genuine
   * single incremental change. See reset()'s own comment for why this
   * exists.
   */
  isDumpPending() {
    return this._dumpPending;
  }

  // ---- public subscription API -------------------------------------

  /**
   * Subscribe to one topic (built with the `topics` helpers). `cb` is
   * called once immediately with the current value, then again every time
   * that exact slice of state changes. Returns an unsubscribe function.
   */
  subscribe(topicKey, cb) {
    let set = this._listeners.get(topicKey);
    if (!set) { set = new Set(); this._listeners.set(topicKey, set); }
    set.add(cb);
    cb(this.get(topicKey));
    return () => { set.delete(cb); if (set.size === 0) this._listeners.delete(topicKey); };
  }

  /** Read the current value at a topic without subscribing. */
  get(topicKey) {
    return this._getByParts(topicKey.split(SEP));
  }

  /**
   * Groups several subscriptions so they can be torn down together (e.g.
   * every subscription a screen made while it was on-screen). Not part of
   * the wire protocol — a plain convenience over subscribe().
   */
  scope() {
    const unsubs = [];
    return {
      subscribe: (topicKey, cb) => { const u = this.subscribe(topicKey, cb); unsubs.push(u); return u; },
      dispose: () => { while (unsubs.length) unsubs.pop()(); },
    };
  }

  /**
   * Low-level escape hatch: fires for every applied server message, with
   * its 1/2-char event code and raw (already-normalized-key) payload. For
   * a debug console or protocol log only — driving UI rendering from this
   * instead of subscribe(topic, cb) reintroduces exactly the "re-render
   * everything on every message" problem client-pattern.en.md warns
   * against, so don't.
   */
  subscribeRaw(cb) {
    this._rawListeners.add(cb);
    return () => this._rawListeners.delete(cb);
  }

  /** Answers to QY queries (qa) — inter-module mechanism, not part of the datastore's persistent state. See client-messages.en.md#qy. */
  subscribeQueryAnswers(cb) {
    this._queryAnswerListeners.add(cb);
    return () => this._queryAnswerListeners.delete(cb);
  }

  /**
   * Resolves the {key: label} map for an element carrying a listOfValues
   * (local) or a globallov+lovScope pair (module or controller). Returns
   * null when the element has no LOV at all.
   */
  resolveLov(moduleName, element) {
    if (element.listOfValues) return element.listOfValues;
    if (!element.globallov) return null;
    const mod = this.modules[moduleName];
    let lov;
    if (element.lovScope === "module") {
      lov = mod && mod.lovs && mod.lovs[element.globallov];
    } else {
      // "controller" (or anything else): wire key is top-level "lovs", not
      // "controllerlovs" as structure.en.md's prose says — see FINDINGS.md #2.
      lov = this.lovs[element.globallov];
    }
    if (lov) return lov.values || {};
    // `loadedModules-<template>` / `profiles-<template>` are documented as
    // resolvable controller LOVs but are never actually generated
    // server-side (see FINDINGS.md #1) — fall back to filtering the base,
    // unfiltered LOV (replicated into every module's own globallovs)
    // client-side by the suffix, same as ostharfang4.
    const dash = element.globallov.indexOf("-");
    if (dash > 0) {
      const base = element.globallov.slice(0, dash);
      const suffix = element.globallov.slice(dash + 1).toLowerCase();
      const baseLov = mod && mod.lovs && mod.lovs[base];
      if (baseLov) {
        const filtered = {};
        for (const [k, label] of Object.entries(baseLov.values || {})) {
          if (k.toLowerCase().includes(suffix) || String(label).toLowerCase().includes(suffix)) filtered[k] = label;
        }
        return Object.keys(filtered).length ? filtered : (baseLov.values || {});
      }
    }
    return {};
  }

  // ---- entry point: one incoming WS frame --------------------------

  /** `raw` is either the JSON text of one WS frame, or an already-parsed object. */
  applyMessage(raw) {
    let obj = raw;
    if (typeof raw === "string") {
      try { obj = JSON.parse(raw); } catch (e) { console.error("Invalid JSON from server", raw); return; }
    }
    const wireKey = Object.keys(obj)[0];
    if (wireKey === undefined) return;
    const code = classifyKey(wireKey);
    const payload = obj[wireKey];
    if (!code) { console.warn("Unrecognized server message key", wireKey, obj); return; }
    try { this._apply(code, payload); } catch (e) { console.error("Error applying message", code, payload, e); }
    for (const fn of this._rawListeners) { try { fn(code, payload); } catch (e) { console.error(e); } }
  }

  // ---- dispatch -------------------------------------------------------

  _apply(code, payload) {
    switch (code) {
      case "d": return this._applyFullDump(payload);
      case "aa": return this._applyModuleDump(payload);
      case "ap": return this._applyPropertyReplace(payload);
      case "ea": return this._applyElementValues(payload);
      case "ee": return this._applyElementValues(payload);
      case "ev": return this._applyElementMeta(payload);
      case "ps": return this._applyPropertyStatus(payload);
      case "dm": return this._applyModuleDeleted(payload);
      case "dp": return this._applyPropertyDeleted(payload);
      case "gc": return this._applyGridCreate(payload);
      case "gu": return this._applyGridUpdate(payload);
      case "gd": return this._applyGridDelete(payload);
      case "gr": return this._applyGridReset(payload);
      case "lc": case "lu": return this._applyLovUpsert(payload);
      case "ld": return this._applyLovDelete(payload);
      case "fs": case "fl": case "fc": return this._applyProfileEvent(payload);
      case "l": return this._applyLog(payload);
      case "uc": return this._applyControllerUpdate(payload);
      case "af": case "df": return this._applyFileEvent(code, payload);
      case "ad": case "dd": return this._applyFolderEvent(code, payload);
      case "qa": return this._applyQueryAnswer(payload);
      case "xx": return; // keepalive pong, no state
      default: return;
    }
  }

  // ---- normalization helpers -----------------------------------------

  _rowsFromArrayGrid(gridheaders, grid) {
    if (!Array.isArray(grid)) return [];
    return grid.map((row) => {
      const obj = {};
      (gridheaders || []).forEach((h, i) => { obj[h] = row[i]; });
      return obj;
    });
  }

  _normalizeProperty(raw) {
    const p = Object.assign({}, raw);
    p.elements = raw.e || raw.elements || {};
    delete p.e;
    p.rows = p.hasGrid ? this._rowsFromArrayGrid(p.gridheaders, p.grid) : [];
    return p;
  }

  _normalizeModule(raw) {
    const infos = raw.infos || {};
    const profile = raw.f || raw.profile || { name: "", changed: false };
    const lovs = raw.l || raw.globallovs || {};
    const propsRaw = raw.p || raw.properties || {};
    const properties = {};
    for (const [name, p] of Object.entries(propsRaw)) properties[name] = this._normalizeProperty(p);
    return { infos, profile, lovs, properties };
  }

  _ensureModule(name) {
    if (!this.modules[name]) this.modules[name] = { infos: { name }, profile: { name: "", changed: false }, lovs: {}, properties: {} };
    return this.modules[name];
  }

  _ensureProperty(moduleName, propName) {
    const mod = this._ensureModule(moduleName);
    if (!mod.properties[propName]) mod.properties[propName] = { elements: {}, rows: [] };
    return mod.properties[propName];
  }

  // ---- per-message-type handlers, each ending with a scoped _emit ----

  _applyFullDump(d) {
    this.grantClient = d["grant-client"];
    this.grantServer = d["grant-server"];
    this.serverLng = d.serverlng;
    this.modules = {};
    for (const [name, raw] of Object.entries(d.m || d.modules || {})) {
      this.modules[name] = this._normalizeModule(raw);
      this.modules[name].infos.name = this.modules[name].infos.name || name;
    }
    this.files = d.files || { folders: [], files: [], selectedfolder: "" };
    this.logs = d.logs || [];
    this.controllerdata = d.controllerdata || {};
    this.lovs = d.lovs || {};
    this._emitAllSubscribed();
    // Cleared after the emit above, not before - any subscriber reading
    // isDumpPending() from inside its own callback for *this* dump must
    // still see true (see reset()'s comment). Only a later, genuinely
    // incremental change sees false.
    this._dumpPending = false;
  }

  _applyModuleDump(payload) {
    for (const [name, raw] of Object.entries(payload)) {
      this.modules[name] = this._normalizeModule(raw);
      this.modules[name].infos.name = this.modules[name].infos.name || name;
      this._emit(topics.module(name));
      this._emit(topics.moduleLovs(name));
      for (const propName of Object.keys(this.modules[name].properties)) this._emit(topics.property(name, propName));
    }
    this._emit(topics.modules());
  }

  _applyPropertyReplace(payload) {
    for (const [modName, modBody] of Object.entries(payload)) {
      const mod = this._ensureModule(modName);
      const props = modBody.p || modBody.properties || {};
      let sawNewProperty = false;
      for (const [propName, raw] of Object.entries(props)) {
        if (!(propName in mod.properties)) sawNewProperty = true;
        mod.properties[propName] = this._normalizeProperty(raw);
        this._emit(topics.property(modName, propName));
      }
      // A property appearing for the first time (e.g. an INDI device's own
      // properties, re-created from scratch after it dropped off the bus
      // and came back) needs the module-level topic too - nothing is ever
      // subscribed to topics.property() for a property that didn't exist
      // yet, so the generic fallback template's level1/level2 layout
      // (built from the module snapshot, see render.js's moduleLifecycle())
      // would otherwise only pick it up on an unrelated full module
      // rebuild. Same reasoning as the aa/dm/dp handlers, which already
      // emit this. A plain replace of an already-known property (e.g. a
      // grid reorder) stays scoped to its own topic, same as before.
      if (sawNewProperty) this._emit(topics.module(modName));
    }
  }

  _applyElementValues(payload) {
    for (const [modName, modBody] of Object.entries(payload)) {
      const props = modBody.p || modBody.properties || {};
      for (const [propName, propBody] of Object.entries(props)) {
        const prop = this._ensureProperty(modName, propName);
        const elems = propBody.e || propBody.elements || {};
        for (const [elName, val] of Object.entries(elems)) {
          if (!prop.elements[elName]) prop.elements[elName] = {};
          prop.elements[elName].value = val;
        }
        this._emit(topics.property(modName, propName));
      }
    }
  }

  _applyElementMeta(payload) {
    // See FINDINGS.md for the open question on the "globallov changed,
    // value field absent" case (client-pattern.en.md's "Known pitfall").
    for (const [modName, modBody] of Object.entries(payload)) {
      const props = modBody.p || modBody.properties || {};
      for (const [propName, propBody] of Object.entries(props)) {
        const prop = this._ensureProperty(modName, propName);
        const elems = propBody.e || propBody.elements || {};
        for (const [elName, meta] of Object.entries(elems)) {
          const existing = prop.elements[elName] || {};
          prop.elements[elName] = Object.assign({}, existing, meta);
        }
        this._emit(topics.property(modName, propName));
      }
    }
  }

  _applyPropertyStatus(payload) {
    for (const [modName, modBody] of Object.entries(payload)) {
      const props = modBody.p || modBody.properties || {};
      for (const [propName, patch] of Object.entries(props)) {
        const prop = this._ensureProperty(modName, propName);
        if ("status" in patch) prop.status = patch.status;
        if ("enabled" in patch) prop.enabled = patch.enabled;
        this._emit(topics.property(modName, propName));
      }
    }
  }

  _applyModuleDeleted(payload) {
    for (const modName of Object.keys(payload)) {
      const mod = this.modules[modName];
      const propNames = mod ? Object.keys(mod.properties) : [];
      delete this.modules[modName];
      for (const propName of propNames) this._emit(topics.property(modName, propName));
      this._emit(topics.moduleLovs(modName));
      this._emit(topics.module(modName));
    }
    this._emit(topics.modules());
  }

  _applyPropertyDeleted(payload) {
    for (const [modName, modBody] of Object.entries(payload)) {
      const props = modBody.p || modBody.properties || {};
      const mod = this.modules[modName];
      for (const propName of Object.keys(props)) {
        if (mod) delete mod.properties[propName];
        this._emit(topics.property(modName, propName));
      }
      this._emit(topics.module(modName));
    }
  }

  _applyGridCreate(payload) {
    for (const [modName, modBody] of Object.entries(payload)) {
      const props = modBody.p || modBody.properties || {};
      for (const [propName, body] of Object.entries(props)) {
        const prop = this._ensureProperty(modName, propName);
        if (!prop.rows) prop.rows = [];
        const idx = typeof body.i === "number" ? body.i : prop.rows.length;
        prop.rows.splice(idx, 0, body.values || {});
        this._emit(topics.property(modName, propName));
      }
    }
  }

  _applyGridUpdate(payload) {
    for (const [modName, modBody] of Object.entries(payload)) {
      const props = modBody.p || modBody.properties || {};
      for (const [propName, body] of Object.entries(props)) {
        const prop = this._ensureProperty(modName, propName);
        if (!prop.rows) prop.rows = [];
        if (typeof body.i === "number" && body.i < prop.rows.length) prop.rows[body.i] = Object.assign({}, prop.rows[body.i], body.values || {});
        this._emit(topics.property(modName, propName));
      }
    }
  }

  _applyGridDelete(payload) {
    for (const [modName, modBody] of Object.entries(payload)) {
      const props = modBody.p || modBody.properties || {};
      for (const [propName, body] of Object.entries(props)) {
        const prop = this._ensureProperty(modName, propName);
        if (!prop.rows) prop.rows = [];
        if (typeof body.i === "number") prop.rows.splice(body.i, 1);
        this._emit(topics.property(modName, propName));
      }
    }
  }

  _applyGridReset(payload) {
    for (const [modName, modBody] of Object.entries(payload)) {
      const props = modBody.p || modBody.properties || {};
      for (const propName of Object.keys(props)) {
        const prop = this._ensureProperty(modName, propName);
        prop.rows = [];
        // `.grid`/`.gridheaders` (the raw wire array-of-arrays, only ever
        // set by _normalizeProperty() on a dump/"ap" replace - see that
        // function's own doc comment) were left stale here until found
        // live, 2026-10-06: a search with zero matches sends "gr" (this
        // handler), a non-empty one sends a full "ap" replace, so only
        // the empty case ever hit this path. `.rows` is this datastore's
        // one documented live-state field (render.js's own
        // renderGridSection() already only ever reads `.rows`, never
        // `.grid`) - cleared here too defensively, so a future caller
        // reaching for the raw wire shape instead doesn't get burned by
        // an empty result silently reading back as the PREVIOUS search's
        // leftover rows.
        prop.grid = [];
        this._emit(topics.property(modName, propName));
      }
    }
  }

  _applyLovUpsert(payload) {
    if (payload.lovs) {
      for (const [k, val] of Object.entries(payload.lovs)) this.lovs[k] = val;
      this._emit(topics.lovs());
      return;
    }
    for (const [modName, modBody] of Object.entries(payload)) {
      const mod = this._ensureModule(modName);
      const lovs = modBody.l || modBody.globallovs || {};
      for (const [k, val] of Object.entries(lovs)) mod.lovs[k] = val;
      this._emit(topics.moduleLovs(modName));
    }
  }

  _applyLovDelete(payload) {
    if (payload.lovs) {
      for (const k of Object.keys(payload.lovs)) delete this.lovs[k];
      this._emit(topics.lovs());
      return;
    }
    for (const [modName, modBody] of Object.entries(payload)) {
      const mod = this._ensureModule(modName);
      const lovs = modBody.l || modBody.globallovs || {};
      for (const k of Object.keys(lovs)) delete mod.lovs[k];
      this._emit(topics.moduleLovs(modName));
    }
  }

  _applyProfileEvent(payload) {
    for (const [modName, modBody] of Object.entries(payload)) {
      const mod = this._ensureModule(modName);
      const f = modBody.f || modBody.profile;
      if (f) mod.profile = Object.assign({}, mod.profile, f);
      this._emit(topics.module(modName));
    }
  }

  _applyLog(entry) {
    this.logs.push(entry);
    if (this.logs.length > 1000) this.logs.shift();
    this._emit(topics.logs());
  }

  _applyControllerUpdate(payload) {
    // A uc carries exactly one key (controller.en.md); loop defensively.
    for (const [k, val] of Object.entries(payload)) {
      this.controllerdata[k] = val;
      if (k === "files" && Array.isArray(val)) this.files.files = val;
      if (k === "folders" && Array.isArray(val)) this.files.folders = val;
      this._emit(topics.controllerData(k));
      this._emit(topics.controllerData());
      if (k === "files" || k === "folders") this._emit(topics.files());
    }
  }

  _applyFileEvent(code, path) {
    if (code === "af") { if (!this.files.files.includes(path)) this.files.files.push(path); }
    else this.files.files = this.files.files.filter((f) => f !== path);
    this._emit(topics.files());
  }

  _applyFolderEvent(code, path) {
    if (code === "ad") { if (!this.files.folders.includes(path)) this.files.folders.push(path); }
    else this.files.folders = this.files.folders.filter((f) => f !== path);
    this._emit(topics.files());
  }

  _applyQueryAnswer(payload) {
    for (const fn of this._queryAnswerListeners) { try { fn(payload); } catch (e) { console.error(e); } }
  }

  // ---- topic <-> state plumbing ---------------------------------------

  _getByParts(parts) {
    const [scope, a, b, c] = parts;
    switch (scope) {
      case "session": return { grantClient: this.grantClient, grantServer: this.grantServer, serverLng: this.serverLng };
      case "modules":
        if (parts.length === 1) return Object.keys(this.modules).sort();
        if (parts.length === 2) return this.modules[a] || null;
        if (parts.length === 3 && b === "lovs") return (this.modules[a] && this.modules[a].lovs) || {};
        if (parts.length === 4 && b === "properties") return (this.modules[a] && this.modules[a].properties[c]) || null;
        return null;
      case "lovs": return this.lovs;
      case "controllerdata": return a ? this.controllerdata[a] : this.controllerdata;
      case "files": return this.files;
      case "logs": return this.logs;
      default: return null;
    }
  }

  _emit(topicKey) {
    const set = this._listeners.get(topicKey);
    if (!set || set.size === 0) return;
    const value = this.get(topicKey);
    for (const fn of set) { try { fn(value); } catch (e) { console.error("subscriber error for", topicKey, e); } }
  }

  // Only fires the currently *subscribed* topics (not every possible one)
  // so a fresh full dump doesn't require enumerating anything a caller
  // never asked about.
  _emitAllSubscribed() {
    for (const topicKey of Array.from(this._listeners.keys())) this._emit(topicKey);
  }
}
