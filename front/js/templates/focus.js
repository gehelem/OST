// templates/focus.js — scaffolded template for the "focus" module (see
// scaffold.js's own header). Real property names confirmed against
// src/modules/focus/focus.json / libs/indimodule.cpp's defineMeAsFocuser():
// values (V-curve data — hasGrid, graphType XY, graphParams focpos/
// loopHFRavg — rendered by the generic renderer's own chart.js, not a
// bespoke error-bars/fit-curve chart like ostnoctua's focus.js has; add one
// here the same way guider's drift/guiding charts were added, if wanted),
// results (hfr/pos readout), progress (spinner), zones (zone/bestpos grid) —
// plus parameters (hasprofile:true, Parameters modal only, not a panel),
// actions (abortfocus/autofocus/loop) and image, both generic to any
// IndiModule via defineMeAsFocuser()/defineMeAsImager().

import { buildScaffoldTemplate } from "./scaffold.js";

export const mount = buildScaffoldTemplate([
  { id: "results", x: 20, y: 330, w: 300 },
  { id: "progress", x: 20, y: 450, w: 300 },
  { id: "values", title: "Focus curve", x: 350, y: 92, w: 300 },
  { id: "zones", x: 350, y: 330, w: 300 },
]);
