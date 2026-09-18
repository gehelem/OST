---
title: Navigator
weight: 40
description: Ricerca di target e messa in stazione precisa tramite plate solving
---

## Ruolo

Il modulo Navigator permette di ricercare un oggetto nei catalogi astronomici, di puntare la montatura verso quell'oggetto, poi di centrare precisamente il campo in loop tramite il plate solving (risoluzione astrometrica). Può anche inviare il target selezionato al modulo Planner.

![Screenshot del modulo Navigator](/images/modules/navigator.png)

## Dispositivi richiesti

| Dispositivo | Ruolo |
|---|---|
| Camera | Acquisizione delle immagini per il plate solving |
| Montatura | Ricezione degli ordini di spostamento |
| GPS | Fornitura delle coordinate geografiche e dell'ora |

## Parametri

### Acquisizione

| Parametro | Descrizione |
|---|---|
| **Esposizione** | Durata dell'esposizione in secondi |
| **Guadagno** | Guadagno della camera |
| **Offset** | Offset della camera |
| **Focale** | Focale ottica in mm — usata per calcolare la scala immagine del solver |

### Centratura

| Parametro | Descrizione |
|---|---|
| **Iterazioni max** | Numero massimo di cicli slew → solve → correct prima di abbandonare |
| **Tolleranza (arcsec)** | Distanza angolare accettabile tra il centro risolto e il target |
| **Sincronizzare la montatura** | Se attivato, sincronizza le coordinate della montatura sulla soluzione dopo una centratura riuscita |

### Moduli slave

| Parametro | Descrizione |
|---|---|
| **Istanza del modulo Planner** | Nome dell'istanza del modulo Planner da usare per l'azione *Aggiungi alla pianificazione* |

## Funzionamento

### Ricerca di target

La ricerca si effettua inserendo un nome o codice nel campo **Ricerca un oggetto**. I catalogi disponibili sono:

| Catalogo | Contenuto |
|---|---|
| Messier | Oggetti del catalogo Messier |
| NGC | New General Catalogue |
| LDN | Lynds' Catalogue of Dark Nebulae |
| SH2 | Sharpless catalogue (nebulose H II) |
| IC | Index Catalogue |
| Stelle | Stelle nominate |

I risultati vengono mostrati nella griglia **Risultati** con: catalogo, codice, RA, DEC, magnitudine, diametro, nome e alias. Un clic su una riga seleziona il target.

### Conversione J2000 → epoca attuale

Non appena un target viene selezionato, le sue coordinate J2000 vengono convertite in coordinate dell'epoca attuale (precessione presa in conto) e mostrate in **Selezione convertita**.

### Loop di centratura

L'azione **Goto target** avvia la sequenza seguente:

1. La montatura punta verso le coordinate convertite all'epoca attuale
2. La camera acquisisce un'immagine che viene risolta dal solver astrometrico (StellarSolver / indici astrometry.net)
3. Viene calcolata la distanza angolare tra il centro risolto e il target
4. Se la distanza è inferiore alla **Tolleranza**, la centratura è dichiarata riuscita
5. Altrimenti, viene calcolato e applicato alla montatura un offset correttivo; il ciclo riprende

Questo processo si ripete fino al successo o fino a raggiungere il numero massimo di iterazioni.

{{% notice style="tip" title="Sincronizzazione montatura" %}}
Se *Sincronizzare la montatura* è attivato, il modulo effettua un SYNC INDI dopo la centratura riuscita, il che migliora la precisione dei puntamenti successivi.
{{% /notice %}}

## Valori mostrati

### Posizione della montatura

| Valore | Descrizione |
|---|---|
| **RA** | Ascensione retta corrente della montatura |
| **DEC** | Declinazione corrente della montatura |

### Posizione GPS

| Valore | Descrizione |
|---|---|
| **Altitudine** | Altitudine del sito di osservazione (m) |
| **Latitudine** | Latitudine del sito di osservazione (°) |
| **Longitudine** | Longitudine del sito di osservazione (°) |

### Data e ora GPS

| Valore | Descrizione |
|---|---|
| **Scostamento UTC** | Fuso orario |
| **Data** | Data corrente |
| **Ora** | Ora corrente |

## Azioni

| Azione | Descrizione |
|---|---|
| **Goto target** | Punta la montatura verso il target selezionato e avvia il loop di centratura |
| **Annulla** | Interrompe immediatamente il goto o la centratura in corso |
| **Aggiungi alla pianificazione** | Invia il target corrente al modulo Planner |
