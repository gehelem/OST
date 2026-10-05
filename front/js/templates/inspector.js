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
// "img" element - one of the inspector's own diagnostic maps, merged into
// one "maps" grid panel below rather than 4 separate ones, see that
// panelSpec's own comment) - plus the generic imager's own image and
// actions.
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
import { buildImageGridPanel } from "../imagelayer.js";

// hfr/shape/collim/corners used to each get their own small panel (4
// extra floating windows, each barely big enough to read the map inside)
// — merged into one 2x2 grid of clickable thumbnails instead, Gilles,
// 2026-10-05, after a real "Shoot" run: "je cherche une facon ergonomique
// d'afficher les 4 autres [cartes], autrement que dans ces petites
// fenetres" — picked over a tabbed one-at-a-time alternative specifically
// so all 4 stay comparable at a glance (see imagelayer.js's own
// buildImageGridPanel() doc comment for the full reasoning).
// Exported too — reused as-is (same 4 properties) by views.js's Home
// dashboard, Gilles, 2026-10-06: "sur inspector ajoute les 4 images de
// controle en vignettes toutes petites".
export const MAP_ITEMS = [
  { propName: "hfr", elName: "image", label: "HFR map" },
  { propName: "shape", elName: "image", label: "Shape map" },
  { propName: "collim", elName: "image", label: "Collimation map" },
  { propName: "corners", elName: "image", label: "Corners map" },
];

export const mount = buildScaffoldTemplate([
  { id: "analysis", x: 20, y: 400, w: 300 },
  { id: "parameters", x: 340, y: 210, w: 260 },
  { id: "focusparams", x: 340, y: 394, w: 260 },
  { id: "screws", x: 340, y: 609, w: 260 },
  { id: "fileselect", x: 340, y: 851, w: 260 },
  { id: "states", x: 620, y: 210, w: 260 },
  { id: "correction", x: 620, y: 449, w: 260 },
  { id: "stars", x: 620, y: 791, w: 260 },
  {
    id: "maps", title: "inspector.maps", x: 900, y: 92, w: 460,
    customProps: ["hfr", "shape", "collim", "corners"],
    custom: (client, moduleName) => buildImageGridPanel(client, moduleName, MAP_ITEMS),
  },
], { x: 1380, y: 92, w: 260 });
