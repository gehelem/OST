// templates/planner.js — scaffolded template for the "planner" module (see
// scaffold.js's own header). Real property names confirmed against
// src/modules/planner/planner.json / libs/indimodule.cpp's
// defineMeAsPlanner():
//
// - planning: the editable grid (object/ra/dec/profile), hasGrid +
//   showGrid + hasprofile:true - a real editable grid, same treatment as
//   sequencer.js's own "sequence", given its own panel for the same
//   reason.
// - liveplanning: same shape as "planning" plus a status light and a
//   progress column, permission:0 (read-only) - backend-built snapshot
//   taken on start (Planner::buildLivePlanning()), not stored in the
//   profile, same pairing as sequencer.js's own "sequence"/"livesequence".
// - progress: the global progress spinner.
//
// slaves (sequencemodule/navigatormodule instance names), visibility
// (elevation/moon thresholds) and location (from giveMeALocation()) are
// all hasprofile:true and not listed below - stay Parameters-modal-only,
// same as Rooster's own planner.component.html, which doesn't surface
// them either. actions (start/stop) is generic to any IndiModule via
// defineMeAsPlanner() - no "image" of its OWN here, unlike sequencer
// (Planner never calls defineMeAsImager()) - imageFrom below borrows its
// slave Sequencer's background image instead. quickPanel:false on it, same
// as sequencer.js's own image handling this mirrors - background + Stats
// button only, no extra small quick-stats panel.
//
// otherPanel: false - the only property that would otherwise land in the
// catch-all is "signals" (giveMeAState()'s generic state/event bookkeeping,
// not a real Planner property), same reasoning as sequencer.js's own
// otherPanel:false.

import { buildScaffoldTemplate } from "./scaffold.js";

export const mount = buildScaffoldTemplate([
  { id: "planning", x: 20, y: 92, w: 420, icon: "list", iconOnly: true, framed: false },
  { id: "liveplanning", x: 460, y: 92, w: 460, icon: "browse_activity", iconOnly: true, framed: false },
  // Folded into the Actions panel's own optional slot instead of its own
  // standalone window (Gilles, 2026-10-22) - same move as inspector's
  // Analysis selection.
  { id: "progress", foldIntoActions: true },
], undefined, {
  otherPanel: false,
  imageQuickPanel: false,
  // A key, not resolved text (see scaffold.js's own spec.title doc comment
  // on why - this opts object is built once at import time, long before a
  // language is even selected).
  imageFromLabel: "imagelayer.sequenceImage",
  imageFrom: (mod) => {
    const e = mod.properties.slaves && mod.properties.slaves.elements;
    return (e && e.sequencemodule && e.sequencemodule.value) || null;
  },
});

// Per-template reorganize layout (panels.js's own reorganizePanels()),
// same scheme as sequencer.js's own (the two modules mirror each other's
// shape - "planning"/"liveplanning" here are "sequence"/"livesequence"
// there): actions and planning stacked in the left column at a quarter
// of the display wide each, natural content height; liveplanning alone
// in the right column at half width, left edge pinned to the display's
// horizontal center (xFrac) rather than the column's own default
// right-edge anchor, which would overflow a half-width panel.
// image-stats stays hidden.
export const PANEL_LAYOUT = {
  actions: { col: 0, order: 0, wFrac: 1 / 4 },
  planning: { col: 0, order: 1, wFrac: 1 / 4 },
  liveplanning: { col: 1, order: 0, wFrac: 1 / 2, xFrac: 1 / 2 },
  "image-stats": { hidden: true },
};
