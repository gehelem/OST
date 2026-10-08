---
title: Installation rapide (Ubuntu Resolute)
weight: 5
description: "La façon la plus rapide de faire fonctionner OST : une machine Ubuntu 26.04 vierge, un script"
---

{{% notice style="tip" title="Idéal pour les débutants" %}}
Vous n'avez jamais utilisé de ligne de commande Linux ? Cette page est
faite pour vous. Suivez simplement les trois étapes ci-dessous, dans
l'ordre.
{{% /notice %}}

Ce script unique installe tout ce qui est nécessaire sur une seule
machine : le back-end OST, l'interface web, nginx, INDI,
`astrometry.net` et la documentation que vous êtes en train de lire.

{{% notice style="note" title="Vous utilisez Arch Linux plutôt ?" %}}
Cette page concerne spécifiquement Ubuntu. Sous Arch Linux, suivez
plutôt l'[Installation rapide (Arch Linux)](../quick-arch/).
{{% /notice %}}

---

## 1. Installer une Ubuntu 26.04 vierge

Installez **Ubuntu Server 26.04 LTS (« Resolute »)** sur la machine
qui va faire tourner OST — un Raspberry Pi, un mini-PC ou une machine
virtuelle. Utilisez l'installeur Ubuntu standard ; aucune option
particulière n'est nécessaire.

{{% notice style="note" title="Pourquoi précisément la 26.04 ?" %}}
Ce script télécharge le paquet compilé pour Ubuntu 26.04. Les autres
versions prises en charge (22.04, 24.04) nécessitent une procédure
différente, plus manuelle — voir plutôt la
[Procédure d'installation complète](../fresh/).
{{% /notice %}}

Une fois Ubuntu installé, ouvrez un terminal (directement sur la
machine, ou en SSH depuis un autre ordinateur).

---

## 2. Télécharger le script d'installation

Copiez la commande ci-dessous, collez-la dans le terminal, puis
appuyez sur Entrée :

```bash
wget https://gitlab.ostserver.fr/ost/ostserver/-/raw/main/install-latest-complete.sh
```

Rien ne s'affiche à l'écran si tout fonctionne — c'est normal. Un
nouveau fichier, `install-latest-complete.sh`, existe maintenant dans
votre dossier courant.

---

## 3. Exécuter le script

```bash
chmod +x install-latest-complete.sh
./install-latest-complete.sh
```

Le script fait alors tout lui-même. Voici ce à quoi vous attendre :

- Il va demander **votre mot de passe** plusieurs fois (`sudo`) —
  c'est normal, il a besoin de la permission d'installer des paquets
  système.
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
de dossiers standard, nginx comme serveur web, Ubuntu 26.04
uniquement). Si vous avez besoin d'autre chose, suivez plutôt la
[Procédure d'installation complète](../fresh/) — même résultat, mais
chaque étape est expliquée et ajustable.
{{% /notice %}}
