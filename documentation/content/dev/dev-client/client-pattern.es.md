---
title: Principios de arquitectura del cliente
weight: 10
description: "Cómo estructurar un cliente OST: datastore único, escritura únicamente vía WS"
---

Esta página describe **cómo debe estructurarse un cliente**, no el formato
de los mensajes intercambiados (ver [Modelo de datos]({{% relref "structure.es.md" %}})
y las páginas siguientes para eso).

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

## Principio: ninguna dependencia de internet

Un servidor OST suele ejecutarse en una red local aislada — un observatorio remoto sin acceso a internet es un despliegue normal, no un caso extremo. Un cliente debe funcionar correctamente en esa situación: **cualquier recurso que necesite debe ir incluido con el propio cliente**, no obtenerse de un CDN u otro servicio externo en tiempo de ejecución.

Esto afecta directamente al renderizado de iconos: [`preicon1`/`preicon2`/`posticon1`/`posticon2`/`preicon`/`posticon`]({{% relref "structure.es.md" %}}) dan un *nombre* de icono de Google Fonts (Material Icons), no una URL — un cliente debe incluir localmente la fuente/los glifos de iconos (vendorizados, autoalojados, o empaquetados en la compilación) en vez de cargar la hoja de estilos del CDN de Google, que deja de funcionar silenciosamente en el momento en que no hay ruta hacia internet.

Lo mismo aplica a cualquier otra cosa que un cliente pudiera verse tentado a traer de un CDN: un framework JS, una librería de gráficos, una fuente web para la propia interfaz. Si el cliente lo necesita para funcionar, va incluido con el cliente.

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

## Renderizado: limitar las actualizaciones a la propiedad modificada

{{% notice style="warning" title="Obligatorio: nunca renderizar más allá de la propiedad modificada" %}}
Algunos módulos pueden enviar varias actualizaciones `ee`/`ea` **por
segundo** (por ejemplo, un módulo que consulta continuamente un
dispositivo). Un cliente que reacciona a cada actualización recibida
volviendo a renderizar toda su UI — o incluso solo todo el módulo
mostrado — se vuelve inutilizable bajo esa carga: pérdida del foco de
entrada, parpadeo y, en el peor de los casos, la UI deja de responder
por completo a la interacción.

Esto no es un caso límite a prevenir "por si acaso": es un
comportamiento real y actual del backend, y los clientes **deben**
construirse para soportarlo desde el principio:
1. En cada actualización, solo debe reemplazarse/mutarse el objeto
   JS/datos de la propiedad modificada. La referencia de cualquier
   otra propiedad debe permanecer exactamente igual mientras no esté
   afectada.
2. El renderizado de la UI debe estar indexado por la identidad de la
   propiedad (o del módulo) — una lista de propiedades debería
   reutilizar el mismo elemento de UI para la misma propiedad entre
   renderizados, sin destruirlo y recrearlo nunca, de forma que un
   campo no afectado nunca se vea alterado, pase lo que pase en otro
   lugar.
3. Limitar solo la **frecuencia** de renderizado (debounce/throttle)
   no es suficiente si el propio renderizado sigue reconstruyendo UI
   no afectada — la solución está en el **alcance** de lo que se
   reconstruye, no solo en su frecuencia.

Rooster (el frontend de producción) implementa este principio por
diseño: una actualización entrante solo reemplaza la referencia de la
propiedad afectada en el store, y cada lista de propiedades se
renderiza con una clave estable por propiedad. Como resultado, la UI
de las propiedades no afectadas nunca se ve perturbada, sin importar
la velocidad a la que otra propiedad se actualice en otro lugar.
{{% /notice %}}

## Plantillas: pantallas por tipo de módulo con repliegue genérico

Cada instancia de módulo puede anunciar `infos.template` (ver [Modelo
de datos]({{% relref "structure.es.md" %}})) — una cadena que nombra
su tipo (`"focus"`, `"guider"`, `"sequencer"`...). Es solo indicativo,
y **puede estar ausente**.

{{% notice style="tip" title="Plantilla ausente = la pantalla por defecto" %}}
Un cliente que gestiona pantallas dedicadas por plantilla debe tratar
un `infos.template` ausente (o no reconocido) exactamente como el
valor literal `"default"` — no como un error. Rooster procede así con
un pequeño registro que asocia un nombre de plantilla en minúsculas a
un componente de pantalla, con repliegue a su pantalla genérica/por
defecto cuando el nombre está ausente o no figura en el registro:
```
template = infos.template?.toLowerCase() || "default"
screen = registry[template] ?? registry["default"]
```
Esto es lo que permite a un cliente seguir siendo plenamente funcional
ante un tipo de módulo que nunca ha visto — un módulo nuevo o de
terceros siempre se muestra mediante la pantalla genérica — a la vez
que ofrece una pantalla cuidada y dedicada para los tipos de módulo
que merecen la inversión.

La pantalla `"default"` no es un repliegue mínimo simbólico: para
algunos tipos de módulo es la **única** pantalla que tendrán jamás, así
que debe renderizar todo lo que una propiedad/elemento genérico puede
llevar — estado, `hint`, pre/posticonos, cuadrículas, gráficos, LOV,
todo — no solo los valores. Una pantalla dedicada por plantilla puede
ser más selectiva (ocultar campos, reorganizar el diseño), pero la
pantalla por defecto no debería hacer desaparecer información
silenciosamente solo porque nadie ha escrito aún una pantalla dedicada
para ese módulo.
{{% /notice %}}

{{% notice style="tip" title="Recomendado: un único componente de renderizado de propiedad, reutilizado en todas partes" %}}
No obligatorio, pero muy recomendable: construir un componente
**único**, responsable de renderizar una propiedad (o un elemento) —
gestionando `directedit`, `permission`, los tres ámbitos de LOV, las
reglas de grupo bool, `autoupdate`, las cuadrículas, los gráficos — y
hacer que **todas** las pantallas lo usen para sus propiedades
individuales, incluidas las pantallas dedicadas por plantilla, no solo
el repliegue genérico.

La alternativa — cada pantalla reimplementando su propio renderizado
de campos — duplica cada matiz del formato de intercambio por toda la
base de código, y una corrección o una nueva capacidad del backend
solo llega a las pantallas que alguien recuerda actualizar. Las
pantallas dedicadas de Rooster (`GuiderComponent`, `FocusComponent`...)
tienen su propia disposición, pero delegan cada propiedad individual
al mismo componente de propiedad genérico que usa la pantalla de
repliegue — una corrección hecha una sola vez ahí (por ejemplo,
gestionar correctamente un grupo de interruptores exclusivos) se
aplica automáticamente en todas partes, pantallas dedicadas incluidas.
{{% /notice %}}

## Formularios de propiedad: `SA`, obligatorio para `directedit: false`

Más allá del `SV` campo a campo, una propiedad editable necesita un **camino de formulario**: una pantalla que liste todos sus elementos editables a la vez, enviados juntos vía [`SA`]({{% relref "client-messages.es.md" %}}#sa).

{{% notice style="warning" title="No opcional para un elemento con `directedit: false`" %}}
`SV` es rechazado por el backend para cualquier elemento con `directedit: false` (ver [Modelo de datos]({{% relref "structure.es.md" %}})). `SA` es la **única** forma de actualizar jamás ese elemento — un cliente sin camino de formulario simplemente no puede escribir en ese elemento, por bien construido que esté el resto de su interfaz.
{{% /notice %}}

Solo los elementos **editables** pertenecen a este formulario — los tipos de solo visualización (`img`, `video`, `light`, `prg`, `message`) se renderizan, nunca se envían; hay que excluirlos por completo del payload en lugar de enviarles un valor sin sentido.

La misma forma de "enviar un conjunto de elementos a la vez" se reutiliza en la creación de línea de cuadrícula ([`GC`]({{% relref "client-messages.es.md" %}}#gc)) y su actualización ([`GU`]({{% relref "client-messages.es.md" %}}#gu)) — ambas toman un mapa `e` exactamente igual que `SA`. Vale la pena construir **un único** componente de formulario reutilizable que sirva tanto al formulario de propiedad simple como al diálogo de creación de fila de cuadrícula y al de edición de fila — en lugar de tres implementaciones separadas de esencialmente la misma lógica ("renderizar elementos editables, recoger sus valores"). Es lo que hace Rooster.

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

## Ejemplo: lectura y escritura en Rooster

Extractos reales que ilustran la separación lectura/escritura del inicio
de esta página, una vez que se construye un cliente real (aquí Rooster, en Angular)
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
