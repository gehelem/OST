// views.js — the non-module screens: sidebar, controller/global settings,
// file browser, log panel, login dialog. Same functional coverage as
// ostcore/UI2/js/views.js (verified against a real server), re-skinned for
// UI3's design. Screens that stay on-screen and need live data
// (controller/files/logs) subscribe themselves through client.scope() and
// return {node, dispose}; the sidebar and login dialog don't hold any
// subscription open, so they just return a node.

import { el, icon, iconButton, moduleIcon, statusDot, updateStatusDot } from "./dom.js";
import { togglePanel, isPanelVisible, make as makePanel } from "./panels.js";
import { t } from "./i18n.js";
// Reused as-is on the Home dashboard's bigger focus/guider cards (Gilles,
// 2026-10-05: "mets les graphes correspondants" then "Focus: values/curve /
// Guide: rms/guidegraph/drift") - the module's own panel-building
// functions, not reinvented here, see renderHome() below.
import { buildCompactFocusResult, buildFocusCurvePanel } from "./templates/focus.js";
import { buildCompactRms, buildGuidingChartPanel, buildDriftChartPanel } from "./templates/guider.js";
// Sequencer's own "last shot" mini thumbnail on its Home card (Gilles,
// 2026-10-06: "ajoute une 'toute petite' vignette de la derniere image
// prise") - same urljpeg/urlthumbnail resolution showBackgroundImage()
// itself uses, not reinvented here.
import { jpegSrc, openImageViewer, refreshImageViewer, buildImageGridPanel } from "./imagelayer.js";
// Inspector's own 4 control maps (hfr/shape/collim/corners), same list its
// own "maps" panel already uses (Gilles, 2026-10-06: "sur inspector ajoute
// les 4 images de controle en vignettes toutes petites").
import { MAP_ITEMS as INSPECTOR_MAP_ITEMS } from "./templates/inspector.js";
// monitor's own "Session graph" (Gilles, 2026-10-06: "la vignette home du
// monitor avec le graphe 'session graph' sans les labels de serie").
import { buildMonitorGraphPanel } from "./templates/monitor.js";

// Shared by renderLogs() (the full-page Logs system view) and
// renderLogPanel() (the always-on floating one, below) - index == the
// wire's own `l` field (server-messages.en.md's "l" message: 0 Debug, 1
// Info, 2 Warning, 3 Error, 4 Critical).
const LOG_LEVELS = ["debug", "info", "warning", "error", "critical"];

// Horizontal module bar (ostnoctua's .on-modbar/.on-mod, see panels.css) —
// only the actually-loaded OST modules. Controller/Files/Logs live in the
// settings dropdown instead (see renderSystemViews) — Gilles, 2026-09-25:
// "deplace les boutons controller/files/logs dans les settings".
//
// Returns {node, dispose}: node is a DocumentFragment, not a wrapping <div>
// — #modbar itself is the flex row (panels.css), and .mod-btn is
// `display:flex` for its own icon-over-label stacking — that makes each
// button a block-level box, so wrapping them in an unstyled <div> stacked
// them vertically instead of letting #modbar's flex row lay them out
// (caught live, 2026-09-25: "la barre de menu est a nouveau verticale"). A
// fragment leaves no such wrapper behind: the buttons become #modbar's
// direct children. `dispose` must be called before the caller rebuilds the
// bar (app.js's renderModbarNow()) — see the per-module subscription below.
export function renderModbar(client, activeKey, onSelect) {
  const bar = document.createDocumentFragment();
  const names = client.get(client.topics.modules());
  const disposers = [];
  // "Home" dashboard - Gilles, 2026-10-05: "une home en premier dans la
  // liste des modules [...] inspire toi de osterix qui a une telle page".
  // Not a real backend module (no entry in `names`, no "signals" to
  // subscribe to), but it belongs IN the modbar itself, first position,
  // same look as a module button - unlike Controller/Files/Logs which are
  // this client's own chrome and live in the settings dropdown instead
  // (see renderSystemViews above). "__home" follows that same sentinel
  // naming convention for app.js's activeView.
  bar.appendChild(el("button", {
    class: "mod-btn" + (activeKey === "__home" ? " active" : ""), title: t("views.home"),
    onclick: () => onSelect("__home"),
  }, [icon("home"), el("span", { class: "mod-btn-label", text: t("views.home") })]));
  for (const name of names) {
    const mod = client.get(client.topics.module(name));
    const label = (mod && (mod.infos.modulelabel || mod.infos.label)) || name;
    // Global-state dot (Gilles, 2026-09-27): the same "signals.state" light
    // element the module's own header panel already shows (see render.js's
    // renderProperty()), so a module in trouble is visible without opening
    // it. Not every module declares "signals" (a handful, e.g.
    // GlobalDatastore, don't call Basemodule::setStateEvent() at all) — for
    // those the dot just stays absent (.hidden) rather than showing a fake
    // standby color. Subscribes directly to the "signals" *property*, not
    // topics.module(name) (Gilles, 2026-09-27: "cette bubulle devrait
    // s'abonner a nomDuModule.signals(.state)") — topics.module() only
    // fires on module add/remove/profile events (see datastore.js's own
    // doc comment on it), never on an element value change (ea/ee), which
    // is exactly how "state" itself updates; only topics.property() covers
    // that.
    const dot = statusDot(0);
    dot.classList.add("mod-btn-dot", "hidden");
    // client.subscribe() fires immediately with the current value (same
    // convention as renderProperty()'s own subscription), so no separate
    // initial call is needed here.
    disposers.push(client.subscribe(client.topics.property(name, "signals"), (prop) => {
      const state = prop && prop.elements && prop.elements.state;
      dot.classList.toggle("hidden", !state);
      if (state) updateStatusDot(dot, state.value);
    }));
    const btn = el("button", { class: "mod-btn" + (activeKey === name ? " active" : ""), title: label, onclick: () => onSelect(name) }, [
      dot,
      icon(moduleIcon(mod)),
      el("span", { class: "mod-btn-label", text: label }),
    ]);
    // Unsaved profile changes (Gilles, 2026-10-15) - same indicator and
    // same underlying flag (module.profile.changed) as the params gear
    // button's own "has-changes" dot (app.js's updateParamsToggle()), just
    // shown per module here instead of only for the currently active one.
    disposers.push(client.subscribe(client.topics.module(name), (m) => {
      btn.classList.toggle("has-changes", !!(m && m.profile && m.profile.changed));
    }));
    bar.appendChild(btn);
  }
  return { node: bar, dispose: () => disposers.forEach((d) => d()) };
}

/**
 * The per-module "quick nav" bar (#quicknav, bottom-center): one button per
 * {id, label, icon?} entry, showing/hiding that panel and reflecting
 * whether it's currently visible (Gilles, 2026-09-27: "l'action du bouton
 * doit masquer/demasquer la fenetre et l'etat du bouton doit montrer si la
 * fenetre est visible ou pas") — Panels.togglePanel()/isPanelVisible() are
 * the *only* shared piece (render.js/templates own the entry list; see
 * render.js's setQuickNavRoot()). `data-panel-id` lets app.js's single
 * "panelvisibility" listener find and update this same button if the
 * panel's visibility is ever changed some other way than this button's
 * own click - decoupled on purpose, even with no such caller today.
 * Same DocumentFragment pattern as renderModbar, for the same reason (no
 * unstyled wrapper element). `icon` (a material-icons name) is optional —
 * most entries are text-only; a template can ask for an icon instead when
 * the button represents an action rather than "jump to this panel" (e.g.
 * navigator's "Settings" button, Gilles, 2026-09-30: "l'icone avec 3
 * sliders").
 */
export function renderQuickNav(entries) {
  const bar = document.createDocumentFragment();
  for (const { id, label, icon: iconName, iconOnly, onClick, disabled } of entries) {
    // `iconOnly` (render.js's own renderActionsPanel(): the actions
    // window's button, Gilles, 2026-10-03: "bouton = juste une icone
    // 'dashboard_customize'") drops the text span entirely - `title`
    // still carries the label as a tooltip either way. Every other entry
    // keeps its previous icon+label (icon given) or plain-text (no icon)
    // shape, unchanged.
    const content = iconOnly ? [icon(iconName)] : iconName ? [icon(iconName), el("span", { text: label })] : label;
    // `onClick` (Gilles, 2026-10-06: "ajoute un bouton ... pas de fenetre
    // associee" - navigator's "center on mount" button) opts a button out
    // of the panel-toggle behaviour entirely: it fires its own callback
    // instead of togglePanel(id), and never gets the "active" class (there
    // is no panel visibility for it to reflect). `disabled` sets the
    // button's initial enabled state at creation time (the caller is
    // responsible for updating it later itself, e.g. from a property
    // subscription - getQuickNavButton() below finds this same element by
    // `data-panel-id` for that).
    const btn = el("button", {
      class: "quicknav-btn" + (iconOnly ? " quicknav-btn-icon-only" : "") + (!onClick && isPanelVisible(id) ? " active" : ""),
      title: label, "data-panel-id": id, disabled: !!disabled,
      onclick: onClick ? onClick : () => btn.classList.toggle("active", togglePanel(id)),
    }, content);
    bar.appendChild(btn);
  }
  return bar;
}

// The three system/utility screens, rendered as a small vertical list inside
// the settings dropdown (#conn-panel) instead of the module bar — they
// aren't OST modules, they're this client's own chrome, so grouping them
// with the other app-level settings reads better than mixing them into the
// live module list.
const SYSTEM_VIEWS = [
  { key: "__controller", get label() { return t("views.controller"); }, icon: "tune" },
  { key: "__files", get label() { return t("views.files"); }, icon: "folder" },
  { key: "__logs", get label() { return t("views.logs"); }, icon: "list" },
];
export function renderSystemViews(activeKey, onSelect) {
  const nav = el("div", { class: "settings-nav" });
  for (const item of SYSTEM_VIEWS) {
    const btn = el("button", { class: "settings-nav-item" + (activeKey === item.key ? " active" : ""), onclick: () => onSelect(item.key) }, [
      icon(item.icon, { small: true }),
      el("span", { text: item.label }),
    ]);
    nav.appendChild(btn);
  }
  return nav;
}

// ---- home dashboard -------------------------------------------------------
// Gilles, 2026-10-05: "une home en premier dans la liste des modules [...]
// doit montrer une fenetre par module, si je clique dessus ca ouvre le
// module en question [...] inspire toi de osterix qui a une telle page
// [...] les apercus ne permettent pas de faire de mise a jour directement
// mais donnent les infos utiles de chaque module". Modeled on OstErix's
// own src/app/pages/home/home.component.ts (read directly): one card per
// loaded module, status dot + icon + label in the header, a handful of
// read-only key figures in the body, click anywhere on the card to open
// that module. Everything but focus/guider stays a plain metric row or,
// lacking even that, a "click to open" card — same as Rooster's own
// default case for any template without a custom widget.
// sequencer and navigator used to share a generic "plain metric row"
// mechanism here (HOME_METRIC_PROPERTY/homeCardMetrics) until each needed
// something its single-property shape couldn't give it - sequencer got its
// own ring gauge (Gilles, 2026-10-06: "fais moi un truc joli qui ne prend
// pas trop de place"), navigator a second property (target) alongside
// mountposition. Removed once nothing used it any more rather than left
// as a never-taken dead branch - see buildHomeCard's own
// template === "sequencer"/"navigator" branches below for both.

// Gilles, 2026-10-05: "fais plutot des presentations en mode horizontal pour
// guider et focus / Focus: values/curve / Guide: rms/guidegraph/drift" —
// each entry lays out that many columns side by side (.home-card-row-layout,
// components.css) instead of the plain metric row every other card gets.
// Every builder is one of the module's own bespoke panel-builders, reused
// as-is (self-subscribed, returns a node with its own `_dispose`) rather
// than reinvented here — same "no duplicated chart/figure logic" approach
// the previous (stacked) version already established. guider's own entry
// went 3 columns -> 2 (Gilles, 2026-10-06: "tu lui fais occuper 3 zones,
// passe a deux" then "limiter ... a 2 paves ... 2/3 pour guidegraphe et
// 1/3 pour drift sur un pave") -> back to 3 (Gilles, 2026-10-06, next:
// "fait prendre plus de place au graphe de drift du guidage, en faisant
// occupe 3 zones a la vignette") once splitting one zone 2:1 left drift
// too small to read - each chart gets its own full column again, drift's
// own column just wider than its original equal share (components.css's
// .home-card-column.gd-chart-square).
const HOME_HORIZONTAL_BUILDERS = {
  focus: [buildCompactFocusResult, (client, name) => buildFocusCurvePanel(client, name, { compact: true })],
  guider: [
    buildCompactRms,
    (client, name) => buildGuidingChartPanel(client, name, { compact: true }),
    (client, name) => buildDriftChartPanel(client, name, { compact: true }),
  ],
};

// Same touches[0]-or-clientX/Y split as panels.js's own pointXY() - not
// imported from there (it isn't exported, and this is two lines) but the
// same reasoning: a MouseEvent and a TouchEvent carry the pointer
// differently, callers below shouldn't have to care which one they got.
function pointXY(ev) {
  const touch = ev.touches && ev.touches[0];
  return touch ? { x: touch.clientX, y: touch.clientY } : { x: ev.clientX, y: ev.clientY };
}

// Manual mouse+touch drag for card reordering - see buildHomeCard's own
// comment on why this isn't native HTML5 drag-and-drop. Tracks the pointer
// with document-level move/up listeners (panels.js's startDrag() pattern),
// highlights whichever other card it is currently over (drop-before/
// drop-after, components.css), and hands the actual reorder off to
// onReorder(draggedName, targetName, insertAfter) once released over one -
// the drop decision reads back whatever the live pointer tracking already
// settled on, same as panels.js's own up() reads the panel's own
// already-updated style.left/top rather than re-deriving anything from the
// release event itself.
function startCardDrag(card, name, ev, onReorder) {
  ev.preventDefault();
  const start = pointXY(ev);
  card.classList.add("dragging");
  // pointer-events:none while dragging so elementFromPoint below can "see
  // through" the dragged card itself to whatever's underneath it once it's
  // translated on top of a neighbour - without this, hovering a card onto
  // another one would keep hitting itself, never the card underneath.
  card.style.pointerEvents = "none";
  let dropTarget = null;
  let dropAfter = false;
  const clearMark = () => { if (dropTarget) dropTarget.classList.remove("drop-before", "drop-after"); };
  const move = (e) => {
    const p = pointXY(e);
    // Gilles, 2026-10-06: "on a deja des fenetres drag/drop qui marchent
    // tres bien, t'es encore en train de me reinventer la poudre pour
    // rien" - panels.js's own startDrag() visibly moves the panel WITH
    // the cursor in real time; the previous version of this only dimmed
    // the card and left it sitting in place, with nothing at all tracking
    // the pointer - looked completely dead even though the reorder logic
    // underneath was already working. Same real-time-follow feedback here.
    card.style.transform = `translate(${p.x - start.x}px, ${p.y - start.y}px)`;
    const hit = document.elementFromPoint(p.x, p.y);
    const target = hit && hit.closest(".home-card");
    if (target !== dropTarget) clearMark();
    if (target && target !== card) {
      const rect = target.getBoundingClientRect();
      dropAfter = p.x - rect.left > rect.width / 2;
      target.classList.toggle("drop-before", !dropAfter);
      target.classList.toggle("drop-after", dropAfter);
      dropTarget = target;
    } else {
      dropTarget = null;
    }
    if (e.touches) e.preventDefault(); // stop the page itself from scrolling under the drag
  };
  const up = () => {
    document.removeEventListener("mousemove", move);
    document.removeEventListener("mouseup", up);
    document.removeEventListener("touchmove", move);
    document.removeEventListener("touchend", up);
    document.removeEventListener("touchcancel", up);
    card.classList.remove("dragging");
    card.style.pointerEvents = "";
    card.style.transform = "";
    clearMark();
    if (dropTarget) onReorder(name, dropTarget.dataset.moduleName, dropAfter);
  };
  document.addEventListener("mousemove", move);
  document.addEventListener("mouseup", up);
  document.addEventListener("touchmove", move, { passive: false });
  document.addEventListener("touchend", up);
  document.addEventListener("touchcancel", up);
}

function buildHomeCard(client, name, onSelect, onReorder) {
  const mod = client.get(client.topics.module(name));
  const template = mod && mod.infos.template;
  const label = (mod && (mod.infos.modulelabel || mod.infos.label)) || name;
  const columnBuilders = HOME_HORIZONTAL_BUILDERS[template];

  const dot = statusDot(0);
  dot.classList.add("hidden");
  // Drag handle (Gilles, 2026-10-06: "ajoute la possibilite de deplacer les
  // vignettes + position remanente"). Deliberately NOT native HTML5
  // drag-and-drop (draggable=true/dragstart/dragover/drop) - this exact
  // codebase already hit that wall once, panels.js's own startDrag(), on
  // this project's own floating panels: "sur smartphone je ne peux pas
  // deplacer les fenetres" - a touch's mousedown/mousemove (and, same
  // story, native HTML5 DnD) aren't reliably delivered by mobile/touch
  // browsers at all. Caught again here the same way: native DnD "worked"
  // under Playwright's own synthetic/CDP-driven mouse, but Gilles still
  // couldn't drag for real. Same fix as panels.js: a manual
  // mouse+touch pointer handler (startCardDrag below), not the browser's
  // own drag semantics. stopPropagation on a plain click so tapping the
  // handle without dragging doesn't also fire the card's onclick.
  const handle = icon("drag_indicator", { small: true });
  handle.classList.add("home-card-handle");
  handle.title = t("views.homeDragHint");
  handle.addEventListener("click", (ev) => ev.stopPropagation());
  const header = el("div", { class: "home-card-header" }, [handle, dot, icon(moduleIcon(mod)), el("span", { class: "home-card-title", text: label })]);
  const card = el("div", {
    class: "home-card" + (columnBuilders ? ` home-card-big home-card-cols-${columnBuilders.length}` : ""), title: t("views.homeGeneric"), onclick: () => onSelect(name),
  }, [header]);
  card.dataset.moduleName = name;
  handle.addEventListener("mousedown", (ev) => startCardDrag(card, name, ev, onReorder));
  handle.addEventListener("touchstart", (ev) => startCardDrag(card, name, ev, onReorder), { passive: false });

  const disposers = [];
  disposers.push(client.subscribe(client.topics.property(name, "signals"), (prop) => {
    const state = prop && prop.elements && prop.elements.state;
    dot.classList.toggle("hidden", !state);
    if (state) updateStatusDot(dot, state.value);
  }));

  if (columnBuilders) {
    // Side-by-side columns (results/RMS boxes + chart(s)), each the
    // module's own panel-building function.
    const row = el("div", { class: "home-card-columns" });
    for (const build of columnBuilders) {
      const node = build(client, name);
      node.classList.add("home-card-column");
      row.appendChild(node);
      disposers.push(node._dispose);
    }
    card.appendChild(row);
    return { node: card, dispose: () => disposers.forEach((d) => d()) };
  }

  const body = el("div", { class: "home-card-body muted small", text: t("views.homeGeneric") });
  card.appendChild(body);

  // Gilles, 2026-10-06: "affiche sur la vignette sequencer l'avancement,
  // fais moi un truc joli qui ne prend pas trop de place" - the same
  // compact ring gauge the system card's own RAM/disk figures use
  // (ringGauge, same conic-gradient "prg" spinner recipe), instead of the
  // generic plain-text row every other template without a bespoke widget
  // gets below.
  if (template === "sequencer") {
    body.classList.remove("muted", "small");
    body.innerHTML = "";
    // The ring (rebuilt on every "progress" update, via ringSlot below)
    // and the last-shot thumbnail (updated independently, its own "image"
    // subscription - a different property entirely) sit side by side in
    // the same row - Gilles, 2026-10-06: "ajoute une 'toute petite'
    // vignette de la derniere image prise (apres/a droite du spinner de
    // 'global progress'), pas plus haut)". A plain img.src swap rather
    // than rebuilding it each time keeps it independent of the ring's own
    // update cadence.
    const row = el("div", { class: "home-ring-row" });
    const ringSlot = el("div", { class: "muted small", text: t("views.homeGeneric") });
    const thumb = el("img", { class: "home-seq-thumb hidden", alt: "" });
    let lastThumbSrc = null;
    thumb.addEventListener("click", (ev) => { ev.stopPropagation(); if (lastThumbSrc) openImageViewer(lastThumbSrc); });
    row.appendChild(ringSlot);
    row.appendChild(thumb);
    body.appendChild(row);
    const extra = el("div");
    body.appendChild(extra);

    disposers.push(client.subscribe(client.topics.property(name, "progress"), (prop) => {
      const e = prop && prop.elements;
      const g = e && e.global && e.global.value;
      ringSlot.innerHTML = "";
      extra.innerHTML = "";
      if (!g) { ringSlot.classList.add("muted", "small"); ringSlot.textContent = t("views.homeGeneric"); return; }
      ringSlot.classList.remove("muted", "small");
      ringSlot.appendChild(ringGauge(g.value, g.dynlabel || t("views.homeProgress")));
      // Gilles, 2026-10-06, right after the ring-only version shipped:
      // "bah ? tu n'as mis que le global: ajoute le 'remaining time' et un
      // progress bar pour l'exposition en cours" - theoreticalremaining is
      // a plain string field (server-formatted duration, not a "prg"), the
      // current exposure gets its own compact bar (prg-bar-*, same
      // component render.js's own non-ring prg type already uses, just
      // narrower here to fit the card).
      if (e.theoreticalremaining && e.theoreticalremaining.value) {
        extra.appendChild(el("div", { class: "home-card-row" }, [
          el("span", { class: "muted small", text: e.theoreticalremaining.label || t("views.homeRemaining") }),
          el("span", { class: "mono", text: e.theoreticalremaining.value }),
        ]));
      }
      const exposure = e.exposure && e.exposure.value;
      if (exposure) {
        const pct = Math.max(0, Math.min(100, exposure.value || 0));
        const bar = el("div", { class: "home-exposure-bar" }, [
          el("div", { class: "muted small", text: e.exposure.label || t("views.homeExposure") }),
          el("div", { class: "prg-bar-outer home-bar-full" }, [el("div", { class: "prg-bar-inner", style: `width:${pct}%` })]),
        ]);
        if (exposure.dynlabel) bar.appendChild(el("div", { class: "muted small", text: exposure.dynlabel }));
        extra.appendChild(bar);
      }
    }));

    disposers.push(client.subscribe(client.topics.property(name, "image"), (prop) => {
      const src = jpegSrc(prop && prop.elements && prop.elements.image);
      thumb.classList.toggle("hidden", !src);
      if (src) { thumb.src = src; lastThumbSrc = src; refreshImageViewer(src); }
    }));

    return { node: card, dispose: () => disposers.forEach((d) => d()) };
  }

  // Gilles, 2026-10-06: "sur inspector ajoute les 4 images de controle en
  // vignettes toutes petites (2x2 qui doivent tenir dans la zone
  // actuelle, sans stretch)" - the module's own "maps" panel
  // (buildImageGridPanel, same 4 properties/MAP_ITEMS), just scaled down
  // (.home-img-grid, components.css) to fit this plain card's normal
  // size instead of a wide floating panel - no cols-N/home-card-big
  // treatment here, unlike focus/guider, since the request was to fit
  // the CURRENT card area, not grow it.
  if (template === "inspector") {
    body.classList.remove("muted", "small");
    body.innerHTML = "";
    const grid = buildImageGridPanel(client, name, INSPECTOR_MAP_ITEMS);
    grid.classList.add("home-img-grid");
    // buildImageGridPanel's own thumbnail click (openImageViewer) has
    // nothing to stopPropagation against in its usual home (a floating
    // panel with no ancestor click handler) - here it bubbles straight
    // into the card's own onclick (navigate to the module), opening the
    // viewer AND navigating away at once. Caught live.
    grid.addEventListener("click", (ev) => ev.stopPropagation());
    body.appendChild(grid);
    disposers.push(grid._dispose);
    return { node: card, dispose: () => disposers.forEach((d) => d()) };
  }

  // Gilles, 2026-10-06: "la vignette home du monitor avec le graphe
  // 'session graph' sans les labels de serie" - the module's own chart
  // (buildMonitorGraphPanel), compact (no legend), filling this plain
  // card's normal body instead of growing it into a multi-column one.
  if (template === "monitor") {
    body.classList.remove("muted", "small");
    body.innerHTML = "";
    const chart = buildMonitorGraphPanel(client, name, { compact: true });
    chart.classList.add("home-embedded-chart");
    body.appendChild(chart);
    disposers.push(chart._dispose);
    return { node: card, dispose: () => disposers.forEach((d) => d()) };
  }

  // Gilles, 2026-10-06: "sur la vignette navigator tu peux ajouter le nom
  // de la cible" - a second property (target.targetname) alongside
  // mountposition's own RA/DEC, so each needs its own subscription/slot
  // that doesn't get wiped out by the other updating independently (same
  // split-slots approach as sequencer's ring vs. thumbnail above).
  if (template === "navigator") {
    body.classList.remove("muted", "small");
    body.innerHTML = "";
    const targetRow = el("div");
    const posRows = el("div");
    body.appendChild(targetRow);
    body.appendChild(posRows);

    disposers.push(client.subscribe(client.topics.property(name, "target"), (prop) => {
      const e = prop && prop.elements;
      const tn = e && e.targetname && e.targetname.value;
      targetRow.innerHTML = "";
      if (tn) {
        targetRow.appendChild(el("div", { class: "home-card-row" }, [
          el("span", { class: "muted small", text: e.targetname.label || t("views.homeTarget") }),
          el("span", { class: "mono", text: tn }),
        ]));
      }
    }));
    disposers.push(client.subscribe(client.topics.property(name, "mountposition"), (prop) => {
      const e = prop && prop.elements;
      posRows.innerHTML = "";
      if (!e) return;
      const num = (v, d) => (typeof v === "number" ? v.toFixed(d) : "—");
      posRows.appendChild(el("div", { class: "home-card-row" }, [el("span", { class: "muted small", text: (e.RA && e.RA.label) || "RA" }), el("span", { class: "mono", text: num(e.RA && e.RA.value, 3) })]));
      posRows.appendChild(el("div", { class: "home-card-row" }, [el("span", { class: "muted small", text: (e.DEC && e.DEC.label) || "DEC" }), el("span", { class: "mono", text: num(e.DEC && e.DEC.value, 3) })]));
    }));

    return { node: card, dispose: () => disposers.forEach((d) => d()) };
  }

  // Gilles, 2026-10-06: "sur la vignette planner tu peux afficher la
  // valeur courante de la target (name/RA/DEC) sans aller chercher sur la
  // grille" - "planning" is planner's own grid property (hasGrid:true,
  // one row per planned object: object/ra/dec/profile/progress - confirmed
  // live against the real server, not guessed), no separate "current
  // target" property of its own. The active row is whichever one's own
  // "progress" dynlabel names an in-progress step (planner.cpp's real
  // setDynLabel() calls: "Checking visibility"/"Slewing"/"Shooting" while
  // working a line, "Queued" before it and "Finished"/"Cancelled"/
  // "Failed: ..."/"Skipped: ..." once resolved) - picks the first match,
  // same order the grid itself processes lines in.
  const PLANNER_ACTIVE_STATES = new Set(["Checking visibility", "Slewing", "Shooting"]);
  if (template === "planner") {
    disposers.push(client.subscribe(client.topics.property(name, "planning"), (prop) => {
      const rows = (prop && prop.rows) || [];
      const active = rows.find((r) => r.progress && PLANNER_ACTIVE_STATES.has(r.progress.dynlabel));
      body.classList.toggle("muted", !active);
      body.classList.toggle("small", !active);
      body.innerHTML = "";
      if (!active) { body.textContent = t("views.homeGeneric"); return; }
      const num = (v, d) => (typeof v === "number" ? v.toFixed(d) : "—");
      const e = prop.elements;
      // Gilles, 2026-10-06, after two earlier attempts ("ca prend trop de
      // place" on a 3-row version, then "c'est moche" on a ring+name row
      // above a RA/DEC row): "l'affichage doit etre separe en deux
      // parties: a gauche nom objet puis RA/DEC en dessous, a droite
      // spinner sur deux lignes" - two side-by-side columns instead of
      // stacked rows: left column (name, then RA/DEC folded into one row
      // via rms-compact-sub/-item, same compact pair guider's RMS boxes
      // use), right column just the ring, vertically centered against the
      // left column's own two-line height.
      const left = el("div", { class: "home-planner-left" });
      left.appendChild(el("div", { class: "mono home-ring-object", text: active.object }));
      const posRow = el("div", { class: "rms-compact-sub" });
      posRow.appendChild(el("div", { class: "rms-compact-item" }, [el("span", { class: "muted small", text: (e.ra && e.ra.label) || "RA" }), el("span", { class: "mono", text: num(active.ra, 3) })]));
      posRow.appendChild(el("div", { class: "rms-compact-item" }, [el("span", { class: "muted small", text: (e.dec && e.dec.label) || "DEC" }), el("span", { class: "mono", text: num(active.dec, 3) })]));
      left.appendChild(posRow);
      const row = el("div", { class: "home-planner-row" });
      row.appendChild(left);
      row.appendChild(ringGauge(active.progress.value, active.progress.dynlabel || t("views.homeProgress")));
      body.appendChild(row);
    }));
    return { node: card, dispose: () => disposers.forEach((d) => d()) };
  }

  return { node: card, dispose: () => disposers.forEach((d) => d()) };
}

// Gilles, 2026-10-06: "ajoute une vignette 'en dur' (qui ne correspond pas
// a un module) avec les donnees systeme ('server load') et fais un truc
// joli avec des spinners" - reuses controllerData()'s own systemwatcher
// data (same source as renderController()'s "Server load" section above,
// just a compact dashboard-card version of it instead of the full table).
// A real determinate ring per percentage figure (RAM, each disk) - same
// conic-gradient recipe as render.js's own "prg" spinner type
// (components.css's .prg-ring*, reused here directly rather than
// reinvented), just bigger (.home-ring). CPU load averages don't have a
// clean 0-100 denominator without knowing the core count (which
// systemwatcher doesn't report) - shown as plain numbers instead of a
// ring that would misrepresent them.
function ringGauge(pct, caption) {
  const p = Math.max(0, Math.min(100, pct || 0));
  return el("div", { class: "home-ring-item" }, [
    el("div", { class: "prg-ring-wrap home-ring" }, [
      el("div", { class: "prg-ring", style: `--pct:${p}` }),
      el("span", { class: "prg-ring-value", text: Math.round(p) + "%" }),
    ]),
    el("div", { class: "muted small home-ring-caption", text: caption }),
  ]);
}

const HOME_SYSTEM_ID = "__system";

// Gilles, 2026-10-06, right after this card shipped: "je ne peux pas
// drag/drop cette vignette !!" - it's "en dur" (not a real module) but
// still expected to move around like every other one. Same handle/
// startCardDrag wiring as buildHomeCard, with HOME_SYSTEM_ID standing in
// for a module name - onReorder/orderedModuleNames (below) don't care
// what the ids mean, any string works, so no special-casing needed there
// beyond seeding it into the id list renderHome() builds cards from.
function buildSystemCard(client, onReorder) {
  const handle = icon("drag_indicator", { small: true });
  handle.classList.add("home-card-handle");
  handle.title = t("views.homeDragHint");
  handle.addEventListener("click", (ev) => ev.stopPropagation());
  const body = el("div", { class: "home-system-body" });
  const card = el("div", { class: "home-card home-card-system" }, [
    el("div", { class: "home-card-header" }, [handle, icon("dns"), el("span", { class: "home-card-title", text: t("views.serverLoad") })]),
    body,
  ]);
  card.dataset.moduleName = HOME_SYSTEM_ID;
  handle.addEventListener("mousedown", (ev) => startCardDrag(card, HOME_SYSTEM_ID, ev, onReorder));
  handle.addEventListener("touchstart", (ev) => startCardDrag(card, HOME_SYSTEM_ID, ev, onReorder), { passive: false });
  const unsub = client.subscribe(client.topics.controllerData(), (cd) => {
    body.innerHTML = "";
    const sw = cd && cd.systemwatcher;
    if (!sw) { body.appendChild(el("div", { class: "muted small", text: t("views.homeGeneric") })); return; }
    const rings = el("div", { class: "home-ring-row" });
    if (sw.ram) rings.appendChild(ringGauge((sw.ram.used_mb / sw.ram.total_mb) * 100, t("views.ramLabel")));
    for (const d of (sw.disks || []).slice(0, 2)) rings.appendChild(ringGauge((d.used_gb / d.total_gb) * 100, d.mountpoint));
    if (rings.children.length) body.appendChild(rings);
    if (sw.cpu) body.appendChild(el("div", { class: "home-card-row" }, [el("span", { class: "muted small", text: t("views.cpuLabel") }), el("span", { class: "mono", text: `${sw.cpu.load_1m} / ${sw.cpu.load_5m} / ${sw.cpu.load_15m}` })]));
    for (const temp of sw.temperatures || []) body.appendChild(el("div", { class: "home-card-row" }, [el("span", { class: "muted small", text: temp.type }), el("span", { class: "mono", text: `${temp.temp_c.toFixed(1)} °C` })]));
  });
  return { node: card, dispose: unsub };
}

// Gilles, 2026-10-06: "ajoute la possibilite de deplacer les vignettes +
// position remanente" - same idea as OstErix's own home page (localStorage
// key "ost-home-card-order", source read directly), just this client's own
// key-naming convention (LOG_FILTER_KEY above). Modules never in the
// saved order (not yet dragged, or loaded since) keep the datastore's own
// relative order, appended after every module the saved order does cover.
const HOME_ORDER_KEY = "ostcore-ui3:home:order";
function loadHomeOrder() {
  try { return JSON.parse(localStorage.getItem(HOME_ORDER_KEY)) || []; } catch (e) { return []; }
}
function saveHomeOrder(order) {
  try { localStorage.setItem(HOME_ORDER_KEY, JSON.stringify(order)); } catch (e) { /* ignore */ }
}
function orderedModuleNames(names) {
  const saved = loadHomeOrder();
  const known = names.filter((n) => saved.includes(n)).sort((a, b) => saved.indexOf(a) - saved.indexOf(b));
  const rest = names.filter((n) => !saved.includes(n));
  return [...known, ...rest];
}

export function renderHome(client, onSelect) {
  // Gilles, 2026-10-06: "sur cette page home, il ne sert a rien d'afficher
  // 'home' avec son icone" - no redundant "Home" heading here; the
  // modbar's own "Home" button is already highlighted active, that's
  // enough to tell a visitor where they are.
  const root = el("div", { class: "home-view" });
  const grid = el("div", { class: "home-grid" });
  root.appendChild(grid);

  // Reorder on drop (startCardDrag's own callback, see buildHomeCard) -
  // reads the currently-displayed card list straight from the DOM (grid's
  // own children), so it's always exactly what's on screen right now
  // regardless of any async timing, rather than a separately-tracked array.
  function onReorder(draggedName, targetName, insertAfter) {
    if (draggedName === targetName) return;
    const order = [...grid.querySelectorAll(".home-card")].map((c) => c.dataset.moduleName).filter((n) => n !== draggedName);
    let idx = order.indexOf(targetName);
    if (insertAfter) idx += 1;
    order.splice(idx, 0, draggedName);
    saveHomeOrder(order);
    rebuild();
  }

  // The "en dur" system card (Gilles, 2026-10-06) takes part in the exact
  // same ordered id list as real module names - HOME_SYSTEM_ID is just
  // another entry orderedModuleNames() sorts/persists like any other
  // (Gilles, right after it first shipped fixed in place: "je ne peux pas
  // drag/drop cette vignette !!").
  let cardDisposers = [];
  function rebuild() {
    cardDisposers.forEach((d) => d());
    cardDisposers = [];
    grid.innerHTML = "";
    const ids = orderedModuleNames([HOME_SYSTEM_ID, ...client.get(client.topics.modules())]);
    for (const id of ids) {
      const { node, dispose } = id === HOME_SYSTEM_ID ? buildSystemCard(client, onReorder) : buildHomeCard(client, id, onSelect, onReorder);
      cardDisposers.push(dispose);
      grid.appendChild(node);
    }
  }
  rebuild();

  // Only the module SET (add/remove/profile swap, which can change
  // infos.template) needs a full rebuild - each card's own figures stay
  // live through its own metric-property subscription above, same
  // division of labour as renderModbar()'s per-module "signals" dot.
  const unsubModules = client.subscribe(client.topics.modules(), rebuild);

  return { node: root, dispose: () => { unsubModules(); cardDisposers.forEach((d) => d()); } };
}

export function renderLogin(client, onDone) {
  const overlay = el("div", { class: "dialog-overlay" });
  const user = el("input", { type: "text", class: "field-input", placeholder: t("views.userPlaceholder"), style: "width:100%" });
  const pw = el("input", { type: "password", class: "field-input", placeholder: t("views.passwordPlaceholder"), style: "width:100%" });
  // Gilles, 2026-10-03: "on ne peut pas fermer la fenetre de login si on
  // veut" - same close convention as render.js's own grid create/edit
  // dialog (backdrop click + an explicit button, no onclick-only way out).
  // Dismissing it without logging in is fine even for the auto-popup case
  // (grant-client "-1") - it just means staying denied, not a dead end.
  const close = () => overlay.remove();
  const submit = () => { client.login(user.value, pw.value); close(); if (onDone) onDone(); };
  pw.addEventListener("keydown", (ev) => { if (ev.key === "Enter") submit(); });
  const dialog = el("div", { class: "dialog" }, [
    el("h3", { text: t("views.loginRequired") }),
    el("div", { class: "muted small", text: t("views.loginRequiresAuth") }),
    el("div", { class: "form-row" }, [el("label", { text: t("views.user") }), user]),
    el("div", { class: "form-row" }, [el("label", { text: t("views.password") }), pw]),
    el("div", { class: "dialog-actions" }, [
      el("button", { class: "btn", onclick: close }, t("common.close")),
      el("button", { class: "btn btn-primary", onclick: submit }, t("views.login")),
    ]),
  ]);
  overlay.appendChild(dialog);
  overlay.addEventListener("click", (ev) => { if (ev.target === overlay) close(); });
  document.body.appendChild(overlay);
}

function fmtBytes(mb) {
  if (mb === undefined) return "—";
  if (mb > 1024) return (mb / 1024).toFixed(1) + " GB";
  return mb.toFixed(0) + " MB";
}

// ---- INDI server & drivers panel -----------------------------------------
// Extracted out of renderController() (issue #238) so the setup wizard's own
// first step (wizard.js) can reuse the exact same controls instead of
// duplicating them - a second copy would drift the moment one of the two
// got a fix/tweak the other didn't. `scope` is the caller's own
// client.scope() (its subscriptions are torn down together with the rest of
// the caller's own screen/step, not separately). `onChange` (optional) fires
// every time this panel's own live data redraws - the wizard step uses it to
// know when to recompute "is there at least one active driver yet" (see its
// own `canProceed` gate), renderController() has no use for it and omits it.
//
// indiserver reflects whether ostserver itself was launched with
// --indiserver, not whether some INDI server happens to be running right
// now (controller.en.md) — "N" means OST isn't the one managing it at all
// (a separately-run indiserver, or none), so there is nothing to
// start/stop/list here. "Y" gets the controls; more to come here per
// Gilles, 2026-10-09 (issue #231, second half).
export function buildIndiPanel(client, scope, onChange) {
  const indiSec = el("section", { class: "panel" });
  const indiTitle = el("h3");
  indiSec.appendChild(indiTitle);
  const indiExternalMsg = el("div", { class: "muted small hidden", text: t("views.indiServerManagedElsewhere") });
  indiSec.appendChild(indiExternalMsg);
  const indiControls = el("div", { class: "hidden" });
  indiControls.appendChild(el("div", { class: "form-row" }, [
    el("button", { class: "btn btn-sm", onclick: () => client.indiStart() }, t("views.startServer")),
    el("button", { class: "btn btn-sm", onclick: () => client.indiStop() }, t("views.stopServer")),
  ]));
  // Top zone: drivers actually running right now (indiactivedrivers) -
  // reload/stop only, no "load" (they're already loaded).
  const connectAllBtn = el("button", { class: "btn btn-sm", onclick: connectAll }, t("views.connectAll"));
  indiControls.appendChild(el("div", { class: "panel-header-row" }, [
    el("div", { class: "muted small", text: t("views.activeIndiDrivers") }),
    connectAllBtn,
  ]));
  const activeDriversList = el("ul");
  indiControls.appendChild(activeDriversList);

  // Gilles, 2026-10-13 (both here and the setup wizard's own step 1, which
  // reuses this same panel - see this function's own doc comment): one
  // click to connect every currently active driver's device, instead of
  // each one individually. Same indiConnectDriver() the per-driver button
  // already calls, just looped.
  function connectAll() {
    const activeDrivers = client.get(client.topics.controllerData("indiactivedrivers")) || [];
    for (const d of activeDrivers) client.indiConnectDriver(d.binary);
  }

  // Bottom zone: free-text search (driverSearchInput is created once,
  // outside any subscription, same caution as labelInput above - never
  // touched again after creation) over the full driver catalogue
  // (indidrivers), each result with its own "start" button. Filtering
  // only kicks in from 3 characters - the catalogue is in the hundreds on
  // a full INDI install, not worth listing unfiltered.
  indiControls.appendChild(el("div", { class: "muted small", text: t("views.startIndiDriver") }));
  const driverSearchInput = el("input", { type: "text", class: "field-input", placeholder: t("views.indiDriverSearchPlaceholder") });
  indiControls.appendChild(driverSearchInput);
  const availableDriversList = el("ul");
  indiControls.appendChild(availableDriversList);
  indiSec.appendChild(indiControls);

  // Case- and accent-insensitive: NFD-normalize then strip the combining
  // diacritical marks Unicode splits accents into, so "é" and "e" compare
  // equal (e.g. "Celestron" / "télescope").
  function normalizeForSearch(s) {
    return s.normalize("NFD").replace(/[̀-ͯ]/g, "").toLowerCase();
  }

  function renderAvailableDrivers() {
    const query = normalizeForSearch(driverSearchInput.value.trim());
    availableDriversList.innerHTML = "";
    if (query.length < 3) return;
    const drivers = client.get(client.topics.controllerData("indidrivers")) || [];
    for (const d of drivers) {
      if (!normalizeForSearch(`${d.label} (${d.family})`).includes(query)) continue;
      const li = el("li");
      li.appendChild(el("span", { class: "spacer small", text: `${d.label} (${d.family})` }));
      li.appendChild(el("button", { class: "btn btn-sm", onclick: () => client.indiLoadDriver(d.binary) }, t("views.start")));
      availableDriversList.appendChild(li);
    }
  }
  driverSearchInput.addEventListener("input", renderAvailableDrivers);

  function renderIndi() {
    const indiserver = client.get(client.topics.controllerData("indiserver"));
    const enabled = indiserver === "Y";
    indiTitle.textContent = t("views.indiServerPrefix") + (enabled ? t("views.enabled") : t("views.disabled"));
    indiExternalMsg.classList.toggle("hidden", enabled);
    indiControls.classList.toggle("hidden", !enabled);
    if (!enabled) {
      if (onChange) onChange();
      return;
    }
    const activeDrivers = client.get(client.topics.controllerData("indiactivedrivers")) || [];
    connectAllBtn.disabled = activeDrivers.length === 0;
    activeDriversList.innerHTML = "";
    for (const d of activeDrivers) {
      // Two lines - label, then icon buttons below it - rather than
      // cramming four buttons on the same row as the label (Gilles,
      // 2026-10-09: that overflowed the panel on a narrow column).
      const li = el("li", { class: "indi-driver-row" });
      li.appendChild(el("div", { class: "small", text: `${d.label} (${d.family})` }));
      const actions = el("div", { class: "icon-btn-row" });
      // Connect/disconnect act on the device itself (YC/YD), separate from
      // the driver process (reload/stop) - starting a driver doesn't
      // connect its device on its own, see issue #231.
      actions.appendChild(iconButton("link", t("views.connect"), () => client.indiConnectDriver(d.binary)));
      actions.appendChild(iconButton("link_off", t("views.disconnect"), () => client.indiDisconnectDriver(d.binary)));
      actions.appendChild(iconButton("refresh", t("views.reload"), () => client.indiReloadDriver(d.binary)));
      actions.appendChild(iconButton("stop", t("views.stop"), () => client.indiStopDriver(d.binary), { danger: true }));
      li.appendChild(actions);
      activeDriversList.appendChild(li);
    }
    if (onChange) onChange();
  }
  scope.subscribe(client.topics.controllerData("indiserver"), renderIndi);
  scope.subscribe(client.topics.controllerData("indidrivers"), renderIndi);
  scope.subscribe(client.topics.controllerData("indiactivedrivers"), renderIndi);
  scope.subscribe(client.topics.controllerData("indidrivers"), renderAvailableDrivers);

  return indiSec;
}

export function renderController(client) {
  const root = el("div", { class: "controller-view" });
  const scope = client.scope();

  // Header: git info only - Gilles, 2026-10-09: drop the "Controller"
  // title and the banner line, both useless here. No live input lives in
  // this slot, so a redraw on every uc is harmless - unlike the sections
  // below, there's no need to reach for a per-key topic for git.
  const headerSlot = el("div");
  root.appendChild(headerSlot);
  scope.subscribe(client.topics.controllerData(), (cd) => {
    cd = cd || {};
    headerSlot.innerHTML = "";
    if (cd.git) headerSlot.appendChild(el("div", { class: "muted small mono", text: `${cd.git.Gittag || ""} ${cd.git.Githash || ""} — ${cd.git.Gitdate || ""}` }));
  });

  const grid = el("div", { class: "panel-grid" });
  root.appendChild(grid);

  // Issue #231: the previous version rebuilt the *entire* view (including
  // the "Load a module" and "Save configuration as" <input>s) from a
  // single render() driven by the blanket controllerData() topic — so a
  // periodic systemwatcher tick (nothing to do with what the user was
  // typing) wiped whatever text was in those fields. Each section below
  // now subscribes only to the controllerdata key(s) it actually renders
  // (client-pattern.en.md "Rendering: scope updates to the property that
  // changed") and redraws only its own sub-container, never the inputs
  // that live in the same panel.

  // ---- Server load (systemwatcher) ----
  // Appended to the grid last (Gilles, 2026-10-09: keep this panel at the
  // end) — created here regardless, so the subscription below is set up
  // alongside every other section instead of out of order.
  const swSec = el("section", { class: "panel hidden" }, [el("h3", { text: t("views.serverLoad") })]);
  const swBody = el("div");
  swSec.appendChild(swBody);
  scope.subscribe(client.topics.controllerData("systemwatcher"), (sw) => {
    swSec.classList.toggle("hidden", !sw);
    swBody.innerHTML = "";
    if (!sw) return;
    if (sw.cpu) swBody.appendChild(el("div", { class: "metric-row" }, [el("span", { class: "label", text: t("views.cpuLabel") }), el("span", { class: "value", text: `${sw.cpu.load_1m} / ${sw.cpu.load_5m} / ${sw.cpu.load_15m}` })]));
    if (sw.ram) swBody.appendChild(el("div", { class: "metric-row" }, [el("span", { class: "label", text: t("views.ramLabel") }), el("span", { class: "value", text: `${fmtBytes(sw.ram.used_mb)} / ${fmtBytes(sw.ram.total_mb)}` })]));
    for (const d of sw.disks || []) swBody.appendChild(el("div", { class: "metric-row" }, [el("span", { class: "label", text: `${d.mountpoint} (${d.device})` }), el("span", { class: "value", text: `${d.used_gb.toFixed(1)} / ${d.total_gb.toFixed(1)} GB` })]));
    for (const n of sw.network || []) swBody.appendChild(el("div", { class: "metric-row" }, [el("span", { class: "label", text: n.iface }), el("span", { class: "value", text: `↓${n.rx_kbps.toFixed(1)} ↑${n.tx_kbps.toFixed(1)} kbps` })]));
    // "temp", not "t" — the latter would shadow the i18n t() used all over this file.
    for (const temp of sw.temperatures || []) swBody.appendChild(el("div", { class: "metric-row" }, [el("span", { class: "label", text: temp.type }), el("span", { class: "value", text: `${temp.temp_c.toFixed(1)} °C` })]));
  });

  // ---- Configurations ----
  const confSec = el("section", { class: "panel" }, [el("h3", { text: t("views.configurations") })]);
  const confList = el("ul");
  confSec.appendChild(confList);
  const nameInput = el("input", { type: "text", class: "field-input", placeholder: t("views.configNamePlaceholder") });
  confSec.appendChild(el("div", { class: "form-row" }, [nameInput, el("button", { class: "btn btn-primary btn-sm", onclick: () => { if (nameInput.value) client.saveConfiguration(nameInput.value); } }, t("views.saveCurrentAs"))]));
  grid.appendChild(confSec);
  function renderConfigurations() {
    const confs = client.get(client.topics.controllerData("availableconfs")) || {};
    const currentconf = client.get(client.topics.controllerData("currentconf"));
    confList.innerHTML = "";
    for (const [name, modlist] of Object.entries(confs)) {
      const li = el("li");
      li.appendChild(el("span", { class: "spacer", text: `${name}${currentconf === name ? t("views.current") : ""} — ${(modlist || []).length} ${t("views.moduleCountSuffix")}` }));
      li.appendChild(el("button", { class: "btn btn-sm", onclick: () => client.loadConfiguration(name) }, t("views.load")));
      confList.appendChild(li);
    }
  }
  // availableconfs and currentconf arrive as two independent uc keys (a uc
  // only ever carries one) but both feed this same list — either one
  // redraws it, each reading the other's latest value fresh.
  scope.subscribe(client.topics.controllerData("availableconfs"), renderConfigurations);
  scope.subscribe(client.topics.controllerData("currentconf"), renderConfigurations);

  // ---- Loaded modules ----
  // "Kill all" (Gilles, 2026-10-13, Controller screen only - explicitly
  // NOT the setup wizard) - same client.killModule() loop a user would do
  // by hand one row at a time, GlobalDatastore excluded same as the
  // per-row button below (the backend itself refuses to kill it anyway,
  // Controller::onExternalEvent's MK case). Disabled whenever there is
  // nothing to kill, kept in sync by the same subscription that redraws
  // the list.
  const killAllBtn = el("button", { class: "btn btn-sm btn-danger", text: t("views.killAll"), onclick: killAll });
  const modSec = el("section", { class: "panel" }, [
    el("div", { class: "panel-header-row" }, [el("h3", { text: t("views.loadedModules") }), killAllBtn]),
  ]);
  const modList = el("ul");
  modSec.appendChild(modList);
  grid.appendChild(modSec);
  function killAll() {
    const names = (client.get(client.topics.modules()) || []).filter((n) => n !== "GlobalDatastore");
    if (names.length === 0) return;
    if (!confirm(t("views.killAllConfirm", { count: String(names.length) }))) return;
    for (const name of names) client.killModule(name);
  }
  scope.subscribe(client.topics.modules(), (names) => {
    modList.innerHTML = "";
    const killable = (names || []).filter((n) => n !== "GlobalDatastore");
    killAllBtn.disabled = killable.length === 0;
    for (const name of names || []) {
      const li = el("li");
      li.appendChild(el("span", { class: "spacer mono small", text: name }));
      if (name !== "GlobalDatastore") li.appendChild(el("button", { class: "btn btn-sm btn-danger", onclick: () => { if (confirm(t("views.unloadConfirm", { name }))) client.killModule(name); } }, t("views.kill")));
      modList.appendChild(li);
    }
  });

  // ---- Load a module ----
  // labelInput is created once, outside any subscription — this is the
  // field issue #231 was filed about, it must never be touched again
  // after the user starts typing in it. profileSelect is a dropdown, not
  // a free-text field, matching OstErix's own Load module dialog
  // (global-parameters.component.ts LoadModuleDialogComponent): options
  // come from controllerdata.profiles, keyed by the library name with its
  // first letter uppercased and the rest lowercased (see
  // controller.en.md's "profiles keying" warning — not a plain capitalize,
  // and not the same string as libSelect's own stripped value).
  const libSec = el("section", { class: "panel" }, [el("h3", { text: t("views.loadAModule") })]);
  const libSelect = el("select", { class: "field-input" });
  const labelInput = el("input", { type: "text", class: "field-input", placeholder: t("views.instanceLabelPlaceholder") });
  const profileSelect = el("select", { class: "field-input" });
  libSec.appendChild(el("div", { class: "form-row" }, [libSelect, labelInput, profileSelect,
    el("button", { class: "btn btn-primary btn-sm", onclick: () => { if (labelInput.value) client.loadModule(libSelect.value, labelInput.value, profileSelect.value || undefined); } }, t("views.load")),
  ]));
  grid.appendChild(libSec);

  // Rebuilds profileSelect's options for whichever library is currently
  // selected. Called both when `profiles` itself updates and when the
  // user switches libraries (via libSelect's own "change" listener below)
  // — a library switch naturally resets the selection to "none" here,
  // same as OstErix's onLibraryChange(), since the old value almost never
  // matches an option in the new library's own profile list.
  function renderProfiles() {
    const profiles = client.get(client.topics.controllerData("profiles")) || {};
    const profileKey = libSelect.value ? libSelect.value.charAt(0).toUpperCase() + libSelect.value.slice(1).toLowerCase() : "";
    const options = Array.isArray(profiles[profileKey]) ? profiles[profileKey] : [];
    const prevValue = profileSelect.value;
    profileSelect.innerHTML = "";
    profileSelect.appendChild(el("option", { value: "", text: t("views.profileOptionalPlaceholder") }));
    for (const p of options) profileSelect.appendChild(el("option", { value: p, text: p }));
    if (options.includes(prevValue)) profileSelect.value = prevValue;
  }
  libSelect.addEventListener("change", renderProfiles);

  scope.subscribe(client.topics.controllerData("libraries"), (libs) => {
    // libraries is static in practice (scanned once at server startup) but
    // keep the currently selected library across a redraw regardless.
    const prevValue = libSelect.value;
    libSelect.innerHTML = "";
    for (const key of Object.keys(libs || {})) {
      const stripped = key.startsWith("libost") ? key.slice(6) : key;
      libSelect.appendChild(el("option", { value: stripped, text: key }));
    }
    if ([...libSelect.options].some((o) => o.value === prevValue)) libSelect.value = prevValue;
    renderProfiles();
  });
  scope.subscribe(client.topics.controllerData("profiles"), renderProfiles);

  // ---- INDI server & drivers ----
  grid.appendChild(buildIndiPanel(client, scope));

  grid.appendChild(swSec);

  return { node: root, dispose: scope.dispose };
}

function buildFolderTree(folders) {
  const root = { name: "", path: "", children: new Map() };
  for (const path of folders) {
    if (path === "") continue;
    const parts = path.split("/").filter(Boolean);
    let node = root;
    let acc = "";
    for (const part of parts) {
      acc += "/" + part;
      if (!node.children.has(part)) node.children.set(part, { name: part, path: acc, children: new Map() });
      node = node.children.get(part);
    }
  }
  return root;
}

function renderFolderNode(node, client, depth) {
  const wrap = el("div", { class: "folder-node", style: `padding-left:${depth * 14}px` });
  const btn = el("button", { class: "folder-btn" }, [icon("folder", { small: true }), el("span", { text: node.name || t("views.rootFolder") })]);
  btn.addEventListener("click", () => client.selectFolder(node.path));
  wrap.appendChild(btn);
  const children = Array.from(node.children.values()).sort((a, b) => a.name.localeCompare(b.name));
  for (const child of children) wrap.appendChild(renderFolderNode(child, client, depth + 1));
  return wrap;
}

export function renderFiles(client, mediaBaseUrl) {
  const root = el("div", { class: "files-view" });
  const scope = client.scope();

  scope.subscribe(client.topics.files(), (files) => {
    files = files || { folders: [], files: [], selectedfolder: "" };
    root.innerHTML = "";
    root.appendChild(el("h2", null, [icon("folder"), el("span", { text: " " + t("views.files") })]));
    const layout = el("div", { class: "files-layout" });
    const tree = buildFolderTree(files.folders || []);
    const treeWrap = el("div", { class: "folder-tree" });
    // The selected folder in the dump is an absolute server-side path, not
    // the relative path used for navigation/FS — we cannot reliably
    // highlight the active node from it alone (see ostcore/FINDINGS.md), so
    // no node is marked "active" here.
    treeWrap.appendChild(renderFolderNode(tree, client, 0));
    layout.appendChild(treeWrap);

    const listWrap = el("div", { class: "file-list-wrap" });
    listWrap.appendChild(el("div", { class: "muted small mono", text: t("views.selectedPrefix") + (files.selectedfolder || t("views.none")) }));
    const fgrid = el("div", { class: "file-grid" });
    for (const f of files.files || []) {
      const url = mediaBaseUrl + (f.startsWith("/") ? f.slice(1) : f);
      const lower = f.toLowerCase();
      const isImg = /\.(jpe?g|png|gif|webp)$/.test(lower);
      const card = el("a", { href: url, target: "_blank", class: "file-card" });
      if (isImg) card.appendChild(el("img", { src: url, class: "file-thumb" }));
      else card.appendChild(icon("description", { small: false }));
      card.appendChild(el("div", { class: "file-name", text: f.split("/").pop() }));
      fgrid.appendChild(card);
    }
    if (!(files.files || []).length) fgrid.appendChild(el("div", { class: "muted small", text: t("views.noFilesInFolder") }));
    listWrap.appendChild(fgrid);
    layout.appendChild(listWrap);
    root.appendChild(layout);
  });

  return { node: root, dispose: scope.dispose };
}

export function renderLogs(client) {
  const root = el("div", { class: "logs-view" });
  const scope = client.scope();

  scope.subscribe(client.topics.logs(), (logs) => {
    root.innerHTML = "";
    root.appendChild(el("h2", null, [icon("list"), el("span", { text: " " + t("views.logs") })]));
    root.appendChild(el("div", { class: "log-toolbar muted small", text: `${(logs || []).length} ${t("views.entriesCappedSuffix")}` }));
    const table = el("table", { class: "log-table" });
    for (const entry of (logs || []).slice().reverse()) {
      const lvl = LOG_LEVELS[entry.l] || "info";
      table.appendChild(el("tr", { class: "log-row log-" + lvl }, [
        el("td", { class: "small muted mono", text: entry.d }),
        el("td", { class: "small mono", text: entry.c }),
        el("td", { text: entry.t }),
      ]));
    }
    root.appendChild(table);
  });

  return { node: root, dispose: scope.dispose };
}

// ---- always-on floating log panel ----------------------------------------
// Gilles, 2026-10-03: "en haut a droite ... ajoute ... un bouton pour la
// gestion de logs - inspire de ostnoctua pour la forme de la fenetre a
// afficher: selection du module / selection du niveau du message / quand
// on clique depuis un module ca selectionne les messages du module /
// couleur selon niveau du message". Shape mirrors ostnoctua's own
// web/log.js (OstLog): a persistent floating panel (Panels.make()'s own
// `persistent: true`, survives module switches, same as ostnoctua's own
// "always-on" framing) with a module + minimum-level filter toolbar above
// a scrollable, colour-coded list - but the module filter here is a real
// per-module dropdown (every module that has actually logged something,
// built from the log entries' own `c` field) rather than ostnoctua's
// simpler this-module/all-modules toggle, since Gilles asked for
// "selection du module" specifically.
const LOG_FILTER_KEY = "ostcore-ui3:logs:filter";
function loadLogFilter() {
  try { return JSON.parse(localStorage.getItem(LOG_FILTER_KEY)) || {}; } catch (e) { return {}; }
}
function saveLogFilter(f) {
  try { localStorage.setItem(LOG_FILTER_KEY, JSON.stringify(f)); } catch (e) { /* ignore */ }
}
const LOG_MODULE_ALL = "__all";

export function renderLogPanel(client, initialModule) {
  const existing = document.querySelector('.panel-float[data-panel="logs"]');
  if (existing) {
    // Already built (this is a toggle, not a re-create - the panel is
    // persistent, it never gets torn down by a module switch). Re-apply
    // the module filter if the button was clicked from a module's own
    // view this time, same as a fresh creation would below, then just
    // show/hide it like any other panel.
    if (initialModule) {
      const sel = existing.querySelector(".log-filter-module");
      if (sel && [...sel.options].some((o) => o.value === initialModule)) {
        sel.value = initialModule;
        sel.dispatchEvent(new Event("change"));
      }
    }
    togglePanel("logs");
    return;
  }

  const filter = loadLogFilter();
  if (initialModule) filter.module = initialModule;
  if (!filter.level) filter.level = "0";

  const body = makePanel({
    id: "logs", title: t("views.logs"), persistent: true,
    x: Math.max(20, window.innerWidth - 460), y: Math.max(80, window.innerHeight - 340), w: 420,
  });
  body.classList.add("log-panel-body");

  const toolbar = el("div", { class: "log-toolbar" });
  const moduleSel = el("select", { class: "field-input log-filter-module" });
  const levelSel = el("select", { class: "field-input log-filter-level" });
  for (const [key, label] of [
    ["0", t("views.logLevelAll")], ["1", t("views.logLevelInfo")], ["2", t("views.logLevelWarning")],
    ["3", t("views.logLevelError")], ["4", t("views.logLevelCritical")],
  ]) {
    const opt = el("option", { value: key }, label);
    levelSel.appendChild(opt);
  }
  levelSel.value = filter.level;
  toolbar.appendChild(moduleSel);
  toolbar.appendChild(levelSel);
  body.appendChild(toolbar);

  const list = el("div", { class: "log-panel-list" });
  body.appendChild(list);

  function passes(entry) {
    if ((entry.l | 0) < (filter.level | 0)) return false;
    return !filter.module || filter.module === LOG_MODULE_ALL || entry.c === filter.module;
  }
  function rebuildRows(logs) {
    // Gilles, 2026-10-06: "la fenetre de logs (partout sur tous les
    // modules) doit afficher le dernier message en premier, sinon on est
    // oblige de scroller pour voir les trucs recents" - newest first
    // (reversed from the wire's own append order, same as renderLogs()'s
    // full-page Logs view already does), and the "stay pinned so new
    // entries remain visible without having to re-scroll" check flips
    // from atBottom to atTop to match: new rows now land at the TOP, not
    // the bottom.
    const atTop = list.scrollTop < 24;
    list.innerHTML = "";
    for (const entry of (logs || []).filter(passes).slice().reverse()) {
      list.appendChild(el("div", { class: "log-panel-row log-" + (LOG_LEVELS[entry.l] || "info") }, [
        el("span", { class: "log-panel-time", text: String(entry.d || "").replace(/^.*[ T]/, "").slice(0, 8) }),
        el("span", { class: "log-panel-src", text: entry.c || "" }),
        el("span", { class: "log-panel-msg", text: entry.t || "" }),
      ]));
    }
    if (atTop) list.scrollTop = 0;
  }

  // Module options: every module that has actually logged something (built
  // from the log entries' own `c` field, not the live module list - a
  // module that's since been unloaded, or the fixed non-module "WS"
  // source, keeps its own past entries reachable this way), plus "All".
  // Rebuilt on every log update; the current selection (including one not
  // presently in the list, e.g. just set via initialModule) is preserved.
  client.subscribe(client.topics.logs(), (logs) => {
    const sources = [...new Set((logs || []).map((e) => e.c).filter(Boolean))].sort();
    if (filter.module && filter.module !== LOG_MODULE_ALL && !sources.includes(filter.module)) sources.push(filter.module);
    const current = moduleSel.value || filter.module || LOG_MODULE_ALL;
    moduleSel.innerHTML = "";
    moduleSel.appendChild(el("option", { value: LOG_MODULE_ALL }, t("views.logModuleAll")));
    for (const src of sources) moduleSel.appendChild(el("option", { value: src }, src));
    moduleSel.value = [...moduleSel.options].some((o) => o.value === current) ? current : LOG_MODULE_ALL;
    rebuildRows(logs);
  });

  moduleSel.addEventListener("change", () => {
    filter.module = moduleSel.value;
    saveLogFilter(filter);
    rebuildRows(client.get(client.topics.logs()));
  });
  levelSel.addEventListener("change", () => {
    filter.level = levelSel.value;
    saveLogFilter(filter);
    rebuildRows(client.get(client.topics.logs()));
  });
}
