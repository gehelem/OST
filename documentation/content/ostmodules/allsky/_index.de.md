---
title: Allsky
weight: 90
description: Allsky-Kamera mit Keogramm, Stack, Timelapse und Wetterdaten
---

## Aufgabe

Das Allsky-Modul steuert eine Weitwinkelkamera (Allsky) in einer kontinuierlichen Schleife, um die Nacht zu dokumentieren. Bei jedem Bild erzeugt es eine Überlagerung mit Wetterdaten, erstellt in Echtzeit ein Keogramm und einen Stack und produziert am Ende der Sitzung ein Timelapse-Video. Vergangene Sitzungen werden archiviert und sind einsehbar.

![Screenshot des Allsky-Moduls](/images/modules/allsky.png)

## Benötigte Geräte

| Gerät | Rolle |
|---|---|
| Kamera | Aufnahme der Allsky-Bilder |
| GPS | Optional — geografische Position zur Berechnung von Sonnenauf- und -untergang |
| Wetterstation | Optional — Wetterdaten, die als Überlagerung und im Keogramm angezeigt werden |

## Parameter

### Aufnahme

| Parameter | Beschreibung |
|---|---|
| **Belichtung** | Belichtungsdauer in Sekunden |
| **Gain** | Gain der Kamera |
| **Offset** | Offset der Kamera |
| **Verzögerung (s)** | Intervall in Sekunden zwischen dem Start zweier aufeinanderfolgender Aufnahmen (muss größer sein als die Belichtungsdauer) |

### Planung

| Parameter | Beschreibung |
|---|---|
| **Manuell** | Startet sofort und läuft kontinuierlich, solange die Play-Aktion aktiv ist |
| **Feste Uhrzeit** | Aufnahme zwischen einer Start- und einer Endzeit (Tag oder Nacht, je nach Reihenfolge der Zeiten) |
| **Sonnenuntergang** | Startet automatisch bei Sonnenuntergang und stoppt bei Sonnenaufgang |

Für die Modi *Feste Uhrzeit* und *Sonnenuntergang* werden die Zeiten des nächsten Sonnenauf- und -untergangs automatisch aus der konfigurierten geografischen Position berechnet (libnova).

### Geografische Position

Die geografische Position (Breiten-, Längengrad) wird für die Berechnung der Sonnenzeiten benötigt. Sie kann manuell eingegeben oder von einem angeschlossenen GPS-Gerät importiert werden.

### Automatische Belichtung/Gain

| Parameter | Beschreibung |
|---|---|
| **Aktiviert** | Aktiviert die automatische Anpassung |
| **Belichtung oder Gain ändern** | Wählt den anzupassenden Parameter |
| **Minimalwert** | Untere Grenze der Anpassung |
| **Maximalwert** | Obere Grenze der Anpassung |
| **Messung** | Als Referenz verwendete Statistik (Mittelwert oder Median) |
| **Zielwert** | Zielwert der Statistik |
| **Schwellenwert (%)** | Derzeit nicht verwendet |

Der Korrekturkoeffizient wird berechnet durch: `neuer_wert = aktueller_wert × (ziel / aktuelle_messung)`.

### Archive

| Parameter | Beschreibung |
|---|---|
| **Einzelbilder behalten** | Wenn aktiviert, werden die einzelnen JPEGs jeder Sitzung im Archiv behalten; andernfalls werden nur Keogramm, Stack und Timelapse aufbewahrt |

## Verarbeitung jedes Bildes

Für jedes aufgenommene Bild:

1. **FITS-Speicherung** des Rohbilds
2. **Überlagerung** von Datum/Uhrzeit und Wetterdaten (Temperatur, Feuchtigkeit, Druck) in Farbe auf dem JPEG
3. **Maximum-Stack**: jedes Pixel behält den Maximalwert aller Bilder der Sitzung
4. **Keogramm**: ein 1 Pixel breiter vertikaler Streifen aus der Bildmitte wird an die vorherigen angehängt und bildet so einen zeitlichen Graphen der Nacht; Wetterdaten werden darin durch farbige Punkte dargestellt (rot = Temperatur, grün = Feuchtigkeit, blau = Druck)
5. **SNR-Protokoll**: das Signal-Rausch-Verhältnis des Bildes wird mit Zeitstempel aufgezeichnet

## Timelapse

Am Ende der Sitzung (Stopp oder Ende der Planung) erzeugt das Modul mittels ffmpeg ein Timelapse-Video (MP4 H.264 mit 30 fps) aus allen JPEGs der Sitzung.

## Archive

Jede abgeschlossene Sitzung wird in einen zeitgestempelten Archivordner verschoben. Die Archive sind direkt über die Oberfläche einsehbar, mit für jede Sitzung:

| Element | Beschreibung |
|---|---|
| **Datum** | Zeitstempel-Kennung der Sitzung |
| **Keogramm** | Zeitlicher Streifen der Nacht |
| **Stack** | Gestapeltes (Maximum-)Bild der Nacht |
| **Timelapse** | Video der Nacht |

## Wetterdaten

Wenn ein INDI-Wettergerät konfiguriert ist, liest das Modul die Eigenschaften `WEATHER_PARAMETERS`, um Temperatur, Feuchtigkeit und Druck zu erhalten. Diese Werte werden:
- In Echtzeit im Abschnitt **Messungen** angezeigt
- Auf jedem Bild überlagert
- Im Keogramm eingetragen
- In einem zeitlichen Graphen historisiert

## Aktionen

| Aktion | Beschreibung |
|---|---|
| **Play** | Startet die Aufnahmeschleife gemäß dem aktiven Planungsmodus |
| **Pause** | Unterbricht die Aufnahmen vorübergehend, ohne die Sitzung zu beenden |
| **Stop** | Beendet die Sitzung, erzeugt das Timelapse und archiviert die Sitzung |
