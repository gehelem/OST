---
title: Modelo de datos
weight: 10
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

Al conectarse, enviar una petición `DU` (ver [Client → Server]({{< relref "client-messages.es.md" >}})).  
El servidor responde con un mensaje dump completo (`d`) que contiene todos los módulos, los datos del controlador y las listas de archivos.

---

## Módulos

Un módulo representa un subsistema funcional (enfoque, guiado, cámara allsky, etc.).  
Su identificador es su nombre interno (sin espacios).

| Campo | Tipo | Descripción |
|---|---|---|
| `infos.label` | string | Nombre mostrado del módulo |
| `infos.name` | string | Nombre interno del módulo |
| `infos.description` | string | Descripción del módulo |
| `infos.template` | string | Tipo de módulo (ej. `"focus"`, `"guider"`) |
| `properties` | object | Mapa nombre de propiedad → objeto propiedad |
| `globallovs` | object | Mapa clave LOV → objeto LOV (listas de valores a nivel módulo) |
| `profile.name` | string | Nombre del perfil activo |
| `profile.changed` | bool | Indica si el perfil tiene modificaciones no guardadas |

{{% notice style="info" title="Formato wire" %}}
En el formato JSON en la red, `properties` se abrevia como `p`, `globallovs` como `l`, y `profile` como `f`. Los clientes deben gestionar estas claves abreviadas.
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
| `hasGraph` | bool | La propiedad contiene un gráfico |
| `graphType` | string | `"XY"` · `"DY"` (eje temporal) · `"PHD"` (guiado) |
| `graphParams` | object | Configuración del gráfico |

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
| `directedit` | bool | `true` = enviar `SV` en cada modificación; `false` = agrupar con `SA` |
| `preicon` | string | Icono antes del elemento (opcional) |
| `posticon` | string | Icono después del elemento (opcional) |

{{% notice style="warning" title="Actualizaciones parciales" %}}
En los mensajes de actualización `ee` y `ea`, los elementos contienen **solo su valor** — sin los campos de metadatos. Los clientes deben conservar los metadatos del último dump completo.
{{% /notice %}}

{{% notice style="tip" title="Omisión de campo" %}}
`directedit` puede estar totalmente ausente en los elementos donde no se aplica — tratar una ausencia como `false`. Esto aligera los dumps de las propiedades que no lo necesitan.
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
| `stars` | int | Número de estrellas detectadas |
| `issolved` | bool | Indica si la resolución de campo tuvo éxito |
| `solverra` / `solverde` | float | Coordenadas AR/Dec resueltas |
| `solverorientation` | float | Ángulo de rotación del campo resuelto |
| `min` / `max` / `mean` / `median` / `stddev` | float[] | Estadísticas por canal (ver más abajo) |
| `histogram` | array | Datos de histograma por canal (ver más abajo) |
| `alternates` | string[] | Opcional: versiones alternativas de la imagen |

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

El LOV en sí se define a nivel módulo (`globallovs`) o controlador (`controllerlovs`):
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
