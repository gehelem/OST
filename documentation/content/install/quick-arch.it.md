---
title: Installazione rapida (Arch Linux)
weight: 6
description: "Il modo più rapido per avviare OST su Arch Linux: uno script"
---

{{% notice style="tip" title="Ideale per i principianti" %}}
Non hai mai usato una riga di comando Linux? Questa pagina è per te.
Segui semplicemente i tre passaggi sotto, in ordine.
{{% /notice %}}

Questo singolo script installa tutto il necessario su una sola
macchina: il back-end OST, l'interfaccia web, nginx, INDI,
`astrometry.net` e la documentazione che stai leggendo in questo
momento.

{{% notice style="note" title="Usi Ubuntu invece?" %}}
Questa pagina è specifica per Arch Linux. Su Ubuntu, segui invece
[Installazione rapida (Ubuntu Resolute)](../quick/).
{{% /notice %}}

---

## 1. Installa un Arch Linux pulito

Installa **Arch Linux** sulla macchina che farà funzionare OST — un
Raspberry Pi, un mini-PC, o una macchina virtuale. Usa l'installer
standard di Arch; non è necessaria alcuna opzione speciale.

Una volta installato Arch, apri un terminale (direttamente sulla
macchina, oppure via SSH da un altro computer).

---

## 2. Scarica lo script di installazione

Un'installazione minima di Arch non include `wget` - installalo
prima:

```bash
sudo pacman -S --needed wget
```

Poi copia il comando sotto, incollalo nel terminale, e premi Invio:

```bash
wget https://gitlab.ostserver.fr/ost/ostserver/-/raw/main/install-latest-complete-arch.sh
```

Se funziona, sullo schermo non succede nulla — è normale. Un nuovo
file, `install-latest-complete-arch.sh`, è ora presente nella tua
cartella corrente.

---

## 3. Esegui lo script

```bash
chmod +x install-latest-complete-arch.sh
./install-latest-complete-arch.sh
```

Lo script farà poi tutto da solo. Ecco alcune cose da aspettarsi:

- Chiederà **la tua password** alcune volte (`sudo`) — è normale, ha
  bisogno dei permessi per installare i pacchetti di sistema.
- Compila due pacchetti da AUR (`astrometry.net` e `gsc`) - questo
  compila codice reale sulla tua macchina e può richiedere alcuni
  minuti. È normale vedere pagine di output del compilatore scorrere
  sullo schermo.
- È noto che `gsc` al momento non riesce a compilare (un vecchio
  pacchetto AUR non più mantenuto - non è un problema di OST). Lo
  script stampa un avviso e continua senza di esso: `gsc` serve solo
  ad alimentare il *simulatore* di CCD di INDI con un campo stellare
  realistico, non è mai necessario per un hardware reale.
- Potrebbe apparire una schermata blu che chiede quali file indice di
  `astrometry.net` scaricare in aggiunta a quelli predefiniti. Usa le
  **freccette** per spostarti, **Spazio** per selezionare/deselezionare,
  **Tab** poi **Invio** per confermare. Se non sei sicuro, lascia tutto
  deselezionato e premi Invio — potrai sempre aggiungerli in seguito.

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
manualmente (da AUR, oppure il `.pkg.tar.zst` precompilato nella pagina
delle release) e configura il resto da solo — i passaggi dello script
stesso fungono da guida per cosa fare manualmente.
{{% /notice %}}
