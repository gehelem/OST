// core/index.js — public entry point. See ../README.md for the full API
// reference; this file is intentionally the only one a UI should need to
// import from `core/` (the individual classes are still exported for
// advanced use, e.g. a debug console that wants `subscribeRaw`).

import { Datastore, topics } from "./datastore.js";
import { Connection } from "./connection.js";

export { Datastore, topics, Connection };

/**
 * Derives the WebSocket URL and the media (image/video/FITS) base URL from
 * the page's own location, per structure.en.md's "Finding the server"
 * table. Only meaningful for a web client served by the OST server itself;
 * a non-web client has no page URL to derive anything from and must get
 * the host some other way (mDNS, manual entry — see the same doc section).
 */
export function deriveDefaultUrls(loc) {
  loc = loc || (typeof window !== "undefined" ? window.location : null);
  if (!loc) return { wsUrl: "ws://127.0.0.1:9624", mediaUrl: "http://127.0.0.1/ostmedia/" };
  if (loc.protocol === "https:") {
    return { wsUrl: `wss://${loc.host}/ws/`, mediaUrl: `https://${loc.host}/ostmedia/` };
  }
  if (loc.protocol === "http:") {
    return { wsUrl: `ws://${loc.hostname}:9624`, mediaUrl: `http://${loc.hostname}/ostmedia/` };
  }
  // file:// or anything else — no page origin to derive from.
  return { wsUrl: "ws://127.0.0.1:9624", mediaUrl: "http://127.0.0.1/ostmedia/" };
}

/** Builds a full media URL from a relative path as sent in the wire data (img/video/file paths — see structure.en.md's "Media URLs" note). `mediaBaseUrl` must already end with a trailing slash. */
export function mediaUrl(mediaBaseUrl, relativePath) {
  if (!relativePath) return "";
  const path = relativePath.startsWith("/") ? relativePath.slice(1) : relativePath;
  return mediaBaseUrl + path;
}

/** mediaUrl()'s own contract above needs a trailing "/" - easy to type
 * without one (Gilles, 2026-10-03, after seeing it bite exactly this way:
 * "on a des soucis avec le / de fin, s'il est omis a la saisie ca ne
 * fonctionne pas"), silently producing a broken concatenated path instead
 * of an error. Applied wherever a media URL is accepted as input (typed,
 * restored from history, or filled in by zeroconf discovery) rather than
 * just once at connect time, so an already-saved bad value self-heals too. */
export function normalizeMediaUrl(url) {
  if (!url) return url;
  return url.endsWith("/") ? url : url + "/";
}

/** Strips a scheme (http://, https://, ws://, wss://, ...), any path
 * (/ostmedia/, /ws/, ...) and a trailing :port a user might paste out of
 * habit — the single host-only connection field (app.js's #server-host,
 * Gilles 2026-10-06: "une seule zone de saisie, juste l'url sans
 * http/https ou ws/wss pas de port") is meant to hold a bare host only;
 * this makes a pasted full URL degrade to its host instead of producing a
 * broken double-prefixed one. */
export function cleanHost(host) {
  if (!host) return "";
  return host.trim()
    .replace(/^[a-z][a-z0-9+.-]*:\/\//i, "")
    .replace(/\/.*$/, "")
    .replace(/:\d+$/, "");
}

/**
 * Derives the WebSocket URL and the media base URL from a bare host and an
 * SSL on/off toggle — the single-field+toggle connection UI (Gilles,
 * 2026-10-06: "une seule zone de saisie ... pas de port pas de /ostmedia
 * ou /ws. Tout ca doit etre determine juste avec le toggle et l'URL").
 * Same two shapes deriveDefaultUrls() above already derives from
 * `loc.protocol` — ssl:true goes through a reverse-proxy-style path route
 * on the standard HTTPS port (no raw 9624 exposed), ssl:false talks to
 * the raw ports directly. Confirmed against Gilles's own worked examples
 * before implementing, not guessed.
 */
export function deriveUrlsFromHost(host, ssl) {
  host = cleanHost(host);
  if (!host) return { wsUrl: "", mediaUrl: "" };
  return ssl
    ? { wsUrl: `wss://${host}/ws/`, mediaUrl: `https://${host}/ostmedia/` }
    : { wsUrl: `ws://${host}:9624`, mediaUrl: `http://${host}/ostmedia/` };
}

/**
 * Convenience factory bundling a Datastore + Connection into one object.
 * Everything it exposes is also reachable through `.datastore`/`.connection`
 * directly — this is just a flatter surface for simple UIs. See README.md
 * "Public API" for the full list and the topic-subscription contract.
 */
export function createClient() {
  const datastore = new Datastore();
  const connection = new Connection(datastore);

  return {
    datastore,
    connection,

    // session
    connect: (url, opts) => connection.connect(url, opts),
    disconnect: () => connection.disconnect(),
    onStatus: (fn) => connection.onStatus(fn),

    // data access — the structural rule: subscribe to the narrowest topic
    // that covers what you render (see datastore.js's module doc comment).
    subscribe: (topicKey, cb) => datastore.subscribe(topicKey, cb),
    get: (topicKey) => datastore.get(topicKey),
    scope: () => datastore.scope(),
    subscribeRaw: (cb) => datastore.subscribeRaw(cb),
    subscribeQueryAnswers: (cb) => datastore.subscribeQueryAnswers(cb),
    isDumpPending: () => datastore.isDumpPending(),
    resolveLov: (moduleName, element) => datastore.resolveLov(moduleName, element),
    topics,

    // writes — every documented client command, see connection.js
    send: (command, payload) => connection.send(command, payload),
    setValue: (...a) => connection.setValue(...a),
    setAll: (...a) => connection.setAll(...a),
    requestPropertyDump: (...a) => connection.requestPropertyDump(...a),
    requestDump: (...a) => connection.requestDump(...a),
    login: (...a) => connection.login(...a),
    setLanguage: (...a) => connection.setLanguage(...a),
    clickPropertyIcon: (...a) => connection.clickPropertyIcon(...a),
    clickElementIcon: (...a) => connection.clickElementIcon(...a),
    loadProfile: (...a) => connection.loadProfile(...a),
    saveProfile: (...a) => connection.saveProfile(...a),
    gridCreate: (...a) => connection.gridCreate(...a),
    gridUpdate: (...a) => connection.gridUpdate(...a),
    gridDelete: (...a) => connection.gridDelete(...a),
    gridFetch: (...a) => connection.gridFetch(...a),
    gridMoveUp: (...a) => connection.gridMoveUp(...a),
    gridMoveDown: (...a) => connection.gridMoveDown(...a),
    gridReset: (...a) => connection.gridReset(...a),
    loadConfiguration: (...a) => connection.loadConfiguration(...a),
    saveConfiguration: (...a) => connection.saveConfiguration(...a),
    loadModule: (...a) => connection.loadModule(...a),
    killModule: (...a) => connection.killModule(...a),
    query: (...a) => connection.query(...a),
    selectFolder: (...a) => connection.selectFolder(...a),
    indiStart: (...a) => connection.indiStart(...a),
    indiStop: (...a) => connection.indiStop(...a),
    indiLoadDriver: (...a) => connection.indiLoadDriver(...a),
    indiReloadDriver: (...a) => connection.indiReloadDriver(...a),
    indiStopDriver: (...a) => connection.indiStopDriver(...a),
    indiConnectDriver: (...a) => connection.indiConnectDriver(...a),
    indiDisconnectDriver: (...a) => connection.indiDisconnectDriver(...a),
    indiLoadConfiguration: (...a) => connection.indiLoadConfiguration(...a),
  };
}
