---
title: Mensajes Cliente → Servidor
weight: 30
description: "Todos los mensajes JSON que los clientes pueden enviar al servidor OST"
---

## Formato de los mensajes

Cada mensaje es un objeto JSON con **una única clave de primer nivel** que identifica el comando. El valor es un objeto cuya estructura depende del comando.

Todas las claves de comandos de cliente están en **MAYÚSCULAS** (ej. `DU`, `SV`, `SA`, `GC`).

---

## Índice de comandos

| Clave | Descripción |
|---|---|
| [`DU`](#du) | Solicitar el dump completo |
| [`XX`](#xx) | Heartbeat / keepalive |
| [`LO`](#lo) | Conexión |
| [`IL`](#il) | Cambiar el idioma |
| [`SV`](#sv) | Enviar el valor de un solo elemento |
| [`SA`](#sa) | Enviar todos los valores de elementos |
| [`I1`](#i1) | Clic en preicon1 de propiedad |
| [`I2`](#i2) | Clic en preicon2 de propiedad |
| [`I3`](#i3) | Clic en posticon1 de propiedad |
| [`I4`](#i4) | Clic en posticon2 de propiedad |
| [`J1`](#j1) | Clic en preicon de elemento |
| [`J2`](#j2) | Clic en posticon de elemento |
| [`PL`](#pl) | Cargar un perfil |
| [`PS`](#ps) | Guardar un perfil |
| [`GC`](#gc) | Crear una línea de cuadrícula |
| [`GU`](#gu) | Actualizar una línea de cuadrícula |
| [`GD`](#gd) | Eliminar una línea de cuadrícula |
| [`GF`](#gf) | Seleccionar una línea de cuadrícula |
| [`GH`](#gh) | Subir una línea de cuadrícula |
| [`GB`](#gb) | Bajar una línea de cuadrícula |
| [`CL`](#cl) | Cargar una configuración de módulos |
| [`CS`](#cs) | Guardar la configuración actual |
| [`FS`](#fs) | Elegir la carpeta de archivos vigilada |
| [`ML`](#ml) | Cargar un módulo |
| [`MK`](#mk) | Descargar un módulo |
| [`YA`](#indi) | Iniciar el servidor INDI embebido |
| [`YZ`](#indi) | Detener el servidor INDI embebido |
| [`YL`](#indi) | Cargar un driver INDI |
| [`YR`](#indi) | Recargar un driver INDI |
| [`YS`](#indi) | Detener un driver INDI |

---

Todos los mensajes dirigidos a un módulo o a una propiedad usan la siguiente envoltura:

```json
{
  "COMANDO": {
    "m": {
      "NombreDelModulo": {
        "p": {
          "nombreDeLaPropiedad": { ... }
        }
      }
    }
  }
}
```

donde `m` designa *modules* y `p` designa *properties*.

---

## Gestión de sesión

### `DU` — Solicitar el dump completo {#du}

Debe enviarse inmediatamente tras la conexión. El servidor responde con un mensaje `d` que contiene todos los módulos.

```json
{ "DU": { "language": "es" } }
```

| Campo | Descripción |
|---|---|
| `language` | Idioma preferido para las etiquetas traducidas (`"en"`, `"fr"`, etc.) |

---

### `XX` — Heartbeat {#xx}

Enviar cada 30 segundos para mantener la conexión activa. El servidor responde con `xx`.

```json
{ "XX": {} }
```

---

### `LO` — Conexión {#lo}

Requerido cuando el servidor tiene activado el control de acceso (`grant-client` vale `"-1"` en la respuesta dump).

```json
{
  "LO": {
    "user": "username",
    "pw": "password",
    "language": "es"
  }
}
```

---

### `IL` — Cambiar el idioma {#il}

Modifica el idioma de las etiquetas traducidas sin reconectar.

```json
{ "IL": { "language": "es" } }
```

---

## Actualización de los valores de propiedad

### `SV` — Enviar el valor de un solo elemento {#sv}

A usar cuando `directedit` es `true` en el elemento — envía inmediatamente el valor de un solo elemento. Sobre un elemento con `directedit: false` el backend lo rechaza (error en el log del módulo, no se aplica nada): usar `SA` en su lugar.

```json
{
  "SV": {
    "m": {
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
}
```

`e` contiene un único par `nombreElemento: valor`.

---

### `SA` — Enviar todos los valores de elementos {#sa}

Usado para enviar todos los elementos editables de una propiedad de una vez (ej. desde un formulario). También usado cuando `directedit` es `false`.

```json
{
  "SA": {
    "m": {
      "Focus": {
        "p": {
          "parameters": {
            "e": {
              "iterations": 7,
              "exposure": 3.5,
              "active": true
            }
          }
        }
      }
    }
  }
}
```

{{% notice style="warning" title="Enviar siempre todos los elementos" %}}
Un `SA` debe incluir todos los elementos editables de la propiedad. Si falta uno solo, el backend no aplica absolutamente nada — ni siquiera los elementos presentes — y registra una advertencia en el servidor (visible para el cliente mediante un mensaje `l`). No es un caso a gestionar con una degradación elegante: indica un error en el cliente, que siempre debe enviar el conjunto completo de elementos.
{{% /notice %}}

---

## Acciones sobre los iconos

Los iconos de propiedad (preicon / posticon) desencadenan acciones de backend como el guardado de un perfil, la carga de un perfil o el lanzamiento de un proceso. Los eventos de clic se envían mediante los siguientes comandos.

### `I1` — Clic en preicon1 de propiedad {#i1}

```json
{
  "I1": {
    "m": {
      "Focus": {
        "p": {
          "saveprofile": {}
        }
      }
    }
  }
}
```

### `I2` — Clic en preicon2 de propiedad {#i2}

```json
{
  "I2": {
    "m": {
      "Focus": {
        "p": {
          "loadprofile": {}
        }
      }
    }
  }
}
```

### `I3` — Clic en posticon1 de propiedad {#i3}

Misma estructura que `I1` / `I2`.

### `I4` — Clic en posticon2 de propiedad {#i4}

Misma estructura que `I1` / `I2`.

---

### `J1` — Clic en preicon de elemento {#j1}

```json
{
  "J1": {
    "m": {
      "Focus": {
        "p": {
          "devices": {
            "e": {
              "myDevice": {}
            }
          }
        }
      }
    }
  }
}
```

### `J2` — Clic en posticon de elemento {#j2}

Misma estructura que `J1`.

---

## Gestión de perfiles

### `PL` — Cargar un perfil {#pl}

```json
{
  "PL": {
    "m": {
      "Focus": {
        "profile": "myProfile"
      }
    }
  }
}
```

El servidor responde con un mensaje `fl` (perfil cargado) y luego envía los valores de propiedades actualizados.

---

### `PS` — Guardar un perfil {#ps}

```json
{
  "PS": {
    "m": {
      "Focus": {
        "profile": "myProfile"
      }
    }
  }
}
```

El servidor responde con un mensaje `fs` (perfil guardado).

---

## Operaciones sobre cuadrículas

Los comandos de cuadrícula apuntan a una propiedad específica dentro de un módulo. Todos usan la envoltura `m` / `p`.

### `GC` — Crear una línea de cuadrícula {#gc}

Añade una nueva línea a la cuadrícula con los valores de elementos proporcionados.

```json
{
  "GC": {
    "m": {
      "Sequencer": {
        "p": {
          "sequence": {
            "e": {
              "target": "M31",
              "exposure": 120,
              "count": 10
            }
          }
        }
      }
    }
  }
}
```

El servidor responde con un mensaje `gc` confirmando el índice de la nueva línea.

{{% notice style="note" title="Los elementos base se actualizan antes que la línea de la cuadrícula" %}}
Crear una línea de cuadrícula aplica primero los valores enviados a los elementos base de la propiedad (cada uno emite su propio `ea`), y luego añade la línea y emite `gc`. Si una propiedad muestra a la vez sus elementos base y su cuadrícula (`showElts` y `showGrid` ambos activos — combinación poco habitual), un cliente que renderiza en directo verá brevemente los elementos base reflejar los valores de la nueva línea antes de que la línea misma aparezca. Este es el estado real del datastore, no un error — adapte su renderizado a este comportamiento en lugar de intentar filtrarlo, y sobre todo nunca haga polling para evitarlo.
{{% /notice %}}

---

### `GU` — Actualizar una línea de cuadrícula {#gu}

Actualiza una línea específica (índice base cero `i`) con nuevos valores de elementos.

```json
{
  "GU": {
    "m": {
      "Sequencer": {
        "p": {
          "sequence": {
            "i": 1,
            "e": {
              "target": "M42",
              "exposure": 60
            }
          }
        }
      }
    }
  }
}
```

---

### `GD` — Eliminar una línea de cuadrícula {#gd}

Elimina la línea en el índice `i` (base cero).

```json
{
  "GD": {
    "m": {
      "Sequencer": {
        "p": {
          "sequence": {
            "i": 0
          }
        }
      }
    }
  }
}
```

---

### `GF` — Seleccionar una línea de cuadrícula {#gf}

Solicita al servidor que cargue una línea de la cuadrícula en los elementos editables de la propiedad (usado al hacer clic en una línea para editarla).

```json
{
  "GF": {
    "m": {
      "Sequencer": {
        "p": {
          "sequence": {
            "i": 2
          }
        }
      }
    }
  }
}
```

---

### `GH` — Subir una línea de cuadrícula {#gh}

Desplaza la línea en el índice `i` una posición hacia arriba.

```json
{
  "GH": {
    "m": {
      "Sequencer": {
        "p": {
          "sequence": {
            "i": 3
          }
        }
      }
    }
  }
}
```

---

### `GB` — Bajar una línea de cuadrícula {#gb}

Desplaza la línea en el índice `i` una posición hacia abajo.

```json
{
  "GB": {
    "m": {
      "Sequencer": {
        "p": {
          "sequence": {
            "i": 2
          }
        }
      }
    }
  }
}
```

---

## Ciclo de vida de los módulos

### `CL` — Cargar una configuración {#cl}

Carga todos los módulos de una configuración guardada (lista en `controllerdata.availableconfs`, ver [Controlador]({{% relref "controller.es.md" %}})).

```json
{ "CL": { "name": "modules" } }
```

---

### `CS` — Guardar la configuración actual {#cs}

Registra el conjunto de módulos actualmente cargados con el nombre indicado (`GlobalDatastore` nunca se incluye).

```json
{ "CS": { "name": "modules" } }
```

---

### `ML` — Cargar un módulo {#ml}

Pide al servidor que cargue dinámicamente un módulo. Estos comandos se dirigen al controlador: el payload **no** usa la envoltura `m`/`p`.

```json
{ "ML": { "lib": "focus", "label": "MyFocuser", "profile": "default" } }
```

| Campo | Descripción |
|---|---|
| `lib` | Biblioteca del módulo, **sin** el prefijo `libost` (lo añade el servidor): `focus` para `libostfocus`. Las bibliotecas disponibles son las claves de `controllerdata.libraries` |
| `label` | Etiqueta de esta instancia; el **nombre del módulo** es esa etiqueta sin espacios. Si ya hay cargado un módulo con ese nombre, la petición se rechaza |
| `profile` | Nombre del perfil a cargar (opcional) |

---

### `MK` — Descargar un módulo {#mk}

Pide al servidor que descargue un módulo en ejecución, designado por su nombre.

```json
{ "MK": { "name": "MyFocuser" } }
```

`GlobalDatastore` no puede descargarse: la petición se rechaza (mensaje de error en los logs).

---

## Sistema de archivos

### `FS` — Elegir la carpeta vigilada {#fs}

Designa la carpeta cuya lista de archivos publica el servidor (ver [Sistema de archivos]({{% relref "controller.es.md" %}}#fichiers)). La ruta es **relativa al webroot**; la cadena vacía designa la raíz.

```json
{ "FS": { "folder": "/Sequencer/gam Cyg/LIGHT/Luminance" } }
```

El cambio es **global en el servidor** (todos los clientes ven la nueva lista). La ruta no se valida: una carpeta inexistente da una lista vacía.

---

## Control del servidor INDI embebido {#indi}

Estos comandos solo están disponibles cuando el servidor dispone de una instancia INDI embebida.

| Comando | Descripción |
|---|---|
| `YA` | Iniciar el servidor INDI embebido |
| `YZ` | Detener el servidor INDI embebido |
| `YL` | Cargar un driver INDI (`{"YL": {"driver": "<binary>"}}`) |
| `YR` | Recargar un driver INDI (`{"YR": {"driver": "<binary>"}}`) |
| `YS` | Detener un driver INDI (`{"YS": {"driver": "<binary>"}}`) |

`YA` y `YZ` no tienen payload (`{"YA": {}}`). Para `YL`, `YR` y `YS`, `driver` es el campo `binary` de una entrada de `controllerdata.indidrivers`.

---

## Resumen del control de acceso

| Valor grant servidor | Comandos permitidos |
|---|---|
| `grant-client: "1"` | Todos los comandos |
| `grant-client: "0"` | Solo `DU`, `LO`, `IL` |
| `grant-client: "-1"` | Solo `LO` (autenticación requerida) |

Los comandos enviados sin permisos suficientes son ignorados silenciosamente por el servidor.
