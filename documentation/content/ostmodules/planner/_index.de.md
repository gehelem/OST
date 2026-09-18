---
title: Planner
weight: 50
description: Multi-Ziel-Planung und automatische Sitzungsorchestrierung
---

## Aufgabe

Das Planner-Modul orchestriert eine astrofotografische Sitzung mit mehreren Zielen. Es arbeitet eine Liste von Planungszeilen ab, indem es das Ausrichten dem Navigator-Modul und die Aufnahme dem Sequencer-Modul überträgt. Für jedes Ziel lädt es automatisch ein Sequenzprofil. Vor dem Ausrichten auf ein Ziel prüft es außerdem, ob dieses während der gesamten geschätzten Sequenzdauer über einer Mindesthöhe bleibt und ausreichend weit vom Mond entfernt ist, wobei Ziele, die diese Bedingungen nicht erfüllen, übersprungen und später erneut versucht werden.

![Screenshot des Planner-Moduls](/images/modules/planner.png)

## Benötigte Geräte

| Gerät | Rolle |
|---|---|
| GPS | Bereitstellung von geografischen Koordinaten und Uhrzeit |

## Struktur eines Plans

Ein Plan besteht aus mehreren Zeilen. Jede Zeile definiert:

| Spalte | Beschreibung |
|---|---|
| **Objekt** | Name des Ziels |
| **RA** | Rektaszension des Ziels (J2000) |
| **DEC** | Deklination des Ziels (J2000) |
| **Profil** | Im Sequencer für dieses Ziel zu ladendes Sequenzprofil |
| **Fortschritt** | Fortschritt der laufenden Zeile |

Die Zeilen werden nicht unbedingt strikt von oben nach unten abgearbeitet: eine Zeile, die die Sichtbarkeits-/Mondprüfung nicht besteht, wird übersprungen und später erneut versucht, sobald alle anderen Zeilen durchlaufen sind (siehe *Zustände einer Zeile* unten). Sie können manuell hinzugefügt oder vom Navigator über die Aktion *Zum Planer hinzufügen* gesendet werden.

## Parameter

### Slave-Module

| Parameter | Beschreibung |
|---|---|
| **Sequencer-Instanz** | Name der zu steuernden Sequencer-Modulinstanz |
| **Navigator-Instanz** | Name der zu steuernden Navigator-Modulinstanz |

### Sichtbarkeitsbedingungen

| Parameter | Beschreibung |
|---|---|
| **Minimum elevation (°)** | Das Ziel muss während der gesamten geschätzten Sequenzdauer über dieser Höhe bleiben, sonst wird die Zeile übersprungen |
| **Moon illumination threshold (%)** | Die Mondvermeidung greift nur, wenn dessen beleuchteter Anteil diesen Prozentsatz übersteigt |
| **Moon separation threshold (°)** | Die Zeile wird nur verworfen, wenn das Ziel näher als dieser Abstand liegt **und** die Mondbeleuchtung den obigen Schwellenwert übersteigt |
| **Max retries before giving up** | Eine Zeile, die die Sichtbarkeits-/Mondprüfung so oft nicht besteht, wechselt in den Zustand *Failed* und wird nie wieder versucht |

## Algorithmus

### Start

1. Prüfung der INDI-Verbindung und der Slave-Module
2. Markierung aller Zeilen im Zustand *Wartend* und Zurücksetzen der Versuchszähler
3. Start der ersten Zeile

### Ausführung einer Zeile

Für jede Zeile des Plans:

1. Der Planner fragt beim Sequencer die theoretische Dauer des angeforderten Profils ab (ohne es zu laden), über eine generische modulübergreifende Anfrage
2. Er prüft, ob das Ziel während dieser gesamten Dauer über der konfigurierten Mindesthöhe bleibt und ausreichend weit vom Mond entfernt ist (Beleuchtungs-/Abstandsschwellen) — schlägt eine der beiden Prüfungen fehl, wird die Zeile übersprungen (siehe *Zustände einer Zeile*) und der Planner geht zur nächsten über
3. Andernfalls konfiguriert er das Ziel des Navigators (RA, DEC, Name)
4. Er lädt das angeforderte Sequenzprofil im Sequencer und konfiguriert das entsprechende Objekt
5. Er löst die Aktion **Goto Ziel** des Navigators aus
6. Er wartet auf das Ende der Zentrierung (Erfolgsereignis des Navigators)
7. Er löst die Aktion **Sequenz starten** des Sequencers aus
8. Er verfolgt den Fortschritt der Sequenz und wartet auf ihr Ende
9. Die Zeile wird als *Finished* markiert, und der Planner geht zur nächsten verwertbaren Zeile über

### Zustände einer Zeile

| Zustand | Bedeutung | Später erneut versucht? |
|---|---|---|
| **Queued** | Noch nicht versucht | — |
| **Checking visibility** | Höhen-/Mondprüfung läuft | — |
| **Slewing** | Zentrierung des Navigators auf das Ziel | — |
| **Skipped : \<Grund\>** | Sichtbarkeits-/Mondprüfung fehlgeschlagen | Ja, sobald alle anderen Zeilen durchlaufen sind |
| **Failed : \<Grund\>** | Sichtbarkeits-/Mondprüfung zu oft fehlgeschlagen (*Max retries* erreicht) | Nein — endgültig ausgeschlossen |
| **Cancelled** | Die Sequenz wurde während der Ausführung manuell unterbrochen (im Gegensatz zu einem normalen Ende) | Nein — endgültig ausgeschlossen |
| **Finished** | Sequenz erfolgreich abgeschlossen | Nein |

{{% notice style="tip" title="Manueller Abbruch vs. normales Ende" %}}
Der Planner unterscheidet beide Fälle: Wird der Sequencer direkt unterbrochen, während er eine Planzeile ausführt, wird diese Zeile als *Cancelled* markiert, und der Planner geht zur nächsten verwertbaren Zeile über — sie wird nicht mit einem Erfolg verwechselt und danach nicht mehr erneut versucht.
{{% /notice %}}

### Ende des Plans

Der Planner durchläuft die Zeilen weiterhin in einer Schleife und versucht die noch *Skipped*-Zeilen erneut, bis jede Zeile entweder *Finished*, *Failed* oder *Cancelled* ist. Zu diesem Zeitpunkt erreicht der Gesamtfortschritt 100 %, und das Modul stoppt.

### Weitergabe des Stopps

Das Stoppen des Planners (ob über die Schaltfläche **Stoppen** oder weil ein übergeordnetes Modul wie der Park Manager es anfordert) unterbricht auch seine Slave-Module: Es sendet die Aktion **Unterbrechen** an den Sequencer und die Aktion **Unterbrechen** an den Navigator, damit ein unterbrochener Plan keine Belichtung oder Ausrichtung unbeaufsichtigt weiterlaufen lässt.

## Fortschritt

| Indikator | Beschreibung |
|---|---|
| **Gesamtfortschritt** | Anzahl der verarbeiteten Zeilen im Verhältnis zur Gesamtzahl |
| **Fortschritt pro Zeile** | Fortschritt der laufenden Aufnahme, vom Sequencer weitergegeben |

## Aktionen

| Aktion | Beschreibung |
|---|---|
| **Starten** | Startet die Ausführung des Plans ab der ersten Zeile |
| **Stoppen** | Unterbricht den laufenden Plan |
