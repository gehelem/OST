---
title: Planner
weight: 50
description: Pianificazione multi-target e orchestrazione automatica della sessione
---

## Ruolo

Il modulo Planner orchestra una sessione di astrofotografia multi-target. Esegue un elenco di righe di pianificazione, delegando successivamente il puntamento al modulo Navigator e l'acquisizione al modulo Sequencer. Per ogni target, carica automaticamente un profilo di sequenza. Prima di puntare un target, verifica anche che resti sopra un'elevazione minima per tutta la durata stimata della sequenza e che sia sufficientemente lontano dalla luna, saltando e ritentando più tardi i target che non rispettano questi vincoli.

![Screenshot del modulo Planner](/images/modules/planner.png)

## Dispositivi richiesti

| Dispositivo | Ruolo |
|---|---|
| GPS | Fornitura delle coordinate geografiche e dell'ora |

## Struttura di una pianificazione

Una pianificazione è composta da più righe. Ogni riga definisce:

| Colonna | Descrizione |
|---|---|
| **Oggetto** | Nome del target |
| **RA** | Ascensione retta del target (J2000) |
| **DEC** | Declinazione del target (J2000) |
| **Profilo** | Profilo di sequenza da caricare nel Sequencer per questo target |
| **Avanzamento** | Progressione della riga in corso |

Le righe non sono necessariamente eseguite strettamente dall'alto verso il basso: una riga che fallisce il controllo di visibilità/luna viene saltata e ritentata più tardi, una volta che tutte le altre righe sono state processate (vedi *Stati di una riga* più sotto). Possono essere aggiunte manualmente o inviate dal Navigator tramite l'azione *Aggiungi alla pianificazione*.

## Parametri

### Moduli slave

| Parametro | Descrizione |
|---|---|
| **Istanza del Sequencer** | Nome dell'istanza del modulo Sequencer da pilotare |
| **Istanza del Navigator** | Nome dell'istanza del modulo Navigator da pilotare |

### Vincoli di visibilità

| Parametro | Descrizione |
|---|---|
| **Minimum elevation (°)** | Il target deve restare sopra questa elevazione per tutta la durata stimata della sequenza, altrimenti la riga viene saltata |
| **Moon illumination threshold (%)** | L'evitamento della luna si applica solo quando la sua frazione illuminata supera questa percentuale |
| **Moon separation threshold (°)** | La riga viene rifiutata solo se il target è più vicino di questa distanza **e** l'illuminazione della luna supera la soglia sopra |
| **Max retries before giving up** | Una riga che fallisce il controllo di visibilità/luna questo numero di volte passa allo stato *Failed* e non viene mai più ritentata |

## Algoritmo

### Avvio

1. Verifica della connessione INDI e dei moduli slave
2. Marcatura di tutte le righe allo stato *In attesa* e reset dei contatori di tentativi
3. Avvio della prima riga

### Esecuzione di una riga

Per ogni riga della pianificazione:

1. Il Planner chiede al Sequencer la durata teorica del profilo richiesto (senza caricarlo), tramite una richiesta generica inter-modulo
2. Verifica che il target resti sopra l'elevazione minima configurata per tutta questa durata, e sufficientemente lontano dalla luna (soglie di illuminazione/distanza) — se uno dei due controlli fallisce, la riga viene saltata (vedi *Stati di una riga*) e il Planner passa alla successiva
3. Altrimenti, configura il target del Navigator (RA, DEC, nome)
4. Carica il profilo di sequenza richiesto nel Sequencer e configura l'oggetto corrispondente
5. Avvia l'azione **Goto target** del Navigator
6. Attende la fine della centratura (evento di successo del Navigator)
7. Avvia l'azione **Avvia la sequenza** del Sequencer
8. Segue l'avanzamento della sequenza e attende la sua fine
9. La riga viene marcata *Terminata* e il Planner passa alla prossima riga utilizzabile

### Stati di una riga

| Stato | Significato | Ritentata più tardi? |
|---|---|---|
| **Queued** | Non ancora tentata | — |
| **Checking visibility** | Controllo elevazione/luna in corso | — |
| **Slewing** | Centratura del Navigator sul target | — |
| **Skipped: \<motivo\>** | Fallimento del controllo di visibilità/luna | Sì, una volta che tutte le altre righe sono state processate |
| **Failed: \<motivo\>** | Fallimento del controllo di visibilità/luna troppe volte (*Max retries* raggiunto) | No — esclusa definitivamente |
| **Cancelled** | La sequenza è stata interrotta manualmente durante l'esecuzione (in contrapposizione a una fine normale) | No — esclusa definitivamente |
| **Finished** | Sequenza terminata con successo | No |

{{% notice style="tip" title="Annullamento manuale vs fine normale" %}}
Il Planner distingue i due casi: se interrompi direttamente il Sequencer mentre esegue una riga della pianificazione, questa riga viene marcata *Cancelled* e il Planner passa alla prossima riga utilizzabile — non viene confusa con un successo, e non viene più ritentata in seguito.
{{% /notice %}}

### Fine della pianificazione

Il Planner continua a ciclare sulle righe, ritentando quelle ancora *Skipped*, finché ogni riga non è *Finished*, *Failed* o *Cancelled*. A quel punto, l'avanzamento globale passa al 100% e il modulo si arresta.

### Propagazione dell'arresto

Arrestare il Planner (sia tramite il pulsante **Arresta**, sia perché un modulo genitore come il Park Manager lo richiede) interrompe anche i suoi moduli slave: invia l'azione **Interrompi** del Sequencer e l'azione **Interrompi** del Navigator, affinché una pianificazione interrotta non lasci un'esposizione o un puntamento continuare senza supervisione.

## Avanzamento

| Indicatore | Descrizione |
|---|---|
| **Avanzamento globale** | Numero di righe elaborate sul totale |
| **Avanzamento per riga** | Progressione dell'acquisizione in corso, ritrasmessa dal Sequencer |

## Azioni

| Azione | Descrizione |
|---|---|
| **Avvia** | Avvia l'esecuzione della pianificazione dalla prima riga |
| **Arresta** | Interrompe la pianificazione in corso |
