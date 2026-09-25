---
title: Modelo de datos
weight: 20
description: "Jerarquía de módulos, propiedades y elementos intercambiados vía WebSocket"
---

## Visión general

Todos los datos se organizan según una jerarquía de tres niveles:

```
Module
  └─ Property (uno o varios por módulo)
       └─ Element (uno o varios por propiedad)
```

---

## Conexión y estado inicial

### Encontrar el servidor

Un cliente **web** es servido *por* el propio servidor OST, así que ya conoce su host — la URL WebSocket debe derivarse de la URL de la página, no pedirse al usuario:

| URL de la página | URL WebSocket |
|---|---|
| `http://<host>/` | `ws://<host>:9624` (directo, puerto fijo) |
| `https://<host>/` | `wss://<host>/ws/` (puerto 443 implícito, a través de una ruta de proxy inverso — las instalaciones SSL no exponen directamente el puerto 9624) |

Es una convención, no algo impuesto por el protocolo — pero todos los clientes existentes la siguen, y un cliente que en su lugar pide al usuario que escriba una URL WebSocket a mano resuelve un problema que no existe en el caso web.

Un cliente **no-web** (móvil, escritorio) no tiene una URL de página de la que derivar nada, y necesita que el host se le proporcione de otra forma:
- **mDNS/Zeroconf**: el servidor se anuncia en la red local con el tipo de servicio `_ostserver_ws._tcp` en el dominio `local`, con un registro TXT `OstServer=Observatoire Sans Tete`. El puerto anunciado es siempre `9624`. Esto permite a un cliente **escanear los servidores disponibles** en la red en lugar de exigir que el usuario conozca una IP — útil para una primera conexión, o siempre que pueda haber más de un servidor OST cerca.
- **Entrada manual, recordada**: ya sea descubierto o escrito, un cliente debería mantener un pequeño historial de servidores ya usados (host/puerto/seguro) para que reconectarse más tarde no requiera repetir el descubrimiento o la entrada. Esto es una comodidad del lado del cliente, no parte del protocolo.

En cualquier caso, la URL base de los **medios** (para imágenes/vídeos, ver la nota "URLs de medios" más abajo) sigue el mismo host, con el puerto HTTP(S) estándar (80/443) en lugar de 9624 — un cliente no-web también necesita este host, exactamente por la misma razón que necesita el host WebSocket.

Al conectarse, enviar una petición `DU` (ver [Client → Server]({{< relref "client-messages.es.md" >}})).  
El servidor responde con un mensaje dump completo (`d`) que contiene todos los módulos, los datos del controlador y las listas de archivos.

Los datos propios del controlador (carga del servidor, configuraciones de módulos, drivers INDI, sistema de archivos) no forman parte del modelo de módulos descrito en esta página — ver [Controlador, GlobalDatastore y archivos]({{% relref "controller.es.md" %}}).

---

## Módulos

Un módulo representa un subsistema funcional (enfoque, guiado, cámara allsky, etc.).  
Su identificador es su nombre interno (sin espacios).

| Campo | Tipo | Descripción |
|---|---|---|
| `infos.modulelabel` | string | Nombre mostrado del módulo |
| `infos.description` | string | Descripción del módulo |
| `infos.template` | string | Tipo de módulo (ej. `"focus"`, `"guider"`) — puede estar ausente |
| `properties` | object | Mapa nombre de propiedad → objeto propiedad |
| `globallovs` | object | Mapa clave LOV → objeto LOV (listas de valores a nivel módulo) |
| `profile.name` | string | Nombre del perfil activo |
| `profile.changed` | bool | Indica si el perfil tiene modificaciones no guardadas |

{{% notice style="info" title="Formato wire" %}}
En el formato JSON en la red, `properties` se abrevia como `p`, `globallovs` como `l`, y `profile` como `f`. Los clientes deben gestionar estas claves abreviadas.
{{% /notice %}}

{{% notice style="note" title="Campos internos" %}}
`infos` también incluye varios campos internos de compilación/versión (`baseGithash`, `thisversion`, `startdatetime`, y otros) — detalle de implementación, no forma parte del contrato estable del lado cliente, no documentado individualmente aquí.
{{% /notice %}}

---

## Propiedades

Una propiedad agrupa elementos relacionados. Las propiedades se muestran jerárquicamente mediante `level1` y `level2`.

| Campo | Tipo | Descripción |
|---|---|---|
| `label` | string | Etiqueta mostrada |
| `order` | string | Orden de clasificación en el mismo nivel |
| `level1` | string | Primer nivel jerárquico (ej. nombre de pestaña) |
| `level2` | string | Segundo nivel jerárquico (ej. nombre de grupo) |
| `status` | int | `0` Reposo · `1` OK (verde) · `2` Ocupado (amarillo) · `3` Error (rojo) |
| `permission` | int | `0` Solo lectura · `1` Solo escritura · `2` Lectura-escritura |
| `enabled` | bool | Indica si se puede interactuar con la propiedad |
| `badge` | bool | Indica si la propiedad está en la lista de favoritos |
| `preicon1` | string | Icono antes de la etiqueta (nombre de icono Google Fonts) |
| `preicon2` | string | Segundo icono antes de la etiqueta |
| `posticon1` | string | Icono después de la etiqueta |
| `posticon2` | string | Segundo icono después de la etiqueta |
| `showElts` | bool | Indica si los valores de los elementos deben mostrarse en línea |
| `hasprofile` | bool | Indica si la propiedad se guarda en los perfiles |
| `freevalue` | string | Campo de texto libre arbitrario |
| `rule` | int | Regla de agrupación de los elementos bool: `0` UnoDeEntre (radio) · `1` AloSumoUno · `2` Cualquiera |
| `elements` | object | Mapa nombre de elemento → objeto elemento |

{{% notice style="tip" title="`preicon`/`posticon`: acciones clicables, no decoraciones" %}}
Un valor no vacío de `preicon1`/`preicon2`/`posticon1`/`posticon2` es el nombre del icono de Google Fonts (Material Icons) a mostrar — y es clicable: hacer clic en él envía [`I1`/`I2`/`I3`/`I4`]({{% relref "client-messages.es.md" %}}#i1) sin valor ni parámetro, solo un payload vacío que apunta a la propiedad. Una **cadena vacía significa que no hay ningún icono** en ese lugar — nada que mostrar, nada que hacer clic, nada que enviar. Los `preicon`/`posticon` a nivel de elemento (más abajo) funcionan igual, vía [`J1`/`J2`]({{% relref "client-messages.es.md" %}}#j1). Ver [Acciones sobre los iconos]({{% relref "client-messages.es.md" %}}#icon-actions) para la referencia completa de mensajes.
{{% /notice %}}

{{% notice style="warning" title="`permission` rige los elementos editables, no si se muestra el valor" %}}
`permission: 1` ("solo escritura") no significa "ocultar el valor actual" — significa que la propiedad no tiene nada significativo que leer de vuelta como entrada enviable (una acción de disparar y olvidar). Una propiedad cuyo único elemento es de un tipo solo de visualización (`img`, `video`, `light`, `prg`, `message`) puede legítimamente tener `permission: 1` — una vista previa en directo de cámara es un ejemplo real — y debe seguir renderizándose a partir de su valor sin importar `permission`. Condicionar la interactividad (mostrar una entrada, habilitar un envío) a `permission`, pero condicionar el renderizado al **tipo** del elemento, no a `permission`.
{{% /notice %}}

{{% notice style="note" title="`rule` solo tiene sentido cuando todos los elementos son `bool`" %}}
`rule` está presente en todas las propiedades (por defecto `0`), pero solo tiene sentido — y solo debería actuarse en consecuencia — cuando **todos** los elementos de la propiedad son de tipo `bool`. También aparece, de forma inerte, en propiedades cuyos elementos son una mezcla de otros tipos (ej. un par de coordenadas de dos elementos `float`) — un cliente debería ignorarlo ahí en lugar de intentar renderizar, por ejemplo, dos floats como un grupo de radio.
{{% /notice %}}

{{% notice style="note" title="`badge` lo fija el desarrollador del módulo, no un cliente en tiempo de ejecución" %}}
No existe ningún mensaje de cliente que active o desactive `badge` — está fijado por propiedad/por elemento en la propia definición del módulo, no es un favorito por usuario o por sesión que un cliente pueda establecer. Una pantalla de "favoritos" puede leer y agregar lo que ya está marcado, pero no tiene forma de dejar que un usuario marque algo como favorito por sí mismo.
{{% /notice %}}

{{% notice style="tip" title="`level1`/`level2`/`order` son informativos, no un contrato" %}}
Estos campos son puramente informativos — el backend nunca los impone, y un cliente es libre de respetarlos parcialmente o no respetarlos en absoluto. Solo expresan cómo el desarrollador de un módulo imaginó organizar sus propias propiedades. Esto es exactamente lo que permite a clientes genéricos (Rooster, ostnoctua) adaptarse a cualquier módulo sin una UI dedicada por módulo.

El cliente de referencia (Rooster) agrupa las propiedades así, para un cliente que busque un diseño genérico similar:
1. `level1` vacío → propiedad mostrada fuera de cualquier pestaña (renderizada directamente, ordenada por `order`).
2. `level1` con valor → agrupada en una pestaña llamada `level1`. Dentro de ella, `level2` vacío → mostrada directamente bajo la pestaña; `level2` con valor → dentro de un grupo plegable llamado `level2`.
3. El orden de las pestañas (y de los grupos dentro de una pestaña) no es un campo aparte — se deduce del valor `order` más pequeño entre las propiedades de ese grupo.
4. Dentro de un grupo, las propiedades se ordenan por su propio `order`.

Dos cosas a tener en cuenta si se construye algo similar:
- `level1`/`level2` forman una "falsa jerarquía" por decisión de diseño, no una estructura de dos niveles garantizada — nada impide que un módulo deje `level1` vacío mientras asigna `level2` (Rooster ni siquiera gestiona esa combinación de forma especial; la propiedad simplemente cae en el grupo "sin level1", ignorando `level2`).
- `level1`/`level2` son etiquetas traducidas, que pasan por el mismo mecanismo que `label`/`hint`. Usar el valor bruto de la cadena como clave de estado de la interfaz (ej. un panel expandido/plegado) es frágil ante cambios de idioma del cliente — no es un error, solo algo a evitar.
{{% /notice %}}

**Propiedades con cuadrícula** (cuando `hasGrid: true`):

| Campo | Tipo | Descripción |
|---|---|---|
| `hasGrid` | bool | La propiedad contiene datos tabulares |
| `showGrid` | bool | La cuadrícula debe mostrarse por defecto |
| `gridLimit` | int | Número máximo de filas en la cuadrícula |
| `gridheaders` | string[] | Orden de las columnas (nombres de los elementos) |
| `grid` | array de arrays | Filas de la cuadrícula; cada fila es un array de valores en el orden de `gridheaders` |

**Propiedades con gráfico** (cuando `hasGraph: true`):

| Campo | Tipo | Descripción |
|---|---|---|
| `hasGraph` | bool | La propiedad contiene un gráfico — **siempre presente**, incluso cuando es `false` |
| `graphType` | string | `"XY"` · `"DY"` · `"SXY"` · `"SDY"` · `"PHD"` — ver abajo |
| `graphParams` | object | Qué elemento aporta cada rol del gráfico — ver abajo |

Una propiedad con gráfico es siempre también una propiedad de cuadrícula (`hasGrid: true`, ver arriba): cada fila de la cuadrícula es un punto de datos, y `graphParams` asocia un rol abstracto del gráfico a la **clave del elemento** (uno de `gridheaders`) cuyo valor, en esa fila, aporta ese rol.

| `graphType` | Forma | Roles de `graphParams` |
|---|---|---|
| `XY` | Una sola serie de dispersión, límites de eje fijos | `X`, `Y`, `Xmin`, `Xmax`, `Ymin`, `Ymax`, además opcionalmente `graphColor` (abajo) |
| `DY` | Una sola serie, fecha/hora en X | `D` (elemento fecha/hora), `Y`, además opcionalmente `graphColor` (abajo) |
| `SXY` | Varias series de dispersión, agrupadas por una clave de serie | `S` (elemento nombre de serie), `X`, `Y`, además opcionalmente `graphColors`/`axis`/`mapping` (abajo) |
| `SDY` | Varias series, fecha/hora en X, agrupadas por una clave de serie | `S`, `D`, `Y`, además opcionalmente `graphColors`/`axis`/`mapping` |
| `PHD` | Gráfico de guiado dedicado estilo PHD2 (deriva + SNR + RMS) | `D`, `RA`, `DE`, `pRA`, `pDE`, `RSB`, `RMS` — roles fijos, específicos de este uso |

Campos opcionales de `graphParams`:

| Campo | Usado por | Descripción |
|---|---|---|
| `graphColor` | `XY`, `DY` | `{"R": int, "G": int, "B": int}` — color para la serie única |
| `graphColors` | `SXY`, `SDY` | `{"<serie>": {"R": int, "G": int, "B": int}}` — color por serie |
| `axis` | `SXY`, `SDY` | `{"<nombre>": {"min": number, "max": number, "pos": "left"\|"right", "text": string}}` — uno o más ejes Y |
| `mapping` | `SXY`, `SDY` | `{"<serie>": "<nombre de eje>"}` — sobre qué entrada de `axis` se traza cada serie |

{{% notice style="warning" title="`PHD` no es un tipo de gráfico genérico" %}}
A diferencia de `XY`/`DY`/`SXY`/`SDY`, cuyos roles son suficientemente genéricos como para impulsar un único componente de gráfico reutilizable, `PHD` tiene roles fijos y específicos para un gráfico de guiado estilo PHD2 (usado por el módulo `guider`). Un cliente que construya un componente de gráfico genérico para los otros cuatro tipos probablemente necesitará una implementación dedicada para `PHD`, o tendrá que omitirlo — es lo que hace actualmente Rooster (solo `SXY`/`SDY`/`XY`/`DY`, `PHD` sin implementar).
{{% /notice %}}

{{% notice style="tip" title="Renderizar los gráficos como componentes vivos, no como imágenes generadas" %}}
Los datos de cuadrícula de una propiedad con gráfico cambian con el tiempo (`gc`/`gu`/`gd`/`gr`, o un `uc`/`ap`/`aa` nuevo) — así que hay que tratarlo como cualquier otro elemento de UI vivo: dibujarlo con un componente de gráfico/canvas que se redibuje en cada actualización, tal como hace Rooster con Chart.js. No renderizarlo en el servidor o en el cliente como una imagen estática (una captura PNG, un gráfico renderizado por el servidor) para luego simplemente sustituir el `<img>` — se ve visiblemente peor que un gráfico real (sin actualizaciones fluidas, sin interactividad, parpadeo/recarga visible en cada punto), y tira por la borda todo el sentido de haber recibido los datos de cuadrícula en vivo por el cable.
{{% /notice %}}

{{% notice style="info" title="Formato wire" %}}
En el formato JSON en la red, `elements` se abrevia como `e`.
{{% /notice %}}

---

## Elementos

Un elemento contiene un valor tipado único. Existen 11 tipos de elementos.

### Campos comunes (todos los elementos, en el dump completo)

| Campo | Tipo | Descripción |
|---|---|---|
| `type` | string | Tipo del elemento (ver lista abajo) |
| `label` | string | Etiqueta mostrada |
| `order` | string | Orden de clasificación dentro de la propiedad |
| `hint` | string | Texto de ayuda / tooltip |
| `autoupdate` | bool | Indicador interno del backend; ignorado por los clientes |
| `badge` | bool | Indica si el elemento está en la lista de favoritos |
| `directedit` | bool | Indica si este elemento puede escribirse solo mediante `SV` — ver más abajo |
| `preicon` | string | Icono antes del elemento — acción clicable, ver nota arriba (opcional) |
| `posticon` | string | Icono después del elemento — acción clicable, ver nota arriba (opcional) |

{{% notice style="warning" title="Actualizaciones parciales" %}}
En los mensajes de actualización `ee` y `ea`, los elementos contienen **solo su valor** — sin los campos de metadatos. Los clientes deben conservar los metadatos del último dump completo.
{{% /notice %}}

{{% notice style="tip" title="Omisión de campo" %}}
`directedit` puede estar totalmente ausente en los elementos donde no se aplica — tratar una ausencia como `false`. Esto aligera los dumps de las propiedades que no lo necesitan.
{{% /notice %}}

{{% notice style="note" title="Significado de `directedit`" %}}
`true` significa que este elemento puede actualizarse por sí solo, independientemente de los demás elementos de la propiedad — enviar `SV` para él (`SA` también funciona). `false` significa que el cliente debe enviar **todos** los elementos de la propiedad juntos — usar siempre `SA`, nunca `SV` para ese elemento solo. Un ejemplo significativo: coordenadas RA/Dec, donde ambos valores deben darse juntos.

El backend lo impone: un `SV` sobre un elemento con `directedit: false` se rechaza con un error en el log del módulo y no se aplica nada. `SA` siempre se acepta.

Elegir el valor según el comportamiento del elemento, no según si «parece relacionado» con sus vecinos:
- **`true` para un elemento independiente** — un `bool` que puede ser verdadero con independencia de sus hermanos (botón de acción como start/abort/loop, interruptor) debe tener `directedit: true`. Un valor así suele reflejar una actividad en curso, y varios pueden ser verdaderos a la vez (p. ej. un bucle de captura en marcha mientras se pide un focus out). Declararlo `false` afirmaría por error que los elementos deben enviarse en grupo, y un cliente que siga esta página enviaría `SA`, saltándose los módulos que solo tratan `SV`.
- **`false` solo para un grupo real** — elementos que solo tienen sentido juntos: coordenadas RA/Dec, o un vector de switches INDI reproducido uno a uno.

Para un grupo `false` de elementos `bool` con regla exclusiva (`OneOfMany` o `AtMostOne`), seleccionar un elemento significa enviarle `true` y **todos sus hermanos `false`** en el mismo `SA` — nunca reenviar los valores actuales de los hermanos, o dos elementos acabarán en `true`.
{{% /notice %}}

---

### Tipo: `int`

| Campo | Tipo | Descripción |
|---|---|---|
| `value` | int | Valor entero actual |
| `min` | int | Valor mínimo permitido |
| `max` | int | Valor máximo permitido |
| `step` | int | Paso de incremento |
| `format` | string | Cadena de formato de visualización |
| `slider` | int | `0` sin slider · `1` solo slider · `2` slider + entrada de valor |
| `listOfValues` | object | Opcional: mapa `{"clave": "Etiqueta"}` de los valores permitidos |
| `globallov` | string | Opcional: clave que referencia un LOV de módulo o de controlador |
| `lovScope` | string | `"module"` o `"controller"` — dónde se define el globallov |
| `lovConstrained` | bool | Indica si el valor debe pertenecer al LOV |
| `nullable` | bool | Indica si el elemento puede dejarse vacío (el frontend debe ofrecer una opción "ninguno") |

### Tipo: `float`

Mismos campos que `int`, con `value`, `min`, `max`, `step` en coma flotante.

### Tipo: `bool`

| Campo | Tipo | Descripción |
|---|---|---|
| `value` | bool | `true` o `false` |

### Tipo: `string`

| Campo | Tipo | Descripción |
|---|---|---|
| `value` | string | Valor de texto |
| `listOfValues` | object | Opcional: mapa `{"clave": "Etiqueta"}` de los valores permitidos |
| `globallov` | string | Opcional: clave que referencia un LOV |
| `lovScope` | string | `"module"` o `"controller"` |
| `lovConstrained` | bool | Indica si el valor debe pertenecer al LOV |
| `nullable` | bool | Indica si el elemento puede dejarse vacío (el frontend debe ofrecer una opción "ninguno") |

### Tipo: `date`

| Campo | Tipo | Descripción |
|---|---|---|
| `value` | object | `{"year": int, "month": int (1-12), "day": int (1-31)}` |

### Tipo: `time`

| Campo | Tipo | Descripción |
|---|---|---|
| `value` | object | `{"hh": int, "mm": int, "ss": int, "ms": int}` |
| `usems` | bool | Indica si deben mostrarse los milisegundos |

### Tipo: `datetime`

| Campo | Tipo | Descripción |
|---|---|---|
| `value` | object | `{"year": int, "month": int, "day": int, "hh": int, "mm": int, "ss": int, "ms": int}` |

### Tipo: `img`

El campo `value` es un objeto que contiene los metadatos de la imagen:

| Campo | Tipo | Descripción |
|---|---|---|
| `urljpeg` | string | Ruta relativa a la imagen JPEG (a usar con la URL base `/ostmedia/`) |
| `urlfits` | string | Ruta relativa al archivo FITS |
| `urlthumbnail` | string | Ruta relativa a la miniatura |
| `urloverlay` | string | Ruta relativa a la imagen de superposición |
| `channels` | int | Número de canales de color |
| `width` / `height` | int | Dimensiones de la imagen en píxeles |
| `snr` | float | Relación señal/ruido |
| `hfravg` | float | HFR medio (radio a mitad de flujo) de las estrellas detectadas |
| `hfravgdev` | float | Desviación estándar de `hfravg` |
| `stars` | int | Número de estrellas detectadas |
| `issolved` | bool | Indica si la resolución de campo tuvo éxito |
| `solverra` / `solverde` | float | Coordenadas AR/Dec resueltas |
| `solverorientation` | float | Ángulo de rotación del campo resuelto |
| `min` / `max` / `mean` / `median` / `stddev` | float[] | Estadísticas por canal (ver más abajo) |
| `histogram` | array | Datos de histograma por canal (ver más abajo) |
| `alternates` | string[] | Opcional: versiones alternativas de la imagen |
| `showstats` | bool | Fijado por el desarrollador del módulo: indica si los campos de estadísticas anteriores son significativos para esta imagen y deben mostrarse (un módulo puede fijarlo en `false` si no los calcula, o si no aplican) |

**Estadísticas por canal**: `min`/`max`/`mean`/`median`/`stddev` son
arrays de **3 posiciones fijas**, sea cual sea el número real de
canales — índice `0` = canal único (imagen mono) o rojo (R), `1` =
verde (G), `2` = azul (B). Solo las primeras `channels` posiciones son
significativas; más allá, los valores son cero y no tienen
significado (una imagen mono solo tiene el índice `0` con datos).

{{% notice style="note" title="Histograma: estructura de los bins" %}}
`histogram` sigue la misma lógica por canal (un array por canal, solo
se usan las primeras `channels` posiciones), pero con dos
particularidades a conocer antes de usarlo:

- **Cada bin está a su vez envuelto en un array de un elemento**:
  `histogram[canal][bin]` vale `[frecuencia]`, no directamente
  `frecuencia` — un artefacto de la serialización actual del backend,
  no un error de lectura del lado del cliente.
- **El valor de píxel que representa cada bin no se transmite
  directamente**, solo la frecuencia. El número de bins varía según la
  imagen (`histogram[canal].length`); el valor de píxel del bin `i` se
  recalcula a partir de `min`/`max` de ese mismo canal:
  ```
  ancho_bin = (max[canal] - min[canal]) / (número_de_bins - 1)
  valor(i)  = min[canal] + i × ancho_bin
  ```
{{% /notice %}}

{{% notice style="tip" title="URLs de medios" %}}
Todas las rutas de imagen/vídeo son relativas. Añadir como prefijo `http(s)://hostname/ostmedia/` para obtener la URL completa.  
Ejemplo: `"urljpeg": "Focus/frame.jpeg"` → `http://hostname/ostmedia/Focus/frame.jpeg`
{{% /notice %}}

{{% notice style="warning" title="Los medios se sirven por separado del puerto WebSocket" %}}
Ese `hostname` **no se puede deducir del protocolo** — ningún campo del dump lo proporciona, y no es el host de la conexión WebSocket que se acaba de establecer. En el despliegue de referencia (ver [Instalación limpia]({{% relref "fresh.es.md" %}})), los medios se sirven mediante un servidor web separado (nginx, puerto 80 por defecto, 443 detrás de SSL) — un puerto completamente distinto del servidor WebSocket (9624). Un cliente debe conocer de antemano, o dejar que el usuario configure, el hostname del servidor OST de forma independiente de la URL WebSocket, y construir las URLs de medios usando el puerto HTTP de ese hostname — no el origen de sus propios ficheros, ni el puerto WebSocket. Un cliente que simplemente antepone una ruta `/ostmedia/` desnuda (resuelta respecto a dónde se sirven sus propios ficheros) obtendrá imágenes rotas.
{{% /notice %}}

### Tipo: `video`

| Campo | Tipo | Descripción |
|---|---|---|
| `value` | object | `{"url": "ruta/relativa/al/video.mp4"}` |

En los mensajes de actualización parcial (`ea`), el elemento de vídeo puede contener `url` directamente al nivel del elemento en lugar de anidado en `value`.

### Tipo: `light`

| Campo | Tipo | Descripción |
|---|---|---|
| `value` | int | `0` Reposo · `1` OK · `2` Advertencia · `3` Error |

### Tipo: `prg`

| Campo | Tipo | Descripción |
|---|---|---|
| `value` | object | `{"value": float (0–100), "dynlabel": string}` |
| `prgtype` | string | `"bar"` o `"spinner"` |

### Tipo: `message` (aún no implementado en el backend actual)

Reservado para la visualización de mensajes/logs.

---

## Listas de valores (LOV)

Los LOV proporcionan un conjunto de valores permitidos para los elementos de tipo `int`, `float` y `string`.

**LOV local** — integrada en el elemento:
```json
"listOfValues": {
  "1": "Opción Uno",
  "2": "Opción Dos"
}
```

**LOV global** — referenciada por clave:
```json
"globallov": "myCameraModels",
"lovScope": "module"
```

El LOV en sí se define a nivel módulo (`globallovs`) o controlador (`lovs`):
```json
"myCameraModels": {
  "label": "Modelos de cámara",
  "type": "string",
  "values": {
    "ASI294MC": "ZWO ASI 294 MC",
    "ASI533MC": "ZWO ASI 533 MC"
  }
}
```

**LOV integrados del controlador** (siempre disponibles):

| Clave | Contenido |
|---|---|
| `loadedModules` | Todos los módulos cargados: `{nombreModulo: etiqueta}` |
| `loadedModules-<template>` | Módulos filtrados por tipo (ej. `loadedModules-focus`) |
| `profiles-<template>` | Perfiles disponibles para un tipo de módulo |

---

## Control de acceso

La respuesta dump contiene los valores `grant-server` y `grant-client`:

| Valor | Significado |
|---|---|
| `"1"` | Acceso completo lectura-escritura |
| `"0"` | Solo lectura |
| `"-1"` | Acceso denegado (autenticación requerida) |

Si `grant-client` vale `"-1"`, no se envía ningún dato de módulo. Usar el mensaje `LO` (login) para autenticarse.
