---
title: Navigator
weight: 40
description: Recherche de cibles et mise en station précise par plate solving
---

## Rôle

Le module Navigator permet de rechercher un objet dans les catalogues astronomiques, de pointer la monture vers cet objet, puis de centrer précisément le champ en boucle via le plate solving (résolution astrométrique). Il peut également envoyer la cible sélectionnée au module Planner.

![Capture d'écran du module Navigator](/images/modules/navigator.png)

## Appareils requis

| Appareil | Rôle |
|---|---|
| Caméra | Acquisition des images pour le plate solving |
| Monture | Réception des ordres de déplacement |
| GPS | Fourniture des coordonnées géographiques et de l'heure |

## Paramètres

### Acquisition

| Paramètre | Description |
|---|---|
| **Exposition** | Durée d'exposition en secondes |
| **Gain** | Gain de la caméra |
| **Offset** | Offset de la caméra |
| **Focale** | Focale optique en mm — utilisée pour calculer l'échelle image du solveur |

### Centrage

| Paramètre | Description |
|---|---|
| **Itérations max** | Nombre maximal de cycles slew → solve → correct avant d'abandonner |
| **Tolérance (arcsec)** | Distance angulaire acceptable entre le centre résolu et la cible |
| **Synchroniser la monture** | Si activé, synchronise les coordonnées de la monture sur la solution après un centrage réussi |

### Modules esclaves

| Paramètre | Description |
|---|---|
| **Instance du module Planner** | Nom de l'instance du module Planner à utiliser pour l'action *Ajouter au planning* |

## Fonctionnement

### Recherche de cibles

La recherche s'effectue par saisie d'un nom ou code dans le champ **Rechercher un objet**. Les catalogues disponibles sont :

| Catalogue | Contenu |
|---|---|
| Messier | Objets du catalogue Messier |
| NGC | New General Catalogue |
| LDN | Lynds' Catalogue of Dark Nebulae |
| SH2 | Sharpless catalogue (nébuleuses H II) |
| IC | Index Catalogue |
| Étoiles | Étoiles nommées |

Les résultats s'affichent dans la grille **Résultats** avec : catalogue, code, RA, DEC, magnitude, diamètre, nom et alias. Un clic sur une ligne sélectionne la cible.

### Conversion J2000 → époque actuelle

Dès qu'une cible est sélectionnée, ses coordonnées J2000 sont converties en coordonnées de l'époque actuelle (précession prise en compte) et affichées dans **Sélection convertie**.

### Boucle de centrage

L'action **Goto cible** déclenche la séquence suivante :

1. La monture pointe vers les coordonnées converties à l'époque actuelle
2. La caméra acquiert une image qui est résolue par le solveur astrométrique (StellarSolver / indices astrometry.net)
3. La distance angulaire entre le centre résolu et la cible est calculée
4. Si la distance est inférieure à la **Tolérance**, le centrage est déclaré réussi
5. Sinon, un offset correctif est calculé et appliqué à la monture ; le cycle recommence

Ce processus se répète jusqu'à succès ou jusqu'à atteindre le nombre maximal d'itérations.

{{% notice style="tip" title="Synchronisation monture" %}}
Si *Synchroniser la monture* est activé, le module effectue un SYNC INDI après le centrage réussi, ce qui améliore la précision des pointages suivants.
{{% /notice %}}

## Valeurs affichées

### Position de la monture

| Valeur | Description |
|---|---|
| **RA** | Ascension droite courante de la monture |
| **DEC** | Déclinaison courante de la monture |

### Position GPS

| Valeur | Description |
|---|---|
| **Altitude** | Altitude du site d'observation (m) |
| **Latitude** | Latitude du site d'observation (°) |
| **Longitude** | Longitude du site d'observation (°) |

### Date et heure GPS

| Valeur | Description |
|---|---|
| **Décalage UTC** | Fuseau horaire |
| **Date** | Date courante |
| **Heure** | Heure courante |

## Actions

| Action | Description |
|---|---|
| **Goto cible** | Pointe la monture vers la cible sélectionnée et lance la boucle de centrage |
| **Abandonner** | Interrompt immédiatement le goto ou le centrage en cours |
| **Ajouter au planning** | Envoie la cible courante au module Planner |
