// js/credits.js — "Credits" modal (Gilles, 2026-10-13/14), opened from a
// button in the general Settings panel, right below "Clear cache +
// localStorage" (index.html).
//
// Frontend only - the backend's own third-party dependencies (Qt, INDI,
// StellarSolver, CFITSIO...) are already covered by
// documentation.ostserver.fr/licences/, not duplicated here. Every
// third-party source actually bundled/derived from in front/ (lib/*.js,
// celestial-data/*.json, assets/fonts/) - see front/celestial-data/
// README.md and each lib/*.js file's own header for where each `license`/
// `url` below comes from. Capacitor/capacitor-zeroconf are only ever
// exercised by the Android build, not a plain web session - listed here
// anyway (Gilles: "tant pis, on ajoute aussi sur le web").
//
// Same plain .dialog-overlay/.dialog shell every other simple dialog in this
// app uses (e.g. render.js's openPropertyEditDialog) - static content, no
// form, so no need for wizard.js's own heavier machinery.

import { el, icon } from "./dom.js";
import { t } from "./i18n.js";

// `license` strings are standard SPDX identifiers - left untranslated on
// purpose, same convention this app already uses for module/backend
// license mentions elsewhere.
const SOURCES = [
  {
    nameKey: "credits.aladin.name",
    descKey: "credits.aladin.desc",
    license: "LGPL-3.0-or-later",
    url: "https://aladin.cds.unistra.fr/AladinLite/",
  },
  {
    // Gilles, 2026-10-14, confirming: "messier.json (et constellation)
    // viennent certes de noctua qui les a eu depuis d3-celestial" - so
    // this credits constellations.lines.json/messier.json too, not just
    // mw.json (the only one front/celestial-data/README.md itself
    // explicitly traces back this far).
    nameKey: "credits.d3celestial.name",
    descKey: "credits.d3celestial.desc",
    license: "BSD-3-Clause",
    url: "https://github.com/ofrohn/d3-celestial",
  },
  {
    nameKey: "credits.hyg.name",
    descKey: "credits.hyg.desc",
    license: "CC BY-SA 4.0",
    url: "https://github.com/astronexus/HYG-Database",
  },
  {
    nameKey: "credits.openngc.name",
    descKey: "credits.openngc.desc",
    license: "CC BY-SA 4.0",
    url: "https://github.com/mattiaverga/OpenNGC",
  },
  {
    nameKey: "credits.chartjs.name",
    descKey: "credits.chartjs.desc",
    license: "MIT",
    url: "https://www.chartjs.org/",
  },
  {
    nameKey: "credits.chartjsErrorBars.name",
    descKey: "credits.chartjsErrorBars.desc",
    license: "MIT",
    url: "https://github.com/sgratzl/chartjs-chart-error-bars",
  },
  {
    nameKey: "credits.astronomyEngine.name",
    descKey: "credits.astronomyEngine.desc",
    license: "MIT",
    url: "https://github.com/cosinekitty/astronomy",
  },
  {
    nameKey: "credits.d3geo.name",
    descKey: "credits.d3geo.desc",
    license: "ISC",
    url: "https://github.com/d3/d3-geo",
  },
  {
    nameKey: "credits.materialSymbols.name",
    descKey: "credits.materialSymbols.desc",
    license: "Apache-2.0",
    url: "https://github.com/google/material-design-icons",
  },
  {
    nameKey: "credits.capacitor.name",
    descKey: "credits.capacitor.desc",
    license: "MIT",
    url: "https://capacitorjs.com/",
  },
  {
    nameKey: "credits.capacitorZeroconf.name",
    descKey: "credits.capacitorZeroconf.desc",
    license: "MIT",
    url: "https://github.com/trik/capacitor-zeroconf",
  },
];

export function openCreditsDialog(client) {
  const overlay = el("div", { class: "dialog-overlay" });
  const closeBtn = el("button", { class: "btn-icon", title: t("common.close"), onclick: () => overlay.remove() }, [icon("close")]);

  const list = el("ul", { class: "credits-list" });
  for (const s of SOURCES) {
    list.appendChild(el("li", null, [
      el("div", null, [
        el("a", { href: s.url, target: "_blank", rel: "noopener", text: t(s.nameKey) }),
        el("span", { class: "muted small mono", text: " " + s.license }),
      ]),
      el("div", { class: "muted small", text: t(s.descKey) }),
    ]));
  }

  // Gilles, 2026-10-14: "tu aurais pu preciser 'Backend: see ...' pour
  // faire la difference front/back car il y a beaucoup d'autres licences
  // sur le back, pas besoin de les relister sur chaque front" - the list
  // above is frontend only (this file's own header comment), this one
  // line is what actually makes that explicit to whoever reads the
  // dialog, not just to a future reader of the source.
  const backendLink = el("a", { href: "https://documentation.ostserver.fr/licences/index.html", target: "_blank", rel: "noopener", text: "documentation.ostserver.fr/licences" });

  // Build info, below the licences - front always (CI/write-gitdata.js's
  // own gitdata.json, fetched fresh here rather than reusing a cached copy
  // elsewhere), backend only while connected (controllerdata.git, same
  // tag/hash/date/message shape - see src/controller.cpp).
  const versions = el("div", { class: "credits-versions" });
  fetch("gitdata.json").then((r) => (r.ok ? r.json() : null)).then((d) => {
    if (!d) return;
    versions.appendChild(el("div", { class: "muted small", text: t("credits.frontLabel") }));
    versions.appendChild(el("div", { class: "small mono", text: `${d.tag} · ${(d.date || "").slice(0, 10)} · ${d.commit}` }));
    versions.appendChild(el("div", { class: "small", text: d.message || "" }));
  }).catch(() => { /* no gitdata.json in this build */ });
  if (client && client.connection.status === "connected") {
    const git = client.get(client.topics.controllerData("git"));
    if (git) {
      versions.appendChild(el("div", { class: "muted small", text: t("credits.backendLabel") }));
      versions.appendChild(el("div", { class: "small mono", text: `${git.Gittag || ""} · ${git.Gitdate || ""} · ${(git.Githash || "").slice(0, 8)}` }));
      versions.appendChild(el("div", { class: "small", text: git.Gitmessage || "" }));
    }
  }

  const dialog = el("div", { class: "dialog credits-dialog" }, [
    // .panel-header-row (components.css) - the generic, non-dialog-scoped
    // "title + action button(s), space-between" shape (.params-dialog's
    // own .dialog-header is scoped to that one dialog specifically).
    el("div", { class: "panel-header-row" }, [el("h3", { text: t("credits.title") }), closeBtn]),
    el("p", { class: "muted small", text: t("credits.intro") }),
    list,
    el("p", { class: "small" }, [el("span", { class: "muted", text: t("credits.backendLabel") + " " }), backendLink]),
    el("div", { class: "settings-sep" }),
    versions,
  ]);
  overlay.appendChild(dialog);
  overlay.addEventListener("click", (ev) => { if (ev.target === overlay) overlay.remove(); });
  document.getElementById("dialog-root").appendChild(overlay);
}
