// js/credits.js — "Credits" modal (Gilles, 2026-10-14: "ajoute une fenetre
// modale 'Credits'... mets a minima la reference a Aladin et D3-celestial et
// bien sur documentation.ostserver.fr"), opened from a button in the general
// Settings panel, right below "Clear cache + localStorage" (index.html).
//
// Deliberately minimal for now - just the 3 sources Gilles explicitly named.
// This app actually bundles/derives from several more third-party sources
// (HYG-Database, OpenNGC - see front/celestial-data/README.md) that belong
// here too; tracked as its own chantier rather than guessed at, see
// notes/NOTES-frontend-ostcore-credits.md.
//
// Same plain .dialog-overlay/.dialog shell every other simple dialog in this
// app uses (e.g. render.js's openPropertyEditDialog) - static content, no
// form, so no need for wizard.js's own heavier machinery.

import { el, icon } from "./dom.js";
import { t } from "./i18n.js";

const SOURCES = [
  {
    nameKey: "credits.aladin.name",
    descKey: "credits.aladin.desc",
    url: "https://aladin.cds.unistra.fr/AladinLite/",
  },
  {
    nameKey: "credits.d3celestial.name",
    descKey: "credits.d3celestial.desc",
    url: "https://github.com/ofrohn/d3-celestial",
  },
];

export function openCreditsDialog() {
  const overlay = el("div", { class: "dialog-overlay" });
  const closeBtn = el("button", { class: "btn-icon", title: t("common.close"), onclick: () => overlay.remove() }, [icon("close")]);

  const list = el("ul", { class: "credits-list" });
  for (const s of SOURCES) {
    list.appendChild(el("li", null, [
      el("a", { href: s.url, target: "_blank", rel: "noopener", text: t(s.nameKey) }),
      el("span", { class: "muted small", text: " — " + t(s.descKey) }),
    ]));
  }

  const docLink = el("a", { href: "https://documentation.ostserver.fr", target: "_blank", rel: "noopener", text: "documentation.ostserver.fr" });

  const dialog = el("div", { class: "dialog credits-dialog" }, [
    // .panel-header-row (components.css) - the generic, non-dialog-scoped
    // "title + action button(s), space-between" shape (.params-dialog's
    // own .dialog-header is scoped to that one dialog specifically).
    el("div", { class: "panel-header-row" }, [el("h3", { text: t("credits.title") }), closeBtn]),
    el("p", { class: "muted small", text: t("credits.intro") }),
    list,
    el("p", { class: "small" }, [docLink]),
  ]);
  overlay.appendChild(dialog);
  overlay.addEventListener("click", (ev) => { if (ev.target === overlay) overlay.remove(); });
  document.getElementById("dialog-root").appendChild(overlay);
}
