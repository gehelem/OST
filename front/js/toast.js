// js/toast.js — transient notification ("toast"/"snackbar" in other UI
// kits - the exact Angular Material name Gilles was trying to recall)
// for log messages that deserve more visibility than the log panel alone
// gives them. Gilles, 2026-10-10, after a module crash (issue #234) only
// showed up as a log entry nobody would necessarily notice: "ça permettra
// de rendre ce type d'évènement plus visible pour l'utilisateur" - a box
// that appears then disappears on its own after a second or two.
//
// Currently triggered only for critical-level log entries (app.js's own
// topics.logs() subscription) - exported generically (any level) in case
// a future use wants to toast something else.

import { el } from "./dom.js";

// Same server-messages.en.md "l" field vocabulary (0 Debug..4 Critical)
// everywhere else in this app already uses (LOG_LEVELS in views.js).
const LEVEL_CLASS = ["toast-debug", "toast-info", "toast-warning", "toast-error", "toast-critical"];
const AUTO_DISMISS_MS = 2500;

let stackEl = null;
function ensureStack() {
  if (stackEl) return stackEl;
  stackEl = el("div", { id: "toast-stack" });
  document.body.appendChild(stackEl);
  return stackEl;
}

/** Shows one transient notification, auto-removed after a couple of
 * seconds. `level` is the log "l" field - out-of-range/undefined falls
 * back to the neutral "info" styling rather than throwing. */
export function showToast(message, level) {
  const stack = ensureStack();
  const levelClass = LEVEL_CLASS[level] || LEVEL_CLASS[1];
  const node = el("div", { class: `toast glass ${levelClass}`, text: message });
  stack.appendChild(node);
  // Two nested rAFs, not one - the "in" class has to be applied on a
  // frame after the node is already in the DOM with its initial
  // (invisible) style actually painted, or the browser coalesces both
  // states into one and the transition never visibly animates.
  requestAnimationFrame(() => requestAnimationFrame(() => node.classList.add("toast-in")));
  setTimeout(() => {
    node.classList.remove("toast-in");
    node.addEventListener("transitionend", () => node.remove(), { once: true });
    // Safety net in case transitionend never fires (e.g. a backgrounded
    // tab, where rAF/transitions can stall indefinitely) - the node must
    // not linger forever either way.
    setTimeout(() => node.remove(), 500);
  }, AUTO_DISMISS_MS);
}
