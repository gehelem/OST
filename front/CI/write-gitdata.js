#!/usr/bin/env node
// CI/write-gitdata.js — writes www/gitdata.json (tag/date/commit/commit
// message of whatever's actually being built), so the running app - web
// or the Android build, both bundling this same www/ output - can show
// which build it is. Gilles, 2026-10-06: "ajoute un fichier 'gitdata' pour
// que l'application android et le site web puissent afficher la version
// (tag date commit message de commit) au niveau des settings, apres les
// infos de connexion" — see index.html's #version-info / js/app.js's own
// fetch of this file.
//
// Run as the last step of `npm run prepare-www` (package.json) rather
// than a CI-only step, so this is always in sync with prepare-www's own
// output and works the same whether that's invoked locally for a plain
// web deploy or inside CI building the Android APK - both read `git`
// directly (not GitLab CI's own predefined variables), on purpose, so a
// local run produces the same shape of file a CI run would.
"use strict";
const { execSync } = require("child_process");
const fs = require("fs");

function git(cmd, fallback) {
  try { return execSync(cmd, { encoding: "utf8" }).trim(); } catch (e) { return fallback; }
}

const data = {
  // Falls back to the short commit hash when no tag is reachable (any
  // commit between two releases) rather than failing outright.
  tag: git("git describe --tags --always", "unknown"),
  commit: git("git rev-parse --short HEAD", "unknown"),
  date: git("git log -1 --format=%cI", "unknown"),
  message: git("git log -1 --format=%s", "unknown"),
};

fs.writeFileSync("www/gitdata.json", JSON.stringify(data, null, 2) + "\n");
console.log("wrote www/gitdata.json:", data);
