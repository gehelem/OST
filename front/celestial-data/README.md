# celestial-data/

Catalog data for the sky-dome planetarium view (`js/sky-*.js`,
`templates/navigator.js`). Data only — no code from any of these sources
is reused; see `notes/NOTES-frontend-ostcore-planetarium.md` for why the
projection/rendering engine itself is a fresh implementation.

| File | Contents | Source |
|---|---|---|
| `stars.6.json` | ~8.9k stars, mag ≤ 6.5 (naked-eye-ish, "light" depth) | Trimmed from `stars.8.json` below |
| `stars.8.json` | ~41k stars, mag ≤ 8 ("deep" depth) | `noctua-master/packages/skymap/public/celestial-data/`, itself derived from the HYG/ATHYG catalog family |
| `stars.hip.json` | ~118k stars, every HYG entry with a Hipparcos number (mag up to ~14, the "full" depth — Gilles, 2026-10-03: "le grand saut : on augmente la densite, on passe la taille au dessus sur les catalogues", after pointing at astro2001's own ~871k-star full catalog as the reference scale to aim toward) | Generated directly from [astronexus/HYG-Database](https://github.com/astronexus/HYG-Database) (`hygdata_v41.csv`), 2026-10-03 — same source and licence as `star-proper-names.json` below, just the full star list (id/ra/dec/mag/bv) instead of only the ~450 proper names. Entries without a HIP number (~1.4% of the file, mostly very faint Gaia-only additions) are skipped, keeping the same id scheme as every other file here. `ra` converted from HYG's own decimal-hours to degrees to match this project's convention. |
| `constellations.lines.json` | Constellation figures (88) | `noctua-master/packages/skymap/public/celestial-data/` |
| `messier.json` | Messier catalog (M1–M110) | `noctua-master/packages/skymap/public/celestial-data/` |
| `mw.json` | Milky Way outline + light fill (5 MultiPolygon features, outer boundaries + holes e.g. the Coalsack, filled with the "evenodd" rule so holes stay holes — Gilles, 2026-10-01: "ajoute son affichage en optionnel dans les settings du module", then "tu peux griser legerement l'interieur de ces contours") | `noctua-master/packages/skymap/public/celestial-data/`, itself from [d3-celestial](https://github.com/ofrohn/d3-celestial) — byte-identical to the copy `ostnoctua`/`OstErix` already ship |
| `star-proper-names.json` | HIP id → common name, for the ~450 stars that have one (e.g. `62956`→`"Alioth"`) | Fetched independently from [astronexus/HYG-Database](https://github.com/astronexus/HYG-Database) (`hygdata_v41.csv`, `proper` column), 2026-09-29 — **not** from skymap, per Gilles: "cherche plutot comment faire proprement avec les nouveaux catalogues". Licensed [CC BY-SA 4.0](http://creativecommons.org/licenses/by-sa/4.0/) — attribution: HYG Database (astronexus), itself compiled from the Hipparcos, Yale Bright Star, and Gliese catalogs. |

Star ids throughout (`stars.*.json`'s `feature.id`, `star-proper-names.json`'s
keys) are Hipparcos catalog numbers (HIP) — the same numbering scheme
regardless of which of the above files they came from, which is what makes
matching them together possible without needing all the data to come from
one single source.
