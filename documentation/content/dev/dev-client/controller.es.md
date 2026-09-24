---
title: Controlador, GlobalDatastore y archivos
weight: 30
description: "Lo que no es un módulo ordinario: datos del controlador, carga del sistema, configuraciones, módulo GlobalDatastore, sistema de archivos"
---

Esta página describe lo que se sale del esquema « módulos → propiedades → elementos » de las páginas anteriores: los **datos del controlador**, el módulo central **`GlobalDatastore`** y el **sistema de archivos** del servidor. Un cliente que se limite a mostrar los módulos no los necesita; un cliente que quiera ofrecer lo que propone una página de parámetros globales (configuraciones, drivers INDI, carga del servidor, equipamiento, imágenes) debe tratarlos.

## Los datos del controlador (`controllerdata`)

El controlador es el componente del servidor que carga los módulos y gestiona el servidor INDI embebido. Expone sus propios datos en el dump, bajo la clave `controllerdata` (ver [`d`]({{% relref "server-messages.es.md" %}}#d)), y luego los mantiene al día con mensajes [`uc`]({{% relref "server-messages.es.md" %}}#uc).

| Clave | Contenido |
|---|---|
| `availableconfs` | Configuraciones de módulos guardadas: objeto `nombre de configuración → lista de {modulelabel, moduletype, profilename}` |
| `currentconf` | Nombre de la configuración actual |
| `banner` | Banner del servidor (texto libre, opción `--banner`) |
| `git` | Versión del servidor: `Gitdate`, `Githash`, `Gitmessage`, `Gittag` |
| `libraries` | Bibliotecas de módulos disponibles, un objeto de metadatos por biblioteca |
| `profiles` | Perfiles disponibles: objeto `nombre de biblioteca transformado → lista de nombres de perfil` — ver la convención de claves más abajo |
| `indiserver` | `"Y"` si el servidor INDI embebido está activado, `"N"` en caso contrario |
| `indidrivers` | Catálogo de drivers INDI instalados: lista de `{binary, family, label, mdpd}` |
| `indiactivedrivers` | Drivers INDI actualmente en ejecución |
| `systemwatcher` | Carga del servidor, ver más abajo |
| `files`, `folders` | Copia de `d.files.files` y `d.files.folders`, ver [Sistema de archivos](#fichiers) |

{{% notice style="warning" title="Claves de `profiles`: ni el nombre de biblioteca tal cual, ni `infos.template`" %}}
`profiles` está indexado por el **nombre de biblioteca** del módulo (la misma cadena que se pasa como `lib` a [`ML`]({{% relref "client-messages.es.md" %}}#ml), es decir, la clave en `controllerdata.libraries` sin el prefijo `libost`) — pero transformada: **primera letra en mayúscula, el resto en minúsculas** (`"focus"` → `"Focus"`, no una simple capitalización — un nombre de biblioteca con otra mayúscula quedaría igualmente aplanado, por ejemplo un hipotético `"myFocus"` → `"Myfocus"`). Esta clave no es ni el nombre/etiqueta de la instancia del módulo, ni necesariamente idéntica a `infos.template` (suelen coincidir en la práctica, pero ambos se definen de forma independiente y el protocolo no garantiza nada al respecto).

Esta convención no está documentada en ningún otro sitio y el backend no la impone — se dedujo a partir de la propia lógica de búsqueda de Rooster. Dos clientes independientes construidos únicamente a partir de esta documentación tropezaron con ella (un selector de perfiles que se renderizaba vacío en silencio). Si necesitas buscar los perfiles disponibles de un módulo, reproduce exactamente esta transformación sobre el nombre de biblioteca en lugar de adivinar.
{{% /notice %}}

{{% notice style="warning" title="`uc`: una clave por mensaje, valor reemplazado por completo" %}}
Un mensaje `uc` lleva **una sola clave** de `controllerdata`, y su valor reemplaza **por completo** al anterior (sin fusión). Un cliente que solo lee la primera clave del payload (como hace Rooster) es por tanto correcto mientras el servidor envíe una sola clave por mensaje, que es lo que ocurre hoy.
{{% /notice %}}

### `systemwatcher`: carga del servidor

El servidor mide su propia carga a intervalos regulares y la difunde a todos los clientes. El intervalo se ajusta con la opción `--systemwatchinterval` (segundos, **10 por defecto**, `0` lo desactiva). El mensaje es un `uc` ordinario, y el último valor también está presente en `controllerdata.systemwatcher` del dump.

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

| Campo | Significado |
|---|---|
| `cpu.load_1m` / `load_5m` / `load_15m` | **Carga media** del sistema (como `/proc/loadavg`), **no un porcentaje de uso**: 1.0 = un núcleo saturado de media |
| `ram.used_mb` / `total_mb` | Memoria utilizada (total − disponible) y total, en MB |
| `disks[]` | Un elemento por dispositivo de almacenamiento montado (`/dev/*`, excepto `loop`): `device`, `mountpoint`, `used_gb`, `total_gb` |
| `network[]` | Un elemento por interfaz (excepto `lo`): caudales `rx_kbps` / `tx_kbps` medidos entre dos mensajes; la primera medida de una interfaz vale 0 |
| `temperatures[]` | Sensores de hardware del procesador: `type`, `temp_c`. **La lista puede estar vacía** según la máquina (solo se leen los sensores de tipo Intel `coretemp` y AMD `k10temp`) |

{{% notice style="tip" title="No suponer que los arrays están rellenos" %}}
`disks`, `network` y sobre todo `temperatures` pueden estar vacíos. Un cliente debe ocultar la sección correspondiente en lugar de mostrar un array vacío. El protocolo no da el número de núcleos del servidor: un indicador de CPU no puede por tanto ser un verdadero porcentaje, hay que mostrar el valor como una carga.
{{% /notice %}}

## Configuraciones y ciclo de vida de los módulos

Una **configuración** es un conjunto con nombre de módulos (biblioteca, etiqueta, perfil) que el servidor puede recargar de una vez. La lista está en `controllerdata.availableconfs`, la configuración activa en `controllerdata.currentconf`.

| Acción | Comando | Detalle |
|---|---|---|
| Cargar una configuración | [`CL`]({{% relref "client-messages.es.md" %}}#cl) | Carga todos los módulos de la configuración |
| Guardar la configuración actual | [`CS`]({{% relref "client-messages.es.md" %}}#cs) | Con el nombre indicado |
| Cargar un módulo | [`ML`]({{% relref "client-messages.es.md" %}}#ml) | Biblioteca, etiqueta, perfil opcional |
| Descargar un módulo | [`MK`]({{% relref "client-messages.es.md" %}}#mk) | Por nombre de módulo |

Los módulos efectivamente cargados son los del dump (`d.m`); aparecen y desaparecen mediante [`aa`]({{% relref "server-messages.es.md" %}}#aa) y [`dm`]({{% relref "server-messages.es.md" %}}#dm). La configuración **nunca** contiene `GlobalDatastore` (ver más abajo), que siempre está presente.

### Servidor INDI y drivers

Si `controllerdata.indiserver` vale `"Y"`, el servidor gestiona una instancia INDI embebida. Los comandos [`YA`/`YZ`/`YL`/`YR`/`YS`]({{% relref "client-messages.es.md" %}}#indi) la inician, la detienen y gestionan los drivers; `controllerdata.indidrivers` da el catálogo disponible y `controllerdata.indiactivedrivers` lo que está en marcha. `binary` es el valor que se pasa como `driver` en `YL`, `YR` e `YS`.

## El módulo `GlobalDatastore` {#globaldatastore}

`GlobalDatastore` es un módulo **como los demás en el dump** (`d.m.GlobalDatastore`, con propiedades, elementos, cuadrículas, perfiles), pero su papel es central:

- lo crea el servidor **antes que todos los demás módulos** y está **siempre presente**: `MK` sobre él se rechaza, y nunca se incluye en una configuración;
- lleva el **referencial compartido del equipamiento**, que consultan todos los demás módulos: ópticas, ubicaciones, equipamientos (conjuntos de dispositivos) y servidores INDI.

Sus cuadrículas sirven de fuente a listas de opciones (LOV) del mismo nombre, presentes en `d.lovs` (`optics`, `locations`, `equipments`, `servers`) y reconstruidas con cada modificación de la cuadrícula. Los demás módulos las usan así: elegir una óptica rellena la focal y el diámetro, elegir una ubicación rellena latitud, longitud y altitud, elegir un conjunto de equipamiento resuelve los dispositivos (cámara, montura, focuser, rueda de filtros, cámara de guiado, GPS, cúpula, meteo) y el servidor INDI al que conectarse.

Propiedades expuestas:

| Propiedad | Función |
|---|---|
| `devices` | Conjunto de equipamiento y GPS seleccionados (`equipments`, `gps`) |
| `devicesactions` | Botones: conectar/desconectar los dispositivos, cargar las configuraciones (`condevs`, `discondevs`, `loadconfs`) |
| `server` | Host y puerto del servidor INDI (`host`, `port`) |
| `serveractions` | Botones: conectar/desconectar el servidor INDI (`conserv`, `disconserv`) |
| `startup` | Casillas de inicio automático (`indiatstart`, `devatstart`, `confsatstart`) |
| `equipments` | Cuadrícula de los conjuntos de equipamiento |
| `locations` | Cuadrícula de las ubicaciones (iconos `refresh` = leer el GPS, `add` = añadir la fila actual) |
| `optics` | Cuadrícula de las ópticas |
| `servers` | Cuadrícula de los servidores INDI |

Todas sus propiedades se manipulan con los mensajes estándar (`SV`, `SA`, `GC`…, `I3`/`I4` sobre `locations`); un cliente genérico las muestra como cualquier otro módulo. Rooster las presenta en una pestaña dedicada de su página de parámetros, reteniendo las propiedades que tienen perfil (`hasprofile`) y agrupándolas por `level1`/`level2`.

## Sistema de archivos {#fichiers}

El servidor publica la lista de los archivos producidos (imágenes, secuencias, archivos) en la carpeta `--webroot`, sin que el cliente tenga que consultarla. Este mecanismo es independiente de los módulos.

### En el dump: `d.files`

```json
"files": {
  "folders": ["", "/Allsky", "/Sequencer/gam Cyg/LIGHT/Luminance"],
  "files": ["/image1.jpeg", "/rapport.txt"],
  "selectedfolder": "/home/gilles/ostmedia"
}
```

| Campo | Contenido |
|---|---|
| `folders` | **Todas** las subcarpetas del webroot, recursivamente, rutas relativas al webroot que empiezan por `/`. La raíz es la cadena vacía `""`. Sin ordenar. |
| `files` | Los archivos **de la sola carpeta seleccionada** (no recursivo), rutas relativas al webroot que empiezan por `/`. Todos los archivos, sin filtro de extensión. |
| `selectedfolder` | La carpeta seleccionada, como **ruta absoluta del sistema de archivos del servidor** (y no relativa). |

No hay paginación ni límite de tamaño: en un webroot muy lleno, la lista puede ser larga.

### Elegir la carpeta: `FS`

[`FS`]({{% relref "client-messages.es.md" %}}#fs) designa la carpeta a vigilar y de la que listar los archivos. El servidor solo vigila el **contenido** de una carpeta a la vez; el árbol de carpetas (`folders`) se sigue de forma independiente, ver más abajo.

{{% notice style="warning" title="La carpeta seleccionada es global en el servidor" %}}
No hay carpeta por cliente: un `FS` cambia la lista de archivos que ven **todos** los clientes conectados. Un cliente no debe usarlo para una simple navegación « local » si es posible que haya otros clientes conectados.

La ruta **se valida**: el destino debe existir y quedar dentro del webroot (sin escapar vía `../`). Una solicitud inválida se rechaza en el servidor (se registra, sin cambio de estado) — no devuelve silenciosamente una lista vacía.
{{% /notice %}}

Un `FS` correcto reemplaza la lista de archivos en bloque: el cliente recibe un único mensaje [`uc`]({{% relref "server-messages.es.md" %}}#uc) `files` con la lista completa de la nueva carpeta, no un flujo de `af`/`df` — seleccionar una carpeta no es una mutación del sistema de archivos, no se añadió ni eliminó realmente nada. **No** hay ningún mensaje que anuncie el nuevo `selectedfolder` en sí, que solo existe en el dump.

### Árbol de carpetas frente a archivos de la carpeta seleccionada

`folders` (el árbol recursivo completo) y `files` (el contenido de la carpeta seleccionada) no tienen el mismo alcance:

| | Alcance | Actualizaciones en vivo |
|---|---|---|
| `folders` / `ad` / `dd` | **Todo el webroot**, recursivamente — independiente de lo seleccionado | Siempre, sea cual sea la selección `FS` actual |
| `files` / `af` / `df` | **Solo la carpeta seleccionada** | Solo para cambios en la carpeta que seleccionó el último `FS` |

Una carpeta creada o eliminada en cualquier parte del webroot se notifica en vivo vía `ad`/`dd` aunque ningún cliente la tenga seleccionada. Un archivo creado o eliminado solo se notifica vía `af`/`df` si ocurre en la carpeta que el `FS` de un cliente seleccionó por última vez — `files`/`af`/`df` no dicen nada sobre el contenido de otras carpetas, un cliente tiene que hacer un `FS` a ellas para saberlo.

### Actualizaciones: `af` / `df` / `ad` / `dd`

Cuando el árbol de carpetas o el contenido de la carpeta seleccionada cambia realmente, el servidor envía un mensaje por cada ruta afectada — el valor es la ruta en sí, sin envolver en un objeto ni en un array:

```json
{ "af": "/Sequencer/gam Cyg/LIGHT/Luminance/img_001.FITS" }
{ "df": "/rapport.txt" }
{ "ad": "/Sequencer/nuevo objetivo" }
```

| Clave | Significado |
|---|---|
| `af` | Archivo aparecido en la carpeta seleccionada |
| `df` | Archivo desaparecido de la carpeta seleccionada |
| `ad` | Subcarpeta aparecida en cualquier parte del webroot |
| `dd` | Subcarpeta desaparecida de cualquier parte del webroot |

Cada mensaje lleva exactamente una ruta (N archivos = N mensajes) — ver [`af`/`df`/`ad`/`dd`]({{% relref "server-messages.es.md" %}}#af) para la referencia completa. Un renombrado se notifica como una eliminación seguida de una creación: el servidor compara una lista antes/después, no detecta los renombrados como tales.

Tras los mensajes por ruta, el servidor envía [`uc`]({{% relref "server-messages.es.md" %}}#uc) `files` y/o `folders` con la lista completa actualizada — pero solo para el que realmente cambió en esta pasada, nunca incondicionalmente. Un cliente puede por tanto aplicar los eventos por ruta de forma incremental, o simplemente reemplazar su(s) lista(s) con el/los mensaje(s) `uc` correspondiente(s).

### Construir la URL de un archivo

Las rutas de `files` son relativas al webroot y empiezan por `/`. La URL de un archivo es la URL base de los medios del servidor (`http(s)://hostname/ostmedia/`, ver la nota « URLs media » del [Modelo de datos]({{% relref "structure.es.md" %}})) seguida de la ruta. Cuidado con el `/` inicial: concatenarlo a una base que ya termina en `/` da un doble `//`.

### Lo que no hace el evento

El servidor detecta el **alta, la baja y el cambio de nombre** de archivos, no la modificación del contenido de un archivo existente: una imagen reescrita con el mismo nombre no produce ningún evento. Es el caso de las imágenes de previsualización de los módulos, cuya URL permanece idéntica en cada toma: un cliente debe añadir un parámetro de cache-busting (`?t=` + marca de tiempo) al recargar una imagen tras una actualización de su propiedad.
