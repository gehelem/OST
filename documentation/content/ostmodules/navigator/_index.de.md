---
title: Navigator
weight: 40
description: Zielsuche und präzise Aufstellung per Plate Solving
---

## Aufgabe

Das Navigator-Modul erlaubt die Suche eines Objekts in astronomischen Katalogen, das Ausrichten der Montierung auf dieses Objekt und anschließend die präzise Zentrierung des Felds in einer Schleife mittels Plate Solving (astrometrische Auflösung). Es kann das ausgewählte Ziel außerdem an das Planner-Modul senden.

![Screenshot des Navigator-Moduls](/images/modules/navigator.png)

## Benötigte Geräte

| Gerät | Rolle |
|---|---|
| Kamera | Aufnahme der Bilder für das Plate Solving |
| Montierung | Empfang der Bewegungsbefehle |
| GPS | Bereitstellung von geografischen Koordinaten und Uhrzeit |

## Parameter

### Aufnahme

| Parameter | Beschreibung |
|---|---|
| **Belichtung** | Belichtungsdauer in Sekunden |
| **Gain** | Gain der Kamera |
| **Offset** | Offset der Kamera |
| **Brennweite** | Optische Brennweite in mm — zur Berechnung des Bildmaßstabs des Solvers |

### Zentrierung

| Parameter | Beschreibung |
|---|---|
| **Max. Iterationen** | Maximale Anzahl der Zyklen Slew → Solve → Correct, bevor abgebrochen wird |
| **Toleranz (arcsec)** | Akzeptabler Winkelabstand zwischen aufgelöstem Zentrum und Ziel |
| **Montierung synchronisieren** | Wenn aktiviert, werden die Koordinaten der Montierung nach einer erfolgreichen Zentrierung mit der Lösung synchronisiert |

### Slave-Module

| Parameter | Beschreibung |
|---|---|
| **Planner-Modulinstanz** | Name der für die Aktion *Zum Planer hinzufügen* zu verwendenden Planner-Modulinstanz |

## Funktionsweise

### Zielsuche

Die Suche erfolgt durch Eingabe eines Namens oder Codes im Feld **Objekt suchen**. Die verfügbaren Kataloge sind:

| Katalog | Inhalt |
|---|---|
| Messier | Objekte des Messier-Katalogs |
| NGC | New General Catalogue |
| LDN | Lynds' Catalogue of Dark Nebulae |
| SH2 | Sharpless-Katalog (H-II-Nebel) |
| IC | Index Catalogue |
| Sterne | Benannte Sterne |

Die Ergebnisse werden in der Tabelle **Ergebnisse** angezeigt mit: Katalog, Code, RA, DEC, Helligkeit, Durchmesser, Name und Alias. Ein Klick auf eine Zeile wählt das Ziel aus.

### Umrechnung J2000 → aktuelle Epoche

Sobald ein Ziel ausgewählt ist, werden seine J2000-Koordinaten in Koordinaten der aktuellen Epoche umgerechnet (Präzession berücksichtigt) und in **Umgerechnete Auswahl** angezeigt.

### Zentrierschleife

Die Aktion **Goto Ziel** löst folgende Sequenz aus:

1. Die Montierung richtet sich auf die zur aktuellen Epoche umgerechneten Koordinaten aus
2. Die Kamera nimmt ein Bild auf, das vom astrometrischen Solver aufgelöst wird (StellarSolver / astrometry.net-Indizes)
3. Der Winkelabstand zwischen aufgelöstem Zentrum und Ziel wird berechnet
4. Ist der Abstand kleiner als die **Toleranz**, wird die Zentrierung als erfolgreich gemeldet
5. Andernfalls wird ein Korrekturoffset berechnet und auf die Montierung angewendet; der Zyklus beginnt erneut

Dieser Vorgang wiederholt sich bis zum Erfolg oder bis die maximale Anzahl an Iterationen erreicht ist.

{{% notice style="tip" title="Montierungssynchronisation" %}}
Ist *Montierung synchronisieren* aktiviert, führt das Modul nach erfolgreicher Zentrierung einen INDI-SYNC durch, was die Genauigkeit nachfolgender Ausrichtungen verbessert.
{{% /notice %}}

## Angezeigte Werte

### Position der Montierung

| Wert | Beschreibung |
|---|---|
| **RA** | Aktuelle Rektaszension der Montierung |
| **DEC** | Aktuelle Deklination der Montierung |

### GPS-Position

| Wert | Beschreibung |
|---|---|
| **Höhe** | Höhe des Beobachtungsstandorts (m) |
| **Breitengrad** | Breitengrad des Beobachtungsstandorts (°) |
| **Längengrad** | Längengrad des Beobachtungsstandorts (°) |

### GPS-Datum und -Uhrzeit

| Wert | Beschreibung |
|---|---|
| **UTC-Versatz** | Zeitzone |
| **Datum** | Aktuelles Datum |
| **Uhrzeit** | Aktuelle Uhrzeit |

## Aktionen

| Aktion | Beschreibung |
|---|---|
| **Goto Ziel** | Richtet die Montierung auf das ausgewählte Ziel und startet die Zentrierschleife |
| **Abbrechen** | Bricht das laufende Goto oder die Zentrierung sofort ab |
| **Zum Planer hinzufügen** | Sendet das aktuelle Ziel an das Planner-Modul |
