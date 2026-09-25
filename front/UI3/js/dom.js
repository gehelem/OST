// dom.js — tiny shared DOM-building helpers used by render.js/views.js.
// Pulled into its own module (UI2 duplicated this in both files) so both
// call sites stay in sync and the icon/status vocabulary lives in one place.

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
const STATUS_LABEL = ["Standby", "OK", "Busy", "Error"];

export function statusDot(status) {
  const s = typeof status === "number" ? status : 0;
  return el("span", { class: "status-dot status-" + STATUS_CLASS[s], title: STATUS_LABEL[s] });
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
// Five of Rooster's icon names (zoom_in_map/calendar_today/manage_search/
// garage/query_stats) don't exist in this project's vendored
// MaterialIcons-Regular.ttf, the older "classic" set — see this file's
// other note on that. Substituted with same-intent names confirmed present
// in the font instead of carrying the gap over silently.
const TEMPLATE_ICON_MAP = {
  focus: "filter_center_focus",
  sequencer: "view_list",
  navigator: "explore",
  guider: "gps_fixed", // Rooster: zoom_in_map (missing from this font)
  allsky: "panorama",
  planner: "event", // Rooster: calendar_today (missing from this font)
  inspector: "search", // Rooster: manage_search (missing from this font)
  parkmanager: "local_parking", // Rooster: garage (missing from this font)
  monitor: "show_chart", // Rooster: query_stats (missing from this font)
};
export function moduleIcon(mod) {
  const template = ((mod && mod.infos && mod.infos.template) || "").toLowerCase();
  return TEMPLATE_ICON_MAP[template] || "extension"; // Rooster's own DEFAULT_TEMPLATE_ICON, e.g. for GlobalDatastore
}
