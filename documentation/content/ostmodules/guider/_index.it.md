---
title: Guider
weight: 20
description: Guida automatica con correzione della deriva
---

## Ruolo

Il modulo Guider assicura la guida automatica del telescopio. Rileva e segue un campo di stelle di riferimento, misura la deriva in tempo reale e invia impulsi di correzione alla montatura per mantenere il puntamento.

![Screenshot del modulo Guider](/images/modules/guider.png)

## Attrezzature richieste

| Attrezzatura | Ruolo |
|---|---|
| Camera di guida | Acquisizione delle immagini del campo di guida |
| Interfaccia di guida | Riceve gli impulsi temporizzati (`TELESCOPE_TIMED_GUIDE_*`) — la montatura stessa, o un dispositivo che trasmette impulsi ST4 |

## Fasi di funzionamento

La guida si svolge in tre fasi successive.

### Fase 1 — Inizializzazione

Il modulo acquisisce un'immagine di riferimento e conserva l'elenco delle stelle rilevate come riferimento per tutta la sessione di guida. Quando la guida viene avviata con una calibrazione salvata valida, solo questa fase precede la guida: un nuovo riferimento viene preso al puntamento corrente, il che permette di riutilizzare la calibrazione dopo uno spostamento della montatura.

### Fase 2 — Calibrazione

Il modulo misura la risposta della montatura agli impulsi di correzione.

1. Una serie fissa di impulsi viene inviata a Ovest, poi a Est, poi a Nord, poi a Sud (`Passo di calibrazione` impulsi per direzione).
2. Lo spostamento immagine per impulso di ogni passata è misurato rispetto all'immagine precedente.
3. **Il tasso di impulso è derivato dalle combinazioni antisimmetriche `(Ovest − Est) / 2` e `(Nord − Sud) / 2`.** Una passata unidirezionale è contaminata dalla deriva di fondo costante (errore di inseguimento siderale, errore periodico, deriva di messa in stazione); la differenza delle due passate opposte annulla questa deriva e fornisce un tasso RA e un tasso DEC puliti. I tassi grezzi per direzione restano visualizzati, a scopo diagnostico.
4. L'orientamento del sensore è dedotto dai due assi (una stima dall'asse RA e una dall'asse DEC, mediate), con l'ambiguità a 180° risolta rispetto all'asse RA.
5. Il tasso RA viene riportato all'equatore (`tasso × cos(DEC di calibrazione)`) per poter essere trasposto a qualsiasi declinazione al momento della guida.

Prima della serie Nord e prima della serie Sud, un impulso opzionale di *recupero del gioco DEC* assorbe prima il gioco meccanico dell'asse DEC, affinché gli impulsi di misura temporizzati non vadano in parte persi nel reingranaggio. Il suo spostamento non è incluso nel calcolo del tasso.

Il risultato della calibrazione viene salvato e riutilizzato da una sessione all'altra, purché il lato della forcella e l'orientamento della camera non siano cambiati.

{{% notice style="note" title="Qualità della calibrazione" %}}
Dopo le quattro passate, il modulo calcola un *rapporto di deriva* — l'ampiezza della deriva di fondo costante rispetto allo spostamento dovuto agli impulsi — e un indicatore **Qualità della calibrazione** (verde / arancione / rosso). Se il rapporto supera il *Rapporto di deriva di fondo max*, la calibrazione viene rifiutata e ripetuta, fino a *Numero di tentativi di calibrazione* volte, poi abbandonata con un messaggio che invita a verificare la messa in stazione. Una calibrazione viene abbandonata anche se la stella si allontana di più di *Escursione stella max* dalla sua posizione di partenza, prima ancora di poter uscire dal sensore.
{{% /notice %}}

### Fase 3 — Guida

Ciclo di correzione continuo:

1. Acquisire un'immagine.
2. Seguire il campo di stelle rispetto al riferimento (vedi *Corrispondenza stelle* più sotto).
3. Calcolare la deriva, proiettata sugli assi RA e DEC con l'orientamento di calibrazione.
4. **Regolazione PI** — la correzione vale `aggressività × deriva + guadagno integrale × deriva accumulata`. Il solo termine proporzionale non può annullare una deriva costante (si stabilizza a `deriva / aggressività`); il termine integrale continua a crescere finché il residuo non è realmente nullo. L'accumulatore è limitato (*Limite dell'integrale*) e congelato per un asse il cui impulso è già al massimo.
5. Il tasso RA è adattato alla declinazione corrente: `tasso equatoriale / cos(DEC corrente)`.
6. Se la *Compensazione del gioco DEC* è attiva e la direzione DEC si è invertita dall'ultima correzione, la stima di compensazione corrente viene aggiunta in sovrapposizione.
7. Gli impulsi di correzione sono inviati alla montatura, limitati a *Impulso min* / *Impulso max*.
8. Le statistiche vengono aggiornate (RMS RA, DEC, totale).
9. Se un impulso di compensazione dell'inversione è stato inviato al ciclo precedente, la riduzione di deriva effettivamente ottenuta viene confrontata con la previsione e la stima di compensazione viene aggiustata al rialzo (sotto-correzione) o al ribasso (sovra-correzione), entro i limiti configurati.

La **prima immagine** di una sessione viene misurata ma non corretta — serve solo a fissare lo zero di lavoro, affinché qualche pixel di stabilizzazione dopo uno spostamento non inneschi un grosso primo impulso.

Una perdita temporanea di correlazione delle stelle fa semplicemente **saltare** l'immagine (nessun impulso); la guida viene abbandonata solo dopo *Immagini perse consecutive max* di fila. Un **watchdog hardware** abbandona la sessione se un'esposizione, un impulso, un reset di frame o un'estrazione di stelle non termina mai entro il tempo *Timeout hardware*.

{{% notice style="note" title="Compensazione del gioco DEC" %}}
Gli ingranaggi della montatura hanno un gioco meccanico: a ogni inversione di direzione della guida DEC, i primi impulsi assorbono in parte questo gioco invece di produrre un movimento reale, da cui un picco di deriva temporaneo. Il gioco reale varia con la posizione di puntamento e il carico, per cui un valore fisso sotto-corregge o sovra-corregge. La stima adattiva si autoregola: ogni inversione è al tempo stesso una correzione e una misura, quindi converge verso il gioco realmente presente.
{{% /notice %}}

## Corrispondenza stelle

Tra due immagini, il campo di stelle subisce una traslazione quasi pura (la rotazione di 180° di un meridian flip è gestita separatamente dagli indicatori di inversione). L'inseguimento cerca direttamente la **traslazione dominante**:

1. Ogni coppia (stella di riferimento, stella corrente) implica una traslazione candidata; quella vera è quella su cui concorda il maggior numero di stelle (fase di voto / RANSAC).
2. I suoi punti coerenti sono affinati da una finestra a due stadi e un clipping sigma iterativo, e la deriva è una media dei residui pesata sul flusso — continua e sub-pixel.

Questo metodo è insensibile a un cambiamento nella classifica delle stelle per flusso e all'apparizione o scomparsa di stelle da un'immagine all'altra. Le impostazioni associate si trovano sotto *Corrispondenza stelle* nei parametri.

{{% notice style="note" title="Trasposizione in declinazione del tasso RA" %}}
Un impulso sposta la stella di `tasso × impulso × cos(DEC)` sul sensore, quindi i ms/px variano in `1 / cos(DEC)`. Il tasso RA viene quindi registrato riportato all'equatore al momento della calibrazione e diviso per `cos(DEC corrente)` durante la guida, affinché una calibrazione fatta a una declinazione resti corretta a un'altra.
{{% /notice %}}

## Parametri

I parametri sono accessibili dal menu **Parametri** del modulo e un set può essere salvato come profilo.

### Attrezzature

| Parametro | Descrizione |
|---|---|
| Esposizione | Durata dell'esposizione in secondi |
| Guadagno | Guadagno della camera |
| Offset | Offset della camera |

### Calibrazione

| Parametro | Descrizione |
|---|---|
| **Durata impulso** | Durata in ms di ogni impulso di calibrazione |
| **Passo di calibrazione** | Numero di impulsi per direzione |
| **Recupero gioco DEC** | Impulso supplementare (ms) prima delle serie Nord e Sud, per assorbire prima il gioco meccanico — 0 per disattivare |
| **Rapporto di deriva di fondo max** | Rifiutare la calibrazione se la deriva di fondo costante supera questa frazione dello spostamento dovuto agli impulsi — un valore elevato indica in genere una messa in stazione mediocre. 0 per disattivare il controllo |
| **Numero di tentativi di calibrazione** | Quante volte ripetere una calibrazione rifiutata prima di abbandonare |
| **Escursione stella max** | Abbandonare la calibrazione se la stella si allontana di più di questa frazione della dimensione più piccola dell'immagine rispetto alla sua partenza. 0 per disattivare |

### Guida

| Parametro | Descrizione |
|---|---|
| **Impulso max** | Durata massima di un impulso di correzione (ms) |
| **Impulso min** | Durata minima di un impulso di correzione (ms) |
| **Aggressività RA** | Frazione proporzionale della correzione RA calcolata che viene applicata |
| **Aggressività DEC** | Frazione proporzionale della correzione DEC calcolata che viene applicata |
| **Guadagno integrale RA** | Guadagno sul residuo RA accumulato — annulla lo scostamento permanente di fronte a una deriva costante. 0 = puramente proporzionale |
| **Guadagno integrale DEC** | Idem, per l'asse DEC |
| **Limite dell'integrale (px)** | Anti-saturazione: ampiezza a cui l'accumulatore integrale è limitato. 0 per disattivare il limite |
| **Soglia RMS** | Moltiplicatore dell'RMS corrente oltre il quale una correzione viene scartata |
| **RMS calcolato su x** | Numero di immagini su cui viene calcolato l'RMS |
| **Inversione secondo lato forcella** | Invertire automaticamente le correzioni RA/DEC quando il lato della forcella differisce da quello della calibrazione |
| **Ampiezza di dither (px)** | Spostamento casuale massimo applicato durante un dither |
| **Immagini perse consecutive max** | Saltare un'immagine di guida in caso di perdita di correlazione; abbandonare solo dopo questo numero di fila |
| **Timeout hardware (s)** | Abbandonare se la camera o la montatura non risponde più per questa durata. 0 per disattivare il watchdog |

### Corrispondenza stelle

| Parametro | Descrizione |
|---|---|
| **Finestra di corrispondenza (px)** | Raggio di ricerca durante la corrispondenza di una stella di riferimento nell'immagine corrente |
| **Stelle corrispondenti min** | Numero minimo di stelle di riferimento da ritrovare perché la deriva sia considerata affidabile |
| **Stelle utilizzate max** | Limite massimo del numero di stelle più luminose considerate in ogni immagine |

### Compensazione gioco DEC

| Parametro | Descrizione |
|---|---|
| **Attivare** | Aggiungere un impulso supplementare adattivo a ogni inversione di direzione della guida DEC |
| **Compensazione corrente (ms)** | Stima adattiva, aggiustata dopo ogni inversione — impostare un valore iniziale o lasciare a 0 perché venga appresa |
| **Compensazione minima / massima (ms)** | Limiti della stima adattiva |
| **Passo di aggiustamento (ms)** | Di quanto viene ritoccata la stima dopo ogni inversione, in base alla sovra/sotto-correzione misurata |

### Inversioni manuali

| Parametro | Descrizione |
|---|---|
| **Invertire RA** | Invertire manualmente la direzione di correzione RA |
| **Invertire DEC** | Invertire manualmente la direzione di correzione DEC |

### Disattivazione correzioni

| Parametro | Descrizione |
|---|---|
| **Disattivare RA+ / RA− / DEC+ / DEC−** | Disattivare gli impulsi di correzione in questa direzione |

### Aiuti per il test

| Parametro | Descrizione |
|---|---|
| **SIM: falso meridian flip** | Aggiunge 180° all'angolo di calibrazione all'avvio della guida (l'effetto che un vero meridian flip ha sulla corrispondenza deriva → RA/DEC), per verificare la gestione delle inversioni di correzione con il simulatore CCD, che non fa mai ruotare il proprio campo. Da lasciare disattivato in uso reale |

## Valori di calibrazione

| Valore | Descrizione |
|---|---|
| **Qualità della calibrazione** | Indicatore: verde (pulita) / arancione (utilizzabile, deriva di fondo notevole) / rosso (rifiutata) |
| **Tasso RA (ms/px)** | Tasso RA equatoriale usato per la guida (Ovest/Est combinati, deriva rimossa) |
| **Tasso DEC (ms/px)** | Tasso DEC usato per la guida (Nord/Sud combinati) |
| **grezzo N / S / E / W (ms/px)** | Tassi grezzi per direzione, solo a scopo diagnostico (portano ancora la deriva di fondo) |
| **Orientamento CCD** | Angolo di calibrazione (gradi) |
| **DEC montatura alla calibrazione** | Declinazione della montatura al momento della calibrazione |
| **Invertire RA / DEC** | Inversioni di correzione registrate durante la calibrazione |

## Valori in diretta

| Valore | Descrizione |
|---|---|
| **Impulso N / S / E / W** | Durata dell'ultimo impulso in ogni direzione (ms) |
| **RMS RA / DEC / totale** | Scarto quadratico medio della deriva (pixel) |

## Grafici

### Deriva (grafico a dispersione)

Grafico XY della deriva di ogni correzione rispetto alla posizione di riferimento, con cerchi di tolleranza.

### Guida (serie temporale)

Grafico in stile PHD2 che mostra, per ogni correzione:

| Curva | Descrizione |
|---|---|
| Deriva RA / deriva DEC | Deriva misurata su ogni asse (pixel) |
| Impulso RA / impulso DEC | Durata dell'impulso di correzione (ms) |
| SNR | Rapporto segnale/rumore del rilevamento della stella |
| RMS | RMS totale corrente (pixel) |

## Azioni

| Azione | Descrizione |
|---|---|
| **Calibrare** | Inizializzazione poi sola calibrazione |
| **Guidare** | Avviare la guida — usa la calibrazione salvata, oppure calibra prima se non ce n'è una |
| **Annulla** | Fermare immediatamente la sequenza in corso |
| **Reset calibrazione** | Eliminare i dati di calibrazione salvati |
| **Dither** | Spostare casualmente la posizione di guida fino a *Ampiezza di dither* |
