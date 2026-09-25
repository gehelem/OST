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
const MODULE_ICON_HINTS = [
  [/mount|eq|telescope/i, "explore"],
  [/guid/i, "gps_fixed"],
  [/focus/i, "center_focus_strong"],
  [/camera|ccd|capture|imag/i, "camera_alt"],
  [/filter/i, "filter_alt"],
  [/sequenc/i, "playlist_play"],
  [/dome|roof|shutter/i, "meeting_room"],
  [/weather|meteo/i, "cloud"],
  [/polar/i, "adjust"],
  [/rotat/i, "screen_rotation"],
  [/dummy/i, "science"],
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
