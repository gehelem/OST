---
title: Grundsätze der Client-Architektur
weight: 50
description: "Wie ein OST-Client strukturiert sein sollte: ein einziger Datastore, Schreibzugriffe nur über WS"
---

Diese Seite beschreibt **wie ein Client strukturiert sein sollte**, nicht das
Format der ausgetauschten Nachrichten (siehe [Datenmodell]({{% relref "structure.de.md" %}})
und die vorherigen Seiten dafür).

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

## Beispiel: Lesen und Schreiben in OstErix

Reale Ausschnitte, die die eingangs beschriebene Trennung von Lesen und
Schreiben veranschaulichen, sobald ein tatsächlicher Client (hier OstErix,
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
