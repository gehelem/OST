// sky-catalog.js — loads the sky-dome's own client-side catalog (stars,
// constellation lines, Messier objects), independent of the server's own
// search/results catalog — see notes/NOTES-frontend-ostcore-planetarium.md
// ("la carte a son propre catalogue client"). Data files are GeoJSON
// (RA/Dec in degrees) — see celestial-data/README.md for exactly where
// each one comes from (most reused as data-only from noctua's skymap
// package; star-proper-names.json fetched independently instead, per
// Gilles, 2026-09-29: "cherche plutot comment faire proprement avec les
// nouveaux catalogues").
//
// Two star depths ship today (celestial-data/): "light" (stars.6.json,
// mag <= 6.5, ~8.9k stars, naked-eye-ish, the default) and "deep"
// (stars.8.json, mag <= 8, ~41k stars) — per Gilles, 2026-09-29:
// "leger par defaut, mais parametrable ... prevoir tous les catalogues
// possibles des le depart". More depths can be added the same way later
// (another file + another entry in STAR_DEPTHS) without touching the
// loader itself.

export const STAR_DEPTHS = {
  light: { file: "stars.6.json", label: "Light (mag <= 6.5, ~8.9k stars)" },
  deep: { file: "stars.8.json", label: "Deep (mag <= 8, ~41k stars)" },
};

let dataBaseUrl = "celestial-data/";
export function setDataBaseUrl(url) { dataBaseUrl = url.endsWith("/") ? url : url + "/"; }

async function fetchJson(name) {
  const res = await fetch(dataBaseUrl + name);
  if (!res.ok) throw new Error(`sky-catalog: ${name} failed to load (${res.status})`);
  return res.json();
}

function starsFromGeoJson(geo, properNames) {
  return geo.features.map((f) => ({
    ra: f.geometry.coordinates[0],
    dec: f.geometry.coordinates[1],
    mag: f.properties.mag,
    bv: f.properties.bv !== undefined ? Number(f.properties.bv) : undefined,
    // f.id is the star's Hipparcos catalog number (HIP) - see
    // celestial-data/README.md - matched against star-proper-names.json's
    // own HIP-keyed lookup. Most stars have no common name (name stays
    // undefined); sky-engine.js's selection falls back to "Star" for those.
    name: properNames[f.id],
  }));
}

function constellationLinesFromGeoJson(geo) {
  // MultiLineString per constellation -> flat list of [ {ra,dec}, {ra,dec} ]
  // segments (what sky-engine.js actually draws, one moveTo/lineTo pair
  // each — no need to keep the per-constellation grouping for that).
  const segments = [];
  for (const f of geo.features) {
    for (const line of f.geometry.coordinates) {
      for (let i = 0; i < line.length - 1; i++) {
        segments.push([
          { ra: line[i][0], dec: line[i][1] },
          { ra: line[i + 1][0], dec: line[i + 1][1] },
        ]);
      }
    }
  }
  return segments;
}

function messierFromGeoJson(geo) {
  return geo.features.map((f) => ({
    ra: f.geometry.coordinates[0],
    dec: f.geometry.coordinates[1],
    name: f.properties.name,
    alt: f.properties.alt || "",
    desig: f.properties.desig || "",
    type: f.properties.type || "",
    mag: f.properties.mag,
  }));
}

/**
 * Loads the catalog: stars (depth: "light"|"deep"), constellation lines,
 * Messier objects. Returns {stars, constellationLines, messier}. Any one
 * catalog failing to load doesn't block the others (each caught
 * independently) - a partial sky (e.g. stars but no constellation lines)
 * is still useful, not a hard failure.
 */
export async function loadCatalog(depth) {
  const starFile = (STAR_DEPTHS[depth] || STAR_DEPTHS.light).file;
  const properNames = await fetchJson("star-proper-names.json").catch((e) => { console.error(e); return {}; });
  const [stars, constellationLines, messier] = await Promise.all([
    fetchJson(starFile).then((geo) => starsFromGeoJson(geo, properNames)).catch((e) => { console.error(e); return []; }),
    fetchJson("constellations.lines.json").then(constellationLinesFromGeoJson).catch((e) => { console.error(e); return []; }),
    fetchJson("messier.json").then(messierFromGeoJson).catch((e) => { console.error(e); return []; }),
  ]);
  return { stars, constellationLines, messier };
}
