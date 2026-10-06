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

### Den Server finden

Ein **Web**-Client wird *vom* OST-Server selbst ausgeliefert und kennt daher bereits seinen Host — die WebSocket-URL sollte von der URL der Seite selbst abgeleitet werden, nicht vom Benutzer erfragt:

| Seiten-URL | WebSocket-URL |
|---|---|
| `http://<host>/` | `ws://<host>:9624` (direkt, fester Port) |
| `https://<host>/` | `wss://<host>/ws/` (impliziter Port 443, über einen Reverse-Proxy-Pfad — SSL-Setups legen Port 9624 nicht direkt offen) |

Das ist eine Konvention, keine vom Protokoll erzwungene Regel — aber jeder existierende Client folgt ihr, und ein Client, der stattdessen den Benutzer bittet, eine WebSocket-URL manuell einzugeben, löst ein Problem, das im Web-Fall gar nicht existiert.

Ein **Nicht-Web**-Client (Mobile, Desktop) hat keine Seiten-URL, von der er etwas ableiten könnte, und braucht den Host auf andere Weise:
- **mDNS/Zeroconf**: der Server meldet sich im lokalen Netzwerk als Diensttyp `_ostserver_ws._tcp` in der Domäne `local`, mit einem TXT-Eintrag `OstServer=Observatoire Sans Tete`. Der angekündigte Port ist immer `9624`. Das erlaubt einem Client, **verfügbare Server im Netzwerk zu scannen**, statt vom Benutzer eine IP-Adresse zu verlangen — nützlich für eine erste Verbindung, oder wann immer mehr als ein OST-Server in der Nähe sein könnte.
- **Manuelle Eingabe, gemerkt**: ob entdeckt oder eingegeben, ein Client sollte eine kleine Historie bereits genutzter Server (Host/Port/sicher) führen, damit eine spätere Wiederverbindung nicht die Erkennung oder Eingabe wiederholen muss. Das ist ein clientseitiger Komfort, kein Teil des Protokolls.

In jedem Fall folgt die Basis-URL der **Medien** (für Bilder/Videos, siehe die Notiz „Media-URLs" weiter unten) demselben Host, mit dem üblichen HTTP(S)-Port (80/443) statt 9624 — ein Nicht-Web-Client braucht diesen Host ebenfalls, aus genau demselben Grund wie den WebSocket-Host.

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
| `infos.template` | string | Anzeige-Hinweis für das Frontend (welche Darstellung verwendet werden soll), z. B. `"focus"`, `"guider"` — rein kosmetisch, kann fehlen |
| `infos.library` | string | Typ des Moduls, kleingeschrieben (die Bibliothek, aus der es geladen wurde, z. B. `"focus"`) |
| `infos.classname` | string | Typ des Moduls, exakte Schreibweise (z. B. `"Focus"`) |
| `infos.families` | Array von string | Fähigkeiten, die diese Modulinstanz deklariert (z. B. `["guiding"]`) — keine, eine oder mehrere |
| `properties` | object | Map von Property-Name → Property-Objekt |
| `globallovs` | object | Map von LOV-Schlüssel → LOV-Objekt (Wertelisten auf Modulebene) |
| `profile.name` | string | Name des aktiven Profils |
| `profile.changed` | bool | Zeigt an, ob das Profil nicht gespeicherte Änderungen hat |

{{% notice style="info" title="Wire-Format" %}}
Im JSON-Format über das Netzwerk wird `properties` zu `p`, `globallovs` zu `l` und `profile` zu `f` abgekürzt. Clients müssen diese abgekürzten Schlüssel verarbeiten können.
{{% /notice %}}

{{% notice style="warning" title="Typ, Template und Familie sind drei verschiedene Dinge" %}}
Es ist verlockend, `infos.template` als Identität des Moduls zu behandeln — nicht tun. Es handelt sich um drei unabhängige Konzepte:
- **Typ** (`infos.library`/`infos.classname`) ist die eigentliche Identität des Moduls — die Bibliothek, aus der es geladen wurde. Zwei Instanzen desselben Typs (z. B. zwei Focus-Module) haben immer denselben Typ.
- **Template** (`infos.template`) ist nur ein Anzeige-Hinweis für das Frontend, rein kosmetisch und völlig optional. Zwei unterschiedliche Typen können sich dasselbe generische Template teilen (z. B. alles, was mit `"default"` gerendert wird).
- **Familie** (`infos.families`) ist eine Fähigkeit, die ein Modul über sich selbst deklariert, unabhängig von Typ und Template. Ein Modul kann keine, eine oder mehrere Familien deklarieren — das existiert, damit ein Client "jedes Modul, das guiden kann" ansprechen kann, ohne sich darum zu kümmern, ob der tatsächliche Typ `Guider` ist oder eine andere Implementierung derselben Fähigkeit.

`loadedModules-<type>` und `loadedFamilies-<family>` (weiter unten) verwenden das kleingeschriebene `infos.library`/`infos.families` als Schlüssel; `profiles-<Type>` dagegen das exakt geschriebene `infos.classname`. Immer das passende Feld direkt verwenden — niemals eines aus dem anderen durch clientseitige Groß-/Kleinschreibungs-Transformation ableiten.
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

{{% notice style="tip" title="`preicon`/`posticon`: klickbare Aktionen, keine Dekoration" %}}
Ein nicht-leerer Wert von `preicon1`/`preicon2`/`posticon1`/`posticon2` ist der Name des zu rendernden Google-Fonts-Icons (Material Icons) — und er ist klickbar: ein Klick darauf sendet [`I1`/`I2`/`I3`/`I4`]({{% relref "client-messages.de.md" %}}#i1) ohne Wert oder Parameter, nur ein leeres Payload, das die Property adressiert. Ein **leerer String bedeutet, dass an dieser Stelle kein Icon vorhanden ist** — nichts zu rendern, nichts zu klicken, nichts zu senden. Die elementbezogenen `preicon`/`posticon` weiter unten funktionieren genauso, über [`J1`/`J2`]({{% relref "client-messages.de.md" %}}#j1). Siehe [Icon-Aktionen]({{% relref "client-messages.de.md" %}}#icon-actions) für die vollständige Nachrichtenreferenz.
{{% /notice %}}

{{% notice style="warning" title="`permission` steuert editierbare Elemente, nicht ob der Wert angezeigt wird" %}}
`permission: 1` ("nur schreibend") bedeutet nicht „aktuellen Wert verstecken" — es bedeutet, dass die Property nichts Sinnvolles besitzt, das sich als übermittelbarer Eingabewert zurücklesen ließe (eine Fire-and-forget-Aktion). Eine Property, deren einziges Element ein reiner Anzeige-Typ ist (`img`, `video`, `light`, `prg`, `message`), kann durchaus `permission: 1` haben — eine Live-Kamera-Vorschau ist ein reales Beispiel — und muss trotzdem anhand ihres Werts gerendert werden, unabhängig von `permission`. Interaktivität (ein Eingabefeld anzeigen, einen Submit aktivieren) an `permission` koppeln, das Rendering dagegen an den Element-**Typ**, nicht an `permission`.
{{% /notice %}}

{{% notice style="note" title="`rule` hat nur Bedeutung, wenn alle Elemente vom Typ `bool` sind" %}}
`rule` ist bei jeder Property vorhanden (Standardwert `0`), hat aber nur dann Bedeutung — und sollte nur dann berücksichtigt werden —, wenn **alle** Elemente der Property vom Typ `bool` sind. Es taucht auch, wirkungslos, bei Properties auf, deren Elemente eine Mischung anderer Typen sind (z. B. ein Koordinatenpaar aus zwei `float`-Elementen) — ein Client sollte es dort ignorieren, statt zu versuchen, etwa zwei Floats als Radiogruppe darzustellen.
{{% /notice %}}

{{% notice style="note" title="`badge` wird vom Modulautor gesetzt, nicht zur Laufzeit von einem Client" %}}
Es gibt keine Client-Nachricht, die `badge` umschaltet — es ist pro Property/pro Element fest in der eigenen Definition des Moduls verdrahtet, kein benutzer- oder sitzungsspezifischer Favorit, den ein Client setzen könnte. Ein Favoriten-Bildschirm kann anzeigen und aggregieren, was bereits markiert ist, hat aber keine Möglichkeit, einen Benutzer selbst etwas als Favorit markieren zu lassen.
{{% /notice %}}

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
| `hasGraph` | bool | Die Property enthält einen Graph — **immer vorhanden**, auch wenn `false` |
| `graphType` | string | `"XY"` · `"DY"` · `"SXY"` · `"SDY"` · `"PHD"` — siehe unten |
| `graphParams` | object | Welches Element welche Rolle im Graph liefert — siehe unten |

Eine Property mit Graph ist immer auch eine Grid-Property (`hasGrid: true`, siehe oben): jede Grid-Zeile ist ein Datenpunkt, und `graphParams` ordnet einer abstrakten Graph-Rolle den **Element-Schlüssel** zu (einer aus `gridheaders`), dessen Wert in dieser Zeile die Rolle liefert.

| `graphType` | Form | Rollen in `graphParams` |
|---|---|---|
| `XY` | Eine Streudiagramm-Serie, feste Achsengrenzen | `X`, `Y`, `Xmin`, `Xmax`, `Ymin`, `Ymax`, optional zusätzlich `graphColor` (unten) |
| `DY` | Eine Serie, Datum/Zeit auf X | `D` (Datum/Zeit-Element), `Y`, optional zusätzlich `graphColor` (unten) |
| `SXY` | Mehrere Streudiagramm-Serien, gruppiert nach einem Serienschlüssel | `S` (Serienname-Element), `X`, `Y`, optional zusätzlich `graphColors`/`axis`/`mapping` (unten) |
| `SDY` | Mehrere Serien, Datum/Zeit auf X, gruppiert nach einem Serienschlüssel | `S`, `D`, `Y`, optional zusätzlich `graphColors`/`axis`/`mapping` |
| `PHD` | Eigener PHD2-artiger Guiding-Graph (Drift + SNR + RMS) | `D`, `RA`, `DE`, `pRA`, `pDE`, `RSB`, `RMS` — feste, zweckgebundene Rollen |

Optionale Felder in `graphParams`:

| Feld | Verwendet von | Beschreibung |
|---|---|---|
| `graphColor` | `XY`, `DY` | `{"R": int, "G": int, "B": int}` — Farbe der einzelnen Serie |
| `graphColors` | `SXY`, `SDY` | `{"<Serie>": {"R": int, "G": int, "B": int}}` — Farbe pro Serie |
| `axis` | `SXY`, `SDY` | `{"<Name>": {"min": number, "max": number, "pos": "left"\|"right", "text": string}}` — eine oder mehrere Y-Achsen |
| `mapping` | `SXY`, `SDY` | `{"<Serie>": "<Achsenname>"}` — gegen welchen Eintrag von `axis` jede Serie geplottet wird |

{{% notice style="warning" title="`PHD` ist kein generischer Graph-Typ" %}}
Anders als `XY`/`DY`/`SXY`/`SDY`, deren Rollen generisch genug sind, um eine einzige wiederverwendbare Graph-Komponente zu steuern, hat `PHD` feste, zweckgebundene Rollen für einen PHD2-artigen Guiding-Graphen (verwendet vom `guider`-Modul). Ein Client, der eine generische Graph-Komponente für die anderen vier Typen baut, braucht für `PHD` wahrscheinlich eine eigene Implementierung — oder lässt es weg. Rooster tut Letzteres (nur `SXY`/`SDY`/`XY`/`DY`, `PHD` nicht behandelt).
{{% /notice %}}

{{% notice style="tip" title="Graphen als lebendige Komponenten rendern, nicht als erzeugte Bilder" %}}
Die Grid-Daten einer Graph-Property ändern sich mit der Zeit (`gc`/`gu`/`gd`/`gr`, oder ein frischer `uc`/`ap`/`aa`) — also wie jedes andere lebendige UI-Element behandeln: mit einer Diagramm-/Canvas-Komponente zeichnen, die sich bei jedem Update neu aufbaut, so wie Rooster es mit Chart.js macht. Nicht server- oder clientseitig in ein statisches Bild rendern (PNG-Schnappschuss, serverseitig gerendertes Diagramm) und dann nur das `<img>` austauschen — das sieht merklich schlechter aus als ein echtes Diagramm (keine flüssigen Updates, keine Interaktivität, sichtbares Flackern/Neuladen bei jedem Punkt), und es verschenkt den ganzen Sinn, Grid-Daten überhaupt live über die Leitung zu bekommen.
{{% /notice %}}

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
| `preicon` | string | Icon vor dem Element — klickbare Aktion, siehe Hinweis oben (optional) |
| `posticon` | string | Icon nach dem Element — klickbare Aktion, siehe Hinweis oben (optional) |

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
| `hfravgdev` | float | Standardabweichung von `hfravg` |
| `stars` | int | Anzahl der erkannten Sterne |
| `issolved` | bool | Zeigt an, ob die Plate-Solving-Auflösung erfolgreich war |
| `solverra` / `solverde` | float | Aufgelöste RA/Dec-Koordinaten |
| `solverorientation` | float | Rotationswinkel des aufgelösten Feldes: von Norden zur Bildoberkante (Grad), nur relevant wenn `issolved` |
| `solverpixscale` | float | Aufgelöste Pixelskala (Bogensekunden/Pixel), nur relevant wenn `issolved` |
| `solverfieldwidth` / `solverfieldheight` | float | Aufgelöste Feldgröße (Bogenminuten), nur relevant wenn `issolved` |
| `sampling` | float | Nominale Pixelskala (Bogensekunden/Pixel), berechnet aus der Brennweite/dem Reducer der Optik und der Pixelgröße der Kamera des Moduls — immer verfügbar, sobald Kamera und Optik konfiguriert sind, unabhängig von einer Auflösung (im Gegensatz zu den obigen `solver*`-Feldern) |
| `min` / `max` / `mean` / `median` / `stddev` | float[] | Statistiken pro Kanal (siehe unten) |
| `histogram` | array | Histogrammdaten pro Kanal (siehe unten) |
| `alternates` | string[] | Optional: alternative Versionen des Bildes |
| `showstats` | bool | Vom Modulentwickler festgelegt: ob die obigen Statistikfelder für dieses Bild aussagekräftig sind und angezeigt werden sollen (ein Modul kann dies auf `false` setzen, wenn es sie nicht berechnet oder sie nicht zutreffen) |

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

Die LOV selbst wird auf Modulebene (`globallovs`) oder Controller-Ebene (`lovs`) definiert:
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

**Eingebaute Controller-LOVs**:

| Schlüssel | Inhalt | Verfügbar |
|---|---|---|
| `loadedModules` | Alle geladenen Module: `{Modulname: Bezeichnung}` | Immer |
| `loadedModules-<type>` | Module eines bestimmten Typs, passend zu `infos.library` (z. B. `loadedModules-focus`) | Sobald ein Modul dieses Typs geladen ist |
| `loadedFamilies-<family>` | Module, die eine bestimmte Familie deklarieren, passend zu `infos.families` (z. B. listet `loadedFamilies-guiding` jedes geladene Modul auf, das guiden kann, unabhängig von seinem tatsächlichen Typ) | Sobald ein Modul, das diese Familie deklariert, geladen ist |
| `profiles-<Type>` | Verfügbare Profile für einen Modultyp, passend zu `infos.classname` exakt — Achtung auf die Großschreibung, z. B. `profiles-Focus` | Sobald ein Profil dieses Typs gespeichert wurde |

---

## Zugriffskontrolle

Die Dump-Antwort enthält die Werte `grant-server` und `grant-client`.

`grant-server` ist die feste Zugriffsrichtlinie des Servers selbst (beim Serverstart festgelegt, für jeden Client gleich):

| Wert | Bedeutung |
|---|---|
| `"0"` | Vollständiger Lese-Schreib-Zugriff, kein Login nötig — `grant-client` ist immer `"1"` |
| `"1"` | Vollständiger Lesezugriff, kein Login nötig; der Schreibzugriff hängt von den Rechten des angemeldeten Benutzers ab — `grant-client` ist `"0"` oder `"1"` |
| `"2"` | Ohne Login kein Zugriff; Lese- und Schreibzugriff hängen von den Rechten des angemeldeten Benutzers ab — `grant-client` ist `"-1"`, `"0"` oder `"1"` |

`grant-client` ist der aktuelle Grant dieser Verbindung, der sich nach einem erfolgreichen `LO` (Login) ändern kann:

| Wert | Bedeutung |
|---|---|
| `"1"` | Vollständiger Lese-Schreib-Zugriff |
| `"0"` | Nur Lesen |
| `"-1"` | Zugriff verweigert (Authentifizierung erforderlich) |

Ist `grant-client` gleich `"-1"`, werden keine Moduldaten gesendet. Die `LO`-Nachricht (Login) zur Authentifizierung verwenden.
