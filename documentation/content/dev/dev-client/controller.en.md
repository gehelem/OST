---
title: Controller, GlobalDatastore and files
weight: 30
description: "What is not an ordinary module: controller data, system load, configurations, the GlobalDatastore module, the file system"
---

This page describes what falls outside the "modules → properties → elements" scheme of the previous pages: the **controller data**, the central **`GlobalDatastore`** module and the server's **file system**. A client that only displays modules does not need them; a client that wants to offer what a global settings page provides (configurations, INDI drivers, server load, equipment, images) has to handle them.

## Controller data (`controllerdata`)

The controller is the server component that loads the modules and drives the embedded INDI server. It exposes its own data in the dump, under the `controllerdata` key (see [`d`]({{% relref "server-messages.en.md" %}}#d)), then keeps it up to date with [`uc`]({{% relref "server-messages.en.md" %}}#uc) messages.

| Key | Content |
|---|---|
| `availableconfs` | Saved module configurations: object `configuration name → list of {modulelabel, moduletype, profilename}` |
| `currentconf` | Name of the current configuration |
| `banner` | Server banner (free text, `--banner` option) |
| `git` | Server version: `Gitdate`, `Githash`, `Gitmessage`, `Gittag` |
| `libraries` | Available module libraries, one metadata object per library — keys are **prefixed with `libost`** (`libostfocus`, `libostdummy`...); strip it before passing to [`ML`]({{% relref "client-messages.en.md" %}}#ml) |
| `profiles` | Available profiles: object `transformed library name → list of profile names` — see the keying convention below |
| `indiserver` | `"Y"` if the embedded INDI server is enabled, `"N"` otherwise |
| `indidrivers` | Catalogue of installed INDI drivers: list of `{binary, family, label, mdpd}` |
| `indiactivedrivers` | INDI drivers currently running |
| `systemwatcher` | Server load, see below |
| `files`, `folders` | Copy of `d.files.files` and `d.files.folders`, see [File system](#fichiers) |

{{% notice style="warning" title="`profiles` keying: not the library name as-is, not `infos.template`" %}}
`profiles` is keyed by the module's **library name** (the same string passed as `lib` to [`ML`]({{% relref "client-messages.en.md" %}}#ml), i.e. the key in `controllerdata.libraries` with the `libost` prefix stripped) — but transformed: **first letter uppercased, the rest lowercased** (`"focus"` → `"Focus"`, not a simple capitalize — a library name containing another uppercase letter would still be flattened, e.g. a hypothetical `"myFocus"` → `"Myfocus"`). This key is neither the module's instance name/label, nor necessarily identical to `infos.template` (they usually match in practice, but the two are set independently and nothing in the protocol guarantees it).

This convention is undocumented anywhere else and isn't enforced by the backend — it was reverse-engineered from Rooster's own lookup logic. Two independent clients built from this documentation alone got tripped up by it (a profile picker silently rendering empty). If you need to look up a module's available profiles, replicate this exact transform on the library name rather than guessing.
{{% /notice %}}

{{% notice style="warning" title="`uc`: one key per message, value replaced as a whole" %}}
A `uc` message carries **a single key** of `controllerdata`, and its value **entirely replaces** the old one (no merge). A client that only reads the first key of the payload (as Rooster does) is therefore correct as long as the server sends only one key per message, which is the case today.
{{% /notice %}}

### `systemwatcher`: server load

The server measures its own load at regular intervals and broadcasts it to all clients. The interval is set with the `--systemwatchinterval` option (seconds, **10 by default**, `0` disables it). The message is an ordinary `uc`, and the latest value is also present in `controllerdata.systemwatcher` of the dump.

```json
{
  "uc-update controller data": {
    "systemwatcher": {
      "cpu": { "load_1m": 10.42, "load_5m": 9.79, "load_15m": 4.51 },
      "ram": { "used_mb": 12194, "total_mb": 31941 },
      "disks": [
        { "device": "/dev/nvme0n1p2", "mountpoint": "/", "used_gb": 789.4, "total_gb": 915.3 }
      ],
      "network": [
        { "iface": "enp4s0", "rx_kbps": 16.27, "tx_kbps": 448.8 }
      ],
      "temperatures": [
        { "type": "Tctl", "temp_c": 75.375 }
      ]
    }
  }
}
```

| Field | Meaning |
|---|---|
| `cpu.load_1m` / `load_5m` / `load_15m` | System **load average** (as in `/proc/loadavg`), **not a utilisation percentage**: 1.0 = one core saturated on average |
| `ram.used_mb` / `total_mb` | Used memory (total − available) and total memory, in MB |
| `disks[]` | One entry per mounted storage device (`/dev/*`, excluding `loop`): `device`, `mountpoint`, `used_gb`, `total_gb` |
| `network[]` | One entry per interface (excluding `lo`): `rx_kbps` / `tx_kbps` rates measured between two messages; the first measurement of an interface is 0 |
| `temperatures[]` | Processor hardware sensors: `type`, `temp_c`. **The list can be empty** depending on the machine (only Intel `coretemp` and AMD `k10temp` sensors are read) |

{{% notice style="tip" title="Don't assume the arrays are filled" %}}
`disks`, `network` and especially `temperatures` can be empty. A client should hide the corresponding section rather than display an empty array. The protocol does not provide the server's number of cores: a CPU gauge therefore cannot be a real percentage, the value has to be shown as a load.
{{% /notice %}}

## Configurations and module lifecycle

A **configuration** is a named set of modules (library, label, profile) that the server can reload in one go. The list is in `controllerdata.availableconfs`, the active configuration in `controllerdata.currentconf`.

| Action | Command | Detail |
|---|---|---|
| Load a configuration | [`CL`]({{% relref "client-messages.en.md" %}}#cl) | Loads all the modules of the configuration |
| Save the current configuration | [`CS`]({{% relref "client-messages.en.md" %}}#cs) | Under the given name |
| Load a module | [`ML`]({{% relref "client-messages.en.md" %}}#ml) | Library, label, optional profile |
| Unload a module | [`MK`]({{% relref "client-messages.en.md" %}}#mk) | By module name |

The modules actually loaded are those of the dump (`d.m`); they appear and disappear through [`aa`]({{% relref "server-messages.en.md" %}}#aa) and [`dm`]({{% relref "server-messages.en.md" %}}#dm). A configuration **never** contains `GlobalDatastore` (see below), which is always present.

{{% notice style="warning" title="A configuration silently includes the active INDI drivers" %}}
`CS` also saves the currently running INDI drivers alongside the module list, and `CL` restarts them — but this is invisible in `controllerdata.availableconfs`, whose shape only describes modules (`{modulelabel, moduletype, profilename}`), never drivers. A client building a configuration editor that only shows/edits the module list is not showing the whole picture of what will actually happen on `CL`.
{{% /notice %}}

{{% notice style="note" title="No delete operation" %}}
There is no command to remove a saved configuration — `CL` and `CS` are the only two operations. A configuration a client no longer wants stays in `controllerdata.availableconfs` indefinitely (it can only be overwritten under the same name by another `CS`).
{{% /notice %}}

### INDI server and drivers

If `controllerdata.indiserver` is `"Y"`, the server drives an embedded INDI instance. The [`YA`/`YZ`/`YL`/`YR`/`YS`]({{% relref "client-messages.en.md" %}}#indi) commands start it, stop it and manage the drivers; `controllerdata.indidrivers` gives the available catalogue and `controllerdata.indiactivedrivers` what is running. `binary` is the value to pass as `driver` in `YL`, `YR` and `YS`.

`controllerdata.indidrivers` and `controllerdata.indiactivedrivers` are both lists of objects `{binary, family, label}` (`indidrivers` additionally has `mdpd`, see the table above).

{{% notice style="warning" title="YA/YZ/YL/YR/YS have no dedicated acknowledgment" %}}
None of these commands trigger a confirmation message of their own. Success or failure has to be inferred indirectly: `YL`/`YR`/`YS` are followed by a `uc` update of `indiactivedrivers` reflecting the requested change (but this only confirms the server *attempted* it, not that the underlying INDI driver actually came up), while `YA`/`YZ` produce no `uc` at all — only `l` (log) messages, which are meant for a human, not for a client to parse programmatically. `controllerdata.indiserver` itself reflects the `--indiserver` launch option, not whether the embedded server is actually running right now.
{{% /notice %}}

## The `GlobalDatastore` module {#globaldatastore}

`GlobalDatastore` is a module **like any other in the dump** (`d.m.GlobalDatastore`, with properties, elements, grids, profiles), but its role is central:

- it is created by the server **before all other modules** and is **always present**: `MK` on it is refused, and it is never included in a configuration;
- it holds the **shared equipment reference** that all the other modules consult: optics, locations, equipments (sets of devices) and INDI servers.

Its grids feed lists of values (LOVs) of the same name, present in `d.lovs` (`optics`, `locations`, `equipments`, `servers`) and rebuilt on every change of the grid. The other modules use them like this: choosing an optic fills in the focal length and diameter, choosing a location fills in latitude, longitude and altitude, choosing an equipment set resolves the devices (camera, mount, focuser, filter wheel, guide camera, GPS, dome, weather) and the INDI server to contact.

Exposed properties:

| Property | Role |
|---|---|
| `devices` | Selected equipment set and GPS (`equipments`, `gps`) |
| `devicesactions` | Buttons: connect/disconnect the devices, load the configurations (`condevs`, `discondevs`, `loadconfs`) |
| `server` | INDI server host and port (`host`, `port`) |
| `serveractions` | Buttons: connect/disconnect the INDI server (`conserv`, `disconserv`) |
| `startup` | Auto-start checkboxes (`indiatstart`, `devatstart`, `confsatstart`) |
| `equipments` | Grid of equipment sets |
| `locations` | Grid of locations (icons `refresh` = read the GPS, `add` = add the current line) |
| `optics` | Grid of optics |
| `servers` | Grid of INDI servers |

All its properties are handled with the standard messages (`SV`, `SA`, `GC`…, `I3`/`I4` on `locations`); a generic client displays them like any other module. Rooster presents them in a dedicated tab of its settings page, keeping the properties that have a profile (`hasprofile`) and grouping them by `level1`/`level2`.

## File system {#fichiers}

The server publishes the list of the files it produces (images, sequences, archives) in the `--webroot` folder, without the client having to query for it. This mechanism is independent of the modules.

### In the dump: `d.files`

```json
"files": {
  "folders": ["", "/Allsky", "/Sequencer/gam Cyg/LIGHT/Luminance"],
  "files": ["/image1.jpeg", "/report.txt"],
  "selectedfolder": "/home/gilles/ostmedia"
}
```

| Field | Content |
|---|---|
| `folders` | **All** the sub-folders of the webroot, recursively, paths relative to the webroot starting with `/`. The root is the empty string `""`. Not sorted. |
| `files` | The files of the **selected folder only** (not recursive), paths relative to the webroot starting with `/`. All files, no extension filter. |
| `selectedfolder` | The selected folder, as an **absolute path on the server's file system** (not a relative one). |

There is no pagination and no size limit: on a very full webroot, the list can be long.

### Choosing the folder: `FS`

[`FS`]({{% relref "client-messages.en.md" %}}#fs) designates the folder to watch and list files from. The server watches **a single** folder for file content at a time; the folder tree itself (`folders`) is tracked independently, see below.

{{% notice style="warning" title="The selected folder is server-wide" %}}
There is no per-client folder: an `FS` changes the file list seen by **all** connected clients. A client should not use it for simple "local" browsing if other clients may be connected.

The path **is validated**: the target must exist and resolve inside the webroot (no `../` escape). An invalid request is rejected server-side (logged, no state change) — it does not silently return an empty list.
{{% /notice %}}

A successful `FS` replaces the file list wholesale: the client receives one [`uc`]({{% relref "server-messages.en.md" %}}#uc) `files` message with the complete new list, not a stream of `af`/`df` — selecting a folder is not a filesystem mutation, nothing was actually added or removed. There is **no** message announcing the new `selectedfolder` itself, which only exists in the dump.

### Folder tree vs. selected folder's files

`folders` (the whole recursive tree) and `files` (the selected folder's content) do not share the same scope:

| | Scope | Live updates |
|---|---|---|
| `folders` / `ad` / `dd` | The **whole webroot**, recursively — independent of what is currently selected | Always, regardless of the current `FS` selection |
| `files` / `af` / `df` | **The selected folder only** | Only for changes to the folder the last `FS` selected |

A folder created or deleted anywhere in the webroot is reported live via `ad`/`dd` even if no client currently has it selected. A file created or deleted is only reported via `af`/`df` if it happens in the folder a client's `FS` last selected — `files`/`af`/`df` say nothing about other folders' content, a client has to `FS` into them to find out.

### Updates: `af` / `df` / `ad` / `dd`

When the folder tree or the selected folder's content actually changes, the server sends one message per affected path — the value is the bare path itself, not wrapped in an object or array:

```json
{ "af": "/Sequencer/gam Cyg/LIGHT/Luminance/img_001.FITS" }
{ "df": "/report.txt" }
{ "ad": "/Sequencer/new target" }
```

| Key | Meaning |
|---|---|
| `af` | File appeared in the selected folder |
| `df` | File disappeared from the selected folder |
| `ad` | Sub-folder appeared anywhere in the webroot |
| `dd` | Sub-folder disappeared anywhere in the webroot |

Each message carries exactly one path (N files = N messages) — see [`af`/`df`/`ad`/`dd`]({{% relref "server-messages.en.md" %}}#af) for the full reference. A rename is reported as a deletion followed by a creation: the server diffs a before/after listing, it does not detect renames as such.

After the per-path messages, the server sends [`uc`]({{% relref "server-messages.en.md" %}}#uc) `files` and/or `folders` with the complete updated list — but only for whichever of the two actually changed on this pass, never unconditionally. A client can therefore either apply the per-path events incrementally, or simply replace its list(s) with the matching `uc` message(s).

### Building a file URL

The paths in `files` are relative to the webroot and start with `/`. A file's URL is the server's media base URL (`http(s)://hostname/ostmedia/`, see the "Media URLs" note of the [Data model]({{% relref "structure.en.md" %}})) followed by the path. Mind the leading `/`: concatenating it to a base that already ends with `/` gives a double `//`.

### What the event does not do

The server detects the **addition, removal and renaming** of files, not a change to the content of an existing file: an image rewritten under the same name produces no event. This is the case for the modules' preview images, whose URL stays identical on every shot: a client must add a cache-busting parameter (`?t=` + timestamp) when it reloads an image after an update of its property.
