---
title: Installation depuis les paquets .deb
weight: 10
description: "Procédure complète sur Ubuntu 22.04 / 24.04 / 26.04 LTS à partir des paquets .deb GitLab"
---

Prérequis : Ubuntu Server 22.04 (Jammy), 24.04 (Noble) ou 26.04 (Resolute) vierge, accès sudo, connexion Internet.

---

## 1. Mise à jour du système

```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y curl wget gnupg2 ca-certificates lsb-release apt-transport-https
```

---

## 2. Installation d'INDI

INDI est la couche d'abstraction matérielle pilotant caméras, montures, focuseurs et roues à filtres. `ostserver` dépend de `libindi1`, et la plupart des caméras (ZWO, QHY, PlayerOne et autres) ont besoin de leur pilote fourni par `indi-3rdparty-drivers`.

### Ubuntu 26.04 (Resolute)

```bash
sudo add-apt-repository -y ppa:mutlaqja/ppa
sudo add-apt-repository -y universe
sudo apt update
sudo apt install -y libindi1 indi-bin gsc indi-3rdparty-libs indi-3rdparty-drivers
```

{{% notice style="note" title="22.04 / 24.04 ?" %}}
Depuis août 2026, `ppa:mutlaqja/ppa` ne publie plus que pour Ubuntu 26.04 — sur 22.04 (Jammy) ou 24.04 (Noble), utilisez plutôt l'archive figée ci-dessous.
{{% /notice %}}

### Ubuntu 22.04 / 24.04 (archive figée)

```bash
# Pick the one for your release:
INDI_TAR=indi-deps-noble.tar      # Ubuntu 24.04 (Noble)
# INDI_TAR=indi-deps-jammy.tar    # Ubuntu 22.04 (Jammy)

wget "https://gitlab.ostserver.fr/api/v4/projects/1/packages/generic/indi-deps/2.2.4/${INDI_TAR}"
tar xf "${INDI_TAR}"
sudo apt install -y ./indi-deps/*.deb
```

`apt` complète automatiquement les dépendances restantes (`libgsl`, `liberfa`, `libpugixml`…) depuis le dépôt `universe` — assurez-vous qu'il est activé (voir §4). Le paquet `gsc` (Global Star Catalog, utilisé par le simulateur CCD d'INDI pour produire un champ d'étoiles réaliste) est inclus dans l'archive.

{{% notice style="note" title="Pas encore de pilotes caméra tiers ici" %}}
Cette archive figée ne couvre que `libindi1`/`libxisf`/`gsc` — elle n'inclut pas `indi-3rdparty-drivers` (ZWO/QHY/PlayerOne…) pour 22.04/24.04 pour le moment.
{{% /notice %}}

---

## 3. Installation des index Astrometry.net

Requis pour le plate solving (modules Navigator et Polar).

```bash
sudo apt install -y astrometry.net
```

Les fichiers d'index ne sont pas disponibles via apt — les télécharger manuellement depuis `http://data.astrometry.net/`.

{{% notice style="info" title="Choisir les bons index" %}}
Ne téléchargez que les index adaptés à votre setup. Pour commencer : les index 4208–4210 couvrent les champs larges. Ajoutez 4206–4207 pour les focales plus longues. Chaque index pèse plusieurs centaines de Mo.
{{% /notice %}}

Décommentez les lignes correspondant à votre setup :

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

Pour télécharger tous les index d'un coup (plusieurs Go) :

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

`-nc` (no-clobber) ignore un fichier déjà téléchargé — vous pouvez relancer cette section sans risque pour compléter un jeu partiel.

---

## 4. Dépendances système

Le paquet `ostserver-complete` requiert plusieurs bibliothèques Qt6, GSL, cfitsio, libnova et StellarSolver, ainsi que nginx.
`apt` les résoudra automatiquement, mais s'assurer que le dépôt `universe` est activé :

```bash
sudo add-apt-repository universe -y
sudo apt update
```

`libstellarsolver` est embarqué directement dans le paquet — aucune version Qt6 n'est encore publiée indépendamment.

---

## 5. Installation d'ostserver-complete

Un seul paquet fournit le back-end, tous les modules, l'interface web et le câblage nginx entre eux. Téléchargez celui correspondant à votre version d'Ubuntu depuis le GitLab OST :

```bash
DEB=ostserver-complete_latest_2604_amd64.deb   # Ubuntu 26.04 (Resolute)
# DEB=ostserver-complete_latest_2404_amd64.deb # Ubuntu 24.04 (Noble)
# DEB=ostserver-complete_latest_2204_amd64.deb # Ubuntu 22.04 (Jammy)

wget "https://gitlab.ostserver.fr/api/v4/projects/1/packages/generic/latest/latest/${DEB}"
sudo apt install -y "./${DEB}"
```

`apt` résout automatiquement toutes les dépendances, nginx inclus.

Le paquet installe :
- `/usr/bin/ostserver` — le binaire principal
- `/usr/lib/libost*.so` — les modules (focus, guider, sequencer, navigator, planner, polar, inspector, indipanel, allsky)
- les fichiers statiques de l'interface web, avec un site nginx déjà configuré pour les servir

{{% notice style="info" title="Service systemd inclus" %}}
Le paquet installe et active automatiquement le service `ostserver`. Le service s'exécute sous l'utilisateur qui a lancé `sudo apt install`, et le répertoire `/var/lib/osterix/media` est initialisé.
{{% /notice %}}

{{% notice style="info" title="Port WebSocket" %}}
`ostserver` écoute en WebSocket sur le port **9624**.
{{% /notice %}}

---

## 6. Vérification de l'installation

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

## 7. Premier démarrage

Depuis un navigateur sur le réseau local : `http://<machine-ip>/`

1. Démarrer le serveur INDI avec vos pilotes
2. Ouvrir l'interface OST → `http://<ip>/`

Les modules sont accessibles depuis le menu principal.

---

## Récapitulatif des services

| Service | Rôle | Port |
|---|---|---|
| `ostserver` | Back-end C++/Qt, orchestration astrophoto | 9624 (WebSocket) |
| `nginx` | Sert l'interface web | 80 |
| `indiserver` | Couche pilotes matériels INDI | 7624 |

---

## Mise à jour

```bash
DEB=ostserver-complete_latest_2604_amd64.deb   # Ubuntu 26.04 (Resolute)
# DEB=ostserver-complete_latest_2404_amd64.deb # Ubuntu 24.04 (Noble)
# DEB=ostserver-complete_latest_2204_amd64.deb # Ubuntu 22.04 (Jammy)

wget "https://gitlab.ostserver.fr/api/v4/projects/1/packages/generic/latest/latest/${DEB}"
sudo apt install -y "./${DEB}"
sudo systemctl restart ostserver
```

---

## Désinstallation

```bash
sudo apt remove ostserver-complete
sudo systemctl disable --now ostserver
sudo rm /etc/systemd/system/ostserver.service
sudo systemctl daemon-reload
```
