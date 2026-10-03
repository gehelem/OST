// templates/scaffold.js — shared builder for a "prepared" module template:
// actions + a caller-given list of the module's own named panels
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
// Hasprofile-backed properties get no panel of their own UNLESS a template
// explicitly lists them in panelSpecs (only then reachable through the
// Parameters modal too, as any hasprofile property is — see
// templates/sequencer.js's own "sequence" entry, a real editable grid
// that's also hasprofile:true, for exactly that case). Anything hasprofile
// a template does NOT list stays modal-only, so a profile-backed setting
// never shows twice by default.

import { renderProperty, updateQuickNav, moduleLifecycle, renderActionsPanel } from "../render.js";
import { addImageBlock } from "../imagelayer.js";
import * as Panels from "../panels.js";
import { t } from "../i18n.js";

/**
 * `panelSpecs`: array of `{ id, title?, x, y, w, extra?, icon?, iconOnly? }`
 * — `id` is both the panel id and the property name passed to
 * renderProperty(); `extra`, if given, is a list of further property names
 * appended into the SAME panel body (see guider's own
 * values+calibrationvalues pairing); `icon`/`iconOnly`, if given, are
 * passed straight through to the quicknav entry (views.js's own
 * renderQuickNav(), same fields renderActionsPanel()'s "actions" button
 * already uses — e.g. sequencer.js's own "sequence" entry, Gilles,
 * 2026-10-03: "bouton 'sequence' => icone seulement = 'list'"). A spec
 * whose `id` isn't actually one of the module's properties is silently
 * skipped (safe to list one "just in case" across similar modules).
 *
 * `imagePos`: optional `{x,y,w}` override for where the image/image-stats
 * panels default to (they'd otherwise land wherever a panelSpec also put
 * something, on a module with a lot of panels).
 *
 * `opts.imageQuickPanel: false` skips the small "Image" quick-stats panel/
 * button (imagelayer.js's own addImageBlock() `quickPanel` flag) while
 * keeping the background image and the deeper Stats panel — Gilles,
 * 2026-10-03, on sequencer specifically: "enleve aussi le bouton 'image'
 * et la fenetre associee" (same request already applied to focus.js/
 * guider.js's own hand-written templates). Other scaffolded templates
 * (inspector/polar/parkmanager) keep the default.
 *
 * `opts.otherPanel: false` drops the "Other" catch-all panel/button below
 * entirely (same thread, right after: "enleve aussi le bouton 'other' et
 * sa fenetre") — unlike guider.js's own removal, this can genuinely drop
 * reachability for whatever `panelSpecs` doesn't list, so only opt out of
 * it for a module actually checked to have nothing worth keeping there.
 *
 * Returns `mount(client, moduleName)`, ready to re-export as a template's
 * own `mount`.
 */
export function buildScaffoldTemplate(panelSpecs, imagePos, opts) {
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

      // Profile load/save lives in the dedicated "Parameters" box (app.js's
      // gear button, buildParamsPanel in render.js) — not built here. No
      // "header" panel either any more (render.js's own renderModuleView
      // doc comment: Gilles, 2026-10-03, "la supprimer completement").

      renderActionsPanel(client, moduleName, mod, track, quickNavEntries);

      const known = new Set(["actions", "image"]);
      for (const spec of panelSpecs) {
        known.add(spec.id);
        for (const extra of spec.extra || []) known.add(extra);
        if (!has(spec.id)) continue;
        // spec.title, when a template gives one (e.g. focus.js's "Focus
        // curve"), is an i18n.js *key*, not literal text - panelSpecs
        // arrays are built once at module-import time (see focus.js/
        // sequencer.js), long before a language is even selected, so
        // resolving it here (inside build(), which re-runs on every module
        // event, including the one a language change triggers) is what
        // keeps it reactive instead of frozen at whatever language was
        // current on import.
        const title = (spec.title && t(spec.title)) || mod.properties[spec.id].label || spec.id;
        const b = Panels.make({ id: spec.id, title, x: spec.x, y: spec.y, w: spec.w });
        b.appendChild(track(renderProperty(client, moduleName, spec.id)));
        for (const extra of spec.extra || []) {
          if (has(extra)) b.appendChild(track(renderProperty(client, moduleName, extra)));
        }
        quickNavEntries.push({ id: spec.id, label: title, icon: spec.icon, iconOnly: spec.iconOnly });
      }

      addImageBlock(client, moduleName, mod, "image", "image", Panels, track, disposers, quickNavEntries, ip, { quickPanel: !opts || opts.imageQuickPanel !== false });

      // Anything this template doesn't know about explicitly (a future
      // property this module gains) still needs to be reachable — fold it
      // into one catch-all panel rather than silently dropping it.
      const hasprofileNames = Object.keys(mod.properties).filter((n) => mod.properties[n].hasprofile === true);
      for (const n of hasprofileNames) known.add(n);
      const rest = Object.keys(mod.properties).filter((n) => !known.has(n));
      if (rest.length && (!opts || opts.otherPanel !== false)) {
        const b = Panels.make({ id: "other", title: t("common.other"), x: 20, y: 620, w: 300 });
        for (const name of rest) b.appendChild(track(renderProperty(client, moduleName, name)));
        quickNavEntries.push({ id: "other", label: t("common.other") });
      }
      updateQuickNav(quickNavEntries);
    }

    return moduleLifecycle(client, moduleName, build, dispose);
  };
}
