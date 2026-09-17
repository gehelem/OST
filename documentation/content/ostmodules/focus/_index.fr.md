---
title: Focuser
weight: 10
description: Mise au point automatique par ajustement polynomial et cartographie du tilt capteur
---

## Rôle

Le module Focuser assure la mise au point automatique du télescope. Il déplace le focaliseur motorisé sur une plage de positions, mesure la taille des étoiles (HFR — Half Flux Radius) à chaque point, puis calcule la position optimale par ajustement polynomial de degré 2. Avec le zoning activé, il produit également une cartographie du tilt du capteur.

## Interface

L'interface affiche en temps réel la courbe HFR en cours de construction, la progression de la séquence, et les résultats (HFR finale et position). Les paramètres sont accessibles via le bouton **Paramètres** en haut à droite.

![Capture d'écran du module Focuser](/images/modules/focus.png)

La courbe HFR est tracée avec des barres d'erreur représentant l'écart-type des mesures à chaque position, et un ajustement polynomial apparaît en trait pointillé orange au fur et à mesure des acquisitions.

![Courbe HFR avec barres d'erreur et ajustement polynomial](/images/modules/focus/focus-chart.png)

## Appareils requis

| Appareil | Rôle |
|---|---|
| Caméra | Acquisition des images pour la mesure HFR |
| Focaliseur motorisé | Déplacement de la mise au point |
| Roue à filtres | Optionnel — utilisé si configuré |

## Paramètres

Ces paramètres sont accessibles depuis le menu **Paramètres** du module. Un jeu de paramètres peut être sauvegardé dans un profil.

### Exposition

| Paramètre | Description |
|---|---|
| **Exposition** | Durée d'exposition en secondes |
| **Gain** | Gain de la caméra |
| **Offset** | Offset de la caméra |

### Focus

| Paramètre | Description |
|---|---|
| **Position de départ** | Position absolue du focaliseur pour débuter la séquence (ignoré si *Autour de la position actuelle* est activé) |
| **Autour de la position actuelle** | Centre automatiquement la séquence autour de la position courante du focaliseur : `pos_actuelle − (pas × itérations / 2)` |
| **Pas** | Écart en unités focaliseur entre deux mesures consécutives |
| **Itérations** | Nombre de points de mesure. Avec 5 itérations et un pas de 2000, le focaliseur explore une plage de 8000 unités |
| **Backlash** | Surshoot appliqué avant chaque déplacement pour corriger le jeu mécanique. Le focaliseur recule de cette valeur avant d'avancer vers la cible |
| **Zoning** | Découpe de l'image en zones (1×1 à 9×9). Voir section dédiée ci-dessous |

## Algorithme

La mise au point se déroule en deux phases.

### Phase 1 — Exploration

1. Le focaliseur se positionne à `position_départ − backlash` (prise en compte du jeu mécanique)
2. Il avance à la `position_départ`
3. Pour chaque itération :
   - Réinitialisation du sous-cadre caméra
   - Acquisition d'une image, calcul de la HFR et de son écart-type
   - Enregistrement du triplet (position, HFR, écart-type)
   - Mise à jour de l'ajustement polynomial en temps réel
   - Avance d'un *pas*

L'ajustement polynomial de degré 2 donne en continu la position du minimum de la courbe HFR :

```
position_optimale = −k1 / (2 × k2)    [pour k0 + k1·x + k2·x²]
```

### Phase 2 — Positionnement final

1. Le focaliseur recule vers `meilleure_position − backlash`
2. Il avance à la position optimale calculée par le polynôme
3. Une image finale est acquise pour mesurer la HFR résultante

{{% notice style="tip" title="Valeur de retour" %}}
La position optimale retenue est celle du **fit polynomial** (`bestposfit`). Si le fit n'a pas pu être calculé (moins de 3 points), c'est la meilleure position mesurée directement (`bestpos`) qui est utilisée.
{{% /notice %}}

## Courbe HFR

La courbe HFR est tracée en temps réel pendant l'exploration et mise à jour à chaque point de mesure. Elle comporte deux éléments superposés :

- **Points mesurés** (vert) avec **barres d'erreur** représentant ±1 écart-type des mesures à chaque position
- **Courbe polynomiale** (pointillé orange) — ajustement de degré 2 mis à jour à chaque nouveau point

L'axe X représente la position absolue du focaliseur, l'axe Y la HFR en pixels.

{{% notice style="note" title="Barres d'erreur" %}}
L'écart-type affiché correspond à la dispersion des HFR individuelles mesurées par étoile à une même position de mise au point. Une barre d'erreur large indique un seeing variable ou une turbulence importante au moment de l'acquisition.
{{% /notice %}}

## Zoning

Le zoning divise l'image en une grille de N×N zones et calcule un ajustement polynomial indépendant pour chaque zone. Il permet à la fois une analyse fine des variations HFR sur le champ et une cartographie du tilt du capteur.

| Valeur | Grille | Utilisation |
|---|---|---|
| 1 | 1×1 | Pas de zoning — une seule mesure globale |
| 2 | 2×2 | Détection basique du tilt (4 quadrants) |
| 3 | 3×3 | Analyse courante — centre + bords + coins |
| 5 | 5×5 | Analyse fine |
| 9 | 9×9 | Analyse très fine — 81 zones |

Pour chaque zone, le module calcule via l'algorithme de Welford (mise à jour incrémentale en une seule passe) :
- La **HFR moyenne** de la zone
- L'**écart-type HFR** de la zone — indicateur de la qualité optique locale

## Cartographie du tilt capteur

Lorsque le zoning est activé (≥ 2×2), le module génère automatiquement quatre images de cartographie à l'issue de la séquence. Chacune représente la position de mise au point optimale de chaque zone, codée en couleur (bleu = position basse / plan rapproché, rouge = position haute / plan éloigné).

![Exemples de cartographies du tilt capteur (4 images)](/images/modules/focus/focus-tilt-maps.png)

Le ratio largeur/hauteur des images correspond à celui du capteur.

### rawTilt — Tilt brut par zone

Chaque zone est représentée par un bloc de couleur uniforme proportionnel à la position de meilleure mise au point. Pas d'interpolation — la résolution spatiale est celle de la grille de zoning.

![rawTilt — blocs de couleur par zone](/images/modules/focus/focus-rawtilt.png)

### localTiltLinear — Interpolation bilinéaire locale

Interpolation bilinéaire entre les centres des zones adjacentes. Produit une carte lisse sans hypothèse sur la forme globale du plan focal.

### globalTiltLinear — Plan global (ajustement linéaire GSL)

Ajustement d'un plan par moindres carrés (bibliothèque GSL) sur l'ensemble des positions de zone. Idéal pour quantifier un tilt pur : inclinaison du capteur par rapport à l'axe optique. Le résidu affiché dans chaque cellule (en jaune) est l'écart entre la mesure et le plan ajusté.

### globalTiltQuadratic — Surface quadratique (ajustement GSL degré 2)

Ajustement d'une surface du second degré. Capture les défauts non plans : courbure du champ, flexion du capteur. La gamme de couleurs couvre le minimum et le maximum réels de la surface (vertex inclus si il se trouve dans les limites du capteur).

{{% notice style="note" title="Interprétation" %}}
- **rawTilt / localTiltLinear** : diagnostics locaux, utiles pour repérer une zone aberrante due à une mauvaise étoile.
- **globalTiltLinear** : mesure directe du tilt. Si le gradient est uniforme, incliner le capteur dans la direction indiquée suffira à corriger.
- **globalTiltQuadratic** : si le résidu du plan global est important et que la surface quadratique est nettement meilleure, la courbure de champ ou une flexion mécanique sont en cause.
{{% /notice %}}

## Valeurs affichées en temps réel

| Valeur | Description |
|---|---|
| **Statut** | État courant de la séquence (Idle / Running / Ok / Error) |
| **Progression** | Barre de progression globale de la séquence |
| **HFR** | HFR mesurée à la position finale |
| **Position** | Position finale du focaliseur |

## Résultat final

À l'issue de la mise au point, le module publie :

| Résultat | Description |
|---|---|
| **Position finale** | Position absolue du focaliseur après positionnement |
| **HFR finale** | HFR mesurée à la position optimale |

Ces valeurs sont également publiées vers les autres modules (séquenceur) via le système d'événements inter-modules.

## Actions

| Action | Description |
|---|---|
| **Lancer la mise au point** | Démarre la séquence complète |
| **Abandonner** | Interrompt immédiatement la séquence en cours |
