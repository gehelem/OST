---
title: Client Architecture Principles
weight: 50
description: "How to structure an OST client: single datastore, writes through the server only"
---

This page describes **how a client should be structured**, not the format
of the exchanged messages (see [Data Model]({{% relref "structure.en.md" %}})
and the previous pages for that).

## Principle: a single datastore holds the truth

The client maintains **a single data structure** representing the current
state (modules → properties → elements). All the UI **reads** this
structure; nothing else modifies it except messages received from the
server.

```
Read  :  Datastore ────────────► UI

Write :  UI ─── SV/SA ───► Server ─── update ───► Datastore
```

- **No duplication**: if a component needs a value, it reads it from the
  datastore, it does not keep its own copy.
- **No direct write**: the UI never modifies the datastore itself, not even
  for an optimistic display.

{{% notice style="tip" title="Writes: always through the server" %}}
A user action (changing a value, clicking a button) triggers sending a
message to the server (`SV`/`SA` — see
[Client → Server]({{% relref "client-messages.en.md" %}})). The local
datastore **only changes in return**, once the server responds with the
confirmed update. The client never assumes the outcome.
{{% /notice %}}

## Full dump vs partial updates

On connection, the client sends `DU`; the server responds with a full dump
(`d`) containing all modules with their complete metadata (label, type,
min/max, LOV, etc.).

{{% notice style="tip" title="Partial updates only carry the value" %}}
The messages that arrive afterwards (`ee`, `ea`) contain **only the
value**, not the metadata. The client must:
1. Keep the metadata from the last full dump.
2. On a partial update, patch **only** the value, never assuming the
   metadata has changed.

If the metadata itself changes (an element's min/max/format), the server
signals it explicitly via a distinct message, `ev`
(see [Server → Client]({{% relref "server-messages.en.md" %}})) — this is
never silent. The client only has to react to the message type it
receives, without having to guess what changed.
{{% /notice %}}

{{% notice style="warning" title="Known pitfall: GlobalLOV updates" %}}
When a property references a global LOV and that LOV changes, the server
sends the update **without the `value` field**. The client must **never
preserve the old value** in this specific case — apply what the server
sends as-is, including the absence of a value. The server is
authoritative; never try to "fill in" a missing field with the previous
local state.
{{% /notice %}}

## Heartbeat

{{% notice style="note" title="Optional" %}}
The heartbeat is not imposed by the protocol. It is a mechanism a client
sets up on its own, only if its technology requires it, to keep the
WebSocket connection alive (some browsers, proxies or platforms close a
connection deemed idle). A client that doesn't need it can skip it
entirely.

The mechanism exists at the protocol level for whoever needs it: send
`XX` every 30 seconds, the server replies `xx` (no meaningful payload).
```json
→ {"XX": {}}
← {"xx": {}}
```
{{% /notice %}}

## Reconnection

{{% notice style="tip" title="Always start again from a full dump" %}}
On any disconnection, the client must **entirely clear its datastore** —
no partial state can be considered reliable after a disconnect.

What the client does next (retry automatically N times, prompt the user
to reconnect, give up...) is entirely up to the client: this is not
specified by the protocol, each client is free to choose its own
strategy.

But once (re)connected, sending `DU` again to get a fresh full dump is
nearly mandatory: rebuilding a usable state from partial updates alone
(`ee`/`ea`/`ev`) is impossible — they only ever give a fragmentary view,
never a complete state. On the server side, nothing needs to be
preserved between two connections of the same client: this is purely a
client-side constraint.
{{% /notice %}}

## Minimal example: first exchange

No framework, just the very first exchange (connect → `DU` → listen) —
taken from `test-ws-advanced.js`, the script used for the server's
hourly health check:

```javascript
const ws = new WebSocket("ws://127.0.0.1:9624");

ws.on("open", () => {
  ws.send(JSON.stringify({ "DU": { "language": "en" } }));
});

ws.on("message", (data) => {
  console.log("received:", data.toString());
});
```

## Example: reading and writing in OstErix

Real excerpts illustrating the read/write separation from the top of
this page, once an actual client (here OstErix, in Angular) is built
around this principle.

**Read** — `datastore.service.ts`, a plain access to the internal state:
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

**Write** — `websocket.service.ts`, no local mutation, just sending the
message:
```typescript
setPropertyOneElement(module: string, property: string, elements: { [key: string]: any }): void {
  this.send({
    SV: { m: { [module]: { p: { [property]: { e: elements } } } } }
  });
}
```
