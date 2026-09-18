---
title: Principios de arquitectura del cliente
weight: 50
description: "Cómo estructurar un cliente OST: datastore único, escritura únicamente vía WS"
---

Esta página describe **cómo debe estructurarse un cliente**, no el formato
de los mensajes intercambiados (ver [Modelo de datos]({{% relref "structure.es.md" %}})
y las páginas anteriores para eso).

## Principio: un datastore único posee la verdad

El cliente mantiene **una única estructura de datos** que representa el estado
actual (módulos → propiedades → elementos). Toda la UI **lee** esta
estructura; nada más la modifica salvo los mensajes recibidos del servidor.

```
Lectura   :  Datastore ────────────► UI

Escritura :  UI ─── SV/SA ───► Servidor ─── update ───► Datastore
```

- **Sin duplicación**: si un componente necesita un valor, lo lee
  en el datastore, no lo almacena por su cuenta.
- **Sin escritura directa**: la UI nunca modifica el datastore
  ella misma, ni siquiera para una visualización optimista.

{{% notice style="tip" title="Escritura: siempre a través del servidor" %}}
Una acción del usuario (cambiar un valor, hacer clic en un botón) desencadena
el envío de un mensaje al servidor (`SV`/`SA` — ver
[Cliente → Servidor]({{% relref "client-messages.es.md" %}})). El datastore
local **solo cambia como respuesta**, cuando el servidor responde con la
actualización confirmada. El cliente nunca da por hecho el resultado.
{{% /notice %}}

## Dump completo vs actualizaciones parciales

Al conectarse, el cliente envía `DU`; el servidor responde con un dump
completo (`d`) que contiene todos los módulos con sus metadatos
completos (label, type, min/max, LOV, etc.).

{{% notice style="tip" title="Las actualizaciones parciales solo llevan el valor" %}}
Los mensajes que llegan después (`ee`, `ea`) contienen **solo el
valor**, no los metadatos. El cliente debe:
1. Conservar los metadatos del último dump completo.
2. En una actualización parcial, aplicar el parche **solo** al valor, sin
   asumir nunca que los metadatos han cambiado.

Si los metadatos en sí cambian (min/max/format de un elemento),
el servidor lo señala explícitamente mediante un mensaje distinto, `ev`
(ver [Servidor → Cliente]({{% relref "server-messages.es.md" %}})) — esto
nunca es silencioso. El cliente solo tiene que reaccionar al tipo de
mensaje recibido, sin tener que adivinar qué ha cambiado.
{{% /notice %}}

{{% notice style="warning" title="Trampa conocida: actualizaciones de GlobalLOV" %}}
Cuando una propiedad referencia una LOV global y esta cambia, el
servidor envía la actualización **sin el campo `value`**. El cliente nunca
debe **preservar el valor antiguo** en este caso concreto — hay que
aplicar lo que el servidor envía tal cual, incluida la ausencia de
valor. El servidor es la autoridad; nunca intentar "rellenar" un campo
ausente con el estado local anterior.
{{% /notice %}}

## Heartbeat

{{% notice style="note" title="Opcional" %}}
El heartbeat no lo impone el protocolo. Es un mecanismo que
el cliente implementa por su cuenta, solo si su tecnología
lo requiere, para mantener activa la conexión WebSocket (algunos
navegadores, proxies o plataformas cierran una conexión considerada
inactiva). Un cliente que no lo necesite puede omitirlo por completo.

El mecanismo existe a nivel de protocolo para quien lo necesite: enviar
`XX` cada 30 segundos, el servidor responde `xx` (sin payload útil).
```json
→ {"XX": {}}
← {"xx": {}}
```
{{% /notice %}}

## Reconexión

{{% notice style="tip" title="Siempre volver a partir de un dump completo" %}}
Ante cualquier corte, el cliente debe **vaciar por completo su
datastore** — ningún estado parcial puede considerarse fiable tras
una desconexión.

Lo que el cliente hace después (reintentar automáticamente N veces,
proponer al usuario que reconecte, abandonar...) es asunto
exclusivamente suyo: no está especificado por el protocolo, cada
cliente es libre de elegir su propia estrategia.

Pero una vez (re)conectado, volver a enviar `DU` para obtener un nuevo dump
completo es casi obligatorio: reconstruir un estado utilizable
a partir únicamente de las actualizaciones parciales que llegarían después
(`ee`/`ea`/`ev`) es imposible — solo dan una vista
fragmentaria, nunca un estado completo. Del lado del servidor, no hay nada que
preservar entre dos conexiones de un mismo cliente: es una
restricción puramente del lado del cliente.
{{% /notice %}}

## Ejemplo mínimo: primer intercambio

Sin framework, solo el primer intercambio (conexión → `DU` →
escucha) — extraído de `test-ws-advanced.js`, el script usado para el
health-check horario del servidor:

```javascript
const ws = new WebSocket("ws://127.0.0.1:9624");

ws.on("open", () => {
  ws.send(JSON.stringify({ "DU": { "language": "en" } }));
});

ws.on("message", (data) => {
  console.log("received:", data.toString());
});
```

## Ejemplo: lectura y escritura en OstErix

Extractos reales que ilustran la separación lectura/escritura del inicio
de esta página, una vez que se construye un cliente real (aquí OstErix, en Angular)
alrededor de este principio.

**Lectura** — `datastore.service.ts`, un simple acceso al estado interno:
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

**Escritura** — `websocket.service.ts`, sin ninguna mutación local, solo
el envío del mensaje:
```typescript
setPropertyOneElement(module: string, property: string, elements: { [key: string]: any }): void {
  this.send({
    SV: { m: { [module]: { p: { [property]: { e: elements } } } } }
  });
}
```
