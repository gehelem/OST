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
| `libraries` | Available module libraries, one metadata object per library |
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

### INDI server and drivers

If `controllerdata.indiserver` is `"Y"`, the server drives an embedded INDI instance. The [`YA`/`YZ`/`YL`/`YR`/`YS`]({{% relref "client-messages.en.md" %}}#indi) commands start it, stop it and manage the drivers; `controllerdata.indidrivers` gives the available catalogue and `controllerdata.indiactivedrivers` what is running. `binary` is the value to pass as `driver` in `YL`, `YR` and `YS`.

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

[`FS`]({{% relref "client-messages.en.md" %}}#fs) designates the folder whose file list you want. The server watches **a single** folder at a time.

{{% notice style="warning" title="The selected folder is server-wide" %}}
There is no per-client folder: an `FS` changes the file list seen by **all** connected clients (the events below are broadcast to everyone). A client should not use it for simple "local" browsing if other clients may be connected.

The server **does not validate** the path: a non-existent folder simply returns an empty list, without an error.
{{% /notice %}}

### Updates: `af` / `df` / `ad` / `dd`

When the content of the selected folder changes (file created, deleted or renamed), or after an `FS`, the server sends one message per affected path — the value is the bare path itself, not wrapped in an object or array:

```json
{ "af": "/Sequencer/gam Cyg/LIGHT/Luminance/img_001.FITS" }
{ "df": "/report.txt" }
```

| Key | Meaning |
|---|---|
| `af` | File appeared in the selected folder |
| `df` | File disappeared from the selected folder |
| `ad` | Sub-folder created in the webroot |
| `dd` | Sub-folder deleted from the webroot |

Each message carries exactly one path (N files = N messages) — see [`af`/`df`/`ad`/`dd`]({{% relref "server-messages.en.md" %}}#af) for the full reference. After an `FS`, the client receives a `df` for each file of the old folder, then an `af` for each file of the new one: there is **no** message announcing the new `selectedfolder`, which only exists in the dump.

On every pass the server also sends [`uc`]({{% relref "server-messages.en.md" %}}#uc) messages `files` and `folders` containing the **complete** lists: a client can therefore either apply the events one by one, or simply replace its lists with these `uc` messages.

### Building a file URL

The paths in `files` are relative to the webroot and start with `/`. A file's URL is the server's media base URL (`http(s)://hostname/ostmedia/`, see the "Media URLs" note of the [Data model]({{% relref "structure.en.md" %}})) followed by the path. Mind the leading `/`: concatenating it to a base that already ends with `/` gives a double `//`.

### What the event does not do

The server detects the **addition, removal and renaming** of files, not a change to the content of an existing file: an image rewritten under the same name produces no event. This is the case for the modules' preview images, whose URL stays identical on every shot: a client must add a cache-busting parameter (`?t=` + timestamp) when it reloads an image after an update of its property.
