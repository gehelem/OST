// templates/sequencer.js — scaffolded template for the "sequencer" module
// (see scaffold.js's own header). Real property names confirmed against
// src/modules/sequencer/sequencer.json / libs/indimodule.cpp's
// defineMeAsSequencer(): progress (global spinner + timing readout),
// object (label/ra/de, hasprofile:false) — plus sequence/slaves/
// parameters/guideparameters/focusparameters/focusprofiles (all
// hasprofile:true, Parameters modal only, not a panel — including
// "sequence" itself, the actual exposure list; see scaffold.js's note on
// that), actions (abortsequence/startsequence) and image, both generic to
// any IndiModule via defineMeAsSequencer()/defineMeAsImager().

import { buildScaffoldTemplate } from "./scaffold.js";

export const mount = buildScaffoldTemplate([
  { id: "object", x: 350, y: 92, w: 300 },
  { id: "progress", x: 20, y: 330, w: 300 },
]);
