---
title: Ejemplo de sesión
weight: 60
description: "Intercambios WebSocket anotados que ilustran una sesión de cliente completa"
---

Esta página detalla una sesión WebSocket realista: conexión, dump, login, recepción de actualizaciones, envío de comandos y keepalive.

Notación:
- **`→`** Cliente → Servidor
- **`←`** Servidor → Cliente

---

## 1. Conexión

Abrir una conexión WebSocket hacia el servidor.

```
→ WS connect to ws://server:9624
← (conexión aceptada)
```

---

## 2. Solicitar el dump completo

Se debe enviar inmediatamente tras la conexión. Especificar el idioma deseado para las etiquetas traducidas.

```json
→ {"DU": {"language": "es"}}
```

El servidor responde con `d`, un mensaje único que contiene todos los módulos, sus propiedades y elementos, la lista de archivos, los logs, los LOV del controlador y los derechos de acceso.

```json
← {
  "d": {
    "grant-client": "1",
    "grant-server": "0",
    "serverlng": "es",
    "m": {
      "MyFocuser": {
        "infos": {"label": "MyFocuser", "name": "MyFocuser", "description": ""},
        "f": {"name": "default", "changed": false},
        "l": {},
        "p": {
          "parameters": {
            "label": "Parámetros", "order": "10",
            "level1": "MyFocuser", "level2": "Config",
            "status": 0, "permission": 2, "enabled": true,
            "showElts": true, "hasGrid": false, "hasGraph": false,
            "rule": 0, "badge": false,
            "preicon1": "", "preicon2": "", "posticon1": "", "posticon2": "",
            "hasprofile": true, "freevalue": "",
            "e": {
              "exposure": {
                "type": "float", "label": "Exposición", "order": "10",
                "hint": "", "autoupdate": false, "badge": false,
                "directedit": true,
                "value": 3.5, "min": 0.1, "max": 30.0, "step": 0.1,
                "format": "%.1f", "slider": 0
              },
              "abortfocus": {
                "type": "bool", "label": "Cancelar", "order": "90",
                "hint": "", "autoupdate": false, "badge": false,
                "directedit": true, "value": false
              }
            }
          }
        }
      }
    },
    "files": {"folders": [], "files": [], "selectedfolder": ""},
    "logs": [],
    "controllerdata": {"profiles": {"MyFocuser": ["default"]}},
    "lovs": {}
  }
}
```

{{% notice style="info" title="grant-client" %}}
Si `grant-client` vale `"-1"`, el servidor exige autenticación antes de aceptar comandos. Ver el paso 3.
Si vale `"0"` o `"1"`, los comandos se aceptan directamente.
{{% /notice %}}

---

## 3. Login (si es necesario)

Necesario únicamente si `grant-client` vale `"-1"` en el dump.

```json
→ {"LO": {"user": "ADMIN", "pw": "admin", "language": "es"}}
```

En caso de éxito, el servidor devuelve un nuevo dump (mismo formato que `d`) con los derechos actualizados.

---

## 4. Recepción de actualizaciones en tiempo real

El servidor envía actualizaciones en cualquier momento. No es necesario ningún polling.

### Actualización de los valores de elementos (`ea`)

El mensaje más frecuente. El servidor envía los valores actuales en cuanto se produce un cambio.

```json
← {
  "ea": {
    "MyFocuser": {
      "p": {
        "parameters": {
          "e": {
            "exposure": 3.5,
            "abortfocus": false
          }
        }
      }
    }
  }
}
```

### Cambio de estado de una propiedad (`ps`)

Enviado cuando una propiedad pasa a modo ocupado, inactivo o desactivado.

```json
← {
  "ps-only property state": {
    "MyFocuser": {
      "p": {
        "parameters": {
          "status": 2,
          "enabled": true
        }
      }
    }
  }
}
```

Valores de estado: `0` Reposo · `1` OK · `2` Ocupado · `3` Error

### Entrada de log (`l`)

```json
← {"l": {"d": "2026-06-25T10:14:08.000", "c": "MyFocuser", "t": "Autofocus démarré", "l": 1}}
```

---

## 5. Enviar un comando

### Cancelar una acción en curso

Los elementos con `directedit: true` envían su valor inmediatamente vía `SV`.

```json
→ {"SV": {"m": {"MyFocuser": {"p": {"actions": {"e": {"abortfocus": true}}}}}}}
```

El servidor confirma la recepción enviando actualizaciones de estado y de valores:

```json
← {"ps-only property state": {"MyFocuser": {"p": {"actions": {"status": 2, "enabled": true}}}}}
← {"ea": {"MyFocuser": {"p": {"actions": {"e": {"abortfocus": false}}}}}}
← {"ps-only property state": {"MyFocuser": {"p": {"actions": {"status": 0, "enabled": true}}}}}
```

### Iniciar una acción

```json
→ {"SV": {"m": {"MyPlanner": {"p": {"actions": {"e": {"start": true}}}}}}}
```

### Enviar varios valores a la vez

Usar `SA` para las propiedades donde `directedit` es `false` (edición mediante formulario).

```json
→ {
  "SA": {
    "m": {
      "MyFocuser": {
        "p": {
          "parameters": {
            "e": {
              "exposure": 5.0,
              "gain": 100
            }
          }
        }
      }
    }
  }
}
```

---

## 6. Keepalive

Enviar `XX` cada 30 segundos para mantener la conexión activa.

```json
→ {"XX": {}}
← {"xx": {}}
```

---

## Secuencia completa (resumen)

```
→ WS connect
→ {"DU": {"language": "es"}}
← {"d": { ... dump completo ... }}

← {"ea": { ... actualizaciones periódicas ... }}

→ {"SV": {"m": {"MyFocuser": {"p": {"actions": {"e": {"abortfocus": true}}}}}}}
← {"ps-...": { ... estado ocupado ... }}
← {"ea": { ... valores ... }}
← {"ps-...": { ... estado reposo ... }}

→ {"XX": {}}
← {"xx": {}}
```
