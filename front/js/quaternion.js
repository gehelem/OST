// quaternion.js — small, generic unit-quaternion utility (rotations in 3D).
// Self-contained, no dependency on sky-*.js — used by sky-engine.js for the
// dome's trackball drag (see that file's own header for why a quaternion,
// not naive az/alt accumulation: the latter is exactly what made skymap's
// old drag "blow up near the zenith", per
// notes/NOTES-frontend-ostcore-planetarium.md).
//
// A quaternion here is a plain {w, x, y, z} object (w = scalar part).

/** The "no rotation" quaternion. */
export function identity() {
  return { w: 1, x: 0, y: 0, z: 0 };
}

/** Hamilton product a*b (apply b first, then a — same convention as 3x3
 * matrix multiplication composing transforms). */
export function multiply(a, b) {
  return {
    w: a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,
    x: a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
    y: a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
    z: a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
  };
}

export function conjugate(q) {
  return { w: q.w, x: -q.x, y: -q.y, z: -q.z };
}

function normalizeVec3(v) {
  const len = Math.hypot(v.x, v.y, v.z) || 1;
  return { x: v.x / len, y: v.y / len, z: v.z / len };
}

/** Rotates a unit vector {x,y,z} by quaternion q. */
export function rotateVector(q, v) {
  // v' = q * v * q^-1, treating v as a pure quaternion (w=0). Expanded
  // directly (cheaper than two full quaternion multiplies).
  const { w, x: qx, y: qy, z: qz } = q;
  const { x: vx, y: vy, z: vz } = v;
  // t = 2 * cross(q.xyz, v)
  const tx = 2 * (qy * vz - qz * vy);
  const ty = 2 * (qz * vx - qx * vz);
  const tz = 2 * (qx * vy - qy * vx);
  // v' = v + w*t + cross(q.xyz, t)
  return {
    x: vx + w * tx + (qy * tz - qz * ty),
    y: vy + w * ty + (qz * tx - qx * tz),
    z: vz + w * tz + (qx * ty - qy * tx),
  };
}

/**
 * The minimal-angle rotation that takes unit vector `a` to unit vector `b`
 * (the core of an arcball/trackball drag: "make the point under the
 * cursor at dragstart track the cursor exactly"). Returns the identity
 * rotation for a==b, and a stable 180-degree rotation (about any axis
 * perpendicular to a) for a==-b, since a==-b has no unique rotation axis.
 */
export function fromTwoVectors(a, b) {
  a = normalizeVec3(a);
  b = normalizeVec3(b);
  const dot = a.x * b.x + a.y * b.y + a.z * b.z;
  if (dot > 0.999999) return identity();
  if (dot < -0.999999) {
    // 180 degrees: pick any axis perpendicular to a.
    let axis = { x: 1, y: 0, z: 0 };
    if (Math.abs(a.x) > 0.9) axis = { x: 0, y: 1, z: 0 };
    const perp = normalizeVec3({
      x: axis.y * a.z - axis.z * a.y,
      y: axis.z * a.x - axis.x * a.z,
      z: axis.x * a.y - axis.y * a.x,
    });
    return { w: 0, x: perp.x, y: perp.y, z: perp.z };
  }
  const cross = { x: a.y * b.z - a.z * b.y, y: a.z * b.x - a.x * b.z, z: a.x * b.y - a.y * b.x };
  const q = { w: 1 + dot, x: cross.x, y: cross.y, z: cross.z };
  const len = Math.hypot(q.w, q.x, q.y, q.z) || 1;
  return { w: q.w / len, x: q.x / len, y: q.y / len, z: q.z / len };
}
