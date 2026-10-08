// templates/index.js — registry of bespoke module screens, keyed by
// infos.template (see structure.en.md). A module whose template isn't
// listed here falls back to render.js's generic, level1-grouped floating
// panels — see app.js's dispatch and client-pattern.en.md's "Templates:
// per-module-type screens with a generic fallback".
//
// infos.template is not fully reliable as a lookup key in general (see
// FINDINGS.md: Sequencer's used to say "sequence") — this registry only
// needs an exact string match for the templates it actually lists, so
// that unreliability only matters if a *listed* template's value drifts,
// which would just fall back to generic (safe failure, not a crash).

import * as guider from "./guider.js";
import * as focus from "./focus.js";
import * as sequencer from "./sequencer.js";
import * as navigator_ from "./navigator.js";
import * as inspector from "./inspector.js";
import * as polar from "./polar.js";
import * as parkmanager from "./parkmanager.js";
import * as monitor from "./monitor.js";

export const templates = {
  guider,
  focus,
  sequencer,
  navigator: navigator_,
  inspector,
  polar,
  parkmanager,
  monitor,
};
