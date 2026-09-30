// sky-catalog.js — loads the sky-dome's own client-side catalog (stars,
// constellation lines, Messier objects, Milky Way outline), independent
// of the server's own
// search/results catalog — see notes/NOTES-frontend-ostcore-planetarium.md
// ("la carte a son propre catalogue client"). Data files are GeoJSON
// (RA/Dec in degrees) — see celestial-data/README.md for exactly where
// each one comes from (most reused as data-only from noctua's skymap
// package; star-proper-names.json fetched independently instead, per
// Gilles, 2026-09-29: "cherche plutot comment faire proprement avec les
// nouveaux catalogues").
//
// Three star depths ship today (celestial-data/): "light" (stars.6.json,
// mag <= 6.5, ~8.9k stars, naked-eye-ish, the default), "deep"
// (stars.8.json, mag <= 8, ~41k stars), and "full" (stars.hip.json, every
// HYG entry with a Hipparcos number, ~118k stars) — per Gilles,
// 2026-09-29: "leger par defaut, mais parametrable ... prevoir tous les
// catalogues possibles des le depart", and 2026-10-03, pointing at
// astro2001's own ~871k-star full catalog as the scale to aim toward:
// "le grand saut : on augmente la densite, on passe la taille au dessus
// sur les catalogues". More depths can be added the same way later
// (another file + another entry in STAR_DEPTHS) without touching the
// loader itself.

export const STAR_DEPTHS = {
  light: { file: "stars.6.json", label: "Light (mag <= 6.5, ~8.9k stars)" },
  deep: { file: "stars.8.json", label: "Deep (mag <= 8, ~41k stars)" },
  full: { file: "stars.hip.json", label: "Full (Hipparcos, ~118k stars)" },
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

// MultiPolygon per feature (5 features) -> flat list of polygons, each a
// list of rings (ring[0] = its outer boundary, ring[1..] = holes, e.g. the
// Coalsack), each ring a list of {ra,dec} points. Kept grouped by polygon
// (not flattened further, unlike constellationLinesFromGeoJson()'s plain
// segment list) - Gilles, 2026-10-01: "tu peux griser legerement
// l'interieur de ces contours" needs the outer/hole grouping intact so
// sky-engine.js can fill each polygon with its own holes correctly
// (evenodd fill rule over every ring in the SAME path), rather than
// filling every ring independently and papering over the holes.
function milkyWayFromGeoJson(geo) {
  const polygons = [];
  for (const f of geo.features) {
    for (const polygon of f.geometry.coordinates) {
      polygons.push(polygon.map((ring) => ring.map(([ra, dec]) => ({ ra, dec }))));
    }
  }
  return polygons;
}

// `dim` (arcmin) is either "WxH" (elongated objects) or a single value
// (round ones, W=H) - checked directly against the actual file: only 2 of
// 110 entries have no dim at all (empty string), and there is NO
// orientation/position-angle field anywhere in this catalog (its only
// properties are name/desig/alt/type/mag/dim/cl) - Gilles, 2026-10-01:
// "est-ce que tu as une taille et une orientation ? si oui essaie d'en
// tenir compte" - taille oui, orientation non. sky-engine.js draws the
// resulting ellipse RA/Dec-axis-aligned for lack of a real PA (same
// "unrotated until real data says otherwise" fallback as
// sensorOrientationDeg elsewhere in this chantier).
function parseMessierDim(dim) {
  if (!dim) return { sizeWDeg: null, sizeHDeg: null };
  const [wArcmin, hArcmin] = String(dim).split(/x/i).map(Number);
  if (!(wArcmin > 0)) return { sizeWDeg: null, sizeHDeg: null };
  return { sizeWDeg: wArcmin / 60, sizeHDeg: (hArcmin > 0 ? hArcmin : wArcmin) / 60 };
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
    ...parseMessierDim(f.properties.dim),
  }));
}

/**
 * Loads the catalog: stars (depth: "light"|"deep"), constellation lines,
 * Messier objects, Milky Way outline. Returns {stars, constellationLines,
 * messier, milkyWay}. Any one catalog failing to load doesn't block the
 * others (each caught independently) - a partial sky (e.g. stars but no
 * constellation lines) is still useful, not a hard failure.
 */
export async function loadCatalog(depth) {
  const starFile = (STAR_DEPTHS[depth] || STAR_DEPTHS.light).file;
  const properNames = await fetchJson("star-proper-names.json").catch((e) => { console.error(e); return {}; });
  const [stars, constellationLines, messier, milkyWay] = await Promise.all([
    fetchJson(starFile).then((geo) => starsFromGeoJson(geo, properNames)).catch((e) => { console.error(e); return []; }),
    fetchJson("constellations.lines.json").then(constellationLinesFromGeoJson).catch((e) => { console.error(e); return []; }),
    fetchJson("messier.json").then(messierFromGeoJson).catch((e) => { console.error(e); return []; }),
    fetchJson("mw.json").then(milkyWayFromGeoJson).catch((e) => { console.error(e); return []; }),
  ]);
  return { stars, constellationLines, messier, milkyWay };
}
