---
title: Presentazione
weight : 10
---

## Perché "Observatoire Sans Tête"?

Questo nome nasce da uno scherzo con il mio amico Bob, che consiste nel tradurre parola per parola "Headless Observatory".
Questo termine descrive bene l'obiettivo cercato: un server autonomo che si può controllare da remoto.

## Cos'è OST?

OST è una piattaforma di astrofotografia autonoma. Funziona in background su una macchina dedicata — Raspberry Pi, mini-PC o macchina virtuale — e si utilizza da qualsiasi browser web sulla rete locale, senza schermo né tastiera collegati alla macchina di controllo.

L'obiettivo è poter gestire un'intera sessione di osservazione dal divano, da un altro edificio, o da remoto via VPN: messa in stazione, puntamento, messa a fuoco, guida, acquisizione e documentazione della notte.

## Architettura

OST si basa su un'architettura client-server:

| Componente | Ruolo |
|---|---|
| **Server OST** | Applicazione C++/Qt in esecuzione in background, pilota i dispositivi tramite il protocollo INDI |
| **Interfaccia web** | Client Angular servito da Nginx, comunica con il server tramite WebSocket |
| **Server INDI** | Livello di abstrazione hardware — driver per fotocamere, montature, focheggiatori, ruote portafiltri, ecc. |

La comunicazione tra il client e il server è interamente basata su messaggi JSON scambiati via WebSocket, il che permette di controllare OST da qualsiasi dispositivo dotato di un browser moderno.

## Moduli

OST è organizzato in moduli indipendenti, ciascuno dedicato a una funzione specifica:

| Modulo | Funzione |
|---|---|
| **Sequencer** | Acquisizione automatizzata: sequenze di scatti per tipo, filtro, guadagno e durata |
| **Guida** | Autoguida tramite PHD2, controllo dell'agitazione e delle correzioni |
| **Navigator** | Risoluzione di campo (plate solving) e centratura automatica su un target |
| **Planner** | Orchestrazione di una lista di oggetti: centratura e poi acquisizione per ciascuno |
| **Polar** | Allineamento polare con il metodo delle 3 immagini |
| **Messa a fuoco** | Autofocus con curva a V e minimizzazione dell'HFR |
| **Inspector** | Analisi della qualità ottica: mappa HFR, mappa delle aberrazioni, mosaico degli angoli |
| **IndiPanel** | Pannello di controllo grezzo di tutti i dispositivi INDI connessi |
| **Allsky** | Camera grandangolare: keogramma, stack, timelapse e sovrapposizione meteo |

## Ispirazioni

Per la sua forma, OST si inspira a numerosi altri progetti opensource:
- https://github.com/GuLinux/AstroPhoto-Plus
- https://github.com/pludov/mobindi
- https://www.indigo-astronomy.org/for-users.html

Naturalmente l'inspirazione viene anche da soluzioni commerciali:
- ASIAIR
- Stellarmate

## Tecnologie utilizzate

| Ambito | Tecnologia |
|---|---|
| Server | C++17, Qt6, libnova, cfitsio, libindi, StellarSolver |
| Interfaccia web | Angular, TypeScript |
| Server web | Nginx |
| Protocollo | INDI, WebSocket, JSON |
| Video | ffmpeg (timelapse allsky) |
| Packaging | PPA Ubuntu (Launchpad) |
