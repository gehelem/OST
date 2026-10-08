---
title: Installation rapide (Arch Linux)
weight: 6
description: "La façon la plus rapide de faire fonctionner OST sur Arch Linux : un script"
---

{{% notice style="tip" title="Idéal pour les débutants" %}}
Vous n'avez jamais utilisé de ligne de commande Linux ? Cette page est
faite pour vous. Suivez simplement les trois étapes ci-dessous, dans
l'ordre.
{{% /notice %}}

Ce script unique installe tout ce qui est nécessaire sur une seule
machine : le back-end OST, l'interface web, nginx, INDI,
`astrometry.net` et la documentation que vous êtes en train de lire.

{{% notice style="note" title="Vous utilisez Ubuntu plutôt ?" %}}
Cette page concerne spécifiquement Arch Linux. Sous Ubuntu, suivez
plutôt l'[Installation rapide (Ubuntu Resolute)](../quick/).
{{% /notice %}}

---

## 1. Installer une Arch Linux vierge

Installez **Arch Linux** sur la machine qui va faire tourner OST — un
Raspberry Pi, un mini-PC ou une machine virtuelle. Utilisez
l'installeur Arch standard ; aucune option particulière n'est
nécessaire.

Une fois Arch installé, ouvrez un terminal (directement sur la
machine, ou en SSH depuis un autre ordinateur).

---

## 2. Télécharger le script d'installation

Une installation Arch minimale n'inclut pas `wget` - installez-le
d'abord :

```bash
sudo pacman -S --needed wget
```

Copiez ensuite la commande ci-dessous, collez-la dans le terminal,
puis appuyez sur Entrée :

```bash
wget https://gitlab.ostserver.fr/ost/ostserver/-/raw/main/install-latest-complete-arch.sh
```

Rien ne s'affiche à l'écran si tout fonctionne — c'est normal. Un
nouveau fichier, `install-latest-complete-arch.sh`, existe maintenant
dans votre dossier courant.

---

## 3. Exécuter le script

```bash
chmod +x install-latest-complete-arch.sh
./install-latest-complete-arch.sh
```

Le script fait alors tout lui-même. Voici ce à quoi vous attendre :

- Il va demander **votre mot de passe** plusieurs fois (`sudo`) —
  c'est normal, il a besoin de la permission d'installer des paquets
  système.
- Il compile deux paquets depuis AUR (`astrometry.net` et `gsc`) -
  cela compile du vrai code sur votre machine et peut prendre quelques
  minutes. Il est normal de voir défiler des pages de sortie du
  compilateur.
- `gsc` est connu pour actuellement échouer à la compilation (un
  vieux paquet AUR non maintenu - ce n'est pas un problème d'OST). Le
  script affiche un avertissement et continue sans lui : `gsc` ne fait
  qu'alimenter le *simulateur* de caméra CCD d'INDI avec un champ
  d'étoiles réaliste, il n'est jamais nécessaire pour du matériel réel.
- Un écran bleu peut apparaître, demandant quels fichiers d'index
  `astrometry.net` télécharger en plus de ceux par défaut. Utilisez
  les **flèches** pour vous déplacer, **Espace** pour cocher/décocher,
  **Tab** puis **Entrée** pour confirmer. Si vous n'êtes pas sûr,
  laissez tout décoché et appuyez sur Entrée — vous pourrez toujours
  les ajouter plus tard.

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
`ostserver`/`ostserver-complete` à la main (depuis AUR, ou le
`.pkg.tar.zst` précompilé sur la page des releases) et configurez le
reste vous-même — les étapes du script font elles-mêmes office de
guide pour ce qu'il faut faire manuellement.
{{% /notice %}}
