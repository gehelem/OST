---
title: Grundsätze der Client-Architektur
weight: 10
description: "Wie ein OST-Client strukturiert sein sollte: ein einziger Datastore, Schreibzugriffe nur über WS"
---

Diese Seite beschreibt **wie ein Client strukturiert sein sollte**, nicht das
Format der ausgetauschten Nachrichten (siehe [Datenmodell]({{% relref "structure.de.md" %}})
und die folgenden Seiten dafür).

## Grundsatz: ein einziger Datastore hält die Wahrheit

Der Client verwaltet **eine einzige Datenstruktur**, die den aktuellen Zustand
abbildet (Module → Properties → Elemente). Die gesamte UI **liest** diese
Struktur; nichts anderes verändert sie außer den vom Server empfangenen
Nachrichten.

```
Lesen    :  Datastore ────────────► UI

Schreiben:  UI ─── SV/SA ───► Server ─── Update ───► Datastore
```

- **Keine Duplizierung**: Braucht eine Komponente einen Wert, liest sie ihn
  aus dem Datastore, statt eine eigene Kopie zu halten.
- **Kein direktes Schreiben**: Die UI verändert den Datastore niemals selbst,
  auch nicht für eine optimistische Anzeige.

{{% notice style="tip" title="Schreiben: immer über den Server" %}}
Eine Benutzeraktion (Wert ändern, Button klicken) löst das Senden einer
Nachricht an den Server aus (`SV`/`SA` — siehe
[Client → Server]({{% relref "client-messages.de.md" %}})). Der lokale
Datastore **ändert sich erst als Reaktion darauf**, sobald der Server mit dem
bestätigten Update antwortet. Der Client nimmt das Ergebnis niemals vorweg.
{{% /notice %}}

## Vollständiger Dump vs. teilweise Updates

Bei der Verbindung sendet der Client `DU`; der Server antwortet mit einem
vollständigen Dump (`d`), der alle Module mit ihren vollständigen Metadaten
enthält (Label, Typ, Min/Max, LOV usw.).

{{% notice style="tip" title="Teilweise Updates enthalten nur den Wert" %}}
Die danach eintreffenden Nachrichten (`ee`, `ea`) enthalten **nur den Wert**,
keine Metadaten. Der Client muss:
1. Die Metadaten des letzten vollständigen Dumps behalten.
2. Bei einem teilweisen Update **nur** den Wert patchen, ohne jemals
   anzunehmen, dass sich die Metadaten geändert haben.

Ändern sich die Metadaten selbst (Min/Max/Format eines Elements), signalisiert
der Server dies explizit über eine eigene Nachricht, `ev`
(siehe [Server → Client]({{% relref "server-messages.de.md" %}})) — das ist
niemals stillschweigend. Der Client muss also nur auf den empfangenen
Nachrichtentyp reagieren, ohne raten zu müssen, was sich geändert hat.
{{% /notice %}}

{{% notice style="warning" title="Bekannte Falle: GlobalLOV-Updates" %}}
Wenn eine Property auf eine globale LOV verweist und diese sich ändert,
sendet der Server das Update **ohne das Feld `value`**. Der Client darf in
diesem speziellen Fall **niemals den alten Wert beibehalten** — er muss
genau das anwenden, was der Server sendet, einschließlich des fehlenden
Wertes. Der Server ist maßgeblich; ein fehlendes Feld darf niemals mit dem
alten lokalen Zustand "aufgefüllt" werden.
{{% /notice %}}

## Rendering: Aktualisierungen auf die geänderte Property beschränken

{{% notice style="warning" title="Verpflichtend: niemals mehr als die geänderte Property neu rendern" %}}
Manche Module können mehrere `ee`/`ea`-Updates **pro Sekunde** senden
(zum Beispiel ein Modul, das kontinuierlich ein Gerät abfragt). Ein
Client, der bei jedem eingehenden Update seine gesamte UI neu rendert
— oder auch nur das gesamte angezeigte Modul —, wird unter dieser Last
unbenutzbar: Verlust des Eingabefokus, Flackern, im schlimmsten Fall
reagiert die UI überhaupt nicht mehr auf Eingaben.

Das ist kein Grenzfall, gegen den man sich "vorsichtshalber" absichert:
Es ist reales, aktuelles Backend-Verhalten, und Clients **müssen** von
Anfang an dafür gebaut sein:
1. Bei jedem Update darf nur das JS-/Datenobjekt der geänderten
   Property ersetzt/verändert werden. Die Referenz jeder anderen
   Property muss exakt gleich bleiben, solange sie nicht betroffen ist.
2. Das UI-Rendering muss anhand der Identität der Property (oder des
   Moduls) indiziert sein — eine Liste von Properties sollte über
   mehrere Render-Durchläufe hinweg dasselbe UI-Element für dieselbe
   Property wiederverwenden, niemals zerstören und neu erzeugen, damit
   ein nicht betroffenes Eingabefeld niemals angefasst wird, egal was
   anderswo gerade passiert.
3. Nur die **Häufigkeit** des Renderns zu begrenzen (Debounce/Throttle)
   reicht nicht aus, wenn das Rendering selbst weiterhin unbeteiligte
   UI neu aufbaut — die Lösung liegt im **Umfang** dessen, was neu
   aufgebaut wird, nicht nur in dessen Häufigkeit.

Rooster (das Produktions-Frontend) setzt dieses Prinzip von Grund auf
um: Ein eingehendes Update ersetzt im Store nur die Referenz der
betroffenen Property, und jede Liste von Properties wird mit einem
stabilen Schlüssel pro Property gerendert. Dadurch wird die UI nicht
betroffener Properties niemals gestört, egal wie schnell sich eine
andere Property anderswo aktualisiert.
{{% /notice %}}

## Templates: Bildschirme pro Modultyp mit generischem Fallback

Jede Modulinstanz kann `infos.template` angeben (siehe
[Datenmodell]({{% relref "structure.de.md" %}})) — eine Zeichenkette,
die ihren Typ benennt (`"focus"`, `"guider"`, `"sequencer"` ...). Das
ist nur ein Hinweis, und er **darf fehlen**.

{{% notice style="tip" title="Fehlendes Template = der Standardbildschirm" %}}
Ein Client, der eigene Bildschirme pro Template unterstützt, sollte ein
fehlendes (oder unbekanntes) `infos.template` genauso behandeln wie den
wörtlichen Wert `"default"` — nicht als Fehler. Rooster macht das mit
einer kleinen Registry, die einen kleingeschriebenen Template-Namen auf
eine Bildschirm-Komponente abbildet, mit Rückfall auf den
generischen/Standard-Bildschirm, wenn der Name fehlt oder nicht in der
Registry steht:
```
template = infos.template?.toLowerCase() || "default"
screen = registry[template] ?? registry["default"]
```
Dadurch bleibt ein Client auch gegenüber einem Modultyp voll
funktionsfähig, den er noch nie gesehen hat — ein neues oder
Drittanbieter-Modul wird immer über den generischen Bildschirm
dargestellt —, während er trotzdem einen ausgefeilten, zweckgebundenen
Bildschirm für die Modultypen bieten kann, in die es sich lohnt zu
investieren.

Der `"default"`-Bildschirm ist kein rein symbolischer Minimal-Fallback:
er ist für manche Modultypen der **einzige** Bildschirm, den sie je
bekommen, und muss deshalb alles rendern, was eine generische
Property/Element tragen kann — Status, `hint`, Prä-/Posticons, Grids,
Graphen, LOVs, alles — nicht nur Werte. Ein eigener Bildschirm pro
Template darf kuratierter sein (Felder ausblenden, Layout
umorganisieren), aber der Standard-Bildschirm sollte keine
Information stillschweigend fallen lassen, nur weil noch niemand einen
eigenen Bildschirm für dieses Modul geschrieben hat.
{{% /notice %}}

{{% notice style="tip" title="Empfehlung: eine einzige, überall wiederverwendete Property-Rendering-Komponente" %}}
Nicht verpflichtend, aber sehr lohnenswert: eine **einzige** Komponente
bauen, die für das Rendering einer Property (oder eines Elements)
zuständig ist — `directedit`, `permission`, die drei LOV-Geltungsbereiche,
Bool-Gruppenregeln, `autoupdate`, Grids, Graphen behandelt — und diese
Komponente von **jedem** Bildschirm für einzelne Properties verwenden
lassen, auch von eigenen Bildschirmen pro Template, nicht nur vom
generischen Fallback.

Die Alternative — jeder Bildschirm implementiert sein eigenes
Feld-Rendering neu — verdoppelt jede Nuance des Wire-Formats über die
gesamte Codebasis hinweg, und ein Fix oder eine neue Backend-Fähigkeit
erreicht nur die Bildschirme, an die jemand denkt, sie zu aktualisieren.
Roosters eigene Bildschirme (`GuiderComponent`, `FocusComponent` ...)
rendern ihr eigenes Layout, delegieren aber jede einzelne Property an
dieselbe generische Property-Komponente, die auch der Fallback-Bildschirm
verwendet — ein Fix, der dort einmal gemacht wird (z. B. eine exklusive
Switch-Gruppe korrekt zu behandeln), wirkt automatisch überall,
eigene Bildschirme eingeschlossen.
{{% /notice %}}

## Property-Formulare: `SA`, verpflichtend bei `directedit: false`

Über das feldweise `SV` hinaus braucht eine editierbare Property einen **Formular-Pfad**: einen Bildschirm, der alle ihre editierbaren Elemente auf einmal auflistet und zusammen über [`SA`]({{% relref "client-messages.de.md" %}}#sa) sendet.

{{% notice style="warning" title="Nicht optional bei einem Element mit `directedit: false`" %}}
`SV` wird vom Backend für jedes Element mit `directedit: false` abgelehnt (siehe [Datenmodell]({{% relref "structure.de.md" %}})). `SA` ist der **einzige** Weg, ein solches Element jemals zu aktualisieren — ein Client ohne Formular-Pfad kann auf dieses Element schlicht nicht schreiben, egal wie gut der Rest seiner UI ist.
{{% /notice %}}

Nur **editierbare** Elemente gehören in dieses Formular — reine Anzeige-Typen (`img`, `video`, `light`, `prg`, `message`) werden gerendert, nie gesendet; sie sollten ganz aus dem Payload weggelassen werden, statt ihnen einen bedeutungslosen Wert zu geben.

Dieselbe „mehrere Elemente auf einmal senden"-Form wird auch von der Grid-Zeilen-Erstellung ([`GC`]({{% relref "client-messages.de.md" %}}#gc)) und -Aktualisierung ([`GU`]({{% relref "client-messages.de.md" %}}#gu)) verwendet — beide nehmen eine `e`-Map genau wie `SA`. Es lohnt sich, **eine** wiederverwendbare Formular-Komponente zu bauen, die sowohl das reine Property-Formular als auch den Grid-Zeile-erstellen- und den Grid-Zeile-bearbeiten-Dialog bedient, statt drei getrennte Implementierungen derselben Logik („editierbare Elemente rendern, ihre Werte sammeln"). Rooster macht das so.

## Heartbeat

{{% notice style="note" title="Optional" %}}
Der Heartbeat wird nicht vom Protokoll vorgeschrieben. Es ist ein Mechanismus,
den ein Client selbst einrichtet, nur falls seine Technologie es erfordert,
um die WebSocket-Verbindung aktiv zu halten (manche Browser, Proxys oder
Plattformen schließen eine als inaktiv geltende Verbindung). Ein Client, der
ihn nicht braucht, kann komplett darauf verzichten.

Der Mechanismus existiert auf Protokollebene für alle, die ihn benötigen:
alle 30 Sekunden `XX` senden, der Server antwortet mit `xx` (ohne
nützlichen Payload).
```json
→ {"XX": {}}
← {"xx": {}}
```
{{% /notice %}}

## Wiederverbindung

{{% notice style="tip" title="Immer wieder mit einem vollständigen Dump beginnen" %}}
Bei jeder Unterbrechung muss der Client **seinen Datastore vollständig
leeren** — nach einer Trennung kann kein Teilzustand als zuverlässig
angesehen werden.

Was der Client danach tut (automatisch N-mal erneut versuchen, den Benutzer
zum Neuverbinden aufzufordern, aufgeben …) geht nur ihn selbst an: Das ist
nicht durch das Protokoll vorgegeben, jeder Client ist in seiner eigenen
Strategie frei.

Aber sobald (erneut) verbunden, ist ein erneutes `DU` für einen frischen
vollständigen Dump quasi nicht optional: einen nutzbaren Zustand allein aus
den danach eintreffenden Teil-Updates (`ee`/`ea`/`ev`) wiederherzustellen ist
unmöglich — sie liefern immer nur einen fragmentarischen Ausschnitt, niemals
einen vollständigen Zustand. Auf Serverseite muss zwischen zwei Verbindungen
desselben Clients nichts aufbewahrt werden: Das ist eine rein clientseitige
Einschränkung.
{{% /notice %}}

## Minimales Beispiel: erster Austausch

Ohne Framework, nur der allererste Austausch (verbinden → `DU` → zuhören) —
entnommen aus `test-ws-advanced.js`, dem Skript für den stündlichen
Health-Check des Servers:

```javascript
const ws = new WebSocket("ws://127.0.0.1:9624");

ws.on("open", () => {
  ws.send(JSON.stringify({ "DU": { "language": "en" } }));
});

ws.on("message", (data) => {
  console.log("received:", data.toString());
});
```

## Beispiel: Lesen und Schreiben in Rooster

Reale Ausschnitte, die die eingangs beschriebene Trennung von Lesen und
Schreiben veranschaulichen, sobald ein tatsächlicher Client (hier Rooster,
in Angular) nach diesem Grundsatz aufgebaut ist.

**Lesen** — `datastore.service.ts`, ein einfacher Zugriff auf den internen
Zustand:
```typescript
getProperty(moduleName: string, propertyName: string): Observable<any> {
  return new Observable(subscriber => {
    const subscription = this.changed$.subscribe(() => {
      const module = this.modules[moduleName];
      const prop = (module as any)?.properties?.[propertyName] || null;
      subscriber.next(prop);
    });
    subscriber.next(this.modules[moduleName]?.properties?.[propertyName] || null);
    return () => subscription.unsubscribe();
  });
}
```

**Schreiben** — `websocket.service.ts`, keine lokale Mutation, nur das
Senden der Nachricht:
```typescript
setPropertyOneElement(module: string, property: string, elements: { [key: string]: any }): void {
  this.send({
    SV: { m: { [module]: { p: { [property]: { e: elements } } } } }
  });
}
```
