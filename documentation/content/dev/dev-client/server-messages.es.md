---
title: Mensajes Servidor → Cliente
weight: 20
description: "Todos los mensajes JSON enviados por el servidor OST a los clientes conectados"
---

## Formato de los mensajes

Cada mensaje es un objeto JSON con **una única clave de primer nivel**. Esta clave codifica el tipo de evento y puede incluir una descripción legible separada por un guion:

```
"aa-dump all data"   →  tipo de evento = 2 primeros caracteres = "aa"
"ea"                 →  tipo de evento = "ea"
"d"                  →  tipo de evento = "d" (carácter único)
```

Los clientes deben comparar los **1 o 2 primeros caracteres** de la clave, no la cadena completa, ya que la parte de descripción puede cambiar.

Todas las claves de eventos enviadas por el servidor están en **minúsculas** (ej. `d`, `ea`, `ap`, `gc`).

{{% notice style="warning" title="Excepción: `evt` empieza por `ev`" %}}
La clave [`evt`](#evt) (eventos de archivos) empieza por los mismos dos caracteres que [`ev`](#ev). Comparar primero la igualdad exacta con `evt` y solo después los prefijos; de lo contrario, un evento de archivo se toma por una actualización de elemento.
{{% /notice %}}

---

## Índice de tipos de eventos

| Clave | Descripción |
|---|---|
| [`d`](#d) | Dump inicial completo (respuesta a `DU`) |
| [`aa`](#aa) | Dump completo de un módulo (módulo cargado/recargado) |
| [`ap`](#ap) | Reemplazo completo de una propiedad |
| [`ea`](#ea) | Actualización de todos los valores de elementos de una propiedad |
| [`ee`](#ee) | Actualización de un solo valor de elemento |
| [`ev`](#ev) | Actualización de un valor de elemento con sus restricciones |
| [`ps`](#ps) | Actualización del estado y del estado activado de una propiedad |
| [`dm`](#dm) | Módulo eliminado |
| [`dp`](#dp) | Propiedad eliminada |
| [`gc`](#gc) | Línea de cuadrícula creada |
| [`gu`](#gu) | Línea de cuadrícula actualizada |
| [`gd`](#gd) | Línea de cuadrícula eliminada |
| [`gr`](#gr) | Reinicio de la cuadrícula (todas las líneas borradas) |
| [`lc`](#lc) | LOV creado o fusionado |
| [`lu`](#lc) | LOV reemplazado |
| [`ld`](#lc) | LOV eliminado |
| [`fs`](#fs) | Perfil guardado |
| [`fl`](#fs) | Perfil cargado |
| [`fc`](#fs) | Perfil modificado (modificaciones no guardadas) |
| [`l`](#l) | Entrada de log |
| [`uc`](#uc) | Actualización de los datos del controlador |
| [`evt`](#evt) | Evento de archivo o de carpeta (alta/baja) |
| [`xx`](#xx) | Pong keepalive |

---

## `d` — Dump inicial completo {#d}

Enviado como respuesta a una petición `DU`. Contiene todo: todos los módulos, los logs, las listas de archivos y los datos del controlador. El detalle de las claves de `controllerdata` y de `files` está en [Controlador, GlobalDatastore y archivos]({{% relref "controller.es.md" %}}); el ejemplo siguiente solo muestra algunas.

```json
{
  "d": {
    "grant-client": "1",
    "grant-server": "0",
    "serverlng": "en",
    "m": {
      "Focus": {
        "infos": { "label": "Focus", "name": "Focus", "description": "..." },
        "f": { "name": "default", "changed": false },
        "l": {
          "myLov": { "label": "My LOV", "type": "string", "values": { "a": "Option A" } }
        },
        "p": {
          "parameters": {
            "label": "Parameters",
            "order": "10",
            "level1": "Focus",
            "level2": "Config",
            "status": 0,
            "permission": 2,
            "enabled": true,
            "showElts": true,
            "hasGrid": false,
            "hasGraph": false,
            "rule": 0,
            "badge": false,
            "preicon1": "", "preicon2": "", "posticon1": "", "posticon2": "",
            "hasprofile": true,
            "freevalue": "",
            "e": {
              "iterations": {
                "type": "int",
                "label": "Iterations",
                "order": "10",
                "hint": "",
                "autoupdate": false,
                "badge": false,
                "directedit": false,
                "value": 5,
                "min": 1,
                "max": 20,
                "step": 1,
                "format": "",
                "slider": 0
              }
            }
          }
        }
      }
    },
    "files": {
      "folders": ["", "/Allsky", "/Allsky/archives"],
      "files": ["/image.jpeg"],
      "selectedfolder": "/home/user/ostmedia"
    },
    "logs": [
      { "d": "2024-01-01T12:00:00.000", "c": "Focus", "t": "Module loaded", "l": 1 }
    ],
    "controllerdata": {
      "profiles": {
        "Focus": ["default", "test"]
      },
      "currentconf": "default"
    },
    "lovs": {
      "sharedLov": { "label": "Shared LOV", "type": "string", "values": {} }
    }
  }
}
```

{{% notice style="warning" title="Abreviaturas del formato wire" %}}
`m` = modules, `p` = properties, `e` = elements, `l` = globallovs, `f` = profile.
Normalizar estas abreviaturas a sus nombres completos al almacenar internamente.
{{% /notice %}}

---

## `aa` — Dump completo de un módulo (módulo cargado/recargado) {#aa}

Enviado cuando un módulo se carga o recarga en tiempo de ejecución. Contiene el estado completo del módulo. La clave empieza por `"aa"`.

```json
{
  "aa-dump all data": {
    "Guider": {
      "infos": { "label": "Guider", "name": "Guider", "description": "..." },
      "f": { "name": "default", "changed": false },
      "l": {},
      "p": {
        "status": { ... }
      }
    }
  }
}
```

---

## `ap` — Reemplazo completo de una propiedad {#ap}

Reemplaza por completo una o varias propiedades. La clave empieza por `"ap"`.

```json
{
  "ap-dump all property data": {
    "Focus": {
      "p": {
        "parameters": {
          "label": "Parameters",
          "status": 0,
          "permission": 2,
          "enabled": true,
          "showElts": true,
          "hasGrid": false,
          "hasGraph": false,
          "rule": 0,
          "e": {
            "iterations": {
              "type": "int",
              "value": 7,
              "min": 1, "max": 20, "step": 1,
              "label": "Iterations", "order": "10", "hint": "",
              "autoupdate": false, "badge": false, "directedit": false,
              "format": "", "slider": 0
            }
          }
        }
      }
    }
  }
}
```

---

## `ea` — Actualización de todos los valores de elementos (nivel propiedad) {#ea}

El mensaje de actualización más común. Envía los valores actuales de todos los elementos de una propiedad. Solo contiene los valores — sin metadatos. La clave es exactamente `"ea"`.

```json
{
  "ea": {
    "Allsky": {
      "p": {
        "coming": {
          "e": {
            "sunrise": { "hh": 7, "mm": 6, "ss": 51 },
            "sunset":  { "hh": 21, "mm": 15, "ss": 1 }
          }
        }
      }
    }
  }
}
```

Para los tipos simples (`int`, `float`, `bool`, `string`, `light`), el valor es un escalar:
```json
{
  "ea": {
    "Focus": {
      "p": {
        "parameters": {
          "e": {
            "iterations": 5,
            "exposure": 3.5,
            "active": true
          }
        }
      }
    }
  }
}
```

---

## `ee` — Actualización de un solo valor de elemento {#ee}

Actualiza el valor de un solo elemento. Mismo formato que `ea` pero para un solo elemento. La clave es exactamente `"ee"`.

```json
{
  "ee": {
    "Focus": {
      "p": {
        "parameters": {
          "e": {
            "iterations": 7
          }
        }
      }
    }
  }
}
```

{{% notice style="note" title="`ee` solo tiene origen en el backend" %}}
`ee` solo se envía por iniciativa propia del backend (ej. un cambio de valor originado por un dispositivo, un sondeo interno) — nunca como respuesta directa a una escritura del cliente. Cualquier respuesta a un `SV` o `SA` iniciado por el cliente es siempre un `ea` de toda la propiedad, sin importar cuántos elementos hayan cambiado realmente.
{{% /notice %}}

---

## `ev` — Actualización de un elemento con metadatos {#ev}

Actualiza el valor de un elemento y sus restricciones (min, max, format). La clave empieza por `"ev"`.

```json
{
  "ev-set one element value/min/max/format ": {
    "Focus": {
      "p": {
        "parameters": {
          "e": {
            "temperature": {
              "value": 12.5,
              "min": -40.0,
              "max": 80.0,
              "format": "%.1f"
            }
          }
        }
      }
    }
  }
}
```

---

## `ps` — Actualización del estado de una propiedad {#ps}

Actualiza únicamente los campos `status` y `enabled` de una propiedad. La clave empieza por `"ps"`.

```json
{
  "ps-only property state": {
    "Focus": {
      "p": {
        "autofocus": {
          "status": 2,
          "enabled": true
        }
      }
    }
  }
}
```

Valores de estado: `0` Reposo · `1` OK · `2` Ocupado · `3` Error

---

## `dm` — Módulo eliminado {#dm}

Enviado cuando un módulo se descarga. El cliente debe eliminar todos los datos de ese módulo. La clave empieza por `"dm"`.

```json
{
  "dm-delete/remove module": {
    "Focus": {}
  }
}
```

---

## `dp` — Propiedad eliminada {#dp}

Enviado cuando una propiedad se elimina de un módulo. La clave empieza por `"dp"`.

```json
{
  "dp-delete/remove property": {
    "Focus": {
      "p": {
        "oldProperty": ""
      }
    }
  }
}
```

---

## `gc` — Línea de cuadrícula creada {#gc}

Enviado tras añadir una nueva línea en una propiedad de tipo cuadrícula. La clave empieza por `"gc"`.

```json
{
  "gc-grid new line ": {
    "Sequencer": {
      "p": {
        "sequence": {
          "i": 3,
          "values": {
            "target": "M31",
            "exposure": 120,
            "count": 10
          }
        }
      }
    }
  }
}
```

`i` es el índice de la línea recién creada. `values` es un objeto indexado por nombre de elemento.

---

## `gu` — Línea de cuadrícula actualizada {#gu}

Enviado cuando se modifica una línea existente de la cuadrícula. La clave empieza por `"gu"`.

```json
{
  "gu-grid update line": {
    "Sequencer": {
      "p": {
        "sequence": {
          "i": 1,
          "values": {
            "target": "M42",
            "exposure": 60,
            "count": 20
          }
        }
      }
    }
  }
}
```

---

## `gd` — Línea de cuadrícula eliminada {#gd}

Enviado cuando se elimina una línea de la cuadrícula. La clave empieza por `"gd"`.

```json
{
  "gd-grid delete line": {
    "Sequencer": {
      "p": {
        "sequence": {
          "i": 0
        }
      }
    }
  }
}
```

`i` es el índice (base cero) de la línea eliminada. Las líneas siguientes se desplazan una posición hacia arriba.

---

## `gr` — Reinicio de la cuadrícula {#gr}

Borra la totalidad de la cuadrícula de una propiedad. La clave empieza por `"gr"`.

```json
{
  "gr-grid reset": {
    "Sequencer": {
      "p": {
        "sequence": {}
      }
    }
  }
}
```

---

## `lc` / `lu` / `ld` — Eventos LOV {#lc}

**`lc` — LOV creado/fusionado** (clave empieza por `"lc"`):

LOV de módulo:
```json
{
  "lc-lov create": {
    "Focus": {
      "l": {
        "myLov": {
          "label": "My LOV",
          "type": "string",
          "values": { "a": "Option A", "b": "Option B" }
        }
      }
    }
  }
}
```

LOV del controlador (sin envoltura de módulo, usa la clave `lovs`):
```json
{
  "lc-lov create": {
    "lovs": {
      "sharedLov": {
        "label": "Shared LOV",
        "type": "string",
        "values": { "x": "Value X" }
      }
    }
  }
}
```

**`lu` — LOV reemplazado** (mismo formato que `lc`, reemplazo completo).

**`ld` — LOV eliminado** (clave empieza por `"ld"`):
```json
{
  "ld-lov delete": {
    "lovs": {
      "sharedLov": {}
    }
  }
}
```

---

## `fs` / `fl` / `fc` — Eventos de perfil {#fs}

Enviados cuando un perfil se guarda, carga o modifica.

```json
{
  "fs-profile saved": {
    "Focus": {
      "f": { "name": "myProfile", "changed": false }
    }
  }
}
```

| Prefijo de clave | Evento |
|---|---|
| `fs` | Perfil guardado |
| `fl` | Perfil cargado |
| `fc` | Perfil con modificaciones no guardadas |

---

## `l` — Entrada de log {#l}

Enviado para cada mensaje de log generado por el servidor. La clave es exactamente `"l"`.

```json
{
  "l": {
    "d": "2024-01-01T12:00:00.000",
    "c": "Focus",
    "t": "Autofocus completed successfully",
    "l": 1
  }
}
```

| Campo | Descripción |
|---|---|
| `d` | Fecha/hora ISO 8601 con milisegundos |
| `c` | Contexto/origen (nombre del módulo o `"WS"`) |
| `t` | Texto del mensaje (traducido al idioma del cliente) |
| `l` | Nivel: `0` Debug · `1` Info · `2` Advertencia · `3` Error · `4` Crítico |

---

## `uc` — Actualización de los datos del controlador {#uc}

Enviado cuando cambian los datos globales del controlador: perfiles, configuraciones, drivers INDI activos, listas de archivos (`files`, `folders`), carga del sistema (`systemwatcher`, cada 10 s por defecto). La clave empieza por `"uc"`. Las claves posibles se describen en [Controlador, GlobalDatastore y archivos]({{% relref "controller.es.md" %}}).

```json
{
  "uc-update controller data": {
    "profiles": {
      "Focus": ["default", "highres"],
      "Guider": ["default"]
    }
  }
}
```

El payload es `{"clave": valor}` donde `valor` puede ser una cadena, un array o un objeto. Cada mensaje lleva **una sola clave**, y su valor **reemplaza por completo** al anterior.

---

## `evt` — Evento de archivo o de carpeta {#evt}

Enviado a todos los clientes cuando cambia el contenido de la carpeta de archivos vigilada (ver [Sistema de archivos]({{% relref "controller.es.md" %}}#fichiers)). A diferencia de los demás mensajes, la clave es exactamente `evt` y su **valor es una cadena** que indica el tipo de evento; las rutas van en un campo hermano `fileevent`.

```json
{ "evt": "fileadd", "fileevent": ["/Allsky/image_001.jpeg"] }
```

| `evt` | Significado |
|---|---|
| `fileadd` | Archivo(s) añadido(s) a la carpeta seleccionada |
| `filedel` | Archivo(s) eliminado(s) de la carpeta seleccionada |
| `folderadd` | Subcarpeta añadida al webroot |
| `folderdel` | Subcarpeta eliminada del webroot |

`fileevent` es siempre un array de rutas relativas al webroot (que empiezan por `/`).

---

## `xx` — Pong keepalive {#xx}

Enviado por el servidor en respuesta a un heartbeat de cliente (`XX`). Sin payload útil; los clientes pueden ignorarlo.

```json
{ "xx": {} }
```
