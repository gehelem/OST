---
title: Datenmodell
weight: 10
description: "Hierarchie der Module, Properties und Elemente, die über WebSocket ausgetauscht werden"
---

## Überblick

Alle Daten sind in einer dreistufigen Hierarchie organisiert:

```
Module
  └─ Property (eine oder mehrere pro Modul)
       └─ Element (eines oder mehrere pro Property)
```

---

## Verbindung und Anfangszustand

Bei der Verbindung eine `DU`-Anfrage senden (siehe [Client → Server]({{< relref "client-messages.de.md" >}})).  
Der Server antwortet mit einer vollständigen Dump-Nachricht (`d`), die alle Module, die Controller-Daten und die Dateilisten enthält.

---

## Module

Ein Modul repräsentiert ein funktionales Subsystem (Fokussierung, Guiding, Allsky-Kamera usw.).  
Seine Kennung ist sein interner Name (ohne Leerzeichen).

| Feld | Typ | Beschreibung |
|---|---|---|
| `infos.label` | string | Angezeigter Name des Moduls |
| `infos.name` | string | Interner Name des Moduls |
| `infos.description` | string | Beschreibung des Moduls |
| `infos.template` | string | Modultyp (z. B. `"focus"`, `"guider"`) |
| `properties` | object | Map von Property-Name → Property-Objekt |
| `globallovs` | object | Map von LOV-Schlüssel → LOV-Objekt (Wertelisten auf Modulebene) |
| `profile.name` | string | Name des aktiven Profils |
| `profile.changed` | bool | Zeigt an, ob das Profil nicht gespeicherte Änderungen hat |

{{% notice style="info" title="Wire-Format" %}}
Im JSON-Format über das Netzwerk wird `properties` zu `p`, `globallovs` zu `l` und `profile` zu `f` abgekürzt. Clients müssen diese abgekürzten Schlüssel verarbeiten können.
{{% /notice %}}

---

## Properties

Eine Property gruppiert zusammengehörige Elemente. Properties werden hierarchisch über `level1` und `level2` angezeigt.

| Feld | Typ | Beschreibung |
|---|---|---|
| `label` | string | Angezeigte Bezeichnung |
| `order` | string | Sortierreihenfolge innerhalb derselben Ebene |
| `level1` | string | Erste Hierarchieebene (z. B. Tab-Name) |
| `level2` | string | Zweite Hierarchieebene (z. B. Gruppenname) |
| `status` | int | `0` Ruhe · `1` OK (grün) · `2` Beschäftigt (gelb) · `3` Fehler (rot) |
| `permission` | int | `0` Nur Lesen · `1` Nur Schreiben · `2` Lesen-Schreiben |
| `enabled` | bool | Zeigt an, ob mit der Property interagiert werden kann |
| `badge` | bool | Zeigt an, ob die Property in der Favoritenliste steht |
| `preicon1` | string | Icon vor der Bezeichnung (Name eines Google-Fonts-Icons) |
| `preicon2` | string | Zweites Icon vor der Bezeichnung |
| `posticon1` | string | Icon nach der Bezeichnung |
| `posticon2` | string | Zweites Icon nach der Bezeichnung |
| `showElts` | bool | Zeigt an, ob die Elementwerte inline angezeigt werden sollen |
| `hasprofile` | bool | Zeigt an, ob die Property in Profilen gespeichert wird |
| `freevalue` | string | Beliebiges Freitextfeld |
| `rule` | int | Gruppierungsregel für bool-Elemente: `0` GenauEines (Radio) · `1` HöchstensEines · `2` Beliebig |
| `elements` | object | Map von Elementname → Element-Objekt |

**Properties mit Grid** (wenn `hasGrid: true`):

| Feld | Typ | Beschreibung |
|---|---|---|
| `hasGrid` | bool | Die Property enthält tabellarische Daten |
| `showGrid` | bool | Die Grid soll standardmäßig angezeigt werden |
| `gridLimit` | int | Maximale Anzahl an Zeilen in der Grid |
| `gridheaders` | string[] | Spaltenreihenfolge (Elementnamen) |
| `grid` | Array von Arrays | Zeilen der Grid; jede Zeile ist ein Array von Werten in der Reihenfolge von `gridheaders` |

**Properties mit Graph** (wenn `hasGraph: true`):

| Feld | Typ | Beschreibung |
|---|---|---|
| `hasGraph` | bool | Die Property enthält einen Graph |
| `graphType` | string | `"XY"` · `"DY"` (Zeitachse) · `"PHD"` (Guiding) |
| `graphParams` | object | Konfiguration des Graphen |

{{% notice style="info" title="Wire-Format" %}}
Im JSON-Format über das Netzwerk wird `elements` zu `e` abgekürzt.
{{% /notice %}}

---

## Elemente

Ein Element enthält einen einzigen typisierten Wert. Es gibt 11 Elementtypen.

### Gemeinsame Felder (alle Elemente, im vollständigen Dump)

| Feld | Typ | Beschreibung |
|---|---|---|
| `type` | string | Typ des Elements (siehe Liste unten) |
| `label` | string | Angezeigte Bezeichnung |
| `order` | string | Sortierreihenfolge innerhalb der Property |
| `hint` | string | Hilfetext / Tooltip |
| `autoupdate` | bool | Interner Backend-Indikator; von Clients ignoriert |
| `badge` | bool | Zeigt an, ob das Element in der Favoritenliste steht |
| `directedit` | bool | `true` = bei jeder Änderung `SV` senden; `false` = mit `SA` gruppieren |
| `preicon` | string | Icon vor dem Element (optional) |
| `posticon` | string | Icon nach dem Element (optional) |

{{% notice style="warning" title="Teilweise Updates" %}}
In den Update-Nachrichten `ee` und `ea` enthalten Elemente **nur ihren Wert** — ohne die Metadatenfelder. Clients müssen die Metadaten des letzten vollständigen Dumps behalten.
{{% /notice %}}

---

### Typ: `int`

| Feld | Typ | Beschreibung |
|---|---|---|
| `value` | int | Aktueller Ganzzahlwert |
| `min` | int | Minimal erlaubter Wert |
| `max` | int | Maximal erlaubter Wert |
| `step` | int | Schrittweite |
| `format` | string | Anzeige-Formatierungsstring |
| `slider` | int | `0` kein Slider · `1` nur Slider · `2` Slider + Werteingabe |
| `listOfValues` | object | Optional: Map `{"Schlüssel": "Bezeichnung"}` der erlaubten Werte |
| `globallov` | string | Optional: Schlüssel, der auf eine Modul- oder Controller-LOV verweist |
| `lovScope` | string | `"module"` oder `"controller"` — wo die globallov definiert ist |
| `lovConstrained` | bool | Zeigt an, ob der Wert zur LOV gehören muss |

### Typ: `float`

Gleiche Felder wie `int`, mit `value`, `min`, `max`, `step` als Gleitkommazahl.

### Typ: `bool`

| Feld | Typ | Beschreibung |
|---|---|---|
| `value` | bool | `true` oder `false` |

### Typ: `string`

| Feld | Typ | Beschreibung |
|---|---|---|
| `value` | string | Textwert |
| `listOfValues` | object | Optional: Map `{"Schlüssel": "Bezeichnung"}` der erlaubten Werte |
| `globallov` | string | Optional: Schlüssel, der auf eine LOV verweist |
| `lovScope` | string | `"module"` oder `"controller"` |

### Typ: `date`

| Feld | Typ | Beschreibung |
|---|---|---|
| `value` | object | `{"year": int, "month": int (1-12), "day": int (1-31)}` |

### Typ: `time`

| Feld | Typ | Beschreibung |
|---|---|---|
| `value` | object | `{"hh": int, "mm": int, "ss": int, "ms": int}` |
| `usems` | bool | Zeigt an, ob Millisekunden angezeigt werden sollen |

### Typ: `datetime`

| Feld | Typ | Beschreibung |
|---|---|---|
| `value` | object | `{"year": int, "month": int, "day": int, "hh": int, "mm": int, "ss": int, "ms": int}` |

### Typ: `img`

Das Feld `value` ist ein Objekt mit den Bild-Metadaten:

| Feld | Typ | Beschreibung |
|---|---|---|
| `urljpeg` | string | Relativer Pfad zum JPEG-Bild (mit der Basis-URL `/ostmedia/` zu verwenden) |
| `urlfits` | string | Relativer Pfad zur FITS-Datei |
| `urlthumbnail` | string | Relativer Pfad zum Thumbnail |
| `urloverlay` | string | Relativer Pfad zum Overlay-Bild |
| `channels` | int | Anzahl der Farbkanäle |
| `width` / `height` | int | Bildabmessungen in Pixeln |
| `snr` | float | Signal-Rausch-Verhältnis |
| `hfravg` | float | Durchschnittlicher HFR (Half-Flux-Radius) der erkannten Sterne |
| `stars` | int | Anzahl der erkannten Sterne |
| `issolved` | bool | Zeigt an, ob die Plate-Solving-Auflösung erfolgreich war |
| `solverra` / `solverde` | float | Aufgelöste RA/Dec-Koordinaten |
| `solverorientation` | float | Rotationswinkel des aufgelösten Feldes |
| `min` / `max` / `mean` / `median` / `stddev` | float[] | Statistiken pro Kanal |
| `histogram` | array | Histogrammdaten pro Kanal |
| `alternates` | string[] | Optional: alternative Versionen des Bildes |

{{% notice style="tip" title="Media-URLs" %}}
Alle Bild-/Video-Pfade sind relativ. Mit `http(s)://hostname/ostmedia/` voranstellen, um die vollständige URL zu erhalten.  
Beispiel: `"urljpeg": "Focus/frame.jpeg"` → `http://hostname/ostmedia/Focus/frame.jpeg`
{{% /notice %}}

{{% notice style="warning" title="Medien werden getrennt vom WebSocket-Port bereitgestellt" %}}
Dieser `hostname` ist **nicht aus dem Protokoll ableitbar** — kein Feld im Dump liefert ihn, und es ist **nicht** der Host der gerade aufgebauten WebSocket-Verbindung. In der Referenz-Installation (siehe [Neuinstallation]({{% relref "fresh.de.md" %}})) werden Medien von einem separaten Webserver bereitgestellt (nginx, standardmäßig Port 80, 443 hinter SSL) — ein völlig anderer Port als der WebSocket-Server (9624). Ein Client muss den Hostname des OST-Servers bereits kennen, oder ihn vom Nutzer konfigurieren lassen, unabhängig von der WebSocket-URL, und Medien-URLs gegen den HTTP-Port dieses Hostnamens aufbauen — nicht gegen den eigenen Ursprung der Seite und nicht gegen den WebSocket-Port. Ein Client, der einfach einen nackten `/ostmedia/`-Pfad voranstellt (aufgelöst relativ zu seinem eigenen Serving-Ursprung), bekommt kaputte Bilder.
{{% /notice %}}

### Typ: `video`

| Feld | Typ | Beschreibung |
|---|---|---|
| `value` | object | `{"url": "relativer/pfad/zum/video.mp4"}` |

In Nachrichten mit teilweisem Update (`ea`) kann das Video-Element `url` direkt auf Elementebene enthalten, statt in `value` verschachtelt.

### Typ: `light`

| Feld | Typ | Beschreibung |
|---|---|---|
| `value` | int | `0` Ruhe · `1` OK · `2` Warnung · `3` Fehler |

### Typ: `prg`

| Feld | Typ | Beschreibung |
|---|---|---|
| `value` | object | `{"value": float (0–100), "dynlabel": string}` |
| `prgtype` | string | `"bar"` oder `"spinner"` |

### Typ: `message` (im aktuellen Backend noch nicht implementiert)

Reserviert für die Anzeige von Nachrichten/Logs.

---

## Wertelisten (LOV)

LOVs stellen eine Menge erlaubter Werte für Elemente vom Typ `int`, `float` und `string` bereit.

**Lokale LOV** — direkt im Element eingebettet:
```json
"listOfValues": {
  "1": "Option Un",
  "2": "Option Deux"
}
```

**Globale LOV** — per Schlüssel referenziert:
```json
"globallov": "myCameraModels",
"lovScope": "module"
```

Die LOV selbst wird auf Modulebene (`globallovs`) oder Controller-Ebene (`controllerlovs`) definiert:
```json
"myCameraModels": {
  "label": "Modèles de caméra",
  "type": "string",
  "values": {
    "ASI294MC": "ZWO ASI 294 MC",
    "ASI533MC": "ZWO ASI 533 MC"
  }
}
```

**Eingebaute Controller-LOVs** (immer verfügbar):

| Schlüssel | Inhalt |
|---|---|
| `loadedModules` | Alle geladenen Module: `{Modulname: Bezeichnung}` |
| `loadedModules-<template>` | Module gefiltert nach Typ (z. B. `loadedModules-focus`) |
| `profiles-<template>` | Verfügbare Profile für einen Modultyp |

---

## Zugriffskontrolle

Die Dump-Antwort enthält die Werte `grant-server` und `grant-client`:

| Wert | Bedeutung |
|---|---|
| `"1"` | Vollständiger Lese-Schreib-Zugriff |
| `"0"` | Nur Lesen |
| `"-1"` | Zugriff verweigert (Authentifizierung erforderlich) |

Ist `grant-client` gleich `"-1"`, werden keine Moduldaten gesendet. Die `LO`-Nachricht (Login) zur Authentifizierung verwenden.
