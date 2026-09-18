---
title: Park Manager
weight: 70
description: Automazione dell'apertura e della chiusura dell'osservatorio
---

## Ruolo

Il modulo Park Manager orchestra l'apertura e la chiusura completa dell'osservatorio: apertura dello sportello della cupola, sparcheggio della montatura, avvio dell'inseguimento — e nell'ordine inverso per la chiusura. Può funzionare in modalità manuale (azione puntuale) o in modalità automatica pilotata da regole orarie e meteo.

![Screenshot del modulo Park Manager](/images/modules/parkmanager.png)

## Dispositivi richiesti

| Dispositivo | Ruolo |
|---|---|
| Cupola | Apertura e chiusura dello sportello, parcheggio |
| Montatura | Sparcheggio, inseguimento, ritorno al home, parcheggio |
| Stazione meteo | Monitoraggio delle condizioni per la modalità automatica (opzionale) |

## Parametri

### Regole meteo

| Parametro | Descrizione |
|---|---|
| **Must be OK** | Se attivato, l'apertura viene rifiutata se il meteo non è al verde. In corso di sessione, un degrado meteo innesca automaticamente la chiusura |
| **Disabled** | Disattiva il monitoraggio meteo (la stazione meteo viene ignorata) |

### Moduli slave

| Parametro | Descrizione |
|---|---|
| **Planner instance** | Nome dell'istanza del modulo Planner da arrestare all'avvio della sequenza di chiusura |

### Regole orarie

Queste regole definiscono la finestra di apertura automatica in modalità **Automate**. Sono mutuamente esclusive.

| Parametro | Descrizione |
|---|---|
| **Fixed time** | Finestra orario fissa definita da un'ora di inizio e un'ora di fine |
| **Fixed time: start** | Ora di inizio della finestra fissa |
| **Fixed time: end** | Ora di fine della finestra fissa (può essere il giorno successivo — gestione del passaggio di mezzanotte) |
| **Dusk and dawn** | Finestra automatica dal tramonto all'alba (calcolata dalle coordinate GPS) |
| **Anytime** | Apertura permanente: il modulo apre non appena è in modalità Auto e resta aperto |

## Sequenze

### Sequenza di apertura

1. Verifica meteo (se la regola *Must be OK* è attiva)
2. Apertura dello sportello della cupola
3. Attesa conferma apertura sportello
4. Sparcheggio della montatura
5. Attesa conferma sparcheggio
6. Avvio dell'inseguimento siderale
7. Ingresso in monitoraggio (*Open Monitoring*)

### Sequenza di chiusura

1. Interruzione del raffreddamento su tutti i dispositivi connessi che esporgono uno switch `CCD_COOLER` (non solo la camera usata dal Sequencer), e richiesta di arresto all'istanza del Planner
2. Arresto dell'inseguimento siderale
3. Invio della montatura in posizione home
4. Attesa arrivo in posizione home
5. Parcheggio della montatura
6. Attesa conferma parcheggio
7. Chiusura dello sportello della cupola
8. Attesa conferma chiusura sportello

{{% notice style="warning" title="Ordine di chiusura" %}}
La montatura viene sempre parcheggiata **prima** della chiusura della cupola. Non interrompere mai manualmente la sequenza tra queste due fasi.
{{% /notice %}}

{{% notice style="tip" title="Propagazione dell'arresto" %}}
Arrestare il Planner interrompe a sua volta il Sequencer (che interrompe a sua volta il Guider e il Focuser) così come il Navigator — vedi la documentazione [Planner](../planner/). Questo garantisce che tutta la catena di acquisizione si arresti correttamente prima della chiusura fisica dell'osservatorio.
{{% /notice %}}

### Arresto d'emergenza

L'azione **Abort** invia simultaneamente un ordine di arresto alla montatura (`TELESCOPE_ABORT_MOTION`) e alla cupola (`DOME_ABORT_MOTION`), poi ritorna in modalità Idle.

## Modalità automatica

In modalità **Automate**, il modulo valuta ogni 2 secondi se l'osservatorio deve essere aperto o chiuso secondo le regole orarie e meteo configurate.

{{< mermaid >}}
flowchart TB
    ATTENTE([Attesa]) -->|"finestra attiva & meteo OK"| OD

    subgraph OUV ["Sequenza di apertura"]
        direction LR
        OD[Apre cupola] --> OW[Attende apertura] --> UD[Sparcheggia montatura] --> UW[Attende sparcheggio] --> TR[Avvia inseguimento]
    end

    TR --> SURV([Monitoraggio])
    SURV -->|"finestra terminata o meteo degradato"| ST

    subgraph FER ["Sequenza di chiusura"]
        direction LR
        ST[Arresta inseguimento] --> GH[Goto home] --> WH[Attende home] --> PM[Parcheggia montatura] --> WP[Attende parcheggio] --> CD[Chiude cupola]
    end

    CD --> ATTENTE
    ATTENTE -->|Abort| IDLE([Idle])
    SURV -->|Abort| IDLE
{{< /mermaid >}}

| Situazione | Azione |
|---|---|
| Finestra aperta + osservatorio chiuso | Avvio della sequenza di apertura |
| Finestra chiusa + osservatorio aperto | Avvio della sequenza di chiusura |
| Meteo degradato in corso di sessione | Avvio della sequenza di chiusura |
| Fine sequenza | Ritorno in attesa — la modalità Auto è conservata |

{{% notice style="tip" title="Persistenza della modalità Auto" %}}
Dopo una sequenza di apertura o chiusura, il modulo resta in modalità **Auto** e continua a monitorare le condizioni. Non è necessario riattivarlo manualmente.
{{% /notice %}}

## Azioni

| Azione | Descrizione |
|---|---|
| **Open all** | Avvia la sequenza di apertura completa |
| **Close all** | Avvia la sequenza di chiusura completa |
| **Automate** | Attiva la modalità automatica pilotata dalle regole |
| **Abort motion** | Arresto d'emergenza di tutti i movimenti |
| **Idle** | Ritorna in modalità inattiva senza azione |

## Stato mostrato

### Montatura

| Valore | Descrizione |
|---|---|
| **Mount RA** | Ascensione retta corrente |
| **Mount DEC** | Declinazione corrente |
| **Mount is tracking** | Inseguimento siderale attivo |
| **Mount is parked** | Montatura in posizione di parcheggio |
| **Mount is at home** | Montatura in posizione home |

### Cupola

| Valore | Descrizione |
|---|---|
| **Dome is parked** | Cupola in posizione di parcheggio |
| **Dome shutter is closed** | Sportello chiuso |

### Meteo ed eventi

| Valore | Descrizione |
|---|---|
| **Global** | Stato globale del meteo (verde / arancione / rosso) |
| **Next sunset** | Ora del prossimo tramonto |
| **Next sunrise** | Ora della prossima alba |
