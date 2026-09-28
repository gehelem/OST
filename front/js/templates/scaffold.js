// templates/scaffold.js — shared builder for a "prepared" module template:
// header + actions + a caller-given list of the module's own named panels
// (each via render.js's generic renderProperty()) + full image handling
// (background, quick stats, stats+histogram button — imagelayer.js) when
// the module has one + a catch-all for anything else + the quicknav list +
// the module subscribe/build/dispose lifecycle.
//
// Mirrors ostnoctua's own templates/scaffolds.js ("the default flat
// property list ... + the shared image background & stats panel") — Gilles,
// 2026-09-29: "Prepare les templates pour focus et sequencer, et ajoute
// leur ce bouton image". Each template file just declares which of its own
// properties get their own panel and where; guider.js stays its own fully
// bespoke file (real Chart.js graphs) since a scaffold has nothing to add
// there.
//
// Hasprofile-backed properties never get their own panel here (same rule
// guider.js's bespoke template already follows) — they're only reachable
// through the Parameters modal (app.js's gear button), so a profile-backed
// setting never shows twice. Note for whoever adds the sequencer template's
// own bespoke version later: this hides "sequence" itself (the exposure
// list) behind that modal too, since it's hasprofile:true like everything
// else here — fine for a first pass, but worth reconsidering once
// sequencer gets real bespoke treatment.

import { renderProperty, updateQuickNav } from "../render.js";
import { addImageBlock } from "../imagelayer.js";
import { el } from "../dom.js";
import * as Panels from "../panels.js";

/**
 * `panelSpecs`: array of `{ id, title?, x, y, w, extra? }` — `id` is both
 * the panel id and the property name passed to renderProperty(); `extra`,
 * if given, is a list of further property names appended into the SAME
 * panel body (see guider's own values+calibrationvalues pairing). A spec
 * whose `id` isn't actually one of the module's properties is silently
 * skipped (safe to list one "just in case" across similar modules).
 *
 * `imagePos`: optional `{x,y,w}` override for where the image/image-stats
 * panels default to (they'd otherwise land wherever a panelSpec also put
 * something, on a module with a lot of panels).
 *
 * Returns `mount(client, moduleName)`, ready to re-export as a template's
 * own `mount`.
 */
export function buildScaffoldTemplate(panelSpecs, imagePos) {
  const ip = { x: 660, y: 92, w: 280, ...imagePos };

  return function mount(client, moduleName) {
    Panels.setScope(moduleName);
    Panels.clear();

    let disposers = [];
    function track(node) { disposers.push(node._dispose); return node; }
    function dispose() { while (disposers.length) disposers.pop()(); }

    function build(mod) {
      dispose();
      Panels.clear();
      const has = (name) => name in mod.properties;
      const quickNavEntries = [];

      const header = Panels.make({ id: "header", title: mod.infos.modulelabel || mod.infos.label || moduleName, x: 20, y: 92, w: 320 });
      header.appendChild(el("div", { class: "muted small", text: mod.infos.description || "" }));
      // Profile load/save lives in the dedicated "Parameters" box (app.js's
      // gear button, buildParamsPanel in render.js) — not built here.

      if (has("actions")) {
        const b = Panels.make({ id: "actions", title: mod.properties.actions.label || "Commands", x: 20, y: 210, w: 300 });
        b.appendChild(track(renderProperty(client, moduleName, "actions")));
        quickNavEntries.push({ id: "actions", label: mod.properties.actions.label || "Actions" });
      }

      const known = new Set(["actions", "image"]);
      for (const spec of panelSpecs) {
        known.add(spec.id);
        for (const extra of spec.extra || []) known.add(extra);
        if (!has(spec.id)) continue;
        const b = Panels.make({ id: spec.id, title: spec.title || mod.properties[spec.id].label || spec.id, x: spec.x, y: spec.y, w: spec.w });
        b.appendChild(track(renderProperty(client, moduleName, spec.id)));
        for (const extra of spec.extra || []) {
          if (has(extra)) b.appendChild(track(renderProperty(client, moduleName, extra)));
        }
        quickNavEntries.push({ id: spec.id, label: spec.title || mod.properties[spec.id].label || spec.id });
      }

      addImageBlock(client, moduleName, mod, "image", "image", Panels, track, disposers, quickNavEntries, ip);

      // Anything this template doesn't know about explicitly (a future
      // property this module gains) still needs to be reachable — fold it
      // into one catch-all panel rather than silently dropping it.
      const hasprofileNames = Object.keys(mod.properties).filter((n) => mod.properties[n].hasprofile === true);
      for (const n of hasprofileNames) known.add(n);
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
  };
}
