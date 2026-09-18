---
title: IndiPanel
weight: 80
description: Rohes Kontrollfeld für alle verbundenen INDI-Geräte
---

## Aufgabe

Das IndiPanel-Modul ist ein transparentes Kontrollfeld, das Zugriff auf alle INDI-Eigenschaften aller mit dem INDI-Server verbundenen Geräte gibt. Im Gegensatz zu den anderen OST-Modulen erfüllt es keine spezifische astrofotografische Funktion: Es legt das gesamte INDI-Protokoll direkt in der OST-Oberfläche offen.

![Screenshot des IndiPanel-Moduls](/images/modules/indipanel.png)

{{% notice style="note" title="Modul ohne Konfiguration" %}}
IndiPanel hat keine Konfigurationsdatei und keine festzulegenden Parameter. Sein Inhalt wird vollständig zur Laufzeit aus den auf dem INDI-Server erkannten Geräten und Eigenschaften erzeugt.
{{% /notice %}}

## Funktionsweise

Beim Start verbindet sich das Modul mit dem INDI-Server und hört auf alle Ereignisse zur Erkennung von Geräten und Eigenschaften. Für jede empfangene INDI-Eigenschaft wird automatisch ein entsprechender Eintrag in der OST-Oberfläche erzeugt:

| INDI-Typ | OST-Darstellung |
|---|---|
| Number | Numerischer Wert |
| Switch | Ein/Aus-Schalter oder Schaltergruppe (OneOfMany, AnyOfMany) |
| Text | Textfeld |
| Light | Statusanzeige |
| BLOB | Bild (JPEG + FITS) |

Die Eigenschaften sind nach INDI-Gerät und nach Gruppe organisiert, genau wie in jedem gängigen INDI-Client.

## Bidirektionale Interaktion

- **INDI → OST**: jede Aktualisierung einer INDI-Eigenschaft (Wert, Status) wirkt sich sofort auf die Oberfläche aus
- **OST → INDI**: jede vom Benutzer vorgenommene Änderung wird an den entsprechenden INDI-Treiber zurückgesendet (`sendNewSwitch`, `sendNewNumber`, `sendNewText`)

Bilder (BLOBs) werden automatisch angezeigt, sobald ein Treiber ein Bild sendet — auch von Kameras, die in keinem anderen Modul konfiguriert sind.

## Anwendungsfälle

- Direkter Zugriff auf erweiterte Eigenschaften eines INDI-Treibers, die von anderen Modulen nicht offengelegt werden
- Debugging eines Geräts oder eines Treibers
- Steuerung eines exotischen Geräts ohne dediziertes OST-Modul
- Einsicht der von INDI-Treibern gesendeten Statusmeldungen

## Geräte

Es ist kein Gerät zu konfigurieren. Das Modul verbindet sich automatisch mit dem INDI-Server und erkennt alle vorhandenen Geräte.
