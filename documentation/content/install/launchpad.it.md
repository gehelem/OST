---
title: Installazione dal PPA Ubuntu (Launchpad)
weight: 20
description: "Procedura completa su Ubuntu 22.04 / 24.04 / 26.04 LTS tramite il repository PPA ufficiale"
---

Prerequisiti: Ubuntu Server 22.04 (Jammy), 24.04 (Noble) o 26.04 (Resolute) vergine, accesso sudo, connessione Internet.

Questo metodo usa il PPA Launchpad ufficiale — `ostserver` viene installato tramite `apt` e si aggiorna automaticamente con `apt upgrade`.

---

## 1. Aggiornamento del sistema

```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y curl wget gnupg2 ca-certificates lsb-release apt-transport-https
```

---

## 2. Installazione di INDI

INDI è il livello di abstrazione hardware che pilota camere, montature, focheggiatori e ruote portafiltri. `ostserver` dipende da `libindi1`.

{{% notice style="note" title="Perché non il PPA mutlaqja?" %}}
Da agosto 2026, il PPA `ppa:mutlaqja/ppa` pubblica solo per Ubuntu 26.04. OST fornisce quindi un set fisso di pacchetti INDI 2.2.4 — accompagnati da `libxisf` e `gsc` — nel proprio registro, per le tre versioni LTS.
{{% /notice %}}

Scaricare l'archivio corrispondente alla propria versione di Ubuntu e installare il suo contenuto:

```bash
# Scegliere in base alla propria versione:
INDI_TAR=indi-deps-noble.tar      # Ubuntu 24.04 (Noble)
# INDI_TAR=indi-deps-jammy.tar    # Ubuntu 22.04 (Jammy)
# INDI_TAR=indi-deps-resolute.tar # Ubuntu 26.04 (Resolute)

wget "https://gitlab.ostserver.fr/api/v4/projects/1/packages/generic/indi-deps/2.2.4/${INDI_TAR}"
tar xf "${INDI_TAR}"
sudo apt install -y ./indi-deps/*.deb
```

`apt` completa automaticamente le dipendenze rimanenti (`libgsl`, `liberfa`, `libpugixml`…) dal repository `universe` — assicurarsi che sia attivato (vedi §4).

Il pacchetto `gsc` (Global Star Catalog, usato dal simulatore CCD di INDI per produrre un campo di stelle realistico) è incluso nell'archivio — nessun passaggio manuale.

---

## 3. Installazione degli indici Astrometry.net

Richiesti per il plate solving (moduli Navigator e Polar).

```bash
sudo apt install -y astrometry.net
```

I file indice non sono disponibili tramite apt — scaricarli manualmente da `http://data.astrometry.net/`.

{{% notice style="info" title="Scegliere gli indici giusti" %}}
Scaricare solo gli indici adatti al proprio setup. Regola generale: iniziare con gli indici 4208–4210 (campi larghi) poi aggiungere 4206–4207 se si ha una focale lunga. Gli indici sono voluminosi — diverse centinaia di MB ciascuno.
{{% /notice %}}

Decommentare le righe corrispondenti al proprio setup:

```bash
# --- Campi larghi > 5° (grandangolo, focale corta) ---
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4214.fits
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4213.fits
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4212.fits
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4211.fits
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4210.fits
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4209.fits
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4208.fits

# --- Campi medi 1°–5° (focale lunga) — 12 file per indice ---
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-00.fits
#...
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-11.fits
```

---

## 4. Dipendenze di sistema

Il pacchetto `ostserver` richiede diverse librerie Qt6, GSL, cfitsio, libnova e StellarSolver.
`apt` le risolverà automaticamente, ma assicurarsi che il repository `universe` sia attivato:

```bash
sudo add-apt-repository universe -y
sudo apt update
```

`libstellarsolver` è incorporato direttamente nel pacchetto `ostserver` — nessuna versione Qt6 è ancora pubblicata indipendentemente.

---

## 5. Installazione del back-end: ostserver

Aggiungere il PPA ufficiale OST e installare `ostserver`:

```bash
sudo add-apt-repository ppa:gehelem/ostserver -y
sudo apt update
sudo apt install -y ostserver
```

`apt` risolve automaticamente tutte le dipendenze. Il pacchetto è disponibile per Ubuntu 22.04, 24.04 e 26.04 — `apt` installa automaticamente la versione corretta per il proprio sistema.

Il pacchetto installa:
- `/usr/bin/ostserver` — il binario principale
- `/usr/lib/libost*.so` — i moduli (focus, guider, sequencer, navigator, planner, polar, inspector, indipanel, allsky)

{{% notice style="info" title="Servizio systemd incluso" %}}
Il pacchetto installa e attiva automaticamente il servizio `ostserver`. Il servizio viene eseguito con l'utente che ha avviato `sudo apt install`, e la cartella `/var/lib/osterix/media` viene inizializzata.
{{% /notice %}}

{{% notice style="info" title="Porta WebSocket" %}}
`ostserver` è in ascolto WebSocket sulla porta **9624**.
{{% /notice %}}

---

## 6. Installazione del front-end: osterix

Scaricare l'ultima release dal GitLab OST:

```bash
wget https://gitlab.ostserver.fr/api/v4/projects/6/packages/generic/latest/latest/osterix-front_latest.deb
```

Installare il pacchetto (nginx verrà installato automaticamente):

```bash
sudo apt install -y ./osterix-front_latest.deb
```

Il pacchetto installa automaticamente:
- I file statici Angular in `/var/www/osterix/`
- La configurazione Nginx in `/etc/nginx/sites-available/osterix`
- Il collegamento simbolico `/etc/nginx/sites-enabled/osterix`
- Ricarica Nginx

---

## 7. Verifica dell'installazione

```bash
# Stato dei servizi
sudo systemctl status ostserver
sudo systemctl status nginx

# Porte in ascolto
ss -tlnp | grep -E '80|9624|7624'

# Log ostserver in tempo reale
sudo journalctl -u ostserver -f

# Testare la configurazione Nginx
sudo nginx -t
```

---

## 8. Primo avvio

Da un browser sulla rete locale: `http://<ip-della-macchina>/`

1. Avviare il server INDI con i propri driver
2. Aprire l'interfaccia OST → `http://<ip>/`

I moduli sono accessibili dal menu principale.

---

## Riepilogo dei servizi

| Servizio | Ruolo | Porta |
|---|---|---|
| `ostserver` | Back-end C++/Qt, orchestrazione astrofoto | 9624 (WebSocket) |
| `nginx` | Serve il front Angular | 80 |
| `indiserver` | Livello driver hardware INDI | 7624 |

---

## Aggiornamento

```bash
sudo apt update && sudo apt upgrade -y
wget https://gitlab.ostserver.fr/api/v4/projects/6/packages/generic/latest/latest/osterix-front_latest.deb
sudo apt install -y ./osterix-front_latest.deb
sudo systemctl restart ostserver
```

`ostserver` si aggiorna tramite `apt upgrade` come qualsiasi pacchetto di sistema. Nginx si ricarica automaticamente tramite il postinst del pacchetto osterix.

---

## Disinstallazione

```bash
sudo apt remove ostserver osterix-front
sudo systemctl disable --now ostserver
sudo rm /etc/systemd/system/ostserver.service
sudo systemctl daemon-reload
```
