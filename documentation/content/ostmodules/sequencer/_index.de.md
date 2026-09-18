---
title: Sequencer
weight: 30
description: Automatische Aufnahme von Bildsequenzen
---

## Aufgabe

Das Sequencer-Modul orchestriert die automatische Aufnahme während einer astrofotografischen Sitzung. Es arbeitet eine Liste von Sequenzzeilen ab, die jeweils den Filter, die Belichtungsdauer und die Anzahl der aufzunehmenden Bilder definieren. Trame-Typ, Gain und Offset werden nun für die gesamte Sequenz gemeinsam festgelegt, statt zeilenweise. Die FITS-Dateien werden automatisch in einer nach Objekt und Filter organisierten Verzeichnisstruktur gespeichert, und das Modul kann bei Bedarf die Kühlung der Kamera, die automatische Fokussierung und das Guiding rund um jede Zeile anfordern.

![Screenshot des Sequencer-Moduls](/images/modules/sequencer.png)

## Benötigte Geräte

| Gerät | Rolle |
|---|---|
| Kamera | Bildaufnahme und Kühlung, falls aktiviert |
| Filterrad | Optional — die Sequenz funktioniert auch ohne; die Positionierung und die Fokusprofile pro Filter werden dann einfach ignoriert |

## Struktur einer Sequenz

Eine Sequenz besteht aus mehreren Zeilen. Jede Zeile definiert:

| Spalte | Beschreibung |
|---|---|
| **Filter** | Auf dem Filterrad ausgewählter Filter |
| **Belichtung** | Belichtungsdauer in Sekunden |
| **Anzahl** | Anzahl der für diese Zeile aufzunehmenden Bilder |

Die Zeilen werden in der Reihenfolge von der ersten bis zur letzten ausgeführt. Trame-Typ, Gain und Offset gelten für alle Zeilen (siehe *Sequenzparameter* unten).

## Parameter

Die Einstellungen sind in drei Gruppen aufgeteilt, um Sequenz-, Guiding- und Fokussierverhalten nicht zu vermischen.

### Objekt

| Parameter | Beschreibung |
|---|---|
| **Name** | Name des abgebildeten Objekts — wird zur Benennung des Speicherordners verwendet |
| **AD** | Rektaszension des Objekts |
| **DEC** | Deklination des Objekts |

### Slave-Module

Der Sequencer interagiert mit anderen Modulen für Fokussierung und Guiding.

| Parameter | Beschreibung |
|---|---|
| **Instanz des Fokussierungsmoduls** | Name der zu verwendenden Focuser-Modulinstanz (z. B. `focus`) |
| **Instanz des Guiding-Moduls** | Name der zu verwendenden Guider-Modulinstanz (z. B. `guider`) |

### Sequenzparameter

| Parameter | Beschreibung |
|---|---|
| **Kamerakühlung aktivieren** | Fordert die Kühlung auf die Zieltemperatur vor dem Start der Sequenz an und wartet, bis sie erreicht ist |
| **Zieltemperatur (°C)** | Zieltemperatur des Sensors |
| **Trame-Typ** | Light (L), Bias (B), Dark (D) oder Flat (F) — gilt für alle Zeilen |
| **Gain** | Gain der Kamera — gilt für alle Zeilen |
| **Offset** | Offset der Kamera — gilt für alle Zeilen |

### Guiding-Parameter

| Parameter | Beschreibung |
|---|---|
| **Guiding verwenden** | Aktiviert die Integration mit dem Guider (Dithering, Stabilisierung, RMS-Überwachung, Pausieren während der Fokussierung). Der Guider muss bereits guiden oder wird bei Bedarf vom Sequencer gestartet. |
| **Stabilisierungszeit nach Wiederaufnahme des Guidings (s)** | Wartezeit in Sekunden nach der Wiederaufnahme des Guidings, bevor die Aufnahmen fortgesetzt werden |
| **Dithering alle N Belichtungen (0=deaktiviert)** | Fordert alle N aufgenommenen Bilder ein Dithering an |
| **RMS-Rekalibrierungsschwelle (0=deaktiviert)** | Bricht die laufende Belichtung ab und löst eine Rekalibrierung des Guiders aus, wenn das maximale RMS während dieser Belichtung diesen Wert überschreitet |

### Fokussierparameter

| Parameter | Beschreibung |
|---|---|
| **Auto-Fokus bei Filterwechsel** | Löst bei jedem Filterwechsel eine Fokussierung aus — dies gilt auch für die allererste Zeile der Sequenz, da in dieser Sitzung des Moduls noch keine Fokussierung stattgefunden hat. Ein erneuter Start derselben Sequenz vermeidet eine überflüssige Fokussierung, wenn die letzte durchgeführte bereits dem aktuellen Filter entspricht. |
| **Guiding während der Fokussierung pausieren** | Pausiert das Guiding vor der Fokussierung und nimmt es danach wieder auf |
| **HFR-Schwelle für Nachfokussierung (0=deaktiviert)** | Löst während einer Zeile eine zusätzliche Fokussierung aus, wenn die gemessene HFR diese Schwelle überschreitet |
| **Zu ladendes Fokusprofil** | Vor der Anforderung des Autofokus auf dem Fokussierungsmodul zu ladendes Profil; leer lassen, um das aktuell geladene Profil zu behalten |

### Fokusprofil pro Filter

Eine eigene Tabelle erlaubt es, das Fokusprofil für bestimmte Filter zu überschreiben — nützlich zum Beispiel, wenn Schmalbandfilter andere Fokuseinstellungen benötigen als LRGB-Filter.

| Spalte | Beschreibung |
|---|---|
| **Filter** | Filter, für den diese Zeile gilt |
| **Fokusprofil** | Beim Fokussieren auf diesem Filter zu ladendes Profil |

Besitzt der aktuelle Filter eine Zeile in dieser Tabelle, wird dessen Profil verwendet. Andernfalls greift der Sequencer auf das oben genannte *Zu ladende Fokusprofil* zurück. Ist auch dieses leer, behält das Fokussierungsmodul das aktuell geladene Profil.

## Algorithmus

### Start der Sequenz

1. Verbindung zur Kamera und Zurücksetzen des Frames
2. Ist *Kamerakühlung aktivieren* aktiv, wird die Zieltemperatur angefordert und abgewartet, bis sie erreicht ist (sofort ignoriert, wenn deaktiviert oder die Kamera nicht konfiguriert ist)
3. Start der ersten Zeile

### Ausführung einer Zeile

Für jede Zeile der Sequenz:

1. Auswahl des Filters auf dem Filterrad (ignoriert, wenn kein Filterrad konfiguriert ist)
2. Ist *Auto-Fokus bei Filterwechsel* aktiviert und unterscheidet sich der Filter von dem bei der letzten Fokussierung dieser Sitzung verwendeten: Auslösen der automatischen Fokussierung (zunächst wird ein Profil geladen — Überschreibung pro Filter, dann allgemeiner Rückgriff —, und das Guiding wird bei Bedarf pausiert/fortgesetzt)
3. Start oder Bestätigung des Guidings, wenn *Guiding verwenden* aktiviert ist
4. Dithering, wenn das konfigurierte Belichtungsintervall erreicht ist
5. Aufnahme der festgelegten Anzahl an Bildern, eines nach dem anderen, mit Nachfokussierung während der Zeile, falls die HFR-Schwelle überschritten wird

### Speicherung der Dateien

Die FITS-Dateien werden in folgender Verzeichnisstruktur gespeichert (die Segmente Objekt und Filter werden ausgelassen, wenn sie nicht definiert sind, statt überflüssige Trennzeichen zu belassen):

```
<objet>/
  LIGHT/
    <filtre>/   ← Light-Frames
  FLAT/
    <filtre>/   ← Flat-Frames
  BIAS/         ← Bias-Frames
  DARK/         ← Dark-Frames
```

Jeder Dateiname enthält den Namen des Objekts, den Trame-Typ, den Filter und einen Zeitstempel.

### Integration mit dem Fokussierungsmodul

Wenn eine automatische Fokussierung angefordert wird:

1. Gilt ein Profil (Überschreibung pro Filter oder allgemeiner Rückgriff), fordert der Sequencer das Focuser-Modul auf, es zu laden
2. Der Sequencer sendet die Aktion `autofocus` an das bezeichnete Focuser-Modul
3. Er pausiert die Aufnahmen und wartet auf das Ende der Fokussierung
4. Ist *Guiding pausieren* aktiviert, sendet er vor der Fokussierung `abortguider` an das Guider-Modul und danach `guide`
5. Ist eine Stabilisierungszeit konfiguriert, wartet er diese Zeit ab, bevor er fortsetzt

### Integration mit dem Guiding-Modul

- Das Guiding wird gestartet (oder seine Anwesenheit bestätigt), bevor die Belichtungen beginnen, wenn *Guiding verwenden* aktiviert ist
- Alle N Belichtungen wird ein Dithering angefordert, danach wartet der Sequencer die konfigurierte Stabilisierungszeit ab
- Überschreitet das maximale RMS während einer Belichtung den konfigurierten Schwellenwert, wird die Belichtung abgebrochen und der Guider vor der Fortsetzung rekalibriert

## Fortschritt

Der Fortschritt wird sowohl live (Aktualisierung während der Aufnahme) als auch als kumulierte Durchschnittswerte/Dauern seit dem Laden des Moduls verfolgt.

| Indikator | Beschreibung |
|---|---|
| **Sequenz (Belichtungen)** | Anzahl abgeschlossener Belichtungen im Verhältnis zur Gesamtzahl der Sequenz |
| **Sequenz (Zeit)** | Verstrichene Belichtungszeit im Verhältnis zur geschätzten Gesamtbelichtungszeit, live aktualisiert während der laufenden Belichtung |
| **Laufende Belichtung** | Fortschritt der laufenden Belichtung (0 bis 100 %) |
| **Theoretische Gesamtdauer** | Summe aus Anzahl × Belichtung über alle Zeilen, bei jeder Änderung der Tabelle neu berechnet — auch vor dem Start der Sequenz |
| **Theoretische Restdauer** | Theoretische Gesamtdauer abzüglich der bereits verstrichenen Zeit |
| **Tatsächlich verstrichene Dauer** | Echte Stoppuhr, bei jedem Start der Sequenz zurückgesetzt |
| **Durchschnittliche Fokussierdauer** | Gleitender Durchschnitt der Wartezeit auf den Autofokus, seit dem Laden des Moduls |
| **Durchschnittliche Guiding-Startdauer** | Gleitender Durchschnitt der Wartezeit auf den Start des Guiders, seit dem Laden des Moduls |

## Aktionen

| Aktion | Beschreibung |
|---|---|
| **Sequenz starten** | Startet die Ausführung der Sequenz ab der ersten Zeile |
| **Unterbrechen** | Bricht die laufende Sequenz sofort ab und leitet auch eine Unterbrechungsanforderung an die in *Slave-Module* konfigurierten Guider- und Focuser-Instanzen weiter, um auch das Guiding und eine eventuell laufende Fokussierung zu stoppen |
