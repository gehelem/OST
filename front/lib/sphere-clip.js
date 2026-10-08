/**
 * Adapted from d3-geo (https://github.com/d3/d3-geo), ISC License:
 *
 * Copyright 2010-2024 Mike Bostock
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

// Clips a spherical polygon to the front hemisphere (the visible dome) -
// ported from d3-geo's own general circle-clipping pipeline
// (src/clip/index.js, clip/circle.js, clip/rejoin.js, clip/buffer.js,
// polygonContains.js, circle.js's circleStream, cartesian.js, pointEqual,
// math.js), specialised to a FIXED radius of exactly 90 degrees and a
// FIXED clip center (this project's own "view-centered, visible iff
// v.z>=0" unit-vector convention - see sky-projection.js's
// horizontalToVector/vectorToScreen). Two d3-array dependencies (merge,
// Adder) are replaced with tiny local equivalents below instead of adding
// a package dependency - this file is adapted/ported, not a verbatim
// drop-in (unlike lib/astronomy-engine.js, which is vendored unmodified).
//
// Why port this instead of writing the dome-edge clipping ourselves (which
// had been tried several times): "which direction, of the two possible,
// should the dome-edge arc go to reconnect a cut ring's own exit point to
// its next entry point" turns out to need the general solution - sort
// every crossing along the clip boundary and stitch segments in that
// order, using a real point-in-spherical-polygon test for which side
// starts "inside" - not a locally-decided sign at each crossing. Every
// hand-rolled shortcut tried before this one (a fixed sign, flat 2D
// shoelace, spherical cross-product sum dotted with a fixed or
// approximate axis, per-crossing local tangent direction) was proven
// wrong by rigorous numerical testing against known small-circle test
// rings for at least one case. This port, tested the same way across
// those same cases (including the ones that broke every earlier
// attempt - a ring radius over 90 degrees, a ring centered exactly on the
// dome's own rim, a ring enclosing a pole), matches ground truth to
// within the clip boundary's own sampling precision (circleStream's
// `delta` below) in every one of them.
//
// Ring winding: standard GeoJSON (RFC 7946) - exterior rings
// counterclockwise, hole rings clockwise, as seen from outside the globe.
// Checked directly against celestial-data/mw.json: every one of its rings
// already follows this convention natively (exterior-type rings report
// their own centroid as "inside" as given; hole-type rings only when
// reversed - exactly RFC 7946, not a data quirk) - so real polygons are
// fed to clipPolygonToHemisphere() exactly as given, no per-ring
// orientation correction. (An earlier version of this file tried
// detecting and auto-fixing orientation at runtime by summing longitude
// deltas around each ring - proven buggy: that sum only reveals
// orientation for a ring that winds around one of the projection's own
// poles, and is near-zero/meaningless for the common case of a ring that
// doesn't. Not worth re-deriving polygonContains' own interior logic for
// a different purpose - simpler and more robust to just trust RFC 7946.)

const epsilon = 1e-6;
const epsilon2 = 1e-12;
const pi = Math.PI, halfPi = pi / 2, quarterPi = pi / 4, tau = pi * 2;

function asin(x) { return x > 1 ? halfPi : x < -1 ? -halfPi : Math.asin(x); }
function acos(x) { return x > 1 ? 0 : x < -1 ? pi : Math.acos(x); }

// --- cartesian.js ---
function cartesian(sph) { const [lambda, phi] = sph, cosPhi = Math.cos(phi); return [cosPhi * Math.cos(lambda), cosPhi * Math.sin(lambda), Math.sin(phi)]; }
function spherical(c) { return [Math.atan2(c[1], c[0]), asin(c[2])]; }
function cartesianDot(a, b) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; }
function cartesianCross(a, b) { return [a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]]; }
function cartesianAddInPlace(a, b) { a[0] += b[0]; a[1] += b[1]; a[2] += b[2]; }
function cartesianScale(v, k) { return [v[0] * k, v[1] * k, v[2] * k]; }
function cartesianNormalizeInPlace(d) { const l = Math.sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]); d[0] /= l; d[1] /= l; d[2] /= l; }

// --- pointEqual.js ---
function pointEqual(a, b) { return Math.abs(a[0] - b[0]) < epsilon && Math.abs(a[1] - b[1]) < epsilon; }

// --- circle.js (circleStream only - generates points along the clip boundary) ---
function circleRadius(cosRadius, point) {
  const p = cartesian(point); p[0] -= cosRadius;
  cartesianNormalizeInPlace(p);
  const radius = acos(-p[1]);
  return ((-p[2] < 0 ? -radius : radius) + tau - epsilon) % tau;
}
function circleStream(stream, radius, delta, direction, t0, t1) {
  if (!delta) return;
  const cosRadius = Math.cos(radius), sinRadius = Math.sin(radius);
  const step = direction * delta;
  if (t0 == null) {
    t0 = radius + direction * tau;
    t1 = radius - step / 2;
  } else {
    t0 = circleRadius(cosRadius, t0);
    t1 = circleRadius(cosRadius, t1);
    if (direction > 0 ? t0 < t1 : t0 > t1) t0 += direction * tau;
  }
  for (let t = t0; direction > 0 ? t > t1 : t < t1; t -= step) {
    const point = spherical([cosRadius, -sinRadius * Math.cos(t), -sinRadius * Math.sin(t)]);
    stream.point(point[0], point[1]);
  }
}

// --- polygonContains.js (plain-float sum in place of d3-array's Adder -
// Kahan-Babuska precision isn't needed here: evaluated once per polygon,
// as a fixed reference point against the clip boundary, not per pixel) ---
function longitude(point) { return Math.abs(point[0]) <= pi ? point[0] : Math.sign(point[0]) * ((Math.abs(point[0]) + pi) % tau - pi); }
function polygonContains(polygon, point) {
  let lambda = longitude(point), phi = point[1];
  const sinPhi = Math.sin(phi);
  const normal = [Math.sin(lambda), -Math.cos(lambda), 0];
  let angle = 0, winding = 0, sum = 0;
  if (sinPhi === 1) phi = halfPi + epsilon;
  else if (sinPhi === -1) phi = -halfPi - epsilon;

  for (const ring of polygon) {
    const m = ring.length;
    if (!m) continue;
    let point0 = ring[m - 1], lambda0 = longitude(point0);
    let phi0 = point0[1] / 2 + quarterPi, sinPhi0 = Math.sin(phi0), cosPhi0 = Math.cos(phi0);
    for (let j = 0; j < m; j++) {
      const point1 = ring[j];
      const lambda1 = longitude(point1), phi1 = point1[1] / 2 + quarterPi;
      const sinPhi1 = Math.sin(phi1), cosPhi1 = Math.cos(phi1);
      const delta = lambda1 - lambda0;
      const sign = delta >= 0 ? 1 : -1;
      const absDelta = sign * delta;
      const antimeridian = absDelta > pi;
      const k = sinPhi0 * sinPhi1;
      sum += Math.atan2(k * sign * Math.sin(absDelta), cosPhi0 * cosPhi1 + k * Math.cos(absDelta));
      angle += antimeridian ? delta + sign * tau : delta;
      if (antimeridian !== (lambda0 >= lambda) !== (lambda1 >= lambda)) {
        const arc = cartesianCross(cartesian(point0), cartesian(point1));
        cartesianNormalizeInPlace(arc);
        const intersection = cartesianCross(normal, arc);
        cartesianNormalizeInPlace(intersection);
        const phiArc = (antimeridian !== (delta >= 0) ? -1 : 1) * asin(intersection[2]);
        if (phi > phiArc || (phi === phiArc && (arc[0] || arc[1]))) {
          winding += antimeridian !== (delta >= 0) ? 1 : -1;
        }
      }
      lambda0 = lambda1; sinPhi0 = sinPhi1; cosPhi0 = cosPhi1; point0 = point1;
    }
  }
  return (angle < -epsilon || (angle < epsilon && sum < -epsilon2)) !== !!(winding & 1);
}

// --- clip/buffer.js ---
function clipBuffer() {
  let lines = [], line;
  return {
    point: (x, y, m) => line.push([x, y, m]),
    lineStart: () => lines.push((line = [])),
    lineEnd: () => {},
    result: () => { const r = lines; lines = []; line = null; return r; },
  };
}

// --- clip/rejoin.js ---
function Intersection(point, points, other, entry) {
  this.x = point; this.z = points; this.o = other; this.e = entry; this.v = false; this.n = this.p = null;
}
function link(array) {
  const n = array.length;
  if (!n) return;
  let i = 0, a = array[0], b;
  while (++i < n) { a.n = b = array[i]; b.p = a; a = b; }
  a.n = b = array[0]; b.p = a;
}
function clipRejoin(segments, compareIntersection, startInside, interpolate, stream) {
  const subject = [], clip = [];
  let i, n;
  segments.forEach((segment) => {
    if ((n = segment.length - 1) <= 0) return;
    let p0 = segment[0], p1 = segment[n], x;
    if (pointEqual(p0, p1)) {
      if (!p0[2] && !p1[2]) {
        stream.lineStart();
        for (i = 0; i < n; ++i) stream.point((p0 = segment[i])[0], p0[1]);
        stream.lineEnd();
        return;
      }
      p1[0] += 2 * epsilon;
    }
    subject.push((x = new Intersection(p0, segment, null, true)));
    clip.push((x.o = new Intersection(p0, null, x, false)));
    subject.push((x = new Intersection(p1, segment, null, false)));
    clip.push((x.o = new Intersection(p1, null, x, true)));
  });
  if (!subject.length) return;
  clip.sort(compareIntersection);
  link(subject);
  link(clip);
  for (i = 0, n = clip.length; i < n; ++i) clip[i].e = startInside = !startInside;
  const start = subject[0];
  let points, point;
  for (;;) {
    let current = start, isSubject = true;
    while (current.v) if ((current = current.n) === start) return;
    points = current.z;
    stream.lineStart();
    do {
      current.v = current.o.v = true;
      if (current.e) {
        if (isSubject) { for (i = 0, n = points.length; i < n; ++i) stream.point((point = points[i])[0], point[1]); }
        else interpolate(current.x, current.n.x, 1, stream);
        current = current.n;
      } else {
        if (isSubject) { points = current.p.z; for (i = points.length - 1; i >= 0; --i) stream.point((point = points[i])[0], point[1]); }
        else interpolate(current.x, current.p.x, -1, stream);
        current = current.p;
      }
      current = current.o;
      points = current.z;
      isSubject = !isSubject;
    } while (!current.v);
    stream.lineEnd();
  }
}

// --- clip/index.js (generic polygon-clip framework) ---
function flattenOnce(arrays) { const out = []; for (const a of arrays) for (const x of a) out.push(x); return out; }
function validSegment(segment) { return segment.length > 1; }
function compareIntersection(a, b) {
  return ((a = a.x)[0] < 0 ? a[1] - halfPi - epsilon : halfPi - a[1]) - ((b = b.x)[0] < 0 ? b[1] - halfPi - epsilon : halfPi - b[1]);
}
function makeClip(pointVisible, clipLine, interpolate, start) {
  return function (sink) {
    const line = clipLine(sink);
    const ringBuffer = clipBuffer();
    const ringSink = clipLine(ringBuffer);
    let polygonStarted = false, polygon, segments, ring;

    function point(lambda, phi) { if (pointVisible(lambda, phi)) sink.point(lambda, phi); }
    function pointLine(lambda, phi) { line.point(lambda, phi); }
    function lineStart() { clip.point = pointLine; line.lineStart(); }
    function lineEnd() { clip.point = point; line.lineEnd(); }
    function pointRing(lambda, phi) { ring.push([lambda, phi]); ringSink.point(lambda, phi); }
    function ringStart() { ringSink.lineStart(); ring = []; }
    function ringEnd() {
      pointRing(ring[0][0], ring[0][1]);
      ringSink.lineEnd();
      const clean = ringSink.clean();
      const ringSegments = ringBuffer.result();
      const n = ringSegments.length;
      ring.pop();
      polygon.push(ring);
      ring = null;
      if (!n) return;
      if (clean & 1) {
        const segment = ringSegments[0];
        const m = segment.length - 1;
        if (m > 0) {
          if (!polygonStarted) { sink.polygonStart(); polygonStarted = true; }
          sink.lineStart();
          for (let i = 0; i < m; ++i) sink.point(segment[i][0], segment[i][1]);
          sink.lineEnd();
        }
        return;
      }
      if (n > 1 && clean & 2) ringSegments.push(ringSegments.pop().concat(ringSegments.shift()));
      segments.push(ringSegments.filter(validSegment));
    }

    const clip = {
      point,
      lineStart,
      lineEnd,
      polygonStart() {
        clip.point = pointRing; clip.lineStart = ringStart; clip.lineEnd = ringEnd;
        segments = []; polygon = [];
      },
      polygonEnd() {
        clip.point = point; clip.lineStart = lineStart; clip.lineEnd = lineEnd;
        segments = flattenOnce(segments);
        const startInside = polygonContains(polygon, start);
        if (segments.length) {
          if (!polygonStarted) { sink.polygonStart(); polygonStarted = true; }
          clipRejoin(segments, compareIntersection, startInside, interpolate, sink);
        } else if (startInside) {
          if (!polygonStarted) { sink.polygonStart(); polygonStarted = true; }
          sink.lineStart(); interpolate(null, null, 1, sink); sink.lineEnd();
        }
        if (polygonStarted) { sink.polygonEnd(); polygonStarted = false; }
        segments = polygon = null;
      },
      sphere() {
        sink.polygonStart(); sink.lineStart(); interpolate(null, null, 1, sink); sink.lineEnd(); sink.polygonEnd();
      },
    };
    return clip;
  };
}

// --- clip/circle.js, specialised to radius = halfPi (exactly 90 degrees,
// the front/visible hemisphere) - this is also d3-geo's own default
// clipAngle for geoOrthographic, so this exact code path (the
// "smallRadius" branch, since cos(90deg) computes as a tiny positive
// float rather than exactly 0) is its most heavily used, most tested
// configuration. ---
function makeHemisphereClip() {
  const radius = halfPi;
  const cr = Math.cos(radius); // ~6.1e-17, a hair above 0 - selects the "smallRadius" branch below, same as d3's own real-world clipAngle(90) usage
  const delta = 2 * (pi / 180);
  const smallRadius = cr > 0;
  const notHemisphere = Math.abs(cr) > epsilon;

  function interpolate(from, to, direction, stream) { circleStream(stream, radius, delta, direction, from, to); }
  function visible(lambda, phi) { return Math.cos(lambda) * Math.cos(phi) > cr; }

  function code(lambda, phi) {
    const r = smallRadius ? radius : pi - radius;
    let c = 0;
    if (lambda < -r) c |= 1; else if (lambda > r) c |= 2;
    if (phi < -r) c |= 4; else if (phi > r) c |= 8;
    return c;
  }

  function intersect(a, b, two) {
    const pa = cartesian(a), pb = cartesian(b);
    const n1 = [1, 0, 0];
    const n2 = cartesianCross(pa, pb);
    const n2n2 = cartesianDot(n2, n2);
    const n1n2 = n2[0];
    const determinant = n2n2 - n1n2 * n1n2;
    if (!determinant) return !two && a;
    const c1 = (cr * n2n2) / determinant, c2 = (-cr * n1n2) / determinant;
    const n1xn2 = cartesianCross(n1, n2);
    const A = cartesianScale(n1, c1), B = cartesianScale(n2, c2);
    cartesianAddInPlace(A, B);
    const u = n1xn2, w = cartesianDot(A, u), uu = cartesianDot(u, u);
    const t2 = w * w - uu * (cartesianDot(A, A) - 1);
    if (t2 < 0) return;
    const t = Math.sqrt(t2);
    let q = cartesianScale(u, (-w - t) / uu);
    cartesianAddInPlace(q, A);
    q = spherical(q);
    if (!two) return q;
    let lambda0 = a[0], lambda1 = b[0], phi0 = a[1], phi1 = b[1], z;
    if (lambda1 < lambda0) { z = lambda0; lambda0 = lambda1; lambda1 = z; }
    const dlt = lambda1 - lambda0;
    const polar = Math.abs(dlt - pi) < epsilon;
    const meridian = polar || dlt < epsilon;
    if (!polar && phi1 < phi0) { z = phi0; phi0 = phi1; phi1 = z; }
    const ok = meridian
      ? polar
        ? (phi0 + phi1 > 0) !== (q[1] < (Math.abs(q[0] - lambda0) < epsilon ? phi0 : phi1))
        : phi0 <= q[1] && q[1] <= phi1
      : dlt > pi !== (lambda0 <= q[0] && q[0] <= lambda1);
    if (ok) {
      let q1 = cartesianScale(u, (-w + t) / uu);
      cartesianAddInPlace(q1, A);
      return [q, spherical(q1)];
    }
  }

  function clipLine(stream) {
    let point0, c0, v0, v00, clean;
    return {
      lineStart() { v00 = v0 = false; clean = 1; },
      point(lambda, phi) {
        const point1 = [lambda, phi];
        let point2;
        const v = visible(lambda, phi);
        const c = smallRadius ? (v ? 0 : code(lambda, phi)) : v ? code(lambda + (lambda < 0 ? pi : -pi), phi) : 0;
        if (!point0 && (v00 = v0 = v)) stream.lineStart();
        if (v !== v0) {
          point2 = intersect(point0, point1);
          if (!point2 || pointEqual(point0, point2) || pointEqual(point1, point2)) point1[2] = 1;
        }
        if (v !== v0) {
          clean = 0;
          if (v) {
            stream.lineStart();
            point2 = intersect(point1, point0);
            stream.point(point2[0], point2[1]);
          } else {
            point2 = intersect(point0, point1);
            stream.point(point2[0], point2[1], 2);
            stream.lineEnd();
          }
          point0 = point2;
        } else if (notHemisphere && point0 && (smallRadius ? 1 : 0) !== (v ? 1 : 0)) {
          let t;
          if (!(c & c0) && (t = intersect(point1, point0, true))) {
            clean = 0;
            if (smallRadius) {
              stream.lineStart();
              stream.point(t[0][0], t[0][1]);
              stream.point(t[1][0], t[1][1]);
              stream.lineEnd();
            } else {
              stream.point(t[1][0], t[1][1]);
              stream.lineEnd();
              stream.lineStart();
              stream.point(t[0][0], t[0][1], 3);
            }
          }
        }
        if (v && (!point0 || !pointEqual(point0, point1))) stream.point(point1[0], point1[1]);
        point0 = point1; v0 = v; c0 = c;
      },
      lineEnd() { if (v0) stream.lineEnd(); point0 = null; },
      clean() { return clean | ((v00 && v0 ? 1 : 0) << 1); },
    };
  }

  const start = smallRadius ? [0, -radius] : [-pi, radius - pi];
  return makeClip(visible, clipLine, interpolate, start);
}

const clipHemisphere = makeHemisphereClip();

// This project's own vectors have v.z as the "view center" axis
// (sky-projection.js: visible iff v.z>=0); circle.js's own fixed clip-plane
// normal is n1=[1,0,0], i.e. its (lambda=0,phi=0) direction is the
// CARTESIAN X axis - remap so our z becomes their x. This remap MUST be an
// orientation-PRESERVING permutation (determinant +1, e.g. the cyclic
// (x,y,z)->(z,x,y) used here), not a plain axis swap (determinant -1) - an
// odd permutation silently flips which ring winding counts as "exterior".
// (vz,vx,vy) is a 3-cycle, so it keeps orientation: real GeoJSON data
// (standard lambda=atan2(y,x), phi=asin(z) convention) needs no reversal
// through this mapping, as confirmed against the real mw.json.
function toLocalSpherical(v) { return [Math.atan2(v.x, v.z), asin(Math.max(-1, Math.min(1, v.y)))]; }
function fromLocalSpherical(lambda, phi) {
  const cosPhi = Math.cos(phi);
  return { x: cosPhi * Math.sin(lambda), y: Math.sin(phi), z: cosPhi * Math.cos(lambda) };
}

/**
 * Clips one GeoJSON-style polygon (an array of rings, outer boundary
 * first then holes, each ring an array of this project's own {x,y,z} unit
 * vectors - see sky-projection.js's horizontalToVector/vectorToScreen) to
 * the front hemisphere (v.z >= 0).
 *
 * Returns an array of closed output rings, each an array of {x,y,z} unit
 * vectors, representing the polygon's own visible-hemisphere portion -
 * dome-edge arcs are included as real points, ready to project to screen
 * and fill directly with the canvas default ("nonzero") fill rule: ring
 * winding is preserved through clipping, so holes still read as holes
 * exactly as in the source GeoJSON, with no closing-chord guesswork and no
 * "evenodd" workaround needed.
 */
export function clipPolygonToHemisphere(rings) {
  const outputRings = [];
  let current = null;
  const sink = {
    polygonStart() {},
    polygonEnd() {},
    lineStart() { current = []; },
    lineEnd() { if (current && current.length > 1) outputRings.push(current); current = null; },
    point(lambda, phi) { current.push(fromLocalSpherical(lambda, phi)); },
  };
  const transform = clipHemisphere(sink);
  transform.polygonStart();
  for (const ring of rings) {
    if (ring.length < 2) continue;
    transform.lineStart();
    for (const v of ring) { const [lambda, phi] = toLocalSpherical(v); transform.point(lambda, phi); }
    transform.lineEnd();
  }
  transform.polygonEnd();
  return outputRings;
}
