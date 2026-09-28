// chart.js — small dependency-free canvas chart renderer for graph properties
// (hasGraph true). No CDN charting library is used, per the "no internet
// dependency" client architecture rule.
//
// Supports graphType XY, DY, SXY, SDY generically (graphParams roles) and a
// minimal, non-generic rendering for PHD (see structure.en.md's warning that
// PHD needs a dedicated implementation - this is a simplified one: RA/DE vs
// D on one axis, not the full drift+SNR+RMS PHD2-style layout).

const PALETTE = ["#4fc3f7", "#ff8a65", "#81c784", "#ba68c8", "#ffd54f", "#4db6ac", "#f06292", "#a1887f"];

function colorFor(seriesName, graphColors, index) {
  if (graphColors && graphColors[seriesName]) {
    const c = graphColors[seriesName];
    return `rgb(${c.R},${c.G},${c.B})`;
  }
  return PALETTE[index % PALETTE.length];
}

function toTimeValue(dateElement) {
  if (!dateElement) return 0;
  if (typeof dateElement === "number") return dateElement;
  const { year = 1970, month = 1, day = 1, hh = 0, mm = 0, ss = 0, ms = 0 } = dateElement;
  return new Date(year, (month || 1) - 1, day || 1, hh, mm, ss, ms).getTime();
}

export function renderGraph(canvas, property) {
  const ctx = canvas.getContext("2d");
  const w = canvas.width, h = canvas.height;
  ctx.clearRect(0, 0, w, h);
  ctx.font = "11px sans-serif";
  const rows = property.rows || [];
  const params = property.graphParams || {};
  const type = property.graphType;

  if (rows.length === 0) {
    ctx.fillStyle = "#888";
    ctx.fillText("No data yet", 10, 20);
    return;
  }

  if (type === "PHD") {
    renderPhdMinimal(ctx, w, h, rows, params);
    return;
  }

  let series; // Map<name, {x[],y[]}>
  let xIsDate = false;
  if (type === "SXY" || type === "SDY") {
    series = new Map();
    xIsDate = type === "SDY";
    for (const row of rows) {
      const s = String(row[params.S]);
      if (!series.has(s)) series.set(s, { x: [], y: [] });
      const bucket = series.get(s);
      bucket.x.push(xIsDate ? toTimeValue(row[params.D]) : Number(row[params.X]));
      bucket.y.push(Number(row[params.Y]));
    }
  } else {
    xIsDate = type === "DY";
    series = new Map([["", { x: [], y: [] }]]);
    for (const row of rows) {
      const bucket = series.get("");
      bucket.x.push(xIsDate ? toTimeValue(row[params.D]) : Number(row[params.X]));
      bucket.y.push(Number(row[params.Y]));
    }
  }

  let xmin = Infinity, xmax = -Infinity, ymin = Infinity, ymax = -Infinity;
  for (const { x, y } of series.values()) {
    for (const v of x) { if (v < xmin) xmin = v; if (v > xmax) xmax = v; }
    for (const v of y) { if (v < ymin) ymin = v; if (v > ymax) ymax = v; }
  }
  if (type === "XY") {
    if (params.Xmin !== undefined && rows[0][params.Xmin] !== undefined) xmin = Number(rows[0][params.Xmin]);
    if (params.Xmax !== undefined && rows[0][params.Xmax] !== undefined) xmax = Number(rows[0][params.Xmax]);
    if (params.Ymin !== undefined && rows[0][params.Ymin] !== undefined) ymin = Number(rows[0][params.Ymin]);
    if (params.Ymax !== undefined && rows[0][params.Ymax] !== undefined) ymax = Number(rows[0][params.Ymax]);
  }
  if (xmin === xmax) { xmin -= 1; xmax += 1; }
  if (ymin === ymax) { ymin -= 1; ymax += 1; }
  const padL = 46, padB = 22, padT = 10, padR = 10;
  const plotW = w - padL - padR, plotH = h - padT - padB;
  const sx = (v) => padL + ((v - xmin) / (xmax - xmin)) * plotW;
  const sy = (v) => padT + plotH - ((v - ymin) / (ymax - ymin)) * plotH;

  // axes
  ctx.strokeStyle = "#555";
  ctx.beginPath();
  ctx.moveTo(padL, padT); ctx.lineTo(padL, padT + plotH); ctx.lineTo(padL + plotW, padT + plotH);
  ctx.stroke();
  ctx.fillStyle = "#aaa";
  ctx.fillText(ymax.toFixed(2), 2, padT + 8);
  ctx.fillText(ymin.toFixed(2), 2, padT + plotH);
  ctx.fillText(xIsDate ? new Date(xmin).toLocaleTimeString() : xmin.toFixed(2), padL, h - 6);
  ctx.fillText(xIsDate ? new Date(xmax).toLocaleTimeString() : xmax.toFixed(2), padL + plotW - 40, h - 6);

  let i = 0;
  for (const [name, { x, y }] of series.entries()) {
    const color = colorFor(name, params.graphColors, i++);
    ctx.strokeStyle = color;
    ctx.fillStyle = color;
    ctx.lineWidth = 1.5;
    ctx.beginPath();
    for (let k = 0; k < x.length; k++) {
      const px = sx(x[k]), py = sy(y[k]);
      if (k === 0) ctx.moveTo(px, py); else ctx.lineTo(px, py);
    }
    ctx.stroke();
    for (let k = 0; k < x.length; k++) {
      ctx.beginPath();
      ctx.arc(sx(x[k]), sy(y[k]), 2, 0, Math.PI * 2);
      ctx.fill();
    }
    if (name) {
      ctx.fillText(name, padL + 4 + i * 60, padT + 10);
    }
  }
}

function renderPhdMinimal(ctx, w, h, rows, params) {
  // Simplified: plot pRA and pDE (guide pulses / drift) against D on one
  // shared axis. This is NOT the full PHD2-style multi-panel graph
  // (drift + SNR + RMS) the docs describe - see FINDINGS.md.
  const xs = rows.map((r) => toTimeValue(r[params.D]));
  const ra = rows.map((r) => Number(r[params.RA] ?? r[params.pRA] ?? 0));
  const de = rows.map((r) => Number(r[params.DE] ?? r[params.pDE] ?? 0));
  let ymin = Math.min(...ra, ...de), ymax = Math.max(...ra, ...de);
  if (ymin === ymax) { ymin -= 1; ymax += 1; }
  let xmin = Math.min(...xs), xmax = Math.max(...xs);
  if (xmin === xmax) { xmin -= 1; xmax += 1; }
  const padL = 46, padB = 22, padT = 10, padR = 10;
  const plotW = w - padL - padR, plotH = h - padT - padB;
  const sx = (v) => padL + ((v - xmin) / (xmax - xmin)) * plotW;
  const sy = (v) => padT + plotH - ((v - ymin) / (ymax - ymin)) * plotH;
  ctx.strokeStyle = "#555";
  ctx.beginPath();
  ctx.moveTo(padL, padT); ctx.lineTo(padL, padT + plotH); ctx.lineTo(padL + plotW, padT + plotH);
  ctx.stroke();
  function plot(arr, color, label, idx) {
    ctx.strokeStyle = color; ctx.fillStyle = color;
    ctx.beginPath();
    for (let k = 0; k < arr.length; k++) {
      const px = sx(xs[k]), py = sy(arr[k]);
      if (k === 0) ctx.moveTo(px, py); else ctx.lineTo(px, py);
    }
    ctx.stroke();
    ctx.fillText(label, padL + 4 + idx * 40, padT + 10);
  }
  plot(ra, "#4fc3f7", "RA", 0);
  plot(de, "#ff8a65", "DE", 1);
}
