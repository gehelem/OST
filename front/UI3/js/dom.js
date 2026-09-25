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

// Best-effort icon per module "kind" (infos.template / infos.lib / the
// module name itself), purely cosmetic — falls back to a generic icon for
// any module type this table doesn't know about, so a new/unknown module
// still renders a sensible sidebar entry (client-pattern.en.md's "generic
// fallback" rule applied to iconography, not just layout).
//
// Every name used anywhere in UI3 (here and elsewhere) must actually exist
// in assets/fonts/MaterialIcons-Regular.ttf — it's the older, ~900-glyph
// "classic" Material Icons set, not the current/expanded one, so several
// newer-looking names (dark_mode, filter_alt, meeting_room, science, ...)
// silently render nothing. Caught live 2026-09-25 ("le toggle de theme n'a
// pas d'icone") and cross-checked every icon name in the codebase against
// the font's actual cmap before picking replacements.
const MODULE_ICON_HINTS = [
  [/mount|eq|telescope/i, "explore"],
  [/guid/i, "gps_fixed"],
  [/focus/i, "center_focus_strong"],
  [/camera|ccd|capture|imag/i, "camera_alt"],
  [/filter/i, "filter_list"],
  [/sequenc/i, "playlist_play"],
  [/dome|roof|shutter/i, "home"],
  [/weather|meteo/i, "cloud"],
  [/polar/i, "adjust"],
  [/rotat/i, "screen_rotation"],
  [/dummy/i, "bug_report"],
  [/blindpec|pec/i, "sync"],
  [/collimat/i, "settings_input_component"],
  [/switch|relay|power/i, "power"],
  [/indi/i, "usb"],
  [/globaldatastore/i, "storage"],
];
export function moduleIcon(mod, name) {
  const hint = ((mod && mod.infos && (mod.infos.template || mod.infos.lib)) || "") + " " + (name || "");
  for (const [re, iconName] of MODULE_ICON_HINTS) if (re.test(hint)) return iconName;
  return "extension";
}
