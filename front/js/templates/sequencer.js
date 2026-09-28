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
// - object (label/ra/de, hasprofile:false).
//
// slaves/parameters/guideparameters/focusparameters/focusprofiles (all
// hasprofile:true, not listed above) stay Parameters-modal-only. actions
// (abortsequence/startsequence) and image are both generic to any
// IndiModule via defineMeAsSequencer()/defineMeAsImager().

import { buildScaffoldTemplate } from "./scaffold.js";

export const mount = buildScaffoldTemplate(
  [
    { id: "sequence", x: 350, y: 92, w: 340 },
    { id: "object", x: 350, y: 330, w: 300 },
    { id: "progress", x: 20, y: 330, w: 300 },
  ],
  { x: 720, y: 92, w: 260 },
);
