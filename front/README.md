# ostcore

A reusable, framework-agnostic client "core" for the OST WebSocket protocol,
plus three UIs built on top of it, from a bare test harness to a
visually-designed reference client.

```
ostcore/
  core/     the shared library — zero rendering logic, see "Public API" below
  UI1/      bare-bones proof of integration, zero design effort
  UI2/      fuller UI, adapted from ostharfang4's render/views/app layer
  UI3/      same functional coverage as UI2, with real visual/ergonomic design
  FINDINGS.md
```

## Where this comes from

`core/` was built by extracting, generalizing and adapting
`ostharfang4/js/datastore.js` and `ostharfang4/js/ws.js` — an existing,
already-live-verified implementation of the same normalization/WebSocket
logic (see `ostharfang4/FINDINGS.md`) — rather than re-deriving the wire
protocol from the documentation from zero. The normalization functions
(`_normalizeModule`/`_normalizeProperty`/grid-row unpacking), the message
key classification scheme, `resolveLov()` (including its `-<template>`
LOV-suffix fallback), the command envelope builder (`_mp`) and every
send/builder method are carried over essentially unchanged. What's new in
`core/` on top of that base:

- a **per-topic subscription mechanism** (`subscribe(topic, cb)`) in place
  of ostharfang4's single `ds.onChange(fn)` — see "The one structural rule"
  below; this is the main design addition, not a rewrite of the protocol
  handling itself;
- a public API split into `core/datastore.js` (state + subscriptions),
  `core/connection.js` (WS lifecycle + senders), `core/index.js` (the
  facade UIs actually import), instead of one file per concern coupled to
  a specific app;
- `core/index.js`'s `deriveDefaultUrls()`/`mediaUrl()` helpers, generalized
  from the equivalent inline logic in ostharfang4's `app.js`/`render.js`.

Both `core/` and this adaptation were re-verified against a real,
freshly-built `ostserver` in this session (see FINDINGS.md and the
"Verification" section below) — not just trusted from the earlier session's
notes.

## The one structural rule: per-property subscription, never "everything changed"

`client-pattern.en.md` ("Rendering: scope updates to the property that
changed") is explicit that a client must never react to an incoming update
by re-rendering more broadly than the one property that changed — modules
can emit several updates per second, and re-rendering a whole screen on
each one causes lost input focus, flicker, or an unresponsive UI.

`core/datastore.js` enforces this at the data layer, so no UI has to
re-solve it: consumers call `subscribe(topic, callback)` with a topic built
from the `topics` helpers (`topics.property(moduleName, propName)`,
`topics.module(name)`, `topics.modules()`, `topics.files()`, ...), and the
callback is invoked **only** when that exact slice of state changes —
never on an unrelated property's `ea`/`ee`/`ps`. There is deliberately no
"subscribe to everything" call in the public API for driving UI rendering
(see `subscribeRaw()` below for the one debug-only exception).

The one legitimate exception is a full dump (`d`) / reconnection: at that
point *everything* genuinely did change (client-pattern.en.md:
"Reconnection: always start again from a full dump" — no partial state
survives a disconnect), so `reset()`/the `d` handler re-fires every
*currently subscribed* topic. It does not fire topics nobody asked about.

## Public API (`core/index.js`)

```js
import { createClient, deriveDefaultUrls, mediaUrl, topics } from "./core/index.js";

const client = createClient();
```

### Connection lifecycle

| Call | Description |
|---|---|
| `client.connect(url, {language})` | Opens the WebSocket, resets the datastore, sends `DU` once open, starts the 30s heartbeat (`XX`). |
| `client.disconnect()` | Closes the socket, stops the heartbeat. The `close` handler resets the datastore too. |
| `client.onStatus(fn)` | `fn(status)` with `status` one of `"disconnected"`/`"connecting"`/`"connected"`. Called immediately with the current status, then on every change. Returns an unsubscribe function. |

### Reading data — the per-topic subscription contract

| Call | Description |
|---|---|
| `client.subscribe(topicKey, cb)` | `cb(value)` immediately with the current value, then again every time that slice of state changes. Returns an unsubscribe function. |
| `client.get(topicKey)` | Reads the current value at a topic without subscribing. |
| `client.scope()` | Returns `{subscribe, dispose}` — a convenience for tearing down every subscription a screen made in one call when it's discarded. Not part of the wire protocol. |
| `client.topics` | The topic-builder namespace, see table below. |
| `client.resolveLov(moduleName, element)` | Resolves an element's `{key: label}` map across all 3 LOV scopes (local `listOfValues`, module `globallovs`, controller `lovs`), including the documented-but-never-actually-sent `-<template>` suffix fallback (see FINDINGS.md #1). Returns `null` if the element has no LOV at all. |
| `client.subscribeQueryAnswers(cb)` | `cb(payload)` for `qa` (query answer) messages — inter-module mechanism, not really client-facing (see client-messages.en.md#qy), included for completeness. |
| `client.subscribeRaw(cb)` | `cb(code, payload)` for **every** applied message. Debug/log-console use only — driving UI rendering from this reintroduces exactly the "re-render everything" problem the topic mechanism exists to avoid. UI1 uses it only for its raw traffic log panel. |

Topic builders (`client.topics.*`):

| Builder | Fires on | Value |
|---|---|---|
| `topics.session()` | `d` (and reset) | `{grantClient, grantServer, serverLng}` |
| `topics.modules()` | `d`, `aa`, `dm` (module added/removed — never on ordinary property traffic) | sorted array of module names |
| `topics.module(name)` | `d`, `aa`, `dm`, `fs`/`fl`/`fc` (profile events) | `{infos, profile, lovs, properties}` or `null` |
| `topics.moduleLovs(name)` | `d`, `aa`, `dm`, module-scoped `lc`/`lu`/`ld` | that module's own LOV map |
| `topics.property(name, prop)` | `ap`, `dp`, `ea`, `ee`, `ev`, `ps`, `gc`, `gu`, `gd`, `gr` — the fine-grained, high-frequency topic | the property object (`elements`, `rows`, `status`, ...) or `null` |
| `topics.lovs()` | `d`, controller-scoped `lc`/`lu`/`ld` | controller-level LOV map (wire key `lovs`, see FINDINGS.md #2) |
| `topics.controllerData(key?)` | `d`, `uc` | one `controllerdata` key's value, or the whole object without `key` |
| `topics.files()` | `d`, `af`/`df`/`ad`/`dd`, `uc` files/folders | `{folders, files, selectedfolder}` |
| `topics.logs()` | `d`, `l` | the full log array (capped at 1000 entries) |

A known, deliberate granularity limitation: `topics.module(name)` does
**not** fire when `ap` introduces a property name that didn't previously
exist (only `aa` — a full module (re)load — rebuilds the property set a
module-screen's layout is keyed on). In every case observed against the
real server, `ap` was used to force-refresh an *existing* property (the
`DP` response), never to add a new one to a running module instance — see
README "Verification" and FINDINGS.md.

### Writing data — one method per documented client command

All of these are thin wrappers around `client.send(command, payload)`
(itself the raw `{[COMMAND]: payload}` sender) and match
`client-messages.en.md` 1:1:

`setValue(mod, prop, el, value)` (`SV`) · `setAll(mod, prop, values)` (`SA`)
· `requestPropertyDump(mod, prop)` (`DP`) · `requestDump(language?)` (`DU`,
beyond the one `connect()` sends automatically) · `login(user, pw, language?)`
(`LO`) · `setLanguage(language)` (`IL`) · `clickPropertyIcon(1|2|3|4, mod, prop)`
(`I1`-`I4`) · `clickElementIcon(1|2, mod, prop, el)` (`J1`/`J2`) ·
`loadProfile`/`saveProfile(mod, profile)` (`PL`/`PS`) ·
`gridCreate(mod, prop, values)` (`GC`) · `gridUpdate(mod, prop, i, values)`
(`GU`) · `gridDelete(mod, prop, i)` (`GD`) · `gridFetch(mod, prop, i)` (`GF`)
· `gridMoveUp`/`gridMoveDown(mod, prop, i)` (`GH`/`GB`) · `gridReset(mod, prop)`
(`GR`) · `loadConfiguration(name)` (`CL`) · `saveConfiguration(name)` (`CS`)
· `loadModule(lib, label, profile?)` (`ML` — `lib` must NOT carry the
`libost` prefix `controllerdata.libraries`' keys have) · `killModule(name)`
(`MK`) · `query(mod, queryName, params?)` (`QY`, inter-module mechanism) ·
`selectFolder(folder)` (`FS`) · `indiStart`/`indiStop()` (`YA`/`YZ`) ·
`indiLoadDriver`/`indiReloadDriver`/`indiStopDriver(binary)` (`YL`/`YR`/`YS`).

### Module-level helpers (`core/index.js`, not tied to a connected client)

- `deriveDefaultUrls(loc?)` — derives `{wsUrl, mediaUrl}` from the page's
  own `location` per structure.en.md's "Finding the server" table. Web
  client only; a non-web client needs its own discovery (mDNS, manual
  entry — not implemented here, out of scope for a web core).
- `mediaUrl(mediaBaseUrl, relativePath)` — joins a media base URL with a
  wire-relative path (`img`/`video`/file paths), handling the "don't
  double the `/`" footgun the docs call out.

### Advanced / escape hatches

`client.datastore` and `client.connection` are the underlying `Datastore`
and `Connection` instances (also exported directly from `core/index.js`),
for a UI that wants more than the facade — e.g. `datastore.modules` for a
direct synchronous read of the whole normalized tree, if a topic-scoped
`get()`/`subscribe()` genuinely doesn't fit a given use case.

## UI1 — bare-bones proof of integration

`UI1/index.html` + `UI1/app.js`. No CSS, no design effort — a test harness,
not a UI. Lists every module/property/element as they arrive (each
property block subscribes to its own `topics.property()` topic and
re-renders only itself), shows a raw message log via `subscribeRaw()`, and
every `directedit` element gets an inline input + "SV" button for a live
write round-trip.

Run: serve `ostcore/` over any static HTTP server (it's plain ES modules,
no build step — e.g. `python3 -m http.server` from `ostcore/`) and open
`UI1/index.html`. The WS URL field defaults to the derived
`ws://<host>:9624`; adjust it if the server isn't local, then Connect.

## UI2 — fuller UI, adapted from ostharfang4

`UI2/index.html` + `UI2/js/{app,render,views,chart}.js` + `UI2/css/style.css`
+ `UI2/assets/fonts/MaterialIcons-Regular.ttf`.

**The ostharfang4 adaptation worked**, and gave UI2 almost for free:
`chart.js` (pure canvas rendering of a `property` object's `rows`/
`graphType`/`graphParams` — never touched the datastore or connection to
begin with) is an **unchanged copy**. `render.js` and `views.js` needed
only a mechanical adaptation: every function that took ostharfang4's
`(ds, conn)` pair now takes one `client` (core's facade), and
`ds.resolveLov`/`conn.setValue`/etc. become `client.resolveLov`/
`client.setValue`/etc. — same call shapes, same object structure for
modules/properties/elements (core's normalized `Property`/`Element` shape
was deliberately kept identical to ostharfang4's, see "Where this comes
from" above).

The one real structural change, and the reason it's worth calling out
rather than a pure find-and-replace: ostharfang4's `app.js` owned a large
`switch` on every incoming message type, driven by one global
`ds.onChange(fn)`, looking up the right DOM node
(`findPropertyNode`/`container._update()`) to patch for each message. Core
doesn't offer that single global hook by design (see "The one structural
rule" above), so that whole dispatcher is gone. Instead:

- `renderProperty(client, moduleName, propName)` (`render.js`) subscribes
  directly to `client.topics.property(moduleName, propName)` and rebuilds
  its own DOM subtree when — and only when — that property changes.
  Returns the container node with its unsubscribe function attached as
  `container._dispose`, which its caller (`renderModuleScreen`) collects.
- `renderModuleScreen(client, moduleName)` subscribes once to
  `client.topics.module(moduleName)` for the module-level layout (property
  set + profile bar), and owns the list of its children's `_dispose`
  functions. Returns `{node, dispose}`.
- `renderController`/`renderFiles`/`renderLogs` (`views.js`) each open
  their own `client.scope()` against the topic(s) they need
  (`controllerData()`+`modules()`, `files()`, `logs()`) and return
  `{node, dispose}`.
- `app.js` is left with only genuine app-level orchestration: which screen
  is active, the sidebar (subscribes to `topics.modules()`), the
  connect/login flow (subscribes to `topics.session()`). It calls
  `dispose()` on the previous screen before building the next one. This is
  **less** code than ostharfang4's original `app.js`, not more — the
  dispatcher it replaces was the single largest function in that file.

Net result: UI2 is functionally equivalent to ostharfang4 (generic
module/property/element/grid/graph rendering, controller/files/logs
screens, login flow, dark/light theme, profile bar with the same
best-effort LOV-key fallback chain) but each piece of the screen now owns
its own precise subscription instead of routing through one dispatcher —
arguably a cleanup enabled by core's design, not just a port.

Run the same way as UI1 (static server, open `UI2/index.html`).

## UI3 — same coverage as UI2, designed

`UI3/index.html` + `UI3/js/{app,render,views,chart,dom}.js` + `UI3/css/
{theme,base,components}.css` + `UI3/assets/fonts/MaterialIcons-Regular.ttf`.

Functionally, UI3 is UI2: the same generic module/property/element/grid/
graph rendering, controller/files/logs screens, login flow, and per-topic
subscription discipline (nothing here reaches for a broader topic than the
one thing being rendered — see "The one structural rule" above). `render.js`
and `views.js` are UI2's logic re-skinned for the new markup/classes (plus
a shared `dom.js` for the `el()`/`icon()`/`statusDot()`/`iconButton()`
helpers both files use, instead of each file redefining them); `chart.js`
is an unchanged copy, same as it was in UI2 from ostharfang4.

What's actually new is the design layer, built by studying `ostnoctua/`
(its `themes.css` token structure, `.glass-panel`/`.on-modbar` component
conventions, spacing rhythm, status-LED glow, monospace-for-numeric-values
habit) and adapting that language — not its floating-panels-over-a-sky-
canvas layout, which doesn't fit a client that must render *every* module
generically — to a conventional sidebar + content shell:

- `css/theme.css` — dark (default) and light theme, CSS custom properties
  only, structured as the same kind of token set ostnoctua's `themes.css`
  uses (surfaces/accent/text/border/status/shadow/rhythm variables) so a
  third theme could be added the same way ostnoctua adds its "sober"/
  "graphite"/"twilight"/"ember" variants, without touching component CSS.
- `css/components.css` — rounded cards with a soft accent glow on hover/
  active states, an icon+label sidebar (module icon inferred from
  `infos.template`/`infos.lib`/the module name via a small best-effort
  table in `dom.js`, generic `extension` fallback for anything unlisted —
  the fallback matters more than the table, since UI3 must render module
  types ostnoctua never implements), uppercase small section labels,
  glowing status dots, a blurred dialog overlay for grid add/edit and
  login, and thin accent-toned scrollbars everywhere — all lifted in
  spirit from ostnoctua's `.glass-panel`/`.on-prop`/`.conn-led` etc.

Run the same way as UI1/UI2 (static server from `ostcore/`, open
`UI3/index.html`).

## Verification

Beyond the reuse of ostharfang4's already-live-verified logic, this
session separately re-verified `core/` and both UIs against a freshly
built, freshly run `ostserver` (`--webroot=/home/gilles/ostmedia`,
`--libpath=.../buildqt6`, `--grant=0`, matching the task's documented
launch command) — see FINDINGS.md for what that surfaced:

- a direct Node script (`createClient()` + Node 22's built-in `WebSocket`,
  no DOM) exercised connect → full dump → `topics.modules()`/
  `topics.property()` subscriptions → an `SV` write round-trip → clean
  reset on disconnect;
- both `UI1/app.js` and `UI2/js/app.js` were loaded as real ES modules
  (via direct Node `import()`, with `document`/`window`/`localStorage`
  supplied by `jsdom`, and `WebSocket` by Node) against the same live
  server: module lists, property/element rendering, grid rendering
  (`GlobalDatastore.equipments`/`locations`/`optics`/`servers`), and an
  actual UI button click (a `directedit` bool "toggle" button in UI2)
  driving a real `SV`/`ea` round trip end-to-end through the DOM;
- `ost.db` was never read, written, or otherwise touched (checked its
  mtime before/after); the server was built, run, and stopped via exactly
  the documented commands; a stray, already-running second `ostserver`
  instance from before this session was left alone except for confirming
  it wasn't the one this session started (see FINDINGS.md's "Server
  process notes").

`UI3/js/app.js` was separately verified the same way, in the session that
built it: real WS connect against a freshly (re)built, freshly run
`ostserver` (same launch command, `libstellarsolver6.so.2` resolved via
`LD_LIBRARY_PATH=/home/gilles/projets/stellarsolver/build` — the prebuilt
binary doesn't find it otherwise, see ostharfang4/FINDINGS.md's note on
the same issue) → sidebar populated with all 7 live modules + Controller/
Files/Logs → Controller screen (5 panels) → a module screen (tabs, 9
properties) → tab switching → a real `SV`/`ea` round trip through an
actual rendered slider + "send" button (`MyCCDinspector.parms.gain`,
100 → 150, confirmed reflected back in the DOM) → grid rendering
(`GlobalDatastore`'s 4 grid tables) → Files screen (172 folder nodes, 83
file cards from the real `/home/gilles/ostmedia` tree) → Logs screen (77
entries) → theme toggle → clean disconnect. `ost.db`'s mtime was
unchanged throughout, and the `ostserver` instance this session started
was killed at the end.
