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
// mobile-compatibility reasoning) updates the picked object's info + GOTO/
// Add-to-planner state - same interaction language as the rest of this
// app, no context menu anywhere. Originally its own floating "Selection"
// panel, later folded into "Actions" together with the time navigator
// (Gilles, 2026-10-22), see renderActionsPanel()'s own doc comment.
//
// Still not wired: search/results (text search still only reachable via
// the generic "Other" catch-all panel), the horizon/terrestrial and
// camera-field views. The visibility bar / time-scrub control (Gilles,
// 2026-10-02) IS wired since then - see buildSelectionPanel()'s own
// day/night canvas further down - this comment just hadn't been updated
// when that landed.
//
// Real property names confirmed against src/modules/navigator/navigator.json
// / libs/indimodule.cpp's defineMeAsNavigator()/giveMeALocation(): search,
// results, selectnow, target (targetname/targetra/targetde), mountposition,
// gpslocation, gpstime, location, actions (abortnavigator/gototarget/
// addtoplanner), centeringparams, slaves - plus image/optic/parms, generic
// to any imager.

import { renderProperty, updateQuickNav, getMediaBaseUrl, moduleLifecycle, renderActionsPanel, getQuickNavButton } from "../render.js";
import { el, iconButton } from "../dom.js";
import * as Panels from "../panels.js";
import { SkyEngine, ZOOM_MIN, ZOOM_MAX } from "../sky-engine.js";
import { createHipsBackground } from "../hips-background.js";
import { STAR_DEPTHS } from "../sky-catalog.js";
import { sunAltitudeCurve, altitudeCurve, mosaicTileCenterRaDec } from "../sky-projection.js";
import { t } from "../i18n.js";
import { mediaUrl } from "../../core/index.js";
import { showToast } from "../toast.js";

function fmtDeg(v) { return v.toFixed(3) + "°"; }
function fmtHours(raDeg) {
  const h = ((raDeg / 15) % 24 + 24) % 24;
  const hh = Math.floor(h);
  const mm = Math.floor((h - hh) * 60);
  const ss = Math.round(((h - hh) * 60 - mm) * 60);
  return `${hh}h${String(mm).padStart(2, "0")}m${String(ss).padStart(2, "0")}s`;
}
function fmtClock(d) {
  if (!d) return "—";
  return String(d.getHours()).padStart(2, "0") + ":" + String(d.getMinutes()).padStart(2, "0");
}

// ---- time navigator + per-object visibility arc (Gilles, 2026-10-02,
// after astro2001's own "barre de temps" + per-object altitude graph) ----

// The 24h window both new widgets share: local noon of "today" (the
// browser's own system timezone - this app has no other notion of the
// site's timezone, same pragmatic choice `new Date()` already makes
// everywhere else here) through local noon the next day, so a single
// night's dusk-to-dawn always sits comfortably inside rather than being
// split across two windows right when it matters most. `t` before local
// noon still belongs to the night that started the PREVIOUS day.
function nightWindowStart(t) {
  const noonToday = new Date(t.getFullYear(), t.getMonth(), t.getDate(), 12, 0, 0, 0);
  return t < noonToday ? new Date(noonToday.getTime() - 86400000) : noonToday;
}

// Night (very dark navy, close to this app's own dark theme background) ->
// day (sky-engine.js's own dayMode sky colour, "#8ec8e8" - reused here
// rather than inventing a second blue) - astronomical twilight (-18deg) to
// sunrise/sunset (0deg) is the fade, not a sharp cut, same as astro2001's
// own reference gradient.
function dayNightColor(altDeg) {
  const f = Math.max(0, Math.min(1, (altDeg + 18) / 18));
  const night = [8, 10, 20], day = [142, 200, 232];
  const c = night.map((n, i) => Math.round(n + (day[i] - n) * f));
  return `rgb(${c[0]},${c[1]},${c[2]})`;
}

// Shared by both canvases below: fills the day/night gradient background
// for one 24h window (`sunSamples`, from sunAltitudeCurve()) across the
// canvas's full width.
function drawDayNightBackground(ctx, w, h, sunSamples) {
  const n = sunSamples.length - 1;
  for (let i = 0; i < n; i++) {
    const x0 = (i / n) * w, x1 = ((i + 1) / n) * w;
    ctx.fillStyle = dayNightColor(sunSamples[i].altitude);
    ctx.fillRect(x0, 0, x1 - x0 + 1, h);
  }
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
/** Shared by "Add to planner"/"Add mosaic to planner" below - lets the
 * user pick which Sequencer profile the planner should use for this
 * target, instead of the backend's own former hardcoded "default" (Gilles,
 * 2026-10-21). Same controller-scope "profiles-<Type>" lov render.js's own
 * openLoadProfileDialog() already uses for a module's OWN profile picker,
 * fixed here to "profiles-Sequencer" since this picks a profile for a
 * DIFFERENT module (whichever Sequencer the planner is configured
 * against), not this module's own classname. `onConfirm(profileName)`
 * fires once, only on an actual pick-and-confirm - never on cancel. */
function openSequencerProfilePicker(client, onConfirm) {
  const select = el("select", { class: "field-input" });
  const unsubscribe = client.subscribe(client.topics.lovs(), (lovs) => {
    const lov = (lovs && lovs["profiles-Sequencer"]) || null;
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
    el("div", { class: "dialog-header" }, [el("h3", { text: t("navigator.chooseSequencerProfile") }), iconButton("close", t("common.close"), close)]),
    el("div", { class: "form-row" }, [select]),
    el("div", { class: "dialog-actions" }, [
      el("button", { class: "btn", onclick: close }, t("common.cancel")),
      el("button", { class: "btn btn-primary", onclick: () => { const picked = select.value; close(); if (picked) onConfirm(picked); } }, t("navigator.sendToPlanner")),
    ]),
  ]);
  overlay.appendChild(dialog);
  overlay.addEventListener("click", (ev) => { if (ev.target === overlay) close(); });
  document.getElementById("dialog-root").appendChild(overlay);
}

/**
 * `getEngine`/`getSimTime`: closures, not values, deliberately - this is
 * built (in mount()'s own build()) BEFORE the engine exists yet (its own
 * onSelect needs to already exist to hand to the engine's constructor),
 * and `getSimTime` reflects whatever the time-navigator panel has most
 * recently scrubbed to, read fresh on every redraw rather than captured
 * once.
 */
function buildSelectionPanel(client, moduleName, getEngine, getSimTime, setSimTime) {
  const wrap = el("div");
  const info = el("div", { class: "muted small", text: t("navigator.tapToSelect") });
  // "Center target"/"Add to planner" are covered by the generic "actions"
  // buttons above this content (Gilles, 2026-10-22: "les boutons 'actions'
  // originaux doivent rester" - no excludeElements for them anymore, see
  // renderActionsPanel()'s caller) - only the mosaic variant lives here,
  // since it's not a single backend action but a loop of them (one
  // "addtoplanner" per tile), with no generic-button equivalent possible.
  // "Mosaic definition" moved here from the display-settings panel (Gilles,
  // 2026-10-22) - sits next to the mosaic action it actually configures
  // rather than in the unrelated display-settings panel. Just toggles the
  // floating panel built once in mount(), same mechanism as a quicknav icon.
  const mosaicBtn = el("button", { type: "button", class: "btn btn-sm", text: t("navigator.mosaicDefinition") });
  mosaicBtn.addEventListener("click", () => Panels.togglePanel("mosaicDefinition"));
  const mosaicToPlannerBtn = el("button", { class: "btn btn-sm", disabled: true }, t("navigator.addMosaicToPlanner"));
  const actionsRow = el("div", { class: "form-row" }, [mosaicBtn, mosaicToPlannerBtn]);
  // The "ce soir" visibility arc (Gilles, 2026-10-02, on astro2001's own
  // per-object fiche: "modifie [...] pour afficher toutes les infos sur
  // l'objet selectionne, donc faire ce joli graphique d'altitude sur
  // 24h") + the draggable time-scrub cursor (Gilles, 2026-10-02, after
  // astro2001's own bottom time bar: "une fenetre supplementaire pour
  // naviguer dans le temps") - one canvas for both now (Gilles,
  // 2026-10-22: "je n'aurais pas voulu dupliquer l'affichage avec le
  // dessin jour/nuit et la culmination ... n'en garde qu'un qui serve aux
  // deux fonctions"), originally two separate panels each drawing their
  // own copy of the same day/night gradient + "now" cursor. The altitude
  // arc + its rise/transit/set/max-altitude readout only appear once
  // something is actually selected; the gradient/cursor/drag-to-scrub
  // work regardless, same as the old time-navigator panel did on its own.
  const chartCanvas = el("canvas", { class: "altitude-chart", width: "280", height: "90" });
  const readout = el("div", { class: "muted small mono timenav-readout" });
  const nowBtn = el("button", { class: "btn btn-sm", text: t("navigator.nowBtn") });
  const timeRow = el("div", { class: "form-row" }, [readout, el("span", { class: "spacer" }), nowBtn]);
  const infoRows = el("div", { class: "metric-row-group hidden" });
  function metricRow(label) {
    const row = el("div", { class: "metric-row" }, [el("span", { class: "label", text: label }), el("span", { class: "value" })]);
    infoRows.appendChild(row);
    return row.lastElementChild;
  }
  const riseVal = metricRow(t("navigator.rise"));
  const transitVal = metricRow(t("navigator.transit"));
  const setVal = metricRow(t("navigator.set"));
  const maxAltVal = metricRow(t("navigator.maxAltitude"));
  const airmassVal = metricRow(t("navigator.airmassAtTransit"));

  wrap.appendChild(info);
  wrap.appendChild(actionsRow);
  wrap.appendChild(chartCanvas);
  wrap.appendChild(timeRow);
  wrap.appendChild(infoRows);

  // Asks which Sequencer profile to use first (Gilles, 2026-10-21: the
  // backend used to hardcode "default" - target.targetprofile is new, read
  // by Navigator::addTargeToPlanner() instead now), reused for every tile
  // ("un seul profil pour une mosaique").
  //
  // One "add to planner" per mosaic tile (Gilles, 2026-10-21: "pour chacun
  // de ces centres il faut envoyer un ordre 'add to planner' avec le nom
  // de l'objet avec un suffixe, et les bonnes coordonnees") - same
  // target.targetname/targetra/targetde + actions.addtoplanner round trip
  // the generic "Add to planner" button uses for a single object, just
  // repeated once per (row,col) with that tile's own centre
  // (mosaicTileCenterRaDec(), pure RA/Dec tangent-plane geometry - no
  // observer/date/live view involved, unlike the dome's own preview
  // rendering) and a name
  // suffixed "<name> Mosaic - <row>x<col>" (1-indexed). FOV checked before
  // even asking for a profile - no point picking one for a mosaic that
  // can't be computed anyway.
  mosaicToPlannerBtn.addEventListener("click", () => {
    if (!lastObj) return;
    const engine = getEngine();
    if (!engine || !(engine.sensorFovWidthDeg > 0) || !(engine.sensorFovHeightDeg > 0)) {
      showToast(t("navigator.mosaicNoFov"), 2);
      return;
    }
    openSequencerProfilePicker(client, (profile) => {
      const cols = Math.max(1, engine.mosaicCols || 1);
      const rows = Math.max(1, engine.mosaicRows || 1);
      const overlapFraction = Math.max(0, Math.min(0.9, (engine.mosaicOverlapPercent || 0) / 100));
      const baseName = lastName || t("navigator.point");
      client.setValue(moduleName, "target", "targetprofile", profile);
      for (let row = 0; row < rows; row++) {
        for (let col = 0; col < cols; col++) {
          const { ra, dec } = mosaicTileCenterRaDec(
            lastObj.ra, lastObj.dec, engine.sensorFovWidthDeg, engine.sensorFovHeightDeg,
            engine.sensorOrientationDeg, cols, rows, col, row, overlapFraction,
          );
          const tileName = `${baseName} Mosaic - ${row + 1}x${col + 1}`;
          client.setValue(moduleName, "target", "targetname", tileName);
          client.setValue(moduleName, "target", "targetra", ra / 15); // degrees -> hours, see onSelect()'s own comment
          client.setValue(moduleName, "target", "targetde", dec);
          client.setValue(moduleName, "actions", "addtoplanner", true);
        }
      }
    });
  });

  let lastObj = null;
  let lastName = null;

  function redraw() {
    const engine = getEngine();
    if (!engine) return;
    const t0 = getSimTime() || new Date();
    const windowStart = nightWindowStart(t0);
    const sunSamples = sunAltitudeCurve(engine.observer, windowStart, 24, 10);

    const w = chartCanvas.width, h = chartCanvas.height;
    const ctx = chartCanvas.getContext("2d");
    ctx.clearRect(0, 0, w, h);
    drawDayNightBackground(ctx, w, h, sunSamples);

    // Local-midnight reference tick - always exactly at mid-width, since
    // the window itself is defined as starting at local noon (see
    // nightWindowStart()'s own doc comment).
    ctx.strokeStyle = "rgba(255,255,255,0.3)";
    ctx.lineWidth = 1;
    ctx.beginPath(); ctx.moveTo(w / 2, 0); ctx.lineTo(w / 2, h); ctx.stroke();

    // Altitude axis: 0deg at the bottom, 90deg at the top - below-horizon
    // stretches aren't drawn (nothing to show there, matches astro2001's
    // own look: the arc only exists while the object is actually up). Only
    // once something is actually selected - the gradient/tick/"now" cursor
    // above and below are meaningful on their own either way.
    if (lastObj) {
      const curve = altitudeCurve(lastObj.ra, lastObj.dec, engine.observer, windowStart, 24, 10);
      const n = curve.samples.length;
      ctx.strokeStyle = "#ffb74d";
      ctx.lineWidth = 2;
      ctx.beginPath();
      let penDown = false;
      for (let i = 0; i < n; i++) {
        const s = curve.samples[i];
        const x = (i / (n - 1)) * w;
        if (s.altitude < 0) { penDown = false; continue; }
        const y = h - (s.altitude / 90) * h;
        if (!penDown) { ctx.moveTo(x, y); penDown = true; } else ctx.lineTo(x, y);
      }
      ctx.stroke();

      riseVal.textContent = fmtClock(curve.riseDate);
      transitVal.textContent = fmtClock(curve.transitDate);
      setVal.textContent = fmtClock(curve.setDate);
      maxAltVal.textContent = curve.maxAltitude.toFixed(1) + "°";
      // Airmass (plane-parallel approximation, sec(zenith angle) - a rough
      // extinction proxy, not a real atmospheric model, same spirit as
      // sky-engine.js's own existing "rough, not a real extinction model"
      // dimming comment) - undefined/meaningless once the object never
      // actually clears the horizon in this window.
      airmassVal.textContent = curve.maxAltitude > 0 ? (1 / Math.sin(curve.maxAltitude * Math.PI / 180)).toFixed(2) : "—";
    }

    // "Now" cursor, drawn last so it stays legible on top of the curve.
    const frac = Math.max(0, Math.min(1, (t0.getTime() - windowStart.getTime()) / (24 * 3600000)));
    const cx = frac * w;
    ctx.strokeStyle = "#ffeb3b";
    ctx.globalAlpha = 0.8;
    ctx.lineWidth = 1;
    ctx.beginPath(); ctx.moveTo(cx, 0); ctx.lineTo(cx, h); ctx.stroke();
    ctx.globalAlpha = 1;

    readout.textContent = t0.toLocaleString();
    nowBtn.disabled = getSimTime() == null;
  }

  // Drag-to-scrub (ex-buildTimeNavPanel) - freezes the dome at the
  // scrubbed instant (`setSimTime` doesn't re-arm itself); the "Now"
  // button clears the override and resumes following real time. Works
  // regardless of whether anything is selected, same as the old separate
  // time-navigator panel did on its own.
  function xToDate(clientX) {
    const r = chartCanvas.getBoundingClientRect();
    const frac = r.width ? Math.max(0, Math.min(1, (clientX - r.left) / r.width)) : 0;
    const windowStart = nightWindowStart(getSimTime() || new Date());
    return new Date(windowStart.getTime() + frac * 24 * 3600000);
  }
  let dragging = false;
  function pointX(ev) { return ev.touches ? ev.touches[0].clientX : ev.clientX; }
  function onDown(ev) { dragging = true; onMove(ev); }
  function onMove(ev) {
    if (!dragging) return;
    setSimTime(xToDate(pointX(ev)));
    if (ev.touches) ev.preventDefault();
  }
  function onUp() { dragging = false; }
  chartCanvas.addEventListener("mousedown", onDown);
  document.addEventListener("mousemove", onMove);
  document.addEventListener("mouseup", onUp);
  chartCanvas.addEventListener("touchstart", onDown, { passive: false });
  document.addEventListener("touchmove", onMove, { passive: false });
  document.addEventListener("touchend", onUp);
  nowBtn.addEventListener("click", () => setSimTime(null));

  // Own 1s tick for a live-updating "now" cursor/readout while "following
  // now" - deliberately separate from buildSkyDome()'s own 30s dome-wide
  // refresh (this is a tiny canvas, redrawing it every second is cheap;
  // the full dome isn't). No initial call here - `getEngine()` isn't safe
  // to call yet at construction time (mount()'s own forward-reference
  // comment on `sky`); mount() kicks off the first real draw once it's
  // actually ready, via onTimeChange() below.
  const timer = setInterval(redraw, 1000);

  function onSelect(obj) {
    const name = obj.name || (obj.kind === "star" ? t("navigator.star") : obj.kind === "messier" ? obj.desig || t("navigator.dso") : t("navigator.point"));
    info.innerHTML = "";
    info.appendChild(el("div", { text: name }));
    info.appendChild(el("div", { class: "muted small mono", text: `RA ${fmtHours(obj.ra)}  Dec ${fmtDeg(obj.dec)}` }));
    if (obj.mag !== undefined) info.appendChild(el("div", { class: "muted small", text: t("navigator.magPrefix") + obj.mag }));
    mosaicToPlannerBtn.disabled = false;
    client.setValue(moduleName, "target", "targetname", name);
    client.setValue(moduleName, "target", "targetra", obj.ra / 15); // degrees -> hours, see this function's own doc comment
    client.setValue(moduleName, "target", "targetde", obj.dec);

    lastObj = obj;
    lastName = name;
    infoRows.classList.remove("hidden");
    redraw();
  }

  // Called whenever the simulated time changes, whether from this
  // widget's own drag/Now interactions (via mount()'s setSimTime) or from
  // mount() itself, once, as soon as the engine is actually ready (see
  // the timer comment above).
  function onTimeChange() { redraw(); }

  wrap._dispose = () => {
    clearInterval(timer);
    document.removeEventListener("mousemove", onMove);
    document.removeEventListener("mouseup", onUp);
    document.removeEventListener("touchmove", onMove);
    document.removeEventListener("touchend", onUp);
  };
  return { node: wrap, onSelect, onTimeChange };
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
      showPlanets: engine.showPlanets,
      showTrueHorizon: engine.showTrueHorizon,
      showEquatorialGrid: engine.showEquatorialGrid,
      showAzimuthalGrid: engine.showAzimuthalGrid,
      showMeridian: engine.showMeridian,
      showMilkyWay: engine.showMilkyWay,
      showEcliptic: engine.showEcliptic,
      showNames: engine.showNames,
      showMountPosition: engine.showMountPosition,
      showSensorFov: engine.showSensorFov,
      showSensorImage: engine.showSensorImage,
      showHips: engine.showHips,
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
function buildSettingsPanel(engine, hipsBg) {
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

  // Star catalog depth - Gilles, 2026-09-29: "leger par defaut, mais
  // parametrable par l'utilisateur - donc prevoir tous les catalogues
  // possibles des le depart" (the actual selector was deferred until now:
  // 2026-10-03, "le grand saut : on augmente la densite, on passe la
  // taille au dessus sur les catalogues", after which sky-catalog.js
  // gained the "full" depth this exposes).
  //
  // A native <select> here (Gilles, 2026-10-06, mobile/touch
  // specifically: "je n'arrive pas a modifier le 'star catalog'") went
  // through two real fixes - moving it above the long checkbox list
  // (it needed an in-panel scroll to reach, "il faut scroller pour
  // l'atteindre") - that still didn't help ("Ko sur le probleme ...
  // aucun progres"). The actual wall: desktop Chrome's own touch/device
  // emulation (everything this project can test against) still renders a
  // <select> as the DESKTOP in-page dropdown - real mobile Chrome/Android
  // WebView instead opens an OS-level native picker, a genuinely
  // different code path no amount of viewport/touch-event emulation ever
  // exercises. Every "it works" result this session got was from testing
  // the wrong implementation entirely.
  //
  // Replaced with a plain 3-button toggle group instead of chasing an
  // unreproducible native-picker bug further - same "des boutons plutot
  // que des cases a cocher/puces" convention this project already uses
  // for booleans (checkboxRow() above), now extended to this one other
  // small closed set. No native OS control involved at all, so there's no
  // platform-specific picker behavior left to break.
  const depthRow = el("div", { class: "form-row" }, [el("span", { class: "muted small", text: t("navigator.catalogDepth") })]);
  const depthBtns = el("div", { class: "form-row" });
  const depthButtons = {};
  let currentDepth = engine.catalogDepth || "light";
  function setDepthActive(depth) {
    for (const key of Object.keys(depthButtons)) depthButtons[key].classList.toggle("active", key === depth);
  }
  for (const key of Object.keys(STAR_DEPTHS)) {
    const btn = el("button", { type: "button", class: "btn btn-sm toggle-btn", title: STAR_DEPTHS[key].label, text: t("navigator.catalogDepth." + key + "Short") });
    btn.addEventListener("click", () => {
      if (currentDepth === key) return;
      currentDepth = key;
      setDepthActive(key);
      engine.loadCatalog(key).then(() => persistAndRender()).catch((e) => console.error("[navigator] catalog load", e));
    });
    depthButtons[key] = btn;
    depthBtns.appendChild(btn);
  }
  setDepthActive(currentDepth);
  wrap.appendChild(depthRow);
  wrap.appendChild(depthBtns);

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

  wrap.appendChild(checkboxRow(t("navigator.constellations"), "showConstellations"));
  wrap.appendChild(checkboxRow(t("navigator.cardinalPoints"), "showCardinals"));
  wrap.appendChild(checkboxRow(t("navigator.messierObjects"), "showMessier"));
  // Gilles, 2026-10-06: "je n'ai pas de coche pour afficher/masquer les
  // planetes" - genuinely missing (sky-engine.js's own render loop drew
  // them unconditionally until now, no show* flag existed at all).
  wrap.appendChild(checkboxRow(t("navigator.planets"), "showPlanets"));
  wrap.appendChild(checkboxRow(t("navigator.trueHorizon"), "showTrueHorizon"));
  wrap.appendChild(checkboxRow(t("navigator.equatorialGrid"), "showEquatorialGrid"));
  wrap.appendChild(checkboxRow(t("navigator.azimuthalGrid"), "showAzimuthalGrid"));
  wrap.appendChild(checkboxRow(t("navigator.meridian"), "showMeridian"));
  wrap.appendChild(checkboxRow(t("navigator.milkyWay"), "showMilkyWay"));
  wrap.appendChild(checkboxRow(t("navigator.ecliptic"), "showEcliptic"));
  wrap.appendChild(checkboxRow(t("navigator.objectNames"), "showNames"));
  wrap.appendChild(checkboxRow(t("navigator.mountPosition"), "showMountPosition"));
  wrap.appendChild(checkboxRow(t("navigator.sensorFov"), "showSensorFov"));
  wrap.appendChild(checkboxRow(t("navigator.sensorImage"), "showSensorImage"));
  wrap.appendChild(checkboxRow(t("navigator.dayMode"), "dayMode"));

  // Not a plain checkboxRow() (it also needs to flip the Aladin layer's
  // own visibility, not just this engine field + a re-render) - Gilles,
  // 2026-10-13: "ajouter les images de fond 'hips'". See js/hips-
  // background.js's own header for what this actually turns on/off.
  const hipsCb = el("input", { type: "checkbox" });
  hipsCb.checked = engine.showHips;
  hipsCb.addEventListener("change", () => {
    engine.showHips = hipsCb.checked;
    hipsBg.setEnabled(engine.showHips);
    persistAndRender();
  });
  wrap.appendChild(el("label", { class: "form-row", style: "cursor:pointer" }, [hipsCb, el("span", { text: t("navigator.hipsBackground") })]));
  // Keeps this checkbox in sync if hipsBg.onFailure (buildSkyDome) rolls
  // engine.showHips back off because Aladin actually failed to load -
  // same "one listener reflects every path" idea as onZoomChange above.
  engine.onHipsFailure = () => { hipsCb.checked = false; };

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

  wrap._dispose = () => { engine.onZoomChange = null; engine.onRotationChange = null; engine.onHipsFailure = null; };
  return wrap;
}

// Persisted the same way search-catalog prefs are (SEARCH_CATALOGS_KEY
// above) - a client-side preference with no server round-trip, own
// localStorage key rather than folding into DISPLAY_SETTINGS_KEY since
// it's not an engine field at all. Gilles, 2026-10-21: rows/cols 1-9
// (default 1), overlap 10-30% (default 20%).
const MOSAIC_SETTINGS_KEY = "ostcore-ui3.navigator.mosaic";
function loadMosaicSettings() {
  let d = {};
  try { d = JSON.parse(localStorage.getItem(MOSAIC_SETTINGS_KEY)) || {}; } catch (e) { /* ignore */ }
  const clamp = (v, lo, hi, dflt) => (Number.isFinite(v) ? Math.min(hi, Math.max(lo, v)) : dflt);
  return {
    rows: clamp(d.rows, 1, 9, 1),
    cols: clamp(d.cols, 1, 9, 1),
    overlap: clamp(d.overlap, 10, 30, 20),
  };
}
function saveMosaicSettings(state) {
  try { localStorage.setItem(MOSAIC_SETTINGS_KEY, JSON.stringify(state)); } catch (e) { /* ignore */ }
}

/** The Mosaic definition panel's body - a plain floating panel (draggable,
 * non-blocking) rather than the blocking modal this started as, so the
 * grid drawn on the dome (sky-engine.js's own framing-preview block) can
 * be watched updating live while these fields are adjusted - a modal's own
 * backdrop would hide the dome entirely regardless of where the dialog box
 * itself sat, defeating that. */
function buildMosaicDefinitionPanel(engine) {
  const state = loadMosaicSettings();
  const wrap = el("div");

  function numberField(value, min, max) {
    return el("input", { type: "number", class: "field-input tiny", min: String(min), max: String(max), step: "1", value: String(value) });
  }
  const rowsInput = numberField(state.rows, 1, 9);
  const colsInput = numberField(state.cols, 1, 9);
  const overlapInput = numberField(state.overlap, 10, 30);
  const clamp = (v, lo, hi, dflt) => (Number.isFinite(v) ? Math.min(hi, Math.max(lo, v)) : dflt);
  function persist() {
    state.rows = clamp(parseInt(rowsInput.value, 10), 1, 9, 1);
    state.cols = clamp(parseInt(colsInput.value, 10), 1, 9, 1);
    state.overlap = clamp(parseInt(overlapInput.value, 10), 10, 30, 20);
    rowsInput.value = String(state.rows);
    colsInput.value = String(state.cols);
    overlapInput.value = String(state.overlap);
    saveMosaicSettings(state);
    // Live update - the mosaic grid itself reads these same three fields
    // on the engine, not localStorage directly, same pattern as every
    // other display setting.
    engine.mosaicRows = state.rows;
    engine.mosaicCols = state.cols;
    engine.mosaicOverlapPercent = state.overlap;
    engine.render();
  }
  rowsInput.addEventListener("change", persist);
  colsInput.addEventListener("change", persist);
  overlapInput.addEventListener("change", persist);

  wrap.appendChild(el("div", { class: "form-row" }, [
    el("span", { text: t("navigator.mosaicRows") }), rowsInput,
    el("span", { text: t("navigator.mosaicCols") }), colsInput,
  ]));
  wrap.appendChild(el("div", { class: "form-row" }, [
    el("span", { text: t("navigator.mosaicOverlap") }), overlapInput, el("span", { text: "%" }),
  ]));

  wrap._dispose = () => {};
  return wrap;
}

// ---- search panel + catalog-selection modal (Gilles, 2026-10-06) --------
//
// "Fonction de recherche multicriteres ... nouveau bouton 'search' ...
// associe a une nouvelle fenetre ... zone de saisie + bouton lancer +
// bouton catalogues ... liste des resultats ... clic sur une ligne =
// centrage + selection". The "multicriteres" part turned out to live
// entirely in WHICH catalogs get searched, not in the search term itself
// - the backend's own protocol (search.name + posticon "forward" -> the
// `results` grid, confirmed against OstErix's catalog-dialog.component.ts
// and navigator.cpp/dbmanager.cpp's searchCatalog()) takes a single
// free-text term and searches EVERY catalog row in one SQL LIKE query -
// there is no server-side per-catalog filter at all, every hit just
// carries its own `catalog` column back. So:
// - local catalogs (stars/messier/planets, client-side data already
//   loaded - see SkyEngine.searchLocal()) genuinely run as independent,
//   separately-filterable searches - the "en parallele" part of Gilles's
//   own suggestion applies here, and for free: they're synchronous array
//   scans, done before the backend has even replied.
// - the backend side is always ONE single catalog, searched or not as a
//   whole - Gilles, 2026-10-06, after noticing "Messier" (the only value
//   its own `catalog` column had on his install) "est sans doute
//   fantaisiste": that column is raw, unvetted content of a DB table with
//   no populating script left anywhere in this repo, not a real
//   taxonomy this client should present as choosable sub-catalogs (an
//   earlier version of this code discovered + listed those names as
//   individual checkboxes - dropped on purpose, per Gilles: "on garde la
//   section backend comme un catalogue a part entiere, sans presumer du
//   type de contenu"). Still shown as a column in the results table
//   (purely informational), just never filtered on.
// Results still stream in "au fil de l'eau" as asked: local results
// render immediately (synchronous), the backend's reply (a real network
// round-trip) gets appended to the same list whenever it arrives.
const SEARCH_CATALOGS_KEY = "ostcore-ui3.navigator.searchCatalogs";
function loadSearchCatalogs() {
  let d = {};
  try { d = JSON.parse(localStorage.getItem(SEARCH_CATALOGS_KEY)) || {}; } catch (e) { /* ignore */ }
  return {
    local: Object.assign({ stars: true, messier: true, planets: true }, d.local),
    backend: d.backend !== false,
  };
}
function saveSearchCatalogs(state) {
  try { localStorage.setItem(SEARCH_CATALOGS_KEY, JSON.stringify(state)); } catch (e) { /* ignore */ }
}

/**
 * Backend `results` property -> the same flat result shape
 * searchLocal() returns, so the search panel can treat every source
 * uniformly. Reads `prop.rows` (array of {header: value} objects,
 * datastore.js's own live-maintained grid representation - see
 * render.js's renderGridSection(), the only other consumer of a grid
 * property, which already only ever reads `.rows` too), NOT
 * `prop.grid`/`.gridheaders` (the raw wire array-of-arrays) - found live,
 * 2026-10-06, after a zero-match search ("gr", grid-reset) showed the
 * PREVIOUS search's rows back: `.grid` is only ever written once per
 * dump/"ap" replace (_normalizeProperty()'s own doc comment), `.gr`
 * (zero matches) only clears `.rows`, so `.grid` goes stale - exactly
 * the field this used to read.
 *
 * Two wire-format quirks confirmed against a real search (not guessed):
 * - `RA` is in HOURS (navigator.cpp/dbmanager.cpp store raw INDI-style
 *   equatorial coordinates, same convention as mountposition.RA/
 *   target.targetra elsewhere in this file) - *15 to this file's own
 *   degrees convention, right at this wire boundary, same pattern as
 *   buildSelectionPanel()'s own targetra conversion.
 * - `DEC` is an unsigned magnitude with the sign carried SEPARATELY in
 *   `NS` ("+"/"-") - confirmed live (e.g. a real M8 row: DEC=24.38,
 *   NS="-", the Lagoon Nebula's actual declination is -24.38degC).
 * `ofDate: false` - this is a static catalog table, not a live-computed
 * position (see SkyEngine.centerOn()'s own doc comment on why that
 * matters for which conversion centers on it correctly). `catalog` is
 * kept as-is for display only - see this section's own header comment
 * on why it's never used to filter.
 */
function parseBackendResults(prop) {
  if (!prop || !prop.rows) return [];
  return prop.rows.map((row) => {
    const decMag = parseFloat(row.DEC) || 0;
    return {
      kind: "backend",
      catalog: row.catalog || "",
      code: row.code || "",
      name: row.name || row.alias || row.code || "",
      ra: (parseFloat(row.RA) || 0) * 15,
      dec: row.NS === "-" ? -decMag : decMag,
      mag: parseFloat(row.mag) || 0,
      diam: parseFloat(row.diam) || 0,
      ofDate: false,
    };
  });
}

/**
 * The "Search" panel: a search term + launch button + catalog-selection
 * modal button, and the result list below (`buildSearchResultsRow()`'s
 * own clicks center the view + feed the Selection panel, same as tapping
 * the dome directly - `onSelect` is `buildSelectionPanel()`'s own, not a
 * second copy).
 *
 * Single-flight by construction: `requestBackendSearch()` cancels
 * whatever backend reply it was still waiting on before sending a new
 * request - the backend's own `results` property has no per-request
 * correlation id at all (confirmed against navigator.cpp - a search
 * just overwrites the one shared grid), so two in-flight requests can
 * never be told apart anyway; this way the most recent one always wins
 * instead of racing unpredictably.
 */
function buildSearchPanel(client, moduleName, engine, onSelect) {
  const wrap = el("div", { class: "search-panel" });
  const input = el("input", { class: "field-input", style: "flex:1", placeholder: t("navigator.searchPlaceholder") });
  const launchBtn = iconButton("search", t("navigator.searchLaunch"), () => runSearch());
  const catalogsBtn = iconButton("filter_alt", t("navigator.searchCatalogsBtn"), () => openCatalogModal());
  wrap.appendChild(el("div", { class: "form-row" }, [input, launchBtn, catalogsBtn]));
  const resultsEl = el("div", { class: "grid-table-wrap" });
  wrap.appendChild(resultsEl);
  input.addEventListener("keydown", (ev) => { if (ev.key === "Enter") runSearch(); });

  const state = loadSearchCatalogs();

  // Cancels itself and resubscribes fresh on every call - see this
  // section's own doc comment on why single-flight, not per-request
  // tracking (the protocol gives us nothing to track a request BY).
  let pendingUnsub = null;
  function requestBackendSearch(term, onReply) {
    if (pendingUnsub) { pendingUnsub(); pendingUnsub = null; }
    // The backend does NOT send one clean "here's the answer" reply -
    // confirmed live by tracing the raw WS traffic: a search streams as
    // a BURST of incremental, per-FIELD "ea" updates (one result row
    // built up one element at a time: DEC, then NS, then RA, ...) each
    // followed by its own "gc" (grid-create) commit once that row is
    // complete, repeated once per matching row. Treating "the next
    // message" as "the final answer" (an earlier version of this code)
    // just grabbed whatever incomplete mid-row state happened to exist
    // at that instant - sometimes an empty grid, sometimes a previous
    // search's leftover rows. `prop.rows` only reflects a genuinely
    // complete row once its own "gc" has landed (core/datastore.js's
    // _applyGridCreate - rows already accumulate correctly there, each
    // "gc" appends one), so this instead stays subscribed for the whole
    // burst and re-renders on every single update, exactly how
    // OstErix's own catalog-dialog.component.ts stays correct too: a
    // perpetual subscription for the component's lifetime, not a
    // one-shot wait. Only ever superseded by a NEWER request
    // (pendingUnsub above) or the panel's own disposal - never
    // auto-unsubscribed mid-search, since there's no "search is done"
    // signal in this protocol to key off of.
    let first = true;
    const unsub = client.subscribe(client.topics.property(moduleName, "results"), (prop) => {
      // Skip only the very first invocation - subscribe()'s own
      // synchronous replay of whatever `results` held BEFORE this
      // request even reached the server (core/datastore.js's own
      // subscribe() doc comment), not a real update at all.
      if (first) { first = false; return; }
      onReply(parseBackendResults(prop));
    });
    pendingUnsub = unsub;
    client.setValue(moduleName, "search", "name", term);
    client.clickElementIcon(2, moduleName, "search", "name"); // J2 = posticon ("forward")
  }

  function renderResults(rows) {
    resultsEl.innerHTML = "";
    if (!rows.length) { resultsEl.appendChild(el("div", { class: "muted small", text: t("navigator.searchNoResults") })); return; }
    const table = el("table", { class: "grid-table" });
    table.appendChild(el("thead", null, [el("tr", null, [
      el("th", { text: t("navigator.searchColName") }),
      el("th", { text: t("navigator.searchColCatalog") }),
      el("th", { text: t("navigator.searchColCoords") }),
      el("th", { text: t("navigator.searchColMag") }),
    ])]));
    const tbody = el("tbody");
    for (const r of rows) {
      const tr = el("tr", { class: "grid-row" });
      tr.appendChild(el("td", { text: r.name }));
      tr.appendChild(el("td", { class: "muted small", text: r.catalog }));
      tr.appendChild(el("td", { class: "muted small mono", text: `${fmtHours(r.ra)}  ${fmtDeg(r.dec)}` }));
      tr.appendChild(el("td", { class: "muted small", text: r.mag !== undefined ? r.mag.toFixed(1) : "" }));
      tr.addEventListener("click", () => {
        onSelect({ kind: r.kind === "star" || r.kind === "messier" ? r.kind : null, name: r.name, ra: r.ra, dec: r.dec, mag: r.mag });
        if (engine) engine.centerOn(r.ra, r.dec, { ofDate: r.ofDate });
      });
      tbody.appendChild(tr);
    }
    table.appendChild(tbody);
    resultsEl.appendChild(table);
  }

  function runSearch() {
    const term = input.value.trim();
    if (!term) return;
    const localKeys = new Set(Object.keys(state.local).filter((k) => state.local[k]));
    // Fixed for this search - only the backend side streams in over
    // several events (see requestBackendSearch()'s own doc comment), so
    // each of those re-renders `localResults.concat(...)` fresh rather
    // than appending onto a running total (prop.rows is already the
    // full cumulative list as of that event, not a delta - concatenating
    // it again on top of itself every time would just duplicate rows).
    const localResults = engine ? engine.searchLocal(term, localKeys) : [];
    renderResults(localResults);

    if (state.backend) {
      requestBackendSearch(term, (backendRows) => renderResults(localResults.concat(backendRows)));
    }
  }

  /** The catalog-selection modal (Gilles: "importante et compliquee" at
   * first, simplified same day once the backend turned out to have no
   * real per-catalog query - see this section's own header comment).
   * Same `.dialog-overlay`/`.dialog` shell as app.js's own Parameters
   * dialog/render.js's grid-row dialogs - not reused directly (both are
   * module-private to their own file), rebuilt here the same way app.js
   * already does for its own bespoke dialog. */
  function openCatalogModal() {
    const overlay = el("div", { class: "dialog-overlay" });
    const close = () => overlay.remove();
    const localBox = el("div");
    for (const key of ["stars", "messier", "planets"]) {
      const cb = el("input", { type: "checkbox" });
      cb.checked = !!state.local[key];
      cb.addEventListener("change", () => { state.local[key] = cb.checked; saveSearchCatalogs(state); });
      localBox.appendChild(el("label", { class: "form-row", style: "cursor:pointer" }, [cb, el("span", { text: t("navigator.searchCatalog." + key) })]));
    }
    const backendCb = el("input", { type: "checkbox" });
    backendCb.checked = state.backend;
    backendCb.addEventListener("change", () => { state.backend = backendCb.checked; saveSearchCatalogs(state); });
    const backendBox = el("label", { class: "form-row", style: "cursor:pointer" }, [backendCb, el("span", { text: t("navigator.searchCatalog.backend") })]);

    const dialog = el("div", { class: "dialog" }, [
      el("div", { class: "dialog-header" }, [el("h3", { text: t("navigator.searchCatalogsTitle") }), iconButton("close", t("common.close"), close)]),
      el("div", { class: "muted small", text: t("navigator.searchLocalCatalogsLabel") }),
      localBox,
      el("div", { class: "muted small", text: t("navigator.searchBackendCatalogsLabel") }),
      backendBox,
    ]);
    overlay.appendChild(dialog);
    overlay.addEventListener("click", (ev) => { if (ev.target === overlay) close(); });
    document.getElementById("dialog-root").appendChild(overlay);
  }

  wrap._dispose = () => { if (pendingUnsub) pendingUnsub(); };
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

function buildSkyDome(client, moduleName, onSelect, getTime) {
  const bgLayer = document.getElementById("bg-layer");
  if (!bgLayer) return { dispose: () => {}, engine: null };
  bgLayer.classList.add("sky-active");
  bgLayer.innerHTML = "";
  // Created before the canvas (js/hips-background.js's own
  // createHipsBackground() inserts itself as bgLayer's first child
  // either way, but this keeps the actual DOM order - and therefore the
  // visual stacking, background behind canvas - obvious at a glance too).
  const hipsBg = createHipsBackground(bgLayer);
  const canvas = el("canvas", { id: "sky-canvas" });
  bgLayer.appendChild(canvas);

  const engine = new SkyEngine(canvas, {
    onNeedsRender: () => engine.render(),
    onSelect,
    getTime,
    lightUI: document.documentElement.getAttribute("data-theme") === "light",
    uiZoom: readUiZoom(),
  });
  engine.onViewChange = (view) => hipsBg.sync(view);
  // Rolls the setting back off if Aladin actually failed to load/init
  // (js/hips-background.js's own onFailure - e.g. WebGL2 unavailable,
  // confirmed live 2026-10-13 on Chrome specifically) - without this,
  // engine.showHips (and the settings checkbox) would keep claiming "on"
  // while render() still skips its own opaque background fill for
  // nothing, leaving the dome visibly blank instead of just reverting.
  hipsBg.onFailure = () => {
    engine.showHips = false;
    saveDisplaySettings(engine);
    engine.render();
    if (engine.onHipsFailure) engine.onHipsFailure();
  };
  // Applied before the catalog/first render below, not in the settings
  // panel (built later, after this function returns) — so the very first
  // frame already reflects saved settings instead of a brief flash at the
  // defaults. buildSettingsPanel() takes it from here for the ongoing
  // wheel-zoom -> persist wiring (engine.onZoomChange).
  const savedSettings = loadDisplaySettings();
  Object.assign(engine, savedSettings);
  const savedMosaic = loadMosaicSettings();
  engine.mosaicRows = savedMosaic.rows;
  engine.mosaicCols = savedMosaic.cols;
  engine.mosaicOverlapPercent = savedMosaic.overlap;
  hipsBg.setEnabled(engine.showHips);
  // "leger par defaut, mais parametrable" (Gilles, 2026-09-29) - actually
  // reads back whichever depth was last chosen (buildSettingsPanel()'s own
  // selector, added 2026-10-03: "le grand saut ... on passe la taille au
  // dessus sur les catalogues") instead of hardcoding one regardless. Set
  // synchronously here (loadCatalog() itself only sets it once the fetch
  // actually resolves) so buildSettingsPanel() - built right after this
  // function returns, well before that fetch is likely to have finished -
  // shows the right depth selected immediately instead of always flashing
  // the fallback first. That fallback was "light" until Gilles, 2026-10-16,
  // after settling on the other display-settings defaults: "coche aussi
  // ... l'affichage deep pour le nombre d'etoiles".
  const initialDepth = savedSettings.catalogDepth || "deep";
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
    // Keep the "center on mount" quicknav button's enabled state current
    // even though this doesn't trigger a module rebuild (see that entry's
    // own doc comment in mount()'s build()) - a no-op before that button
    // exists yet (first dump, build() hasn't reached updateQuickNav() yet).
    const btn = getQuickNavButton("centerOnMount");
    if (btn) btn.disabled = engine.mountRaDeg === null;
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
      // The captured JPEG itself, warped into the FOV frame (Gilles,
      // 2026-10-01: "dans le cadre du capteur de la camera essaie de
      // m'afficher l'image prise ... toujours en optionnel") - same
      // urljpeg/urlthumbnail + cache-busting-query-param convention as
      // js/imagelayer.js's own jpegSrc(), not reused directly since that
      // one takes a property-element wrapper rather than the already-
      // unwrapped `img` this handler already has. Gated on issolved, same
      // as sensorOrientationDeg just above - the frame's own placement
      // (and therefore where the image would land inside it) is only
      // trustworthy once solved.
      if (img.issolved) {
        const path = img.urljpeg || img.urlthumbnail;
        if (path) engine.setSensorImage(mediaUrl(getMediaBaseUrl(), path) + "?t=" + Date.now());
      }
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
    hips: hipsBg,
    dispose: () => {
      unsub();
      unsubMount();
      unsubImage();
      ro.disconnect();
      themeObserver.disconnect();
      clearInterval(timer);
      engine.destroy();
      hipsBg.dispose();
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

  // Lives at mount()'s own scope, not build()'s - build() re-runs on every
  // module update (any property change), and a scrubbed time must survive
  // that, not reset to "now" on the next server dump.
  let simulatedTime = null;

  function build(mod) {
    dispose();
    Panels.clear();
    const has = (name) => name in mod.properties;
    const quickNavEntries = [];

    // `sky` is referenced inside this closure before it's assigned below -
    // safe, since it's never actually CALLED until well after this whole
    // function has finished running (a user interaction), by which point
    // `sky` is long since set. Same forward-reference pattern app.js's own
    // logToggleEl listener already relies on (see its own comment).
    // `setSimTime` itself is a hoisted function declaration, so passing it
    // here by value (not wrapped the same way) is already safe regardless
    // of where it's textually defined below.
    const selection = buildSelectionPanel(client, moduleName, () => sky.engine, () => simulatedTime, setSimTime);
    const sky = buildSkyDome(client, moduleName, selection.onSelect, () => simulatedTime || new Date());
    disposers.push(sky.dispose);

    // Single entry point for every time change (selection.node's own
    // drag/Now controls) - redraws the dome at the new instant and keeps
    // the Actions panel's own "now" cursor (and, if the window itself
    // shifted, the whole altitude curve) in sync.
    function setSimTime(d) {
      simulatedTime = d;
      if (sky.engine) sky.engine.render();
      selection.onTimeChange();
    }
    // First real draw, now that `sky` is actually ready (selection.node's
    // own canvas couldn't safely call getEngine() any earlier - see its
    // own doc comment).
    selection.onTimeChange();

    // No "header" panel any more (render.js's own renderModuleView doc
    // comment: Gilles, 2026-10-03, "la supprimer completement").

    // "Actions" first in the quicknav bar (Gilles, 2026-10-03: "bouton
    // 'actions' en premier") - every other template already calls this
    // first (focus.js/guider.js's own build()); navigator.js was the one
    // outlier, pushing it after Selection/Settings/Time instead.
    //
    // Selection and the time navigator used to be their own panels;
    // folded in here instead (Gilles, 2026-10-22: "ce qui me permettrait
    // de supprimer la fenetre selection", then "integre la fonction de la
    // fenetre 'time navigator' dans la fenetre actions", then, on the two
    // having each drawn their own copy of the same day/night gradient/
    // cursor: "n'en garde qu'un qui serve aux deux fonctions" - now a
    // single canvas inside selection.node, see its own doc comment),
    // appended via `extra.node` right after the original generic action
    // buttons ("Center target"/"Add to planner"/"Abort navigator",
    // unchanged - "les boutons actions originaux doivent rester").
    renderActionsPanel(client, moduleName, mod, track, quickNavEntries, undefined, {
      node: selection.node,
    });

    // "Search" (Gilles, 2026-10-06: "nouveau bouton 'search' ... associe a
    // une nouvelle fenetre") - see buildSearchPanel()'s own header comment
    // for the full design. collapsed:true - Gilles, 2026-10-16: "je
    // voudrais que l'affichage par defaut des fenetres soit seulement
    // actions + selection + display settings" - every other panel in this
    // template (Search/Timenav/Location/Mountposition below) now starts
    // collapsed for that same reason, reachable via its own quicknav
    // button, which togglePanel() also raises to the front (z-order).
    // Originally collapsed for a narrower reason too (Gilles, 2026-10-06:
    // search panel was open by default, "impossible de saisir ... rien du
    // tout au toucher" - the tap was landing on a different, overlapping
    // panel's own content instead, confirmed live via elementFromPoint()).
    // `opts.collapsed` is only ever the first-ever-render default anyway
    // (panels.js's own Panels.make() doc comment) - a browser that already
    // has a saved per-panel state for this module keeps that instead, so
    // this change has no effect there until that state is cleared.
    const searchPanel = Panels.make({ id: "search", title: t("navigator.search"), x: 340, y: 580, w: 340, collapsed: true });
    searchPanel.appendChild(track(buildSearchPanel(client, moduleName, sky.engine, selection.onSelect)));
    quickNavEntries.push({ id: "search", label: t("navigator.search"), icon: "search", iconOnly: true });

    if (sky.engine) {
      const settingsPanel = Panels.make({ id: "settings", title: t("navigator.displaySettings"), x: 660, y: 92, w: 260 });
      settingsPanel.appendChild(track(buildSettingsPanel(sky.engine, sky.hips)));
      // Icon-only (Gilles, 2026-10-03: "boutons 'settings' et 'time' :
      // juste les icones aussi (qui y sont deja), plus de libelle" - same
      // icon/iconOnly fields as the Stats/Guiding graph/Drift buttons just
      // before).
      quickNavEntries.push({ id: "settings", label: t("navigator.settingsQuicknav"), icon: "tune", iconOnly: true });

      // Mosaic definition - own panel (not nested inside Settings) so it
      // can stay open, draggable, next to the dome while its grid updates
      // live (see buildMosaicDefinitionPanel()'s own doc comment). Reached
      // either from its Settings-panel button or this quicknav icon, same
      // togglePanel() either way.
      const mosaicPanel = Panels.make({ id: "mosaicDefinition", title: t("navigator.mosaicDefinition"), x: 660, y: 330, w: 260, collapsed: true });
      mosaicPanel.appendChild(track(buildMosaicDefinitionPanel(sky.engine)));
      quickNavEntries.push({ id: "mosaicDefinition", label: t("navigator.mosaicDefinition"), icon: "grid_view", iconOnly: true });

      // "Center on mount" (Gilles, 2026-10-06: "ajoute un bouton
      // 'align_flex_center' ... pas de fenetre associee ... centrer la vue
      // sur la position du telescope si elle est definie (bouton inactif
      // sinon)") - a pure action, no panel to toggle (onClick, see
      // views.js's renderQuickNav() own doc comment on that field).
      // Initial disabled state read straight off the engine (already
      // live by this point - see buildSkyDome()'s own mountposition
      // subscribe, which delivers its cached value synchronously same as
      // every other subscribe in this file); kept in sync afterwards by
      // that same handler directly flipping this button's `disabled`
      // attribute (getQuickNavButton()) - build() only reruns on a real
      // module dump, not on every mountposition tick, so there's no
      // other hook to redo this from.
      quickNavEntries.push({
        id: "centerOnMount", label: t("navigator.centerOnMount"), icon: "align_flex_center", iconOnly: true,
        disabled: sky.engine.mountRaDeg === null,
        onClick: () => sky.engine.centerOnMount(),
      });
    }

    if (has("location")) {
      const b = Panels.make({ id: "location", title: mod.properties.location.label || "Site", x: 20, y: 371, w: 300, collapsed: true });
      // framed:false (Gilles, 2026-10-03: "comme il n'y a qu'une propriete
      // dedans, ca ne sert a rien de garder le cadre, on a mis une option
      // expres pour ca") - the outer Panels.make box above already gives
      // this its own border/title bar; renderProperty()'s own inner
      // "prop-header" (status dot + repeated label) was pure duplication
      // for a panel with only one property in it.
      b.appendChild(track(renderProperty(client, moduleName, "location", { framed: false })));
      // Icon-only (Gilles, 2026-10-03: "bouton 'location' => juste icone =
      // 'location_on'" - a ligature that happens to resolve to the same
      // glyph as "place", confirmed by properly unwrapping the font's
      // GSUB ExtensionSubst (lookup type 7) this time; a first, simpler
      // check missed it - see base.css's own header comment on this
      // verification technique).
      quickNavEntries.push({ id: "location", label: mod.properties.location.label || "Site", icon: "location_on", iconOnly: true });
    }
    if (has("mountposition")) {
      const b = Panels.make({ id: "mountposition", title: mod.properties.mountposition.label || "Mount", x: 20, y: 774, w: 260, collapsed: true });
      // framed:false - same reasoning as "location" just above (Gilles,
      // 2026-10-03: "idem sur 'mount position'").
      b.appendChild(track(renderProperty(client, moduleName, "mountposition", { framed: false })));
      quickNavEntries.push({ id: "mountposition", label: "Mount" });
    }

    // No "Other" catch-all any more (Gilles, 2026-10-03: "enleve le
    // 'other' et la fenetre", same request already applied to guider.js -
    // unlike that case, this one genuinely drops reachability for
    // whatever this v1 has no dedicated panel for yet: search, target,
    // results, selectnow, gpslocation, gpstime, the generic device-
    // connection block, image/optic/parms... — worth a dedicated panel
    // later if any of those turn out to matter day-to-day).
    updateQuickNav(quickNavEntries);
  }

  return moduleLifecycle(client, moduleName, build, dispose);
}
