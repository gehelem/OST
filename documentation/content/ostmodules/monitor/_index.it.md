---
title: Monitor
weight: 75
description: Monitoraggio e storicizzazione degli eventi di sessione
---

## Ruolo

Il modulo Monitor registra gli eventi emessi dai moduli slave durante una sessione di osservazione e li restituisce in forma grafica. Permette di visualizzare a posteriori la qualità della guida, l'evoluzione dell'HFR delle immagini e gli interventi di messa a fuoco, su un asse temporale comune.

![Screenshot del modulo Monitor](/images/modules/monitor.png)

## Principio di funzionamento

Il Monitor osserva la proprietà `signals` di ogni modulo slave configurato. Ogni volta che un modulo emette un evento (tramite `setStateEvent`), il Monitor lo registra con una marca temporale e i valori numerici associati.

Gli eventi sono memorizzati per tutta la sessione. All'arresto, vengono esportati in un file JSON con marca temporale sotto `<webroot>/monitor/`.

## Moduli monitorati

| Modulo | Eventi captati |
|---|---|
| Guider | `guideRMS` — RMS totale della guida (arcsec) |
| Focus | `focusdone` — HFR finale dopo la messa a fuoco (pixel) |
| Guider | `guideSNR` — rapporto segnale/rumore del rilevamento delle stelle |
| Sequencer | `imagehfr` — HFR dell'immagine registrata (pixel) |

## Grafico di sessione

Il grafico principale mostra l'intera sessione su un asse temporale comune.

| Curva | Descrizione |
|---|---|
| Linea arancione | RMS della guida (asse sinistro, arcsec) |
| Punti blu | HFR delle immagini acquisite (asse destro, pixel) |
| Triangoli verdi | HFR misurata a fine messa a fuoco (asse destro, pixel) |
| Linea rossa tratteggiata | SNR della guida (nascosta di default, attivabile dalla legenda) |

Il doppio slider sotto il grafico permette di limitare la finestra temporale visualizzata senza perdita dei dati in memoria.

## Azioni

| Azione | Descrizione |
|---|---|
| **Start** | Avvia una nuova sessione di registrazione (elimina i dati precedenti) |
| **Stop** | Arresta la registrazione ed esporta i dati in JSON |

## Parametri

### Parms

| Parametro | Descrizione |
|---|---|
| **Auto-start** | Avvia automaticamente la registrazione al caricamento del modulo (salvato nel profilo) |

### Filtraggio della vista

| Parametro | Descrizione |
|---|---|
| **From** | Limite inferiore della visualizzazione (filtra gli eventi anteriori a questa ora) |
| **To** | Limite superiore della visualizzazione (filtra gli eventi posteriori a questa ora) |
| **Max rows** | Numero massimo di righe conservate nella vista live (predefinito: 200) |

### Moduli monitorati (Slaves)

Ogni voce permette di selezionare l'istanza di modulo da monitorare per ogni tipo (guider, sequencer, focus, ecc.).

## Persistenza JSON

A ogni arresto di sessione, viene esportato un file sotto:

```
<webroot>/monitor/YYYYMMDD-HHmmss.json
```

Il file contiene le intestazioni di colonna (`gridheaders`) e la griglia di dati (`grid`) in formato tabella di tabelle, direttamente riutilizzabile per un'analisi esterna.

{{% notice style="note" title="Avvio automatico" %}}
Con **Auto-start** attivato nel profilo, il modulo avvia la registrazione un secondo dopo il suo caricamento, senza intervento manuale.
{{% /notice %}}
