// templates/navigator.js — bespoke screen for the "navigator" module: the
// planetarium sky-dome view (js/sky-engine.js) as the full background,
// same #bg-layer treatment as guider's own guide-frame image
// (js/imagelayer.js) — see notes/NOTES-frontend-ostcore-planetarium.md for
// the full chantier context and the decisions this implements:
//   - own client-side catalog (js/sky-catalog.js), independent of the
//     server's search/results catalog;
//   - dome view first (v1 scope: fixed/zenith-centered, wheel-zoom only -
//     drag/pan deliberately deferred to its own follow-up step, see
//     sky-engine.js's own header for why);
//   - full-screen background placement, panels float over it.
//
// Selection (Gilles, 2026-09-29: "point epineux : la selection d'un objet,
// on fait comment ?"): a tap/click on the dome (sky-engine.js's own
// onSelect, not a right-click/long-press — see that file's header for the
// mobile-compatibility reasoning) opens/updates a regular "Selection"
// panel with the picked object's info + GOTO/Add-to-planner buttons - same
// interaction language as the rest of this app, no context menu anywhere.
//
// Still not wired in this pass: search/results (text search still only
// reachable via the generic "Other" catch-all panel), the visibility bar /
// time-scrub control (engine already accepts an arbitrary getTime(), just
// nothing drives it yet), the horizon/terrestrial and camera-field views.
//
// Real property names confirmed against src/modules/navigator/navigator.json
// / libs/indimodule.cpp's defineMeAsNavigator()/giveMeALocation(): search,
// results, selectnow, target (targetname/targetra/targetde), mountposition,
// gpslocation, gpstime, location, actions (abortnavigator/gototarget/
// addtoplanner), centeringparams, slaves - plus image/optic/parms, generic
// to any imager.

import { renderProperty, updateQuickNav } from "../render.js";
import { el } from "../dom.js";
import * as Panels from "../panels.js";
import { SkyEngine } from "../sky-engine.js";

function fmtDeg(v) { return v.toFixed(3) + "°"; }
function fmtHours(raDeg) {
  const h = ((raDeg / 15) % 24 + 24) % 24;
  const hh = Math.floor(h);
  const mm = Math.floor((h - hh) * 60);
  const ss = Math.round(((h - hh) * 60 - mm) * 60);
  return `${hh}h${String(mm).padStart(2, "0")}m${String(ss).padStart(2, "0")}s`;
}

/** The "Selection" panel's body: updated in place from sky-engine.js's
 * onSelect callback (client-only transient state, not server-sourced - no
 * client.subscribe() here), plus two buttons that actually talk to the
 * server: setting target.targetname/targetra/targetde (SV, immediately on
 * selection, so the module's own idea of "the current pick" stays in sync
 * with what's shown here) and firing actions.gototarget/addtoplanner.
 *
 * targetra is sent in HOURS (0-24), not degrees — INDI::IEquatorialCoordinates
 * ("Right Ascension in Hours (0 to 24)", libastro.h) is what navigator.cpp's
 * own gototarget handler feeds straight into INDI::J2000toObserved(). Every
 * RA elsewhere in this file (sky-projection.js, sky-catalog.js, sky-engine.js)
 * stays in degrees for internal consistency with Dec — this /15 is the one
 * and only place it matters, right at the wire boundary. Sending degrees
 * here was the actual cause of a GOTO landing in a "weird position" (Gilles,
 * 2026-09-30) — a 15x error, not the smaller J2000/of-date precession gap
 * fixed alongside it in sky-projection.js. */
function buildSelectionPanel(client, moduleName) {
  const wrap = el("div");
  const info = el("div", { class: "muted small", text: "Tap a star or object on the sky to select it." });
  const gotoBtn = el("button", { class: "btn btn-primary btn-sm", disabled: true }, [el("span", { class: "material-icons", style: "font-size:15px;vertical-align:middle" }, "gps_fixed"), " GOTO"]);
  const plannerBtn = el("button", { class: "btn btn-sm", disabled: true }, "Add to planner");
  const actionsRow = el("div", { class: "form-row" }, [gotoBtn, plannerBtn]);
  wrap.appendChild(info);
  wrap.appendChild(actionsRow);

  gotoBtn.addEventListener("click", () => client.setValue(moduleName, "actions", "gototarget", true));
  plannerBtn.addEventListener("click", () => client.setValue(moduleName, "actions", "addtoplanner", true));

  function onSelect(obj) {
    const name = obj.name || (obj.kind === "star" ? "Star" : obj.kind === "messier" ? obj.desig || "DSO" : "Point");
    info.innerHTML = "";
    info.appendChild(el("div", { text: name }));
    info.appendChild(el("div", { class: "muted small mono", text: `RA ${fmtHours(obj.ra)}  Dec ${fmtDeg(obj.dec)}` }));
    if (obj.mag !== undefined) info.appendChild(el("div", { class: "muted small", text: `mag ${obj.mag}` }));
    gotoBtn.disabled = false;
    plannerBtn.disabled = false;
    client.setValue(moduleName, "target", "targetname", name);
    client.setValue(moduleName, "target", "targetra", obj.ra / 15); // degrees -> hours, see this function's own doc comment
    client.setValue(moduleName, "target", "targetde", obj.dec);
  }

  wrap._dispose = () => {};
  return { node: wrap, onSelect };
}

// Same shared #bg-layer element imagelayer.js targets for guider/focus/
// sequencer's own image background - looked up directly here (unlike
// imagelayer.js's setBgLayer() indirection) since this is the only
// consumer and there's no boot-time ordering to get right either way.
function buildSkyDome(client, moduleName, onSelect) {
  const bgLayer = document.getElementById("bg-layer");
  if (!bgLayer) return { dispose: () => {} };
  bgLayer.classList.add("sky-active");
  bgLayer.innerHTML = "";
  const canvas = el("canvas", { id: "sky-canvas" });
  bgLayer.appendChild(canvas);

  const engine = new SkyEngine(canvas, {
    onNeedsRender: () => engine.render(),
    onSelect,
  });
  engine.loadCatalog("light").then(() => engine.render()).catch((e) => console.error("[navigator] catalog load", e));

  const ro = new ResizeObserver(() => {
    const r = bgLayer.getBoundingClientRect();
    engine.resize(r.width, r.height);
    engine.render();
  });
  ro.observe(bgLayer);

  // Site: prefer the module's own configured "location" (see
  // giveMeALocation()) - always present once the module is loaded, unlike
  // gpslocation which depends on an actual GPS sensor being wired up.
  const unsub = client.subscribe(client.topics.property(moduleName, "location"), (prop) => {
    const lat = prop && prop.elements.lat && prop.elements.lat.value;
    const lon = prop && prop.elements.lon && prop.elements.lon.value;
    const alt = prop && prop.elements.alt && prop.elements.alt.value;
    if (typeof lat === "number" && typeof lon === "number") {
      engine.setSite(lat, lon, alt || 0);
      engine.render();
    }
  });

  // The sky visibly moves even standing still - a light periodic refresh
  // keeps it live without needing a real interaction. 30s is plenty (the
  // whole dome shifts ~0.125deg/30s from Earth's rotation, imperceptible
  // faster than that anyway).
  const timer = setInterval(() => engine.render(), 30000);

  return {
    dispose: () => {
      unsub();
      ro.disconnect();
      clearInterval(timer);
      engine.destroy();
      bgLayer.innerHTML = "";
      bgLayer.classList.remove("sky-active");
    },
  };
}

export function mount(client, moduleName) {
  Panels.setScope(moduleName);
  Panels.clear();

  let disposers = [];
  function track(node) { disposers.push(node._dispose); return node; }
  function dispose() { while (disposers.length) disposers.pop()(); }

  function build(mod) {
    dispose();
    Panels.clear();
    const has = (name) => name in mod.properties;
    const quickNavEntries = [];

    const selection = buildSelectionPanel(client, moduleName);
    disposers.push(buildSkyDome(client, moduleName, selection.onSelect).dispose);

    const header = Panels.make({ id: "header", title: mod.infos.modulelabel || mod.infos.label || moduleName, x: 20, y: 92, w: 320 });
    header.appendChild(el("div", { class: "muted small", text: mod.infos.description || "" }));

    const selectionPanel = Panels.make({ id: "selection", title: "Selection", x: 340, y: 92, w: 300 });
    selectionPanel.appendChild(track(selection.node));
    quickNavEntries.push({ id: "selection", label: "Selection" });

    if (has("actions")) {
      const b = Panels.make({ id: "actions", title: mod.properties.actions.label || "Commands", x: 20, y: 210, w: 300 });
      b.appendChild(track(renderProperty(client, moduleName, "actions")));
      quickNavEntries.push({ id: "actions", label: mod.properties.actions.label || "Actions" });
    }
    if (has("location")) {
      const b = Panels.make({ id: "location", title: mod.properties.location.label || "Site", x: 20, y: 330, w: 300 });
      b.appendChild(track(renderProperty(client, moduleName, "location")));
      quickNavEntries.push({ id: "location", label: mod.properties.location.label || "Site" });
    }
    if (has("mountposition")) {
      const b = Panels.make({ id: "mountposition", title: mod.properties.mountposition.label || "Mount", x: 20, y: 500, w: 260 });
      b.appendChild(track(renderProperty(client, moduleName, "mountposition")));
      quickNavEntries.push({ id: "mountposition", label: "Mount" });
    }

    // Anything this v1 doesn't have a dedicated panel for yet (search,
    // target, results, gpslocation, gpstime, centeringparams, slaves,
    // image/optic/parms...) still needs to be reachable - one catch-all
    // panel rather than silently dropping it. hasprofile properties
    // (centeringparams, slaves) are excluded here since they're already
    // reachable from the Parameters modal, same rule every other template
    // in this codebase follows.
    const hasprofileNames = Object.keys(mod.properties).filter((n) => mod.properties[n].hasprofile === true);
    const known = new Set(["actions", "location", "mountposition", ...hasprofileNames]);
    const rest = Object.keys(mod.properties).filter((n) => !known.has(n));
    if (rest.length) {
      const b = Panels.make({ id: "other", title: "Other", x: 340, y: 500, w: 340 });
      for (const name of rest) b.appendChild(track(renderProperty(client, moduleName, name)));
      quickNavEntries.push({ id: "other", label: "Other" });
    }
    updateQuickNav(quickNavEntries);
  }

  const unsub = client.subscribe(client.topics.module(moduleName), (mod) => {
    if (!mod) {
      dispose();
      Panels.clear();
      updateQuickNav([]);
      const b = Panels.make({ id: "missing", title: moduleName, x: 20, y: 92, w: 320 });
      b.appendChild(el("div", { class: "muted", text: "Module not found (unloaded?)" }));
      return;
    }
    build(mod);
  });

  return { dispose: () => { unsub(); dispose(); Panels.clear(); updateQuickNav([]); } };
}
