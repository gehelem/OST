---
title: Client → Server Messages
weight: 30
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

Use when `directedit` is `true` on the element — sends only one element's value immediately.

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

---

## Icon actions

Property icons (preicon / posticon) trigger backend actions such as saving a profile, loading a profile, or starting a process. Click events are sent using the following commands.

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

## Module lifecycle

### `ML` — Load module {#ml}

Requests the server to dynamically load a module.

```json
{
  "ML": {
    "m": {
      "ModuleName": {}
    }
  }
}
```

---

### `MK` — Kill (unload) module {#mk}

Requests the server to unload a running module.

```json
{
  "MK": {
    "m": {
      "ModuleName": {}
    }
  }
}
```

---

## Embedded INDI server control {#indi}

These commands are only available when the server has an embedded INDI instance.

| Command | Description |
|---|---|
| `YA` | Start the embedded INDI server |
| `YZ` | Stop the embedded INDI server |
| `YL` | Load an INDI driver |
| `YR` | Reload an INDI driver |
| `YS` | Stop an INDI driver |

---

## Access control summary

| Server grant | Allowed commands |
|---|---|
| `grant-client: "1"` | All commands |
| `grant-client: "0"` | `DU`, `LO`, `IL` only |
| `grant-client: "-1"` | `LO` only (login required) |

Commands sent without sufficient grant are silently rejected by the server.
