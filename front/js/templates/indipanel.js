// templates/indipanel.js — bespoke, DOCKED screen for the "indipanel"
// module: a raw INDI property browser (src/modules/indipanel/
// indipanel.cpp), whose level1 groups are typically one per connected INDI
// device - each with potentially dozens of its own properties. The
// generic renderModuleView() fallback turns every level1 group into its
// own floating panel, which for this module alone means a pile of windows
// to drag/resize/position rather than a handful (Gilles, 2026-10-25: "le
// nombre d'info qu'il y a dedans est énorme, nos options actuelles le
// rendent difficile à utiliser"). This template instead reuses the exact
// same level1/level2 grouping (render.js's buildLayout()/fillLevel1Panel())
// but shows one group at a time in a single docked block, switched with
// the quicknav bar acting as a plain tab strip rather than per-panel show/
// hide toggles (Gilles: "on garderait le quicknav pour passer d'un level1 à
// l'autre").
//
// DOCKED, read by app.js's renderContentNow(): opts this module out of the
// floating #panel-layer entirely (no Panels.make() calls at all here) in
// favor of #docked-view, the same container Home/Controller already use -
// see app.js's own doc comment on the two containers.

import { buildLayout, fillLevel1Panel, updateQuickNav, getQuickNavButton } from "../render.js";
import { el } from "../dom.js";
import { t } from "../i18n.js";

export const DOCKED = true;

export function mount(client, moduleName) {
  const root = el("div", { class: "indipanel-view" });
  const body = el("section", { class: "panel" });
  root.appendChild(body);

  let disposers = [];
  function track(node) { disposers.push(node._dispose); return node; }
  function dispose() { while (disposers.length) disposers.pop()(); }

  // Survives across rebuilds (every module dump/profile event calls
  // build() again) so switching devices/groups doesn't get reset back to
  // the first one on the next unrelated property update.
  let activeId = null;

  function build(mod) {
    dispose();
    body.innerHTML = "";

    if (!mod) {
      body.appendChild(el("div", { class: "muted", text: t("common.moduleNotFound") }));
      updateQuickNav([]);
      return;
    }

    // Anything already reachable from the Parameters box (render.js's
    // buildParamsPanel(): every hasprofile property, plus
    // serveractions/devicesactions specifically even though neither is
    // hasprofile) is excluded from the main view too - each lives under
    // its own level1 group here with nothing else in it (indipanel's own
    // "parms"/serveractions/devicesactions), so dropping them also drops
    // that now-redundant quicknav tab entirely, without this template
    // having to hardcode the group's own backend-defined label text.
    const properties = Object.fromEntries(Object.entries(mod.properties)
      .filter(([n, p]) => p.hasprofile !== true && n !== "serveractions" && n !== "devicesactions"));
    const { noLevel1, tabs } = buildLayout(properties);
    const groups = [];
    if (noLevel1.length) groups.push({ id: "general", label: t("common.general"), direct: noLevel1, groups: [] });
    for (const tab of tabs) groups.push({ id: "lvl-" + tab.name, label: tab.name || "(ungrouped)", direct: tab.direct, groups: tab.groups });

    if (!groups.length) {
      updateQuickNav([]);
      return;
    }
    if (!groups.some((g) => g.id === activeId)) activeId = groups[0].id;

    for (const g of groups) {
      if (g.id !== activeId) continue;
      fillLevel1Panel(body, client, moduleName, track, g);
    }

    function setActive(id) {
      activeId = id;
      build(mod);
    }
    updateQuickNav(groups.map((g) => ({ id: g.id, label: g.label, onClick: () => setActive(g.id) })));
    for (const g of groups) {
      const btn = getQuickNavButton(g.id);
      if (btn) btn.classList.toggle("active", g.id === activeId);
    }
  }

  const unsub = client.subscribe(client.topics.module(moduleName), build);
  return { node: root, dispose: () => { unsub(); dispose(); updateQuickNav([]); } };
}
