---
title: Example Session
weight: 40
description: "Annotated WebSocket exchange showing a complete client session"
---

This page walks through a realistic WebSocket session: connection, dump, login, reading updates, sending commands, and keepalive.

Notation:
- **`→`** Client → Server
- **`←`** Server → Client

---

## 1. Connect

Open a plain WebSocket connection to the server.

```
→ WS connect to ws://server:9624
← (connection accepted)
```

---

## 2. Request the full dump

Send immediately after connecting. Pass the preferred language for translated labels.

```json
→ {"DU": {"language": "en"}}
```

The server replies with `d`, a single large message containing all modules, their properties and elements, the file list, logs, controller LOVs, and access control grants.

```json
← {
  "d": {
    "grant-client": "1",
    "grant-server": "0",
    "serverlng": "en",
    "m": {
      "MyFocuser": {
        "infos": {"label": "MyFocuser", "name": "MyFocuser", "description": ""},
        "f": {"name": "default", "changed": false},
        "l": {},
        "p": {
          "parameters": {
            "label": "Parameters", "order": "10",
            "level1": "MyFocuser", "level2": "Config",
            "status": 0, "permission": 2, "enabled": true,
            "showElts": true, "hasGrid": false, "hasGraph": false,
            "rule": 0, "badge": false,
            "preicon1": "", "preicon2": "", "posticon1": "", "posticon2": "",
            "hasprofile": true, "freevalue": "",
            "e": {
              "exposure": {
                "type": "float", "label": "Exposure", "order": "10",
                "hint": "", "autoupdate": false, "badge": false,
                "directedit": true,
                "value": 3.5, "min": 0.1, "max": 30.0, "step": 0.1,
                "format": "%.1f", "slider": 0
              },
              "abortfocus": {
                "type": "bool", "label": "Abort", "order": "90",
                "hint": "", "autoupdate": false, "badge": false,
                "directedit": true, "value": false
              }
            }
          }
        }
      }
    },
    "files": {"folders": [], "files": [], "selectedfolder": ""},
    "logs": [],
    "controllerdata": {"profiles": {"MyFocuser": ["default"]}},
    "lovs": {}
  }
}
```

{{% notice style="info" title="grant-client" %}}
If `grant-client` is `"-1"`, the server requires a login before accepting commands. See step 3.
If it is `"0"` or `"1"`, commands are accepted immediately.
{{% /notice %}}

---

## 3. Login (if required)

Only needed when `grant-client` is `"-1"` in the dump.

```json
→ {"LO": {"user": "ADMIN", "pw": "admin", "language": "en"}}
```

On success, the server sends a new dump (same format as `d`) with updated grant values.

---

## 4. Receive live updates

The server pushes updates at any time. No polling required.

### Property value update (`ea`)

The most common message. The server sends current element values whenever something changes.

```json
← {
  "ea": {
    "MyFocuser": {
      "p": {
        "parameters": {
          "e": {
            "exposure": 3.5,
            "abortfocus": false
          }
        }
      }
    }
  }
}
```

### Property status change (`ps`)

Sent when a property becomes busy, idle, or disabled.

```json
← {
  "ps-only property state": {
    "MyFocuser": {
      "p": {
        "parameters": {
          "status": 2,
          "enabled": true
        }
      }
    }
  }
}
```

Status values: `0` Standby · `1` OK · `2` Busy · `3` Error

### Log entry (`l`)

```json
← {"l": {"d": "2026-06-25T10:14:08.000", "c": "MyFocuser", "t": "Autofocus started", "l": 1}}
```

---

## 5. Send a command

### Abort a running action

`directedit: true` elements send their value immediately with `SV`.

```json
→ {"SV": {"m": {"MyFocuser": {"p": {"actions": {"e": {"abortfocus": true}}}}}}}
```

The server acknowledges by pushing status and value updates:

```json
← {"ps-only property state": {"MyFocuser": {"p": {"actions": {"status": 2, "enabled": true}}}}}
← {"ea": {"MyFocuser": {"p": {"actions": {"e": {"abortfocus": false}}}}}}
← {"ps-only property state": {"MyFocuser": {"p": {"actions": {"status": 0, "enabled": true}}}}}
```

### Start an action

```json
→ {"SV": {"m": {"MyPlanner": {"p": {"actions": {"e": {"start": true}}}}}}}
```

### Set multiple values at once

Use `SA` for properties where `directedit` is `false` (form-style editing).

```json
→ {
  "SA": {
    "m": {
      "MyFocuser": {
        "p": {
          "parameters": {
            "e": {
              "exposure": 5.0,
              "gain": 100
            }
          }
        }
      }
    }
  }
}
```

---

## 6. Keepalive

Send `XX` every 30 seconds to prevent the server from closing the connection.

```json
→ {"XX": {}}
← {"xx": {}}
```

---

## Full sequence (condensed)

```
→ WS connect
→ {"DU": {"language": "en"}}
← {"d": { ... full dump ... }}

← {"ea": { ... periodic updates ... }}

→ {"SV": {"m": {"MyFocuser": {"p": {"actions": {"e": {"abortfocus": true}}}}}}}
← {"ps-...": { ... status busy ... }}
← {"ea": { ... values ... }}
← {"ps-...": { ... status standby ... }}

→ {"XX": {}}
← {"xx": {}}
```
