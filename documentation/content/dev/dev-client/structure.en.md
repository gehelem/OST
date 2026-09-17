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
| `infos.label` | string | Human-readable module label |
| `infos.name` | string | Internal module name |
| `infos.description` | string | Module description |
| `infos.template` | string | Module template/type (e.g. `"focus"`, `"guider"`) |
| `properties` | object | Map of property name → property object |
| `globallovs` | object | Map of LOV key → LOV object (module-level lists of values) |
| `profile.name` | string | Currently active profile name |
| `profile.changed` | bool | Whether the profile has unsaved changes |

{{% notice style="info" title="Wire format" %}}
In the JSON wire format, `properties` is abbreviated as `p`, `globallovs` as `l`, and `profile` as `f`. Clients must handle these abbreviated keys.
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
| `directedit` | bool | `true` = send `SV` on change; `false` = batch with `SA` |
| `preicon` | string | Icon before the element (optional) |
| `posticon` | string | Icon after the element (optional) |

{{% notice style="warning" title="Partial updates" %}}
In `ee` and `ea` update messages, elements contain **only their value** — no metadata fields. Clients must preserve metadata from the last full dump.
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
| `min` / `max` / `mean` / `median` / `stddev` | float[] | Per-channel statistics |
| `histogram` | array | Per-channel histogram data |
| `alternates` | string[] | Optional: alternative versions of the image |

{{% notice style="tip" title="Media URLs" %}}
All image/video paths are relative. Prepend `http(s)://hostname/ostmedia/` to get the full URL.  
Example: `"urljpeg": "Focus/frame.jpeg"` → `http://hostname/ostmedia/Focus/frame.jpeg`
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
