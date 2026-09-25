// UI1/app.js — bare-bones proof of integration for core/. No design effort,
// no CSS: the only goal is to prove connect/dump/subscribe/write works
// end-to-end against a real server, using core's public API only (nothing
// here reaches into core's internals).

import { createClient, deriveDefaultUrls } from "../core/index.js";

const client = createClient();

const wsUrlInput = document.getElementById("ws-url");
const statusSpan = document.getElementById("status");
const grantSpan = document.getElementById("grant");
const modulesDiv = document.getElementById("modules");
const logPre = document.getElementById("log");

wsUrlInput.value = deriveDefaultUrls().wsUrl;

document.getElementById("connect-btn").addEventListener("click", () => {
  client.connect(wsUrlInput.value, { language: "en" });
});
document.getElementById("disconnect-btn").addEventListener("click", () => client.disconnect());

client.onStatus((s) => { statusSpan.textContent = s; });

// Debug-only raw feed (see core's subscribeRaw doc comment: not for driving
// UI rendering, only shown here as a log).
client.subscribeRaw((code, payload) => {
  const line = code + " " + JSON.stringify(payload).slice(0, 300);
  logPre.textContent = line + "\n" + logPre.textContent;
  if (logPre.textContent.length > 20000) logPre.textContent = logPre.textContent.slice(0, 20000);
});

client.subscribe(client.topics.session(), (session) => {
  grantSpan.textContent = session.grantClient === null ? "" : session.grantClient;
});

// ---- per-property scoped rendering ---------------------------------------
// Each property block subscribes directly to its own topic and only
// rebuilds itself — demonstrating the "never re-render more broadly than
// the changed property" rule even in this bare harness.

function renderElementRow(moduleName, propName, elName, e) {
  const li = document.createElement("li");
  const label = document.createElement("code");
  label.textContent = elName + " (" + e.type + ")";
  li.appendChild(label);
  li.appendChild(document.createTextNode(" = "));

  const valueSpan = document.createElement("code");
  valueSpan.textContent = JSON.stringify(e.value);
  li.appendChild(valueSpan);

  if (e.directedit) {
    const input = document.createElement("input");
    input.value = typeof e.value === "object" ? JSON.stringify(e.value) : String(e.value);
    const btn = document.createElement("button");
    btn.textContent = "SV";
    btn.addEventListener("click", () => {
      let v = input.value;
      if (e.type === "int") v = parseInt(v, 10);
      else if (e.type === "float") v = parseFloat(v);
      else if (e.type === "bool") v = v === "true" || v === "1";
      else if (typeof e.value === "object") { try { v = JSON.parse(v); } catch (err) { alert("invalid JSON: " + err.message); return; } }
      client.setValue(moduleName, propName, elName, v);
    });
    li.appendChild(document.createTextNode("  edit: "));
    li.appendChild(input);
    li.appendChild(btn);
  }
  return li;
}

function renderPropertyBlock(moduleName, propName) {
  const details = document.createElement("details");
  const summary = document.createElement("summary");
  summary.textContent = propName;
  details.appendChild(summary);
  const body = document.createElement("div");
  details.appendChild(body);

  const scope = client.scope();
  scope.subscribe(client.topics.property(moduleName, propName), (prop) => {
    body.innerHTML = "";
    if (!prop) { body.textContent = "(deleted)"; return; }
    const meta = document.createElement("div");
    meta.textContent = "label=" + prop.label + " status=" + prop.status + " permission=" + prop.permission + " enabled=" + prop.enabled;
    body.appendChild(meta);
    const ul = document.createElement("ul");
    for (const [elName, e] of Object.entries(prop.elements)) ul.appendChild(renderElementRow(moduleName, propName, elName, e));
    body.appendChild(ul);
    if (prop.hasGrid) {
      const gridInfo = document.createElement("div");
      gridInfo.textContent = "grid rows: " + (prop.rows || []).length + " (headers: " + (prop.gridheaders || []).join(", ") + ")";
      body.appendChild(gridInfo);
    }
  });

  return details;
}

function renderModuleBlock(moduleName) {
  const details = document.createElement("details");
  const summary = document.createElement("summary");
  summary.textContent = moduleName;
  details.appendChild(summary);
  const propsDiv = document.createElement("div");
  propsDiv.style.marginLeft = "20px";
  details.appendChild(propsDiv);

  const scope = client.scope();
  let renderedProps = null;
  scope.subscribe(client.topics.module(moduleName), (mod) => {
    if (!mod) { propsDiv.textContent = "(module removed)"; return; }
    const propNames = Object.keys(mod.properties).sort();
    const key = propNames.join(",");
    if (key === renderedProps) return; // property set unchanged, don't rebuild
    renderedProps = key;
    propsDiv.innerHTML = "";
    for (const propName of propNames) propsDiv.appendChild(renderPropertyBlock(moduleName, propName));
  });

  return details;
}

client.subscribe(client.topics.modules(), (names) => {
  modulesDiv.innerHTML = "";
  for (const name of names) modulesDiv.appendChild(renderModuleBlock(name));
});
