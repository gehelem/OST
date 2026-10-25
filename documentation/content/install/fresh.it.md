---
title: Installazione dai pacchetti .deb
weight: 10
description: "Procedura completa su Ubuntu 22.04 / 24.04 / 26.04 LTS a partire dai pacchetti .deb GitLab"
---

Prerequisiti: Ubuntu Server 22.04 (Jammy), 24.04 (Noble) o 26.04 (Resolute) vergine, accesso sudo, connessione Internet.

---

## 1. Aggiornamento del sistema

```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y curl wget gnupg2 ca-certificates lsb-release apt-transport-https
```

---

## 2. Installazione di INDI

INDI è il livello di abstrazione hardware che pilota camere, montature, focheggiatori e ruote portafiltri. `ostserver` dipende da `libindi1`, e la maggior parte delle camere (ZWO, QHY, PlayerOne e altre) necessita del proprio driver da `indi-3rdparty-drivers`.

### Ubuntu 26.04 (Resolute)

```bash
sudo add-apt-repository -y ppa:mutlaqja/ppa
sudo add-apt-repository -y universe
sudo apt update
sudo apt install -y libindi1 indi-bin gsc indi-3rdparty-libs indi-3rdparty-drivers
```

{{% notice style="note" title="22.04 / 24.04?" %}}
Da agosto 2026 il PPA `ppa:mutlaqja/ppa` pubblica solo per Ubuntu 26.04 — su 22.04 (Jammy) o 24.04 (Noble), usare invece l'archivio fisso sotto.
{{% /notice %}}

### Ubuntu 22.04 / 24.04 (archivio fisso)

```bash
# Pick the one for your release:
INDI_TAR=indi-deps-noble.tar      # Ubuntu 24.04 (Noble)
# INDI_TAR=indi-deps-jammy.tar    # Ubuntu 22.04 (Jammy)

wget "https://gitlab.ostserver.fr/api/v4/projects/1/packages/generic/indi-deps/2.2.4/${INDI_TAR}"
tar xf "${INDI_TAR}"
sudo apt install -y ./indi-deps/*.deb
```

`apt` completa automaticamente le dipendenze rimanenti (`libgsl`, `liberfa`, `libpugixml`…) dal repository `universe` — assicurarsi che sia attivato (vedi §4). Il pacchetto `gsc` (Global Star Catalog, usato dal simulatore CCD di INDI per produrre un campo di stelle realistico) è incluso nell'archivio.

{{% notice style="note" title="Ancora nessun driver camera di terze parti qui" %}}
Questo archivio fisso copre solo `libindi1`/`libxisf`/`gsc` — non include `indi-3rdparty-drivers` (ZWO/QHY/PlayerOne…) per 22.04/24.04 al momento.
{{% /notice %}}

---

## 3. Installazione degli indici Astrometry.net

Richiesti per il plate solving (moduli Navigator e Polar).

```bash
sudo apt install -y astrometry.net
```

I file indice non sono disponibili tramite apt — scaricarli manualmente da `http://data.astrometry.net/`.

{{% notice style="info" title="Scegliere gli indici giusti" %}}
Scaricare solo gli indici adatti al proprio setup. Come punto di partenza: gli indici 4208–4210 coprono campi larghi. Aggiungere 4206–4207 per focali più lunghe. Ogni indice pesa diverse centinaia di MB.
{{% /notice %}}

Decommentare le righe corrispondenti al proprio setup:

```bash
# --- Wide field > 5° (wide-angle, short focal length) ---
sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4214.fits
sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4213.fits
sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4212.fits
sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4211.fits
sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4210.fits
sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4209.fits
sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4208.fits

# --- Medium field 1°–5° (long focal length) — 12 files per index ---
#sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-00.fits
#sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-01.fits
#sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-02.fits
#sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-03.fits
#sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-04.fits
#sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-05.fits
#sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-06.fits
#sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-07.fits
#sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-08.fits
#sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-09.fits
#sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-10.fits
#sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-11.fits
#sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-00.fits
#sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-01.fits
#sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-02.fits
#sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-03.fits
#sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-04.fits
#sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-05.fits
#sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-06.fits
#sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-07.fits
#sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-08.fits
#sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-09.fits
#sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-10.fits
#sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-11.fits
```

Per scaricare tutti gli indici in una volta (diversi GB):

```bash
for idx in 4208 4209 4210 4211 4212 4213 4214; do
    sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-${idx}.fits
done
for idx in 4206 4207; do
    for i in $(seq -w 0 11); do
        sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-${idx}-${i}.fits
    done
done
```

`-nc` (no-clobber) salta un file già scaricato — si può rilanciare questa sezione in sicurezza per completare un set parziale.

---

## 4. Dipendenze di sistema

Il pacchetto `ostserver-complete` richiede diverse librerie Qt6, GSL, cfitsio, libnova e StellarSolver, oltre a nginx.
`apt` le risolverà automaticamente, ma assicurarsi che il repository `universe` sia attivato:

```bash
sudo add-apt-repository universe -y
sudo apt update
```

`libstellarsolver` è incorporato direttamente nel pacchetto — nessuna versione Qt6 è ancora pubblicata indipendentemente.

---

## 5. Installazione di ostserver-complete

Un singolo pacchetto fornisce il back-end, tutti i moduli, l'interfaccia web e il collegamento nginx tra di essi. Scaricare quello corrispondente alla propria versione di Ubuntu dal GitLab OST:

```bash
DEB=ostserver-complete_latest_2604_amd64.deb   # Ubuntu 26.04 (Resolute)
# DEB=ostserver-complete_latest_2404_amd64.deb # Ubuntu 24.04 (Noble)
# DEB=ostserver-complete_latest_2204_amd64.deb # Ubuntu 22.04 (Jammy)

wget "https://gitlab.ostserver.fr/api/v4/projects/1/packages/generic/latest/latest/${DEB}"
sudo apt install -y "./${DEB}"
```

`apt` risolve automaticamente tutte le dipendenze, nginx incluso.

Il pacchetto installa:
- `/usr/bin/ostserver` — il binario principale
- `/usr/lib/libost*.so` — i moduli (focus, guider, sequencer, navigator, planner, polar, inspector, indipanel, allsky)
- i file statici dell'interfaccia web, con un sito nginx già configurato per servirli

{{% notice style="info" title="Servizio systemd incluso" %}}
Il pacchetto installa e attiva automaticamente il servizio `ostserver`. Il servizio viene eseguito con l'utente che ha avviato `sudo apt install`, e la cartella `/var/lib/osterix/media` viene inizializzata.
{{% /notice %}}

{{% notice style="info" title="Porta WebSocket" %}}
`ostserver` è in ascolto WebSocket sulla porta **9624**.
{{% /notice %}}

---

## 6. Verifica dell'installazione

```bash
# Service status
sudo systemctl status ostserver
sudo systemctl status nginx

# Listening ports
ss -tlnp | grep -E '80|9624|7624'

# Live ostserver logs
sudo journalctl -u ostserver -f

# Test Nginx config
sudo nginx -t
```

---

## 7. Primo avvio

Da un browser sulla rete locale: `http://<machine-ip>/`

1. Avviare il server INDI con i propri driver
2. Aprire l'interfaccia OST → `http://<ip>/`

I moduli sono accessibili dal menu principale.

---

## Riepilogo dei servizi

| Servizio | Ruolo | Porta |
|---|---|---|
| `ostserver` | Back-end C++/Qt, orchestrazione astrofoto | 9624 (WebSocket) |
| `nginx` | Serve l'interfaccia web | 80 |
| `indiserver` | Livello driver hardware INDI | 7624 |

---

## Aggiornamento

```bash
DEB=ostserver-complete_latest_2604_amd64.deb   # Ubuntu 26.04 (Resolute)
# DEB=ostserver-complete_latest_2404_amd64.deb # Ubuntu 24.04 (Noble)
# DEB=ostserver-complete_latest_2204_amd64.deb # Ubuntu 22.04 (Jammy)

wget "https://gitlab.ostserver.fr/api/v4/projects/1/packages/generic/latest/latest/${DEB}"
sudo apt install -y "./${DEB}"
sudo systemctl restart ostserver
```

---

## Disinstallazione

```bash
sudo apt remove ostserver-complete
sudo systemctl disable --now ostserver
sudo rm /etc/systemd/system/ostserver.service
sudo systemctl daemon-reload
```
