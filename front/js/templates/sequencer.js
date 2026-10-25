// templates/sequencer.js — scaffolded template for the "sequencer" module
// (see scaffold.js's own header). Real property names confirmed against
// src/modules/sequencer/sequencer.json / libs/indimodule.cpp's
// defineMeAsSequencer():
//
// - sequence: the actual exposure list (filter/exposure/count), hasGrid +
//   showGrid + permission:2 — a real editable grid, rendered as such by
//   render.js's generic renderGridSection() (add/edit/delete row).
//   hasprofile:true like several sibling properties here, but explicitly
//   given its own panel anyway (scaffold.js's hasprofile-exclusion only
//   applies to properties NOT listed in panelSpecs) — Gilles, 2026-09-29:
//   "sequence pour une fenetre qui montre le detail de la sequence (grille
//   editable)".
// - progress: global/totaltime spinners + duration readouts — Gilles:
//   "avancement pour voir les donnees d'avancement".
// - livesequence: same shape as "sequence" plus two per-line progress
//   columns, permission:0 (read-only) - backend-built snapshot taken on
//   start (Sequencer::buildLiveSequence()), not stored in the profile
//   (issue #241). renderGridSection() hides add/edit/delete/move for a
//   permission:0 grid, and renders a "prg"-typed column as a real
//   progress bar (renderDisplayOnly()) instead of raw text.
// - object (label/ra/de, hasprofile:false).
//
// slaves/parameters/guideparameters/focusparameters/focusprofiles (all
// hasprofile:true, not listed above) stay Parameters-modal-only. actions
// (abortsequence/startsequence) and image are both generic to any
// IndiModule via defineMeAsSequencer()/defineMeAsImager().

import { buildScaffoldTemplate } from "./scaffold.js";

export const mount = buildScaffoldTemplate(
  [
    // Icon-only (Gilles, 2026-10-03: "bouton 'sequence' => icone seulement
    // = 'list'") and no inner frame either (same thread, right after:
    // "idem sur 'sequence'").
    { id: "sequence", x: 350, y: 92, w: 340, icon: "list", iconOnly: true, framed: false },
    // "progress" and "object" folded into the Actions panel's own
    // optional slot instead of their own standalone windows (Gilles,
    // 2026-10-22) - same move as polar's states/inspector's Analysis
    // selection. This order (progress above object) is deliberate.
    { id: "progress", foldIntoActions: true },
    { id: "object", foldIntoActions: true },
    // Run-time copy of "sequence" with per-line progress, read-only
    // (permission:0) - rebuilt on each start, not persisted (issue #241).
    // Placed/sized by guesswork, not measured against a real dump - drag
    // position persists per (scope,id) regardless, see panels.js.
    { id: "livesequence", x: 20, y: 92, w: 340, icon: "browse_activity", iconOnly: true, framed: false },
  ],
  { x: 720, y: 92, w: 260 },
  // No small "Image" quick-stats panel/button (Gilles, 2026-10-03: "enleve
  // aussi le bouton 'image' et la fenetre associee") - the background
  // already shows the picture full-screen, and the Stats panel/button
  // already covers the same figures in more depth.
  //
  // No "Other" catch-all either (same thread, right after: "enleve aussi
  // le bouton 'other' et sa fenetre") - every one of sequencer.json's own
  // remaining properties (slaves/parameters/guideparameters/
  // focusparameters/focusprofiles) is hasprofile:true and already reaches
  // the Parameters modal on its own, so nothing module-specific is lost
  // here - but "Other" also carried the generic per-device
  // connect/disconnect controls every IndiModule gets (libs/indimodule.cpp's
  // "devicesactions"/"devices"/"server", not in sequencer.json itself),
  // which this drops too.
  { imageQuickPanel: false, otherPanel: false },
);

// Per-template reorganize layout (panels.js's own reorganizePanels()),
// same scheme as guider.js/focus.js: actions and sequence stacked in the
// left column at a quarter of the display wide each, natural content
// height; livesequence alone in the right column at half width, left
// edge pinned to the display's horizontal center (xFrac) rather than the
// column's own default right-edge anchor, which would overflow a
// half-width panel. image-stats stays hidden.
export const PANEL_LAYOUT = {
  actions: { col: 0, order: 0, wFrac: 1 / 4 },
  sequence: { col: 0, order: 1, wFrac: 1 / 4 },
  livesequence: { col: 1, order: 0, wFrac: 1 / 2, xFrac: 1 / 2 },
  "image-stats": { hidden: true },
};
