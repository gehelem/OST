---
title: Data Model
weight: 10
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

On connection, send a `DU` request (see [Client → Server]({{< relref "client-messages.en.md" >}})).  
The server replies with a full dump message (`d`) containing all modules, controller data, and file lists.

---

## Modules

A module represents a functional subsystem (focus, guiding, allsky camera, etc.).  
Its identifier is its internal name (no spaces).

| Field | Type | Description |
|---|---|---|
| `infos.modulelabel` | string | Human-readable module label |
| `infos.description` | string | Module description |
| `infos.template` | string | Module template/type (e.g. `"focus"`, `"guider"`) — may be absent |
| `properties` | object | Map of property name → property object |
| `globallovs` | object | Map of LOV key → LOV object (module-level lists of values) |
| `profile.name` | string | Currently active profile name |
| `profile.changed` | bool | Whether the profile has unsaved changes |

{{% notice style="info" title="Wire format" %}}
In the JSON wire format, `properties` is abbreviated as `p`, `globallovs` as `l`, and `profile` as `f`. Clients must handle these abbreviated keys.
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
| `hasGraph` | bool | Property has a graph |
| `graphType` | string | `"XY"` · `"DY"` (time-axis) · `"PHD"` (guiding) |
| `graphParams` | object | Graph configuration |

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
| `preicon` | string | Icon before the element (optional) |
| `posticon` | string | Icon after the element (optional) |

{{% notice style="warning" title="Partial updates" %}}
In `ee` and `ea` update messages, elements contain **only their value** — no metadata fields. Clients must preserve metadata from the last full dump.
{{% /notice %}}

{{% notice style="tip" title="Field omission" %}}
`directedit` may be omitted entirely for elements where it doesn't apply — treat a missing `directedit` as `false`. This keeps dumps lighter for properties that don't need it.
{{% /notice %}}

{{% notice style="note" title="`directedit` meaning" %}}
`true` means this element can be updated on its own, independently of the property's other elements — send `SV` for it (`SA` also works). `false` means the client must submit **every** element of the property together — always use `SA`, never `SV` for that element alone. A good example: RA/Dec coordinates, where both values must be given together.

This is currently a convention the client is expected to follow, **not enforced by the backend** — sending an `SV` for a `directedit: false` element is not rejected today. A backend change to actually enforce this is planned but not yet implemented.
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
| `stars` | int | Number of detected stars |
| `issolved` | bool | Whether plate solving succeeded |
| `solverra` / `solverde` | float | Solved RA/Dec coordinates |
| `solverorientation` | float | Solved field rotation angle |
| `min` / `max` / `mean` / `median` / `stddev` | float[] | Per-channel statistics (see below) |
| `histogram` | array | Per-channel histogram data (see below) |
| `alternates` | string[] | Optional: alternative versions of the image |

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

The LOV itself is defined at module level (`globallovs`) or controller level (`controllerlovs`):
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

**Built-in controller LOVs** (always available):

| Key | Contents |
|---|---|
| `loadedModules` | All loaded modules: `{moduleName: label}` |
| `loadedModules-<template>` | Modules filtered by template (e.g. `loadedModules-focus`) |
| `profiles-<template>` | Available profiles for a module type |

---

## Access control

The dump response includes `grant-server` and `grant-client` values:

| Value | Meaning |
|---|---|
| `"1"` | Full read-write access |
| `"0"` | Read-only |
| `"-1"` | Access denied (login required) |

If `grant-client` is `"-1"`, no module data is sent. Use the `LO` (login) message to authenticate.
