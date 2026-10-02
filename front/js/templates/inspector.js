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
//
// Every y below is a *measured* stack, not guessed: Playwright against a
// real live dump (12 real panels, real content - 8 action buttons
// wrapping to 3 rows, a 6-element "correction" readout, full-size map
// images at ~400px tall each...), reading each panel's own actual
// rendered height and stacking the next one 20px below it - a first
// guess at "plausible" gaps overlapped in no fewer than 9 places
// (confirmed with a getBoundingClientRect() pairwise check), the most
// consequential being "analysis" sitting on top of "actions" and eating
// every click meant for its buttons - not the rendering/send logic
// Gilles first suspected, 2026-10-0X: "j'appuis sur le bouton shoot ...
// il ne se passe rien". Column 1 stops at "analysis": anything lower
// there would collide with scaffold.js's own hardcoded (20, 620, 300)
// "other" catch-all panel (inspector's own stray "focushome"), which
// this template doesn't control.

import { buildScaffoldTemplate } from "./scaffold.js";

export const mount = buildScaffoldTemplate([
  { id: "analysis", x: 20, y: 400, w: 300 },
  { id: "parameters", x: 340, y: 210, w: 260 },
  { id: "focusparams", x: 340, y: 394, w: 260 },
  { id: "screws", x: 340, y: 609, w: 260 },
  { id: "fileselect", x: 340, y: 851, w: 260 },
  { id: "states", x: 620, y: 210, w: 260 },
  { id: "correction", x: 620, y: 449, w: 260 },
  { id: "stars", x: 620, y: 791, w: 260 },
  { id: "hfr", x: 900, y: 92, w: 220 },
  { id: "collim", x: 900, y: 540, w: 220 },
  { id: "shape", x: 1140, y: 92, w: 220 },
  { id: "corners", x: 1140, y: 540, w: 220 },
], { x: 1380, y: 92, w: 260 });
