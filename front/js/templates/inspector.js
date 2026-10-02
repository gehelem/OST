// templates/inspector.js — scaffolded template for the "inspector" module
// (see scaffold.js's own header). Real property names confirmed against
// src/modules/inspector/inspector.json / libs/indimodule.cpp's
// defineMeAsImager() (called directly by inspector.cpp): analysis
// (hasprofile:true - inspector/collimator mode), fileselect (pick a file
// to reload), parameters (hasprofile:true, generic imager settings -
// zoning), focusparams (hasprofile:true - offset), states (idle/shooting/
// analyzing), correction (collimation vector: cx/cy/amplitude/
// convergencex/convergencey/quality), screws (screw1/2/3), stars
// (hasGrid - detected star list), hfr/shape/collim/corners (each a single
// "img" element - one of the inspector's own diagnostic maps) - plus the
// generic imager's own image and actions.
//
// Rooster's own inspector.component.html combines analysis + a single
// element each from parameters/focusparams into one card via its own
// [onlyElements] filter - ostcore's renderProperty has no partial-element
// filter yet, so these three get their own FULL property panel instead
// (more reachable, not less - same tradeoff sequencer.js's own "sequence"
// panel already accepted for a hasprofile property).

import { buildScaffoldTemplate } from "./scaffold.js";

export const mount = buildScaffoldTemplate([
  { id: "analysis", x: 20, y: 330, w: 300 },
  { id: "fileselect", x: 20, y: 460, w: 300 },
  { id: "parameters", x: 350, y: 330, w: 260 },
  { id: "focusparams", x: 350, y: 460, w: 260 },
  { id: "states", x: 630, y: 330, w: 220 },
  { id: "correction", x: 630, y: 460, w: 260 },
  { id: "screws", x: 20, y: 580, w: 300 },
  { id: "stars", x: 350, y: 580, w: 300 },
  { id: "hfr", x: 910, y: 92, w: 220 },
  { id: "shape", x: 910, y: 240, w: 220 },
  { id: "collim", x: 910, y: 388, w: 220 },
  { id: "corners", x: 910, y: 536, w: 220 },
]);
