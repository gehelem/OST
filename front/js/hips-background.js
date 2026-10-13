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
// Rotation is NOT synced — a search result mentioned Aladin Lite v3
// having a setRotation(), but it never appears in the actual reference
// API doc (https://aladin.cds.unistra.fr/AladinLite/doc/API/), so it's
// left unimplemented rather than guessed blindly. Our own dome allows an
// arbitrary pan orientation (trackball), not just North-up, so the HiPS
// background can visibly drift out of rotational alignment with our own
// stars/grid during a pan — acceptable for a first pass (it's an ambience
// backdrop, not meant to pixel-align with the vector overlay), to revisit
// live once Gilles can actually see it.
//
// Network: HiPS tiles are fetched live from the CDS's own alasky.*
// servers the moment this is enabled — this feature does nothing useful
// without internet access at the time it's switched on, and there is no
// bundled/offline survey data. No persistent (Cache Storage/IndexedDB)
// tile caching yet either — see the notes file's own "Mise en cache des
// tuiles" section for that separate follow-up.

import { el } from "./dom.js";
import { showToast } from "./toast.js";
import { t } from "./i18n.js";

const DEFAULT_SURVEY = "P/DSS2/color";

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
 * - sync({ra, dec, fovDeg}) — degrees, J2000 — re-centers/re-zooms Aladin
 *   to match our own current view; a no-op while disabled or before
 *   Aladin has actually finished loading (the most recent view is kept
 *   and applied the moment it becomes ready instead of being lost).
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
        });
        if (pendingView) apply(pendingView);
      }
      return aladin;
    }));
  }

  function apply(view) {
    aladin.gotoRaDec(view.ra, view.dec);
    aladin.setFov(view.fovDeg);
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
