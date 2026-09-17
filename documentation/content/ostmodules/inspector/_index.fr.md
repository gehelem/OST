---
title: Inspector
weight: 70
description: Analyse de la qualité optique (HFR, forme des étoiles) et assistant de collimation
---

## Rôle

Le module Inspector effectue deux analyses indépendantes sur un champ d'étoiles. Chacune s'active séparément — aucune, l'une, l'autre, ou les deux à la fois :

- **Analyse Inspector** — qualité optique sur l'ensemble de l'image, sur une image *mise au point*. Elle produit trois cartes complémentaires (HFR par zone, élongation des étoiles, mosaïque des coins) qui permettent de diagnostiquer les défauts optiques tels que le tilt, le coma ou la courbure de champ.
- **Analyse Collimator** — collimation optique (alignement des miroirs) des télescopes Newton, RC (Ritchey-Chrétien) et SC (Schmidt-Cassegrain), sur une image *défocalisée*. Elle analyse l'ensemble du champ d'étoiles et affiche en continu l'écart de collimation et les corrections par vis. En semi-manuel : le logiciel défocalise, capture et analyse en boucle pendant que l'utilisateur agit physiquement sur les vis de collimation (non motorisées) en observant le retour visuel se mettre à jour en temps quasi réel.

L'analyse peut être effectuée sur une image acquise en direct depuis la caméra, ou sur un fichier FITS chargé depuis le disque.

![Capture d'écran du module Inspector](/images/modules/inspector.png)

## Appareils requis

| Appareil | Rôle |
|---|---|
| Caméra | Acquisition des images (optionnel si chargement depuis fichier) |
| Focuser (facultatif) | Pilotage automatique de la défocalisation pour le flux de collimation, et fonction de position « home ». Si absent, défocalisation manuelle avant chaque capture |

## Sélection des analyses

| Bascule | Effet |
|---|---|
| **Analyse Inspector** | Lance l'analyse HFR / forme / coins sur chaque image capturée (ou rechargée) |
| **Analyse Collimator** | Lance l'analyse de collimation sur chaque image capturée (ou rechargée) |

Les deux bascules sont indépendantes (non exclusives) et sont persistées avec le profil du module. Lorsqu'une analyse est désactivée, ses images de résultat sont effacées afin de ne jamais afficher une carte périmée à côté d'une carte fraîche issue d'une autre image.

## Paramètres

### Acquisition

| Paramètre | Description |
|---|---|
| **Exposition** | Durée d'exposition en secondes |
| **Gain** | Gain de la caméra |
| **Offset** | Offset de la caméra |
| **Focale** | Focale optique en mm — utilisée pour convertir les HFR et le vecteur de collimation en secondes d'arc |
| **Taille des coins (pixels)** | Largeur en pixels des patches extraits pour la mosaïque de coins |

### Analyse

| Paramètre | Description |
|---|---|
| **Zoning** | Grille N×N de découpe de l'image pour l'analyse par zone (de 1×1 à 64×64) |

### Défocalisation

| Paramètre | Description |
|---|---|
| **Focuser offset (steps)** | Décalage du focuser (en pas) appliqué par *Go intra* / *Go extra* pour atteindre la position défocalisée |

## Cartes produites (analyse Inspector)

### Carte HFR

La carte HFR divise l'image en une grille N×N (selon le paramètre *Zoning*). Pour chaque zone, la HFR moyenne est calculée sur toutes les étoiles détectées, puis lissée par une moyenne des 8 cellules voisines.

La couleur de chaque zone va du vert (meilleure HFR) au rouge (moins bonne HFR).

En superposition :
- Cercles bleus autour de chaque étoile détectée, de rayon proportionnel à la HFR individuelle
- Rectangle blanc reliant les centres des quatre quadrants, dont la forme reflète la distribution du HFR : un carré parfait indique une uniformité idéale
- Valeur HFR de chaque quadrant (haut-gauche, haut-droite, bas-gauche, bas-droite) et HFR globale, exprimées en secondes d'arc

### Carte de forme (aberrations)

La carte de forme analyse l'élongation des étoiles par zone. Pour chaque zone de la grille, un trait est dessiné :

- **Orientation** : direction de l'axe principal des étoiles (angle moyen)
- **Longueur** : proportionnelle à l'élongation `a/b − 1` (rapport grand axe / petit axe)

Un trait court et uniforme sur toute l'image indique des étoiles rondes. Des traits longs en périphérie signalent du coma ou de l'astigmatisme.

### Mosaïque de coins

La mosaïque assemble neuf patches de taille *Taille des coins* extraits de neuf positions de l'image originale, disposés en grille 3×3 :

```
Haut-gauche   |  Haut-centre   |  Haut-droite
Milieu-gauche |  Centre        |  Milieu-droite
Bas-gauche    |  Bas-centre    |  Bas-droite
```

Cette vue permet de comparer directement la qualité des étoiles au centre et aux bords du champ.

## Analyse de collimation

Une étoile défocalisée apparaît comme un anneau ("donut") dont la forme et le centrage renseignent sur la collimation. Contrairement à une approche classique comparant une étoile unique en intra-focal et en extra-focal, l'analyse de collimation exploite **toutes les étoiles d'un seul champ, en une seule capture** :

1. Défocalisation (automatique via le focuser, ou manuelle)
2. Capture d'une image contenant plusieurs étoiles réparties dans le champ
3. Détection de chaque donut exploitable et calcul d'un vecteur de déformation par étoile
4. Régression sur l'ensemble des vecteurs pour déterminer le vecteur de collimation global
5. Conversion en correction à appliquer sur chacune des 3 vis
6. Nouvelle capture, nouvelle analyse, boucle continue jusqu'à convergence

### Pourquoi le plein champ suffit

Pour une étoile à la position `P` dans l'image (repère centré sur l'image), le vecteur de déformation observé suit un modèle linéaire :

```
D(P) = C - k·P
```

où `C` est le vecteur de collimation recherché (constant, indépendant de la position dans le champ) et `k` regroupe l'effet du coma de champ (aberration optique normale, présente même bien collimaté, qui croît avec la distance au centre et avec l'ouverture du télescope). Un tel champ vectoriel a toujours un point de convergence unique — avec quelques étoiles à des positions de champ différentes, une régression permet de séparer la part collimation de la part coma, sans avoir besoin de comparer intra et extra-focal.

Les boutons **Go intra**/**Go extra** restent disponibles pour une vérification manuelle par l'utilisateur, mais ce n'est pas une étape du flux automatisé.

### Détection par étoile

Pour chaque étoile détectée :
- Seuillage adaptatif local puis affiné individuellement (un seuil global unique désavantagerait les étoiles les plus faibles du champ)
- Extraction du contour externe de l'anneau et de l'ombre du secondaire (si visible)
- Un ajustement de cercle par moindres carrés sur le contour externe donne le centre théorique de l'étoile — cette méthode reste fiable même quand l'anneau est fortement déformé (croissant, en cas de décollimation importante)
- Le vecteur de déformation est l'écart entre ce centre théorique et le centroïde pondéré par intensité de l'anneau (ombre exclue)

### Régression et point de convergence

Les vecteurs de déformation de toutes les étoiles détectées sont ajustés par moindres carrés au modèle `D(P) = C - k·P`. Le vecteur `C` obtenu est converti en arcsecondes (focale et taille de pixel du capteur), puis projeté sur les 3 vis de collimation à 120° pour obtenir un signe et une amplitude par vis.

{{% notice style="note" title="Pas de calibration en tours de vis" %}}
L'amplitude par vis est exprimée en arcsecondes, pas en tours ou fractions de tour. La relation entre un écart optique et un mouvement physique de vis dépend du montage mécanique du secondaire (pas commun, sensibilité), propre à chaque instrument — plutôt que d'inventer une calibration approximative, le réglage se fait au ressenti : l'amplitude et le sens indiqués donnent la tendance, l'utilisateur ajuste et observe la mise à jour en direct.
{{% /notice %}}

### Carte de collimation

L'analyse de collimation publie sa propre image annotée (l'image principale reste une image brute). Elle comporte, pour chaque étoile détectée :
- Le contour externe du donut
- Une flèche de déformation (exagérée pour rester visible), colorée selon l'amplitude (vert / orange / rouge)

Et pour l'ensemble du champ :
- Un bullseye (anneaux pointillés) centré sur l'image, matérialisant les mêmes seuils de tolérance que le voyant qualité
- Une ligne reliant le centre théorique de l'image au point de convergence calculé

#### Avant collimation (exemple)

![Overlay de collimation, collimation à corriger](/images/modules/collimator/collimator-ko.png)

#### Après collimation (exemple)

![Overlay de collimation, collimation correcte](/images/modules/collimator/collimator-ok.png)

### Valeurs affichées (collimation)

#### Vecteur de collimation

| Valeur | Description |
|---|---|
| **C.x / C.y (px)** | Vecteur de collimation, en pixels image |
| **Amplitude (arcsec)** | Amplitude du vecteur de collimation, convertie en arcsecondes |
| **Convergence point x/y (px)** | Point de convergence calculé (où les vecteurs de déformation, prolongés, se croisent) |
| **Quality** | Voyant : vert si l'amplitude est faible, orange en zone intermédiaire, rouge au-delà du seuil haut |

#### Corrections par vis

| Valeur | Description |
|---|---|
| **Screw 1 / 2 / 3 (turns)** | Amplitude et sens de correction par vis, exprimée en arcsecondes (positif = serrer) — convention fixe à 120°, pas de détection automatique de l'orientation réelle des vis |

#### Étoiles détectées

Grille listant chaque étoile exploitable de la dernière analyse : position (x, y) et vecteur de déformation (dx, dy) en pixels.

## États

| État | Description |
|---|---|
| **Idle** | Module inactif |
| **Shooting** | Acquisition d'image en cours |
| **Analyzing** | Analyse en cours |

## Position « home » du focuser

*Set home* mémorise la position absolue actuelle du focuser ; *Go home* y ramène ensuite le focuser. La position n'est **pas** lue au démarrage et **pas** persistée — rien ne garantit que la mise au point soit au moins dégrossie à ce moment-là, elle doit donc être définie explicitement. La propriété **Focuser home** indique si une position home est définie et sa valeur ; tant que *Set home* n'a pas été utilisé, *Go home* se contente d'émettre un avertissement et ne fait rien.

## Chargement depuis fichier

Le champ **Sélectionner un fichier** permet de charger un fichier FITS depuis le disque pour l'analyser sans déclencher d'acquisition. L'analyse est alors lancée automatiquement à la saisie du chemin, selon la *sélection des analyses* courante.

## Actions

| Action | Description |
|---|---|
| **Shoot** | Acquiert une seule image et lance les analyses activées |
| **Loop** | Acquisition et analyse en continu — se répète automatiquement jusqu'à l'arrêt |
| **Stop** | Arrête la boucle continue |
| **Reload** | Recharge et réanalyse la dernière image ou le dernier fichier |
| **Set home** | Mémorise la position actuelle du focuser comme position de retour |
| **Go home** | Ramène le focuser à la position home mémorisée (avertit si aucune n'a été définie) |
| **Go intra** | Déplace le focuser vers la position intra-focale (vérification manuelle) |
| **Go extra** | Déplace le focuser vers la position extra-focale (vérification manuelle) |

Les boutons **Go intra** / **Go extra** / **Go home** restent enfoncés pendant le déplacement du focuser, et sont relâchés lorsqu'il atteint la cible.
