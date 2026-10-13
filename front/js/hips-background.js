// js/hips-background.js — HiPS sky-survey background for the planetarium
// dome (Gilles, 2026-10-13: "ajouter les images de fond 'hips'" — see
// notes/NOTES-frontend-ostcore-planetarium.md, "Images de fond HiPS", for
// the study/decision behind this).
//
// Approach A of the 3 weighed there: embed Aladin Lite (CDS,
// LGPL-3.0-or-later, vendored unmodified at lib/aladin.js — see that
// file's own header) as a plain background layer sitting BEHIND
// sky-engine.js's own <canvas> inside #bg-layer (same DOM-order stacking
// every other absolutely-positioned layer in this app already relies
// on), synced every render() to our own current view (center RA/Dec +
// field of view) via SkyEngine's onViewChange callback. Aladin's own UI
// chrome is disabled wholesale at init — it never shows its own
// controls, reticle, coordinate frame etc., just the sky image.
//
// Rotation: Gilles, 2026-10-13, after the first pass (no rotation sync at
// all) shipped: "les mouvements sont inverses sur l'image de fond" - the
// public reference API doc never lists a setRotation(), but grepping the
// vendored bundle itself (lib/aladin.js) directly found a real one
// (aliased setViewCenter2NorthPoleAngle - "the angle between view center
// and the North pole"), now wired via sky-engine.js's own onViewChange
// (its own rotationDeg field, negated there - degrees, positive
// counter-clockwise per Aladin's own source JSDoc, the opposite sense
// from this engine's own clockwise-from-up measurement). Confirmed
// correct live by Gilles, 2026-10-14, after that sign fix.
//
// Network: HiPS tiles are fetched through this app's own server (see
// DEFAULT_SURVEY below) rather than hitting the CDS's alasky.* servers
// directly - an nginx proxy+cache (cmake/cpack/complete/postinst's own
// "/hips-proxy/dss2color/" location) means a tile already viewed by ANY
// client through this server is served locally afterward. A genuine
// cache MISS (a tile nobody has ever viewed through this server yet)
// still needs live internet, and there is no bundled/pre-downloaded
// survey data (the full DSS2 survey is several TB) - this is an
// on-demand, shared cache, not an offline mirror.

import { el } from "./dom.js";
import { showToast } from "./toast.js";
import { t } from "./i18n.js";

// A relative path (resolved against this app's own origin), not a CDS
// preset id - Aladin Lite accepts either (its own doc: "an http url
// towards a HiPS" or "a relative path to your HiPS"). Routed through
// this server's own nginx proxy+cache (cmake/cpack/complete/postinst's
// own "/hips-proxy/dss2color/" location, proxying/caching
// alasky.cds.unistra.fr/DSS/DSSColor/) rather than hitting the CDS
// directly - Gilles, 2026-10-14: "possibilite de telecharger tout ou
// partie des ressources sur notre serveur web local ?" - every tile,
// once fetched by ANY client through this server, is served locally to
// every later request instead of going back to the internet. Still
// needs this app to actually be served BEHIND that nginx (true for
// ostserver-complete once (re)installed - not for a bare dev/static
// server with no such proxy route, where this 404s and the onFailure
// path below reports it instead of staying silently blank).
const DEFAULT_SURVEY = "/hips-proxy/dss2color";

// lib/aladin.js is an ES module with a single default export (the `A`
// namespace the upstream README calls `A.aladin()`/`A.init` on) — a plain
// dynamic import() is the natural way to defer loading its ~2.4MB (it's
// never fetched at all until the HiPS background is actually switched
// on), no <script> tag/global needed. Cached across every
// createHipsBackground() call (not just within one instance) so toggling
// the setting off then on again, or a second dome mount, doesn't re-fetch
// it a second time.
let modulePromise = null;
function loadAladin() {
  if (!modulePromise) modulePromise = import("../lib/aladin.js").then((mod) => mod.default);
  return modulePromise;
}

/**
 * Mounts a HiPS background div as `container`'s FIRST child — whatever
 * the caller appends after this call (sky-engine.js's own <canvas>) then
 * stacks visually on top of it. Returns {onFailure, setEnabled(bool),
 * sync(view), dispose()}:
 * - setEnabled(true) lazily loads+initializes Aladin on first call (never
 *   before — switching the background on is an explicit, visible choice,
 *   not something to pay the load cost for by default). If that load
 *   fails (confirmed live, 2026-10-13: Chrome on Gilles's own dev machine
 *   refuses WebGL2 context creation there, while Firefox on the same
 *   machine works fine - a browser/environment limitation, not a bug in
 *   this module) it shows a toast AND rolls `enabled` back to false
 *   itself, then calls the returned object's own `onFailure(error)`
 *   property if the caller set one (same assign-after-construction style
 *   as SkyEngine's onViewChange/onZoomChange/etc.) - so it can also reset
 *   whatever UI state (a checkbox, engine.showHips) led here, otherwise
 *   that state keeps claiming "on" while nothing is actually showing.
 * - sync({ra, dec, fovDeg, rotationDeg}) — degrees, J2000 — re-centers/
 *   re-zooms/re-rotates Aladin to match our own current view; a no-op
 *   while disabled or before Aladin has actually finished loading (the
 *   most recent view is kept and applied the moment it becomes ready
 *   instead of being lost).
 */
export function createHipsBackground(container) {
  const div = el("div", { class: "hips-bg" });
  container.insertBefore(div, container.firstChild);

  let aladin = null;
  let enabled = false;
  let pendingView = null;

  function ensureAladin() {
    if (aladin) return Promise.resolve(aladin);
    return loadAladin().then((A) => A.init.then(() => {
      if (!aladin) {
        aladin = A.aladin(div, {
          survey: DEFAULT_SURVEY,
          fov: 60,
          cooFrame: "equatorial",
          showReticle: false,
          showZoomControl: false,
          showLayersControl: false,
          showFullscreenControl: false,
          showFrame: false,
          showGotoControl: false,
          showContextMenu: false,
          showCooGrid: false,
          showCooGridControl: false,
          // Gilles, 2026-10-14: "des boites monochromes qui apparaissent
          // sous mes fenetres de controle" / "des petites vignettes pour
          // faire la pub de Aladin" - these 6 default to true in
          // lib/aladin.js (grepped its own DEFAULT_OPTIONS directly) and
          // were simply missed in the first pass above.
          showCatalog: false,
          showCooLocation: false,
          showFov: false,
          showLabels: false,
          showProjectionControl: false,
          showStatusBar: false,
        });
        if (pendingView) apply(pendingView);
      }
      return aladin;
    }));
  }

  function apply(view) {
    aladin.gotoRaDec(view.ra, view.dec);
    aladin.setFov(view.fovDeg);
    // setRotation(0) is silently rejected by lib/aladin.js itself (its own
    // `if (!g) { console.warn(...); return; }` guard - 0 is falsy in JS,
    // a real bug in the vendored library, not something to fix there) -
    // nudged off exact zero so "no rotation" still actually applies
    // instead of silently keeping whatever rotation was set previously.
    aladin.setRotation(view.rotationDeg || 1e-6);
  }

  const api = {
    onFailure: null, // caller-assignable, see this function's own doc comment
    setEnabled(v) {
      enabled = !!v;
      div.classList.toggle("active", enabled);
      if (enabled) {
        ensureAladin().catch((e) => {
          console.error("[hips-background] Aladin load failed", e);
          enabled = false;
          div.classList.remove("active");
          showToast(t("navigator.hipsBackgroundFailed") + " (" + e + ")", 3);
          if (api.onFailure) api.onFailure(e);
        });
      }
    },
    sync(view) {
      pendingView = view;
      if (enabled && aladin) apply(view);
    },
    dispose() {
      div.remove();
    },
  };
  return api;
}
