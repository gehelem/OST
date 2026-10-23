// templates/inspector.js — scaffolded template for the "inspector" module
// (see scaffold.js's own header). Real property names confirmed against
// src/modules/inspector/inspector.json / libs/indimodule.cpp's
// defineMeAsImager() (called directly by inspector.cpp).
//
// Gilles, 2026-10-06, trimming this down from a 9-panel screen (every one
// of inspector.json's own properties got its own panel at first, then
// hfr/shape/collim/corners got merged into the "maps" grid below): "garde
// uniquement les boutons et fenetres suivantes: analysis maps / actions /
// analysis selection / et le truc specifique image de fond qu'on a mis en
// place plus tot avec ses stats" - down to just "analysis" (hasprofile:true,
// its own real server label IS "Analysis selection" - confirmed, not
// guessed) + "maps" below, "actions" being automatic (every
// buildScaffoldTemplate call gets one), and the shared background+Stats
// image handling (addImageBlock, below) - its own small "Image" quick
// panel dropped too (imageQuickPanel:false), same as every other module
// that got this exact request (focus/guider/sequencer: "supprime le
// bouton 'image' et la fenetre associee", the background + Stats panel
// already cover the same figures). otherPanel:false means every other
// property this module has (fileselect/states/correction/screws/stars,
// all hasprofile:false - no Parameters-modal fallback either) is no
// longer reachable from this screen at all - a deliberate consequence of
// "garde uniquement", not an oversight.
//
// hfr/shape/collim/corners used to each get their own small panel (4
// extra floating windows, each barely big enough to read the map inside)
// — merged into one 2x2 grid of clickable thumbnails instead, Gilles,
// 2026-10-05, after a real "Shoot" run: "je cherche une facon ergonomique
// d'afficher les 4 autres [cartes], autrement que dans ces petites
// fenetres" — picked over a tabbed one-at-a-time alternative specifically
// so all 4 stay comparable at a glance (see imagelayer.js's own
// buildImageGridPanel() doc comment for the full reasoning).

import { buildScaffoldTemplate } from "./scaffold.js";
import { buildImageGridPanel } from "../imagelayer.js";
import { el } from "../dom.js";

// Exported too — reused as-is (same 4 properties) by views.js's Home
// dashboard, Gilles, 2026-10-06: "sur inspector ajoute les 4 images de
// controle en vignettes toutes petites".
export const MAP_ITEMS = [
  { propName: "hfr", elName: "image", label: "HFR map" },
  { propName: "shape", elName: "image", label: "Shape map" },
  { propName: "collim", elName: "image", label: "Collimation map" },
  { propName: "corners", elName: "image", label: "Corners map" },
];

// "Sharpness" (Laplacian variance) result box, shown only while the
// "sharpness" analysis toggle is on - same .value-row/.value-box boxed-
// figure look as focus.js's buildResultsBoxes() (Gilles, 2026-10-23: same
// presentation, for inspector's star-independent sharpness metric). Folded
// into the Actions panel's optional slot, right after "exposure" (Gilles,
// 2026-10-23: moved out of the "maps" window - more practical there, next
// to the toggle that actually controls it). Needs both "sharpness" (the
// value) and "analysis" (the toggle) - two subscriptions kept in sync
// through shared mutable state, same pattern buildImageGridPanel uses
// per-tile just above.
function buildSharpnessResultBox(client, moduleName) {
  const wrap = el("div", { class: "value-row" });
  let enabled = false;
  let latest = null;
  function render() {
    wrap.innerHTML = "";
    if (!enabled || !latest || !latest.elements.variance) return;
    const v = latest.elements.variance;
    wrap.appendChild(el("div", { class: "value-box" }, [
      el("div", { class: "value-box-label", text: v.label || "variance" }),
      el("div", { class: "value-box-number", text: (v.value || 0).toFixed(2) }),
    ]));
  }
  const unsubAnalysis = client.subscribe(client.topics.property(moduleName, "analysis"), (prop) => {
    enabled = !!(prop && prop.elements.sharpness && prop.elements.sharpness.value);
    render();
  });
  const unsubSharpness = client.subscribe(client.topics.property(moduleName, "sharpness"), (prop) => {
    latest = prop;
    render();
  });
  wrap._dispose = () => { unsubAnalysis(); unsubSharpness(); };
  return wrap;
}

export const mount = buildScaffoldTemplate(
  [
    // Folded into the Actions panel's own optional slot instead of its own
    // standalone window (Gilles, 2026-10-22) - same move as navigator's
    // Selection/Time navigator panels and focus/guider's own Final
    // result/Session values panels.
    { id: "analysis", foldIntoActions: true },
    // Just the exposure field out of "parms" (its gain/offset/bitdepth
    // siblings stay Parameters-modal-only) - a dedicated inline spot to
    // type into, requested (Gilles, 2026-10-22) right after the
    // focus-loss fix above, to directly retest a directedit slider+number
    // field living in the Actions panel.
    { id: "parms", foldIntoActions: true, includeElements: ["exposure"] },
    // Right after "exposure" (Gilles, 2026-10-23) - see buildSharpnessResultBox().
    { id: "sharpness-result", foldIntoActions: true, customProps: ["sharpness"],
      custom: (client, moduleName) => buildSharpnessResultBox(client, moduleName) },
    {
      // Icon-only too (same thread, right before: "sur le bouton analysis
      // maps laisse uniquement une icone 'experiment'").
      id: "maps", title: "inspector.maps", x: 900, y: 92, w: 460, icon: "experiment", iconOnly: true,
      customProps: ["hfr", "shape", "collim", "corners"],
      custom: (client, moduleName) => buildImageGridPanel(client, moduleName, MAP_ITEMS),
    },
  ],
  { x: 1380, y: 92, w: 260 },
  { imageQuickPanel: false, otherPanel: false },
);
