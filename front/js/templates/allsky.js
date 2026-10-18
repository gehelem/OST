// templates/allsky.js — scaffolded template for the "allsky" module (see
// scaffold.js's own header). Real property names confirmed against
// src/modules/allsky/allsky.json / allsky.cpp (defineMeAsSequencer(), same
// capability sequencer.cpp itself uses - abortsequence/startsequence
// actions, image + parms + optic, all generic to any IndiModule):
//
// - keogram: single result image (devcat "Results").
// - timelapse: single result video (devcat "Results").
// - log: read-only grid + graph (time/snr, devcat "Results").
// - measures: live weather readings, paired with "coming" (next
//   sunrise/sunset) in the same panel body - same extra-pairing idiom as
//   parkmanager.js's own weatherstate+coming.
// - history: weather readings over time, grid + graph (devcat "Meteo").
// - archives: past days' keogram/stack/timelapse, a real grid (devcat
//   "Archives").
//
// type/daily/geo/autoparms/keepimages are all hasprofile:true (schedule
// and dynamic-exposure settings) and not listed below - stay
// Parameters-modal-only, same as Rooster's own allsky.component.html,
// which doesn't surface them either. gps/geogps (both explicitly
// enabled:false in the module's own json) fall into the generic "Other"
// catch-all along with the camera/gps/meteo device connection controls -
// unlike planner.js's own case, that catch-all isn't just internal
// bookkeeping here, so it stays.

import { buildScaffoldTemplate } from "./scaffold.js";

export const mount = buildScaffoldTemplate([
  { id: "keogram", x: 20, y: 330, w: 300 },
  { id: "timelapse", x: 350, y: 330, w: 300 },
  { id: "log", x: 680, y: 330, w: 300 },
  { id: "measures", extra: ["coming"], x: 20, y: 460, w: 300 },
  { id: "history", x: 350, y: 460, w: 360 },
  { id: "archives", x: 740, y: 460, w: 300 },
]);
