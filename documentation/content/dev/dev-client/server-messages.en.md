---
title: Server → Client Messages
weight: 20
description: "All JSON messages sent by the OST server to connected clients"
---

## Message format

Every message is a JSON object with **one top-level key**. That key encodes the event type and may include a human-readable description separated by a dash:

```
"aa-dump all data"   →  event type = first 2 characters = "aa"
"ea"                 →  event type = "ea"
"d"                  →  event type = "d" (single character)
```

Clients should match on the **first 1 or 2 characters** of the key, not the full string, since the description part can change.

All server-sent event keys are **lowercase** (e.g. `d`, `ea`, `ap`, `gc`).

---

## Event type index

| Key | Description |
|---|---|
| [`d`](#d) | Full initial dump (response to `DU`) |
| [`aa`](#aa) | Full module dump (module loaded/reloaded) |
| [`ap`](#ap) | Full property replacement |
| [`ea`](#ea) | Set all element values for a property |
| [`ee`](#ee) | Set one element value |
| [`ev`](#ev) | Set element value with updated constraints |
| [`ps`](#ps) | Property status and enabled update |
| [`dm`](#dm) | Module deleted |
| [`dp`](#dp) | Property deleted |
| [`gc`](#gc) | Grid line created |
| [`gu`](#gu) | Grid line updated |
| [`gd`](#gd) | Grid line deleted |
| [`gr`](#gr) | Grid reset (all rows cleared) |
| [`lc`](#lc) | LOV created or merged |
| [`lu`](#lc) | LOV replaced |
| [`ld`](#lc) | LOV deleted |
| [`fs`](#fs) | Profile saved |
| [`fl`](#fs) | Profile loaded |
| [`fc`](#fs) | Profile has unsaved changes |
| [`l`](#l) | Log entry |
| [`uc`](#uc) | Controller data update |
| [`xx`](#xx) | Keepalive pong |

---

## `d` — Full initial dump {#d}

Sent in response to a `DU` request. Contains everything: all modules, logs, file lists, and controller data.

```json
{
  "d": {
    "grant-client": "1",
    "grant-server": "0",
    "serverlng": "en",
    "m": {
      "Focus": {
        "infos": { "label": "Focus", "name": "Focus", "description": "..." },
        "f": { "name": "default", "changed": false },
        "l": {
          "myLov": { "label": "My LOV", "type": "string", "values": { "a": "Option A" } }
        },
        "p": {
          "parameters": {
            "label": "Parameters",
            "order": "10",
            "level1": "Focus",
            "level2": "Config",
            "status": 0,
            "permission": 2,
            "enabled": true,
            "showElts": true,
            "hasGrid": false,
            "hasGraph": false,
            "rule": 0,
            "badge": false,
            "preicon1": "", "preicon2": "", "posticon1": "", "posticon2": "",
            "hasprofile": true,
            "freevalue": "",
            "e": {
              "iterations": {
                "type": "int",
                "label": "Iterations",
                "order": "10",
                "hint": "",
                "autoupdate": false,
                "badge": false,
                "directedit": false,
                "value": 5,
                "min": 1,
                "max": 20,
                "step": 1,
                "format": "",
                "slider": 0
              }
            }
          }
        }
      }
    },
    "files": {
      "folders": ["Allsky/archives"],
      "files": ["Allsky/archives/image.fits"],
      "selectedfolder": ""
    },
    "logs": [
      { "d": "2024-01-01T12:00:00.000", "c": "Focus", "t": "Module loaded", "l": 1 }
    ],
    "controllerdata": {
      "profiles": {
        "Focus": ["default", "test"]
      }
    },
    "lovs": {
      "sharedLov": { "label": "Shared LOV", "type": "string", "values": {} }
    }
  }
}
```

{{% notice style="warning" title="Wire format abbreviations" %}}
`m` = modules, `p` = properties, `e` = elements, `l` = globallovs, `f` = profile.
Normalize these to their full names when storing internally.
{{% /notice %}}

---

## `aa` — Full module dump (module loaded/reloaded) {#aa}

Sent when a module is loaded or reloaded at runtime. Contains the complete module state. Key starts with `"aa"`.

```json
{
  "aa-dump all data": {
    "Guider": {
      "infos": { "label": "Guider", "name": "Guider", "description": "..." },
      "f": { "name": "default", "changed": false },
      "l": {},
      "p": {
        "status": { ... }
      }
    }
  }
}
```

---

## `ap` — Full property replacement {#ap}

Replaces one or more properties entirely. Key starts with `"ap"`.

```json
{
  "ap-dump all property data": {
    "Focus": {
      "p": {
        "parameters": {
          "label": "Parameters",
          "status": 0,
          "permission": 2,
          "enabled": true,
          "showElts": true,
          "hasGrid": false,
          "hasGraph": false,
          "rule": 0,
          "e": {
            "iterations": {
              "type": "int",
              "value": 7,
              "min": 1, "max": 20, "step": 1,
              "label": "Iterations", "order": "10", "hint": "",
              "autoupdate": false, "badge": false, "directedit": false,
              "format": "", "slider": 0
            }
          }
        }
      }
    }
  }
}
```

---

## `ea` — Set all element values (property-level update) {#ea}

The most common update message. Sends the current values of all elements in a property. Contains values only — no metadata. Key is exactly `"ea"`.

```json
{
  "ea": {
    "Allsky": {
      "p": {
        "coming": {
          "e": {
            "sunrise": { "hh": 7, "mm": 6, "ss": 51 },
            "sunset":  { "hh": 21, "mm": 15, "ss": 1 }
          }
        }
      }
    }
  }
}
```

For simple types (`int`, `float`, `bool`, `string`, `light`), the value is a scalar:
```json
{
  "ea": {
    "Focus": {
      "p": {
        "parameters": {
          "e": {
            "iterations": 5,
            "exposure": 3.5,
            "active": true
          }
        }
      }
    }
  }
}
```

---

## `ee` — Set one element value {#ee}

Updates a single element's value. Same format as `ea` but for one element only. Key is exactly `"ee"`.

```json
{
  "ee": {
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
```

---

## `ev` — Set element with metadata {#ev}

Updates an element's value and its constraints (min, max, format). Key starts with `"ev"`.

```json
{
  "ev-set one element value/min/max/format ": {
    "Focus": {
      "p": {
        "parameters": {
          "e": {
            "temperature": {
              "value": 12.5,
              "min": -40.0,
              "max": 80.0,
              "format": "%.1f"
            }
          }
        }
      }
    }
  }
}
```

---

## `ps` — Property status update {#ps}

Updates only the `status` and `enabled` fields of a property. Key starts with `"ps"`.

```json
{
  "ps-only property state": {
    "Focus": {
      "p": {
        "autofocus": {
          "status": 2,
          "enabled": true
        }
      }
    }
  }
}
```

Status values: `0` Standby · `1` OK · `2` Busy · `3` Error

---

## `dm` — Module deleted {#dm}

Sent when a module is unloaded. The client should remove all data for this module. Key starts with `"dm"`.

```json
{
  "dm-delete/remove module": {
    "Focus": {}
  }
}
```

---

## `dp` — Property deleted {#dp}

Sent when a property is removed from a module. Key starts with `"dp"`.

```json
{
  "dp-delete/remove property": {
    "Focus": {
      "p": {
        "oldProperty": ""
      }
    }
  }
}
```

---

## `gc` — Grid line created {#gc}

Sent after a new row is added to a grid property. Key starts with `"gc"`.

```json
{
  "gc-grid new line ": {
    "Sequencer": {
      "p": {
        "sequence": {
          "i": 3,
          "values": {
            "target": "M31",
            "exposure": 120,
            "count": 10
          }
        }
      }
    }
  }
}
```

`i` is the index of the newly created row. `values` is an object keyed by element name.

---

## `gu` — Grid line updated {#gu}

Sent when an existing grid row is modified. Key starts with `"gu"`.

```json
{
  "gu-grid update line": {
    "Sequencer": {
      "p": {
        "sequence": {
          "i": 1,
          "values": {
            "target": "M42",
            "exposure": 60,
            "count": 20
          }
        }
      }
    }
  }
}
```

---

## `gd` — Grid line deleted {#gd}

Sent when a grid row is removed. Key starts with `"gd"`.

```json
{
  "gd-grid delete line": {
    "Sequencer": {
      "p": {
        "sequence": {
          "i": 0
        }
      }
    }
  }
}
```

`i` is the (zero-based) index of the deleted row. Rows after `i` shift down by one.

---

## `gr` — Grid reset {#gr}

Clears the entire grid for a property. Key starts with `"gr"`.

```json
{
  "gr-grid reset": {
    "Sequencer": {
      "p": {
        "sequence": {}
      }
    }
  }
}
```

---

## `lc` / `lu` / `ld` — LOV events {#lc}

**`lc` — LOV created/merged** (key starts with `"lc"`):

Module LOV:
```json
{
  "lc-lov create": {
    "Focus": {
      "l": {
        "myLov": {
          "label": "My LOV",
          "type": "string",
          "values": { "a": "Option A", "b": "Option B" }
        }
      }
    }
  }
}
```

Controller LOV (no module wrapper, uses `lovs` key):
```json
{
  "lc-lov create": {
    "lovs": {
      "sharedLov": {
        "label": "Shared LOV",
        "type": "string",
        "values": { "x": "Value X" }
      }
    }
  }
}
```

**`lu` — LOV replaced** (same format as `lc`, full replacement).

**`ld` — LOV deleted** (key starts with `"ld"`):
```json
{
  "ld-lov delete": {
    "lovs": {
      "sharedLov": {}
    }
  }
}
```

---

## `fs` / `fl` / `fc` — Profile events {#fs}

Sent when a profile is saved, loaded, or modified.

```json
{
  "fs-profile saved": {
    "Focus": {
      "f": { "name": "myProfile", "changed": false }
    }
  }
}
```

| Key prefix | Event |
|---|---|
| `fs` | Profile saved |
| `fl` | Profile loaded |
| `fc` | Profile has unsaved changes |

---

## `l` — Log entry {#l}

Sent for every log message generated by the server. Key is exactly `"l"`.

```json
{
  "l": {
    "d": "2024-01-01T12:00:00.000",
    "c": "Focus",
    "t": "Autofocus completed successfully",
    "l": 1
  }
}
```

| Field | Description |
|---|---|
| `d` | ISO 8601 datetime with milliseconds |
| `c` | Context/source (module name or `"WS"`) |
| `t` | Message text (translated to client language) |
| `l` | Log level: `0` Debug · `1` Info · `2` Warning · `3` Error · `4` Critical |

---

## `uc` — Controller data update {#uc}

Sent when global controller data changes (e.g. file list, profiles). Key starts with `"uc"`.

```json
{
  "uc-update controller data": {
    "profiles": {
      "Focus": ["default", "highres"],
      "Guider": ["default"]
    }
  }
}
```

The payload is `{"key": value}` where `value` can be a string, array, or object.

---

## `xx` — Keepalive pong {#xx}

Sent by the server in response to a client heartbeat (`XX`). No useful payload; clients can ignore it.

```json
{ "xx": {} }
```
