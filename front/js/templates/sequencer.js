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
    // No inner frame either (same thread, right after: "idem sur
    // 'object'").
    { id: "object", x: 350, y: 330, w: 300, framed: false },
    // Icon-only too (same thread, right after: "bouton 'progress' =>
    // seulement icone = 'progress_activity'"), and no inner frame either
    // (same thread yet again: "pas besoin du cadre car une seule
    // propriete" - the outer panel box above already gives it one).
    { id: "progress", x: 20, y: 330, w: 300, icon: "progress_activity", iconOnly: true, framed: false },
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
