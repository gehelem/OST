---
title: Esempio di sessione
weight: 40
description: "Scambi WebSocket annotati che illustrano una sessione client completa"
---

Questa pagina dettaglia una sessione WebSocket realistica: connessione, dump, login, ricezione degli aggiornamenti, invio di comandi e keepalive.

Notazione:
- **`→`** Client → Server
- **`←`** Server → Client

---

## 1. Connessione

Aprire una connessione WebSocket verso il server.

```
→ WS connect to ws://server:9624
← (connessione accettata)
```

---

## 2. Richiedere il dump completo

Da inviare immediatamente dopo la connessione. Specificare la lingua desiderata per le etichette tradotte.

```json
→ {"DU": {"language": "it"}}
```

Il server risponde con `d`, un unico messaggio contenente tutti i moduli, le loro proprietà ed elementi, l'elenco dei file, i log, i LOV del controller e i diritti di accesso.

```json
← {
  "d": {
    "grant-client": "1",
    "grant-server": "0",
    "serverlng": "it",
    "m": {
      "MyFocuser": {
        "infos": {"label": "MyFocuser", "name": "MyFocuser", "description": ""},
        "f": {"name": "default", "changed": false},
        "l": {},
        "p": {
          "parameters": {
            "label": "Parametri", "order": "10",
            "level1": "MyFocuser", "level2": "Config",
            "status": 0, "permission": 2, "enabled": true,
            "showElts": true, "hasGrid": false, "hasGraph": false,
            "rule": 0, "badge": false,
            "preicon1": "", "preicon2": "", "posticon1": "", "posticon2": "",
            "hasprofile": true, "freevalue": "",
            "e": {
              "exposure": {
                "type": "float", "label": "Esposizione", "order": "10",
                "hint": "", "autoupdate": false, "badge": false,
                "directedit": true,
                "value": 3.5, "min": 0.1, "max": 30.0, "step": 0.1,
                "format": "%.1f", "slider": 0
              },
              "abortfocus": {
                "type": "bool", "label": "Annulla", "order": "90",
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
Se `grant-client` vale `"-1"`, il server richiede un'autenticazione prima di accettare comandi. Vedi il passo 3.
Se vale `"0"` o `"1"`, i comandi sono accettati direttamente.
{{% /notice %}}

---

## 3. Login (se richiesto)

Necessario solo se `grant-client` vale `"-1"` nel dump.

```json
→ {"LO": {"user": "ADMIN", "pw": "admin", "language": "it"}}
```

In caso di successo, il server rinvia un nuovo dump (stesso formato di `d`) con i diritti aggiornati.

---

## 4. Ricezione degli aggiornamenti in tempo reale

Il server invia aggiornamenti in qualsiasi momento. Non è necessario alcun polling.

### Aggiornamento dei valori degli elementi (`ea`)

Il messaggio più frequente. Il server invia i valori correnti non appena avviene un cambiamento.

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

### Cambiamento di stato di una proprietà (`ps`)

Inviato quando una proprietà passa in modalità occupato, inattivo o disattivato.

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

Valori di stato: `0` Standby · `1` OK · `2` Occupato · `3` Errore

### Voce di log (`l`)

```json
← {"l": {"d": "2026-06-25T10:14:08.000", "c": "MyFocuser", "t": "Autofocus avviato", "l": 1}}
```

---

## 5. Invio di un comando

### Annullare un'azione in corso

Gli elementi con `directedit: true` inviano il loro valore immediatamente tramite `SV`.

```json
→ {"SV": {"m": {"MyFocuser": {"p": {"actions": {"e": {"abortfocus": true}}}}}}}
```

Il server confirma la ricezione inviando aggiornamenti di stato e valori:

```json
← {"ps-only property state": {"MyFocuser": {"p": {"actions": {"status": 2, "enabled": true}}}}}
← {"ea": {"MyFocuser": {"p": {"actions": {"e": {"abortfocus": false}}}}}}
← {"ps-only property state": {"MyFocuser": {"p": {"actions": {"status": 0, "enabled": true}}}}}
```

### Avviare un'azione

```json
→ {"SV": {"m": {"MyPlanner": {"p": {"actions": {"e": {"start": true}}}}}}}
```

### Invio di più valori in una volta

Usare `SA` per le proprietà dove `directedit` è `false` (modifica tramite form).

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

Inviare `XX` ogni 30 secondi per mantenere attiva la connessione.

```json
→ {"XX": {}}
← {"xx": {}}
```

---

## Sequenza completa (riepilogo)

```
→ WS connect
→ {"DU": {"language": "it"}}
← {"d": { ... dump completo ... }}

← {"ea": { ... aggiornamenti periodici ... }}

→ {"SV": {"m": {"MyFocuser": {"p": {"actions": {"e": {"abortfocus": true}}}}}}}
← {"ps-...": { ... stato occupato ... }}
← {"ea": { ... valori ... }}
← {"ps-...": { ... stato standby ... }}

→ {"XX": {}}
← {"xx": {}}
```
