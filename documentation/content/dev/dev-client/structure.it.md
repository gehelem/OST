---
title: Modello dati
weight: 10
description: "Gerarchia dei moduli, proprietà ed elementi scambiati via WebSocket"
---

## Panoramica

Tutti i dati sono organizzati secondo una gerarchia a tre livelli:

```
Module
  └─ Property (una o più per modulo)
       └─ Element (uno o più per proprietà)
```

---

## Connessione e stato iniziale

Alla connessione, inviare una richiesta `DU` (vedi [Client → Server]({{< relref "client-messages.it.md" >}})).  
Il server risponde con un messaggio dump completo (`d`) contenente tutti i moduli, i dati del controller e gli elenchi di file.

---

## Moduli

Un modulo rappresenta un sottosistema funzionale (messa a fuoco, guida, camera allsky, ecc.).  
Il suo identificatore è il suo nome interno (senza spazi).

| Campo | Tipo | Descrizione |
|---|---|---|
| `infos.label` | string | Nome visualizzato del modulo |
| `infos.name` | string | Nome interno del modulo |
| `infos.description` | string | Descrizione del modulo |
| `infos.template` | string | Tipo di modulo (es. `"focus"`, `"guider"`) |
| `properties` | object | Mappa nome proprietà → oggetto proprietà |
| `globallovs` | object | Mappa chiave LOV → oggetto LOV (liste di valori a livello modulo) |
| `profile.name` | string | Nome del profilo attivo |
| `profile.changed` | bool | Indica se il profilo ha modifiche non salvate |

{{% notice style="info" title="Formato wire" %}}
Nel formato JSON di rete, `properties` è abbreviato in `p`, `globallovs` in `l`, e `profile` in `f`. I client devono gestire queste chiavi abbreviate.
{{% /notice %}}

---

## Proprietà

Una proprietà raggruppa elementi collegati. Le proprietà sono mostrate in modo gerarchico tramite `level1` e `level2`.

| Campo | Tipo | Descrizione |
|---|---|---|
| `label` | string | Etichetta visualizzata |
| `order` | string | Ordine di ordinamento allo stesso livello |
| `level1` | string | Primo livello gerarchico (es. nome della scheda) |
| `level2` | string | Secondo livello gerarchico (es. nome del gruppo) |
| `status` | int | `0` Standby · `1` OK (verde) · `2` Occupato (giallo) · `3` Errore (rosso) |
| `permission` | int | `0` Solo lettura · `1` Solo scrittura · `2` Lettura-scrittura |
| `enabled` | bool | Indica se è possibile interagire con la proprietà |
| `badge` | bool | Indica se la proprietà è nell'elenco dei preferiti |
| `preicon1` | string | Icona prima dell'etichetta (nome icona Google Fonts) |
| `preicon2` | string | Seconda icona prima dell'etichetta |
| `posticon1` | string | Icona dopo l'etichetta |
| `posticon2` | string | Seconda icona dopo l'etichetta |
| `showElts` | bool | Indica se i valori degli elementi devono essere mostrati in linea |
| `hasprofile` | bool | Indica se la proprietà è salvata nei profili |
| `freevalue` | string | Campo di testo libero arbitrario |
| `rule` | int | Regola di raggruppamento degli elementi bool: `0` UnoTra (radio) · `1` AlPiùUno · `2` Qualsiasi |
| `elements` | object | Mappa nome elemento → oggetto elemento |

**Proprietà con griglia** (quando `hasGrid: true`):

| Campo | Tipo | Descrizione |
|---|---|---|
| `hasGrid` | bool | La proprietà contiene dati tabulari |
| `showGrid` | bool | La griglia deve essere mostrata di default |
| `gridLimit` | int | Numero massimo di righe nella griglia |
| `gridheaders` | string[] | Ordine delle colonne (nomi degli elementi) |
| `grid` | array di array | Righe della griglia; ogni riga è un array di valori nell'ordine di `gridheaders` |

**Proprietà con grafico** (quando `hasGraph: true`):

| Campo | Tipo | Descrizione |
|---|---|---|
| `hasGraph` | bool | La proprietà contiene un grafico |
| `graphType` | string | `"XY"` · `"DY"` (asse temporale) · `"PHD"` (guida) |
| `graphParams` | object | Configurazione del grafico |

{{% notice style="info" title="Formato wire" %}}
Nel formato JSON di rete, `elements` è abbreviato in `e`.
{{% /notice %}}

---

## Elementi

Un elemento contiene un unico valore tipizzato. Esistono 11 tipi di elementi.

### Campi comuni (tutti gli elementi, nel dump completo)

| Campo | Tipo | Descrizione |
|---|---|---|
| `type` | string | Tipo dell'elemento (vedi elenco più sotto) |
| `label` | string | Etichetta visualizzata |
| `order` | string | Ordine di ordinamento all'interno della proprietà |
| `hint` | string | Testo di aiuto / tooltip |
| `autoupdate` | bool | Indicatore interno del backend; ignorato dai client |
| `badge` | bool | Indica se l'elemento è nell'elenco dei preferiti |
| `directedit` | bool | `true` = inviare `SV` a ogni modifica; `false` = raggruppare con `SA` |
| `preicon` | string | Icona prima dell'elemento (opzionale) |
| `posticon` | string | Icona dopo l'elemento (opzionale) |

{{% notice style="warning" title="Aggiornamenti parziali" %}}
Nei messaggi di aggiornamento `ee` e `ea`, gli elementi contengono **solo il loro valore** — senza i campi di metadati. I client devono conservare i metadati dell'ultimo dump completo.
{{% /notice %}}

---

### Tipo: `int`

| Campo | Tipo | Descrizione |
|---|---|---|
| `value` | int | Valore intero corrente |
| `min` | int | Valore minimo consentito |
| `max` | int | Valore massimo consentito |
| `step` | int | Passo di incremento |
| `format` | string | Stringa di formattazione per la visualizzazione |
| `slider` | int | `0` nessuno slider · `1` solo slider · `2` slider + inserimento valore |
| `listOfValues` | object | Opzionale: mappa `{"chiave": "Etichetta"}` dei valori consentiti |
| `globallov` | string | Opzionale: chiave che fa riferimento a un LOV di modulo o di controller |
| `lovScope` | string | `"module"` o `"controller"` — dove è definito il globallov |
| `lovConstrained` | bool | Indica se il valore deve appartenere al LOV |

### Tipo: `float`

Stessi campi di `int`, con `value`, `min`, `max`, `step` in virgola mobile.

### Tipo: `bool`

| Campo | Tipo | Descrizione |
|---|---|---|
| `value` | bool | `true` o `false` |

### Tipo: `string`

| Campo | Tipo | Descrizione |
|---|---|---|
| `value` | string | Valore testo |
| `listOfValues` | object | Opzionale: mappa `{"chiave": "Etichetta"}` dei valori consentiti |
| `globallov` | string | Opzionale: chiave che fa riferimento a un LOV |
| `lovScope` | string | `"module"` o `"controller"` |

### Tipo: `date`

| Campo | Tipo | Descrizione |
|---|---|---|
| `value` | object | `{"year": int, "month": int (1-12), "day": int (1-31)}` |

### Tipo: `time`

| Campo | Tipo | Descrizione |
|---|---|---|
| `value` | object | `{"hh": int, "mm": int, "ss": int, "ms": int}` |
| `usems` | bool | Indica se i millisecondi devono essere mostrati |

### Tipo: `datetime`

| Campo | Tipo | Descrizione |
|---|---|---|
| `value` | object | `{"year": int, "month": int, "day": int, "hh": int, "mm": int, "ss": int, "ms": int}` |

### Tipo: `img`

Il campo `value` è un oggetto contenente i metadati dell'immagine:

| Campo | Tipo | Descrizione |
|---|---|---|
| `urljpeg` | string | Percorso relativo verso l'immagine JPEG (da usare con l'URL di base `/ostmedia/`) |
| `urlfits` | string | Percorso relativo verso il file FITS |
| `urlthumbnail` | string | Percorso relativo verso la miniatura |
| `urloverlay` | string | Percorso relativo verso l'immagine di sovrapposizione |
| `channels` | int | Numero di canali colore |
| `width` / `height` | int | Dimensioni dell'immagine in pixel |
| `snr` | float | Rapporto segnale/rumore |
| `hfravg` | float | HFR medio (raggio a mezzo flusso) delle stelle rilevate |
| `stars` | int | Numero di stelle rilevate |
| `issolved` | bool | Indica se la risoluzione di campo è riuscita |
| `solverra` / `solverde` | float | Coordinate RA/DEC risolte |
| `solverorientation` | float | Angolo di rotazione del campo risolto |
| `min` / `max` / `mean` / `median` / `stddev` | float[] | Statistiche per canale |
| `histogram` | array | Dati dell'istogramma per canale |
| `alternates` | string[] | Opzionale: versioni alternative dell'immagine |

{{% notice style="tip" title="URL media" %}}
Tutti i percorsi immagine/video sono relativi. Aggiungere il prefisso `http(s)://hostname/ostmedia/` per ottenere l'URL completo.  
Esempio: `"urljpeg": "Focus/frame.jpeg"` → `http://hostname/ostmedia/Focus/frame.jpeg`
{{% /notice %}}

### Tipo: `video`

| Campo | Tipo | Descrizione |
|---|---|---|
| `value` | object | `{"url": "percorso/relativo/verso/video.mp4"}` |

Nei messaggi di aggiornamento parziale (`ea`), l'elemento video può contenere `url` direttamente a livello dell'elemento invece che annidato in `value`.

### Tipo: `light`

| Campo | Tipo | Descrizione |
|---|---|---|
| `value` | int | `0` Standby · `1` OK · `2` Avviso · `3` Errore |

### Tipo: `prg`

| Campo | Tipo | Descrizione |
|---|---|---|
| `value` | object | `{"value": float (0–100), "dynlabel": string}` |
| `prgtype` | string | `"bar"` o `"spinner"` |

### Tipo: `message` (non ancora implementato nel backend attuale)

Riservato per la visualizzazione di messaggi/log.

---

## Liste di valori (LOV)

I LOV forniscono un insieme di valori consentiti per gli elementi di tipo `int`, `float` e `string`.

**LOV locale** — integrata nell'elemento:
```json
"listOfValues": {
  "1": "Opzione Uno",
  "2": "Opzione Due"
}
```

**LOV globale** — referenziata per chiave:
```json
"globallov": "myCameraModels",
"lovScope": "module"
```

Il LOV stesso è definito a livello modulo (`globallovs`) o controller (`controllerlovs`):
```json
"myCameraModels": {
  "label": "Modelli di camera",
  "type": "string",
  "values": {
    "ASI294MC": "ZWO ASI 294 MC",
    "ASI533MC": "ZWO ASI 533 MC"
  }
}
```

**LOV integrati del controller** (sempre disponibili):

| Chiave | Contenuto |
|---|---|
| `loadedModules` | Tutti i moduli caricati: `{nomeModulo: etichetta}` |
| `loadedModules-<template>` | Moduli filtrati per tipo (es. `loadedModules-focus`) |
| `profiles-<template>` | Profili disponibili per un tipo di modulo |

---

## Controllo degli accessi

La risposta dump contiene i valori `grant-server` e `grant-client`:

| Valore | Significato |
|---|---|
| `"1"` | Accesso completo lettura-scrittura |
| `"0"` | Solo lettura |
| `"-1"` | Accesso negato (autenticazione richiesta) |

Se `grant-client` vale `"-1"`, non viene inviato alcun dato di modulo. Usare il messaggio `LO` (login) per autenticarsi.
