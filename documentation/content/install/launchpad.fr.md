---
title: Installation depuis le PPA Ubuntu (Launchpad)
weight: 20
description: "Procédure complète sur Ubuntu 26.04 LTS via le dépôt PPA officiel"
---

Prérequis : Ubuntu Server 26.04 (Resolute) vierge, accès sudo, connexion Internet.

Cette méthode utilise le PPA Launchpad officiel — `ostserver-complete` est installé via `apt` et se met à jour automatiquement avec `apt upgrade`.

{{% notice style="note" title="22.04 / 24.04 ?" %}}
Le PPA ne publie actuellement que pour Ubuntu 26.04 — sur 22.04 (Jammy) ou 24.04 (Noble), utilisez plutôt la [procédure manuelle .deb]({{% relref "fresh.fr.md" %}}).
{{% /notice %}}

---

## 1. Mise à jour du système

```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y curl wget gnupg2 ca-certificates lsb-release apt-transport-https
```

---

## 2. Installation d'INDI

INDI est la couche d'abstraction matérielle pilotant caméras, montures, focuseurs et roues à filtres. `ostserver` dépend de `libindi1`, et la plupart des caméras (ZWO, QHY, PlayerOne et autres) ont besoin de leur pilote fourni par `indi-3rdparty-drivers`.

```bash
sudo add-apt-repository -y ppa:mutlaqja/ppa
sudo add-apt-repository -y universe
sudo apt update
sudo apt install -y libindi1 indi-bin gsc indi-3rdparty-libs indi-3rdparty-drivers
```

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
#...
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

Ajoutez le PPA officiel OST et installez `ostserver-complete` — back-end, tous les modules, l'interface web et le câblage nginx entre eux, en un seul paquet :

```bash
sudo add-apt-repository ppa:gehelem/ostserver -y
sudo apt update
sudo apt install -y ostserver-complete
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
sudo apt update && sudo apt upgrade -y
```

`ostserver-complete` se met à jour via `apt upgrade` comme n'importe quel paquet système — nginx et les fichiers statiques de l'interface web suivent avec lui.

---

## Désinstallation

```bash
sudo apt remove ostserver-complete
sudo systemctl disable --now ostserver
sudo rm /etc/systemd/system/ostserver.service
sudo systemctl daemon-reload
```
