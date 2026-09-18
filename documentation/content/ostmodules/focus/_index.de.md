---
title: Focuser
weight: 10
description: Automatischer Fokus durch Polynomanpassung und Sensor-Tilt-Kartierung
---

## Aufgabe

Das Focuser-Modul übernimmt die automatische Fokussierung des Teleskops. Es bewegt den motorisierten Fokussierer über einen Positionsbereich, misst an jedem Punkt die Sterngröße (HFR — Half Flux Radius) und berechnet dann die optimale Position durch eine Polynomanpassung 2. Grades. Bei aktiviertem Zoning erzeugt es zusätzlich eine Kartierung des Sensor-Tilts.

## Oberfläche

Die Oberfläche zeigt in Echtzeit die im Aufbau befindliche HFR-Kurve, den Fortschritt der Sequenz und die Ergebnisse (endgültige HFR und Position). Die Parameter sind über die Schaltfläche **Parameter** oben rechts erreichbar.

![Screenshot des Focuser-Moduls](/images/modules/focus.png)

Die HFR-Kurve wird mit Fehlerbalken gezeichnet, die die Standardabweichung der Messungen an jeder Position darstellen, und eine Polynomanpassung erscheint als gestrichelte orange Linie, während die Aufnahmen fortschreiten.

![HFR-Kurve mit Fehlerbalken und Polynomanpassung](/images/modules/focus/focus-chart.png)

## Benötigte Geräte

| Gerät | Rolle |
|---|---|
| Kamera | Aufnahme der Bilder für die HFR-Messung |
| Motorisierter Fokussierer | Bewegung des Fokus |
| Filterrad | Optional — verwendet, falls konfiguriert |

## Parameter

Diese Parameter sind über das Menü **Parameter** des Moduls erreichbar. Ein Parametersatz kann in einem Profil gespeichert werden.

### Belichtung

| Parameter | Beschreibung |
|---|---|
| **Belichtung** | Belichtungsdauer in Sekunden |
| **Gain** | Gain der Kamera |
| **Offset** | Offset der Kamera |

### Fokus

| Parameter | Beschreibung |
|---|---|
| **Startposition** | Absolute Position des Fokussierers zu Beginn der Sequenz (wird ignoriert, wenn *Um die aktuelle Position* aktiviert ist) |
| **Um die aktuelle Position** | Zentriert die Sequenz automatisch um die aktuelle Position des Fokussierers: `aktuelle_pos − (schritt × iterationen / 2)` |
| **Schritt** | Abstand in Fokussierer-Einheiten zwischen zwei aufeinanderfolgenden Messungen |
| **Iterationen** | Anzahl der Messpunkte. Mit 5 Iterationen und einem Schritt von 2000 durchläuft der Fokussierer einen Bereich von 8000 Einheiten |
| **Backlash** | Überschuss, der vor jeder Bewegung angewendet wird, um das mechanische Spiel zu korrigieren. Der Fokussierer fährt um diesen Wert zurück, bevor er sich auf das Ziel zu bewegt |
| **Zoning** | Aufteilung des Bilds in Zonen (1×1 bis 9×9). Siehe eigener Abschnitt unten |

## Algorithmus

Die Fokussierung läuft in zwei Phasen ab.

### Phase 1 — Erkundung

1. Der Fokussierer positioniert sich auf `startposition − backlash` (Berücksichtigung des mechanischen Spiels)
2. Er fährt vor zur `startposition`
3. Für jede Iteration:
   - Zurücksetzen des Kamera-Subframes
   - Aufnahme eines Bilds, Berechnung der HFR und ihrer Standardabweichung
   - Aufzeichnung des Tripels (Position, HFR, Standardabweichung)
   - Fortlaufende Aktualisierung der Polynomanpassung
   - Vorrücken um einen *Schritt*

Die Polynomanpassung 2. Grades liefert kontinuierlich die Position des Minimums der HFR-Kurve:

```
optimale_position = −k1 / (2 × k2)    [für k0 + k1·x + k2·x²]
```

### Phase 2 — Endpositionierung

1. Der Fokussierer fährt zurück zu `beste_position − backlash`
2. Er fährt vor zur vom Polynom berechneten optimalen Position
3. Ein letztes Bild wird aufgenommen, um die resultierende HFR zu messen

{{% notice style="tip" title="Rückgabewert" %}}
Die verwendete optimale Position ist die des **Polynom-Fits** (`bestposfit`). Konnte der Fit nicht berechnet werden (weniger als 3 Punkte), wird die direkt gemessene beste Position (`bestpos`) verwendet.
{{% /notice %}}

## HFR-Kurve

Die HFR-Kurve wird während der Erkundung in Echtzeit gezeichnet und bei jedem Messpunkt aktualisiert. Sie besteht aus zwei überlagerten Elementen:

- **Gemessene Punkte** (grün) mit **Fehlerbalken**, die ±1 Standardabweichung der Messungen an jeder Position darstellen
- **Polynomkurve** (gestrichelt orange) — Anpassung 2. Grades, bei jedem neuen Punkt aktualisiert

Die X-Achse zeigt die absolute Position des Fokussierers, die Y-Achse die HFR in Pixeln.

{{% notice style="note" title="Fehlerbalken" %}}
Die angezeigte Standardabweichung entspricht der Streuung der einzelnen, pro Stern gemessenen HFR-Werte an derselben Fokusposition. Ein breiter Fehlerbalken deutet auf ein variables Seeing oder eine starke Turbulenz zum Aufnahmezeitpunkt hin.
{{% /notice %}}

## Zoning

Zoning unterteilt das Bild in ein Raster von N×N Zonen und berechnet für jede Zone eine unabhängige Polynomanpassung. Es ermöglicht sowohl eine feine Analyse der HFR-Variationen über das Feld als auch eine Kartierung des Sensor-Tilts.

| Wert | Raster | Verwendung |
|---|---|---|
| 1 | 1×1 | Kein Zoning — eine einzige globale Messung |
| 2 | 2×2 | Grundlegende Tilt-Erkennung (4 Quadranten) |
| 3 | 3×3 | Gängige Analyse — Mitte + Rand + Ecken |
| 5 | 5×5 | Feine Analyse |
| 9 | 9×9 | Sehr feine Analyse — 81 Zonen |

Für jede Zone berechnet das Modul über den Welford-Algorithmus (inkrementelle Aktualisierung in einem Durchgang):
- Die **mittlere HFR** der Zone
- Die **HFR-Standardabweichung** der Zone — Indikator für die lokale optische Qualität

## Kartierung des Sensor-Tilts

Wenn Zoning aktiviert ist (≥ 2×2), erzeugt das Modul am Ende der Sequenz automatisch vier Kartierungsbilder. Jedes zeigt die optimale Fokusposition jeder Zone, farbkodiert (blau = niedrige Position / nahe Ebene, rot = hohe Position / entfernte Ebene).

![Beispiele von Sensor-Tilt-Kartierungen (4 Bilder)](/images/modules/focus/focus-tilt-maps.png)

Das Breite-Höhe-Verhältnis der Bilder entspricht dem des Sensors.

### rawTilt — Roher Tilt pro Zone

Jede Zone wird durch einen einheitlichen Farbblock proportional zur besten Fokusposition dargestellt. Keine Interpolation — die räumliche Auflösung entspricht der des Zoning-Rasters.

![rawTilt — Farbblöcke pro Zone](/images/modules/focus/focus-rawtilt.png)

### localTiltLinear — Lokale bilineare Interpolation

Bilineare Interpolation zwischen den Zentren benachbarter Zonen. Erzeugt eine glatte Karte ohne Annahme über die globale Form der Fokusebene.

### globalTiltLinear — Globale Ebene (lineare GSL-Anpassung)

Anpassung einer Ebene per Methode der kleinsten Quadrate (GSL-Bibliothek) über alle Zonenpositionen. Ideal, um einen reinen Tilt zu quantifizieren: Neigung des Sensors relativ zur optischen Achse. Das in jeder Zelle angezeigte Residuum (gelb) ist die Abweichung zwischen Messung und angepasster Ebene.

### globalTiltQuadratic — Quadratische Fläche (GSL-Anpassung 2. Grades)

Anpassung einer Fläche 2. Grades. Erfasst nicht-ebene Fehler: Feldkrümmung, Sensorbiegung. Die Farbskala deckt das tatsächliche Minimum und Maximum der Fläche ab (Scheitelpunkt eingeschlossen, falls er innerhalb der Sensorgrenzen liegt).

{{% notice style="note" title="Interpretation" %}}
- **rawTilt / localTiltLinear**: lokale Diagnosen, nützlich, um eine durch einen schlechten Stern verursachte Ausreißerzone zu erkennen.
- **globalTiltLinear**: direkte Messung des Tilts. Ist der Gradient gleichmäßig, reicht ein Neigen des Sensors in die angezeigte Richtung zur Korrektur aus.
- **globalTiltQuadratic**: ist das Residuum der globalen Ebene groß und die quadratische Fläche deutlich besser, sind Feldkrümmung oder eine mechanische Biegung die Ursache.
{{% /notice %}}

## In Echtzeit angezeigte Werte

| Wert | Beschreibung |
|---|---|
| **Status** | Aktueller Zustand der Sequenz (Idle / Running / Ok / Error) |
| **Fortschritt** | Globaler Fortschrittsbalken der Sequenz |
| **HFR** | An der Endposition gemessene HFR |
| **Position** | Endposition des Fokussierers |

## Endergebnis

Am Ende der Fokussierung veröffentlicht das Modul:

| Ergebnis | Beschreibung |
|---|---|
| **Endposition** | Absolute Position des Fokussierers nach der Positionierung |
| **End-HFR** | An der optimalen Position gemessene HFR |

Diese Werte werden über das modulübergreifende Ereignissystem auch an andere Module (Sequencer) weitergegeben.

## Aktionen

| Aktion | Beschreibung |
|---|---|
| **Fokussierung starten** | Startet die vollständige Sequenz |
| **Abbrechen** | Unterbricht die laufende Sequenz sofort |
