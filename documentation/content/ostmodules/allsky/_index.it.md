---
title: Allsky
weight: 90
description: Camera allsky con keogramma, stack, timelapse e dati meteo
---

## Ruolo

Il modulo Allsky pilota una camera grandangolare (allsky) in loop continuo per documentare la notte. A ogni immagine, genera una sovrapposizione dei dati meteo, costruisce un keogramma e uno stack in tempo reale, poi produce un timelapse video a fine sessione. Le sessioni passate sono archiviate e consultabili.

![Screenshot del modulo Allsky](/images/modules/allsky.png)

## Dispositivi richiesti

| Dispositivo | Ruolo |
|---|---|
| Camera | Acquisizione delle immagini allsky |
| GPS | Opzionale — posizione geografica per il calcolo degli orari di alba/tramonto |
| Stazione meteo | Opzionale — dati meteo visualizzati in sovrapposizione e nel keogramma |

## Parametri

### Acquisizione

| Parametro | Descrizione |
|---|---|
| **Esposizione** | Durata dell'esposizione in secondi |
| **Guadagno** | Guadagno della camera |
| **Offset** | Offset della camera |
| **Ritardo (s)** | Intervallo in secondi tra l'avvio di due acquisizioni consecutive (deve essere superiore all'esposizione) |

### Pianificazione

| Parametro | Descrizione |
|---|---|
| **Manuale** | Avvia immediatamente e funziona in continuo finché l'azione Play è attiva |
| **Ora fissa** | Acquisizione tra un'ora di inizio e un'ora di fine (giorno o notte secondo l'ordine degli orari) |
| **Tramonto** | Si avvia automaticamente al tramonto e si arresta all'alba |

Per le modalità *Ora fissa* e *Tramonto*, gli orari del prossimo sorgere e tramontare del sole sono calcolati automaticamente a partire dalla posizione geografica configurata (libnova).

### Posizione geografica

La posizione geografica (latitudine, longitudine) è necessaria per il calcolo degli orari solari. Può essere inserita manualmente o importata da un dispositivo GPS connesso.

### Esposizione/guadagno automatico

| Parametro | Descrizione |
|---|---|
| **Attivato** | Attiva la regolazione automatica |
| **Modificare esposizione o guadagno** | Sceglie il parametro da regolare |
| **Valore minimo** | Limite inferiore della regolazione |
| **Valore massimo** | Limite superiore della regolazione |
| **Misura** | Statistica usata come riferimento (Media o Mediana) |
| **Valore target** | Valore target della statistica |
| **Soglia (%)** | Non utilizzato attualmente |

Il coefficiente di correzione è calcolato con: `nuovo_valore = valore_attuale × (target / misura_attuale)`.

### Archivi

| Parametro | Descrizione |
|---|---|
| **Conservare le immagini individuali** | Se attivato, i JPEG individuali di ogni sessione sono conservati negli archivi; altrimenti vengono conservati solo il keogramma, lo stack e il timelapse |

## Elaborazione di ogni immagine

Per ogni immagine acquisita:

1. **Salvataggio FITS** dell'immagine grezza
2. **Sovrapposizione** di data/ora e dati meteo (temperatura, umidità, pressione) a colori sul JPEG
3. **Stack per massimo**: ogni pixel conserva il valore massimo di tutte le immagini della sessione
4. **Keogramma**: una banda verticale di 1 pixel prelevata al centro di ogni immagine viene accostata alle precedenti per formare un grafico temporale della notte; i dati meteo vi sono rappresentati da punti colorati (rosso = temperatura, verde = umidità, blu = pressione)
5. **Log SNR**: il rapporto segnale/rumore dell'immagine viene registrato con la marca temporale

## Timelapse

A fine sessione (arresto o fine della pianificazione), il modulo genera un timelapse video (MP4 H.264 a 30 fps) a partire da tutti i JPEG della sessione, tramite ffmpeg.

## Archivi

Ogni sessione terminata viene spostata in una cartella di archivio con marca temporale. Gli archivi sono consultabili direttamente dall'interfaccia con, per ogni sessione:

| Elemento | Descrizione |
|---|---|
| **Data** | Identificativo con marca temporale della sessione |
| **Keogramma** | Banda temporale della notte |
| **Stack** | Immagine impilata (massimo) della notte |
| **Timelapse** | Video della notte |

## Dati meteo

Se è configurato un dispositivo meteo INDI, il modulo legge le proprietà `WEATHER_PARAMETERS` per recuperare temperatura, umidità e pressione. Questi valori sono:
- Visualizzati in tempo reale nella sezione **Misure**
- Sovrapposti su ogni immagine
- Tracciati nel keogramma
- Storicizzati in un grafico temporale

## Azioni

| Azione | Descrizione |
|---|---|
| **Play** | Avvia il loop di acquisizione secondo la modalità di pianificazione attiva |
| **Pause** | Sospende temporaneamente le acquisizioni senza chiudere la sessione |
| **Stop** | Arresta la sessione, genera il timelapse e archivia la sessione |
