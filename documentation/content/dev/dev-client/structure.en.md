---
title: Data Model
weight: 20
description: "Hierarchy of modules, properties, and elements exchanged over WebSocket"
---

## Overview

All data is organized in a three-level hierarchy:

```
Module
  └─ Property (one or many per module)
       └─ Element (one or many per property)
```

---

## Connection and initial state

### Finding the server

A **web** client is served *by* the OST server itself, so it already knows its host — the WebSocket URL should be derived from the page's own URL, not asked from the user:

| Page URL | WebSocket URL |
|---|---|
| `http://<host>/` | `ws://<host>:9624` (direct, fixed port) |
| `https://<host>/` | `wss://<host>/ws/` (implicit port 443, through a reverse proxy path — SSL setups don't expose 9624 directly) |

This is a convention, not something the protocol enforces — but every existing client follows it, and a client that instead asks the user to type a WebSocket URL by hand is solving a problem that doesn't exist for the web case.

A **non-web** client (mobile, desktop) has no page URL to derive anything from, and needs the host to be supplied some other way:
- **mDNS/Zeroconf**: the server advertises itself on the local network as service type `_ostserver_ws._tcp` in the `local` domain, with a TXT record `OstServer=Observatoire Sans Tete`. The advertised port is always `9624`. This lets a client **scan for available servers** on the network rather than requiring the user to know an IP — useful for a first connection, or whenever there may be more than one OST server around.
- **Manual entry, remembered**: whether discovered or typed in, a client should keep a small history of previously-used servers (host/port/secure) so reconnecting later doesn't require repeating the discovery or entry step. This is a client-side convenience, not part of the protocol.

Either way, the **media** base URL (for images/video, see the "Media URLs" note further down) follows the same host, with the standard HTTP(S) port (80/443) rather than 9624 — a non-web client needs this host too, for exactly the same reason it needs the WebSocket host.

On connection, send a `DU` request (see [Client → Server]({{< relref "client-messages.en.md" >}})).  
The server replies with a full dump message (`d`) containing all modules, controller data, and file lists.

Controller-level data (server load, module configurations, INDI drivers, the file system) is not part of the module model described on this page — see [Controller, GlobalDatastore and files]({{% relref "controller.en.md" %}}).

---

## Modules

A module represents a functional subsystem (focus, guiding, allsky camera, etc.).  
Its identifier is its internal name (no spaces).

| Field | Type | Description |
|---|---|---|
| `infos.modulelabel` | string | Human-readable module label |
| `infos.description` | string | Module description |
| `infos.template` | string | Display hint for the frontend (which rendering to use), e.g. `"focus"`, `"guider"` — purely cosmetic, may be absent |
| `infos.library` | string | The module's type, lowercase (the library it was loaded from, e.g. `"focus"`) |
| `infos.classname` | string | The module's type, exact case (e.g. `"Focus"`) |
| `infos.families` | array of string | Capabilities this module instance declares (e.g. `["guiding"]`) — zero, one, or several |
| `properties` | object | Map of property name → property object |
| `globallovs` | object | Map of LOV key → LOV object (module-level lists of values) |
| `profile.name` | string | Currently active profile name |
| `profile.changed` | bool | Whether the profile has unsaved changes |

{{% notice style="info" title="Wire format" %}}
In the JSON wire format, `properties` is abbreviated as `p`, `globallovs` as `l`, and `profile` as `f`. Clients must handle these abbreviated keys.
{{% /notice %}}

{{% notice style="warning" title="type, template and family are 3 different things" %}}
It's tempting to treat `infos.template` as the module's identity — don't. These are 3 independent notions:
- **type** (`infos.library`/`infos.classname`) is the module's real identity — the library it was loaded from. Two instances of the same type (e.g. two Focus modules) always share the same type.
- **template** (`infos.template`) is only a display hint for the frontend, purely cosmetic and entirely optional. Two different types can share the same generic template (e.g. anything rendered with `"default"`).
- **family** (`infos.families`) is a capability a module declares about itself, independent of both type and template. A module can declare zero, one, or several families — it exists so a client can target "any module that can guide", say, without caring whether the actual type is `Guider` or a different implementation of the same capability.

`loadedModules-<type>` and `loadedFamilies-<family>` (below) key off the lowercase `infos.library`/`infos.families`; `profiles-<Type>` keys off the exact-case `infos.classname` instead. Use the matching field directly — never derive one from the other by transforming case client-side.
{{% /notice %}}

{{% notice style="note" title="Internal fields" %}}
`infos` also carries a number of internal build/version fields (`baseGithash`, `thisversion`, `startdatetime`, and others) — implementation detail, not part of the stable client contract, and not documented individually here.
{{% /notice %}}

---

## Properties

A property groups related elements. Properties are displayed hierarchically using `level1` and `level2`.

| Field | Type | Description |
|---|---|---|
| `label` | string | Display label |
| `order` | string | Sort order within the same level |
| `level1` | string | First hierarchy level (e.g. tab name) |
| `level2` | string | Second hierarchy level (e.g. group name) |
| `status` | int | `0` Standby · `1` OK (green) · `2` Busy (yellow) · `3` Error (red) |
| `permission` | int | `0` Read-only · `1` Write-only · `2` Read-write |
| `enabled` | bool | Whether the property can be interacted with |
| `badge` | bool | Whether the property is in the favourites list |
| `preicon1` | string | Icon before the label (Google Fonts icon name) |
| `preicon2` | string | Second icon before the label |
| `posticon1` | string | Icon after the label |
| `posticon2` | string | Second icon after the label |
| `showElts` | bool | Whether element values should be shown inline |
| `hasprofile` | bool | Whether the property is saved in profiles |
| `freevalue` | string | Arbitrary free-text field |
| `rule` | int | Bool element grouping rule: `0` OneOfMany (radio) · `1` AtMostOne · `2` Any |
| `elements` | object | Map of element name → element object |

{{% notice style="tip" title="`preicon`/`posticon`: clickable actions, not decorations" %}}
A non-empty `preicon1`/`preicon2`/`posticon1`/`posticon2` value is the Google Fonts (Material Icons) icon name to render — and it's clickable: clicking it sends [`I1`/`I2`/`I3`/`I4`]({{% relref "client-messages.en.md" %}}#i1) with no value or parameter, just an empty payload targeting the property. An **empty string means there is no icon at all** for that slot — nothing to render, nothing to click, nothing to send. The element-level `preicon`/`posticon` below work the same way, via [`J1`/`J2`]({{% relref "client-messages.en.md" %}}#j1) instead. See [Icon actions]({{% relref "client-messages.en.md" %}}#icon-actions) for the full message reference.
{{% /notice %}}

{{% notice style="warning" title="`permission` governs editable elements, not whether to display the value" %}}
`permission: 1` ("write-only") does not mean "hide the current value" — it means the property has nothing meaningful to read back as a submittable input (a fire-and-forget action). A property whose only element is a display-only type (`img`, `video`, `light`, `prg`, `message`) can legitimately be `permission: 1` — a live camera preview is a real example — and must still be rendered from its value regardless of `permission`. Gate interactivity (showing an input, enabling a submit) on `permission`, but gate rendering on the element **type**, not on `permission`.
{{% /notice %}}

{{% notice style="note" title="`rule` only means something when every element is `bool`" %}}
`rule` is present on every property (default `0`), but it only has meaning — and should only be acted on — when **all** of the property's elements are of type `bool`. It also shows up, inertly, on properties whose elements are a mix of other types (e.g. a coordinate pair of two `float` elements) — a client should ignore it there rather than try to render, say, two floats as a radio group.
{{% /notice %}}

{{% notice style="note" title="`badge` is set by the module author, not by a client at runtime" %}}
There is no client message that toggles `badge` — it is fixed per-property/per-element in the module's own definition, not a per-user, per-session favourite a client can set. A "favourites" screen can read and aggregate what's already flagged, but has no way to let a user mark something as a favourite themselves.
{{% /notice %}}

{{% notice style="tip" title="`level1`/`level2`/`order` are advisory, not a contract" %}}
These fields are purely informative — the backend never enforces them, and a client is free to honor them partially or not at all. They only express how a given module's developer imagined organizing its own properties. This is exactly what lets generic clients (Rooster, ostnoctua) work with any module without custom UI per module.

The reference client (Rooster) groups properties this way, for clients that want a similar generic layout:
1. Empty `level1` → property shown outside any tab (rendered directly, sorted by `order`).
2. Non-empty `level1` → grouped into a tab named `level1`. Inside it, empty `level2` → shown directly under the tab; non-empty `level2` → shown inside a collapsible group named `level2`.
3. Tab order (and group order within a tab) isn't a separate field — it's derived from the *smallest* `order` value among the properties in that group.
4. Within a group, properties are sorted by their own `order`.

Two things worth knowing if you build something similar:
- `level1`/`level2` form a "fake hierarchy" by design choice, not a guaranteed two-level structure — nothing stops a module from leaving `level1` empty while setting `level2` (Rooster doesn't even handle that combination specially; the property just lands in the "no level1" bucket, `level2` ignored).
- `level1`/`level2` are translatable labels, going through the same mechanism as `label`/`hint`. Keying UI state (e.g. an expanded/collapsed panel) off the raw string value is fragile across a client's language changes — not a bug, just worth avoiding.
{{% /notice %}}

**Grid properties** (when `hasGrid: true`):

| Field | Type | Description |
|---|---|---|
| `hasGrid` | bool | Property has tabular grid data |
| `showGrid` | bool | Grid should be shown by default |
| `gridLimit` | int | Maximum number of grid rows |
| `gridheaders` | string[] | Column order (element names) |
| `grid` | array of arrays | Grid rows; each row is an array of values in `gridheaders` order |

**Graph properties** (when `hasGraph: true`):

| Field | Type | Description |
|---|---|---|
| `hasGraph` | bool | Property has a graph — **always present**, even when `false` |
| `graphType` | string | `"XY"` · `"DY"` · `"SXY"` · `"SDY"` · `"PHD"` — see below |
| `graphParams` | object | Which element supplies which chart role — see below |

A graph property is always also a grid property (`hasGrid: true`, see above): each grid row is one data point, and `graphParams` maps an abstract chart role to the **element key** (one of `gridheaders`) whose value in that row supplies it.

| `graphType` | Shape | `graphParams` roles |
|---|---|---|
| `XY` | Single scatter series, fixed axis bounds | `X`, `Y`, `Xmin`, `Xmax`, `Ymin`, `Ymax`, plus optionally `graphColor` (below) |
| `DY` | Single series, date/time on X | `D` (date/time element), `Y`, plus optionally `graphColor` (below) |
| `SXY` | Multiple scatter series, grouped by a series key | `S` (series-name element), `X`, `Y`, plus optionally `graphColors`/`axis`/`mapping` (below) |
| `SDY` | Multiple series, date/time on X, grouped by a series key | `S`, `D`, `Y`, plus optionally `graphColors`/`axis`/`mapping` |
| `PHD` | Dedicated PHD2-style guiding graph (drift + SNR + RMS) | `D`, `RA`, `DE`, `pRA`, `pDE`, `RSB`, `RMS` — fixed, purpose-specific roles |

Optional `graphParams` fields:

| Field | Used by | Description |
|---|---|---|
| `graphColor` | `XY`, `DY` | `{"R": int, "G": int, "B": int}` — color for the single series |
| `graphColors` | `SXY`, `SDY` | `{"<series>": {"R": int, "G": int, "B": int}}` — color per series |
| `axis` | `SXY`, `SDY` | `{"<name>": {"min": number, "max": number, "pos": "left"\|"right", "text": string}}` — one or more Y axes |
| `mapping` | `SXY`, `SDY` | `{"<series>": "<axis name>"}` — which entry of `axis` each series plots against |

{{% notice style="warning" title="`PHD` is not a generic chart type" %}}
Unlike `XY`/`DY`/`SXY`/`SDY`, whose roles are generic enough to drive a single reusable chart component, `PHD` has fixed, purpose-built roles for a PHD2-style guiding graph (used by the `guider` module). A client building one generic graph component for the other four types will likely need a dedicated implementation for `PHD`, or to skip it — Rooster currently does the latter (`SXY`/`SDY`/`XY`/`DY` only, `PHD` unhandled).
{{% /notice %}}

{{% notice style="tip" title="Render graphs as live components, not generated images" %}}
A graph property's grid data changes over time (`gc`/`gu`/`gd`/`gr`, or a fresh `uc`/`ap`/`aa`), so treat it like any other live UI element: draw it with a charting/canvas component that redraws on each update, the way Rooster does with Chart.js. Do not render it server- or client-side into a static image (a PNG snapshot, a server-rendered plot) and just swap the `<img>` — it looks noticeably worse than a real chart (no smooth updates, no interactivity, visible flicker/reload on every point), and it throws away the whole point of getting live grid data over the wire in the first place.
{{% /notice %}}

{{% notice style="info" title="Wire format" %}}
In the JSON wire format, `elements` is abbreviated as `e`.
{{% /notice %}}

---

## Elements

An element holds a single typed value. There are 11 element types.

### Common fields (all elements, in full dump)

| Field | Type | Description |
|---|---|---|
| `type` | string | Element type (see list below) |
| `label` | string | Display label |
| `order` | string | Sort order within the property |
| `hint` | string | Tooltip/help text |
| `autoupdate` | bool | Internal backend flag; not used by clients |
| `badge` | bool | Whether the element is in the favourites list |
| `directedit` | bool | Whether this element can be written alone via `SV` — see below |
| `preicon` | string | Icon before the element — clickable action, see the note above (optional) |
| `posticon` | string | Icon after the element — clickable action, see the note above (optional) |

{{% notice style="warning" title="Partial updates" %}}
In `ee` and `ea` update messages, elements contain **only their value** — no metadata fields. Clients must preserve metadata from the last full dump.
{{% /notice %}}

{{% notice style="tip" title="Field omission" %}}
`directedit` may be omitted entirely for elements where it doesn't apply — treat a missing `directedit` as `false`. This keeps dumps lighter for properties that don't need it.
{{% /notice %}}

{{% notice style="note" title="`directedit` meaning" %}}
`true` means this element can be updated on its own, independently of the property's other elements — send `SV` for it (`SA` also works). `false` means the client must submit **every** element of the property together — always use `SA`, never `SV` for that element alone. A good example: RA/Dec coordinates, where both values must be given together.

The backend enforces this: an `SV` on a `directedit: false` element is rejected with an error in the module log, and nothing is applied. `SA` is always accepted.

Choose the value by how the element behaves, not by whether it feels "related" to its neighbours:
- **`true` for an independent element** — a `bool` that can be true regardless of its siblings (an action button such as start/abort/loop, a toggle) must be `directedit: true`. Such a value often reflects an ongoing activity, and several can legitimately be true at once (e.g. a capture loop running while a focus-out is requested). Declaring it `false` would wrongly claim the elements must be submitted as a group, and a client following this page would send `SA`, bypassing modules that only handle `SV`.
- **`false` only for a real group** — elements that are only meaningful together: RA/Dec coordinates, or an INDI switch vector mirrored one-to-one.

For a `false` group of `bool` elements under an exclusive rule (`OneOfMany` or `AtMostOne`), selecting one element means sending it `true` and **every sibling `false`** in the same `SA` — never resend the siblings' current values, or two elements end up `true`.
{{% /notice %}}

---

### Type: `int`

| Field | Type | Description |
|---|---|---|
| `value` | int | Current integer value |
| `min` | int | Minimum allowed value |
| `max` | int | Maximum allowed value |
| `step` | int | Increment step |
| `format` | string | Display format string |
| `slider` | int | `0` no slider · `1` slider only · `2` slider + value input |
| `listOfValues` | object | Optional: `{"key": "Label"}` map of allowed values |
| `globallov` | string | Optional: key referencing a module or controller LOV |
| `lovScope` | string | `"module"` or `"controller"` — where the globallov is defined |
| `lovConstrained` | bool | Whether value must belong to the LOV |
| `nullable` | bool | Whether the element may be left empty (frontend should offer a "none" choice) |

### Type: `float`

Same fields as `int`, with floating-point `value`, `min`, `max`, `step`.

### Type: `bool`

| Field | Type | Description |
|---|---|---|
| `value` | bool | `true` or `false` |

### Type: `string`

| Field | Type | Description |
|---|---|---|
| `value` | string | Text value |
| `listOfValues` | object | Optional: `{"key": "Label"}` map of allowed values |
| `globallov` | string | Optional: key referencing a LOV |
| `lovScope` | string | `"module"` or `"controller"` |
| `lovConstrained` | bool | Whether value must belong to the LOV |
| `nullable` | bool | Whether the element may be left empty (frontend should offer a "none" choice) |

### Type: `date`

| Field | Type | Description |
|---|---|---|
| `value` | object | `{"year": int, "month": int (1-12), "day": int (1-31)}` |

### Type: `time`

| Field | Type | Description |
|---|---|---|
| `value` | object | `{"hh": int, "mm": int, "ss": int, "ms": int}` |
| `usems` | bool | Whether milliseconds should be displayed |

### Type: `datetime`

| Field | Type | Description |
|---|---|---|
| `value` | object | `{"year": int, "month": int, "day": int, "hh": int, "mm": int, "ss": int, "ms": int}` |

### Type: `img`

The `value` field is an object with image metadata:

| Field | Type | Description |
|---|---|---|
| `urljpeg` | string | Relative path to JPEG image (use with `/ostmedia/` base URL) |
| `urlfits` | string | Relative path to FITS file |
| `urlthumbnail` | string | Relative path to thumbnail |
| `urloverlay` | string | Relative path to overlay image |
| `channels` | int | Number of colour channels |
| `width` / `height` | int | Image dimensions in pixels |
| `snr` | float | Signal-to-noise ratio |
| `hfravg` | float | Average HFR (half-flux radius) of detected stars |
| `hfravgdev` | float | Standard deviation of `hfravg` |
| `stars` | int | Number of detected stars |
| `issolved` | bool | Whether plate solving succeeded |
| `solverra` / `solverde` | float | Solved RA/Dec coordinates |
| `solverorientation` | float | Solved field rotation angle |
| `min` / `max` / `mean` / `median` / `stddev` | float[] | Per-channel statistics (see below) |
| `histogram` | array | Per-channel histogram data (see below) |
| `alternates` | string[] | Optional: alternative versions of the image |
| `showstats` | bool | Set by the module developer: whether the statistics fields above are meaningful for this image and should be shown (a module may set this `false` if it doesn't compute them, or they don't apply) |

**Per-channel statistics**: `min`/`max`/`mean`/`median`/`stddev` are
**fixed 3-slot arrays**, regardless of the actual number of channels —
index `0` is the single channel (mono image) or red (R), `1` is green
(G), `2` is blue (B). Only the first `channels` slots are meaningful;
beyond that, values are zero and carry no meaning (a mono image only
has index `0` populated).

{{% notice style="note" title="Histogram: bin structure" %}}
`histogram` follows the same per-channel logic (one array per channel,
only the first `channels` used), but has two quirks worth knowing
before using it:

- **Each bin is itself wrapped in a one-element array**:
  `histogram[channel][bin]` is `[frequency]`, not `frequency` directly
  — an artifact of the current backend serialization, not a client
  parsing error.
- **The pixel value each bin represents is not sent directly**, only
  the frequency is. The number of bins varies per image
  (`histogram[channel].length`); the pixel value of bin `i` is
  recomputed from that same channel's `min`/`max`:
  ```
  bin_width = (max[channel] - min[channel]) / (bin_count - 1)
  value(i)  = min[channel] + i × bin_width
  ```
{{% /notice %}}

{{% notice style="tip" title="Media URLs" %}}
All image/video paths are relative. Prepend `http(s)://hostname/ostmedia/` to get the full URL.  
Example: `"urljpeg": "Focus/frame.jpeg"` → `http://hostname/ostmedia/Focus/frame.jpeg`
{{% /notice %}}

{{% notice style="warning" title="Media is served separately from the WebSocket port" %}}
That `hostname` is **not derivable from the protocol** — no field in the dump gives it to you, and it is **not** the WebSocket connection you just made. In the reference deployment (see [Fresh install]({{% relref "fresh.en.md" %}})), media is served by a separate web server (nginx, port 80 by default, 443 behind SSL) — a completely different port from the WebSocket server (9624). A client must already know, or let the user configure, the OST server's hostname independently of the WebSocket URL, and build media URLs against that hostname's HTTP port — not against the page's own origin, and not against the WebSocket port. A client that just prepends a bare `/ostmedia/` path (resolving it against wherever its own files happen to be served) will get broken images.
{{% /notice %}}

### Type: `video`

| Field | Type | Description |
|---|---|---|
| `value` | object | `{"url": "relative/path/to/video.mp4"}` |

In partial update (`ea`) messages, the video element may contain `url` directly at the element level rather than nested in `value`.

### Type: `light`

| Field | Type | Description |
|---|---|---|
| `value` | int | `0` Standby · `1` OK · `2` Warning · `3` Error |

### Type: `prg`

| Field | Type | Description |
|---|---|---|
| `value` | object | `{"value": float (0–100), "dynlabel": string}` |
| `prgtype` | string | `"bar"` or `"spinner"` |

### Type: `message` (not yet implemented in current backend)

Reserved for log/message display.

---

## Lists of Values (LOV)

LOVs provide a set of allowed values for `int`, `float`, and `string` elements.

**Local LOV** — embedded in the element:
```json
"listOfValues": {
  "1": "Option One",
  "2": "Option Two"
}
```

**Global LOV** — referenced by key:
```json
"globallov": "myCameraModels",
"lovScope": "module"
```

The LOV itself is defined at module level (`globallovs`) or controller level (`lovs`):
```json
"myCameraModels": {
  "label": "Camera models",
  "type": "string",
  "values": {
    "ASI294MC": "ZWO ASI 294 MC",
    "ASI533MC": "ZWO ASI 533 MC"
  }
}
```

**Built-in controller LOVs**:

| Key | Contents | Available |
|---|---|---|
| `loadedModules` | All loaded modules: `{moduleName: label}` | Always |
| `loadedModules-<type>` | Modules of a given type, matching `infos.library` (e.g. `loadedModules-focus`) | Once a module of that type is loaded |
| `loadedFamilies-<family>` | Modules declaring a given family, matching `infos.families` (e.g. `loadedFamilies-guiding` lists every loaded module that can guide, whatever its actual type) | Once a module declaring that family is loaded |
| `profiles-<Type>` | Available profiles for a module type, matching `infos.classname` exactly — note the capitalization, e.g. `profiles-Focus` | Once a profile of that type has been saved |

---

## Access control

The dump response includes `grant-server` and `grant-client` values:

| Value | Meaning |
|---|---|
| `"1"` | Full read-write access |
| `"0"` | Read-only |
| `"-1"` | Access denied (login required) |

If `grant-client` is `"-1"`, no module data is sent. Use the `LO` (login) message to authenticate.
