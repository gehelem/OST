---
title: Polar
weight: 60
description: Assistant de mise en station polaire par plate solving
---

## Rôle

Le module Polar assiste la mise en station polaire de la monture. Il acquiert trois images du ciel en déplaçant la monture en RA entre chaque prise, résout astrométriquement chaque image, puis calcule l'erreur d'azimut et d'altitude de l'axe polaire. Une boucle de correction continue permet ensuite de guider l'utilisateur pendant qu'il ajuste mécaniquement la monture.

![Capture d'écran du module Polar](/images/modules/polar.png)

## Appareils requis

| Appareil | Rôle |
|---|---|
| Caméra | Acquisition des images pour le plate solving |
| Monture | Déplacement en RA et lecture des coordonnées |
| GPS | Position géographique de l'observateur |

## Paramètres

| Paramètre | Description |
|---|---|
| **Exposition** | Durée d'exposition en secondes |
| **Gain** | Gain de la caméra |
| **Offset** | Offset de la caméra |
| **Focale** | Focale optique en mm — utilisée pour définir l'échelle image du solveur |

## Algorithme

### Phase 1 — Calibration (3 images)

Le module acquiert trois images en effectuant un déplacement de 30 minutes d'angle (0,5 h) en RA entre chaque prise :

1. **Image 0** — position initiale
2. Déplacement de ±0,5 h en RA (sens adapté au côté de pier)
3. **Image 1** — nouvelle position
4. Nouveau déplacement de ±0,5 h en RA
5. **Image 2** — position finale

Chaque image est plate-solvée. Les trois solutions (RA, DEC en J2000) sont converties en coordonnées horizontales (azimut/altitude) à l'instant exact de la prise de vue, en tenant compte de la position géographique de l'observateur.

### Calcul de l'axe polaire

Les trois points Az/Alt décrivent un arc sur la sphère céleste correspondant à la rotation de la monture. Le module calcule le vecteur normal à cet arc — c'est la direction de l'axe de rotation réel de la monture.

Les erreurs sont alors déduites :

| Erreur | Définition |
|---|---|
| **Erreur azimut** | Écart entre l'azimut de l'axe et le Nord (azimut = 0°) |
| **Erreur altitude** | Différence entre l'altitude de l'axe et la latitude de l'observateur |
| **Erreur totale** | Erreur combinée : `√(az² + alt²)` |

Les erreurs sont affichées en degrés et en minutes d'arc.

### Phase 2 — Boucle de correction

Après la calibration, le module entre dans une boucle continue sans déplacement de la monture :

1. Acquisition d'une image
2. Plate solving
3. Calcul du déplacement Az/Alt par rapport à l'image de référence (image 2)
4. Mise à jour de l'erreur résiduelle = erreur initiale − correction déjà appliquée

L'utilisateur ajuste mécaniquement l'azimut et l'altitude de la monture ; l'erreur résiduelle diminue en temps réel à mesure que la mise en station s'améliore.

### Superposition graphique

L'image affichée comporte une superposition représentant les corrections à appliquer :

| Trait | Couleur | Signification |
|---|---|---|
| Axe polaire → correction altitude | Jaune | Erreur d'altitude |
| Correction altitude → pôle | Vert | Erreur d'azimut |
| Axe polaire → pôle | Rouge | Erreur totale |

Le repère est orienté de façon à ce que le Nord céleste soit en haut de l'image.

## États

| État | Description |
|---|---|
| **Idle** | Module inactif |
| **Moving** | Monture en déplacement |
| **Shooting** | Acquisition d'image en cours |
| **Solving** | Plate solving en cours |
| **Compute** | Calcul de l'erreur polaire |

## Valeurs affichées

### Mesures de calibration

| Valeur | Description |
|---|---|
| **RA 0 / DE 0 / Time 0** | Solution astrométrique et horodatage de l'image 0 |
| **RA 1 / DE 1 / Time 1** | Solution astrométrique et horodatage de l'image 1 |
| **RA 2 / DE 2 / Time 2** | Solution astrométrique et horodatage de l'image 2 |

### Erreur polaire

| Valeur | Description |
|---|---|
| **Erreur azimut (°)** | Erreur d'azimut en degrés |
| **Erreur altitude (°)** | Erreur d'altitude en degrés |
| **Erreur totale (°)** | Erreur combinée en degrés |
| **Erreur azimut (arcmin)** | Erreur d'azimut en minutes d'arc |
| **Erreur altitude (arcmin)** | Erreur d'altitude en minutes d'arc |
| **Erreur totale (arcmin)** | Erreur combinée en minutes d'arc |

## Actions

| Action | Description |
|---|---|
| **Démarrer** | Lance la séquence de calibration (3 images + calcul) puis la boucle de correction |
| **Abandonner** | Interrompt immédiatement la séquence en cours |
