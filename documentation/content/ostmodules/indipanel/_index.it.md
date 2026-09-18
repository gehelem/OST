---
title: IndiPanel
weight: 80
description: Pannello di controllo grezzo di tutti i dispositivi INDI connessi
---

## Ruolo

Il modulo IndiPanel è un pannello di controllo trasparente che dà accesso a tutte le proprietà INDI di tutti i dispositivi connessi al server INDI. A differenza degli altri moduli OST, non svolge una funzione astrofotografica specifica: espone direttamente l'intero protocollo INDI nell'interfaccia OST.

![Screenshot del modulo IndiPanel](/images/modules/indipanel.png)

{{% notice style="note" title="Modulo senza configurazione" %}}
IndiPanel non ha alcun file di configurazione né parametri da definire. Il suo contenuto è generato interamente al volo a partire dai dispositivi e dalle proprietà scoperte sul server INDI.
{{% /notice %}}

## Funzionamento

Fin dall'avvio, il modulo si connette al server INDI e ascolta tutti gli eventi di scoperta di dispositivi e proprietà. Per ogni proprietà INDI ricevuta, viene creata automaticamente una voce corrispondente nell'interfaccia OST:

| Tipo INDI | Rappresentazione OST |
|---|---|
| Number | Valore numerico |
| Switch | Pulsante on/off o gruppo di pulsanti (OneOfMany, AnyOfMany) |
| Text | Campo di testo |
| Light | Indicatore di stato |
| BLOB | Immagine (JPEG + FITS) |

Le proprietà sono organizzate per dispositivo INDI e per gruppo, esattamente come in qualsiasi client INDI standard.

## Interazione bidirezionale

- **INDI → OST**: qualsiasi aggiornamento di una proprietà INDI (valore, stato) si riflette immediatamente nell'interfaccia
- **OST → INDI**: qualsiasi modifica effettuata dall'utente viene rinviata al driver INDI corrispondente (`sendNewSwitch`, `sendNewNumber`, `sendNewText`)

Le immagini (BLOB) vengono visualizzate automaticamente non appena un driver invia un'immagine — anche da camere non configurate in altri moduli.

## Casi d'uso

- Accesso diretto alle proprietà avanzate di un driver INDI non esposte dagli altri moduli
- Debug di un dispositivo o di un driver
- Controllo di un dispositivo esotico senza un modulo OST dedicato
- Consultazione dei messaggi di stato emessi dai driver INDI

## Dispositivi

Nessun dispositivo da configurare. Il modulo si connette automaticamente al server INDI e scopre tutti i dispositivi presenti.
