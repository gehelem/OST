---
title: Installation rapide (Fedora)
weight: 7
description: "La façon la plus rapide de faire fonctionner OST sur Fedora : un script"
---

{{% notice style="tip" title="Idéal pour les débutants" %}}
Vous n'avez jamais utilisé de ligne de commande Linux ? Cette page est
faite pour vous. Suivez simplement les trois étapes ci-dessous, dans
l'ordre.
{{% /notice %}}

Ce script unique installe tout ce qui est nécessaire sur une seule
machine : le back-end OST, l'interface web, nginx, INDI,
`astrometry.net` et la documentation que vous êtes en train de lire.

{{% notice style="note" title="Vous utilisez Ubuntu ou Arch Linux plutôt ?" %}}
Cette page concerne spécifiquement Fedora. Suivez plutôt
l'[Installation rapide (Ubuntu Resolute)](../quick/) ou
l'[Installation rapide (Arch Linux)](../quick-arch/).
{{% /notice %}}

---

## 1. Installer une Fedora vierge

Installez **Fedora Server** (ou Workstation) sur la machine qui va
faire tourner OST — un Raspberry Pi, un mini-PC ou une machine
virtuelle. Utilisez l'installeur Fedora standard ; aucune option
particulière n'est nécessaire.

Une fois Fedora installé, ouvrez un terminal (directement sur la
machine, ou en SSH depuis un autre ordinateur).

---

## 2. Télécharger le script d'installation

Copiez la commande ci-dessous, collez-la dans le terminal, puis
appuyez sur Entrée :

```bash
wget https://gitlab.ostserver.fr/ost/ostserver/-/raw/main/install-latest-complete-fedora.sh
```

Rien ne s'affiche à l'écran si tout fonctionne — c'est normal. Un
nouveau fichier, `install-latest-complete-fedora.sh`, existe
maintenant dans votre dossier courant.

---

## 3. Exécuter le script

```bash
chmod +x install-latest-complete-fedora.sh
./install-latest-complete-fedora.sh
```

Le script fait alors tout lui-même. Voici ce à quoi vous attendre :

- Il va demander **votre mot de passe** plusieurs fois (`sudo`) —
  c'est normal, il a besoin de la permission d'installer des paquets
  système.
- Il ouvre les ports de l'interface web (80) et du WebSocket (9624)
  dans `firewalld`, activé par défaut sur Fedora (contrairement à
  Ubuntu/Arch), qui bloquerait sinon les deux depuis n'importe quelle
  autre machine de votre réseau.
- Il supprime la page par défaut livrée avec nginx sur Fedora
  (intégrée directement dans `/etc/nginx/nginx.conf`) - sinon elle
  prend silencieusement le pas sur la page d'OST sur le port 80.
- Il installe `gsc` (données de champ d'étoiles réaliste pour le
  *simulateur* de caméra CCD d'INDI uniquement, jamais nécessaire pour
  du matériel réel) depuis un paquet précompilé maison - aucun paquet
  Fedora officiel n'existe pour celui-ci.
- Un écran bleu peut apparaître, demandant quels fichiers d'index
  `astrometry.net` télécharger en plus de ceux par défaut. Utilisez
  les **flèches** pour vous déplacer, **Espace** pour cocher/décocher,
  **Tab** puis **Entrée** pour confirmer. Si vous n'êtes pas sûr,
  laissez tout décoché et appuyez sur Entrée — vous pourrez toujours
  les ajouter plus tard.
- Il affiche des messages de progression pendant le téléchargement et
  l'installation. Cela peut prendre quelques minutes selon votre
  connexion Internet.

À la toute fin, il affiche deux adresses, par exemple :

```
Web UI: http://192.168.1.42/
Documentation: http://192.168.1.42/documentation/
```

---

## Terminé

Ouvrez la première adresse (`Web UI: ...`) dans un navigateur web,
depuis n'importe quel appareil sur le même réseau local que la
machine sur laquelle vous venez d'installer OST. Vous devriez voir
l'interface OST.

{{% notice style="info" title="Besoin de plus de contrôle ?" %}}
Ce script fait des choix à votre place (SSL désactivé, arborescence
de dossiers standard, nginx comme serveur web). Si vous avez besoin
d'autre chose, installez plutôt le paquet
`ostserver`/`ostserver-complete` à la main (le `.rpm` précompilé sur
la page des releases) et configurez le reste vous-même — les étapes
du script font elles-mêmes office de guide pour ce qu'il faut faire
manuellement.
{{% /notice %}}
