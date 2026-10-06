---
title: Inspector
weight: 70
description: Analisi della qualità ottica (HFR, forma delle stelle) e assistente di collimazione
---

## Ruolo

Il modulo Inspector effettua due analisi indipendenti su un campo di stelle. Ciascuna si attiva separatamente — nessuna, una, l'altra, o entrambe insieme:

- **Analisi Inspector** — qualità ottica sull'intera immagine, su un'immagine *a fuoco*. Produce tre mappe complementari (HFR per zona, elongazione delle stelle, mosaico degli angoli) che permettono di diagnosticare difetti ottici come il tilt, il coma o la curvatura di campo.
- **Analisi Collimator** — collimazione ottica (allineamento degli specchi) di telescopi Newton, RC (Ritchey-Chrétien) e SC (Schmidt-Cassegrain), su un'immagine *defocalizzata*. Analizza l'intero campo di stelle e mostra continuamente lo scostamento di collimazione e le correzioni per vite. In semi-manuale: il software defocalizza, acquisisce e analizza in loop mentre l'utente agisce fisicamente sulle viti di collimazione (non motorizzate) osservando il feedback visivo aggiornarsi in tempo quasi reale.

L'analisi può essere effettuata su un'immagine acquisita in diretta dalla camera, o su un file FITS caricato dal disco.

![Screenshot del modulo Inspector](/images/modules/inspector.png)

## Dispositivi richiesti

| Dispositivo | Ruolo |
|---|---|
| Camera | Acquisizione delle immagini (opzionale se caricamento da file) |
| Focheggiatore (facoltativo) | Pilotaggio automatico della defocalizzazione per il flusso di collimazione, e funzione di posizione "home". Se assente, defocalizzazione manuale prima di ogni acquisizione |

## Selezione delle analisi

| Interruttore | Effetto |
|---|---|
| **Analisi Inspector** | Avvia l'analisi HFR / forma / angoli su ogni immagine acquisita (o ricaricata) |
| **Analisi Collimator** | Avvia l'analisi di collimazione su ogni immagine acquisita (o ricaricata) |

I due interruttori sono indipendenti (non esclusivi) e sono salvati con il profilo del modulo. Quando un'analisi è disattivata, le sue immagini di risultato vengono eliminate per non mostrare mai una mappa obsoleta accanto a una mappa fresca proveniente da un'altra immagine.

## Parametri

### Acquisizione

| Parametro | Descrizione |
|---|---|
| **Esposizione** | Durata dell'esposizione in secondi |
| **Guadagno** | Guadagno della camera |
| **Offset** | Offset della camera |
| **Focale** | Focale ottica in mm — usata per convertire gli HFR e il vettore di collimazione in secondi d'arco |
| **Dimensione degli angoli (pixel)** | Larghezza in pixel delle porzioni estratte per il mosaico degli angoli |

### Analisi

| Parametro | Descrizione |
|---|---|
| **Zoning** | Griglia N×N di suddivisione dell'immagine per l'analisi per zona (da 1×1 a 64×64) |

### Rilevamento della collimazione

Questi parametri limitano e regolano il rilevamento dei donut per l'analisi di collimazione (vedi «Rilevamento per stella» più sotto) — valori predefiniti pensati per funzionare senza alcuna regolazione, da modificare solo se necessario.

| Parametro | Predefinito | Descrizione |
|---|---|---|
| **Deviazione standard minima (ADU)** | 20 | Sotto questo valore, l'immagine è considerata piatta (sovraesposta, sottoesposta, nessun donut) e l'analisi di collimazione viene saltata interamente per quella acquisizione |
| **Dimensione massima candidato (% immagine)** | 5 % | Un contorno più grande di questa frazione dell'immagine viene scartato d'ufficio (non è mai un vero donut — tipicamente un artefatto di saturazione) |
| **Margine di rilevamento (0-255)** | 5 | Contrasto locale minimo richiesto perché un pixel sia considerato parte di un donut. **Diminuire** per rilevare donut più deboli/meno contrastati (con il rischio di catturare più rumore — già limitato dagli altri parametri qui) |
| **Finezza di rilevamento (divisore)** | 8 | Controlla la dimensione della finestra di analisi locale (dimensione immagine / N). **Aumentare** (finestra più piccola, più locale) aiuta anche a rilevare donut deboli immersi in uno sfondo leggermente irregolare |
| **Raggio minimo candidato (pixel)** | 5 | Scarta i candidati troppo piccoli/puntiformi per essere un vero donut defocalizzato — evita che un campo di stelle non sufficientemente defocalizzato (punti quasi puntiformi) inquini l'adattamento di collimazione con rumore |

### Defocalizzazione

| Parametro | Descrizione |
|---|---|
| **Offset focheggiatore (step)** | Spostamento del focheggiatore (in step) applicato da *Go intra* / *Go extra* per raggiungere la posizione defocalizzata |

## Mappe prodotte (analisi Inspector)

### Mappa HFR

La mappa HFR divide l'immagine in una griglia N×N (secondo il parametro *Zoning*). Per ogni zona, viene calcolata l'HFR media su tutte le stelle rilevate, poi levigata con una media delle 8 celle vicine.

Il colore di ogni zona va dal verde (miglior HFR) al rosso (HFR peggiore).

In sovrapposizione:
- Cerchi blu attorno a ogni stella rilevata, di raggio proporzionale all'HFR individuale
- Rettangolo bianco che collega i centri dei quattro quadranti, la cui forma riflette la distribuzione dell'HFR: un quadrato perfetto indica un'uniformità ideale
- Valore HFR di ogni quadrante (alto-sinistra, alto-destra, basso-sinistra, basso-destra) e HFR globale, espressi in secondi d'arco

### Mappa di forma (aberrazioni)

La mappa di forma analizza l'elongazione delle stelle per zona. Per ogni zona della griglia, viene disegnato un tratto:

- **Orientamento**: direzione dell'asse principale delle stelle (angolo medio)
- **Lunghezza**: proporzionale all'elongazione `a/b − 1` (rapporto asse maggiore / asse minore)

Un tratto corto e uniforme su tutta l'immagine indica stelle rotonde. Tratti lunghi in periferia segnalano coma o astigmatismo.

### Mosaico degli angoli

Il mosaico assembla nove porzioni della dimensione *Dimensione degli angoli* estratte da nove posizioni dell'immagine originale, disposte in una griglia 3×3:

```
Alto-sinistra   |  Alto-centro   |  Alto-destra
Centro-sinistra |  Centro        |  Centro-destra
Basso-sinistra  |  Basso-centro  |  Basso-destra
```

Questa vista permette di confrontare direttamente la qualità delle stelle al centro e ai bordi del campo.

## Analisi di collimazione

Una stella defocalizzata appare come un anello ("donut") la cui forma e centratura forniscono informazioni sulla collimazione. A differenza di un approccio classico che confronta una singola stella in intra-focale ed extra-focale, l'analisi di collimazione utilizza **tutte le stelle di un unico campo, in una sola acquisizione**:

1. Defocalizzazione (automatica tramite il focheggiatore, o manuale)
2. Acquisizione di un'immagine contenente più stelle distribuite nel campo
3. Rilevamento di ogni donut utilizzabile e calcolo di un vettore di deformazione per stella
4. Regressione sull'insieme dei vettori per determinare il vettore di collimazione globale
5. Conversione in correzione da applicare su ciascuna delle 3 viti
6. Nuova acquisizione, nuova analisi, loop continuo fino alla convergenza

### Perché il campo pieno basta

Per una stella alla posizione `P` nell'immagine (riferimento centrato sull'immagine), il vettore di deformazione osservato segue un modello lineare:

```
D(P) = C - k·P
```

dove `C` è il vettore di collimazione ricercato (costante, indipendente dalla posizione nel campo) e `k` raggruppa l'effetto del coma di campo (aberrazione ottica normale, presente anche con una buona collimazione, che cresce con la distanza dal centro e con l'apertura del telescopio). Un tale campo vettoriale ha sempre un punto di convergenza unico — con alcune stelle a posizioni di campo diverse, una regressione permette di separare la parte di collimazione dalla parte di coma, senza dover confrontare intra ed extra-focale.

I pulsanti **Go intra**/**Go extra** restano disponibili per una verifica manuale da parte dell'utente, ma non sono una fase del flusso automatizzato.

### Rilevamento per stella

Per ogni stella rilevata:
- Sogliatura adattiva locale poi affinata individualmente (una soglia globale unica svantaggerebbe le stelle più debole del campo)
- Estrazione del contorno esterno dell'anello e dell'ombra del secondario (se visibile)
- Un adattamento di cerchio ai minimi quadrati sul contorno esterno fornisce il centro teorico della stella — questo metodo resta affidabile anche quando l'anello è fortemente deformato (a mezzaluna, in caso di forte decollimazione)
- Il vettore di deformazione è lo scostamento tra questo centro teorico e il centroide pesato per intensità dell'anello (ombra esclusa)

Donut visibilmente presenti nell'immagine ma non rilevati indicano una soglia di rilevamento troppo stretta (diminuire il **Margine di rilevamento**, o aumentare la **Finezza di rilevamento**). Al contrario, un campo di stelle non sufficientemente defocalizzato (punti quasi puntiformi piuttosto che veri anelli) non fornisce alcun segnale di deformazione utilizzabile — il **Raggio minimo candidato** assicura che tali campi ricadano correttamente su «non sono stati rilevati donut sufficienti» invece di produrre un adattamento rumoroso e un punto di convergenza aberrante. Vedi «Rilevamento della collimazione» nei parametri sopra.

### Regressione e punto di convergenza

I vettori di deformazione di tutte le stelle rilevate sono adattati ai minimi quadrati al modello `D(P) = C - k·P`. Il vettore `C` ottenuto è convertito in secondi d'arco (focale e dimensione del pixel del sensore), poi proiettato sulle 3 viti di collimazione a 120° per ottenere un segno e un'ampiezza per vite.

{{% notice style="note" title="Nessuna calibrazione in giri di vite" %}}
L'ampiezza per vite è espressa in secondi d'arco, non in giri o frazioni di giro. La relazione tra uno scostamento ottico e un movimento fisico della vite dipende dal montaggio meccanico del secondario (passo, sensibilità), specifico a ogni strumento — invece di inventare una calibrazione approssimativa, la regolazione si fa a sensazione: l'ampiezza e il senso indicati danno la tendenza, l'utente regola e osserva l'aggiornamento in diretta.
{{% /notice %}}

### Mappa di collimazione

L'analisi di collimazione pubblica una propria immagine annotata (l'immagine principale resta un'immagine grezza). Comprende, per ogni stella rilevata:
- Il contorno esterno del donut
- Una freccia di deformazione (esagerata per restare visibile), colorata secondo l'ampiezza (verde / arancione / rosso)

E per l'insieme del campo:
- Un bullseye (anelli tratteggiati) centrato sull'immagine, che materializza le stesse soglie di tolleranza dell'indicatore di qualità
- Una linea che collega il centro teorico dell'immagine al punto di convergenza calcolato — assente se l'adattamento è considerato poco affidabile (punto di convergenza estrapolato troppo lontano fuori immagine, segno di un campo non sufficientemente defocalizzato o di troppo pochi donut utilizzabili)

#### Prima della collimazione (esempio)

![Overlay di collimazione, collimazione da correggere](/images/modules/collimator/collimator-ko.png)

#### Dopo la collimazione (esempio)

![Overlay di collimazione, collimazione corretta](/images/modules/collimator/collimator-ok.png)

### Valori mostrati (collimazione)

#### Vettore di collimazione

| Valore | Descrizione |
|---|---|
| **C.x / C.y (px)** | Vettore di collimazione, in pixel immagine |
| **Ampiezza (arcsec)** | Ampiezza del vettore di collimazione, convertita in secondi d'arco |
| **Punto di convergenza x/y (px)** | Punto di convergenza calcolato (dove i vettori di deformazione, prolungati, si incrociano) |
| **Quality** | Indicatore: verde se l'ampiezza è bassa, arancione in zona intermedia, rosso oltre la soglia alta |

#### Correzioni per vite

| Valore | Descrizione |
|---|---|
| **Screw 1 / 2 / 3 (turns)** | Ampiezza e senso di correzione per vite, espressa in secondi d'arco (positivo = stringere) — convenzione fissa a 120°, nessuna rilevazione automatica dell'orientamento reale delle viti |

#### Stelle rilevate

Griglia che elenca ogni stella utilizzabile dell'ultima analisi: posizione (x, y) e vettore di deformazione (dx, dy) in pixel.

## Stati

| Stato | Descrizione |
|---|---|
| **Idle** | Modulo inattivo |
| **Shooting** | Acquisizione immagine in corso |
| **Analyzing** | Analisi in corso |

## Posizione "home" del focheggiatore

*Set home* memorizza la posizione assoluta attuale del focheggiatore; *Go home* vi riporta poi il focheggiatore. La posizione **non** viene letta all'avvio e **non** è persistente — nulla garantisce che la messa a fuoco sia almeno approssimativa in quel momento, deve quindi essere definita esplicitamente. La proprietà **Focheggiatore home** indica se una posizione home è definita e il suo valore; finché *Set home* non è stato usato, *Go home* si limita a emettere un avviso e non fa nulla.

## Caricamento da file

Il campo **Seleziona un file** permette di caricare un file FITS dal disco per analizzarlo senza avviare un'acquisizione. L'analisi viene quindi avviata automaticamente all'inserimento del percorso, secondo la *selezione delle analisi* corrente.

## Azioni

| Azione | Descrizione |
|---|---|
| **Shoot** | Acquisisce una singola immagine e avvia le analisi attivate |
| **Loop** | Acquisizione e analisi in continuo — si ripete automaticamente fino all'arresto |
| **Stop** | Arresta il loop continuo |
| **Reload** | Ricarica e ri-analizza l'ultima immagine o l'ultimo file |
| **Set home** | Memorizza la posizione attuale del focheggiatore come posizione di ritorno |
| **Go home** | Riporta il focheggiatore alla posizione home memorizzata (avvisa se nessuna è stata definita) |
| **Go intra** | Sposta il focheggiatore verso la posizione intra-focale (verifica manuale) |
| **Go extra** | Sposta il focheggiatore verso la posizione extra-focale (verifica manuale) |

I pulsanti **Go intra** / **Go extra** / **Go home** restano premuti durante lo spostamento del focheggiatore, e vengono rilasciati quando raggiunge il target.
