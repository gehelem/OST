---
title: Polar
weight: 60
description: Assistente per la messa in stazione polare tramite plate solving
---

## Ruolo

Il modulo Polar assiste la messa in stazione polare della montatura. Acquisisce tre immagini del cielo spostando la montatura in RA tra ogni scatto, risolve astrometricamente ogni immagine, poi calcola l'errore di azimut e altitudine dell'asse polare. Un loop di correzione continuo permette poi di guidare l'utente mentre regola meccanicamente la montatura.

![Screenshot del modulo Polar](/images/modules/polar.png)

## Dispositivi richiesti

| Dispositivo | Ruolo |
|---|---|
| Camera | Acquisizione delle immagini per il plate solving |
| Montatura | Spostamento in RA e lettura delle coordinate |
| GPS | Posizione geografica dell'osservatore |

## Parametri

| Parametro | Descrizione |
|---|---|
| **Esposizione** | Durata dell'esposizione in secondi |
| **Guadagno** | Guadagno della camera |
| **Offset** | Offset della camera |
| **Focale** | Focale ottica in mm — usata per definire la scala immagine del solver |

## Algoritmo

### Fase 1 — Calibrazione (3 immagini)

Il modulo acquisisce tre immagini effettuando uno spostamento di 30 minuti d'angolo (0,5 h) in RA tra ogni scatto:

1. **Immagine 0** — posizione iniziale
2. Spostamento di ±0,5 h in RA (senso adattato al lato della forcella)
3. **Immagine 1** — nuova posizione
4. Nuovo spostamento di ±0,5 h in RA
5. **Immagine 2** — posizione finale

Ogni immagine viene plate-risolta. Le tre soluzioni (RA, DEC in J2000) sono convertite in coordinate orizzontali (azimut/altitudine) all'istante esatto dello scatto, tenendo conto della posizione geografica dell'osservatore.

### Calcolo dell'asse polare

I tre punti Az/Alt descrivono un arco sulla sfera celeste corrispondente alla rotazione della montatura. Il modulo calcola il vettore normale a questo arco — è la direzione dell'asse di rotazione reale della montatura.

Gli errori vengono quindi dedotti:

| Errore | Definizione |
|---|---|
| **Errore azimut** | Scostamento tra l'azimut dell'asse e il Nord (azimut = 0°) |
| **Errore altitudine** | Differenza tra l'altitudine dell'asse e la latitudine dell'osservatore |
| **Errore totale** | Errore combinato: `√(az² + alt²)` |

Gli errori sono mostrati in gradi e in minuti d'arco.

### Fase 2 — Loop di correzione

Dopo la calibrazione, il modulo entra in un loop continuo senza spostamento della montatura:

1. Acquisizione di un'immagine
2. Plate solving
3. Calcolo dello spostamento Az/Alt rispetto all'immagine di riferimento (immagine 2)
4. Aggiornamento dell'errore residuo = errore iniziale − correzione già applicata

L'utente regola meccanicamente l'azimut e l'altitudine della montatura; l'errore residuo diminuisce in tempo reale con il migliorare della messa in stazione.

### Sovrapposizione grafica

L'immagine mostrata comporta una sovrapposizione che rappresenta le correzioni da applicare:

| Tratto | Colore | Significato |
|---|---|---|
| Asse polare → correzione altitudine | Giallo | Errore di altitudine |
| Correzione altitudine → polo | Verde | Errore di azimut |
| Asse polare → polo | Rosso | Errore totale |

Il riferimento è orientato in modo che il Nord celeste sia in alto nell'immagine.

## Stati

| Stato | Descrizione |
|---|---|
| **Idle** | Modulo inattivo |
| **Moving** | Montatura in movimento |
| **Shooting** | Acquisizione immagine in corso |
| **Solving** | Plate solving in corso |
| **Compute** | Calcolo dell'errore polare |

## Valori mostrati

### Misure di calibrazione

| Valore | Descrizione |
|---|---|
| **RA 0 / DE 0 / Time 0** | Soluzione astrometrica e marca temporale dell'immagine 0 |
| **RA 1 / DE 1 / Time 1** | Soluzione astrometrica e marca temporale dell'immagine 1 |
| **RA 2 / DE 2 / Time 2** | Soluzione astrometrica e marca temporale dell'immagine 2 |

### Errore polare

| Valore | Descrizione |
|---|---|
| **Errore azimut (°)** | Errore di azimut in gradi |
| **Errore altitudine (°)** | Errore di altitudine in gradi |
| **Errore totale (°)** | Errore combinato in gradi |
| **Errore azimut (arcmin)** | Errore di azimut in minuti d'arco |
| **Errore altitudine (arcmin)** | Errore di altitudine in minuti d'arco |
| **Errore totale (arcmin)** | Errore combinato in minuti d'arco |

## Azioni

| Azione | Descrizione |
|---|---|
| **Avvia** | Avvia la sequenza di calibrazione (3 immagini + calcolo) poi il loop di correzione |
| **Annulla** | Interrompe immediatamente la sequenza in corso |
