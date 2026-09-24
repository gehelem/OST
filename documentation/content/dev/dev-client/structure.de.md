---
title: Datenmodell
weight: 20
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

Der Server kann im lokalen Netzwerk über mDNS/Zeroconf gefunden werden: er meldet sich als Diensttyp `_ostserver_ws._tcp` in der Domäne `local`, mit einem TXT-Eintrag `OstServer=Observatoire Sans Tete`. Der angekündigte Port ist immer `9624` (der WebSocket-Port ist nicht konfigurierbar). Die Erkennung ist optional — ein Client, der den Host bereits kennt, kann sich auch direkt verbinden.

Bei der Verbindung eine `DU`-Anfrage senden (siehe [Client → Server]({{< relref "client-messages.de.md" >}})).  
Der Server antwortet mit einer vollständigen Dump-Nachricht (`d`), die alle Module, die Controller-Daten und die Dateilisten enthält.

Controller-spezifische Daten (Serverlast, Modulkonfigurationen, INDI-Treiber, Dateisystem) sind nicht Teil des auf dieser Seite beschriebenen Modul-Modells — siehe [Controller, GlobalDatastore und Dateien]({{% relref "controller.de.md" %}}).

---

## Module

Ein Modul repräsentiert ein funktionales Subsystem (Fokussierung, Guiding, Allsky-Kamera usw.).  
Seine Kennung ist sein interner Name (ohne Leerzeichen).

| Feld | Typ | Beschreibung |
|---|---|---|
| `infos.modulelabel` | string | Angezeigter Name des Moduls |
| `infos.description` | string | Beschreibung des Moduls |
| `infos.template` | string | Modultyp (z. B. `"focus"`, `"guider"`) — kann fehlen |
| `properties` | object | Map von Property-Name → Property-Objekt |
| `globallovs` | object | Map von LOV-Schlüssel → LOV-Objekt (Wertelisten auf Modulebene) |
| `profile.name` | string | Name des aktiven Profils |
| `profile.changed` | bool | Zeigt an, ob das Profil nicht gespeicherte Änderungen hat |

{{% notice style="info" title="Wire-Format" %}}
Im JSON-Format über das Netzwerk wird `properties` zu `p`, `globallovs` zu `l` und `profile` zu `f` abgekürzt. Clients müssen diese abgekürzten Schlüssel verarbeiten können.
{{% /notice %}}

{{% notice style="note" title="Interne Felder" %}}
`infos` enthält außerdem mehrere interne Build-/Versionsfelder (`baseGithash`, `thisversion`, `startdatetime` und andere) — Implementierungsdetail, nicht Teil des stabilen Client-Vertrags, hier nicht einzeln dokumentiert.
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

{{% notice style="tip" title="`level1`/`level2`/`order` sind informativ, kein Vertrag" %}}
Diese Felder sind rein informativ — das Backend erzwingt sie nie, und ein Client kann sie teilweise oder gar nicht befolgen. Sie drücken nur aus, wie sich der Entwickler eines Moduls die Organisation seiner eigenen Properties vorgestellt hat. Genau das ermöglicht generischen Clients (Rooster, ostnoctua), sich an jedes Modul anzupassen, ohne modulspezifische UI.

Der Referenz-Client (Rooster) gruppiert Properties so, für einen Client, der ein ähnliches generisches Layout anstrebt:
1. `level1` leer → Property außerhalb jedes Tabs angezeigt (direkt gerendert, nach `order` sortiert).
2. `level1` gesetzt → in einem Tab namens `level1` gruppiert. Darin: `level2` leer → direkt unter dem Tab angezeigt; `level2` gesetzt → in einer einklappbaren Gruppe namens `level2`.
3. Die Reihenfolge der Tabs (und der Gruppen innerhalb eines Tabs) ist kein eigenes Feld — sie ergibt sich aus dem *kleinsten* `order`-Wert unter den Properties dieser Gruppe.
4. Innerhalb einer Gruppe werden Properties nach ihrem eigenen `order` sortiert.

Zwei Dinge, die man kennen sollte, wenn man etwas Ähnliches baut:
- `level1`/`level2` bilden bewusst eine "unechte Hierarchie", keine garantierte zweistufige Struktur — nichts hindert ein Modul daran, `level1` leer zu lassen und trotzdem `level2` zu setzen (Rooster behandelt diese Kombination nicht einmal gesondert; die Property landet einfach im "kein level1"-Topf, `level2` wird ignoriert).
- `level1`/`level2` sind übersetzte Bezeichnungen, die denselben Mechanismus wie `label`/`hint` durchlaufen. Den rohen String-Wert als UI-Zustandsschlüssel zu verwenden (z. B. für ein auf-/zugeklapptes Panel) ist über Sprachwechsel des Clients hinweg brüchig — kein Bug, nur etwas, das man vermeiden sollte.
{{% /notice %}}

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
| `directedit` | bool | Zeigt an, ob dieses Element allein per `SV` geschrieben werden kann — siehe unten |
| `preicon` | string | Icon vor dem Element (optional) |
| `posticon` | string | Icon nach dem Element (optional) |

{{% notice style="warning" title="Teilweise Updates" %}}
In den Update-Nachrichten `ee` und `ea` enthalten Elemente **nur ihren Wert** — ohne die Metadatenfelder. Clients müssen die Metadaten des letzten vollständigen Dumps behalten.
{{% /notice %}}

{{% notice style="tip" title="Feldauslassung" %}}
`directedit` kann bei Elementen, auf die es nicht zutrifft, ganz fehlen — ein fehlendes `directedit` als `false` behandeln. Das hält die Dumps von Properties, die es nicht brauchen, schlanker.
{{% /notice %}}

{{% notice style="note" title="Bedeutung von `directedit`" %}}
`true` bedeutet, dieses Element kann für sich allein aktualisiert werden, unabhängig von den anderen Elementen der Property — dafür `SV` senden (`SA` funktioniert auch). `false` bedeutet, der Client muss **alle** Elemente der Property zusammen übermitteln — dafür immer `SA` verwenden, nie `SV` für dieses einzelne Element. Ein anschauliches Beispiel: RA/Dec-Koordinaten, bei denen beide Werte zusammen angegeben werden müssen.

Das Backend erzwingt dies: Ein `SV` auf ein Element mit `directedit: false` wird mit einem Fehler im Modul-Log abgelehnt, und nichts wird angewendet. `SA` wird immer akzeptiert.

Den Wert nach dem Verhalten des Elements wählen, nicht danach, ob es sich mit seinen Nachbarn „zusammengehörig anfühlt“:
- **`true` für ein unabhängiges Element** — ein `bool`, das unabhängig von seinen Geschwistern wahr sein kann (Aktions-Button wie start/abort/loop, Schalter), muss `directedit: true` haben. Ein solcher Wert spiegelt oft eine laufende Aktivität wider, und mehrere können gleichzeitig wahr sein (z. B. eine laufende Aufnahmeschleife, während ein Focus-out angefordert wird). `false` zu setzen würde fälschlich behaupten, die Elemente müssten als Gruppe übermittelt werden, und ein Client, der dieser Seite folgt, würde `SA` senden und Module umgehen, die nur `SV` verarbeiten.
- **`false` nur für eine echte Gruppe** — Elemente, die nur zusammen sinnvoll sind: RA/Dec-Koordinaten oder ein 1:1 gespiegelter INDI-Switch-Vektor.

Bei einer `false`-Gruppe aus `bool`-Elementen mit exklusiver Regel (`OneOfMany` oder `AtMostOne`) bedeutet die Auswahl eines Elements, ihm `true` und **allen Geschwistern `false`** im selben `SA` zu senden — niemals die aktuellen Werte der Geschwister erneut senden, sonst sind zwei Elemente `true`.
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
| `nullable` | bool | Zeigt an, ob das Element leer bleiben darf (Frontend sollte eine "keine"-Option anbieten) |

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
| `lovConstrained` | bool | Zeigt an, ob der Wert zur LOV gehören muss |
| `nullable` | bool | Zeigt an, ob das Element leer bleiben darf (Frontend sollte eine "keine"-Option anbieten) |

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
| `min` / `max` / `mean` / `median` / `stddev` | float[] | Statistiken pro Kanal (siehe unten) |
| `histogram` | array | Histogrammdaten pro Kanal (siehe unten) |
| `alternates` | string[] | Optional: alternative Versionen des Bildes |

**Statistiken pro Kanal**: `min`/`max`/`mean`/`median`/`stddev` sind
Arrays mit **3 festen Plätzen**, unabhängig von der tatsächlichen
Kanalzahl — Index `0` = einzelner Kanal (Mono-Bild) oder Rot (R),
`1` = Grün (G), `2` = Blau (B). Nur die ersten `channels` Plätze sind
aussagekräftig; darüber hinaus sind die Werte null und bedeutungslos
(ein Mono-Bild hat nur Index `0` befüllt).

{{% notice style="note" title="Histogramm: Bin-Struktur" %}}
`histogram` folgt derselben Kanal-Logik (ein Array pro Kanal, nur die
ersten `channels` verwendet), hat aber zwei Besonderheiten, die man
vor der Nutzung kennen sollte:

- **Jeder Bin ist selbst in ein Array mit einem Element verpackt**:
  `histogram[Kanal][Bin]` ist `[Häufigkeit]`, nicht direkt
  `Häufigkeit` — ein Artefakt der aktuellen Backend-Serialisierung,
  kein Lesefehler auf Client-Seite.
- **Der Pixelwert, den jeder Bin repräsentiert, wird nicht direkt
  übertragen**, nur die Häufigkeit. Die Anzahl der Bins variiert je
  Bild (`histogram[Kanal].length`); der Pixelwert von Bin `i` wird aus
  `min`/`max` desselben Kanals neu berechnet:
  ```
  bin_breite = (max[Kanal] - min[Kanal]) / (bin_anzahl - 1)
  wert(i)    = min[Kanal] + i × bin_breite
  ```
{{% /notice %}}

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
