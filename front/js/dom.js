// dom.js — tiny shared DOM-building helpers used by render.js/views.js.
// Pulled into its own module (UI2 duplicated this in both files) so both
// call sites stay in sync and the icon/status vocabulary lives in one place.

import { t } from "./i18n.js";

export function el(tag, attrs, children) {
  const node = document.createElement(tag);
  if (attrs) for (const [k, v] of Object.entries(attrs)) {
    if (k === "class") node.className = v;
    else if (k === "text") node.textContent = v;
    else if (k.startsWith("on") && typeof v === "function") node.addEventListener(k.slice(2), v);
    else if (v !== undefined && v !== null && v !== false) node.setAttribute(k, v === true ? "" : v);
  }
  if (children) for (const c of [].concat(children)) {
    if (c === null || c === undefined) continue;
    node.appendChild(typeof c === "string" ? document.createTextNode(c) : c);
  }
  return node;
}

export function icon(name, opts) {
  const span = el("span", { class: "material-icons" + (opts && opts.small ? " icon-sm" : "") });
  span.textContent = name;
  return span;
}

const STATUS_CLASS = ["standby", "ok", "busy", "error"];
const STATUS_KEY = ["status.standby", "status.ok", "status.busy", "status.error"];

/** Sets/refreshes an existing dot's status class + tooltip in place, without
 * touching its other classes — split out of statusDot() so a live-updated
 * dot (e.g. the modbar's per-module global-state dot) doesn't need to be
 * recreated/replaced on every update. */
export function updateStatusDot(dotEl, status) {
  const s = typeof status === "number" ? status : 0;
  for (const c of STATUS_CLASS) dotEl.classList.remove("status-" + c);
  dotEl.classList.add("status-" + STATUS_CLASS[s]);
  dotEl.title = t(STATUS_KEY[s]);
}

/**
 * Small tab-bar + tab-panels widget (.tab-bar/.tab-btn/.tab-panel, same
 * classes/click-swap behavior as render.js's own level1/level2 property
 * tabs — kept visually identical rather than inventing a second tab look)
 * — `tabs` is `[{label, node}, ...]`, the first one starts active. Returns
 * the combined wrapper; callers keep their own reference to each `node` to
 * update its content in place later (the tab bar/panels themselves are
 * only ever built once, e.g. imagelayer.js's own Stats/Histogram split —
 * Gilles, 2026-10-03: "modifie la fenetre 'image stats' pour qu'elle soit
 * plus dense, par exemple un onglet histogramme et un onglet stats"). */
export function buildTabs(tabs) {
  const wrap = el("div");
  const tabBar = el("div", { class: "tab-bar" });
  const tabPanels = el("div", { class: "tab-panels" });
  tabs.forEach(({ label, node }, i) => {
    const btn = el("button", { class: "tab-btn" + (i === 0 ? " active" : ""), text: label });
    const panel = el("div", { class: "tab-panel" + (i === 0 ? "" : " hidden") });
    panel.appendChild(node);
    btn.addEventListener("click", () => {
      tabBar.querySelectorAll(".tab-btn").forEach((b) => b.classList.remove("active"));
      tabPanels.querySelectorAll(".tab-panel").forEach((p) => p.classList.add("hidden"));
      btn.classList.add("active");
      panel.classList.remove("hidden");
    });
    tabBar.appendChild(btn);
    tabPanels.appendChild(panel);
  });
  wrap.appendChild(tabBar);
  wrap.appendChild(tabPanels);
  return wrap;
}

export function statusDot(status) {
  const dot = el("span", { class: "status-dot" });
  updateStatusDot(dot, status);
  return dot;
}

export function iconButton(name, title, onClick, opts) {
  return el("button", { class: "btn-icon" + (opts && opts.danger ? " danger" : ""), title, onclick: onClick }, [icon(name)]);
}

// Icon per module type, keyed by `infos.template` — the module's own
// metadata, not a guess from its instance name/label. Reuses the exact
// mapping OstErix/Rooster already maintains for this purpose
// (src/app/constants.ts TEMPLATE_ICON_MAP) instead of inventing UI3's own
// heuristic: Gilles, 2026-09-25, on an earlier regex-matching version,
// "c'est cretin tes icones de modules : normalement c'est dans les
// metadonnees du module, ne pas surcharger ca". `infos.template` can be
// absent or (per FINDINGS.md) not always match a module's real identity —
// an unlisted/missing template just falls through to the same "extension"
// default Rooster itself uses.
//
// Matches Rooster's own names exactly (base.css's own header comment has
// the full story on why five of these used to be substituted here for a
// font that didn't have them — not needed anymore since the switch to
// Material Symbols Outlined), except "polar": Rooster's own
// TEMPLATE_ICON_MAP doesn't give it one either (falls back to
// "extension" there too) — Gilles, 2026-10-0X, asked for "center" here,
// which isn't a real icon name in this font; "recenter" (confirmed
// present) is the closest match he picked instead.
const TEMPLATE_ICON_MAP = {
  focus: "filter_center_focus",
  sequencer: "view_list",
  navigator: "explore",
  guider: "zoom_in_map",
  allsky: "panorama",
  planner: "calendar_today",
  inspector: "manage_search",
  parkmanager: "garage",
  monitor: "query_stats",
  polar: "recenter",
  // indipanel (infos.template, src/modules/indipanel/indipanel.cpp) renders
  // on the same generic fallback as any other unlisted template - this own
  // entry exists only to give it a distinct shortcut icon, not a bespoke
  // screen.
  indipanel: "tornado",
};
export function moduleIcon(mod) {
  const template = ((mod && mod.infos && mod.infos.template) || "").toLowerCase();
  return TEMPLATE_ICON_MAP[template] || "extension"; // Rooster's own DEFAULT_TEMPLATE_ICON, e.g. for GlobalDatastore
}
