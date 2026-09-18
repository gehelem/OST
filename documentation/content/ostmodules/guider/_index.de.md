---
title: Guider
weight: 20
description: Automatisches Guiding durch Driftkorrektur
---

## Aufgabe

Das Guider-Modul übernimmt das automatische Guiding des Teleskops. Es erkennt und verfolgt ein Referenzsternfeld, misst die Drift in Echtzeit und sendet Korrekturimpulse an die Montierung, um die Ausrichtung zu halten.

![Screenshot des Guider-Moduls](/images/modules/guider.png)

## Benötigte Ausrüstung

| Ausrüstung | Rolle |
|---|---|
| Guiding-Kamera | Aufnahme der Bilder des Guiding-Felds |
| Guiding-Schnittstelle | Empfängt die zeitgesteuerten Impulse (`TELESCOPE_TIMED_GUIDE_*`) — die Montierung selbst, oder eine Box, die ST4-Impulse weiterleitet |

## Betriebsphasen

Das Guiding läuft in drei aufeinanderfolgenden Phasen ab.

### Phase 1 — Initialisierung

Das Modul nimmt ein Referenzbild auf und behält die Liste der erkannten Sterne als Referenz für die gesamte Guiding-Sitzung. Wird das Guiding mit einer gültigen gespeicherten Kalibrierung gestartet, geht nur diese Phase dem Guiding voraus: eine neue Referenz wird an der aktuellen Ausrichtung aufgenommen, was die Wiederverwendung der Kalibrierung nach einer Bewegung der Montierung ermöglicht.

### Phase 2 — Kalibrierung

Das Modul misst die Reaktion der Montierung auf Korrekturimpulse.

1. Eine feste Serie von Impulsen wird nach Westen, dann Osten, dann Norden, dann Süden gesendet (`Kalibrierschritte` Impulse pro Richtung).
2. Die Bild-für-Bild-Verschiebung jedes Durchgangs wird gegenüber dem vorherigen Bild gemessen.
3. **Die Impulsrate wird aus den antisymmetrischen Kombinationen `(West − Ost) / 2` und `(Nord − Süd) / 2` abgeleitet.** Ein einseitiger Durchgang ist durch die konstante Hintergrunddrift verzerrt (Fehler der Sternzeitverfolgung, periodischer Fehler, Aufstellungsfehler); die Differenz der beiden entgegengesetzten Durchgänge hebt diese Drift auf und liefert eine saubere RA-Rate und eine saubere DEC-Rate. Die rohen Raten pro Richtung werden weiterhin zu Diagnosezwecken angezeigt.
4. Die Sensorausrichtung wird aus beiden Achsen abgeleitet (eine Schätzung über die RA-Achse und eine über die DEC-Achse, gemittelt), wobei die 180°-Ambiguität relativ zur RA-Achse aufgelöst wird.
5. Die RA-Rate wird auf den Äquator normiert (`rate × cos(Kalibrierungs-DEC)`), damit sie bei jeder beliebigen Deklination zum Zeitpunkt des Guidings übertragen werden kann.

Vor der Nord- und vor der Südserie nimmt ein optionaler *DEC-Spielausgleich*-Impuls zunächst das mechanische Spiel der DEC-Achse auf, damit die zeitgesteuerten Messimpulse nicht teilweise beim erneuten Eingreifen der Zahnräder verloren gehen. Seine eigene Verschiebung wird aus der Ratenberechnung ausgeschlossen.

Das Kalibrierergebnis wird gespeichert und von einer Sitzung zur nächsten wiederverwendet, solange sich die Meridianseite und die Kameraausrichtung nicht geändert haben.

{{% notice style="note" title="Qualität der Kalibrierung" %}}
Nach den vier Durchgängen berechnet das Modul ein *Driftverhältnis* — das Ausmaß der konstanten Hintergrunddrift im Verhältnis zur durch die Impulse verursachten Verschiebung — und eine Anzeige **Kalibrierqualität** (grün / orange / rot). Überschreitet das Verhältnis das *Max. Hintergrunddriftverhältnis*, wird die Kalibrierung verworfen und bis zu *Anzahl Kalibrierversuche* Mal neu gestartet, dann mit einer Meldung abgebrochen, die zur Überprüfung der Aufstellung auffordert. Eine Kalibrierung wird ebenfalls abgebrochen, wenn sich der Stern um mehr als die *Max. Sternabweichung* von seiner Startposition entfernt, noch bevor er den Sensor verlassen könnte.
{{% /notice %}}

### Phase 3 — Guiding

Kontinuierliche Korrekturschleife:

1. Ein Bild aufnehmen.
2. Das Sternfeld gegenüber der Referenz verfolgen (siehe *Sternverfolgung* unten).
3. Die Drift berechnen, projiziert auf die RA- und DEC-Achsen mit der Kalibrierausrichtung.
4. **PI-Regelung** — die Korrektur beträgt `aggressivität × drift + integralverstärkung × akkumulierte drift`. Der proportionale Term allein kann eine konstante Drift nicht aufheben (er stabilisiert sich bei `drift / aggressivität`); der Integralterm wächst weiter, bis das Residuum tatsächlich null ist. Der Akkumulator ist begrenzt (*Integralgrenze*) und wird für eine Achse eingefroren, deren Impuls bereits am Maximum liegt.
5. Die RA-Rate wird an die aktuelle Deklination angepasst: `äquatoriale_rate / cos(aktuelle DEC)`.
6. Ist die *DEC-Spielkompensation* aktiv und hat sich die DEC-Richtung seit der letzten Korrektur umgekehrt, wird die aktuelle Kompensationsschätzung zusätzlich addiert.
7. Die Korrekturimpulse werden an die Montierung gesendet, begrenzt durch *Min. Impuls* / *Max. Impuls*.
8. Die Statistiken werden aktualisiert (RMS RA, DEC, gesamt).
9. Wurde im vorherigen Zyklus ein Umkehrkompensationsimpuls gesendet, wird die tatsächlich erreichte Driftreduzierung mit der Vorhersage verglichen und die Kompensationsschätzung entsprechend nach oben (Unterkorrektur) oder unten (Überkorrektur) angepasst, innerhalb der konfigurierten Grenzen.

Das **erste Bild** einer Sitzung wird gemessen, aber nicht korrigiert — es legt lediglich den Arbeitsnullpunkt fest, damit ein paar Stabilisierungspixel nach einer Bewegung keinen großen ersten Impuls auslösen.

Ein vorübergehender Verlust der Sternkorrelation lässt das Bild einfach **überspringen** (kein Impuls); das Guiding wird erst nach *Max. aufeinanderfolgende verlorene Bilder* in Folge abgebrochen. Ein **Hardware-Watchdog** bricht die Sitzung ab, wenn eine Belichtung, ein Impuls, ein Frame-Reset oder eine Sternextraktion nie innerhalb der *Hardware-Zeitlimit* abgeschlossen wird.

{{% notice style="note" title="DEC-Spielkompensation" %}}
Die Getriebe der Montierung haben ein mechanisches Spiel: bei jeder Richtungsumkehr des DEC-Guidings nehmen die ersten Impulse dieses Spiel teilweise auf, statt eine echte Bewegung zu erzeugen, was zu einer vorübergehenden Driftspitze führt. Das tatsächliche Spiel variiert mit der Zeigeposition und der Last, sodass ein fester Wert unter- oder überkorrigiert. Die adaptive Schätzung regelt sich selbst: jede Umkehr ist gleichzeitig eine Korrektur und eine Messung, sie konvergiert daher zum tatsächlich vorhandenen Spiel.
{{% /notice %}}

## Sternverfolgung

Zwischen zwei Bildern erfährt das Sternfeld eine fast reine Translation (die 180°-Drehung bei einem Meridianschwenk wird separat durch die Umkehrindikatoren behandelt). Die Verfolgung sucht direkt die **dominante Translation**:

1. Jedes Paar (Referenzstern, aktueller Stern) impliziert eine Kandidaten-Translation; die richtige ist diejenige, auf die sich die meisten Sterne einigen (Abstimmungs-/RANSAC-Schritt).
2. Ihre konsistenten Punkte werden durch ein zweistufiges Fenster und ein iteratives Sigma-Clipping verfeinert, und die Drift ist ein flussgewichteter Mittelwert der Residuen — kontinuierlich und subpixelgenau.

Diese Methode ist unempfindlich gegenüber einer Änderung der Sternrangfolge nach Fluss und dem Auftauchen oder Verschwinden von Sternen von einem Bild zum nächsten. Die zugehörigen Einstellungen befinden sich unter *Sternabgleich* in den Parametern.

{{% notice style="note" title="Deklinationsübertragung der RA-Rate" %}}
Ein Impuls verschiebt den Stern um `rate × impuls × cos(DEC)` auf dem Sensor, daher variieren die ms/px mit `1 / cos(DEC)`. Die RA-Rate wird daher zum Zeitpunkt der Kalibrierung auf den Äquator normiert gespeichert und beim Guiding durch `cos(aktuelle DEC)` geteilt, damit eine bei einer Deklination durchgeführte Kalibrierung bei einer anderen korrekt bleibt.
{{% /notice %}}

## Parameter

Die Parameter sind über das Menü **Parameter** des Moduls erreichbar, und ein Satz kann als Profil gespeichert werden.

### Ausrüstung

| Parameter | Beschreibung |
|---|---|
| Belichtung | Belichtungsdauer in Sekunden |
| Gain | Gain der Kamera |
| Offset | Offset der Kamera |

### Kalibrierung

| Parameter | Beschreibung |
|---|---|
| **Impulsdauer** | Dauer in ms jedes Kalibrierimpulses |
| **Kalibrierschritte** | Anzahl der Impulse pro Richtung |
| **DEC-Spielausgleich** | Zusätzlicher Impuls (ms) vor den Nord- und Südserien, um zunächst das mechanische Spiel aufzunehmen — 0 zum Deaktivieren |
| **Max. Hintergrunddriftverhältnis** | Kalibrierung verwerfen, wenn die konstante Hintergrunddrift diesen Anteil der durch die Impulse verursachten Verschiebung überschreitet — ein hoher Wert deutet in der Regel auf eine mangelhafte Aufstellung hin. 0 zum Deaktivieren der Prüfung |
| **Anzahl Kalibrierversuche** | Wie oft eine verworfene Kalibrierung neu gestartet wird, bevor abgebrochen wird |
| **Max. Sternabweichung** | Kalibrierung abbrechen, wenn sich der Stern um mehr als diesen Anteil der kleinsten Bilddimension von seiner Startposition entfernt. 0 zum Deaktivieren |

### Guiding

| Parameter | Beschreibung |
|---|---|
| **Max. Impuls** | Maximale Dauer eines Korrekturimpulses (ms) |
| **Min. Impuls** | Minimale Dauer eines Korrekturimpulses (ms) |
| **Aggressivität RA** | Proportionaler Anteil der berechneten RA-Korrektur, der angewendet wird |
| **Aggressivität DEC** | Proportionaler Anteil der berechneten DEC-Korrektur, der angewendet wird |
| **Integralverstärkung RA** | Verstärkung auf das akkumulierte RA-Residuum — hebt die dauerhafte Abweichung bei einer konstanten Drift auf. 0 = rein proportional |
| **Integralverstärkung DEC** | Dasselbe für die DEC-Achse |
| **Integralgrenze (px)** | Anti-Windup: Amplitude, auf die der Integralakkumulator begrenzt wird. 0 zum Deaktivieren der Grenze |
| **RMS-Schwellenwert** | Multiplikator des aktuellen RMS, über dem eine Korrektur verworfen wird |
| **RMS berechnet über x** | Anzahl der Bilder, über die das RMS berechnet wird |
| **Umkehr je nach Meridianseite** | RA/DEC-Korrekturen automatisch umkehren, wenn die Meridianseite von der bei der Kalibrierung abweicht |
| **Dither-Amplitude (px)** | Maximale zufällige Verschiebung bei einem Dither |
| **Max. aufeinanderfolgende verlorene Bilder** | Ein Guiding-Bild bei Korrelationsverlust überspringen; erst nach dieser Anzahl in Folge abbrechen |
| **Hardware-Zeitlimit (s)** | Abbrechen, wenn Kamera oder Montierung für diese Dauer nicht mehr antworten. 0 zum Deaktivieren des Watchdogs |

### Sternabgleich

| Parameter | Beschreibung |
|---|---|
| **Abgleichfenster (px)** | Suchradius beim Abgleich eines Referenzsterns im aktuellen Bild |
| **Min. abgeglichene Sterne** | Mindestanzahl an Referenzsternen, die wiedergefunden werden müssen, damit die Drift als zuverlässig gilt |
| **Max. verwendete Sterne** | Obergrenze der Anzahl der hellsten Sterne, die in jedem Bild berücksichtigt werden |

### DEC-Spielkompensation

| Parameter | Beschreibung |
|---|---|
| **Aktivieren** | Fügt bei jeder Richtungsumkehr des DEC-Guidings einen zusätzlichen adaptiven Impuls hinzu |
| **Aktuelle Kompensation (ms)** | Adaptive Schätzung, nach jeder Umkehr angepasst — einen Startwert festlegen oder bei 0 lassen, damit sie erlernt wird |
| **Minimale / maximale Kompensation (ms)** | Grenzen der adaptiven Schätzung |
| **Anpassungsschritt (ms)** | Um wie viel die Schätzung nach jeder Umkehr angepasst wird, je nach gemessener Über-/Unterkorrektur |

### Manuelle Umkehrungen

| Parameter | Beschreibung |
|---|---|
| **RA umkehren** | Manuelles Umkehren der RA-Korrekturrichtung |
| **DEC umkehren** | Manuelles Umkehren der DEC-Korrekturrichtung |

### Deaktivierung von Korrekturen

| Parameter | Beschreibung |
|---|---|
| **RA+ / RA− / DEC+ / DEC− deaktivieren** | Korrekturimpulse in dieser Richtung deaktivieren |

### Testhilfen

| Parameter | Beschreibung |
|---|---|
| **SIM: simulierter Meridianschwenk** | Fügt beim Start des Guidings 180° zum Kalibrierwinkel hinzu (der Effekt, den ein echter Meridianschwenk auf die Zuordnung Drift → RA/DEC hat), um die Behandlung von Korrekturumkehrungen mit dem CCD-Simulator zu testen, der sein eigenes Feld niemals dreht. Im realen Einsatz deaktiviert lassen |

## Kalibrierwerte

| Wert | Beschreibung |
|---|---|
| **Kalibrierqualität** | Anzeige: grün (saubere) / orange (nutzbar, merkliche Hintergrunddrift) / rot (verworfen) |
| **RA-Rate (ms/px)** | Für das Guiding verwendete äquatoriale RA-Rate (West/Ost kombiniert, Drift entfernt) |
| **DEC-Rate (ms/px)** | Für das Guiding verwendete DEC-Rate (Nord/Süd kombiniert) |
| **roh N / S / O / W (ms/px)** | Rohe Raten pro Richtung, nur zur Diagnose (enthalten noch die Hintergrunddrift) |
| **CCD-Ausrichtung** | Kalibrierwinkel (Grad) |
| **Montierungs-DEC bei Kalibrierung** | Deklination der Montierung zum Zeitpunkt der Kalibrierung |
| **RA / DEC umkehren** | Bei der Kalibrierung gespeicherte Korrekturumkehrungen |

## Live-Werte

| Wert | Beschreibung |
|---|---|
| **Impuls N / S / O / W** | Dauer des letzten Impulses in jeder Richtung (ms) |
| **RMS RA / DEC / gesamt** | Quadratisches Mittel der Drift (Pixel) |

## Graphen

### Drift (Streudiagramm)

XY-Streudiagramm der Drift jeder Korrektur relativ zur Referenzposition, mit Toleranzkreisen.

### Guiding (Zeitreihe)

Graph im PHD2-Stil, der pro Korrektur zeigt:

| Kurve | Beschreibung |
|---|---|
| RA-Drift / DEC-Drift | Auf jeder Achse gemessene Drift (Pixel) |
| RA-Impuls / DEC-Impuls | Dauer des Korrekturimpulses (ms) |
| SNR | Signal-Rausch-Verhältnis der Sternerkennung |
| RMS | Aktuelles Gesamt-RMS (Pixel) |

## Aktionen

| Aktion | Beschreibung |
|---|---|
| **Kalibrieren** | Nur Initialisierung und Kalibrierung |
| **Guiden** | Guiding starten — verwendet die gespeicherte Kalibrierung oder kalibriert zunächst, falls keine vorhanden ist |
| **Abbrechen** | Die laufende Sequenz sofort abbrechen |
| **Kalibrierung zurücksetzen** | Gespeicherte Kalibrierdaten löschen |
| **Dither** | Die Guiding-Position bis zur *Dither-Amplitude* zufällig verschieben |
