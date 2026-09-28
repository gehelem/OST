// imagelayer.js — full-viewport background image + fullscreen viewer,
// ported from ostnoctua's own imagelayer.js (OstImage.background/viewer)
// onto core/'s client API — Gilles, 2026-09-29: "sur le template guider
// affiche l'image derriere en ecran de fond, comme dans ostnoctua, en
// redimensionnant mais en preservant l'aspect ratio / un clic sur l'image
// doit l'ouvrir en plein ecran ... avec un bouton pour la fermer. On
// navigue dans l'image avec la souris et on peut zoomer dezoomer dedans".
// Generic over any module/property/element (not hardcoded to guider's own
// "image"/"image") so any future template can reuse it the same way
// ostnoctua's own templates (guider, capture, ...) share one helper.
//
//   setBgLayer(node)                                          once, at boot
//   showBackgroundImage(client, moduleName, propName, elName) -> {dispose}
//   openImageViewer(src)                                      standalone use

import { el, icon } from "./dom.js";
import { getMediaBaseUrl } from "./render.js";
import { mediaUrl as buildMediaUrl } from "../core/index.js";

let bgLayer = null;
export function setBgLayer(node) { bgLayer = node; }

function jpegSrc(elDef) {
  const v = elDef && elDef.value;
  const path = v && (v.urljpeg || v.urlthumbnail);
  if (!path) return null;
  return buildMediaUrl(getMediaBaseUrl(), path) + "?t=" + Date.now();
}

/**
 * Fills the shared #bg-layer with one module's image element, letterboxed
 * (CSS object-fit: contain — resized, aspect ratio always preserved, see
 * panels.css's #bg-image). A click opens the fullscreen viewer. Self-
 * subscribes to the element's own property (topics.property — the
 * fine-grained topic ea/ee/ps/ap traffic actually fires on) so it stays
 * live independent of whatever else is rebuilding around it. Returns
 * {dispose}; the caller must call it when the view changes away (same
 * convention as every other subscription in this codebase).
 */
export function showBackgroundImage(client, moduleName, propName, elName) {
  if (!bgLayer) return { dispose: () => {} };
  bgLayer.classList.add("img-active");
  bgLayer.innerHTML = "";
  const img = el("img", { id: "bg-image", alt: "" });
  bgLayer.appendChild(img);
  let lastSrc = null;
  const onClick = () => { if (lastSrc) openImageViewer(lastSrc); };
  img.addEventListener("click", onClick);

  const unsub = client.subscribe(client.topics.property(moduleName, propName), (prop) => {
    const src = jpegSrc(prop && prop.elements && prop.elements[elName]);
    if (src) { img.src = src; lastSrc = src; }
  });

  return {
    dispose: () => {
      unsub();
      img.removeEventListener("click", onClick);
      bgLayer.innerHTML = "";
      bgLayer.classList.remove("img-active");
    },
  };
}

// ------------------------------------------------------------------ viewer
// One shared overlay (built lazily, reused across opens) — wheel zooms
// toward the cursor, drag pans, Esc/backdrop click/close button closes.
let viewerCtl = null;
function buildViewer() {
  const root = el("div", { class: "img-viewer" });
  root.hidden = true;
  const stage = el("div", { class: "img-viewer-stage" });
  const img = el("img", { class: "img-viewer-img" });
  stage.appendChild(img);
  const close = el("button", { class: "img-viewer-close", type: "button", title: "Close" }, [icon("close")]);
  root.appendChild(stage);
  root.appendChild(close);
  document.body.appendChild(root);

  let scale = 1, tx = 0, ty = 0;
  const apply = () => { img.style.transform = `translate(${tx}px,${ty}px) scale(${scale})`; };

  function fit() {
    const vw = root.clientWidth, vh = root.clientHeight;
    const iw = img.naturalWidth || vw, ih = img.naturalHeight || vh;
    scale = (Math.min(vw / iw, vh / ih) * 0.95) || 1;
    tx = (vw - iw * scale) / 2;
    ty = (vh - ih * scale) / 2;
    apply();
  }
  img.addEventListener("load", fit);

  stage.addEventListener("wheel", (ev) => {
    ev.preventDefault();
    const r = img.getBoundingClientRect();
    const cx = ev.clientX - r.left, cy = ev.clientY - r.top; // point under the cursor, in the (scaled) image's own coordinates
    const f = ev.deltaY < 0 ? 1.15 : 1 / 1.15;
    const ns = Math.max(0.1, Math.min(40, scale * f));
    const k = ns / scale;
    tx -= cx * (k - 1); // keep that same point fixed under the cursor
    ty -= cy * (k - 1);
    scale = ns;
    apply();
  }, { passive: false });

  let drag = null;
  stage.addEventListener("mousedown", (ev) => {
    if (ev.button !== 0) return;
    drag = { x: ev.clientX, y: ev.clientY, tx, ty };
    stage.classList.add("grabbing");
    ev.preventDefault();
  });
  window.addEventListener("mousemove", (ev) => {
    if (!drag) return;
    tx = drag.tx + (ev.clientX - drag.x);
    ty = drag.ty + (ev.clientY - drag.y);
    apply();
  });
  window.addEventListener("mouseup", () => { drag = null; stage.classList.remove("grabbing"); });

  const onKey = (ev) => { if (ev.key === "Escape") hide(); };
  function hide() {
    root.hidden = true;
    document.removeEventListener("keydown", onKey);
  }
  close.addEventListener("click", hide);
  root.addEventListener("mousedown", (ev) => { if (ev.target === root) hide(); });

  return {
    open(src) {
      root.hidden = false;
      document.addEventListener("keydown", onKey);
      if (img.src !== src) img.src = src; else fit();
    },
  };
}

export function openImageViewer(src) {
  if (!src) return;
  if (!viewerCtl) viewerCtl = buildViewer();
  viewerCtl.open(src);
}
