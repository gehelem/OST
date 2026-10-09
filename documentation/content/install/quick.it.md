---
title: Installazione rapida (Ubuntu Resolute)
weight: 5
description: "Il modo più rapido per avviare OST: una macchina Ubuntu 26.04 pulita, uno script"
---

{{% notice style="tip" title="Ideale per i principianti" %}}
Non hai mai usato una riga di comando Linux? Questa pagina è per te.
Segui semplicemente i tre passaggi sotto, in ordine.
{{% /notice %}}

Questo singolo script installa tutto il necessario su una sola
macchina: il back-end OST, l'interfaccia web, nginx, INDI,
`astrometry.net` e la documentazione che stai leggendo in questo
momento.

{{% notice style="note" title="Usi Arch Linux o Fedora invece?" %}}
Questa pagina è specifica per Ubuntu. Su Arch Linux, segui invece
[Installazione rapida (Arch Linux)](../quick-arch/) oppure, su
Fedora, [Installazione rapida (Fedora)](../quick-fedora/).
{{% /notice %}}

---

## 1. Installa un Ubuntu 26.04 pulito

Installa **Ubuntu Server 26.04 LTS ("Resolute")** sulla macchina che
farà funzionare OST — un Raspberry Pi, un mini-PC, o una macchina
virtuale. Usa l'installer standard di Ubuntu; non è necessaria alcuna
opzione speciale.

{{% notice style="note" title="Perché proprio la 26.04?" %}}
Questo script scarica il pacchetto compilato per Ubuntu 26.04. Le
altre versioni supportate (22.04, 24.04) richiedono una procedura
diversa, più manuale — vedi invece la [procedura di installazione
completa](../fresh/).
{{% /notice %}}

Una volta installato Ubuntu, apri un terminale (direttamente sulla
macchina, oppure via SSH da un altro computer).

---

## 2. Scarica lo script di installazione

Copia il comando sotto, incollalo nel terminale, e premi Invio:

```bash
wget https://gitlab.ostserver.fr/ost/ostserver/-/raw/main/install-latest-complete.sh
```

Se funziona, sullo schermo non succede nulla — è normale. Un nuovo
file, `install-latest-complete.sh`, è ora presente nella tua cartella
corrente.

---

## 3. Esegui lo script

```bash
chmod +x install-latest-complete.sh
./install-latest-complete.sh
```

Lo script farà poi tutto da solo. Ecco alcune cose da aspettarsi:

- Chiederà **la tua password** alcune volte (`sudo`) — è normale, ha
  bisogno dei permessi per installare i pacchetti di sistema.
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
cartelle standard, nginx come web server, solo Ubuntu 26.04). Se hai
bisogno di qualcosa di diverso, segui invece la [procedura di
installazione completa](../fresh/) — stesso risultato, ogni passaggio
spiegato e personalizzabile.
{{% /notice %}}
