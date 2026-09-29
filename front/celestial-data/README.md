# celestial-data/

Catalog data for the sky-dome planetarium view (`js/sky-*.js`,
`templates/navigator.js`). Data only — no code from any of these sources
is reused; see `notes/NOTES-frontend-ostcore-planetarium.md` for why the
projection/rendering engine itself is a fresh implementation.

| File | Contents | Source |
|---|---|---|
| `stars.6.json` | ~8.9k stars, mag ≤ 6.5 (naked-eye-ish, default depth) | Trimmed from `stars.8.json` below |
| `stars.8.json` | ~41k stars, mag ≤ 8 (deep depth) | `noctua-master/packages/skymap/public/celestial-data/`, itself derived from the HYG/ATHYG catalog family |
| `constellations.lines.json` | Constellation figures (88) | `noctua-master/packages/skymap/public/celestial-data/` |
| `messier.json` | Messier catalog (M1–M110) | `noctua-master/packages/skymap/public/celestial-data/` |
| `star-proper-names.json` | HIP id → common name, for the ~450 stars that have one (e.g. `62956`→`"Alioth"`) | Fetched independently from [astronexus/HYG-Database](https://github.com/astronexus/HYG-Database) (`hygdata_v41.csv`, `proper` column), 2026-09-29 — **not** from skymap, per Gilles: "cherche plutot comment faire proprement avec les nouveaux catalogues". Licensed [CC BY-SA 4.0](http://creativecommons.org/licenses/by-sa/4.0/) — attribution: HYG Database (astronexus), itself compiled from the Hipparcos, Yale Bright Star, and Gliese catalogs. |

Star ids throughout (`stars.*.json`'s `feature.id`, `star-proper-names.json`'s
keys) are Hipparcos catalog numbers (HIP) — the same numbering scheme
regardless of which of the above files they came from, which is what makes
matching them together possible without needing all the data to come from
one single source.
