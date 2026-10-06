---
title: Focuser
weight: 10
description: Messa a fuoco automatica con adattamento polinomiale e mappatura del tilt del sensore
---

## Ruolo

Il modulo Focuser esegue la messa a fuoco automatica del telescopio. Sposta il focheggiatore motorizzato su un intervallo di posizioni, misura la dimensione delle stelle (HFR — Half Flux Radius) a ogni punto, poi calcola la posizione ottimale tramite un adattamento polinomiale di secondo grado. Con lo zoning attivato, produce anche una mappatura del tilt del sensore.

## Interfaccia

L'interfaccia mostra in tempo reale la curva HFR in costruzione, l'avanzamento della sequenza e i risultati (HFR finale e posizione). I parametri sono accessibili tramite il pulsante **Parametri** in alto a destra.

![Screenshot del modulo Focuser](/images/modules/focus.png)

La curva HFR è tracciata con barre d'errore che rappresentano la deviazione standard delle misure a ogni posizione, e un adattamento polinomiale appare come linea tratteggiata arancione con l'avanzare delle acquisizioni.

![Curva HFR con barre d'errore e adattamento polinomiale](/images/modules/focus/focus-chart.png)

## Dispositivi richiesti

| Dispositivo | Ruolo |
|---|---|
| Camera | Acquisizione delle immagini per la misura HFR |
| Focheggiatore motorizzato | Spostamento della messa a fuoco |
| Ruota portafiltri | Opzionale — utilizzata se configurata |

## Parametri

Questi parametri sono accessibili dal menu **Parametri** del modulo. Un set di parametri può essere salvato in un profilo.

### Esposizione

| Parametro | Descrizione |
|---|---|
| **Esposizione** | Durata dell'esposizione in secondi |
| **Guadagno** | Guadagno della camera |
| **Offset** | Offset della camera |

### Focus

| Parametro | Descrizione |
|---|---|
| **Posizione di partenza** | Posizione assoluta del focheggiatore per iniziare la sequenza (ignorata se *Intorno alla posizione attuale* è attivato) |
| **Intorno alla posizione attuale** | Centra automaticamente la sequenza intorno alla posizione corrente del focheggiatore: `pos_attuale − (passo × iterazioni / 2)` |
| **Passo** | Differenza in unità focheggiatore tra due misure consecutive |
| **Iterazioni** | Numero di punti di misura. Con 5 iterazioni e un passo di 2000, il focheggiatore esplora un intervallo di 8000 unità |
| **Backlash** | Overshoot applicato prima di ogni spostamento per correggere il gioco meccanico. Il focheggiatore arretra di questo valore prima di avanzare verso il target |
| **Zoning** | Suddivisione dell'immagine in zone (1×1 a 9×9). Vedi sezione dedicata più sotto |

### Rilevamento delle stelle

La misura HFR si basa su StellarSolver (estrazione di sorgenti SEP), con un'impostazione pensata per restare valida su tutto l'intervallo percorso dalla messa a fuoco — dal punto nitido fino alle estremità volontariamente defocalizzate a inizio/fine sequenza.

| Parametro | Predefinito | Descrizione |
|---|---|---|
| **Soglia di rilevamento (σ)** | 4 | Soglia di rilevamento, in multipli del rumore di fondo. Aumentarla riduce i falsi positivi (rumore del sensore rilevato come stella) |
| **Area min. di rilevamento (px)** | 20 | Area minima in pixel perché un rilevamento venga considerato valido — elimina i piccoli artefatti di rumore |
| **Contrasto di separazione (%)** | 5 % | Flusso minimo che un picco secondario deve rappresentare (in % del flusso totale) prima che SEP lo separi in un oggetto distinto. Una stella defocalizzata (anello irregolare, ombra del secondario) viene altrimenti facilmente scomposta in più «stelle» — aumentarlo per evitarlo |
| **Raggio min. della stella (px)** | 5 | Rigetta i rilevamenti più piccoli di questo raggio — filtra i frammenti residui di una sovra-segmentazione |

{{% notice style="note" title="Numero di stelle anormalmente elevato, o che varia bruscamente in defocalizzato" %}}
Se il numero di stelle rilevate appare molto superiore a quanto visibile a occhio nell'immagine, o aumenta nettamente alle estremità defocalizzate della sequenza, è il segno che queste impostazioni sono troppo permissive per il sensore/l'ottica utilizzata — aumentare la soglia di rilevamento e/o il contrasto di separazione indicati sopra.
{{% /notice %}}

## Algoritmo

La messa a fuoco si svolge in due fasi.

### Fase 1 — Esplorazione

1. Il focheggiatore si posiziona a `posizione_partenza − backlash` (per compensare il gioco meccanico)
2. Avanza alla `posizione_partenza`
3. Per ogni iterazione:
   - Reset del sotto-frame della camera
   - Acquisizione di un'immagine, calcolo dell'HFR e della sua deviazione standard
   - Registrazione della tripla (posizione, HFR, deviazione standard)
   - Aggiornamento dell'adattamento polinomiale in tempo reale
   - Avanzamento di un *passo*

L'adattamento polinomiale di secondo grado fornisce continuamente la posizione del minimo della curva HFR:

```
posizione_ottimale = −k1 / (2 × k2)    [per k0 + k1·x + k2·x²]
```

### Fase 2 — Posizionamento finale

1. Il focheggiatore arretra verso `migliore_posizione − backlash`
2. Avanza alla posizione ottimale calcolata dal polinomio
3. Viene acquisita un'immagine finale per misurare l'HFR risultante

{{% notice style="tip" title="Valore di ritorno" %}}
La posizione ottimale scelta è quella del **fit polinomiale** (`bestposfit`). Se il fit non è stato calcolabile (meno di 3 punti), viene usata la migliore posizione misurata direttamente (`bestpos`).
{{% /notice %}}

## Curva HFR

La curva HFR è tracciata in tempo reale durante l'esplorazione e aggiornata a ogni punto di misura. Comprende due elementi sovrapposti:

- **Punti misurati** (verde) con **barre d'errore** che rappresentano ±1 deviazione standard delle misure a ogni posizione
- **Curva polinomiale** (tratteggio arancione) — adattamento di secondo grado aggiornato a ogni nuovo punto

L'asse X rappresenta la posizione assoluta del focheggiatore, l'asse Y l'HFR in pixel.

{{% notice style="note" title="Barre d'errore" %}}
La deviazione standard mostrata corrisponde alla dispersione delle HFR individuali misurate per stella a una stessa posizione di messa a fuoco. Una barra d'errore ampia indica un seeing variabile o una turbolenza importante al momento dell'acquisizione.
{{% /notice %}}

## Zoning

Lo zoning divide l'immagine in una griglia di N×N zone e calcola un adattamento polinomiale indipendente per ciascuna zona. Permette sia un'analisi fine delle variazioni HFR sul campo che una mappatura del tilt del sensore.

| Valore | Griglia | Utilizzo |
|---|---|---|
| 1 | 1×1 | Nessuno zoning — una sola misura globale |
| 2 | 2×2 | Rilevamento basico del tilt (4 quadranti) |
| 3 | 3×3 | Analisi corrente — centro + bordi + angoli |
| 5 | 5×5 | Analisi fine |
| 9 | 9×9 | Analisi molto fine — 81 zone |

Per ogni zona, il modulo calcola tramite l'algoritmo di Welford (aggiornamento incrementale in un solo passaggio):
- L'**HFR media** della zona
- La **deviazione standard HFR** della zona — indicatore della qualità ottica locale

## Mappatura del tilt del sensore

Quando lo zoning è attivato (≥ 2×2), il modulo genera automaticamente quattro immagini di mappatura al termine della sequenza. Ciascuna rappresenta la posizione di messa a fuoco ottimale di ogni zona, codificata a colori (blu = posizione bassa / piano vicino, rosso = posizione alta / piano lontano).

![Esempi di mappature del tilt del sensore (4 immagini)](/images/modules/focus/focus-tilt-maps.png)

Il rapporto larghezza/altezza delle immagini corrisponde a quello del sensore.

### rawTilt — Tilt grezzo per zona

Ogni zona è rappresentata da un blocco di colore uniforme proporzionale alla posizione di miglior fuoco. Nessuna interpolazione — la risoluzione spaziale è quella della griglia di zoning.

![rawTilt — blocchi di colore per zona](/images/modules/focus/focus-rawtilt.png)

### localTiltLinear — Interpolazione bilineare locale

Interpolazione bilineare tra i centri delle zone adiacenti. Produce una mappa fluida senza ipotesi sulla forma globale del piano focale.

### globalTiltLinear — Piano globale (adattamento lineare GSL)

Adattamento di un piano ai minimi quadrati (libreria GSL) sull'insieme delle posizioni di zona. Ideale per quantificare un tilt puro: inclinazione del sensore rispetto all'asse ottico. Il residuo mostrato in ogni cella (in giallo) è la differenza tra la misura e il piano adattato.

### globalTiltQuadratic — Superficie quadratica (adattamento GSL di secondo grado)

Adattamento di una superficie di secondo grado. Rileva i difetti non planari: curvatura di campo, flessione del sensore. La gamma di colori copre il minimo e il massimo reali della superficie (vertice incluso se si trova nei limiti del sensore).

{{% notice style="note" title="Interpretazione" %}}
- **rawTilt / localTiltLinear**: diagnostiche locali, utili per individuare una zona anomala dovuta a una stella difettosa.
- **globalTiltLinear**: misura diretta del tilt. Se il gradiente è uniforme, inclinare il sensore nella direzione indicata basterà a correggere.
- **globalTiltQuadratic**: se il residuo del piano globale è importante e la superficie quadratica è nettamente migliore, la curvatura di campo o una flessione meccanica ne sono la causa.
{{% /notice %}}

## Valori mostrati in tempo reale

| Valore | Descrizione |
|---|---|
| **Stato** | Stato corrente della sequenza (Idle / Running / Ok / Error) |
| **Avanzamento** | Barra di avanzamento globale della sequenza |
| **HFR** | HFR misurata alla posizione finale |
| **Posizione** | Posizione finale del focheggiatore |

## Risultato finale

Al termine della messa a fuoco, il modulo pubblica:

| Risultato | Descrizione |
|---|---|
| **Posizione finale** | Posizione assoluta del focheggiatore dopo il posizionamento |
| **HFR finale** | HFR misurata alla posizione ottimale |

Questi valori sono anche pubblicati verso gli altri moduli (sequencer) tramite il sistema di eventi inter-modulo.

## Azioni

| Azione | Descrizione |
|---|---|
| **Avvia la messa a fuoco** | Avvia la sequenza completa |
| **Annulla** | Interrompe immediatamente la sequenza in corso |
