---
title: Sequencer
weight: 30
description: Acquisizione automatica di sequenze di immagini
---

## Ruolo

Il modulo Sequencer orchestra l'acquisizione automatica durante una sessione di astrofotografia. Esegue un elenco di righe di sequenza, ciascuna definendo il filtro, la durata di posa e il numero di immagini da acquisire. Il tipo di frame, il guadagno e l'offset sono ora condivisi da tutta la sequenza invece di essere definiti riga per riga. I file FITS vengono registrati automaticamente in un albero organizzato per oggetto e per filtro, e il modulo può richiedere il raffreddamento della camera, la messa a fuoco automatica e la guida attorno a ogni riga secondo necessità.

![Screenshot del modulo Sequencer](/images/modules/sequencer.png)

## Dispositivi richiesti

| Dispositivo | Ruolo |
|---|---|
| Camera | Acquisizione delle immagini, e raffreddamento se attivato |
| Ruota portafiltri | Opzionale — la sequenza funziona senza di essa; il posizionamento e i profili di messa a fuoco per filtro vengono semplicemente ignorati |

## Struttura di una sequenza

Una sequenza è composta da più righe. Ogni riga definisce:

| Colonna | Descrizione |
|---|---|
| **Filtro** | Filtro selezionato sulla ruota portafiltri |
| **Esposizione** | Durata dell'esposizione in secondi |
| **Numero** | Numero di immagini da acquisire per questa riga |

Le righe vengono eseguite in ordine, dalla prima all'ultima. Il tipo di frame, il guadagno e l'offset si applicano a tutte le righe (vedi *Parametri di sequenza* più sotto).

## Parametri

Le impostazioni sono divise in tre gruppi, per non mescolare il comportamento di sequenza, guida e messa a fuoco.

### Oggetto

| Parametro | Descrizione |
|---|---|
| **Nome** | Nome dell'oggetto fotografato — usato per nominare la cartella di registrazione |
| **AD** | Ascensione retta dell'oggetto |
| **DEC** | Declinazione dell'oggetto |

### Moduli slave

Il sequencer interagisce con altri moduli per la messa a fuoco e la guida.

| Parametro | Descrizione |
|---|---|
| **Istanza del modulo di messa a fuoco** | Nome dell'istanza del modulo Focuser da usare (es. `focus`) |
| **Istanza del modulo di guida** | Nome dell'istanza del modulo Guider da usare (es. `guider`) |

### Parametri di sequenza

| Parametro | Descrizione |
|---|---|
| **Attivare il raffreddamento camera** | Richiede il raffreddamento alla temperatura target prima di avviare la sequenza, e attende che venga raggiunta |
| **Temperatura target (°C)** | Temperatura target del sensore |
| **Tipo di frame** | Light (L), Bias (B), Dark (D), o Flat (F) — si applica a tutte le righe |
| **Guadagno** | Guadagno della camera — si applica a tutte le righe |
| **Offset** | Offset della camera — si applica a tutte le righe |

### Parametri di guida

| Parametro | Descrizione |
|---|---|
| **Usare la guida** | Attiva l'integrazione con il guider (dithering, stabilizzazione, monitoraggio dell'RMS, sospensione durante la messa a fuoco). Il guider deve già stare guidando, o essere avviato dal sequencer se necessario. |
| **Tempo di stabilizzazione dopo la ripresa della guida (s)** | Ritardo di attesa in secondi dopo la ripresa della guida, prima di continuare le acquisizioni |
| **Dithering ogni N pose (0=disattivato)** | Richiede un dithering ogni N immagini acquisite |
| **Soglia di ricalibrazione RMS (0=disattivato)** | Interrompe l'esposizione in corso e avvia una ricalibrazione del guider quando l'RMS massimo durante questa esposizione supera questo valore |

### Parametri di messa a fuoco

| Parametro | Descrizione |
|---|---|
| **Messa a fuoco auto al cambio di filtro** | Avvia una messa a fuoco a ogni cambio di filtro — questo copre anche la primissima riga della sequenza, poiché nessuna messa a fuoco è ancora avvenuta in questa sessione del modulo. Riavviare la stessa sequenza evita una messa a fuoco ridondante se l'ultima effettuata corrisponde già al filtro corrente. |
| **Sospendere la guida durante la messa a fuoco** | Metti in pausa la guida prima della messa a fuoco e la riprende successivamente |
| **Soglia di ri-messa a fuoco HFR (0=disattivato)** | Avvia una messa a fuoco supplementare durante la riga quando l'HFR misurata supera questa soglia |
| **Profilo di messa a fuoco da caricare** | Profilo da caricare sul modulo di messa a fuoco prima di richiedere l'autofocus; lasciare vuoto per conservare il profilo attualmente caricato |

### Profilo di messa a fuoco per filtro

Una griglia dedicata permette di sovrascrivere il profilo di messa a fuoco per filtri specifici — utile ad esempio quando i filtri a banda stretta richiedono impostazioni di messa a fuoco diverse dai filtri LRGB.

| Colonna | Descrizione |
|---|---|
| **Filtro** | Filtro a cui si applica questa riga |
| **Profilo di messa a fuoco** | Profilo da caricare durante la messa a fuoco su questo filtro |

Se il filtro corrente ha una riga in questa griglia, viene usato il suo profilo. Altrimenti, il sequencer ricade sul *Profilo di messa a fuoco da caricare* sopra. Se anche questo è vuoto, il modulo di messa a fuoco conserva il profilo attualmente caricato.

## Algoritmo

### Avvio della sequenza

1. Connessione alla camera e reset del frame
2. Se *Attivare il raffreddamento camera* è attivo, richiede la temperatura target e attende che venga raggiunta (ignorato immediatamente se disattivato o se la camera non è configurata)
3. Avvio della prima riga

### Esecuzione di una riga

Per ogni riga della sequenza:

1. Selezione del filtro sulla ruota portafiltri (ignorato se nessuna ruota portafiltri è configurata)
2. Se *Messa a fuoco auto al cambio di filtro* è attivato e il filtro differisce da quello usato nell'ultima messa a fuoco di questa sessione: avvio della messa a fuoco automatica (caricando prima un profilo — sovrascrittura per filtro, poi ripiego generale — e sospendendo/riprendendo la guida se configurato)
3. Avvio o conferma della guida se *Usare la guida* è attivato
4. Dithering se l'intervallo di pose configurato è raggiunto
5. Acquisizione del numero di immagini definito una per una, con ri-messa a fuoco durante la riga se la soglia HFR viene superata

### Registrazione dei file

I file FITS sono registrati nel seguente albero (i segmenti oggetto e filtro sono omessi quando non definiti, invece di lasciare separatori superflui):

```
<oggetto>/
  LIGHT/
    <filtro>/   ← Frame Light
  FLAT/
    <filtro>/   ← Frame Flat
  BIAS/         ← Frame Bias
  DARK/         ← Frame Dark
```

Ogni nome di file comprende il nome dell'oggetto, il tipo di frame, il filtro e una marca temporale.

### Integrazione con il modulo di messa a fuoco

Quando viene richiesta una messa a fuoco automatica:

1. Se si applica un profilo (sovrascrittura per filtro, o ripiego generale), il sequencer chiede al modulo Focuser di caricarlo
2. Il sequencer invia l'azione `autofocus` al modulo Focuser designato
3. Metti in pausa le acquisizioni e attende la fine della messa a fuoco
4. Se *Sospendere la guida* è attivato, invia `abortguider` al modulo Guider prima della messa a fuoco, poi `guide` successivamente
5. Se è configurato un tempo di stabilizzazione, attende questo ritardo prima di riprendere

### Integrazione con il modulo di guida

- La guida viene avviata (o la sua presenza confermata) prima dell'inizio delle esposizioni, quando *Usare la guida* è attivato
- Un dithering viene richiesto ogni N pose, poi il sequencer attende il tempo di stabilizzazione configurato
- Se l'RMS massimo durante un'esposizione supera la soglia configurata, l'esposizione viene interrotta e il guider viene ricalibrato prima di riprendere

## Avanzamento

L'avanzamento è seguito sia in diretta (aggiornamento durante l'acquisizione) che sotto forma di medie/durate cumulate dal caricamento del modulo.

| Indicatore | Descrizione |
|---|---|
| **Sequenza (pose)** | Numero di pose terminate sul totale dell'intera sequenza |
| **Sequenza (tempo)** | Tempo di esposizione trascorso sul tempo di esposizione totale stimato, aggiornato in diretta durante l'esposizione in corso |
| **Esposizione in corso** | Avanzamento dell'esposizione in corso (0 al 100%) |
| **Durata totale teorica** | Somma di numero × esposizione su tutte le righe, ricalcolata a ogni modifica della griglia — anche prima dell'avvio della sequenza |
| **Durata rimanente teorica** | Durata totale teorica meno il tempo già trascorso |
| **Durata reale trascorsa** | Cronometro reale, azzerato a ogni avvio della sequenza |
| **Durata media di messa a fuoco** | Media mobile del tempo passato ad attendere l'autofocus, dal caricamento del modulo |
| **Durata media di avvio della guida** | Media mobile del tempo passato ad attendere l'avvio del guider, dal caricamento del modulo |

## Azioni

| Azione | Descrizione |
|---|---|
| **Avvia la sequenza** | Avvia l'esecuzione della sequenza dalla prima riga |
| **Interrompi** | Arresta immediatamente la sequenza in corso, e propaga anche una richiesta di interruzione alle istanze Guider e Focuser configurate in *Moduli slave*, per arrestare anche la guida e un'eventuale messa a fuoco in corso |
