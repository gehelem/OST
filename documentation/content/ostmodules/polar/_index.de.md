---
title: Polar
weight: 60
description: Assistent für die Polausrichtung per Plate Solving
---

## Aufgabe

Das Polar-Modul unterstützt die Polausrichtung der Montierung. Es nimmt drei Himmelsbilder auf, wobei die Montierung zwischen jeder Aufnahme in RA bewegt wird, löst jedes Bild astrometrisch auf und berechnet dann den Azimut- und Höhenfehler der Polachse. Eine kontinuierliche Korrekturschleife führt den Benutzer anschließend, während er die Montierung mechanisch nachjustiert.

![Screenshot des Polar-Moduls](/images/modules/polar.png)

## Benötigte Geräte

| Gerät | Rolle |
|---|---|
| Kamera | Aufnahme der Bilder für das Plate Solving |
| Montierung | Bewegung in RA und Auslesen der Koordinaten |
| GPS | Geografische Position des Beobachters |

## Parameter

| Parameter | Beschreibung |
|---|---|
| **Belichtung** | Belichtungsdauer in Sekunden |
| **Gain** | Gain der Kamera |
| **Offset** | Offset der Kamera |
| **Brennweite** | Optische Brennweite in mm — zur Festlegung des Bildmaßstabs des Solvers |

## Algorithmus

### Phase 1 — Kalibrierung (3 Bilder)

Das Modul nimmt drei Bilder auf und bewegt sich dabei zwischen jeder Aufnahme um 30 Bogenminuten (0,5 h) in RA:

1. **Bild 0** — Ausgangsposition
2. Bewegung um ±0,5 h in RA (Richtung passend zur Meridianseite)
3. **Bild 1** — neue Position
4. Erneute Bewegung um ±0,5 h in RA
5. **Bild 2** — Endposition

Jedes Bild wird per Plate Solving aufgelöst. Die drei Lösungen (RA, DEC in J2000) werden zum genauen Aufnahmezeitpunkt in Horizontkoordinaten (Azimut/Höhe) umgerechnet, unter Berücksichtigung der geografischen Position des Beobachters.

### Berechnung der Polachse

Die drei Az/Alt-Punkte beschreiben einen Bogen auf der Himmelskugel, der der Rotation der Montierung entspricht. Das Modul berechnet den zu diesem Bogen normalen Vektor — das ist die Richtung der tatsächlichen Rotationsachse der Montierung.

Daraus werden die Fehler abgeleitet:

| Fehler | Definition |
|---|---|
| **Azimutfehler** | Abweichung zwischen dem Azimut der Achse und Norden (Azimut = 0°) |
| **Höhenfehler** | Differenz zwischen der Höhe der Achse und dem Breitengrad des Beobachters |
| **Gesamtfehler** | Kombinierter Fehler: `√(az² + alt²)` |

Die Fehler werden in Grad und Bogenminuten angezeigt.

### Phase 2 — Korrekturschleife

Nach der Kalibrierung tritt das Modul in eine kontinuierliche Schleife ohne Bewegung der Montierung ein:

1. Aufnahme eines Bilds
2. Plate Solving
3. Berechnung der Az/Alt-Verschiebung relativ zum Referenzbild (Bild 2)
4. Aktualisierung des Restfehlers = Anfangsfehler − bereits angewendete Korrektur

Der Benutzer justiert Azimut und Höhe der Montierung mechanisch nach; der Restfehler nimmt in Echtzeit ab, während sich die Polausrichtung verbessert.

### Grafische Überlagerung

Das angezeigte Bild enthält eine Überlagerung, die die anzuwendenden Korrekturen darstellt:

| Linie | Farbe | Bedeutung |
|---|---|---|
| Polachse → Höhenkorrektur | Gelb | Höhenfehler |
| Höhenkorrektur → Pol | Grün | Azimutfehler |
| Polachse → Pol | Rot | Gesamtfehler |

Das Koordinatensystem ist so ausgerichtet, dass der Himmelsnordpol oben im Bild liegt.

## Zustände

| Zustand | Beschreibung |
|---|---|
| **Idle** | Modul inaktiv |
| **Moving** | Montierung in Bewegung |
| **Shooting** | Bildaufnahme läuft |
| **Solving** | Plate Solving läuft |
| **Compute** | Berechnung des Polfehlers |

## Angezeigte Werte

### Kalibrierungsmessungen

| Wert | Beschreibung |
|---|---|
| **RA 0 / DE 0 / Time 0** | Astrometrische Lösung und Zeitstempel von Bild 0 |
| **RA 1 / DE 1 / Time 1** | Astrometrische Lösung und Zeitstempel von Bild 1 |
| **RA 2 / DE 2 / Time 2** | Astrometrische Lösung und Zeitstempel von Bild 2 |

### Polfehler

| Wert | Beschreibung |
|---|---|
| **Azimutfehler (°)** | Azimutfehler in Grad |
| **Höhenfehler (°)** | Höhenfehler in Grad |
| **Gesamtfehler (°)** | Kombinierter Fehler in Grad |
| **Azimutfehler (arcmin)** | Azimutfehler in Bogenminuten |
| **Höhenfehler (arcmin)** | Höhenfehler in Bogenminuten |
| **Gesamtfehler (arcmin)** | Kombinierter Fehler in Bogenminuten |

## Aktionen

| Aktion | Beschreibung |
|---|---|
| **Starten** | Startet die Kalibriersequenz (3 Bilder + Berechnung) und dann die Korrekturschleife |
| **Abbrechen** | Bricht die laufende Sequenz sofort ab |
