// templates/parkmanager.js — scaffolded template for the "parkmanager"
// module (see scaffold.js's own header). Real property names confirmed
// against src/modules/parkmanager/parkmanager.json / libs/indimodule.cpp's
// giveMeAnActions()/giveMeALocation() (parkmanager.cpp never calls
// defineMeAsImager() - no "image" property here, matching Rooster's own
// parkmanager.component.html, which has no image card either):
// slaves (hasprofile:true - plannermodule, stays Parameters-modal-only,
// same as any hasprofile property this template doesn't list below),
// mountstate (RA/DEC/home/parked/tracking), domestate (parked/
// shutterclosed), weatherstate (global) + coming (sunrise/sunset, shown
// alongside weatherstate via panelSpecs' own "extra" - the same pairing
// guider.js uses for values+calibrationvalues), weatherrules/timerules
// (both hasprofile:true, given their own panel like sequencer.js's own
// "sequence" since Rooster surfaces them directly too, not modal-only) -
// plus the generic actions.

import { buildScaffoldTemplate } from "./scaffold.js";

export const mount = buildScaffoldTemplate([
  { id: "mountstate", x: 20, y: 330, w: 300 },
  { id: "domestate", x: 350, y: 330, w: 260 },
  { id: "weatherstate", extra: ["coming"], x: 20, y: 460, w: 300 },
  { id: "timerules", x: 350, y: 460, w: 300 },
  { id: "weatherrules", x: 680, y: 460, w: 260 },
]);
