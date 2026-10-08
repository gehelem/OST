# FINDINGS — documentation sufficiency log for ostcore

Scope: this log covers building `ostcore/core/` (by adapting
`ostharfang4/js/datastore.js`/`ws.js`, per this task's brief) and verifying
it — plus `UI1/` and the `UI2/` adaptation of `ostharfang4/js/render.js`/
`views.js`/`app.js`/`chart.js` — against `documentation/content/dev/dev-client/`
and a real, freshly built `ostserver`.

Most of the wire-protocol findings from `ostharfang4/FINDINGS.md` (the
session that first built the code `core/` is adapted from) still apply
unchanged, since `core/`'s normalization/LOV-resolution logic is a direct
carry-over. They're summarized below rather than re-derived from scratch,
with a note on what (if anything) this session re-confirmed live. New
findings specific to this session follow after.

---

## Findings carried over from ostharfang4/FINDINGS.md (still apply to core/)

1. **`profiles-<template>`/`loadedModules-<template>` LOVs are documented
   but never actually sent** — the server-side code that would generate
   them is commented out (`src/controller.cpp`); `core/datastore.js`'s
   `resolveLov()` keeps ostharfang4's client-side fallback (filter the base
   unfiltered LOV by suffix). Not re-derived this session; carried over
   as-is (still the right behavior, nothing in this session's live traffic
   contradicted it).
2. **Controller-level LOVs: `structure.en.md`'s prose says `controllerlovs`,
   the wire key is `lovs`** — re-confirmed live this session (see below,
   finding "New — `availableconfs` is an undocumented 5th key of `d.lovs`");
   `controllerlovs` still doesn't appear anywhere on the wire.
3. **`infos.template` is not a reliable key into `controllerdata.profiles`**
   (e.g. `Sequencer`'s `template` is `"sequence"`, which transforms to
   `"Sequence"`, matching neither `"Sequencer"` — the real key — nor
   anything else) — `core`'s `UI2/js/render.js` `renderProfileBar()` keeps
   ostharfang4's same best-effort fallback chain (transform →
   case-insensitive/prefix match → free-text input). Not independently
   re-verified against `Sequencer` specifically this session (this
   session's live dump did include a `Sequencer`-keyed `controllerdata.profiles`
   entry, consistent with the prior finding, but the mismatch itself
   wasn't re-triggered).
4. **`img` element carries undocumented `showstats`/`hfravgdev` fields** —
   no impact on `core/`, which doesn't allowlist element fields.
5. **`GC`'s "base elements update before the grid line" is correct but
   easy to under-read** (each changed base element gets its own full-property
   `ea`, sequentially, before the final `gc`) — no client-side handling
   needed either way; `core`'s per-property topic just fires once per `ea`
   as they arrive, same as ostharfang4's dispatcher did.
6. **`PL` emits a transient `fc` immediately before `fl`**, not mentioned
   in `client-messages.en.md`'s `PL` entry — harmless (the profile bar just
   reflects whatever `mod.profile` currently holds), not specially handled.

See `ostharfang4/FINDINGS.md` for the full detail, live-traffic excerpts,
and suggested doc wording on each of these.

---

## New findings from this session

### N1. Several documented `controllerdata` keys are absent from the *initial* dump, only appearing after their first `uc`

**What the docs say:** `controller.en.md`'s `controllerdata` table lists
`systemwatcher`, `files`, `folders` as regular content of `controllerdata`
— and `systemwatcher`'s own section is explicit: *"The message is an
ordinary `uc`, and the latest value **is also present** in
`controllerdata.systemwatcher` of the dump."* That reads as a guarantee:
connect, get a `d`, and `controllerdata.systemwatcher`/`.files`/`.folders`
are already there.

**What a live dump actually contained**, captured within ~1.5s of
connecting to a freshly started `ostserver --systemwatchinterval=30`:

```
controllerdata keys: [
  'availableconfs', 'banner', 'currentconf', 'git',
  'indiactivedrivers', 'indidrivers', 'indiserver',
  'libraries', 'profiles'
]
```

No `systemwatcher`, `files`, or `folders` key at all — not an empty object,
simply absent. (`d.files` itself, the top-level sibling key, *was* present
and populated, as documented separately — this is specifically about the
`controllerdata`-level mirror/copy of that data and of `systemwatcher`.)

**How I resolved it:** `core/datastore.js`'s `_applyControllerUpdate()`
(driven by `uc`) already tolerates any key being absent at dump time —
`topics.controllerData(key)` simply returns `undefined` until the first
`uc` for that key arrives, and `UI2/js/views.js`'s `renderController()`
guards every section on the field being present (`if (sw) {...}`, `cd.files`
never dereferenced without a guard). So no code change was needed, but this
was verified by reading the code defensively, not by the docs telling me
to expect the gap.

**What I'd want the docs to say instead:** either (a) guarantee these three
keys populate `controllerdata` synchronously as part of assembling the `d`
response (matching what the `systemwatcher` section currently promises), or
(b) state plainly that `systemwatcher`/`files`/`folders` under
`controllerdata` are best-effort mirrors that populate lazily via the first
matching `uc` (server load takes up to `--systemwatchinterval` seconds; the
`files`/`folders` mirror may simply not be wired into the initial dump
assembly at all) — a client relying on the current wording could
reasonably skip defensive-coding a "not there yet" state for
`systemwatcher` specifically, since the doc calls that one out by name as
dump content.

### N2. `availableconfs` is an undocumented 5th key of `d.lovs` / `lc`'s controller-scope LOVs

**What the docs say:** `controller.en.md`'s `GlobalDatastore` section: *"Its
grids feed lists of values (LOVs) of the same name, present in `d.lovs`
(`optics`, `locations`, `equipments`, `servers`)..."* — four keys, named
exhaustively (the sentence structure and the parenthetical don't read as
"e.g.").

**What the live dump actually contained:**

```
lovs keys: [ 'availableconfs', 'equipments', 'locations', 'optics', 'servers' ]
```

Five keys — `availableconfs` (also, separately and correctly, documented
as a `controllerdata` key: "Saved module configurations") is *additionally*
exposed as a controller-scope LOV, which the `GlobalDatastore` LOV list
doesn't mention. (`ostharfang4/FINDINGS.md`'s finding #2 captured the same
raw key list in its own live dump as supporting evidence for the
`controllerlovs`-vs-`lovs` naming issue, but didn't call out `availableconfs`
itself as a 5th, undocumented LOV key — this is a refinement of that
observation, independently re-captured this session.)

**How I resolved it:** no code impact — `core/datastore.js` doesn't
allowlist `lovs` keys, so an extra one is simply stored and available via
`topics.lovs()` like any other. Purely a completeness gap for anyone
building a "pick a saved configuration" dropdown from `lovScope:
"controller"` + `globallov: "availableconfs"` and wondering where that key
comes from.

**What I'd want the docs to say instead:** add `availableconfs` to the
`GlobalDatastore` section's LOV list, or state explicitly that the list is
illustrative/non-exhaustive.

### N3. Open, unresolved: how a client is supposed to detect "this `ev`/`ee` is the globallov-changed, value-omitted case" (not re-tested live)

`client-pattern.en.md`'s "Known pitfall: GlobalLOV updates" says a
referencing property's update "sends... without the `value` field" when its
global LOV changes, and that the client must never preserve the old value
in that case. What isn't specified anywhere: the actual JSON shape of that
omission (is the affected element's entry in the `e` map an empty object
`{}`? does an `ev` for it simply lack a `value` sub-key while still
carrying `min`/`max`/`format`? is `ee`/`ea` involved at all, or only `ev`?),
nor any positive signal a client could use to distinguish "this update
legitimately doesn't include a fresher value, so don't touch what's
there" from "this update is the pitfall case, so blank the value even
though the key wasn't sent."

**How I resolved it:** I did not attempt to reproduce this live this
session (it requires driving a global-LOV-referencing element through a
LOV mutation, which none of the currently loaded demo modules made
straightforward to trigger in the time available) — `core/datastore.js`'s
`_applyElementMeta()` keeps ostharfang4's existing behavior (merge via
`Object.assign`, which does *not* implement the "never preserve" rule for
this specific case — see the code comment left at the top of that
function). This is a known, flagged gap, not a silent one.

**What I'd want the docs to say instead:** show the actual JSON of one such
update (the same way the `d`/`ea`/`gc` examples already do for everything
else), so a client can pattern-match it instead of inferring a detection
rule from prose alone.

### N4. Design assumption, not contradicted by any observed traffic: `ap` never introduces a property name absent from the last `aa`/`d`

`core/`'s `topics.module(name)` (the topic a module screen's *layout* —
which properties exist, how they're grouped — is built from) intentionally
does **not** re-fire on `ap` (`ap` only touches `topics.property(name,
prop)` for the property it replaces). This is correct and matches
`client-pattern.en.md`'s "never re-render more broadly than the changed
property" if — and only if — `ap` is always a refresh of an
already-existing property (the documented use: the response to a client's
own `DP`), never the mechanism by which a module gains a property it didn't
have a moment ago.

Nothing in `client-messages.en.md`/`server-messages.en.md` states this
explicitly either way, and nothing in this session's live traffic
contradicted the assumption (every `ap` observed targeted a property
already present in the preceding `d`/`aa`). Flagging it because a module
that *did* add a property outside of a full `aa` reload — which the docs
don't rule out — would silently fail to show up in a module screen built
this way until the next `aa`.

**What I'd want the docs to say instead:** state explicitly that a
property's existence (as opposed to its content) only ever changes via
`aa` (added, as part of a full module reload) and `dp` (removed) — never
introduced piecemeal by `ap` — so a client can safely key structural
layout off `aa`/`dp` alone, the way `core/` now does.

---

## Testing / environment notes (not documentation findings)

- **A stray, already-running `ostserver` instance predated this session**
  (started earlier the same day, before this task began, with the exact
  launch command this task's brief documents) and was already listening on
  9624. My first attempt to start a *second* instance failed to bind the
  port (logged, not silent) but kept running otherwise (INDI connections
  etc.) until I killed it — at which point the *original*, pre-existing
  instance also disappeared (most likely both instances were racing for
  the same embedded-INDI FIFO/IPC and the survivor didn't survive the
  other's teardown). I started a fresh instance after that and used it for
  every subsequent test in this session; I never touched `ost.db` (checked
  its mtime before and after: unchanged, `2026-09-24 16:01`, predating this
  session entirely) and stopped my instance at the end of the session, per
  the task's instructions.
- **`--grant=0` on the launch command produced `grant-client: "1"`** (full
  read-write) in the live dump, not `"0"` (read-only) as the flag's value
  might suggest by analogy with the wire-level `grant-client` values table.
  This is a server *launch option*, not part of the client-facing wire
  protocol `dev-client/` documents, so it's noted here rather than as a
  documentation finding against those pages specifically — but worth
  knowing if anyone assumes the two are the same numbering.
- **DOM-level verification used the same method as ostharfang4's prior
  session**: no browser automation was available (Claude in Chrome not
  configured in this environment), so `UI1/app.js` and `UI2/js/app.js` were
  loaded as real ES modules via Node's native `import()` (not jsdom's own
  `<script type="module">` execution, which jsdom 16.7 — the version
  available to borrow from a sibling project's `node_modules`, same as
  ostharfang4's session — doesn't support), with `document`/`window`
  supplied by `jsdom` and `WebSocket` by Node 22's built-in global. This
  exercised real DOM construction (module list, property/element rendering,
  grid rendering) and one full click-driven `SV`/`ea` round trip through an
  actual rendered button, against the live server, for both UIs.
