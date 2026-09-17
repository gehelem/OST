---
title: Installation depuis le PPA Ubuntu (Launchpad)
weight: 20
description: "Procédure complète sur Ubuntu 22.04 / 24.04 / 26.04 LTS via le dépôt PPA officiel"
---

Prérequis : Ubuntu Server 22.04 (Jammy), 24.04 (Noble) ou 26.04 (Resolute) vierge, accès sudo, connexion Internet.

Cette méthode utilise le PPA Launchpad officiel — `ostserver` est installé via `apt` et se met à jour automatiquement avec `apt upgrade`.

---

## 1. Mise à jour du système

```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y curl wget gnupg2 ca-certificates lsb-release apt-transport-https
```

---

## 2. Installation d'INDI

INDI est la couche d'abstraction matérielle pilotant caméras, montures, focuseurs et roues à filtres. `ostserver` dépend de `libindi1`.

{{% notice style="note" title="Pourquoi pas le PPA mutlaqja ?" %}}
Depuis août 2026, le PPA `ppa:mutlaqja/ppa` ne publie plus que pour Ubuntu 26.04. OST fournit donc un jeu figé de paquets INDI 2.2.4 — accompagnés de `libxisf` et de `gsc` — dans son propre registre, pour les trois versions LTS.
{{% /notice %}}

Téléchargez l'archive correspondant à votre version d'Ubuntu et installez son contenu :

```bash
# Choisissez selon votre version :
INDI_TAR=indi-deps-noble.tar      # Ubuntu 24.04 (Noble)
# INDI_TAR=indi-deps-jammy.tar    # Ubuntu 22.04 (Jammy)
# INDI_TAR=indi-deps-resolute.tar # Ubuntu 26.04 (Resolute)

wget "https://gitlab.ostserver.fr/api/v4/projects/1/packages/generic/indi-deps/2.2.4/${INDI_TAR}"
tar xf "${INDI_TAR}"
sudo apt install -y ./indi-deps/*.deb
```

`apt` complète automatiquement les dépendances restantes (`libgsl`, `liberfa`, `libpugixml`…) depuis le dépôt `universe` — assurez-vous qu'il est activé (voir §4).

Le paquet `gsc` (Global Star Catalog, utilisé par le simulateur CCD d'INDI pour produire un champ d'étoiles réaliste) est inclus dans l'archive — aucune étape manuelle.

---

## 3. Installation des index Astrometry.net

Requis pour le plate solving (modules Navigator et Polar).

```bash
sudo apt install -y astrometry.net
```

Les fichiers d'index ne sont pas disponibles via apt — les télécharger manuellement depuis `http://data.astrometry.net/`.

{{% notice style="info" title="Choisir les bons index" %}}
Téléchargez uniquement les index adaptés à votre setup. En règle générale : commencez par les index 4208–4210 (champs larges) puis ajoutez 4206–4207 si vous avez une longue focale. Les index sont volumineux — plusieurs centaines de Mo chacun.
{{% /notice %}}

Décommentez les lignes correspondant à votre setup :

```bash
# --- Champs larges > 5° (grand angle, petite focale) ---
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4214.fits
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4213.fits
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4212.fits
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4211.fits
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4210.fits
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4209.fits
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4208.fits

# --- Champs moyens 1°–5° (longue focale) — 12 fichiers par index ---
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-00.fits
#...
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-11.fits
```

---

## 4. Dépendances système

Le paquet `ostserver` requiert plusieurs bibliothèques Qt6, GSL, cfitsio, libnova et StellarSolver.
`apt` les résoudra automatiquement, mais s'assurer que le dépôt `universe` est activé :

```bash
sudo add-apt-repository universe -y
sudo apt update
```

`libstellarsolver` est embarqué directement dans le paquet `ostserver` — aucune version Qt6 n'est encore publiée indépendamment.

---

## 5. Installation du back-end : ostserver

Ajouter le PPA officiel OST et installer `ostserver` :

```bash
sudo add-apt-repository ppa:gehelem/ostserver -y
sudo apt update
sudo apt install -y ostserver
```

`apt` résout automatiquement toutes les dépendances. Le paquet est disponible pour Ubuntu 22.04, 24.04 et 26.04 — `apt` installe automatiquement la version correcte pour votre système.

Le paquet installe :
- `/usr/bin/ostserver` — le binaire principal
- `/usr/lib/libost*.so` — les modules (focus, guider, sequencer, navigator, planner, polar, inspector, indipanel, allsky)

{{% notice style="info" title="Service systemd inclus" %}}
Le paquet installe et active automatiquement le service `ostserver`. Le service s'exécute sous l'utilisateur qui a lancé `sudo apt install`, et le répertoire `/var/lib/osterix/media` est initialisé.
{{% /notice %}}

{{% notice style="info" title="Port WebSocket" %}}
`ostserver` écoute en WebSocket sur le port **9624**.
{{% /notice %}}

---

## 6. Installation du front-end : osterix

Télécharger la dernière release depuis le GitLab OST :

```bash
wget https://gitlab.ostserver.fr/api/v4/projects/6/packages/generic/latest/latest/osterix-front_latest.deb
```

Installer le paquet (nginx sera installé automatiquement) :

```bash
sudo apt install -y ./osterix-front_latest.deb
```

Le paquet installe automatiquement :
- Les fichiers statiques Angular dans `/var/www/osterix/`
- La config Nginx dans `/etc/nginx/sites-available/osterix`
- Le lien symbolique `/etc/nginx/sites-enabled/osterix`
- Recharge Nginx

---

## 7. Vérification de l'installation

```bash
# État des services
sudo systemctl status ostserver
sudo systemctl status nginx

# Ports en écoute
ss -tlnp | grep -E '80|9624|7624'

# Logs ostserver en temps réel
sudo journalctl -u ostserver -f

# Tester la config Nginx
sudo nginx -t
```

---

## 8. Premier démarrage

Depuis un navigateur sur le réseau local : `http://<ip-de-la-machine>/`

1. Démarrer le serveur INDI avec vos pilotes
2. Ouvrir l'interface OST → `http://<ip>/`

Les modules sont accessibles depuis le menu principal.

---

## Récapitulatif des services

| Service | Rôle | Port |
|---|---|---|
| `ostserver` | Back-end C++/Qt, orchestration astrophoto | 9624 (WebSocket) |
| `nginx` | Sert le front Angular | 80 |
| `indiserver` | Couche pilotes matériels INDI | 7624 |

---

## Mise à jour

```bash
sudo apt update && sudo apt upgrade -y
wget https://gitlab.ostserver.fr/api/v4/projects/6/packages/generic/latest/latest/osterix-front_latest.deb
sudo apt install -y ./osterix-front_latest.deb
sudo systemctl restart ostserver
```

`ostserver` se met à jour via `apt upgrade` comme n'importe quel paquet système. Nginx se recharge automatiquement via le postinst du paquet osterix.

---

## Désinstallation

```bash
sudo apt remove ostserver osterix-front
sudo systemctl disable --now ostserver
sudo rm /etc/systemd/system/ostserver.service
sudo systemctl daemon-reload
```
