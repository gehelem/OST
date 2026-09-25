---
title: Client → Server Messages
weight: 50
description: "All JSON messages that clients can send to the OST server"
---

## Message format

Every message is a JSON object with **one top-level key** identifying the command. The value is an object whose structure depends on the command.

All client command keys are **UPPERCASE** (e.g. `DU`, `SV`, `SA`, `GC`).

---

## Command index

| Key | Description |
|---|---|
| [`DU`](#du) | Request full dump |
| [`XX`](#xx) | Heartbeat / keepalive |
| [`LO`](#lo) | Login |
| [`IL`](#il) | Set language |
| [`SV`](#sv) | Set single element value |
| [`SA`](#sa) | Set all element values |
| [`DP`](#dp) | Request property dump |
| [`I1`](#i1) | Click property preicon1 |
| [`I2`](#i2) | Click property preicon2 |
| [`I3`](#i3) | Click property posticon1 |
| [`I4`](#i4) | Click property posticon2 |
| [`J1`](#j1) | Click element preicon |
| [`J2`](#j2) | Click element posticon |
| [`PL`](#pl) | Load profile |
| [`PS`](#ps) | Save profile |
| [`GC`](#gc) | Create grid line |
| [`GU`](#gu) | Update grid line |
| [`GD`](#gd) | Delete grid line |
| [`GF`](#gf) | Fetch/select grid line |
| [`GH`](#gh) | Move grid line up |
| [`GB`](#gb) | Move grid line down |
| [`GR`](#gr) | Reset (clear) grid |
| [`CL`](#cl) | Load a module configuration |
| [`CS`](#cs) | Save the current configuration |
| [`FS`](#fs) | Select the watched file folder |
| [`ML`](#ml) | Load module |
| [`MK`](#mk) | Kill (unload) module |
| [`YA`](#indi) | Start embedded INDI server |
| [`YZ`](#indi) | Stop embedded INDI server |
| [`YL`](#indi) | Load INDI driver |
| [`YR`](#indi) | Reload INDI driver |
| [`YS`](#indi) | Stop INDI driver |

---

All messages that target a module or property use the wrapper:

```json
{
  "COMMAND": {
    "m": {
      "ModuleName": {
        "p": {
          "propertyName": { ... }
        }
      }
    }
  }
}
```

where `m` stands for *modules* and `p` stands for *properties*.

---

## Session management

### `DU` — Request full dump {#du}

Must be sent immediately after connecting. The server replies with a `d` message containing all modules.

```json
{ "DU": { "language": "en" } }
```

| Field | Description |
|---|---|
| `language` | Preferred language for translated labels (`"en"`, `"fr"`, etc.) |

---

### `XX` — Heartbeat {#xx}

Send every 30 seconds to keep the connection alive. The server replies with `xx`.

```json
{ "XX": {} }
```

---

### `LO` — Login {#lo}

Required when the server has access control enabled (`grant-client` is `"-1"` in the dump response).

```json
{
  "LO": {
    "user": "username",
    "pw": "password",
    "language": "en"
  }
}
```

---

### `IL` — Set language {#il}

Changes the language for translated labels without re-connecting.

```json
{ "IL": { "language": "fr" } }
```

---

## Property value updates

### `SV` — Set single element value {#sv}

Use when `directedit` is `true` on the element — sends only one element's value immediately. On a `directedit: false` element the backend rejects it (error in the module log, nothing applied): use `SA` instead.

```json
{
  "SV": {
    "m": {
      "Focus": {
        "p": {
          "parameters": {
            "e": {
              "iterations": 7
            }
          }
        }
      }
    }
  }
}
```

`e` contains a single `elementName: value` pair.

---

### `SA` — Set all element values {#sa}

Use to submit all editable elements of a property at once (e.g. from a form). Also used when `directedit` is `false`.

```json
{
  "SA": {
    "m": {
      "Focus": {
        "p": {
          "parameters": {
            "e": {
              "iterations": 7,
              "exposure": 3.5,
              "active": true
            }
          }
        }
      }
    }
  }
}
```

{{% notice style="warning" title="Always send every element" %}}
An `SA` must include every editable element of the property. If any is missing, the backend applies nothing at all — not even the elements that were present — and logs a warning server-side (visible to the client via an `l` message). This is not a runtime condition to handle gracefully: it signals a bug in the client, which must always submit the complete set of elements.
{{% /notice %}}

---

### `DP` — Request property dump {#dp}

Asks the server to resend the full state of one property, as if it had just been (re)loaded — useful to force-refresh a property a client suspects is out of sync, without reloading the whole module. The server replies with an [`ap`]({{% relref "server-messages.en.md" %}}#ap) message for that property alone.

```json
{
  "DP": {
    "m": {
      "Focus": {
        "p": {
          "parameters": {}
        }
      }
    }
  }
}
```

No `e` field: `DP` targets the whole property, not a specific element.

---

## Icon actions {#icon-actions}

`preicon1`/`preicon2`/`posticon1`/`posticon2` (properties) and `preicon`/`posticon` (elements, see [Data model]({{% relref "structure.en.md" %}})) are not decorations: each is a **clickable action**. A non-empty value is the Google Fonts (Material Icons) icon name to render for it; clicking the rendered icon sends the matching command below with **no value or parameter at all** — an empty payload that simply targets the property (or element). What the action actually does server-side (saving a profile, loading a profile, starting a process...) is entirely up to the module; the client's only job is to render the icon and forward the click as-is.

An **empty string means there is no icon in that slot** — nothing to render, and consequently nothing to click or send. Don't render a placeholder or wire up a click handler for an empty icon field.

### `I1` — Click property preicon1 {#i1}

```json
{
  "I1": {
    "m": {
      "Focus": {
        "p": {
          "saveprofile": {}
        }
      }
    }
  }
}
```

### `I2` — Click property preicon2 {#i2}

```json
{
  "I2": {
    "m": {
      "Focus": {
        "p": {
          "loadprofile": {}
        }
      }
    }
  }
}
```

### `I3` — Click property posticon1 {#i3}

Same structure as `I1` / `I2`.

### `I4` — Click property posticon2 {#i4}

Same structure as `I1` / `I2`.

---

### `J1` — Click element preicon {#j1}

```json
{
  "J1": {
    "m": {
      "Focus": {
        "p": {
          "devices": {
            "e": {
              "myDevice": {}
            }
          }
        }
      }
    }
  }
}
```

### `J2` — Click element posticon {#j2}

Same structure as `J1`.

---

## Profile management

### `PL` — Load profile {#pl}

```json
{
  "PL": {
    "m": {
      "Focus": {
        "profile": "myProfile"
      }
    }
  }
}
```

The server replies with an `fl` (profile loaded) message and then sends updated property values.

{{% notice style="note" title="A transient `fc` precedes `fl`" %}}
The server sends an [`fc`]({{% relref "server-messages.en.md" %}}#fs) (unsaved changes) message immediately before `fl`, as an intermediate step of loading the profile, before the property's values actually change. A client that reacts to `fc` by showing an "unsaved changes" indicator will see it flash on for one message and self-correct once `fl` arrives right after — expected, not a bug.
{{% /notice %}}

---

### `PS` — Save profile {#ps}

```json
{
  "PS": {
    "m": {
      "Focus": {
        "profile": "myProfile"
      }
    }
  }
}
```

The server replies with an `fs` (profile saved) message.

---

## Grid operations

Grid commands target a specific property within a module. All use the `m` / `p` wrapper.

### `GC` — Create grid line {#gc}

Adds a new row to the grid with the provided element values.

```json
{
  "GC": {
    "m": {
      "Sequencer": {
        "p": {
          "sequence": {
            "e": {
              "target": "M31",
              "exposure": 120,
              "count": 10
            }
          }
        }
      }
    }
  }
}
```

The server replies with a `gc` message confirming the new row index.

{{% notice style="note" title="Base elements update before the grid line does" %}}
Creating a grid line applies the submitted values to the property's base elements first, then appends the row and emits `gc`. This is not one batched update: each changed base element gets its **own full-property `ea`**, one after another (submitting `target`/`exposure`/`count` as above produces three separate `ea` messages, each showing one more field changed than the last), before the final `gc`. If a property shows both its base elements and its grid at the same time (`showElts` and `showGrid` both true — an uncommon combination), a live-rendering client will see the base elements flicker through these intermediate states before the row itself appears. This is the real datastore state, not a bug — adapt your rendering to it (it re-renders correctly either way if you always apply whatever `ea` says, rather than assuming a single message carries every change) rather than filtering it out, and never poll to work around it.
{{% /notice %}}

---

### `GU` — Update grid line {#gu}

Updates a specific row (zero-based index `i`) with new element values.

```json
{
  "GU": {
    "m": {
      "Sequencer": {
        "p": {
          "sequence": {
            "i": 1,
            "e": {
              "target": "M42",
              "exposure": 60
            }
          }
        }
      }
    }
  }
}
```

---

### `GD` — Delete grid line {#gd}

Removes the row at index `i` (zero-based).

```json
{
  "GD": {
    "m": {
      "Sequencer": {
        "p": {
          "sequence": {
            "i": 0
          }
        }
      }
    }
  }
}
```

---

### `GF` — Fetch/select grid line {#gf}

Requests the server to load a grid row into the property's editable elements (used when clicking a row to edit it).

```json
{
  "GF": {
    "m": {
      "Sequencer": {
        "p": {
          "sequence": {
            "i": 2
          }
        }
      }
    }
  }
}
```

---

### `GH` — Move grid line up {#gh}

Moves the row at index `i` up by one position.

```json
{
  "GH": {
    "m": {
      "Sequencer": {
        "p": {
          "sequence": {
            "i": 3
          }
        }
      }
    }
  }
}
```

---

### `GB` — Move grid line down {#gb}

Moves the row at index `i` down by one position.

```json
{
  "GB": {
    "m": {
      "Sequencer": {
        "p": {
          "sequence": {
            "i": 2
          }
        }
      }
    }
  }
}
```

---

### `GR` — Reset grid {#gr}

Clears the entire grid of a property (all rows removed) — not a single row like `GD`. The server broadcasts [`gr`]({{% relref "server-messages.en.md" %}}#gr) in response.

```json
{
  "GR": {
    "m": {
      "Sequencer": {
        "p": {
          "sequence": {}
        }
      }
    }
  }
}
```

No `i` field: `GR` targets the whole grid, not one row.

---

## Module lifecycle

### `CL` — Load a configuration {#cl}

Loads all the modules of a saved configuration (list in `controllerdata.availableconfs`, see [Controller]({{% relref "controller.en.md" %}})).

```json
{ "CL": { "name": "modules" } }
```

---

### `CS` — Save the current configuration {#cs}

Saves the set of currently loaded modules under the given name (`GlobalDatastore` is never included).

```json
{ "CS": { "name": "modules" } }
```

---

### `ML` — Load module {#ml}

Requests the server to dynamically load a module. These commands target the controller: the payload does **not** use the `m`/`p` envelope.

```json
{ "ML": { "lib": "focus", "label": "MyFocuser", "profile": "default" } }
```

| Field | Description |
|---|---|
| `lib` | Module library **without** the `libost` prefix (the server adds it): `focus` for `libostfocus`. Available libraries are the keys of `controllerdata.libraries` |
| `label` | Label of this instance; the **module name** is this label without spaces. If a module with that name is already loaded, the request is refused |
| `profile` | Profile name to load (optional) |

{{% notice style="warning" title="Don't pass a controllerdata.libraries key as-is" %}}
The keys of `controllerdata.libraries` are already prefixed (`libostfocus`, `libostdummy`...). The server prepends `libost` again unconditionally, so forwarding a key straight through double-prefixes it and fails:

```json
{ "ML": { "lib": "libostdummy", "label": "MyDummy" } }
```
```
Error loading library MyDummy - Cannot load library libostlibostdummy: cannot open shared object file: No such file or directory
```

Strip the `libost` prefix from the key first (same transform `profiles`' keying convention relies on, see [Controller data]({{% relref "controller.en.md" %}})):

```json
{ "ML": { "lib": "dummy", "label": "MyDummy" } }
```
{{% /notice %}}

---

### `MK` — Kill (unload) module {#mk}

Requests the server to unload a running module, identified by its name.

```json
{ "MK": { "name": "MyFocuser" } }
```

`GlobalDatastore` cannot be unloaded: the request is refused (error message in the logs).

---

## File system

### `FS` — Select the watched folder {#fs}

Selects the folder whose file list the server publishes (see [File system]({{% relref "controller.en.md" %}}#fichiers)). The path is **relative to the webroot**; the empty string designates the root.

```json
{ "FS": { "folder": "/Sequencer/gam Cyg/LIGHT/Luminance" } }
```

The change is **server-wide** (all clients see the new list). The path is not validated: a non-existent folder yields an empty list.

---

## Embedded INDI server control {#indi}

These commands are only available when the server has an embedded INDI instance.

| Command | Description |
|---|---|
| `YA` | Start the embedded INDI server |
| `YZ` | Stop the embedded INDI server |
| `YL` | Load an INDI driver (`{"YL": {"driver": "<binary>"}}`) |
| `YR` | Reload an INDI driver (`{"YR": {"driver": "<binary>"}}`) |
| `YS` | Stop an INDI driver (`{"YS": {"driver": "<binary>"}}`) |

`YA` and `YZ` have no payload (`{"YA": {}}`). For `YL`, `YR` and `YS`, `driver` is the `binary` field of an entry of `controllerdata.indidrivers`.

---

## Access control summary

| Server grant | Allowed commands |
|---|---|
| `grant-client: "1"` | All commands |
| `grant-client: "0"` | `DU`, `LO`, `IL` only |
| `grant-client: "-1"` | `LO` only (login required) |

Commands sent without sufficient grant are silently rejected by the server.
