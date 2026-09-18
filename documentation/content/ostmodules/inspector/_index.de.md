---
title: Inspector
weight: 70
description: Analyse der optischen Qualität (HFR, Sternform) und Kollimationsassistent
---

## Aufgabe

Das Inspector-Modul führt zwei unabhängige Analysen an einem Sternfeld durch. Jede wird separat aktiviert — keine, eine, die andere oder beide gleichzeitig:

- **Inspector-Analyse** — optische Qualität über das gesamte Bild, auf einem *fokussierten* Bild. Sie erzeugt drei ergänzende Karten (HFR pro Zone, Sternlängung, Eckenmosaik), mit denen sich optische Fehler wie Tilt, Koma oder Feldkrümmung diagnostizieren lassen.
- **Collimator-Analyse** — optische Kollimation (Spiegelausrichtung) von Newton-, RC- (Ritchey-Chrétien) und SC-Teleskopen (Schmidt-Cassegrain), auf einem *defokussierten* Bild. Sie analysiert das gesamte Sternfeld und zeigt fortlaufend die Kollimationsabweichung und die Korrekturen je Schraube an. Halbmanuell: Die Software defokussiert, nimmt auf und analysiert in einer Schleife, während der Benutzer physisch an den (nicht motorisierten) Kollimationsschrauben dreht und dabei die visuelle Rückmeldung nahezu in Echtzeit aktualisiert sieht.

Die Analyse kann auf einem live von der Kamera aufgenommenen Bild oder auf einer von der Festplatte geladenen FITS-Datei durchgeführt werden.

![Screenshot des Inspector-Moduls](/images/modules/inspector.png)

## Benötigte Geräte

| Gerät | Rolle |
|---|---|
| Kamera | Aufnahme der Bilder (optional bei Laden aus Datei) |
| Focuser (optional) | Automatische Steuerung der Defokussierung für den Kollimationsablauf, sowie „Home"-Positionsfunktion. Falls nicht vorhanden, manuelle Defokussierung vor jeder Aufnahme |

## Auswahl der Analysen

| Schalter | Effekt |
|---|---|
| **Inspector-Analyse** | Startet die HFR-/Form-/Ecken-Analyse für jedes aufgenommene (oder neu geladene) Bild |
| **Collimator-Analyse** | Startet die Kollimationsanalyse für jedes aufgenommene (oder neu geladene) Bild |

Beide Schalter sind unabhängig voneinander (nicht exklusiv) und werden mit dem Profil des Moduls gespeichert. Wird eine Analyse deaktiviert, werden ihre Ergebnisbilder gelöscht, damit niemals eine veraltete Karte neben einer frischen Karte aus einem anderen Bild angezeigt wird.

## Parameter

### Aufnahme

| Parameter | Beschreibung |
|---|---|
| **Belichtung** | Belichtungsdauer in Sekunden |
| **Gain** | Gain der Kamera |
| **Offset** | Offset der Kamera |
| **Brennweite** | Optische Brennweite in mm — zur Umrechnung von HFR und Kollimationsvektor in Bogensekunden |
| **Eckengröße (Pixel)** | Breite in Pixeln der für das Eckenmosaik extrahierten Patches |

### Analyse

| Parameter | Beschreibung |
|---|---|
| **Zoning** | N×N-Raster zur Bildaufteilung für die Zonenanalyse (von 1×1 bis 64×64) |

### Defokussierung

| Parameter | Beschreibung |
|---|---|
| **Focuser-Offset (Schritte)** | Verschiebung des Focusers (in Schritten), die durch *Go intra* / *Go extra* angewendet wird, um die defokussierte Position zu erreichen |

## Erzeugte Karten (Inspector-Analyse)

### HFR-Karte

Die HFR-Karte teilt das Bild in ein N×N-Raster (gemäß dem Parameter *Zoning*). Für jede Zone wird die mittlere HFR über alle erkannten Sterne berechnet und dann über die 8 Nachbarzellen geglättet.

Die Farbe jeder Zone reicht von grün (beste HFR) bis rot (schlechteste HFR).

Als Überlagerung:
- Blaue Kreise um jeden erkannten Stern, mit einem zur individuellen HFR proportionalen Radius
- Weißes Rechteck, das die Zentren der vier Quadranten verbindet, dessen Form die HFR-Verteilung widerspiegelt: ein perfektes Quadrat zeigt ideale Gleichmäßigkeit an
- HFR-Wert jedes Quadranten (oben-links, oben-rechts, unten-links, unten-rechts) und globale HFR, in Bogensekunden ausgedrückt

### Formkarte (Aberrationen)

Die Formkarte analysiert die Sternlängung pro Zone. Für jede Zone des Rasters wird ein Strich gezeichnet:

- **Orientierung**: Richtung der Hauptachse der Sterne (Mittelwinkel)
- **Länge**: proportional zur Längung `a/b − 1` (Verhältnis große Achse / kleine Achse)

Ein kurzer und gleichmäßiger Strich über das gesamte Bild zeigt runde Sterne an. Lange Striche am Rand deuten auf Koma oder Astigmatismus hin.

### Eckenmosaik

Das Mosaik setzt neun Patches der Größe *Eckengröße* zusammen, die aus neun Positionen des Originalbilds extrahiert und in einem 3×3-Raster angeordnet werden:

```
Oben-links   |  Oben-Mitte   |  Oben-rechts
Mitte-links  |  Zentrum      |  Mitte-rechts
Unten-links  |  Unten-Mitte  |  Unten-rechts
```

Diese Ansicht ermöglicht den direkten Vergleich der Sternqualität in der Mitte und an den Rändern des Feldes.

## Kollimationsanalyse

Ein defokussierter Stern erscheint als Ring ("Donut"), dessen Form und Zentrierung Aufschluss über die Kollimation geben. Im Gegensatz zu einem klassischen Ansatz, der einen einzelnen Stern intra- und extrafokal vergleicht, nutzt die Kollimationsanalyse **alle Sterne eines einzigen Felds in einer einzigen Aufnahme**:

1. Defokussierung (automatisch über den Focuser oder manuell)
2. Aufnahme eines Bilds mit mehreren im Feld verteilten Sternen
3. Erkennung jedes verwertbaren Donuts und Berechnung eines Verformungsvektors pro Stern
4. Regression über alle Vektoren zur Bestimmung des globalen Kollimationsvektors
5. Umrechnung in eine anzuwendende Korrektur für jede der 3 Schrauben
6. Neue Aufnahme, neue Analyse, kontinuierliche Schleife bis zur Konvergenz

### Warum das Vollfeld ausreicht

Für einen Stern an Position `P` im Bild (auf das Bildzentrum bezogenes Koordinatensystem) folgt der beobachtete Verformungsvektor einem linearen Modell:

```
D(P) = C - k·P
```

wobei `C` der gesuchte Kollimationsvektor ist (konstant, unabhängig von der Position im Feld) und `k` den Effekt der Feldkoma zusammenfasst (eine normale optische Aberration, die auch bei guter Kollimation vorhanden ist und mit der Entfernung zum Zentrum sowie der Öffnung des Teleskops zunimmt). Ein solches Vektorfeld hat immer einen eindeutigen Konvergenzpunkt — mit einigen Sternen an unterschiedlichen Feldpositionen erlaubt eine Regression, den Kollimations- vom Koma-Anteil zu trennen, ohne intra- und extrafokal vergleichen zu müssen.

Die Schaltflächen **Go intra**/**Go extra** bleiben für eine manuelle Überprüfung durch den Benutzer verfügbar, sind aber kein Schritt des automatisierten Ablaufs.

### Erkennung pro Stern

Für jeden erkannten Stern:
- Lokale adaptive Schwellenwertbildung, dann individuell verfeinert (ein einziger globaler Schwellenwert würde die schwächsten Sterne des Felds benachteiligen)
- Extraktion der Außenkontur des Rings und des Schattens des Sekundärspiegels (falls sichtbar)
- Eine Kreisanpassung nach kleinsten Quadraten an der Außenkontur ergibt das theoretische Zentrum des Sterns — diese Methode bleibt auch bei stark verformtem Ring (sichelförmig, bei starker Dekollimation) zuverlässig
- Der Verformungsvektor ist die Abweichung zwischen diesem theoretischen Zentrum und dem intensitätsgewichteten Schwerpunkt des Rings (Schatten ausgeschlossen)

### Regression und Konvergenzpunkt

Die Verformungsvektoren aller erkannten Sterne werden per kleinste Quadrate an das Modell `D(P) = C - k·P` angepasst. Der erhaltene Vektor `C` wird in Bogensekunden umgerechnet (Brennweite und Pixelgröße des Sensors) und dann auf die 3 im 120°-Abstand angeordneten Kollimationsschrauben projiziert, um Vorzeichen und Amplitude pro Schraube zu erhalten.

{{% notice style="note" title="Keine Kalibrierung in Schraubenumdrehungen" %}}
Die Amplitude pro Schraube wird in Bogensekunden ausgedrückt, nicht in Umdrehungen oder Bruchteilen davon. Der Zusammenhang zwischen einer optischen Abweichung und einer physischen Schraubbewegung hängt von der mechanischen Konstruktion des Sekundärspiegels ab (Gewindesteigung, Empfindlichkeit), die für jedes Instrument unterschiedlich ist — statt eine ungenaue Kalibrierung zu erfinden, erfolgt die Einstellung nach Gefühl: Amplitude und Richtung geben die Tendenz vor, der Benutzer justiert und beobachtet die Live-Aktualisierung.
{{% /notice %}}

### Kollimationskarte

Die Kollimationsanalyse veröffentlicht ihr eigenes annotiertes Bild (das Hauptbild bleibt ein Rohbild). Es enthält für jeden erkannten Stern:
- Die Außenkontur des Donuts
- Einen Verformungspfeil (übertrieben dargestellt, um sichtbar zu bleiben), farbig je nach Amplitude (grün / orange / rot)

Und für das gesamte Feld:
- Ein Bullseye (gepunktete Ringe), zentriert auf dem Bild, das dieselben Toleranzschwellen wie die Qualitätsanzeige darstellt
- Eine Linie, die das theoretische Zentrum des Bilds mit dem berechneten Konvergenzpunkt verbindet

#### Vor der Kollimation (Beispiel)

![Kollimations-Overlay, Kollimation muss korrigiert werden](/images/modules/collimator/collimator-ko.png)

#### Nach der Kollimation (Beispiel)

![Kollimations-Overlay, korrekte Kollimation](/images/modules/collimator/collimator-ok.png)

### Angezeigte Werte (Kollimation)

#### Kollimationsvektor

| Wert | Beschreibung |
|---|---|
| **C.x / C.y (px)** | Kollimationsvektor, in Bildpixeln |
| **Amplitude (arcsec)** | Amplitude des Kollimationsvektors, in Bogensekunden umgerechnet |
| **Convergence point x/y (px)** | Berechneter Konvergenzpunkt (an dem sich die verlängerten Verformungsvektoren schneiden) |
| **Quality** | Anzeige: grün bei geringer Amplitude, orange im Zwischenbereich, rot über dem oberen Schwellenwert |

#### Korrekturen pro Schraube

| Wert | Beschreibung |
|---|---|
| **Screw 1 / 2 / 3 (turns)** | Amplitude und Korrekturrichtung pro Schraube, in Bogensekunden ausgedrückt (positiv = anziehen) — feste 120°-Konvention, keine automatische Erkennung der tatsächlichen Schraubenausrichtung |

#### Erkannte Sterne

Tabelle mit jedem verwertbaren Stern der letzten Analyse: Position (x, y) und Verformungsvektor (dx, dy) in Pixeln.

## Zustände

| Zustand | Beschreibung |
|---|---|
| **Idle** | Modul inaktiv |
| **Shooting** | Bildaufnahme läuft |
| **Analyzing** | Analyse läuft |

## „Home"-Position des Focusers

*Set home* speichert die aktuelle absolute Position des Focusers; *Go home* bringt den Focuser danach wieder dorthin zurück. Die Position wird beim Start **nicht** gelesen und **nicht** gespeichert — es gibt keine Garantie, dass der Fokus zu diesem Zeitpunkt zumindest grob stimmt, sie muss daher explizit festgelegt werden. Die Eigenschaft **Focuser home** zeigt an, ob eine Home-Position definiert ist, und deren Wert; solange *Set home* nicht verwendet wurde, gibt *Go home* nur eine Warnung aus und tut nichts.

## Laden aus Datei

Das Feld **Datei auswählen** erlaubt das Laden einer FITS-Datei von der Festplatte zur Analyse, ohne eine Aufnahme auszulösen. Die Analyse startet dann automatisch bei Eingabe des Pfads, gemäß der aktuellen *Auswahl der Analysen*.

## Aktionen

| Aktion | Beschreibung |
|---|---|
| **Shoot** | Nimmt ein einzelnes Bild auf und startet die aktivierten Analysen |
| **Loop** | Kontinuierliche Aufnahme und Analyse — wiederholt sich automatisch bis zum Stopp |
| **Stop** | Beendet die kontinuierliche Schleife |
| **Reload** | Lädt das letzte Bild oder die letzte Datei neu und analysiert erneut |
| **Set home** | Speichert die aktuelle Position des Focusers als Rückkehrposition |
| **Go home** | Bringt den Focuser zur gespeicherten Home-Position zurück (warnt, wenn keine definiert ist) |
| **Go intra** | Bewegt den Focuser zur intrafokalen Position (manuelle Überprüfung) |
| **Go extra** | Bewegt den Focuser zur extrafokalen Position (manuelle Überprüfung) |

Die Schaltflächen **Go intra** / **Go extra** / **Go home** bleiben während der Bewegung des Focusers gedrückt und werden freigegeben, wenn er das Ziel erreicht.
