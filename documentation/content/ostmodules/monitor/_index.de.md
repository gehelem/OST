---
title: Monitor
weight: 75
description: Überwachung und Protokollierung der Sitzungsereignisse
---

## Aufgabe

Das Monitor-Modul zeichnet die von den Slave-Modulen während einer Beobachtungssitzung ausgelösten Ereignisse auf und stellt sie grafisch dar. Es erlaubt die nachträgliche Betrachtung der Guiding-Qualität, der HFR-Entwicklung der Bilder und der Fokussierungseingriffe auf einer gemeinsamen Zeitachse.

![Screenshot des Monitor-Moduls](/images/modules/monitor.png)

## Funktionsprinzip

Der Monitor beobachtet die Eigenschaft `signals` jedes konfigurierten Slave-Moduls. Jedes Mal, wenn ein Modul ein Ereignis auslöst (über `setStateEvent`), zeichnet der Monitor es mit einem Zeitstempel und den zugehörigen numerischen Werten auf.

Die Ereignisse werden während der gesamten Sitzung im Speicher gehalten. Beim Beenden werden sie in eine zeitgestempelte JSON-Datei unter `<webroot>/monitor/` exportiert.

## Überwachte Module

| Modul | Erfasste Ereignisse |
|---|---|
| Guider | `guideRMS` — Gesamt-RMS des Guidings (arcsec) |
| Guider | `guideSNR` — Signal-Rausch-Verhältnis der Sternerkennung |
| Focus | `focusdone` — End-HFR nach der Fokussierung (Pixel) |
| Sequencer | `imagehfr` — HFR des gespeicherten Bilds (Pixel) |

## Sitzungsgraph

Der Hauptgraph zeigt die gesamte Sitzung auf einer gemeinsamen Zeitachse.

| Kurve | Beschreibung |
|---|---|
| Orange Linie | RMS des Guidings (linke Achse, arcsec) |
| Blaue Punkte | HFR der aufgenommenen Bilder (rechte Achse, Pixel) |
| Grüne Dreiecke | Am Ende der Fokussierung gemessene HFR (rechte Achse, Pixel) |
| Rote gestrichelte Linie | SNR des Guidings (standardmäßig ausgeblendet, über die Legende aktivierbar) |

Der Doppel-Schieberegler unter dem Graphen erlaubt die Einschränkung des angezeigten Zeitfensters ohne Verlust der im Speicher gehaltenen Daten.

## Aktionen

| Aktion | Beschreibung |
|---|---|
| **Start** | Startet eine neue Aufzeichnungssitzung (löscht vorherige Daten) |
| **Stop** | Beendet die Aufzeichnung und exportiert die Daten als JSON |

## Parameter

### Parms

| Parameter | Beschreibung |
|---|---|
| **Auto-start** | Startet die Aufzeichnung automatisch beim Laden des Moduls (im Profil gespeichert) |

### Ansichtsfilterung

| Parameter | Beschreibung |
|---|---|
| **From** | Untere Grenze der Anzeige (filtert Ereignisse vor dieser Uhrzeit) |
| **To** | Obere Grenze der Anzeige (filtert Ereignisse nach dieser Uhrzeit) |
| **Max rows** | Maximale Anzahl an Zeilen, die in der Live-Ansicht gehalten werden (Standard: 200) |

### Überwachte Module (Slaves)

Jeder Eintrag erlaubt die Auswahl der zu überwachenden Modulinstanz für jeden Typ (Guider, Sequencer, Focus usw.).

## JSON-Persistenz

Bei jedem Sitzungsende wird eine Datei exportiert unter:

```
<webroot>/monitor/YYYYMMDD-HHmmss.json
```

Die Datei enthält die Spaltenüberschriften (`gridheaders`) und die Datentabelle (`grid`) im Format Array von Arrays, direkt wiederverwendbar für eine externe Analyse.

{{% notice style="note" title="Automatischer Start" %}}
Bei aktiviertem **Auto-start** im Profil startet das Modul die Aufzeichnung eine Sekunde nach seinem Laden, ohne manuellen Eingriff.
{{% /notice %}}
