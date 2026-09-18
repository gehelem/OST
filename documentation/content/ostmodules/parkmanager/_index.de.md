---
title: Park Manager
weight: 70
description: Automatisierung des Öffnens und Schließens der Sternwarte
---

## Aufgabe

Das Park-Manager-Modul steuert das vollständige Öffnen und Schließen der Sternwarte: Öffnen der Kuppelklappe, Entparken der Montierung, Start der Nachführung — und in umgekehrter Reihenfolge zum Schließen. Es kann im manuellen Modus (einzelne Aktion) oder im automatischen Modus, gesteuert durch Zeit- und Wetterregeln, betrieben werden.

![Screenshot des Park-Manager-Moduls](/images/modules/parkmanager.png)

## Benötigte Geräte

| Gerät | Rolle |
|---|---|
| Kuppel | Öffnen und Schließen der Klappe, Parken |
| Montierung | Entparken, Nachführung, Rückkehr zur Home-Position, Parken |
| Wetterstation | Überwachung der Bedingungen für den automatischen Modus (optional) |

## Parameter

### Wetterregeln

| Parameter | Beschreibung |
|---|---|
| **Must be OK** | Wenn aktiviert, wird das Öffnen verweigert, wenn das Wetter nicht grün ist. Während der Sitzung löst eine Wetterverschlechterung automatisch das Schließen aus |
| **Disabled** | Deaktiviert die Wetterüberwachung (die Wetterstation wird ignoriert) |

### Slave-Module

| Parameter | Beschreibung |
|---|---|
| **Planner instance** | Name der Planner-Modulinstanz, die beim Start der Schließsequenz gestoppt wird |

### Zeitregeln

Diese Regeln definieren das automatische Öffnungsfenster im **Automate**-Modus. Sie schließen sich gegenseitig aus.

| Parameter | Beschreibung |
|---|---|
| **Fixed time** | Festes Zeitfenster, definiert durch eine Start- und eine Endzeit |
| **Fixed time : start** | Startzeit des festen Fensters |
| **Fixed time : end** | Endzeit des festen Fensters (kann am nächsten Tag liegen — Behandlung des Mitternachtsübergangs) |
| **Dusk and dawn** | Automatisches Fenster von Sonnenuntergang bis Sonnenaufgang (berechnet aus den GPS-Koordinaten) |
| **Anytime** | Dauerhaftes Öffnen: das Modul öffnet, sobald es im Auto-Modus ist, und bleibt offen |

## Sequenzen

### Öffnungssequenz

1. Wetterprüfung (wenn die Regel *Must be OK* aktiv ist)
2. Öffnen der Kuppelklappe
3. Warten auf Bestätigung der Klappenöffnung
4. Entparken der Montierung
5. Warten auf Bestätigung des Entparkens
6. Start der Sternzeitnachführung
7. Eintritt in die Überwachung (*Open Monitoring*)

### Schließsequenz

1. Abschalten der Kühlung bei allen verbundenen Geräten, die einen `CCD_COOLER`-Schalter besitzen (nicht nur die vom Sequencer verwendete Kamera), und Stoppanforderung an die Planner-Instanz
2. Stopp der Sternzeitnachführung
3. Senden der Montierung in die Home-Position
4. Warten auf Erreichen der Home-Position
5. Parken der Montierung
6. Warten auf Bestätigung des Parkens
7. Schließen der Kuppelklappe
8. Warten auf Bestätigung der Klappenschließung

{{% notice style="warning" title="Reihenfolge beim Schließen" %}}
Die Montierung wird immer **vor** dem Schließen der Kuppel geparkt. Die Sequenz zwischen diesen beiden Schritten niemals manuell unterbrechen.
{{% /notice %}}

{{% notice style="tip" title="Weitergabe des Stopps" %}}
Das Stoppen des Planners unterbricht seinerseits den Sequencer (der wiederum den Guider und den Focuser unterbricht) sowie den Navigator — siehe die [Planner](../planner/)-Dokumentation. Dies stellt sicher, dass die gesamte Aufnahmekette ordnungsgemäß gestoppt wird, bevor die Sternwarte physisch geschlossen wird.
{{% /notice %}}

### Notstopp

Die Aktion **Abort** sendet gleichzeitig einen Stoppbefehl an die Montierung (`TELESCOPE_ABORT_MOTION`) und an die Kuppel (`DOME_ABORT_MOTION`) und wechselt anschließend in den Idle-Modus.

## Automatischer Modus

Im **Automate**-Modus prüft das Modul alle 2 Sekunden, ob die Sternwarte gemäß den konfigurierten Zeit- und Wetterregeln geöffnet oder geschlossen werden soll.

{{< mermaid >}}
flowchart TB
    ATTENTE([Warten]) -->|"Fenster aktiv & Wetter OK"| OD

    subgraph OUV ["Öffnungssequenz"]
        direction LR
        OD[Kuppel öffnen] --> OW[Warten auf Öffnung] --> UD[Montierung entparken] --> UW[Warten auf Entparken] --> TR[Nachführung starten]
    end

    TR --> SURV([Überwachung])
    SURV -->|"Fenster beendet oder Wetter verschlechtert"| ST

    subgraph FER ["Schließsequenz"]
        direction LR
        ST[Nachführung stoppen] --> GH[Goto home] --> WH[Warten auf home] --> PM[Montierung parken] --> WP[Warten auf Parken] --> CD[Kuppel schließen]
    end

    CD --> ATTENTE
    ATTENTE -->|Abort| IDLE([Idle])
    SURV -->|Abort| IDLE
{{< /mermaid >}}

| Situation | Aktion |
|---|---|
| Fenster offen + Sternwarte geschlossen | Start der Öffnungssequenz |
| Fenster geschlossen + Sternwarte offen | Start der Schließsequenz |
| Wetterverschlechterung während der Sitzung | Start der Schließsequenz |
| Ende der Sequenz | Rückkehr in den Wartezustand — der Auto-Modus bleibt erhalten |

{{% notice style="tip" title="Persistenz des Auto-Modus" %}}
Nach einer Öffnungs- oder Schließsequenz bleibt das Modul im **Auto**-Modus und überwacht weiterhin die Bedingungen. Eine manuelle Reaktivierung ist nicht nötig.
{{% /notice %}}

## Aktionen

| Aktion | Beschreibung |
|---|---|
| **Open all** | Startet die vollständige Öffnungssequenz |
| **Close all** | Startet die vollständige Schließsequenz |
| **Automate** | Aktiviert den durch die Regeln gesteuerten automatischen Modus |
| **Abort motion** | Notstopp aller Bewegungen |
| **Idle** | Wechselt ohne Aktion in den inaktiven Modus |

## Angezeigter Zustand

### Montierung

| Wert | Beschreibung |
|---|---|
| **Mount RA** | Aktuelle Rektaszension |
| **Mount DEC** | Aktuelle Deklination |
| **Mount is tracking** | Sternzeitnachführung aktiv |
| **Mount is parked** | Montierung in Parkposition |
| **Mount is at home** | Montierung in Home-Position |

### Kuppel

| Wert | Beschreibung |
|---|---|
| **Dome is parked** | Kuppel in Parkposition |
| **Dome shutter is closed** | Klappe geschlossen |

### Wetter und Ereignisse

| Wert | Beschreibung |
|---|---|
| **Global** | Globaler Wetterstatus (grün / orange / rot) |
| **Next sunset** | Uhrzeit des nächsten Sonnenuntergangs |
| **Next sunrise** | Uhrzeit des nächsten Sonnenaufgangs |
