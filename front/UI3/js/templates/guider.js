// templates/guider.js — bespoke floating-panel arrangement for the
// "guider" module, adapted from ostnoctua's templates/guider.js (see that
// file's own header) onto core/'s client API and UI3's generic property
// renderer instead of hand-written graph code.
//
// Unlike ostnoctua's version, this does not vendor Chart.js or draw the
// guiding/drift graphs by hand: the guider module's `guiding`/`drift`
// properties already carry hasGraph+graphType, which render.js's generic
// renderProperty() already turns into a canvas via chart.js — see
// render.js's renderGridSection(). A "template" here is therefore purely
// about *arrangement*: which of the module's properties get their own
// panel, where, and at what size — not about reimplementing rendering that
// the generic path already does correctly (client-pattern.en.md's "a
// bespoke template only ever adds presentation, never a second source of
// truth for data the generic renderer already gets right").
//
// Real property names confirmed against src/modules/guider/guider.json /
// guider.cpp: actions, values, calibrationvalues, drift, guiding,
// calParams, guideParams, revCorrections, disCorrections, backlash — plus
// `image`, defined generically by IndiModule::defineMeAsImager() (see
// libs/indimodule.cpp) for any imaging module, guider included.

import { renderProperty, updateQuickNav } from "../render.js";
import { el } from "../dom.js";
import * as Panels from "../panels.js";

export function mount(client, moduleName) {
  Panels.setScope(moduleName);
  Panels.clear();

  let disposers = [];
  function track(node) { disposers.push(node._dispose); return node; }
  function dispose() { while (disposers.length) disposers.pop()(); }

  function build(mod) {
    dispose();
    Panels.clear();
    const has = (name) => name in mod.properties;
    // Curated quicknav list — Gilles, 2026-09-27: "actions/image/devices/
    // control et peut-etre d'autres plus tard, qu'on devra pouvoir adapter
    // selon le template". "devices"/"control" don't exist as their own
    // panels yet (guider.json has no such property) - using what's
    // actually built today (actions/image/guiding/drift); add devices/
    // control here once those panels are actually created.
    const quickNavEntries = [];

    const header = Panels.make({ id: "header", title: mod.infos.modulelabel || mod.infos.label || moduleName, x: 20, y: 92, w: 320 });
    header.appendChild(el("div", { class: "muted small", text: mod.infos.description || "" }));
    // Profile load/save lives in the dedicated "Parameters" box below
    // (buildParamsPanel, shared with the generic renderer) — not here.

    if (has("actions")) {
      const b = Panels.make({ id: "actions", title: mod.properties.actions.label || "Commands", x: 20, y: 210, w: 300 });
      b.appendChild(track(renderProperty(client, moduleName, "actions")));
      quickNavEntries.push({ id: "actions", label: mod.properties.actions.label || "Actions" });
    }
    if (has("values")) {
      const b = Panels.make({ id: "values", title: mod.properties.values.label || "Session values", x: 20, y: 470, w: 300 });
      b.appendChild(track(renderProperty(client, moduleName, "values")));
      if (has("calibrationvalues")) b.appendChild(track(renderProperty(client, moduleName, "calibrationvalues")));
    }
    if (has("image")) {
      const b = Panels.make({ id: "image", title: mod.properties.image.label || "Guide frame", x: 350, y: 92, w: 380 });
      b.appendChild(track(renderProperty(client, moduleName, "image")));
      quickNavEntries.push({ id: "image", label: mod.properties.image.label || "Image" });
    }
    if (has("guiding")) {
      const b = Panels.make({ id: "guiding", title: mod.properties.guiding.label || "Guiding", x: 350, y: 300, w: 620 });
      b.appendChild(track(renderProperty(client, moduleName, "guiding")));
      quickNavEntries.push({ id: "guiding", label: mod.properties.guiding.label || "Guiding" });
    }
    if (has("drift")) {
      const b = Panels.make({ id: "drift", title: mod.properties.drift.label || "Drift", x: 990, y: 92, w: 340 });
      b.appendChild(track(renderProperty(client, moduleName, "drift")));
      quickNavEntries.push({ id: "drift", label: mod.properties.drift.label || "Drift" });
    }
    // The profile-centric "Parameters" box (buildParamsPanel, render.js) is
    // not built here — it opens on demand from the gear button (app.js),
    // same as the generic renderer. Its properties (hasprofile:true, from
    // the module's own metadata, not a hardcoded list) are still excluded
    // below so they don't also land in the "Other" catch-all.

    // Anything this template doesn't know about explicitly (a future
    // property this module gains) still needs to be reachable — fold it
    // into one catch-all panel rather than silently dropping it.
    const hasprofileNames = Object.keys(mod.properties).filter((n) => mod.properties[n].hasprofile === true);
    const known = new Set(["actions", "values", "calibrationvalues", "image", "guiding", "drift", ...hasprofileNames]);
    const rest = Object.keys(mod.properties).filter((n) => !known.has(n));
    if (rest.length) {
      const b = Panels.make({ id: "other", title: "Other", x: 20, y: 620, w: 300 });
      for (const name of rest) b.appendChild(track(renderProperty(client, moduleName, name)));
      quickNavEntries.push({ id: "other", label: "Other" });
    }
    updateQuickNav(quickNavEntries);
  }

  const unsub = client.subscribe(client.topics.module(moduleName), (mod) => {
    if (!mod) {
      dispose();
      Panels.clear();
      updateQuickNav([]);
      const b = Panels.make({ id: "missing", title: moduleName, x: 20, y: 92, w: 320 });
      b.appendChild(el("div", { class: "muted", text: "Module not found (unloaded?)" }));
      return;
    }
    build(mod);
  });

  return { dispose: () => { unsub(); dispose(); Panels.clear(); updateQuickNav([]); } };
}
