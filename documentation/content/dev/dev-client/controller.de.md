---
title: Controller, GlobalDatastore und Dateien
weight: 30
description: "Was kein gewöhnliches Modul ist: Controller-Daten, Systemlast, Konfigurationen, das Modul GlobalDatastore, Dateisystem"
---

Diese Seite beschreibt, was aus dem Schema „Module → Properties → Elemente“ der vorherigen Seiten herausfällt: die **Controller-Daten**, das zentrale Modul **`GlobalDatastore`** und das **Dateisystem** des Servers. Ein Client, der sich darauf beschränkt, Module anzuzeigen, braucht sie nicht; ein Client, der das anbieten will, was eine Seite mit globalen Einstellungen bietet (Konfigurationen, INDI-Treiber, Serverlast, Ausrüstung, Bilder), muss sie verarbeiten.

## Die Controller-Daten (`controllerdata`)

Der Controller ist die Komponente des Servers, die die Module lädt und den eingebetteten INDI-Server steuert. Er stellt seine eigenen Daten im Dump unter dem Schlüssel `controllerdata` bereit (siehe [`d`]({{% relref "server-messages.de.md" %}}#d)) und hält sie anschließend mit [`uc`]({{% relref "server-messages.de.md" %}}#uc)-Nachrichten aktuell.

| Schlüssel | Inhalt |
|---|---|
| `availableconfs` | Gespeicherte Modulkonfigurationen: Objekt `Konfigurationsname → Liste von {modulelabel, moduletype, profilename}` |
| `currentconf` | Name der aktuellen Konfiguration |
| `banner` | Banner des Servers (Freitext, Option `--banner`) |
| `git` | Version des Servers: `Gitdate`, `Githash`, `Gitmessage`, `Gittag` |
| `libraries` | Verfügbare Modulbibliotheken, ein Metadatenobjekt pro Bibliothek |
| `profiles` | Verfügbare Profile: Objekt `transformierter Bibliotheksname → Liste von Profilnamen` — siehe die Schlüsselkonvention unten |
| `indiserver` | `"Y"`, wenn der eingebettete INDI-Server aktiviert ist, sonst `"N"` |
| `indidrivers` | Katalog der installierten INDI-Treiber: Liste von `{binary, family, label, mdpd}` |
| `indiactivedrivers` | Aktuell gestartete INDI-Treiber |
| `systemwatcher` | Serverlast, siehe unten |
| `files`, `folders` | Kopie von `d.files.files` und `d.files.folders`, siehe [Dateisystem](#fichiers) |

{{% notice style="warning" title="`profiles`-Schlüssel: weder der rohe Bibliotheksname noch `infos.template`" %}}
`profiles` wird über den **Bibliotheksnamen** des Moduls indiziert (derselbe String, der als `lib` an [`ML`]({{% relref "client-messages.de.md" %}}#ml) übergeben wird, also der Schlüssel in `controllerdata.libraries` ohne das `libost`-Präfix) — allerdings transformiert: **erster Buchstabe groß, der Rest klein** (`"focus"` → `"Focus"`, keine einfache Großschreibung des ersten Buchstabens — ein Bibliotheksname mit einem weiteren Großbuchstaben würde trotzdem eingeebnet, z. B. ein hypothetisches `"myFocus"` → `"Myfocus"`). Dieser Schlüssel ist weder der Instanzname/das Label des Moduls, noch notwendigerweise identisch mit `infos.template` (beide stimmen in der Praxis meist überein, werden aber unabhängig gesetzt, und nichts im Protokoll garantiert eine Übereinstimmung).

Diese Konvention ist sonst nirgends dokumentiert und wird vom Backend nicht erzwungen — sie wurde aus Roosters eigener Lookup-Logik zurückentwickelt. Zwei unabhängige, allein aus dieser Dokumentation gebaute Clients sind darüber gestolpert (eine Profilauswahl, die stillschweigend leer blieb). Wer die verfügbaren Profile eines Moduls nachschlagen will, sollte genau diese Transformation auf den Bibliotheksnamen anwenden, statt zu raten.
{{% /notice %}}

{{% notice style="warning" title="`uc`: ein Schlüssel pro Nachricht, Wert wird komplett ersetzt" %}}
Eine `uc`-Nachricht trägt **einen einzigen Schlüssel** von `controllerdata`, und ihr Wert ersetzt den alten **vollständig** (keine Zusammenführung). Ein Client, der nur den ersten Schlüssel des Payloads liest (wie es Rooster tut), ist daher korrekt, solange der Server nur einen Schlüssel pro Nachricht sendet — was heute der Fall ist.
{{% /notice %}}

### `systemwatcher`: Serverlast

Der Server misst seine eigene Last in regelmäßigen Abständen und sendet sie an alle Clients. Das Intervall wird mit der Option `--systemwatchinterval` eingestellt (Sekunden, **Standard 10**, `0` deaktiviert). Die Nachricht ist ein gewöhnliches `uc`, und der letzte Wert steht auch in `controllerdata.systemwatcher` des Dumps.

```json
{
  "uc-update controller data": {
    "systemwatcher": {
      "cpu": { "load_1m": 10.42, "load_5m": 9.79, "load_15m": 4.51 },
      "ram": { "used_mb": 12194, "total_mb": 31941 },
      "disks": [
        { "device": "/dev/nvme0n1p2", "mountpoint": "/", "used_gb": 789.4, "total_gb": 915.3 }
      ],
      "network": [
        { "iface": "enp4s0", "rx_kbps": 16.27, "tx_kbps": 448.8 }
      ],
      "temperatures": [
        { "type": "Tctl", "temp_c": 75.375 }
      ]
    }
  }
}
```

| Feld | Bedeutung |
|---|---|
| `cpu.load_1m` / `load_5m` / `load_15m` | **Durchschnittliche Systemlast** (wie `/proc/loadavg`), **kein Auslastungsprozentsatz**: 1.0 = im Mittel ein ausgelasteter Kern |
| `ram.used_mb` / `total_mb` | Belegter Speicher (gesamt − verfügbar) und Gesamtspeicher, in MB |
| `disks[]` | Ein Element pro eingehängtem Speichergerät (`/dev/*`, ohne `loop`): `device`, `mountpoint`, `used_gb`, `total_gb` |
| `network[]` | Ein Element pro Schnittstelle (ohne `lo`): Raten `rx_kbps` / `tx_kbps`, gemessen zwischen zwei Nachrichten; der erste Messwert einer Schnittstelle ist 0 |
| `temperatures[]` | Hardware-Sensoren des Prozessors: `type`, `temp_c`. **Die Liste kann je nach Rechner leer sein** (nur Sensoren vom Typ Intel `coretemp` und AMD `k10temp` werden gelesen) |

{{% notice style="tip" title="Nicht davon ausgehen, dass die Arrays gefüllt sind" %}}
`disks`, `network` und vor allem `temperatures` können leer sein. Ein Client sollte den entsprechenden Abschnitt ausblenden, statt ein leeres Array anzuzeigen. Das Protokoll liefert nicht die Kernzahl des Servers: eine CPU-Anzeige kann daher kein echter Prozentwert sein, der Wert muss als Last dargestellt werden.
{{% /notice %}}

## Konfigurationen und Modul-Lebenszyklus

Eine **Konfiguration** ist eine benannte Menge von Modulen (Bibliothek, Label, Profil), die der Server auf einmal neu laden kann. Die Liste steht in `controllerdata.availableconfs`, die aktive Konfiguration in `controllerdata.currentconf`.

| Aktion | Befehl | Detail |
|---|---|---|
| Konfiguration laden | [`CL`]({{% relref "client-messages.de.md" %}}#cl) | Lädt alle Module der Konfiguration |
| Aktuelle Konfiguration speichern | [`CS`]({{% relref "client-messages.de.md" %}}#cs) | Unter dem angegebenen Namen |
| Modul laden | [`ML`]({{% relref "client-messages.de.md" %}}#ml) | Bibliothek, Label, optionales Profil |
| Modul entladen | [`MK`]({{% relref "client-messages.de.md" %}}#mk) | Nach Modulname |

Die tatsächlich geladenen Module sind die des Dumps (`d.m`); sie erscheinen und verschwinden über [`aa`]({{% relref "server-messages.de.md" %}}#aa) und [`dm`]({{% relref "server-messages.de.md" %}}#dm). Die Konfiguration enthält **niemals** `GlobalDatastore` (siehe unten), das immer vorhanden ist.

{{% notice style="warning" title="Eine Konfiguration enthält stillschweigend die aktiven INDI-Treiber" %}}
`CS` speichert neben der Modulliste auch die gerade laufenden INDI-Treiber, und `CL` startet sie wieder — das ist aber in `controllerdata.availableconfs` unsichtbar, dessen Form nur Module beschreibt (`{modulelabel, moduletype, profilename}`), niemals Treiber. Ein Client, der einen Konfigurationseditor baut, der nur die Modulliste zeigt/bearbeitet, zeigt nicht das vollständige Bild dessen, was bei `CL` tatsächlich passiert.
{{% /notice %}}

{{% notice style="note" title="Keine Löschoperation" %}}
Es gibt keinen Befehl, um eine gespeicherte Konfiguration zu entfernen — `CL` und `CS` sind die einzigen beiden Operationen. Eine Konfiguration, die ein Client nicht mehr will, bleibt dauerhaft in `controllerdata.availableconfs` (sie kann nur unter demselben Namen durch ein weiteres `CS` überschrieben werden).
{{% /notice %}}

### INDI-Server und Treiber

Wenn `controllerdata.indiserver` den Wert `"Y"` hat, steuert der Server eine eingebettete INDI-Instanz. Die Befehle [`YA`/`YZ`/`YL`/`YR`/`YS`]({{% relref "client-messages.de.md" %}}#indi) starten und stoppen sie und verwalten die Treiber; `controllerdata.indidrivers` liefert den verfügbaren Katalog und `controllerdata.indiactivedrivers` das, was läuft. `binary` ist der Wert, der als `driver` in `YL`, `YR` und `YS` übergeben wird.

`controllerdata.indidrivers` und `controllerdata.indiactivedrivers` sind beide Listen von Objekten `{binary, family, label}` (`indidrivers` hat zusätzlich `mdpd`, siehe Tabelle oben).

{{% notice style="warning" title="YA/YZ/YL/YR/YS haben keine eigene Bestätigung" %}}
Keiner dieser Befehle löst eine eigene Bestätigungsnachricht aus. Erfolg oder Misserfolg muss indirekt abgeleitet werden: Auf `YL`/`YR`/`YS` folgt eine `uc`-Aktualisierung von `indiactivedrivers`, die die angeforderte Änderung widerspiegelt (das bestätigt aber nur, dass der Server es *versucht* hat, nicht dass der zugrunde liegende INDI-Treiber tatsächlich hochgefahren ist), während `YA`/`YZ` überhaupt kein `uc` erzeugen — nur `l`-Nachrichten (Log), die für einen Menschen gedacht sind, nicht zum programmatischen Auswerten durch einen Client. `controllerdata.indiserver` selbst spiegelt die Startoption `--indiserver` wider, nicht ob der eingebettete Server gerade tatsächlich läuft.
{{% /notice %}}

## Das Modul `GlobalDatastore` {#globaldatastore}

`GlobalDatastore` ist im Dump ein Modul **wie jedes andere** (`d.m.GlobalDatastore`, mit Properties, Elementen, Grids, Profilen), spielt aber eine zentrale Rolle:

- es wird vom Server **vor allen anderen Modulen** erstellt und ist **immer vorhanden**: `MK` darauf wird abgelehnt, und es ist nie Teil einer Konfiguration;
- es trägt das **gemeinsame Ausrüstungsverzeichnis**, das alle anderen Module nutzen: Optiken, Standorte, Ausrüstungen (Gerätesätze) und INDI-Server.

Seine Grids dienen als Quelle für Auswahllisten (LOV) gleichen Namens, die in `d.lovs` stehen (`optics`, `locations`, `equipments`, `servers`) und bei jeder Änderung des Grids neu aufgebaut werden. Die anderen Module nutzen sie so: Die Wahl einer Optik füllt Brennweite und Durchmesser, die Wahl eines Standorts füllt Breite, Länge und Höhe, die Wahl eines Ausrüstungssatzes löst die Geräte auf (Kamera, Montierung, Fokussierer, Filterrad, Guiding-Kamera, GPS, Kuppel, Wetter) sowie den zu kontaktierenden INDI-Server.

Bereitgestellte Properties:

| Property | Rolle |
|---|---|
| `devices` | Ausgewählter Ausrüstungssatz und GPS (`equipments`, `gps`) |
| `devicesactions` | Schaltflächen: Geräte verbinden/trennen, Konfigurationen laden (`condevs`, `discondevs`, `loadconfs`) |
| `server` | Host und Port des INDI-Servers (`host`, `port`) |
| `serveractions` | Schaltflächen: INDI-Server verbinden/trennen (`conserv`, `disconserv`) |
| `startup` | Kontrollkästchen für den automatischen Start (`indiatstart`, `devatstart`, `confsatstart`) |
| `equipments` | Grid der Ausrüstungssätze |
| `locations` | Grid der Standorte (Icons `refresh` = GPS lesen, `add` = aktuelle Zeile hinzufügen) |
| `optics` | Grid der Optiken |
| `servers` | Grid der INDI-Server |

Alle Properties werden mit den Standardnachrichten bedient (`SV`, `SA`, `GC` …, `I3`/`I4` auf `locations`); ein generischer Client zeigt sie wie jedes andere Modul an. Rooster stellt sie in einem eigenen Tab seiner Einstellungsseite dar, wobei es die Properties mit Profil (`hasprofile`) auswählt und nach `level1`/`level2` gruppiert.

## Dateisystem {#fichiers}

Der Server veröffentlicht die Liste der erzeugten Dateien (Bilder, Sequenzen, Archive) im Ordner `--webroot`, ohne dass der Client danach fragen muss. Dieser Mechanismus ist unabhängig von den Modulen.

### Im Dump: `d.files`

```json
"files": {
  "folders": ["", "/Allsky", "/Sequencer/gam Cyg/LIGHT/Luminance"],
  "files": ["/image1.jpeg", "/rapport.txt"],
  "selectedfolder": "/home/gilles/ostmedia"
}
```

| Feld | Inhalt |
|---|---|
| `folders` | **Alle** Unterordner des Webroots, rekursiv, Pfade relativ zum Webroot und mit führendem `/`. Das Wurzelverzeichnis ist die leere Zeichenkette `""`. Nicht sortiert. |
| `files` | Die Dateien **nur des ausgewählten Ordners** (nicht rekursiv), Pfade relativ zum Webroot und mit führendem `/`. Alle Dateien, ohne Erweiterungsfilter. |
| `selectedfolder` | Der ausgewählte Ordner als **absoluter Pfad im Dateisystem des Servers** (nicht relativ). |

Es gibt weder Paginierung noch Größenbegrenzung: bei einem sehr gefüllten Webroot kann die Liste lang sein.

### Den Ordner wählen: `FS`

[`FS`]({{% relref "client-messages.de.md" %}}#fs) bestimmt den Ordner, der überwacht und dessen Dateien aufgelistet werden sollen. Der Server überwacht den **Inhalt** immer nur eines **einzigen** Ordners; der Ordnerbaum (`folders`) wird unabhängig davon verfolgt, siehe unten.

{{% notice style="warning" title="Der ausgewählte Ordner gilt global für den Server" %}}
Es gibt keinen Ordner pro Client: ein `FS` ändert die Dateiliste, die **alle** verbundenen Clients sehen. Ein Client sollte es nicht für eine rein „lokale“ Navigation verwenden, wenn andere Clients verbunden sein können.

Der Pfad **wird validiert**: das Ziel muss existieren und innerhalb des Webroots liegen (kein Entkommen über `../`). Eine ungültige Anfrage wird serverseitig abgelehnt (protokolliert, kein Zustandswechsel) — sie liefert nicht stillschweigend eine leere Liste.
{{% /notice %}}

Ein erfolgreiches `FS` ersetzt die Dateiliste vollständig: der Client erhält eine einzige [`uc`]({{% relref "server-messages.de.md" %}}#uc)-Nachricht `files` mit der kompletten neuen Liste, keinen Strom von `af`/`df` — eine Ordnerauswahl ist keine Änderung am Dateisystem, es wurde tatsächlich nichts hinzugefügt oder entfernt. Es gibt **keine** Nachricht, die den neuen `selectedfolder` selbst ankündigt, der nur im Dump existiert.

### Ordnerbaum vs. Dateien des ausgewählten Ordners

`folders` (der gesamte rekursive Baum) und `files` (der Inhalt des ausgewählten Ordners) haben unterschiedliche Reichweiten:

| | Reichweite | Live-Aktualisierungen |
|---|---|---|
| `folders` / `ad` / `dd` | **Der gesamte Webroot**, rekursiv — unabhängig von der aktuellen Auswahl | Immer, unabhängig von der aktuellen `FS`-Auswahl |
| `files` / `af` / `df` | **Nur der ausgewählte Ordner** | Nur für Änderungen am Ordner, den das letzte `FS` ausgewählt hat |

Ein Ordner, der irgendwo im Webroot erstellt oder gelöscht wird, wird live über `ad`/`dd` gemeldet, auch wenn kein Client ihn gerade ausgewählt hat. Eine Datei wird nur über `af`/`df` gemeldet, wenn sie den Ordner betrifft, den das `FS` eines Clients zuletzt ausgewählt hat — `files`/`af`/`df` sagen nichts über den Inhalt anderer Ordner aus, ein Client muss dorthin ein `FS` senden, um es zu erfahren.

### Aktualisierungen: `af` / `df` / `ad` / `dd`

Wenn sich der Ordnerbaum oder der Inhalt des ausgewählten Ordners tatsächlich ändert, sendet der Server eine Nachricht pro betroffenem Pfad — der Wert ist der reine Pfad selbst, nicht in ein Objekt oder Array verpackt:

```json
{ "af": "/Sequencer/gam Cyg/LIGHT/Luminance/img_001.FITS" }
{ "df": "/rapport.txt" }
{ "ad": "/Sequencer/neues Ziel" }
```

| Schlüssel | Bedeutung |
|---|---|
| `af` | Datei im ausgewählten Ordner hinzugekommen |
| `df` | Datei aus dem ausgewählten Ordner verschwunden |
| `ad` | Unterordner irgendwo im Webroot erschienen |
| `dd` | Unterordner irgendwo im Webroot verschwunden |

Jede Nachricht trägt genau einen Pfad (N Dateien = N Nachrichten) — siehe [`af`/`df`/`ad`/`dd`]({{% relref "server-messages.de.md" %}}#af) für die vollständige Referenz. Eine Umbenennung wird als Löschung gefolgt von einer Erstellung gemeldet: der Server vergleicht eine Vorher-/Nachher-Liste, er erkennt Umbenennungen nicht als solche.

Nach den Pfad-Nachrichten sendet der Server [`uc`]({{% relref "server-messages.de.md" %}}#uc)-Nachrichten `files` und/oder `folders` mit der vollständigen aktualisierten Liste — aber nur für das, was sich bei diesem Durchlauf tatsächlich geändert hat, nie bedingungslos. Ein Client kann also entweder die Pfad-Ereignisse laufend anwenden oder seine Liste(n) einfach mit der/den passenden `uc`-Nachricht(en) ersetzen.

### Die URL einer Datei bilden

Die Pfade in `files` sind relativ zum Webroot und beginnen mit `/`. Die URL einer Datei ist die Basis-URL der Medien des Servers (`http(s)://hostname/ostmedia/`, siehe den Hinweis „URLs media“ im [Datenmodell]({{% relref "structure.de.md" %}})) gefolgt vom Pfad. Vorsicht beim führenden `/`: an eine Basis angehängt, die schon mit `/` endet, ergibt es ein doppeltes `//`.

### Was das Ereignis nicht leistet

Der Server erkennt das **Hinzufügen, Löschen und Umbenennen** von Dateien, nicht die Änderung des Inhalts einer bestehenden Datei: ein unter demselben Namen neu geschriebenes Bild löst kein Ereignis aus. Das gilt für die Vorschaubilder der Module, deren URL bei jeder Aufnahme identisch bleibt: ein Client muss einen Cache-Busting-Parameter (`?t=` + Zeitstempel) anhängen, wenn er ein Bild nach einer Aktualisierung seiner Property neu lädt.
