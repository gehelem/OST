// js/wizard.js — "Setup wizard" (issue #238): a guided, linear sequence of
// steps to get a first-time setup running, or to build a new named "jeu
// d'utilisation" (configuration) more easily than doing each piece
// separately from the Controller settings screen.
//
// Gilles, 2026-10-13: "je pense que tu as toutes les infos necessaires deja
// sur le front dans les settings generales, il faut les centraliser dans cet
// assistant" - confirmed true for 4 of the 5 steps below (INDI drivers,
// optics, equipment/device assignment, save/load configuration all reuse
// the exact same controls/primitives views.js's renderController() and
// render.js's generic grid-row form already use - no new backend wiring).
// Only the "modules under a common prefix" step (4) is genuinely new UI; a
// single INDI driver listing/display path is deliberately deferred (1B,
// "serveur INDI non integre" - Gilles: "on affinera ce cas dans un second
// temps").
//
// Shape: a modal dialog (same .dialog-overlay/.dialog shell render.js's own
// openGridDialog() uses), one step visible at a time, driven by STEPS
// below. Linear forward progress only (Gilles: "impossible de passer au
// suivant tant qu'on a pas un minimum d'info sur l'etape en cours") - each
// step calls the shared `setValid(bool)` callback it's given whenever its
// own "is there enough here yet" answer might have changed, and the
// wizard shell enables/disables "Next" accordingly. "Back" (the button, or
// clicking any earlier step number in the stepper) is always free - every
// action a step takes (starting a driver, creating a grid row, loading a
// module, saving a configuration) already applies immediately through the
// same client.* calls the Controller screen itself uses, so there is
// nothing to roll back by going backward, only live state to re-check.

import { el, icon } from "./dom.js";
import { buildIndiPanel } from "./views.js";
import { buildElementsForm } from "./render.js";
import { t } from "./i18n.js";

const STEPS = [
  { titleKey: "wizard.step.indi", render: renderIndiStep },
  { titleKey: "wizard.step.optics", render: renderOpticsStep },
  { titleKey: "wizard.step.equipment", render: renderEquipmentStep },
  { titleKey: "wizard.step.modules", render: renderModulesStep },
  { titleKey: "wizard.step.save", render: renderSaveStep },
];

export function openSetupWizard(client) {
  let stepIndex = 0;
  let stepDispose = null;
  let valid = false;

  const stepperEl = el("div", { class: "wizard-stepper" });
  const bodyEl = el("div", { class: "wizard-body" });
  const backBtn = el("button", { class: "btn btn-sm", onclick: goBack }, t("wizard.back"));
  const nextBtn = el("button", { class: "btn btn-primary btn-sm", onclick: goNext });
  // Fullscreen toggle (Gilles, 2026-10-13: step 4's own checklist - module
  // + "set slaves" + equipment/optics pickers per row - "ça commence a
  // faire du monde sur ce panneau") - a plain CSS class swap, no layout
  // logic of its own; every step's content already reflows into
  // .wizard-body's available height either way.
  const fullscreenBtn = el("button", { class: "btn-icon", title: t("wizard.fullscreen"), onclick: toggleFullscreen }, [icon("open_in_full")]);
  const closeBtn = el("button", { class: "btn-icon", title: t("common.close"), onclick: close }, [icon("close")]);

  const dialog = el("div", { class: "dialog wizard-dialog" }, [
    el("div", { class: "wizard-header" }, [el("h3", { text: t("wizard.title") }), fullscreenBtn, closeBtn]),
    stepperEl,
    bodyEl,
    el("div", { class: "dialog-actions" }, [backBtn, nextBtn]),
  ]);
  function toggleFullscreen() {
    const full = dialog.classList.toggle("fullscreen");
    fullscreenBtn.querySelector(".material-icons").textContent = full ? "close_fullscreen" : "open_in_full";
    fullscreenBtn.title = full ? t("wizard.exitFullscreen") : t("wizard.fullscreen");
  }
  const overlay = el("div", { class: "dialog-overlay" }, [dialog]);
  // Clicking the backdrop does NOT close this one (unlike openGridDialog's
  // single-form dialogs) - every step's own actions already apply for
  // real as they happen (see module doc comment above), so there is no
  // unsaved draft to warn about, but losing the stepper's place by a stray
  // click while e.g. filling the module-prefix field would still be
  // annoying. Only the explicit close button (or Escape) does.
  document.addEventListener("keydown", onKeydown);
  document.body.appendChild(overlay);

  renderStepper();
  renderStep();

  function renderStepper() {
    stepperEl.innerHTML = "";
    STEPS.forEach((s, i) => {
      const btn = el("button", {
        class: "wizard-step-btn" + (i === stepIndex ? " active" : "") + (i < stepIndex ? " done" : ""),
        disabled: i > stepIndex,
        onclick: () => {
          if (i > stepIndex) return;
          stepIndex = i;
          renderStepper();
          renderStep();
        },
      }, [
        el("span", { class: "wizard-step-num", text: i < stepIndex ? "✓" : String(i + 1) }),
        el("span", { class: "wizard-step-label", text: t(s.titleKey) }),
      ]);
      stepperEl.appendChild(btn);
    });
  }

  function setValid(v) {
    valid = !!v;
    nextBtn.disabled = !valid;
  }

  function renderStep() {
    if (stepDispose) { stepDispose(); stepDispose = null; }
    bodyEl.innerHTML = "";
    backBtn.disabled = stepIndex === 0;
    nextBtn.textContent = stepIndex === STEPS.length - 1 ? t("wizard.finish") : t("wizard.next");
    setValid(false);
    const result = STEPS[stepIndex].render(client, bodyEl, setValid);
    stepDispose = result && result.dispose;
  }

  function goNext() {
    if (!valid) return;
    if (stepIndex < STEPS.length - 1) { stepIndex++; renderStepper(); renderStep(); }
    else close();
  }
  function goBack() {
    if (stepIndex === 0) return;
    stepIndex--;
    renderStepper();
    renderStep();
  }
  function onKeydown(ev) { if (ev.key === "Escape") close(); }
  function close() {
    if (stepDispose) stepDispose();
    document.removeEventListener("keydown", onKeydown);
    overlay.remove();
  }
}

// ---- Step 1: INDI server & drivers ---------------------------------------
// Reuses views.js's own buildIndiPanel() wholesale (see its own doc
// comment) - this step adds nothing on top except the validity gate.
function renderIndiStep(client, container, setValid) {
  const scope = client.scope();
  container.appendChild(el("p", { class: "muted small", text: t("wizard.indi.intro") }));
  container.appendChild(buildIndiPanel(client, scope, checkValid));
  // buildIndiPanel's own subscriptions fire immediately (client.subscribe()
  // convention), so checkValid has already run at least once by the time
  // this returns - no separate initial call needed here.
  function checkValid() {
    const indiserver = client.get(client.topics.controllerData("indiserver"));
    if (indiserver !== "Y") {
      // 1B ("serveur INDI non integre") is deliberately deferred (Gilles:
      // "on affinera ce cas dans un second temps") - never block on it,
      // there is nothing yet for this step to require here.
      setValid(true);
      return;
    }
    const active = client.get(client.topics.controllerData("indiactivedrivers")) || [];
    setValid(active.length > 0);
  }
  return { dispose: scope.dispose };
}

// ---- Step 2: optics -------------------------------------------------------
// GlobalDatastore's own "optics" grid (name/focal/diameter/reducer/type,
// see src/globaldatastore.json) - no cross-reference to anything else
// needed, so the generic row-form (buildElementsForm(), render.js - same
// one openGridDialog() uses for every module's own "Add row" dialog) is
// reused directly instead of a bespoke form.
function renderOpticsStep(client, container, setValid) {
  return renderSimpleGridStep(client, container, setValid, {
    propName: "optics",
    introKey: "wizard.optics.intro",
    addLabelKey: "wizard.optics.add",
    rowLabel: (row) => `${row.name || "—"} — f=${row.focal ?? "?"}mm Ø${row.diameter ?? "?"}mm`,
  });
}

// ---- Step 3: equipment / device assignment --------------------------------
// GlobalDatastore's own "equipments" grid - each device-role column
// (camera/mount/focuser/filter/guidecamera/guidest4/gps/dome/weather)
// already carries its own listOfValues pointing at the right
// DRIVER_INTERFACE-* global LOV (populated from whatever INDI devices are
// actually detected, see giveMeADevice()/defineMeAsXXX(), libs/indimodule.cpp),
// and buildElementsForm() already renders a LOV-constrained string element
// as a dropdown (render.js) - so this step gets the "pick from detected
// devices" behaviour for free, same as step 2, no new logic needed for it.
function renderEquipmentStep(client, container, setValid) {
  return renderSimpleGridStep(client, container, setValid, {
    propName: "equipments",
    introKey: "wizard.equipment.intro",
    addLabelKey: "wizard.equipment.add",
    rowLabel: (row) => row.name || "—",
  });
}

// Shared by steps 2 and 3 - both are "list GlobalDatastore.<propName>'s
// rows, offer an 'Add' form built from the property's own live element
// defs, done once there is at least one row" with nothing else different
// between them.
function renderSimpleGridStep(client, container, setValid, { propName, introKey, addLabelKey, rowLabel }) {
  const scope = client.scope();
  container.appendChild(el("p", { class: "muted small", text: t(introKey) }));
  const list = el("ul");
  container.appendChild(list);
  const formSlot = el("div", { class: "hidden" });
  const addBtn = el("button", { class: "btn btn-sm", text: t(addLabelKey), onclick: openForm });
  container.appendChild(addBtn);
  container.appendChild(formSlot);

  function openForm() {
    const prop = client.get(client.topics.property("GlobalDatastore", propName));
    if (!prop) return;
    formSlot.innerHTML = "";
    formSlot.classList.remove("hidden");
    const form = buildElementsForm(client, "GlobalDatastore", prop.elements, prop.gridheaders || [], {});
    formSlot.appendChild(form.node);
    formSlot.appendChild(el("div", { class: "form-row" }, [
      el("button", { class: "btn btn-sm", text: t("common.cancel"), onclick: () => formSlot.classList.add("hidden") }),
      el("button", { class: "btn btn-primary btn-sm", text: t("wizard.add"), onclick: () => {
        client.gridCreate("GlobalDatastore", propName, form.getValues());
        formSlot.classList.add("hidden");
      } }),
    ]));
  }

  function refresh() {
    const prop = client.get(client.topics.property("GlobalDatastore", propName));
    const rows = (prop && prop.rows) || [];
    list.innerHTML = "";
    for (const row of rows) list.appendChild(el("li", { class: "small", text: rowLabel(row) }));
    setValid(rows.length > 0);
  }
  scope.subscribe(client.topics.property("GlobalDatastore", propName), refresh);
  return { dispose: scope.dispose };
}

// Mirrors each module's own "slaves" property definition (<lib>.json,
// slaves.*.listOfValues: "loadedModules-X" or "loadedFamilies-X") - Gilles,
// 2026-10-13: "si on dit S1-Sequence il faut lui attribuer les slaves
// S1-Focus et S1-Guider, et aussi forcer les coches de selection de ces
// modules". No generic protocol message exposes a library's own property
// schema before it is actually loaded (controllerdata.libraries' metadata
// is just getModuleInfo() - label/families/etc, never an "e" element list),
// so this has to be kept in sync by hand with each module's own slaves
// property if it ever changes. A "loadedFamilies-X" reference (focus,
// guiding) is resolved here to one specific library that provides that
// family (guider for "guiding" - blindpec provides it too, but isn't the
// default any wizard-driven batch-load should assume).
const SLAVE_SCHEMA = {
  planner: [{ elt: "sequencemodule", lib: "sequencer" }, { elt: "navigatormodule", lib: "navigator" }],
  sequencer: [{ elt: "focusmodule", lib: "focus" }, { elt: "guidermodule", lib: "guider" }],
  navigator: [{ elt: "plannermodule", lib: "planner" }],
  parkmanager: [{ elt: "plannermodule", lib: "planner" }],
  monitor: [
    { elt: "guider", lib: "guider" }, { elt: "sequencer", lib: "sequencer" }, { elt: "focus", lib: "focus" },
    { elt: "navigator", lib: "navigator" }, { elt: "planner", lib: "planner" }, { elt: "parkmanager", lib: "parkmanager" },
    { elt: "polar", lib: "polar" }, { elt: "inspector", lib: "inspector" },
  ],
};

// Same reasoning/limitation as SLAVE_SCHEMA above (no protocol message
// exposes a library's device roles before it's loaded) - mirrors each
// module's own giveMeADevice(name, ...) calls (<lib>.cpp constructor),
// mapped to the matching GlobalDatastore "equipments" column. Most roles
// share the exact same name on both sides; the guiding-family modules
// (guider, blindpec) name their own roles from the guiding module's own
// point of view ("camera" = the GUIDE camera, "guider" = the ST4/guide
// port) rather than matching equipments' column names directly - see
// src/globaldatastore.json's own "equipments" columns (guidecamera,
// guidest4) vs. guider.cpp/blindpec.cpp's own giveMeADevice() calls.
// allsky's "meteo" role (GENERAL_INTERFACE) has no equipments column at
// all and is deliberately left unmapped.
const DEVICE_ROLES = {
  focus: { camera: "camera", focuser: "focuser", filter: "filter" },
  guider: { camera: "guidecamera", guider: "guidest4" },
  blindpec: { camera: "guidecamera", guider: "guidest4" },
  sequencer: { camera: "camera", filter: "filter" },
  navigator: { camera: "camera", mount: "mount", gps: "gps" },
  inspector: { camera: "camera", focuser: "focuser" },
  polar: { camera: "camera", mount: "mount", gps: "gps" },
  allsky: { camera: "camera", gps: "gps" },
  parkmanager: { dome: "dome", mount: "mount", weather: "weather" },
};

// Which module types get an "optic" property (giveMeAnOptic(), reached via
// defineMeAsImager() - every real imaging-chain module except planner/
// monitor). Picking a name here only needs one client.setValue() call -
// IndiModule::onExternalEvent() already copies focal/diameter from the
// matching GlobalDatastore "optics" row onto the module's own optic.fl/
// optic.diam itself (libs/indimodule.cpp) the moment "optic.optic" is set,
// same mechanism a user picking it by hand through the module's own
// property panel would trigger - nothing new to replicate here.
const HAS_OPTIC = new Set(["focus", "guider", "blindpec", "sequencer", "navigator", "inspector", "polar", "allsky"]);

// ---- Step 4: modules under a common prefix --------------------------------
// The one genuinely new piece (Gilles's point 4: "S1 : S1-Navigator
// S1-Planner etc etc") - today's "Load a module" section (renderController())
// loads one instance at a time, by hand; this adds a batch layer over the
// exact same primitive (client.loadModule()), nothing new server-side.
function renderModulesStep(client, container, setValid) {
  const scope = client.scope();
  container.appendChild(el("p", { class: "muted small", text: t("wizard.modules.intro") }));

  const prefixInput = el("input", { type: "text", class: "field-input", placeholder: t("wizard.modules.prefixPlaceholder") });
  // Global equipment/optics pickers (Gilles, 2026-10-13: "a droite de la
  // zone prefix ... quand on les change ca met a jour les lignes de tous
  // les modules selectionnes") - a bulk-apply shortcut over the per-row
  // <select>s below, not a separate state of their own: picking one here
  // just sets every currently CHECKED module row's own select to this
  // value (skipping rows that don't have one - a module with no device
  // roles has no equipment <select> to set). Options are the exact same
  // GlobalDatastore rows, kept in sync by the same refresh functions.
  const globalEquipmentSelect = el("select", { class: "field-input" });
  const globalOpticsSelect = el("select", { class: "field-input" });
  globalEquipmentSelect.addEventListener("change", () => applyGlobalSelect(equipmentSelects, globalEquipmentSelect.value));
  globalOpticsSelect.addEventListener("change", () => applyGlobalSelect(opticsSelects, globalOpticsSelect.value));
  function applyGlobalSelect(selectsMap, value) {
    if (!value) return;
    for (const [lib, sel] of selectsMap) {
      const cb = checkboxes.get(lib);
      if (cb && cb.checked && [...sel.options].some((o) => o.value === value)) sel.value = value;
    }
  }
  container.appendChild(el("div", { class: "form-row" }, [
    prefixInput,
    el("label", { class: "wizard-module-picker" }, [el("span", { text: t("wizard.modules.equipment") }), globalEquipmentSelect]),
    el("label", { class: "wizard-module-picker" }, [el("span", { text: t("wizard.modules.optics") }), globalOpticsSelect]),
  ]));

  // A real table (Gilles, 2026-10-13: "organise plutot le panneau 4 en
  // tableau avec des entetes module/equipment/optics/slaves") - one row
  // per library, the Equipment/Optics/Slaves cells left empty for a
  // module that doesn't declare the matching device roles / optic /
  // slaves property (see DEVICE_ROLES/HAS_OPTIC/SLAVE_SCHEMA) rather than
  // skipping the cell, so every row stays aligned under its column.
  const checklistTable = el("table", { class: "wizard-module-table" }, [
    el("thead", null, [el("tr", null, [
      el("th", { text: t("wizard.modules.colModule") }),
      el("th", { text: t("wizard.modules.equipment") }),
      el("th", { text: t("wizard.modules.optics") }),
      el("th", { text: t("wizard.modules.colSlaves") }),
    ])]),
  ]);
  const checklistEl = el("tbody");
  checklistTable.appendChild(checklistEl);
  container.appendChild(checklistTable);

  const createBtn = el("button", { class: "btn btn-primary btn-sm", text: t("wizard.modules.create"), onclick: createChecked });
  container.appendChild(createBtn);

  const checkboxes = new Map(); // stripped lib name -> <input type=checkbox> ("load this one")
  const setSlavesCheckboxes = new Map(); // stripped lib name -> <input type=checkbox> ("wire its slaves too")
  const equipmentSelects = new Map(); // stripped lib name -> <select> (GlobalDatastore.equipments row name, or "")
  const opticsSelects = new Map(); // stripped lib name -> <select> (GlobalDatastore.optics row name, or "")

  function capitalize(s) {
    return s ? s.charAt(0).toUpperCase() + s.slice(1).toLowerCase() : s;
  }

  function refreshCreateEnabled() {
    const hasPrefix = !!prefixInput.value.trim();
    const hasChecked = [...checkboxes.values()].some((cb) => cb.checked);
    createBtn.disabled = !(hasPrefix && hasChecked);
  }

  function checkValid() {
    const prefix = prefixInput.value.trim();
    const names = client.get(client.topics.modules()) || [];
    setValid(!!prefix && names.some((n) => n.startsWith(prefix + "-")));
  }

  prefixInput.addEventListener("input", () => { refreshCreateEnabled(); checkValid(); });

  // Equipment/optics <select>s (Gilles, 2026-10-13: "il faut aussi assigner
  // les differents devices selon les modules, selon la liste 'equipment' -
  // idem avec optics ! => des listes de choix en face de chaque module") -
  // both read from the exact same GlobalDatastore rows steps 2/3 of this
  // wizard just created (or whatever already existed before it was
  // opened). A blank first option always means "don't touch this".
  function rowNames(propName) {
    const prop = client.get(client.topics.property("GlobalDatastore", propName));
    return ((prop && prop.rows) || []).map((r) => r.name).filter(Boolean);
  }
  function refreshSelectOptions(select, names) {
    const prev = select.value;
    select.innerHTML = "";
    select.appendChild(el("option", { value: "" }, "—"));
    for (const n of names) select.appendChild(el("option", { value: n, text: n }));
    if (names.includes(prev)) select.value = prev;
  }
  function refreshAllEquipmentSelects() {
    const names = rowNames("equipments");
    refreshSelectOptions(globalEquipmentSelect, names);
    for (const sel of equipmentSelects.values()) refreshSelectOptions(sel, names);
  }
  function refreshAllOpticsSelects() {
    const names = rowNames("optics");
    refreshSelectOptions(globalOpticsSelect, names);
    for (const sel of opticsSelects.values()) refreshSelectOptions(sel, names);
  }
  scope.subscribe(client.topics.property("GlobalDatastore", "equipments"), refreshAllEquipmentSelects);
  scope.subscribe(client.topics.property("GlobalDatastore", "optics"), refreshAllOpticsSelects);

  // Checking "set slaves" force-checks every module it depends on (Gilles:
  // "forcer les coches de selection de ces modules") - never the reverse,
  // unchecking a dependency doesn't touch "set slaves". Unchecking the
  // module itself turns "set slaves" back off too - wiring slaves onto an
  // instance that won't exist makes no sense.
  function onMainToggle(stripped, cb) {
    if (!cb.checked) {
      const setCb = setSlavesCheckboxes.get(stripped);
      if (setCb) setCb.checked = false;
    }
    refreshCreateEnabled();
  }
  function onSetSlavesToggle(stripped) {
    if (!setSlavesCheckboxes.get(stripped).checked) return;
    for (const { lib } of SLAVE_SCHEMA[stripped] || []) {
      const depCb = checkboxes.get(lib);
      if (depCb && !depCb.checked) depCb.checked = true;
    }
    refreshCreateEnabled();
  }

  function createChecked() {
    const prefix = prefixInput.value.trim();
    if (!prefix) return;
    for (const [lib, cb] of checkboxes) {
      if (cb.checked) client.loadModule(lib, `${prefix}-${capitalize(lib)}`);
    }
    // Slave wiring has to wait for loadModule() to actually produce each
    // instance (async - neither the module being configured nor the
    // slaves it points at are guaranteed to exist yet the instant
    // loadModule() returns). One subscription per "set slaves" module,
    // applying client.setValue() as soon as both sides of each link exist,
    // disposing itself once every link for that module is done - no
    // polling/setTimeout, same event-driven shape as the rest of this app.
    for (const [lib, setCb] of setSlavesCheckboxes) {
      if (!setCb.checked) continue;
      const ownName = `${prefix}-${capitalize(lib)}`;
      const pending = new Map((SLAVE_SCHEMA[lib] || []).map((s) => [s.elt, `${prefix}-${capitalize(s.lib)}`]));
      const unsub = scope.subscribe(client.topics.modules(), (names) => {
        if (!(names || []).includes(ownName)) return;
        for (const [elt, slaveName] of [...pending]) {
          if ((names || []).includes(slaveName)) {
            client.setValue(ownName, "slaves", elt, slaveName);
            pending.delete(elt);
          }
        }
        if (pending.size === 0) unsub();
      });
    }
    // Equipment/optics - same "wait for the instance to actually exist"
    // reasoning as slave wiring above, just a single module to wait for
    // instead of a module-plus-its-slaves set.
    for (const [lib, cb] of checkboxes) {
      if (!cb.checked) continue;
      const ownName = `${prefix}-${capitalize(lib)}`;
      const equipment = equipmentSelects.get(lib) ? equipmentSelects.get(lib).value : "";
      const optics = opticsSelects.get(lib) ? opticsSelects.get(lib).value : "";
      if (!equipment && !optics) continue;
      const unsub = scope.subscribe(client.topics.modules(), (names) => {
        if (!(names || []).includes(ownName)) return;
        if (equipment) {
          const row = (client.get(client.topics.property("GlobalDatastore", "equipments")).rows || [])
            .find((r) => r.name === equipment);
          const roles = DEVICE_ROLES[lib] || {};
          if (row) for (const [ownRole, equipCol] of Object.entries(roles)) {
            if (row[equipCol]) client.setValue(ownName, "devices", ownRole, row[equipCol]);
          }
        }
        if (optics) client.setValue(ownName, "optic", "optic", optics);
        unsub();
      });
    }
  }

  // Gilles, 2026-10-13: "coche par defaut sur le[s] focus/guider/
  // navigator/sequencer/planner" - the 5 "core" modules of a typical
  // setup, pre-checked the first time this step builds its table (and
  // only then - a later rebuild, e.g. libraries reported again, must
  // respect whatever the user already chose, never silently re-check
  // something they unchecked).
  const DEFAULT_CHECKED = new Set(["focus", "guider", "navigator", "sequencer", "planner"]);
  let firstBuild = true;

  scope.subscribe(client.topics.controllerData("libraries"), (libs) => {
    const prevChecked = new Set([...checkboxes].filter(([, cb]) => cb.checked).map(([k]) => k));
    const prevSetSlaves = new Set([...setSlavesCheckboxes].filter(([, cb]) => cb.checked).map(([k]) => k));
    checklistEl.innerHTML = "";
    checkboxes.clear();
    setSlavesCheckboxes.clear();
    equipmentSelects.clear();
    opticsSelects.clear();
    for (const key of Object.keys(libs || {})) {
      const stripped = key.startsWith("libost") ? key.slice(6) : key;
      const cb = el("input", { type: "checkbox" });
      if (firstBuild ? DEFAULT_CHECKED.has(stripped) : prevChecked.has(stripped)) cb.checked = true;
      cb.addEventListener("change", () => onMainToggle(stripped, cb));
      checkboxes.set(stripped, cb);
      const moduleCell = el("td", null, [el("label", { class: "wizard-module-check" }, [cb, el("span", { text: capitalize(stripped) })])]);
      // Equipment/optics/slaves cells (Gilles: "des listes de choix en
      // face de chaque module" + une 4e colonne "slaves") - only modules
      // that actually declare the matching device roles / optic / slaves
      // property get real content (DEVICE_ROLES/HAS_OPTIC/SLAVE_SCHEMA);
      // every other module still gets the empty cell, so columns stay
      // aligned. Select options filled in by refreshAllEquipmentSelects()/
      // refreshAllOpticsSelects() right after this loop, not here - a
      // freshly (re)built <select> starts with just the blank placeholder.
      const equipmentCell = el("td");
      if (DEVICE_ROLES[stripped]) {
        const sel = el("select", { class: "field-input" });
        equipmentSelects.set(stripped, sel);
        equipmentCell.appendChild(sel);
      }
      const opticsCell = el("td");
      if (HAS_OPTIC.has(stripped)) {
        const sel = el("select", { class: "field-input" });
        opticsSelects.set(stripped, sel);
        opticsCell.appendChild(sel);
      }
      const slavesCell = el("td");
      if (SLAVE_SCHEMA[stripped]) {
        const setCb = el("input", { type: "checkbox" });
        if (firstBuild ? DEFAULT_CHECKED.has(stripped) : prevSetSlaves.has(stripped)) setCb.checked = true;
        setCb.addEventListener("change", () => onSetSlavesToggle(stripped));
        setSlavesCheckboxes.set(stripped, setCb);
        slavesCell.appendChild(el("label", { class: "wizard-module-setslaves" }, [setCb, el("span", { text: t("wizard.modules.setSlaves") })]));
      }
      checklistEl.appendChild(el("tr", null, [moduleCell, equipmentCell, opticsCell, slavesCell]));
    }
    // A "set slaves" default-check still needs its own dependencies
    // force-checked, same as a user clicking it would get via
    // onSetSlavesToggle() - e.g. defaulting "sequencer"'s slaves on must
    // still force "focus"/"guider" on even though they're already
    // defaulted on anyway in this particular set.
    if (firstBuild) for (const stripped of setSlavesCheckboxes.keys()) onSetSlavesToggle(stripped);
    firstBuild = false;
    refreshAllEquipmentSelects();
    refreshAllOpticsSelects();
    refreshCreateEnabled();
  });
  scope.subscribe(client.topics.modules(), checkValid);

  return { dispose: scope.dispose };
}

// ---- Step 5: save as a named configuration ---------------------------------
// Same primitive renderController()'s own "Configurations" section already
// uses (client.saveConfiguration()/loadConfiguration()) - last step, no
// "Next" to gate (setValid(true) unconditionally - Finish just closes the
// wizard, nothing forces a save here since the point of this screen is
// also to let Gilles review what's already in place).
function renderSaveStep(client, container, setValid) {
  const scope = client.scope();
  container.appendChild(el("p", { class: "muted small", text: t("wizard.save.intro") }));

  const list = el("ul");
  container.appendChild(list);
  const nameInput = el("input", { type: "text", class: "field-input", placeholder: t("views.configNamePlaceholder") });
  container.appendChild(el("div", { class: "form-row" }, [
    nameInput,
    el("button", { class: "btn btn-primary btn-sm", text: t("views.saveCurrentAs"), onclick: () => { if (nameInput.value) client.saveConfiguration(nameInput.value); } }),
  ]));

  function refresh() {
    const confs = client.get(client.topics.controllerData("availableconfs")) || {};
    const currentconf = client.get(client.topics.controllerData("currentconf"));
    list.innerHTML = "";
    for (const [name, modlist] of Object.entries(confs)) {
      const li = el("li");
      li.appendChild(el("span", { class: "spacer small", text: `${name}${currentconf === name ? t("views.current") : ""} — ${(modlist || []).length} ${t("views.moduleCountSuffix")}` }));
      li.appendChild(el("button", { class: "btn btn-sm", text: t("views.load"), onclick: () => client.loadConfiguration(name) }));
      list.appendChild(li);
    }
  }
  scope.subscribe(client.topics.controllerData("availableconfs"), refresh);
  scope.subscribe(client.topics.controllerData("currentconf"), refresh);

  setValid(true);
  return { dispose: scope.dispose };
}
