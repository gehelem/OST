// zeroconf.js — OST server discovery via mDNS/Zeroconf, Android-app-only
// (Gilles, 2026-10-03: "ajoute la fonction discover dans le cas ou on
// utilise l'app android ... ecouter le zeroconf/discover comme sur
// rooster"). Browsers have no API for raw mDNS - this only ever does
// anything inside the real Capacitor Android app, where the native
// plugin bridges real multicast DNS into JS; isAvailable() below is the
// gate app.js uses to keep the whole feature invisible everywhere else.
//
// ostcore has no bundler (plain ES modules, loaded directly by the
// browser) - @capacitor/core and capacitor-zeroconf can't be imported
// as bare specifiers like a bundled app would, so both are vendored
// into lib/ (capacitor-core.js, capacitor-zeroconf*.js - see those
// files' own header comments) instead. `npm install` isn't removed
// either: android/'s own native build still needs the real packages
// there for `npx cap sync` to link the native plugin code in.
//
// Service type/domain, the 9624 port default, and the ".local" stripping
// below all come directly from OstErix/Rooster's own
// src/app/services/zeroconf.service.ts (the existing, working client
// against this same backend), not guessed:
//  - type "_ostserver_ws._tcp.", domain "local." - what the OST backend
//    itself actually announces under.
//  - Rooster's own comment, word for word: "Hostname: strip .local /
//    .local. suffix (mDNS format) — plain hostname works on Android" -
//    Gilles independently hit the same thing from the client side,
//    2026-10-03: "il y a un sujet avec le format de l'url ... un truc
//    avec nomduserver.local => il faut enlever le local".
//
// Discovery only ever makes sense unencrypted on a local network (no CA
// is going to vouch for a TLS cert on a randomly-discovered LAN
// hostname) - Gilles: "cette fonction discover n'a d'usage que si on
// est en reseau local, donc sans SSL" - so the URLs built here are
// always ws://host:port and http://host/ostmedia/, never wss/https.

import { ZeroConf } from "../lib/capacitor-zeroconf.js";
import { Capacitor } from "../lib/capacitor-core.js";

const SERVICE_TYPE = "_ostserver_ws._tcp.";
const SERVICE_DOMAIN = "local.";
const DEFAULT_PORT = 9624;

export function isAvailable() {
  try { return !!(Capacitor && Capacitor.isNativePlatform && Capacitor.isNativePlatform()); }
  catch (e) { return false; }
}

export function stripLocalSuffix(host) {
  return host.replace(/\.local\.?$/i, "");
}

/**
 * Pure mapping from one raw `watch()` callback event to a discovered
 * server (or null if the event isn't a genuine, usable "resolved"
 * service) - split out from discover() below so the actual parsing/URL-
 * building logic is unit-testable without faking the whole native
 * Capacitor bridge (not reproducible in a plain Node/jsdom environment -
 * this specific function is the one place that matters for the
 * ".local"-stripping/URL-shape bug Gilles reported, so it's the one
 * place worth testing directly).
 */
export function parseWatchEvent(result) {
  // The plugin's own first callback invocation is an empty ack (the
  // native watch() resolves once immediately, separately from each later
  // per-service event - see android's own ZeroConfPlugin.java) - nothing
  // to act on there, same as a non-"resolved" (added/removed) event.
  if (!result || !result.service || result.action !== "resolved") return null;
  const svc = result.service;
  const rawHost = (svc.hostname && svc.hostname.length) ? svc.hostname : svc.name;
  if (!rawHost) return null;
  const host = stripLocalSuffix(rawHost);
  const port = svc.port || DEFAULT_PORT;
  return {
    name: svc.name || host,
    host,
    wsUrl: `ws://${host}:${port}`,
    mediaUrl: `http://${host}/ostmedia/`,
  };
}

/**
 * Scans for `timeoutMs` (default 10s, matching Rooster's own scan
 * window) and resolves with every distinct server found (de-duplicated
 * by host). Starts and stops its own watch every call - this scans on
 * demand (a button press), never continuously in the background, so
 * there's no persistent subscription to manage between calls.
 */
export function discover(timeoutMs) {
  if (!isAvailable()) return Promise.resolve([]);
  const found = new Map();
  return new Promise((resolve) => {
    let settled = false;
    const finish = () => {
      if (settled) return;
      settled = true;
      clearTimeout(timer);
      try { ZeroConf.unwatch({ type: SERVICE_TYPE, domain: SERVICE_DOMAIN }); } catch (e) { /* ignore */ }
      resolve([...found.values()]);
    };
    const timer = setTimeout(finish, timeoutMs || 10000);
    ZeroConf.watch({ type: SERVICE_TYPE, domain: SERVICE_DOMAIN }, (result, err) => {
      if (err) return;
      const server = parseWatchEvent(result);
      if (server) found.set(server.host, server);
    }).catch(finish); // watch() itself never got going (e.g. permission denied) - stop waiting instead of hanging the full timeout
  });
}
