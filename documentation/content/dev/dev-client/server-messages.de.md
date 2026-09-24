---
title: Nachrichten Server → Client
weight: 40
description: "Alle JSON-Nachrichten, die der OST-Server an verbundene Clients sendet"
---

## Nachrichtenformat

Jede Nachricht ist ein JSON-Objekt mit **genau einem Schlüssel der obersten Ebene**. Dieser Schlüssel kodiert den Ereignistyp und kann eine lesbare, durch einen Bindestrich getrennte Beschreibung enthalten:

```
"aa-dump all data"   →  Ereignistyp = erste 2 Zeichen = "aa"
"ea"                 →  Ereignistyp = "ea"
"d"                  →  Ereignistyp = "d" (einzelnes Zeichen)
```

Clients müssen die **ersten 1 oder 2 Zeichen** des Schlüssels abgleichen, nicht die vollständige Zeichenkette, da sich der Beschreibungsteil ändern kann.

Alle vom Server gesendeten Ereignisschlüssel sind **klein geschrieben** (z. B. `d`, `ea`, `ap`, `gc`).

{{% notice style="warning" title="Ausnahme: `evt` beginnt mit `ev`" %}}
Der Schlüssel [`evt`](#evt) (Dateiereignisse) beginnt mit denselben zwei Zeichen wie [`ev`](#ev). Zuerst auf exakte Gleichheit mit `evt` prüfen, erst danach die Präfixe, sonst wird ein Dateiereignis für ein Element-Update gehalten.
{{% /notice %}}

---

## Übersicht der Ereignistypen

| Schlüssel | Beschreibung |
|---|---|
| [`d`](#d) | Vollständiger initialer Dump (Antwort auf `DU`) |
| [`aa`](#aa) | Vollständiger Dump eines Moduls (Modul geladen/neu geladen) |
| [`ap`](#ap) | Vollständiger Ersatz einer Property |
| [`ea`](#ea) | Update aller Elementwerte einer Property |
| [`ee`](#ee) | Update eines einzelnen Elementwerts |
| [`ev`](#ev) | Update eines Elementwerts mit seinen Einschränkungen |
| [`ps`](#ps) | Update von Status und Enabled-Zustand einer Property |
| [`dm`](#dm) | Modul gelöscht |
| [`dp`](#dp) | Property gelöscht |
| [`gc`](#gc) | Grid-Zeile erstellt |
| [`gu`](#gu) | Grid-Zeile aktualisiert |
| [`gd`](#gd) | Grid-Zeile gelöscht |
| [`gr`](#gr) | Grid-Reset (alle Zeilen gelöscht) |
| [`lc`](#lc) | LOV erstellt oder zusammengeführt |
| [`lu`](#lc) | LOV ersetzt |
| [`ld`](#lc) | LOV gelöscht |
| [`fs`](#fs) | Profil gespeichert |
| [`fl`](#fs) | Profil geladen |
| [`fc`](#fs) | Profil geändert (nicht gespeicherte Änderungen) |
| [`l`](#l) | Log-Eintrag |
| [`uc`](#uc) | Update der Controller-Daten |
| [`evt`](#evt) | Datei- oder Ordnerereignis (Hinzufügen/Löschen) |
| [`xx`](#xx) | Keepalive-Pong |

---

## `d` — Vollständiger initialer Dump {#d}

Wird als Antwort auf eine `DU`-Anfrage gesendet. Enthält alles: alle Module, Logs, Dateilisten und Controller-Daten. Die Details der Schlüssel von `controllerdata` und `files` stehen in [Controller, GlobalDatastore und Dateien]({{% relref "controller.de.md" %}}); das Beispiel unten zeigt nur einige davon.

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
      "folders": ["", "/Allsky", "/Allsky/archives"],
      "files": ["/image.jpeg"],
      "selectedfolder": "/home/user/ostmedia"
    },
    "logs": [
      { "d": "2024-01-01T12:00:00.000", "c": "Focus", "t": "Module loaded", "l": 1 }
    ],
    "controllerdata": {
      "profiles": {
        "Focus": ["default", "test"]
      },
      "currentconf": "default"
    },
    "lovs": {
      "sharedLov": { "label": "Shared LOV", "type": "string", "values": {} }
    }
  }
}
```

{{% notice style="warning" title="Abkürzungen im Wire-Format" %}}
`m` = modules, `p` = properties, `e` = elements, `l` = globallovs, `f` = profile.
Diese Abkürzungen bei der internen Speicherung auf ihre vollständigen Namen normalisieren.
{{% /notice %}}

---

## `aa` — Vollständiger Dump eines Moduls (Modul geladen/neu geladen) {#aa}

Wird gesendet, wenn ein Modul zur Laufzeit geladen oder neu geladen wird. Enthält den vollständigen Zustand des Moduls. Der Schlüssel beginnt mit `"aa"`.

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

## `ap` — Vollständiger Ersatz einer Property {#ap}

Ersetzt eine oder mehrere Properties vollständig. Der Schlüssel beginnt mit `"ap"`.

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

## `ea` — Update aller Elementwerte (Property-Ebene) {#ea}

Die häufigste Update-Nachricht. Sendet die aktuellen Werte aller Elemente einer Property. Enthält ausschließlich Werte — keine Metadaten. Der Schlüssel ist genau `"ea"`.

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

Bei einfachen Typen (`int`, `float`, `bool`, `string`, `light`) ist der Wert ein Skalar:
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

## `ee` — Update eines einzelnen Elementwerts {#ee}

Aktualisiert den Wert eines einzelnen Elements. Gleiches Format wie `ea`, aber für ein einzelnes Element. Der Schlüssel ist genau `"ee"`.

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

{{% notice style="note" title="`ee` kommt nur vom Backend aus" %}}
`ee` wird ausschließlich auf Initiative des Backends selbst gesendet (z. B. eine geräteseitige Wertänderung, internes Polling) — nie als direkte Antwort auf einen Client-Schreibvorgang. Jede Antwort auf ein clientseitig ausgelöstes `SV` oder `SA` ist immer ein `ea` der gesamten Property, unabhängig davon, wie viele Elemente sich tatsächlich geändert haben.
{{% /notice %}}

---

## `ev` — Update eines Elements mit Metadaten {#ev}

Aktualisiert den Wert eines Elements sowie seine Einschränkungen (min, max, format). Der Schlüssel beginnt mit `"ev"`.

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

## `ps` — Update des Status einer Property {#ps}

Aktualisiert ausschließlich die Felder `status` und `enabled` einer Property. Der Schlüssel beginnt mit `"ps"`.

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

Statuswerte: `0` Ruhe · `1` OK · `2` Beschäftigt · `3` Fehler

---

## `dm` — Modul gelöscht {#dm}

Wird gesendet, wenn ein Modul entladen wird. Der Client muss alle Daten dieses Moduls entfernen. Der Schlüssel beginnt mit `"dm"`.

```json
{
  "dm-delete/remove module": {
    "Focus": {}
  }
}
```

---

## `dp` — Property gelöscht {#dp}

Wird gesendet, wenn eine Property aus einem Modul entfernt wird. Der Schlüssel beginnt mit `"dp"`.

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

## `gc` — Grid-Zeile erstellt {#gc}

Wird gesendet, nachdem eine neue Zeile in einer Grid-Property hinzugefügt wurde. Der Schlüssel beginnt mit `"gc"`.

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

`i` ist der Index der neu erstellten Zeile. `values` ist ein Objekt, indexiert nach Elementname.

---

## `gu` — Grid-Zeile aktualisiert {#gu}

Wird gesendet, wenn eine bestehende Grid-Zeile geändert wird. Der Schlüssel beginnt mit `"gu"`.

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

## `gd` — Grid-Zeile gelöscht {#gd}

Wird gesendet, wenn eine Grid-Zeile entfernt wird. Der Schlüssel beginnt mit `"gd"`.

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

`i` ist der (nullbasierte) Index der gelöschten Zeile. Nachfolgende Zeilen rücken um eine Position vor.

---

## `gr` — Grid-Reset {#gr}

Löscht die gesamte Grid einer Property. Der Schlüssel beginnt mit `"gr"`.

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

## `lc` / `lu` / `ld` — LOV-Ereignisse {#lc}

**`lc` — LOV erstellt/zusammengeführt** (Schlüssel beginnt mit `"lc"`):

Modul-LOV:
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

Controller-LOV (ohne Modul-Struktur, verwendet den Schlüssel `lovs`):
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

**`lu` — LOV ersetzt** (gleiches Format wie `lc`, vollständiger Ersatz).

**`ld` — LOV gelöscht** (Schlüssel beginnt mit `"ld"`):
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

## `fs` / `fl` / `fc` — Profil-Ereignisse {#fs}

Werden gesendet, wenn ein Profil gespeichert, geladen oder geändert wird.

```json
{
  "fs-profile saved": {
    "Focus": {
      "f": { "name": "myProfile", "changed": false }
    }
  }
}
```

| Schlüsselpräfix | Ereignis |
|---|---|
| `fs` | Profil gespeichert |
| `fl` | Profil geladen |
| `fc` | Profil mit nicht gespeicherten Änderungen |

---

## `l` — Log-Eintrag {#l}

Wird für jede vom Server erzeugte Log-Nachricht gesendet. Der Schlüssel ist genau `"l"`.

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

| Feld | Beschreibung |
|---|---|
| `d` | ISO-8601-Datum/Uhrzeit mit Millisekunden |
| `c` | Kontext/Quelle (Modulname oder `"WS"`) |
| `t` | Nachrichtentext (übersetzt in die Sprache des Clients) |
| `l` | Stufe: `0` Debug · `1` Info · `2` Warnung · `3` Fehler · `4` Kritisch |

---

## `uc` — Update der Controller-Daten {#uc}

Wird gesendet, wenn sich globale Controller-Daten ändern: Profile, Konfigurationen, aktive INDI-Treiber, Dateilisten (`files`, `folders`), Systemlast (`systemwatcher`, standardmäßig alle 10 s). Der Schlüssel beginnt mit `"uc"`. Die möglichen Schlüssel sind in [Controller, GlobalDatastore und Dateien]({{% relref "controller.de.md" %}}) beschrieben.

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

Der Payload ist `{"Schlüssel": Wert}`, wobei `Wert` eine Zeichenkette, ein Array oder ein Objekt sein kann. Jede Nachricht trägt **einen einzigen Schlüssel**, und ihr Wert **ersetzt den alten vollständig**.

---

## `evt` — Datei- oder Ordnerereignis {#evt}

Wird an alle Clients gesendet, wenn sich der Inhalt des überwachten Dateiordners ändert (siehe [Dateisystem]({{% relref "controller.de.md" %}}#fichiers)). Anders als bei den anderen Nachrichten lautet der Schlüssel genau `evt`, und sein **Wert ist eine Zeichenkette**, die den Ereignistyp angibt; die Pfade stehen in einem Geschwisterfeld `fileevent`.

```json
{ "evt": "fileadd", "fileevent": ["/Allsky/image_001.jpeg"] }
```

| `evt` | Bedeutung |
|---|---|
| `fileadd` | Datei(en) zum ausgewählten Ordner hinzugefügt |
| `filedel` | Datei(en) aus dem ausgewählten Ordner gelöscht |
| `folderadd` | Unterordner zum Webroot hinzugefügt |
| `folderdel` | Unterordner aus dem Webroot gelöscht |

`fileevent` ist immer ein Array von Pfaden relativ zum Webroot (mit führendem `/`).

---

## `xx` — Keepalive-Pong {#xx}

Wird vom Server als Antwort auf einen Client-Heartbeat (`XX`) gesendet. Kein nützlicher Payload; Clients können ihn ignorieren.

```json
{ "xx": {} }
```
