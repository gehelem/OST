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
import { SkyEngine, ZOOM_MIN, ZOOM_MAX } from "../sky-engine.js";
import { STAR_DEPTHS } from "../sky-catalog.js";
import { t } from "../i18n.js";

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
  const info = el("div", { class: "muted small", text: t("navigator.tapToSelect") });
  const gotoBtn = el("button", { class: "btn btn-primary btn-sm", disabled: true }, [el("span", { class: "material-icons", style: "font-size:15px;vertical-align:middle" }, "gps_fixed"), " " + t("navigator.goto")]);
  const plannerBtn = el("button", { class: "btn btn-sm", disabled: true }, t("navigator.addToPlanner"));
  const actionsRow = el("div", { class: "form-row" }, [gotoBtn, plannerBtn]);
  wrap.appendChild(info);
  wrap.appendChild(actionsRow);

  gotoBtn.addEventListener("click", () => client.setValue(moduleName, "actions", "gototarget", true));
  plannerBtn.addEventListener("click", () => client.setValue(moduleName, "actions", "addtoplanner", true));

  function onSelect(obj) {
    const name = obj.name || (obj.kind === "star" ? t("navigator.star") : obj.kind === "messier" ? obj.desig || t("navigator.dso") : t("navigator.point"));
    info.innerHTML = "";
    info.appendChild(el("div", { text: name }));
    info.appendChild(el("div", { class: "muted small mono", text: `RA ${fmtHours(obj.ra)}  Dec ${fmtDeg(obj.dec)}` }));
    if (obj.mag !== undefined) info.appendChild(el("div", { class: "muted small", text: t("navigator.magPrefix") + obj.mag }));
    gotoBtn.disabled = false;
    plannerBtn.disabled = false;
    client.setValue(moduleName, "target", "targetname", name);
    client.setValue(moduleName, "target", "targetra", obj.ra / 15); // degrees -> hours, see this function's own doc comment
    client.setValue(moduleName, "target", "targetde", obj.dec);
  }

  wrap._dispose = () => {};
  return { node: wrap, onSelect };
}

// Persisted like the theme/UI-zoom widgets elsewhere in this app (one
// global key, not per module instance — matches how those work too).
// saveDisplaySettings() takes the engine itself (reads its current fields)
// so *every* path that changes a setting — the panel's own checkboxes/
// slider, but also the canvas's own wheel-zoom (Gilles, 2026-09-30: "le
// niveau de zoom ne persiste pas" — wheel-zoom went through sky-engine.js's
// own _onWheel(), which never called back into this file's save logic at
// all) — can call the exact same function instead of two copies drifting
// apart.
const DISPLAY_SETTINGS_KEY = "ostcore-ui3.navigator.display";
function loadDisplaySettings() {
  try { return JSON.parse(localStorage.getItem(DISPLAY_SETTINGS_KEY)) || {}; } catch (e) { return {}; }
}
function saveDisplaySettings(engine) {
  try {
    localStorage.setItem(DISPLAY_SETTINGS_KEY, JSON.stringify({
      showConstellations: engine.showConstellations,
      showCardinals: engine.showCardinals,
      showMessier: engine.showMessier,
      showTrueHorizon: engine.showTrueHorizon,
      showEquatorialGrid: engine.showEquatorialGrid,
      showAzimuthalGrid: engine.showAzimuthalGrid,
      showMeridian: engine.showMeridian,
      showMountPosition: engine.showMountPosition,
      showSensorFov: engine.showSensorFov,
      dayMode: engine.dayMode,
      zoom: engine.zoom,
      rotation: engine.rotation, // pan/trackball state - a plain {w,x,y,z} quaternion, see js/quaternion.js
      projection: engine.projection,
      catalogDepth: engine.catalogDepth,
    }));
  } catch (e) { /* ignore */ }
}

/** The "Settings" panel's body — Gilles, 2026-09-30: "ajoute un bouton
 * 'reglages' ... qui ouvre une fenetre pour personnaliser l'affichage
 * (reperes, horizon, niveau de zoom, jour/nuit)". Plain checkboxes/slider
 * directly flipping js/sky-engine.js's own public display-setting fields
 * and calling render() again — no server round-trip, this is purely
 * client-side presentation, so it's persisted to localStorage instead
 * (same pattern as the theme/UI-zoom widgets). Saved settings are already
 * applied to `engine` by buildSkyDome() before this is even called (so the
 * very first render is correct, not a flash-then-jump) — this only needs
 * to reflect that starting state in the controls themselves. */
function buildSettingsPanel(engine) {
  const wrap = el("div");

  function persistAndRender() {
    saveDisplaySettings(engine);
    engine.render();
  }

  function checkboxRow(label, key) {
    const cb = el("input", { type: "checkbox" });
    cb.checked = engine[key];
    cb.addEventListener("change", () => { engine[key] = cb.checked; persistAndRender(); });
    return el("label", { class: "form-row", style: "cursor:pointer" }, [cb, el("span", { text: label })]);
  }

  wrap.appendChild(checkboxRow(t("navigator.constellations"), "showConstellations"));
  wrap.appendChild(checkboxRow(t("navigator.cardinalPoints"), "showCardinals"));
  wrap.appendChild(checkboxRow(t("navigator.messierObjects"), "showMessier"));
  wrap.appendChild(checkboxRow(t("navigator.trueHorizon"), "showTrueHorizon"));
  wrap.appendChild(checkboxRow(t("navigator.equatorialGrid"), "showEquatorialGrid"));
  wrap.appendChild(checkboxRow(t("navigator.azimuthalGrid"), "showAzimuthalGrid"));
  wrap.appendChild(checkboxRow(t("navigator.meridian"), "showMeridian"));
  wrap.appendChild(checkboxRow(t("navigator.mountPosition"), "showMountPosition"));
  wrap.appendChild(checkboxRow(t("navigator.sensorFov"), "showSensorFov"));
  wrap.appendChild(checkboxRow(t("navigator.dayMode"), "dayMode"));

  // Not a plain boolean engine field (it's the string "orthographic"/
  // "stereographic", see sky-engine.js's own `projection` doc comment) -
  // checkboxRow() above only knows how to flip a boolean field directly,
  // so this one gets its own row instead. Gilles, 2026-10-02: "dans les
  // settings d'affichage prevois un toggle pour afficher en
  // stereographique/orthographique et implemente le".
  const projCb = el("input", { type: "checkbox" });
  projCb.checked = engine.projection === "stereographic";
  projCb.addEventListener("change", () => {
    engine.projection = projCb.checked ? "stereographic" : "orthographic";
    persistAndRender();
  });
  wrap.appendChild(el("label", { class: "form-row", style: "cursor:pointer" }, [projCb, el("span", { text: t("navigator.stereographic") })]));

  // Star catalog depth - Gilles, 2026-09-29: "leger par defaut, mais
  // parametrable par l'utilisateur - donc prevoir tous les catalogues
  // possibles des le depart" (the actual selector was deferred until now:
  // 2026-10-03, "le grand saut : on augmente la densite, on passe la
  // taille au dessus sur les catalogues", after which sky-catalog.js
  // gained the "full" depth this exposes). Re-fetches on change (not just
  // a display-setting flip like everything else in this panel), so it
  // gets its own persistAndRender()-alike rather than reusing
  // checkboxRow()'s synchronous one.
  const depthRow = el("div", { class: "form-row" }, [el("span", { class: "muted small", text: t("navigator.catalogDepth") })]);
  const depthSelect = el("select", { class: "field-input" });
  for (const key of Object.keys(STAR_DEPTHS)) {
    depthSelect.appendChild(el("option", { value: key, text: t("navigator.catalogDepth." + key) }));
  }
  depthSelect.value = engine.catalogDepth || "light";
  depthSelect.addEventListener("change", () => {
    engine.loadCatalog(depthSelect.value).then(() => persistAndRender()).catch((e) => console.error("[navigator] catalog load", e));
  });
  depthRow.appendChild(depthSelect);
  wrap.appendChild(depthRow);

  const zoomRow = el("div", { class: "form-row" }, [el("span", { class: "muted small", text: t("navigator.zoom") })]);
  const zoomSlider = el("input", { type: "range", min: String(ZOOM_MIN), max: String(ZOOM_MAX), step: "0.1", value: String(engine.zoom), style: "flex:1" });
  zoomSlider.addEventListener("input", () => { engine.zoom = parseFloat(zoomSlider.value); persistAndRender(); });
  zoomRow.appendChild(zoomSlider);
  wrap.appendChild(zoomRow);
  // Kept in sync when zoom changes some other way (wheel, or a future
  // pinch-zoom) — same "one listener reflects every path" idea as
  // app.js's own panelvisibility listener for the quicknav buttons.
  engine.onZoomChange = () => { zoomSlider.value = String(engine.zoom); saveDisplaySettings(engine); };
  // Gilles, 2026-10-01: "quand je fais F5 la position du planetarium n'est
  // pas preservee" - see sky-engine.js's own onRotationChange doc comment.
  engine.onRotationChange = () => saveDisplaySettings(engine);

  wrap._dispose = () => { engine.onZoomChange = null; engine.onRotationChange = null; };
  return wrap;
}

// Same shared #bg-layer element imagelayer.js targets for guider/focus/
// sequencer's own image background - looked up directly here (unlike
// imagelayer.js's setBgLayer() indirection) since this is the only
// consumer and there's no boot-time ordering to get right either way.

// app.js's own applyZoom() publishes the "UI size" slider's current value
// as a real CSS custom property (needed there so #zoom-widget can counter
// it - see that file's own comment) - reused here as the one clean way to
// read it without a direct import/coupling to app.js's own module-local
// state. Gilles, 2026-10-02: "il faudrait que les libelles ... tiennent
// compte du niveau d'affichage Tt" (that same "UI size" slider) - see
// sky-engine.js's own `uiZoom` field for what it does with this value.
function readUiZoom() {
  const raw = parseFloat(document.documentElement.style.getPropertyValue("--ui-zoom"));
  return raw > 0 ? raw : 1;
}

function buildSkyDome(client, moduleName, onSelect) {
  const bgLayer = document.getElementById("bg-layer");
  if (!bgLayer) return { dispose: () => {}, engine: null };
  bgLayer.classList.add("sky-active");
  bgLayer.innerHTML = "";
  const canvas = el("canvas", { id: "sky-canvas" });
  bgLayer.appendChild(canvas);

  const engine = new SkyEngine(canvas, {
    onNeedsRender: () => engine.render(),
    onSelect,
    lightUI: document.documentElement.getAttribute("data-theme") === "light",
    uiZoom: readUiZoom(),
  });
  // Applied before the catalog/first render below, not in the settings
  // panel (built later, after this function returns) — so the very first
  // frame already reflects saved settings instead of a brief flash at the
  // defaults. buildSettingsPanel() takes it from here for the ongoing
  // wheel-zoom -> persist wiring (engine.onZoomChange).
  const savedSettings = loadDisplaySettings();
  Object.assign(engine, savedSettings);
  // "leger par defaut, mais parametrable" (Gilles, 2026-09-29) - defaults
  // to "light" same as before, but now actually reads back whichever
  // depth was last chosen (buildSettingsPanel()'s own selector, added
  // 2026-10-03: "le grand saut ... on passe la taille au dessus sur les
  // catalogues") instead of hardcoding "light" regardless. Set
  // synchronously here (loadCatalog() itself only sets it once the fetch
  // actually resolves) so buildSettingsPanel() - built right after this
  // function returns, well before that fetch is likely to have finished -
  // shows the right depth selected immediately instead of always
  // flashing "light" first.
  const initialDepth = savedSettings.catalogDepth || "light";
  engine.catalogDepth = initialDepth;
  engine.loadCatalog(initialDepth).then(() => engine.render()).catch((e) => console.error("[navigator] catalog load", e));

  // lightUI/uiZoom track the app's own live theme/UI-size, not saved
  // preferences of their own (unlike the fields loadDisplaySettings()
  // restores above) — they'd just drift out of sync with the real
  // setting otherwise. Gilles, 2026-09-30, having switched the app to
  // light theme: "en mode clair le fond reste noir ... il faut aussi
  // inverser les couleurs"; 2026-10-02, on the UI-size slider: "il
  // faudrait que les libelles ... tiennent compte du niveau d'affichage
  // Tt". The app sets <html data-theme> on toggle and publishes the UI
  // size as <html>'s own --ui-zoom custom property (both in app.js) -
  // watch both attributes directly (setProperty() touches the `style`
  // attribute's own serialised value, which "style" in attributeFilter
  // does catch).
  const themeObserver = new MutationObserver(() => {
    engine.lightUI = document.documentElement.getAttribute("data-theme") === "light";
    engine.uiZoom = readUiZoom();
    engine.render();
  });
  themeObserver.observe(document.documentElement, { attributes: true, attributeFilter: ["data-theme", "style"] });

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

  // Telescope position (Gilles, 2026-10-03: "affiche la position du
  // telescope (repere orange)") - navigator.cpp's own mountposition, fed
  // straight from the mount's INDI EQUATORIAL_EOD_COORD (RA in HOURS,
  // of-date - see engine.mountRaDeg's own doc comment in sky-engine.js
  // for why that's a different conversion than everything else here uses;
  // /15 here mirrors the *15 templates/navigator.js already does the
  // other way for target.targetra).
  const unsubMount = client.subscribe(client.topics.property(moduleName, "mountposition"), (prop) => {
    const ra = prop && prop.elements.RA && prop.elements.RA.value;
    const dec = prop && prop.elements.DEC && prop.elements.DEC.value;
    if (typeof ra === "number" && typeof dec === "number") {
      engine.mountRaDeg = ra * 15;
      engine.mountDecDeg = dec;
      engine.render();
    }
  });

  // Sensor field of view (Gilles, 2026-10-03: "le champ du capteur (cadre
  // rouge)") - the camera-FOV-not-eyepiece decision from this chantier's
  // own requirements (Gilles, 2026-09-29: "on peut determiner le champ
  // avec [le GPS, la camera et l'optique]"). `sampling` (arcsec/px) is
  // always available once camera+optic are configured, independent of any
  // plate solve - see OST/libs/model/element/common.h's own doc comment
  // on ImgData::sampling.
  const unsubImage = client.subscribe(client.topics.property(moduleName, "image"), (prop) => {
    const img = prop && prop.elements.image && prop.elements.image.value;
    const sampling = img && img.sampling;
    if (sampling > 0 && img.width > 0 && img.height > 0) {
      engine.sensorFovWidthDeg = (img.width * sampling) / 3600;
      engine.sensorFovHeightDeg = (img.height * sampling) / 3600;
      // Only meaningful once a plate solve has actually succeeded (see
      // ImgData::solverOrientation's own doc comment in common.h) -
      // Gilles, 2026-10-0X: "on ne tient pas compte de l'orientation
      // (pourtant je te l'ai demande)" - navigator.cpp's own
      // OnSucessSolve() has populated this since the earlier backend
      // work in this chantier, it just wasn't read here yet.
      if (img.issolved) engine.sensorOrientationDeg = img.solverorientation || 0;
      engine.render();
    }
  });

  // The sky visibly moves even standing still - a light periodic refresh
  // keeps it live without needing a real interaction. 30s is plenty (the
  // whole dome shifts ~0.125deg/30s from Earth's rotation, imperceptible
  // faster than that anyway).
  const timer = setInterval(() => engine.render(), 30000);

  return {
    engine,
    dispose: () => {
      unsub();
      unsubMount();
      unsubImage();
      ro.disconnect();
      themeObserver.disconnect();
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
    const sky = buildSkyDome(client, moduleName, selection.onSelect);
    disposers.push(sky.dispose);

    const header = Panels.make({ id: "header", title: mod.infos.modulelabel || mod.infos.label || moduleName, x: 20, y: 92, w: 320 });
    header.appendChild(el("div", { class: "muted small", text: mod.infos.description || "" }));

    const selectionPanel = Panels.make({ id: "selection", title: t("navigator.selection"), x: 340, y: 92, w: 300 });
    selectionPanel.appendChild(track(selection.node));
    quickNavEntries.push({ id: "selection", label: t("navigator.selection") });

    if (sky.engine) {
      const settingsPanel = Panels.make({ id: "settings", title: t("navigator.displaySettings"), x: 660, y: 92, w: 260, collapsed: true });
      settingsPanel.appendChild(track(buildSettingsPanel(sky.engine)));
      quickNavEntries.push({ id: "settings", label: t("navigator.settingsQuicknav"), icon: "tune" });
    }

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
      const b = Panels.make({ id: "other", title: t("common.other"), x: 340, y: 500, w: 340 });
      for (const name of rest) b.appendChild(track(renderProperty(client, moduleName, name)));
      quickNavEntries.push({ id: "other", label: t("common.other") });
    }
    updateQuickNav(quickNavEntries);
  }

  const unsub = client.subscribe(client.topics.module(moduleName), (mod) => {
    if (!mod) {
      dispose();
      Panels.clear();
      updateQuickNav([]);
      const b = Panels.make({ id: "missing", title: moduleName, x: 20, y: 92, w: 320 });
      b.appendChild(el("div", { class: "muted", text: t("common.moduleNotFound") }));
      return;
    }
    build(mod);
  });

  return { dispose: () => { unsub(); dispose(); Panels.clear(); updateQuickNav([]); } };
}
