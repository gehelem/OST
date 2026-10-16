---
title: Messaggi Client → Server
weight: 50
description: "Tutti i messaggi JSON che i client possono inviare al server OST"
---

## Formato dei messaggi

Ogni messaggio è un oggetto JSON con **una sola chiave di primo livello** che identifica il comando. Il valore è un oggetto la cui struttura dipende dal comando.

Tutte le chiavi dei comandi client sono in **MAIUSCOLO** (es. `DU`, `SV`, `SA`, `GC`).

---

## Indice dei comandi

| Chiave | Descrizione |
|---|---|
| [`DU`](#du) | Richiedere il dump completo |
| [`XX`](#xx) | Heartbeat / keepalive |
| [`LO`](#lo) | Connessione |
| [`IL`](#il) | Cambiare la lingua |
| [`SV`](#sv) | Inviare il valore di un singolo elemento |
| [`SA`](#sa) | Inviare tutti i valori degli elementi |
| [`DP`](#dp) | Richiedere il dump di una proprietà |
| [`I1`](#i1) | Clic su preicon1 di proprietà |
| [`I2`](#i2) | Clic su preicon2 di proprietà |
| [`I3`](#i3) | Clic su posticon1 di proprietà |
| [`I4`](#i4) | Clic su posticon2 di proprietà |
| [`J1`](#j1) | Clic su preicon di elemento |
| [`J2`](#j2) | Clic su posticon di elemento |
| [`PL`](#pl) | Caricare un profilo |
| [`PS`](#ps) | Salvare un profilo |
| [`GC`](#gc) | Creare una riga di griglia |
| [`GU`](#gu) | Aggiornare una riga di griglia |
| [`GD`](#gd) | Eliminare una riga di griglia |
| [`GF`](#gf) | Selezionare una riga di griglia |
| [`GH`](#gh) | Spostare in alto una riga di griglia |
| [`GB`](#gb) | Spostare in basso una riga di griglia |
| [`GR`](#gr) | Azzerare (svuotare) la griglia |
| [`CL`](#cl) | Caricare una configurazione di moduli |
| [`CS`](#cs) | Salvare la configurazione corrente |
| [`FS`](#fs) | Scegliere la cartella di file sorvegliata |
| [`ML`](#ml) | Caricare un modulo |
| [`MK`](#mk) | Scaricare un modulo |
| [`QY`](#qy) | Interrogare un modulo (meccanismo inter-modulo, non destinato al client) |
| [`YA`](#indi) | Avviare il server INDI integrato |
| [`YZ`](#indi) | Arrestare il server INDI integrato |
| [`YL`](#indi) | Caricare un driver INDI |
| [`YR`](#indi) | Ricaricare un driver INDI |
| [`YS`](#indi) | Arrestare un driver INDI |
| [`YC`](#indi) | Connettere il device di un driver INDI |
| [`YD`](#indi) | Disconnettere il device di un driver INDI |
| [`YK`](#indi) | Caricare una configurazione di driver INDI salvata |

---

Tutti i messaggi destinati a un modulo o una proprietà usano il seguente involucro:

```json
{
  "COMANDO": {
    "m": {
      "NomeDelModulo": {
        "p": {
          "nomeDellaProprietà": { ... }
        }
      }
    }
  }
}
```

dove `m` indica *modules* e `p` indica *properties*.

---

## Gestione della sessione

### `DU` — Richiedere il dump completo {#du}

Deve essere inviato immediatamente dopo la connessione. Il server risponde con un messaggio `d` contenente tutti i moduli.

```json
{ "DU": { "language": "it" } }
```

| Campo | Descrizione |
|---|---|
| `language` | Lingua preferita per le etichette tradotte (`"en"`, `"it"`, ecc.) |

---

### `XX` — Heartbeat {#xx}

Da inviare ogni 30 secondi per mantenere attiva la connessione. Il server risponde con `xx`.

```json
{ "XX": {} }
```

---

### `LO` — Connessione {#lo}

Richiesto quando il server ha il controllo degli accessi attivato (`grant-client` vale `"-1"` nella risposta dump).

```json
{
  "LO": {
    "user": "username",
    "pw": "password",
    "language": "it"
  }
}
```

---

### `IL` — Cambiare la lingua {#il}

Modifica la lingua delle etichette tradotte senza riconnettersi.

```json
{ "IL": { "language": "it" } }
```

---

## Aggiornamento dei valori di proprietà

### `SV` — Inviare il valore di un singolo elemento {#sv}

Da usare quando `directedit` è `true` sull'elemento — invia immediatamente il valore di un singolo elemento. Su un elemento con `directedit: false` il backend lo rifiuta (errore nel log del modulo, nulla viene applicato): usare invece `SA`.

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

`e` contiene una sola coppia `nomeElemento: valore`.

---

### `SA` — Inviare tutti i valori degli elementi {#sa}

Usato per inviare tutti gli elementi editabili di una proprietà in una volta (es. da un form). Usato anche quando `directedit` è `false`.

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

{{% notice style="warning" title="Inviare sempre tutti gli elementi" %}}
Un `SA` deve contenere tutti gli elementi editabili della proprietà. Se anche uno solo manca, il backend non applica nulla — nemmeno gli elementi presenti — e registra un avviso lato server (visibile al client tramite un messaggio `l`). Non è un caso da gestire con una degradazione elegante: indica un bug nel client, che deve sempre inviare l'insieme completo degli elementi.
{{% /notice %}}

---

### `DP` — Richiedere il dump di una proprietà {#dp}

Chiede al server di rinviare lo stato completo di una proprietà, come se fosse appena stata (ri)caricata — utile per forzare l'aggiornamento di una proprietà che un client sospetta essere disallineata, senza ricaricare l'intero modulo. Il server risponde con un messaggio [`ap`]({{% relref "server-messages.it.md" %}}#ap) per quella sola proprietà.

```json
{
  "DP": {
    "m": {
      "Focus": {
        "p": {
          "parameters": {}
        }
      }
    }
  }
}
```

Nessun campo `e`: `DP` riguarda l'intera proprietà, non un singolo elemento.

---

## Azioni sulle icone {#icon-actions}

`preicon1`/`preicon2`/`posticon1`/`posticon2` (proprietà) e `preicon`/`posticon` (elementi, vedi [Modello dati]({{% relref "structure.it.md" %}})) non sono decorazioni: ciascuna è un'**azione cliccabile**. Un valore non vuoto è il nome dell'icona Google Fonts (Material Icons) da visualizzare; cliccando sull'icona visualizzata si invia il comando corrispondente sotto, **senza alcun valore o parametro** — un payload vuoto che punta semplicemente alla proprietà (o all'elemento). Cosa fa realmente l'azione lato server (salvare un profilo, caricare un profilo, avviare un processo...) dipende interamente dal modulo; l'unico compito del client è visualizzare l'icona e inoltrare il clic così com'è.

Una **stringa vuota significa che non c'è alcuna icona** in quella posizione — nulla da visualizzare, e quindi nulla da cliccare o inviare. Non visualizzare un segnaposto né collegare un gestore di clic per un campo icona vuoto.

### `I1` — Clic su preicon1 di proprietà {#i1}

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

### `I2` — Clic su preicon2 di proprietà {#i2}

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

### `I3` — Clic su posticon1 di proprietà {#i3}

Stessa struttura di `I1` / `I2`.

### `I4` — Clic su posticon2 di proprietà {#i4}

Stessa struttura di `I1` / `I2`.

---

### `J1` — Clic su preicon di elemento {#j1}

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

### `J2` — Clic su posticon di elemento {#j2}

Stessa struttura di `J1`.

---

## Gestione dei profili

### `PL` — Caricare un profilo {#pl}

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

Il server risponde con un messaggio `fl` (profilo caricato) poi invia i valori delle proprietà aggiornati.

{{% notice style="note" title="Un `fc` transitorio precede `fl`" %}}
Il server invia un messaggio [`fc`]({{% relref "server-messages.it.md" %}}#fs) (modifiche non salvate) immediatamente prima di `fl`, come passaggio intermedio del caricamento del profilo, prima che i valori della proprietà cambino effettivamente. Un client che reagisce a `fc` mostrando un indicatore "modifiche non salvate" lo vedrà lampeggiare per un messaggio e autocorreggersi non appena arriva `fl` subito dopo — atteso, non un bug.
{{% /notice %}}

---

### `PS` — Salvare un profilo {#ps}

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

Il server risponde con un messaggio `fs` (profilo salvato).

---

## Operazioni sulle griglie

I comandi di griglia sono destinati a una proprietà specifica all'interno di un modulo. Tutti usano l'involucro `m` / `p`.

### `GC` — Creare una riga di griglia {#gc}

Aggiunge una nuova riga alla griglia con i valori degli elementi forniti.

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

Il server risponde con un messaggio `gc` che confirma l'indice della nuova riga.

{{% notice style="note" title="Applicate ogni `ea` così com'è — niente da chiedersi" %}}
Creare una riga di griglia applica prima i valori inviati agli elementi di base della proprietà, poi aggiunge la riga ed emette `gc`. Questo può richiedere più di un messaggio `ea` (uno per ogni elemento di base modificato, inviati uno dopo l'altro — inviare `target`/`exposure`/`count` come sopra produce tre messaggi `ea` distinti prima del `gc` finale), ma un client non ha mai bisogno di conoscere o prevedere questo numero: il server invia sempre e solo lo stretto necessario per mantenere corretto il datastore del client, né più né meno (vedi [Principi di architettura client]({{% relref "client-pattern.it.md" %}})). Applicate ogni messaggio esattamente come arriva, come fate sempre, e il risultato è corretto indipendentemente dal numero di messaggi ricevuti.

L'unico effetto visibile: una proprietà che mostra sia i suoi elementi di base sia la sua griglia contemporaneamente (`showElts` e `showGrid` entrambi attivi — combinazione rara) sfarfallerà attraverso questi stati intermedi prima che la nuova riga appaia. Questo è il vero stato del datastore, non un bug, e si risolve da solo — non serve filtrarlo né fare polling per aggirarlo.
{{% /notice %}}

---

### `GU` — Aggiornare una riga di griglia {#gu}

Aggiorna una riga specifica (indice a base zero `i`) con nuovi valori degli elementi.

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

### `GD` — Eliminare una riga di griglia {#gd}

Elimina la riga all'indice `i` (base zero).

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

### `GF` — Selezionare una riga di griglia {#gf}

Chiede al server di caricare una riga della griglia negli elementi editabili della proprietà (usato quando si clicca su una riga per modificarla).

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

### `GH` — Spostare in alto una riga di griglia {#gh}

Sposta la riga all'indice `i` di una posizione verso l'alto.

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

### `GB` — Spostare in basso una riga di griglia {#gb}

Sposta la riga all'indice `i` di una posizione verso il basso.

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

### `GR` — Azzerare la griglia {#gr}

Svuota completamente la griglia di una proprietà (tutte le righe rimosse) — non una singola riga come `GD`. Il server diffonde [`gr`]({{% relref "server-messages.it.md" %}}#gr) in risposta.

```json
{
  "GR": {
    "m": {
      "Sequencer": {
        "p": {
          "sequence": {}
        }
      }
    }
  }
}
```

Nessun campo `i`: `GR` riguarda l'intera griglia, non una singola riga.

---

## Ciclo di vita dei moduli

### `CL` — Caricare una configurazione {#cl}

Carica tutti i moduli di una configurazione salvata (elenco in `controllerdata.availableconfs`, vedi [Controller]({{% relref "controller.it.md" %}})).

```json
{ "CL": { "name": "modules" } }
```

---

### `CS` — Salvare la configurazione corrente {#cs}

Registra l'insieme dei moduli attualmente caricati con il nome indicato (`GlobalDatastore` non è mai incluso).

```json
{ "CS": { "name": "modules" } }
```

---

### `ML` — Caricare un modulo {#ml}

Chiede al server di caricare dinamicamente un modulo. Questi comandi si rivolgono al controller: il payload **non** usa l'involucro `m`/`p`.

```json
{ "ML": { "lib": "focus", "label": "MyFocuser", "profile": "default" } }
```

| Campo | Descrizione |
|---|---|
| `lib` | Libreria del modulo, **senza** il prefisso `libost` (lo aggiunge il server): `focus` per `libostfocus`. Le librerie disponibili sono le chiavi di `controllerdata.libraries` |
| `label` | Etichetta di questa istanza; il **nome del modulo** è questa etichetta senza spazi. Se un modulo con questo nome è già caricato, la richiesta viene rifiutata |
| `profile` | Nome del profilo da caricare (facoltativo) |

{{% notice style="warning" title="Non passare una chiave di controllerdata.libraries così com'è" %}}
Le chiavi di `controllerdata.libraries` sono già con prefisso (`libostfocus`, `libostdummy`...). Il server aggiunge di nuovo `libost` incondizionatamente, quindi inoltrare una chiave direttamente la prefissa due volte e fallisce:

```json
{ "ML": { "lib": "libostdummy", "label": "MyDummy" } }
```
```
Error loading library MyDummy - Cannot load library libostlibostdummy: cannot open shared object file: No such file or directory
```

Rimuovere prima il prefisso `libost` dalla chiave (stessa trasformazione su cui si basa la convenzione di chiave di `profiles`, vedi [Controller data]({{% relref "controller.it.md" %}})):

```json
{ "ML": { "lib": "dummy", "label": "MyDummy" } }
```
{{% /notice %}}

---

### `MK` — Scaricare un modulo {#mk}

Chiede al server di scaricare un modulo in esecuzione, indicato dal suo nome.

```json
{ "MK": { "name": "MyFocuser" } }
```

`GlobalDatastore` non può essere scaricato: la richiesta viene rifiutata (messaggio di errore nei log).

---

## Interrogare un modulo

### `QY` — Interrogare un modulo {#qy}

{{% notice style="warning" title="Comunicazione inter-modulo, non una funzionalità client" %}}
`QY`/`qa` esiste principalmente perché un modulo possa porre una domanda a un altro modulo internamente (un modulo che chiama `Basemodule::otherModuleQuery()`, che passa per questo stesso meccanismo). Un client può tecnicamente inviare `QY` anche lui — l'involucro è identico — ma oggi non esiste un caso d'uso lato client consolidato. Documentato qui per completezza, e perché sarà rilevante quando si documenterà come costruire un modulo — non come qualcosa su cui un client dovrebbe costruire una funzionalità.
{{% /notice %}}

Meccanismo generico di richiesta/risposta specifico per modulo: porre a un modulo una domanda nominata con parametri, ricevere una risposta nominata tramite [`qa`]({{% relref "server-messages.it.md" %}}#qa). Quali nomi di richiesta un modulo comprenda realmente (se ne comprende) dipende interamente da quel modulo — il protocollo definisce solo l'involucro. Da notare: nessun involucro `p` qui, `QY` punta al modulo stesso, non a una sua proprietà.

```json
{
  "QY": {
    "m": {
      "Sequencer": {
        "query": "profileduration",
        "params": { "profile": "default" }
      }
    }
  }
}
```

{{% notice style="note" title="Ancora poco da vedere" %}}
Oggi solo il modulo `Sequencer` implementa una vera risposta (`profileduration`), usata internamente dal modulo Planner. Nessun altro modulo distribuito risponde in modo significativo a nessun nome di richiesta per ora.
{{% /notice %}}

---

## File system

### `FS` — Scegliere la cartella sorvegliata {#fs}

Indica la cartella di cui il server pubblica l'elenco dei file (vedi [File system]({{% relref "controller.it.md" %}}#fichiers)). Il percorso è **relativo al webroot**; la stringa vuota indica la radice.

```json
{ "FS": { "folder": "/Sequencer/gam Cyg/LIGHT/Luminance" } }
```

Il cambiamento è **globale per il server** (tutti i client vedono il nuovo elenco). Il percorso non viene validato: una cartella inesistente dà un elenco vuoto.

---

## Controllo del server INDI integrato {#indi}

Questi comandi sono disponibili solo quando il server dispone di un'istanza INDI integrata.

| Comando | Descrizione |
|---|---|
| `YA` | Avviare il server INDI integrato |
| `YZ` | Arrestare il server INDI integrato |
| `YL` | Caricare un driver INDI (`{"YL": {"driver": "<binary>"}}`) |
| `YR` | Ricaricare un driver INDI (`{"YR": {"driver": "<binary>"}}`) |
| `YS` | Arrestare un driver INDI (`{"YS": {"driver": "<binary>"}}`) |
| `YC` | Connettere il device di un driver INDI (`{"YC": {"driver": "<binary>"}}`) |
| `YD` | Disconnettere il device di un driver INDI (`{"YD": {"driver": "<binary>"}}`) |
| `YK` | Caricare una configurazione di driver INDI salvata (`{"YK": {"name": "<nome configurazione>"}}`) |

`YA` e `YZ` non hanno payload (`{"YA": {}}`). Per `YL`, `YR`, `YS`, `YC` e `YD`, `driver` è il campo `binary` di una voce di `controllerdata.indidrivers`. Per `YK`, `name` è una chiave di `controllerdata.indiconfigurations` (vedi [Controller]({{% relref "controller.it.md" %}})) — avvia tutti i driver salvati con quel nome, indipendentemente da `CL`/`CS`, che invece raggruppano i driver attivi in una configurazione di moduli.

{{% notice style="tip" title="Caricare un driver non connette il suo device" %}}
`YL` avvia solo il processo del driver - il device deve ancora essere connesso prima di essere utilizzabile, con `YC` (la proprietà INDI `CONNECTION.CONNECT`). È un passaggio separato di proposito: INDI stesso modella "processo attivo" e "device connesso" in modo indipendente. Come `YA`/`YZ`, `YC`/`YD` non hanno un riscontro dedicato (vedi l'avviso sopra): al momento non esiste alcun campo `controllerdata` che indichi se il device di un driver è effettivamente connesso, solo se il suo processo è attivo (`indiactivedrivers`).
{{% /notice %}}

---

## Riepilogo del controllo degli accessi

| Valore grant server | Comandi autorizzati |
|---|---|
| `grant-client: "1"` | Tutti i comandi |
| `grant-client: "0"` | Solo `DU`, `LO`, `IL` |
| `grant-client: "-1"` | Solo `LO` (autenticazione richiesta) |

I comandi inviati senza diritti sufficienti vengono silenziosamente ignorati dal server.
