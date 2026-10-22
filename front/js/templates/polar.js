// templates/polar.js — scaffolded template for the "polar" module (see
// scaffold.js's own header). Real property names confirmed against
// src/modules/polar/polar.json / libs/indimodule.cpp's defineMeAsImager()
// (called directly by polar.cpp, no giveMeATarget()-style extras beyond
// that): values (ra0/de0/t0 + ra1/de1/t1 + ra2/de2/t2, the 3 measurement
// points), errors (erraz/erralt/errtot + their arcmin counterparts),
// states (idle/moving/shooting/solving/compute) — plus the generic
// imager's own parameters (hasprofile:true, Parameters modal only) and
// image, both generic to any IndiModule via defineMeAsImager().
//
// Rooster's own polar.component.html hand-formats states/errors as light
// dots and split degree/arcmin values rather than generic property
// panels, and never surfaces "values" at all — not reproduced here (no
// onlyElements-style partial rendering in ostcore yet, see inspector.js's
// own note on the same tradeoff); "values" just falls into the generic
// catch-all "Other" panel scaffold.js already provides for anything a
// template doesn't explicitly list.

import { buildScaffoldTemplate } from "./scaffold.js";

export const mount = buildScaffoldTemplate([
  // Folded into the Actions panel's own optional slot instead of its own
  // standalone window (Gilles, 2026-10-22) - same move as inspector's
  // Analysis selection/planner's progress.
  { id: "states", foldIntoActions: true },
  { id: "errors", x: 350, y: 330, w: 300 },
]);
