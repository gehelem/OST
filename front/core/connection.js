// core/connection.js
//
// WebSocket connection lifecycle (connect/heartbeat/reconnection) plus a
// typed builder/sender for every documented client -> server command (see
// OST/documentation/content/dev/dev-client/client-messages.en.md). No DOM
// access, no rendering — this file only ever calls `this.ds.applyMessage()`
// on incoming frames and `this.socket.send()` on outgoing ones.

export class Connection {
  /** @param {import("./datastore.js").Datastore} datastore */
  constructor(datastore) {
    this.ds = datastore;
    this.socket = null;
    this.url = null;
    this.language = "en";
    this.heartbeatTimer = null;
    this.heartbeatIntervalMs = 30000; // per client-pattern.en.md's "Heartbeat" (optional, but sent regardless — see README)
    this._statusListeners = new Set();
    this.status = "disconnected"; // "disconnected" | "connecting" | "connected"
  }

  onStatus(fn) {
    this._statusListeners.add(fn);
    fn(this.status);
    return () => this._statusListeners.delete(fn);
  }

  _setStatus(s) {
    this.status = s;
    for (const fn of this._statusListeners) { try { fn(s); } catch (e) { console.error(e); } }
  }

  /**
   * Opens the WebSocket, resets the datastore (client-pattern.en.md:
   * "On any disconnection, the client must entirely clear its datastore" —
   * applied here too so a manual reconnect never mixes stale + fresh
   * state), and sends `DU` once open. The server replies with a full `d`
   * dump (see datastore.js's reset()/applyMessage()).
   */
  connect(url, opts) {
    this.url = url;
    this.language = (opts && opts.language) || this.language;
    this.ds.reset();
    this._setStatus("connecting");
    const socket = new WebSocket(url);
    this.socket = socket;
    socket.addEventListener("open", () => {
      this._setStatus("connected");
      this.send("DU", { language: this.language });
      this._startHeartbeat();
    });
    socket.addEventListener("message", (ev) => this.ds.applyMessage(ev.data));
    socket.addEventListener("close", () => {
      this._setStatus("disconnected");
      this._stopHeartbeat();
      this.ds.reset();
    });
    socket.addEventListener("error", () => {
      // "close" always follows "error" for a WebSocket; nothing extra to do.
    });
  }

  disconnect() {
    if (this.socket) this.socket.close();
    this._stopHeartbeat();
  }

  _startHeartbeat() {
    this._stopHeartbeat();
    // The protocol doesn't require this (client-pattern.en.md: "Optional"),
    // but browsers/proxies commonly close idle WebSocket connections, so
    // core sends it unconditionally rather than leaving every UI to
    // reimplement the same 30s timer.
    this.heartbeatTimer = setInterval(() => this.send("XX", {}), this.heartbeatIntervalMs);
  }

  _stopHeartbeat() {
    if (this.heartbeatTimer) clearInterval(this.heartbeatTimer);
    this.heartbeatTimer = null;
  }

  /** Raw send: {[command]: payload}. Every method below is a thin wrapper around this. */
  send(command, payload) {
    if (!this.socket || this.socket.readyState !== WebSocket.OPEN) {
      console.warn("Cannot send, socket not open", command, payload);
      return false;
    }
    this.socket.send(JSON.stringify({ [command]: payload }));
    return true;
  }

  _mp(moduleName, propName, extra) {
    return { m: { [moduleName]: { p: { [propName]: extra || {} } } } };
  }

  // ---- session ---------------------------------------------------------

  /** Not required by connect() to be called separately — connect() already sends it on open. Use this to re-request a fresh dump without reconnecting (e.g. after suspecting drift). */
  requestDump(language) { this.send("DU", { language: language || this.language }); }
  login(user, pw, language) { this.send("LO", { user, pw, language: language || this.language }); }
  setLanguage(language) { this.language = language; this.send("IL", { language }); }

  // ---- element writes ---------------------------------------------------

  /** directedit:true elements only — SV on a directedit:false element is rejected server-side. */
  setValue(moduleName, propName, elementName, value) {
    this.send("SV", this._mp(moduleName, propName, { e: { [elementName]: value } }));
  }
  /** Submit every editable element of a property at once. Mandatory for directedit:false elements, and must include *every* editable element or the backend applies nothing at all (client-messages.en.md). */
  setAll(moduleName, propName, values) {
    this.send("SA", this._mp(moduleName, propName, { e: values }));
  }
  /** Force-refresh one property (server replies with `ap`). */
  requestPropertyDump(moduleName, propName) {
    this.send("DP", this._mp(moduleName, propName, {}));
  }

  // ---- icon actions (preicon1/2, posticon1/2 on a property; preicon/posticon on an element) --

  /** slot: 1 -> I1 (preicon1), 2 -> I2 (preicon2), 3 -> I3 (posticon1), 4 -> I4 (posticon2). */
  clickPropertyIcon(slot, moduleName, propName) {
    this.send("I" + slot, this._mp(moduleName, propName, {}));
  }
  /** slot: 1 -> J1 (preicon), 2 -> J2 (posticon). */
  clickElementIcon(slot, moduleName, propName, elementName) {
    this.send("J" + slot, this._mp(moduleName, propName, { e: { [elementName]: {} } }));
  }

  // ---- profiles ----------------------------------------------------------

  loadProfile(moduleName, profile) { this.send("PL", { m: { [moduleName]: { profile } } }); }
  saveProfile(moduleName, profile) { this.send("PS", { m: { [moduleName]: { profile } } }); }

  // ---- grid ---------------------------------------------------------------

  gridCreate(moduleName, propName, values) { this.send("GC", this._mp(moduleName, propName, { e: values })); }
  gridUpdate(moduleName, propName, index, values) { this.send("GU", this._mp(moduleName, propName, { i: index, e: values })); }
  gridDelete(moduleName, propName, index) { this.send("GD", this._mp(moduleName, propName, { i: index })); }
  gridFetch(moduleName, propName, index) { this.send("GF", this._mp(moduleName, propName, { i: index })); }
  gridMoveUp(moduleName, propName, index) { this.send("GH", this._mp(moduleName, propName, { i: index })); }
  gridMoveDown(moduleName, propName, index) { this.send("GB", this._mp(moduleName, propName, { i: index })); }
  gridReset(moduleName, propName) { this.send("GR", this._mp(moduleName, propName, {})); }

  // ---- module lifecycle / configurations -----------------------------------

  loadConfiguration(name) { this.send("CL", { name }); }
  saveConfiguration(name) { this.send("CS", { name }); }
  /** `lib` must NOT carry the `libost` prefix that controllerdata.libraries' keys have — the server adds it (see client-messages.en.md's warning on ML). */
  loadModule(lib, label, profile) {
    const body = { lib, label };
    if (profile) body.profile = profile;
    this.send("ML", body);
  }
  killModule(name) { this.send("MK", { name }); }

  // ---- inter-module query (documented for completeness — see client-messages.en.md#qy) --

  query(moduleName, queryName, params) {
    this.send("QY", { m: { [moduleName]: { query: queryName, params: params || {} } } });
  }

  // ---- files ---------------------------------------------------------------

  /** Server-wide: changes the watched folder for every connected client (controller.en.md). */
  selectFolder(folder) { this.send("FS", { folder }); }

  // ---- embedded INDI server control -----------------------------------------

  indiStart() { this.send("YA", {}); }
  indiStop() { this.send("YZ", {}); }
  indiLoadDriver(binary) { this.send("YL", { driver: binary }); }
  indiReloadDriver(binary) { this.send("YR", { driver: binary }); }
  indiStopDriver(binary) { this.send("YS", { driver: binary }); }
  indiConnectDriver(binary) { this.send("YC", { driver: binary }); }
  indiDisconnectDriver(binary) { this.send("YD", { driver: binary }); }
}
