---
title: Nachrichten Client → Server
weight: 50
description: "Alle JSON-Nachrichten, die Clients an den OST-Server senden können"
---

## Nachrichtenformat

Jede Nachricht ist ein JSON-Objekt mit **genau einem Schlüssel der obersten Ebene**, der den Befehl identifiziert. Der Wert ist ein Objekt, dessen Struktur vom Befehl abhängt.

Alle Client-Befehlsschlüssel stehen in **GROSSBUCHSTABEN** (z. B. `DU`, `SV`, `SA`, `GC`).

---

## Befehlsübersicht

| Schlüssel | Beschreibung |
|---|---|
| [`DU`](#du) | Vollständigen Dump anfordern |
| [`XX`](#xx) | Heartbeat / Keepalive |
| [`LO`](#lo) | Anmeldung |
| [`IL`](#il) | Sprache ändern |
| [`SV`](#sv) | Wert eines einzelnen Elements senden |
| [`SA`](#sa) | Alle Elementwerte senden |
| [`DP`](#dp) | Property-Dump anfordern |
| [`I1`](#i1) | Klick auf preicon1 der Property |
| [`I2`](#i2) | Klick auf preicon2 der Property |
| [`I3`](#i3) | Klick auf posticon1 der Property |
| [`I4`](#i4) | Klick auf posticon2 der Property |
| [`J1`](#j1) | Klick auf preicon des Elements |
| [`J2`](#j2) | Klick auf posticon des Elements |
| [`PL`](#pl) | Profil laden |
| [`PS`](#ps) | Profil speichern |
| [`GC`](#gc) | Grid-Zeile erstellen |
| [`GU`](#gu) | Grid-Zeile aktualisieren |
| [`GD`](#gd) | Grid-Zeile löschen |
| [`GF`](#gf) | Grid-Zeile auswählen |
| [`GH`](#gh) | Grid-Zeile nach oben verschieben |
| [`GB`](#gb) | Grid-Zeile nach unten verschieben |
| [`GR`](#gr) | Grid zurücksetzen (leeren) |
| [`CL`](#cl) | Modulkonfiguration laden |
| [`CS`](#cs) | Aktuelle Konfiguration speichern |
| [`FS`](#fs) | Überwachten Dateiordner wählen |
| [`ML`](#ml) | Modul laden |
| [`MK`](#mk) | Modul entladen |
| [`YA`](#indi) | Eingebetteten INDI-Server starten |
| [`YZ`](#indi) | Eingebetteten INDI-Server stoppen |
| [`YL`](#indi) | INDI-Treiber laden |
| [`YR`](#indi) | INDI-Treiber neu laden |
| [`YS`](#indi) | INDI-Treiber stoppen |

---

Alle Nachrichten, die ein Modul oder eine Property ansprechen, verwenden folgende Struktur:

```json
{
  "COMMANDE": {
    "m": {
      "NomDuModule": {
        "p": {
          "nomDeLaPropriété": { ... }
        }
      }
    }
  }
}
```

wobei `m` für *modules* und `p` für *properties* steht.

---

## Sitzungsverwaltung

### `DU` — Vollständigen Dump anfordern {#du}

Muss unmittelbar nach der Verbindung gesendet werden. Der Server antwortet mit einer `d`-Nachricht, die alle Module enthält.

```json
{ "DU": { "language": "fr" } }
```

| Feld | Beschreibung |
|---|---|
| `language` | Bevorzugte Sprache für übersetzte Bezeichnungen (`"en"`, `"fr"`, usw.) |

---

### `XX` — Heartbeat {#xx}

Alle 30 Sekunden zu senden, um die Verbindung aktiv zu halten. Der Server antwortet mit `xx`.

```json
{ "XX": {} }
```

---

### `LO` — Anmeldung {#lo}

Erforderlich, wenn der Server die Zugriffskontrolle aktiviert hat (`grant-client` ist `"-1"` in der Dump-Antwort).

```json
{
  "LO": {
    "user": "username",
    "pw": "password",
    "language": "fr"
  }
}
```

---

### `IL` — Sprache ändern {#il}

Ändert die Sprache der übersetzten Bezeichnungen ohne erneute Verbindung.

```json
{ "IL": { "language": "fr" } }
```

---

## Aktualisierung von Property-Werten

### `SV` — Wert eines einzelnen Elements senden {#sv}

Zu verwenden, wenn `directedit` beim Element `true` ist — sendet den Wert eines einzelnen Elements sofort. Bei einem Element mit `directedit: false` lehnt das Backend es ab (Fehler im Modul-Log, nichts wird angewendet): stattdessen `SA` verwenden.

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

`e` enthält genau ein Paar `Elementname: Wert`.

---

### `SA` — Alle Elementwerte senden {#sa}

Wird verwendet, um alle editierbaren Elemente einer Property auf einmal zu übermitteln (z. B. aus einem Formular). Wird auch verwendet, wenn `directedit` `false` ist.

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

{{% notice style="warning" title="Immer alle Elemente senden" %}}
Ein `SA` muss alle editierbaren Elemente der Property enthalten. Fehlt auch nur eines, wendet das Backend gar nichts an — nicht einmal die vorhandenen Elemente — und protokolliert serverseitig eine Warnung (für den Client sichtbar über eine `l`-Nachricht). Das ist kein Laufzeitfall, der elegant abgefangen werden soll: Es zeigt einen Bug im Client an, der immer den vollständigen Satz an Elementen senden muss.
{{% /notice %}}

---

### `DP` — Property-Dump anfordern {#dp}

Bittet den Server, den vollständigen Zustand einer Property erneut zu senden, als wäre sie gerade (neu) geladen worden — nützlich, um eine Property zu aktualisieren, bei der ein Client eine Desynchronisation vermutet, ohne das ganze Modul neu zu laden. Der Server antwortet mit einer [`ap`]({{% relref "server-messages.de.md" %}}#ap)-Nachricht für genau diese Property.

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

Kein `e`-Feld: `DP` bezieht sich auf die gesamte Property, nicht auf ein einzelnes Element.

---

## Icon-Aktionen

Die Icons einer Property (preicon / posticon) lösen Backend-Aktionen aus, etwa das Speichern eines Profils, das Laden eines Profils oder das Starten eines Prozesses. Klick-Ereignisse werden über folgende Befehle gesendet.

### `I1` — Klick auf preicon1 der Property {#i1}

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

### `I2` — Klick auf preicon2 der Property {#i2}

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

### `I3` — Klick auf posticon1 der Property {#i3}

Gleiche Struktur wie `I1` / `I2`.

### `I4` — Klick auf posticon2 der Property {#i4}

Gleiche Struktur wie `I1` / `I2`.

---

### `J1` — Klick auf preicon des Elements {#j1}

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

### `J2` — Klick auf posticon des Elements {#j2}

Gleiche Struktur wie `J1`.

---

## Profilverwaltung

### `PL` — Profil laden {#pl}

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

Der Server antwortet mit einer `fl`-Nachricht (Profil geladen) und sendet anschließend die aktualisierten Property-Werte.

---

### `PS` — Profil speichern {#ps}

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

Der Server antwortet mit einer `fs`-Nachricht (Profil gespeichert).

---

## Grid-Operationen

Grid-Befehle betreffen eine bestimmte Property innerhalb eines Moduls. Alle verwenden die `m` / `p`-Struktur.

### `GC` — Grid-Zeile erstellen {#gc}

Fügt der Grid eine neue Zeile mit den übergebenen Elementwerten hinzu.

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

Der Server antwortet mit einer `gc`-Nachricht, die den Index der neuen Zeile bestätigt.

{{% notice style="note" title="Basis-Elemente aktualisieren sich vor der Grid-Zeile" %}}
Das Erstellen einer Grid-Zeile wendet die übermittelten Werte zuerst auf die Basis-Elemente der Property an (jedes sendet sein eigenes `ea`), fügt dann die Zeile hinzu und sendet `gc`. Zeigt eine Property gleichzeitig ihre Basis-Elemente und ihr Grid an (`showElts` und `showGrid` beide aktiv — eine unübliche Kombination), sieht ein live-rendernder Client kurz, wie die Basis-Elemente die Werte der neuen Zeile widerspiegeln, bevor die Zeile selbst erscheint. Das ist der echte Datastore-Zustand, kein Bug — die Darstellung sollte sich daran anpassen, statt es herauszufiltern, und vor allem niemals Polling einsetzen, um das zu umgehen.
{{% /notice %}}

---

### `GU` — Grid-Zeile aktualisieren {#gu}

Aktualisiert eine bestimmte Zeile (nullbasierter Index `i`) mit neuen Elementwerten.

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

### `GD` — Grid-Zeile löschen {#gd}

Löscht die Zeile mit Index `i` (nullbasiert).

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

### `GF` — Grid-Zeile auswählen {#gf}

Fordert den Server auf, eine Grid-Zeile in die editierbaren Elemente der Property zu laden (wird verwendet, wenn eine Zeile zur Bearbeitung angeklickt wird).

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

### `GH` — Grid-Zeile nach oben verschieben {#gh}

Verschiebt die Zeile mit Index `i` um eine Position nach oben.

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

### `GB` — Grid-Zeile nach unten verschieben {#gb}

Verschiebt die Zeile mit Index `i` um eine Position nach unten.

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

### `GR` — Grid zurücksetzen {#gr}

Leert das gesamte Grid einer Property (alle Zeilen entfernt) — nicht nur eine einzelne Zeile wie `GD`. Der Server sendet daraufhin [`gr`]({{% relref "server-messages.de.md" %}}#gr).

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

Kein `i`-Feld: `GR` bezieht sich auf das gesamte Grid, nicht auf eine einzelne Zeile.

---

## Modul-Lebenszyklus

### `CL` — Konfiguration laden {#cl}

Lädt alle Module einer gespeicherten Konfiguration (Liste in `controllerdata.availableconfs`, siehe [Controller]({{% relref "controller.de.md" %}})).

```json
{ "CL": { "name": "modules" } }
```

---

### `CS` — Aktuelle Konfiguration speichern {#cs}

Speichert die Menge der aktuell geladenen Module unter dem angegebenen Namen (`GlobalDatastore` ist nie enthalten).

```json
{ "CS": { "name": "modules" } }
```

---

### `ML` — Modul laden {#ml}

Fordert den Server auf, ein Modul dynamisch zu laden. Diese Befehle richten sich an den Controller: der Payload verwendet **nicht** den `m`/`p`-Umschlag.

```json
{ "ML": { "lib": "focus", "label": "MyFocuser", "profile": "default" } }
```

| Feld | Beschreibung |
|---|---|
| `lib` | Bibliothek des Moduls, **ohne** das Präfix `libost` (der Server fügt es hinzu): `focus` für `libostfocus`. Die verfügbaren Bibliotheken sind die Schlüssel von `controllerdata.libraries` |
| `label` | Label dieser Instanz; der **Modulname** ist dieses Label ohne Leerzeichen. Ist ein Modul dieses Namens bereits geladen, wird die Anfrage abgelehnt |
| `profile` | Name des zu ladenden Profils (optional) |

---

### `MK` — Modul entladen {#mk}

Fordert den Server auf, ein laufendes Modul zu entladen, bezeichnet durch seinen Namen.

```json
{ "MK": { "name": "MyFocuser" } }
```

`GlobalDatastore` kann nicht entladen werden: die Anfrage wird abgelehnt (Fehlermeldung in den Logs).

---

## Dateisystem

### `FS` — Überwachten Ordner wählen {#fs}

Bestimmt den Ordner, dessen Dateiliste der Server veröffentlicht (siehe [Dateisystem]({{% relref "controller.de.md" %}}#fichiers)). Der Pfad ist **relativ zum Webroot**; die leere Zeichenkette bezeichnet das Wurzelverzeichnis.

```json
{ "FS": { "folder": "/Sequencer/gam Cyg/LIGHT/Luminance" } }
```

Die Änderung gilt **global für den Server** (alle Clients sehen die neue Liste). Der Pfad wird nicht validiert: ein nicht existierender Ordner ergibt eine leere Liste.

---

## Steuerung des eingebetteten INDI-Servers {#indi}

Diese Befehle sind nur verfügbar, wenn der Server über eine eingebettete INDI-Instanz verfügt.

| Befehl | Beschreibung |
|---|---|
| `YA` | Eingebetteten INDI-Server starten |
| `YZ` | Eingebetteten INDI-Server stoppen |
| `YL` | INDI-Treiber laden (`{"YL": {"driver": "<binary>"}}`) |
| `YR` | INDI-Treiber neu laden (`{"YR": {"driver": "<binary>"}}`) |
| `YS` | INDI-Treiber stoppen (`{"YS": {"driver": "<binary>"}}`) |

`YA` und `YZ` haben keinen Payload (`{"YA": {}}`). Bei `YL`, `YR` und `YS` ist `driver` das Feld `binary` eines Eintrags aus `controllerdata.indidrivers`.

---

## Zusammenfassung der Zugriffskontrolle

| Grant-Wert des Servers | Erlaubte Befehle |
|---|---|
| `grant-client: "1"` | Alle Befehle |
| `grant-client: "0"` | Nur `DU`, `LO`, `IL` |
| `grant-client: "-1"` | Nur `LO` (Authentifizierung erforderlich) |

Befehle, die ohne ausreichende Rechte gesendet werden, werden vom Server stillschweigend ignoriert.
