---
title: Modello dati
weight: 20
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

Il server può essere scoperto sulla rete locale tramite mDNS/Zeroconf: si annuncia con il tipo di servizio `_ostserver_ws._tcp` nel dominio `local`, con un record TXT `OstServer=Observatoire Sans Tete`. La porta annunciata è sempre `9624` (la porta WebSocket non è configurabile). La scoperta è opzionale — un client che conosce già l'host può connettersi direttamente.

Alla connessione, inviare una richiesta `DU` (vedi [Client → Server]({{< relref "client-messages.it.md" >}})).  
Il server risponde con un messaggio dump completo (`d`) contenente tutti i moduli, i dati del controller e gli elenchi di file.

I dati propri del controller (carico del server, configurazioni dei moduli, driver INDI, file system) non fanno parte del modello a moduli descritto in questa pagina — vedi [Controller, GlobalDatastore e file]({{% relref "controller.it.md" %}}).

---

## Moduli

Un modulo rappresenta un sottosistema funzionale (messa a fuoco, guida, camera allsky, ecc.).  
Il suo identificatore è il suo nome interno (senza spazi).

| Campo | Tipo | Descrizione |
|---|---|---|
| `infos.modulelabel` | string | Nome visualizzato del modulo |
| `infos.description` | string | Descrizione del modulo |
| `infos.template` | string | Tipo di modulo (es. `"focus"`, `"guider"`) — può essere assente |
| `properties` | object | Mappa nome proprietà → oggetto proprietà |
| `globallovs` | object | Mappa chiave LOV → oggetto LOV (liste di valori a livello modulo) |
| `profile.name` | string | Nome del profilo attivo |
| `profile.changed` | bool | Indica se il profilo ha modifiche non salvate |

{{% notice style="info" title="Formato wire" %}}
Nel formato JSON di rete, `properties` è abbreviato in `p`, `globallovs` in `l`, e `profile` in `f`. I client devono gestire queste chiavi abbreviate.
{{% /notice %}}

{{% notice style="note" title="Campi interni" %}}
`infos` porta anche diversi campi interni di build/versione (`baseGithash`, `thisversion`, `startdatetime`, e altri) — dettaglio implementativo, non fa parte del contratto stabile lato client, non documentato individualmente qui.
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

{{% notice style="tip" title="`level1`/`level2`/`order` sono informativi, non un contratto" %}}
Questi campi sono puramente informativi — il backend non li impone mai, e un client è libero di rispettarli parzialmente o per niente. Esprimono solo il modo in cui lo sviluppatore di un modulo ha pensato di organizzare le proprie proprietà. È esattamente ciò che permette a client generici (Rooster, ostnoctua) di adattarsi a qualsiasi modulo senza UI dedicata per modulo.

Il client di riferimento (Rooster) raggruppa le proprietà così, per un client che punti a un layout generico simile:
1. `level1` vuoto → proprietà mostrata fuori da qualsiasi scheda (renderizzata direttamente, ordinata per `order`).
2. `level1` valorizzato → raggruppata in una scheda chiamata `level1`. Al suo interno, `level2` vuoto → mostrata direttamente sotto la scheda; `level2` valorizzato → in un gruppo comprimibile chiamato `level2`.
3. L'ordine delle schede (e dei gruppi all'interno di una scheda) non è un campo separato — si deduce dal valore `order` più piccolo tra le proprietà di quel gruppo.
4. All'interno di un gruppo, le proprietà sono ordinate per il proprio `order`.

Due cose da sapere se si costruisce qualcosa di simile:
- `level1`/`level2` formano una "falsa gerarchia" per scelta di progettazione, non una struttura a due livelli garantita — nulla impedisce a un modulo di lasciare `level1` vuoto valorizzando `level2` (Rooster non gestisce nemmeno questa combinazione: la proprietà finisce semplicemente nel gruppo "senza level1", `level2` ignorato).
- `level1`/`level2` sono etichette tradotte, tramite lo stesso meccanismo di `label`/`hint`. Usare il valore grezzo della stringa come chiave di stato dell'interfaccia (es. un pannello espanso/compresso) è fragile al cambio di lingua lato client — non un bug, solo da evitare.
{{% /notice %}}

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
| `directedit` | bool | Indica se questo elemento può essere scritto da solo via `SV` — vedi sotto |
| `preicon` | string | Icona prima dell'elemento (opzionale) |
| `posticon` | string | Icona dopo l'elemento (opzionale) |

{{% notice style="warning" title="Aggiornamenti parziali" %}}
Nei messaggi di aggiornamento `ee` e `ea`, gli elementi contengono **solo il loro valore** — senza i campi di metadati. I client devono conservare i metadati dell'ultimo dump completo.
{{% /notice %}}

{{% notice style="tip" title="Omissione del campo" %}}
`directedit` può essere del tutto assente per gli elementi in cui non si applica — considerare un'assenza come `false`. Questo alleggerisce i dump delle proprietà che non ne hanno bisogno.
{{% /notice %}}

{{% notice style="note" title="Significato di `directedit`" %}}
`true` significa che questo elemento può essere aggiornato da solo, indipendentemente dagli altri elementi della proprietà — inviare `SV` per esso (funziona anche `SA`). `false` significa che il client deve inviare **tutti** gli elementi della proprietà insieme — usare sempre `SA`, mai `SV` per quell'elemento da solo. Un esempio significativo: coordinate RA/Dec, dove entrambi i valori devono essere dati insieme.

Il backend lo impone: un `SV` su un elemento con `directedit: false` viene rifiutato con un errore nel log del modulo e non viene applicato nulla. `SA` è sempre accettato.

Scegliere il valore in base al comportamento dell'elemento, non a quanto «sembra legato» ai vicini:
- **`true` per un elemento indipendente** — un `bool` che può essere vero indipendentemente dai fratelli (pulsante d'azione come start/abort/loop, interruttore) deve avere `directedit: true`. Un valore simile riflette spesso un'attività in corso e più valori possono essere veri insieme (es. un ciclo di acquisizione in corso mentre si richiede un focus out). Dichiararlo `false` affermerebbe a torto che gli elementi vanno inviati in gruppo, e un client che segue questa pagina invierebbe `SA`, aggirando i moduli che gestiscono solo `SV`.
- **`false` solo per un vero gruppo** — elementi che hanno senso solo insieme: coordinate RA/Dec, o un vettore di switch INDI riprodotto uno a uno.

Per un gruppo `false` di elementi `bool` con regola esclusiva (`OneOfMany` o `AtMostOne`), selezionare un elemento significa inviargli `true` e **tutti i fratelli `false`** nello stesso `SA` — mai rinviare i valori correnti dei fratelli, altrimenti due elementi finiscono `true`.
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
| `nullable` | bool | Indica se l'elemento può essere lasciato vuoto (il frontend deve offrire una scelta "nessuno") |

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
| `lovConstrained` | bool | Indica se il valore deve appartenere al LOV |
| `nullable` | bool | Indica se l'elemento può essere lasciato vuoto (il frontend deve offrire una scelta "nessuno") |

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
| `min` / `max` / `mean` / `median` / `stddev` | float[] | Statistiche per canale (vedi sotto) |
| `histogram` | array | Dati dell'istogramma per canale (vedi sotto) |
| `alternates` | string[] | Opzionale: versioni alternative dell'immagine |

**Statistiche per canale**: `min`/`max`/`mean`/`median`/`stddev` sono
array a **3 posizioni fisse**, indipendentemente dal numero reale di
canali — indice `0` = canale unico (immagine mono) o rosso (R), `1` =
verde (G), `2` = blu (B). Solo le prime `channels` posizioni sono
significative; oltre, i valori sono a zero e privi di significato
(un'immagine mono ha valorizzato solo l'indice `0`).

{{% notice style="note" title="Istogramma: struttura dei bin" %}}
`histogram` segue la stessa logica per canale (un array per canale,
solo le prime `channels` posizioni usate), ma con due particolarità
da conoscere prima di usarlo:

- **Ogni bin è a sua volta incapsulato in un array di un elemento**:
  `histogram[canale][bin]` vale `[frequenza]`, non direttamente
  `frequenza` — un artefatto dell'attuale serializzazione lato
  backend, non un errore di lettura lato client.
- **Il valore di pixel rappresentato da ogni bin non viene trasmesso
  direttamente**, solo la frequenza lo è. Il numero di bin varia da
  un'immagine all'altra (`histogram[canale].length`); il valore di
  pixel del bin `i` si ricalcola da `min`/`max` dello stesso canale:
  ```
  larghezza_bin = (max[canale] - min[canale]) / (numero_di_bin - 1)
  valore(i)     = min[canale] + i × larghezza_bin
  ```
{{% /notice %}}

{{% notice style="tip" title="URL media" %}}
Tutti i percorsi immagine/video sono relativi. Aggiungere il prefisso `http(s)://hostname/ostmedia/` per ottenere l'URL completo.  
Esempio: `"urljpeg": "Focus/frame.jpeg"` → `http://hostname/ostmedia/Focus/frame.jpeg`
{{% /notice %}}

{{% notice style="warning" title="I media sono serviti separatamente dalla porta WebSocket" %}}
Questo `hostname` **non è deducibile dal protocollo** — nessun campo del dump lo fornisce, e non è l'host della connessione WebSocket appena stabilita. Nel deployment di riferimento (vedi [Installazione pulita]({{% relref "fresh.it.md" %}})), i media sono serviti da un server web separato (nginx, porta 80 di default, 443 dietro SSL) — una porta completamente diversa dal server WebSocket (9624). Un client deve già conoscere, o far configurare all'utente, l'hostname del server OST indipendentemente dall'URL WebSocket, e costruire gli URL dei media usando la porta HTTP di quell'hostname — non l'origine dei propri file, né la porta WebSocket. Un client che si limita ad anteporre un percorso `/ostmedia/` nudo (risolto rispetto a dove sono serviti i propri file) otterrà immagini rotte.
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
