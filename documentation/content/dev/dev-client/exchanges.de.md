---
title: Beispielsitzung
weight: 60
description: "Kommentierter WebSocket-Austausch, der eine vollständige Client-Sitzung veranschaulicht"
---

Diese Seite beschreibt eine realistische WebSocket-Sitzung im Detail: Verbindung, Dump, Login, Empfang von Updates, Senden von Befehlen und Keepalive.

Notation:
- **`→`** Client → Server
- **`←`** Server → Client

---

## 1. Verbindung

Eine WebSocket-Verbindung zum Server öffnen.

```
→ WS connect to ws://server:9624
← (Verbindung akzeptiert)
```

---

## 2. Vollständigen Dump anfordern

Unmittelbar nach der Verbindung zu senden. Die gewünschte Sprache für übersetzte Labels angeben.

```json
→ {"DU": {"language": "fr"}}
```

Der Server antwortet mit `d`, einer einzigen Nachricht, die alle Module, deren Properties und Elemente, die Dateiliste, die Logs, die Controller-LOVs und die Zugriffsrechte enthält.

```json
← {
  "d": {
    "grant-client": "1",
    "grant-server": "0",
    "serverlng": "fr",
    "m": {
      "MyFocuser": {
        "infos": {"label": "MyFocuser", "name": "MyFocuser", "description": ""},
        "f": {"name": "default", "changed": false},
        "l": {},
        "p": {
          "parameters": {
            "label": "Paramètres", "order": "10",
            "level1": "MyFocuser", "level2": "Config",
            "status": 0, "permission": 2, "enabled": true,
            "showElts": true, "hasGrid": false, "hasGraph": false,
            "rule": 0, "badge": false,
            "preicon1": "", "preicon2": "", "posticon1": "", "posticon2": "",
            "hasprofile": true, "freevalue": "",
            "e": {
              "exposure": {
                "type": "float", "label": "Exposition", "order": "10",
                "hint": "", "autoupdate": false, "badge": false,
                "directedit": true,
                "value": 3.5, "min": 0.1, "max": 30.0, "step": 0.1,
                "format": "%.1f", "slider": 0
              },
              "abortfocus": {
                "type": "bool", "label": "Annuler", "order": "90",
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
Ist `grant-client` gleich `"-1"`, verlangt der Server eine Authentifizierung, bevor er Befehle akzeptiert. Siehe Schritt 3.
Ist er `"0"` oder `"1"`, werden Befehle direkt akzeptiert.
{{% /notice %}}

---

## 3. Login (falls erforderlich)

Nur nötig, wenn `grant-client` im Dump `"-1"` ist.

```json
→ {"LO": {"user": "ADMIN", "pw": "admin", "language": "fr"}}
```

Bei Erfolg sendet der Server einen neuen Dump (gleiches Format wie `d`) mit den aktualisierten Rechten zurück.

---

## 4. Empfang von Echtzeit-Updates

Der Server sendet Updates jederzeit. Kein Polling ist erforderlich.

### Update der Elementwerte (`ea`)

Die häufigste Nachricht. Der Server sendet die aktuellen Werte, sobald eine Änderung eintritt.

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

### Statusänderung einer Property (`ps`)

Wird gesendet, wenn eine Property in den Zustand busy, idle oder disabled wechselt.

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

Statuswerte: `0` Ruhe · `1` OK · `2` Beschäftigt · `3` Fehler

### Log-Eintrag (`l`)

```json
← {"l": {"d": "2026-06-25T10:14:08.000", "c": "MyFocuser", "t": "Autofocus démarré", "l": 1}}
```

---

## 5. Einen Befehl senden

### Eine laufende Aktion abbrechen

Elemente mit `directedit: true` senden ihren Wert sofort über `SV`.

```json
→ {"SV": {"m": {"MyFocuser": {"p": {"actions": {"e": {"abortfocus": true}}}}}}}
```

Der Server bestätigt den Empfang, indem er Status- und Wert-Updates sendet:

```json
← {"ps-only property state": {"MyFocuser": {"p": {"actions": {"status": 2, "enabled": true}}}}}
← {"ea": {"MyFocuser": {"p": {"actions": {"e": {"abortfocus": false}}}}}}
← {"ps-only property state": {"MyFocuser": {"p": {"actions": {"status": 0, "enabled": true}}}}}
```

### Eine Aktion starten

```json
→ {"SV": {"m": {"MyPlanner": {"p": {"actions": {"e": {"start": true}}}}}}}
```

### Mehrere Werte gleichzeitig senden

`SA` für Properties verwenden, bei denen `directedit` `false` ist (Bearbeitung über Formular).

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

Alle 30 Sekunden `XX` senden, um die Verbindung aktiv zu halten.

```json
→ {"XX": {}}
← {"xx": {}}
```

---

## Vollständige Sequenz (Zusammenfassung)

```
→ WS connect
→ {"DU": {"language": "fr"}}
← {"d": { ... vollständiger Dump ... }}

← {"ea": { ... periodische Updates ... }}

→ {"SV": {"m": {"MyFocuser": {"p": {"actions": {"e": {"abortfocus": true}}}}}}}
← {"ps-...": { ... Status beschäftigt ... }}
← {"ea": { ... Werte ... }}
← {"ps-...": { ... Status Ruhe ... }}

→ {"XX": {}}
← {"xx": {}}
```
