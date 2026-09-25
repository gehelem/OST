---
title: Messaggi Server → Client
weight: 40
description: "Tutti i messaggi JSON inviati dal server OST ai client connessi"
---

## Formato dei messaggi

Ogni messaggio è un oggetto JSON con **una sola chiave di primo livello**. Questa chiave codifica il tipo di evento e può includere una descrizione leggibile separata da un trattino:

```
"aa-dump all data"   →  tipo evento = primi 2 caratteri = "aa"
"ea"                 →  tipo evento = "ea"
"d"                  →  tipo evento = "d" (carattere unico)
```

I client devono confrontare i **primi 1 o 2 caratteri** della chiave, non l'intera stringa, poiché la parte descrittiva può cambiare.

Tutte le chiavi degli eventi inviate dal server sono in **minuscolo** (es. `d`, `ea`, `ap`, `gc`).

---

## Indice dei tipi di evento

| Chiave | Descrizione |
|---|---|
| [`d`](#d) | Dump iniziale completo (risposta a `DU`) |
| [`aa`](#aa) | Dump completo di un modulo (modulo caricato/ricaricato) |
| [`ap`](#ap) | Sostituzione completa di una proprietà |
| [`ea`](#ea) | Aggiornamento di tutti i valori degli elementi di una proprietà |
| [`ee`](#ee) | Aggiornamento di un singolo valore di elemento |
| [`ev`](#ev) | Aggiornamento di un valore di elemento con i suoi vincoli |
| [`ps`](#ps) | Aggiornamento dello stato e dell'abilitazione di una proprietà |
| [`dm`](#dm) | Modulo eliminato |
| [`dp`](#dp) | Proprietà eliminata |
| [`gc`](#gc) | Riga di griglia creata |
| [`gu`](#gu) | Riga di griglia aggiornata |
| [`gd`](#gd) | Riga di griglia eliminata |
| [`gr`](#gr) | Reset della griglia (tutte le righe eliminate) |
| [`lc`](#lc) | LOV creato o fuso |
| [`lu`](#lc) | LOV sostituito |
| [`ld`](#lc) | LOV eliminato |
| [`fs`](#fs) | Profilo salvato |
| [`fl`](#fs) | Profilo caricato |
| [`fc`](#fs) | Profilo modificato (modifiche non salvate) |
| [`l`](#l) | Voce di log |
| [`uc`](#uc) | Aggiornamento dei dati del controller |
| [`af`](#af) | File aggiunto |
| [`df`](#af) | File eliminato |
| [`ad`](#af) | Cartella aggiunta |
| [`dd`](#af) | Cartella eliminata |
| [`xx`](#xx) | Pong keepalive |

---

## `d` — Dump iniziale completo {#d}

Inviato in risposta a una richiesta `DU`. Contiene tutto: tutti i moduli, i log, gli elenchi di file e i dati del controller. Il dettaglio delle chiavi di `controllerdata` e di `files` è in [Controller, GlobalDatastore e file]({{% relref "controller.it.md" %}}); l'esempio seguente ne mostra solo alcune.

```json
{
  "d": {
    "grant-client": "1",
    "grant-server": "0",
    "serverlng": "en",
    "m": {
      "Focus": {
        "infos": { "modulelabel": "Focus", "description": "..." },
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

{{% notice style="warning" title="Abbreviazioni del formato wire" %}}
`m` = modules, `p` = properties, `e` = elements, `l` = globallovs, `f` = profile.
Normalizzare queste abbreviazioni ai loro nomi completi durante la memorizzazione interna.
{{% /notice %}}

---

## `aa` — Dump completo di un modulo (modulo caricato/ricaricato) {#aa}

Inviato quando un modulo viene caricato o ricaricato a runtime. Contiene lo stato completo del modulo. La chiave inizia con `"aa"`.

```json
{
  "aa-dump all data": {
    "Guider": {
      "infos": { "modulelabel": "Guider", "description": "..." },
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

## `ap` — Sostituzione completa di una proprietà {#ap}

Sostituisce interamente una o più proprietà. La chiave inizia con `"ap"`.

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

## `ea` — Aggiornamento di tutti i valori degli elementi (livello proprietà) {#ea}

Il messaggio di aggiornamento più comune. Invia i valori correnti di tutti gli elementi di una proprietà. Contiene solo i valori — senza metadati. La chiave è esattamente `"ea"`.

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

Per i tipi semplici (`int`, `float`, `bool`, `string`, `light`), il valore è uno scalare:
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

## `ee` — Aggiornamento di un singolo valore di elemento {#ee}

Aggiorna il valore di un singolo elemento. Stesso formato di `ea` ma per un solo elemento. La chiave è esattamente `"ee"`.

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

{{% notice style="note" title="`ee` è solo di origine backend" %}}
`ee` viene inviato solo per iniziativa del backend stesso (es. un cambio di valore proveniente da un device, un polling interno) — mai come risposta diretta a una scrittura del client. Qualsiasi risposta a un `SV` o `SA` avviato dal client è sempre un `ea` dell'intera proprietà, indipendentemente dal numero di elementi effettivamente cambiati.
{{% /notice %}}

---

## `ev` — Aggiornamento di un elemento con metadati {#ev}

Aggiorna il valore di un elemento e i suoi vincoli (min, max, format). La chiave inizia con `"ev"`.

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

## `ps` — Aggiornamento dello stato di una proprietà {#ps}

Aggiorna solo i campi `status` e `enabled` di una proprietà. La chiave inizia con `"ps"`.

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

Valori di stato: `0` Standby · `1` OK · `2` Occupato · `3` Errore

---

## `dm` — Modulo eliminato {#dm}

Inviato quando un modulo viene scaricato. Il client deve eliminare tutti i dati di questo modulo. La chiave inizia con `"dm"`.

```json
{
  "dm-delete/remove module": {
    "Focus": {}
  }
}
```

---

## `dp` — Proprietà eliminata {#dp}

Inviato quando una proprietà viene rimossa da un modulo. La chiave inizia con `"dp"`.

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

## `gc` — Riga di griglia creata {#gc}

Inviato dopo l'aggiunta di una nuova riga in una proprietà di tipo griglia. La chiave inizia con `"gc"`.

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

`i` è l'indice della riga appena creata. `values` è un oggetto indicizzato per nome elemento.

---

## `gu` — Riga di griglia aggiornata {#gu}

Inviato quando una riga esistente della griglia viene modificata. La chiave inizia con `"gu"`.

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

## `gd` — Riga di griglia eliminata {#gd}

Inviato quando una riga della griglia viene rimossa. La chiave inizia con `"gd"`.

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

`i` è l'indice (base zero) della riga eliminata. Le righe successive si spostano di una posizione verso l'alto.

---

## `gr` — Reset della griglia {#gr}

Elimina l'intera griglia di una proprietà. La chiave inizia con `"gr"`.

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

## `lc` / `lu` / `ld` — Eventi LOV {#lc}

**`lc` — LOV creato/fuso** (chiave inizia con `"lc"`):

LOV di modulo:
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

LOV del controller (senza involucro modulo, usa la chiave `lovs`):
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

**`lu` — LOV sostituito** (stesso formato di `lc`, sostituzione completa).

**`ld` — LOV eliminato** (chiave inizia con `"ld"`):
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

## `fs` / `fl` / `fc` — Eventi di profilo {#fs}

Inviati quando un profilo viene salvato, caricato o modificato.

```json
{
  "fs-profile saved": {
    "Focus": {
      "f": { "name": "myProfile", "changed": false }
    }
  }
}
```

| Prefisso chiave | Evento |
|---|---|
| `fs` | Profilo salvato |
| `fl` | Profilo caricato |
| `fc` | Profilo con modifiche non salvate |

---

## `l` — Voce di log {#l}

Inviato per ogni messaggio di log generato dal server. La chiave è esattamente `"l"`.

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

| Campo | Descrizione |
|---|---|
| `d` | Data/ora ISO 8601 con millisecondi |
| `c` | Contesto/fonte (nome del modulo o `"WS"`) |
| `t` | Testo del messaggio (tradotto nella lingua del client) |
| `l` | Livello: `0` Debug · `1` Info · `2` Avviso · `3` Errore · `4` Critico |

---

## `uc` — Aggiornamento dei dati del controller {#uc}

Inviato quando i dati globali del controller cambiano: profili, configurazioni, driver INDI attivi, elenchi di file (`files`, `folders`), carico del sistema (`systemwatcher`, ogni 10 s per impostazione predefinita). La chiave inizia con `"uc"`. Le chiavi possibili sono descritte in [Controller, GlobalDatastore e file]({{% relref "controller.it.md" %}}).

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

Il payload è `{"chiave": valore}` dove `valore` può essere una stringa, un array o un oggetto. Ogni messaggio porta **una sola chiave** e il suo valore **sostituisce interamente** quello precedente.

---

## `af` / `df` / `ad` / `dd` — Eventi di file e cartella {#af}

Inviato a tutti i client quando il contenuto della cartella di file sorvegliata cambia (vedi [File system]({{% relref "controller.it.md" %}}#fichiers)). A differenza degli altri messaggi, il valore è una **stringa nuda**: il percorso interessato, non racchiuso in un oggetto o array — il server segnala sempre un solo percorso per messaggio.

```json
{ "af": "/Allsky/image_001.jpeg" }
{ "df": "/Allsky/old_image.jpeg" }
```

| Chiave | Significato |
|---|---|
| `af` | File aggiunto alla cartella selezionata |
| `df` | File eliminato dalla cartella selezionata |
| `ad` | Sottocartella aggiunta ovunque nel webroot |
| `dd` | Sottocartella eliminata ovunque nel webroot |

`af`/`df` riguardano solo la cartella attualmente selezionata tramite [`FS`]({{% relref "client-messages.it.md" %}}#fs) — non viene segnalato nulla per il contenuto delle altre cartelle. `ad`/`dd` sono globali: si attivano per l'intero albero del webroot, indipendentemente da cosa è attualmente selezionato. Vedi [File system]({{% relref "controller.it.md" %}}#fichiers) per il quadro completo.

---

## `xx` — Pong keepalive {#xx}

Inviato dal server in risposta a un heartbeat client (`XX`). Nessun payload utile; i client possono ignorarlo.

```json
{ "xx": {} }
```
