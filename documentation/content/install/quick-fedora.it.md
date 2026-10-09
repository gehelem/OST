---
title: Installazione rapida (Fedora)
weight: 7
description: "Il modo più rapido per avviare OST su Fedora: uno script"
---

{{% notice style="tip" title="Ideale per i principianti" %}}
Non hai mai usato una riga di comando Linux? Questa pagina è per te.
Segui semplicemente i tre passaggi sotto, in ordine.
{{% /notice %}}

Questo singolo script installa tutto il necessario su una sola
macchina: il back-end OST, l'interfaccia web, nginx, INDI,
`astrometry.net` e la documentazione che stai leggendo in questo
momento.

{{% notice style="note" title="Usi Ubuntu o Arch Linux invece?" %}}
Questa pagina è specifica per Fedora. Su Ubuntu, segui invece
[Installazione rapida (Ubuntu Resolute)](../quick/) oppure, su Arch
Linux, [Installazione rapida (Arch Linux)](../quick-arch/).
{{% /notice %}}

---

## 1. Installa un Fedora pulito

Installa **Fedora Server** (o Workstation) sulla macchina che farà
funzionare OST — un Raspberry Pi, un mini-PC, o una macchina virtuale.
Usa l'installer standard di Fedora; non è necessaria alcuna opzione
speciale.

Una volta installato Fedora, apri un terminale (direttamente sulla
macchina, oppure via SSH da un altro computer).

---

## 2. Scarica lo script di installazione

Copia il comando sotto, incollalo nel terminale, e premi Invio:

```bash
wget https://gitlab.ostserver.fr/ost/ostserver/-/raw/main/install-latest-complete-fedora.sh
```

Se funziona, sullo schermo non succede nulla — è normale. Un nuovo
file, `install-latest-complete-fedora.sh`, è ora presente nella tua
cartella corrente.

---

## 3. Esegui lo script

```bash
chmod +x install-latest-complete-fedora.sh
./install-latest-complete-fedora.sh
```

Lo script farà poi tutto da solo. Ecco alcune cose da aspettarsi:

- Chiederà **la tua password** alcune volte (`sudo`) — è normale, ha
  bisogno dei permessi per installare i pacchetti di sistema.
- Apre le porte dell'interfaccia web (80) e del WebSocket (9624) in
  `firewalld`, attivo di default su Fedora (a differenza di
  Ubuntu/Arch), che altrimenti le bloccherebbe entrambe da qualsiasi
  altra macchina della tua rete.
- Rimuove la pagina predefinita che nginx installa di default su
  Fedora (integrata direttamente in `/etc/nginx/nginx.conf`) -
  altrimenti prevarrebbe silenziosamente sulla pagina di OST sulla
  porta 80.
- Installa `gsc` (dati di campo stellare realistico solo per il
  *simulatore* di CCD di INDI, mai necessario per un hardware reale)
  da un pacchetto precompilato nostro - non esiste alcun pacchetto
  Fedora ufficiale per questo.
- Potrebbe apparire una schermata blu che chiede quali file indice di
  `astrometry.net` scaricare in aggiunta a quelli predefiniti. Usa le
  **freccette** per spostarti, **Spazio** per selezionare/deselezionare,
  **Tab** poi **Invio** per confermare. Se non sei sicuro, lascia tutto
  deselezionato e premi Invio — potrai sempre aggiungerli in seguito.
- Mostra messaggi di avanzamento mentre scarica e installa. Questo può
  richiedere alcuni minuti, a seconda della tua connessione Internet.

Alla fine, mostra due indirizzi, ad esempio:

```
Web UI: http://192.168.1.42/
Documentation: http://192.168.1.42/documentation/
```

---

## Fatto

Apri il primo indirizzo (`Web UI: ...`) in un browser web, da
qualsiasi dispositivo sulla stessa rete locale della macchina su cui
hai appena installato OST. Dovresti vedere l'interfaccia OST.

{{% notice style="info" title="Hai bisogno di più controllo?" %}}
Questo script fa delle scelte per te (SSL disattivato, struttura di
cartelle standard, nginx come web server). Se hai bisogno di qualcosa
di diverso, installa invece il pacchetto `ostserver`/`ostserver-complete`
manualmente (il `.rpm` precompilato nella pagina delle release) e
configura il resto da solo — i passaggi dello script stesso fungono da
guida per cosa fare manualmente.
{{% /notice %}}
