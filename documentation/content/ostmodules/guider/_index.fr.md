---
title: Guider
weight: 20
description: Guidage automatique par correction de dérive
---

## Rôle

Le module Guider assure le guidage automatique du télescope. Il détecte et suit un champ d'étoiles de référence, mesure la dérive en temps réel et envoie des impulsions de correction à la monture pour maintenir le pointage.

![Capture d'écran du module Guider](/images/modules/guider.png)

## Équipements requis

| Équipement | Rôle |
|---|---|
| Caméra de guidage | Acquisition des images du champ de guidage |
| Interface de guidage | Reçoit les impulsions temporisées (`TELESCOPE_TIMED_GUIDE_*`) — la monture elle-même, ou un boîtier relayant des impulsions ST4 |

## Phases de fonctionnement

Le guidage se déroule en trois phases successives.

### Phase 1 — Initialisation

Le module acquiert une image de référence et conserve la liste d'étoiles détectées comme référence pour toute la session de guidage. Lorsque le guidage est lancé avec une calibration enregistrée valide, seule cette phase précède le guidage : une nouvelle référence est prise au pointage courant, ce qui permet de réutiliser la calibration après un déplacement de la monture.

### Phase 2 — Calibration

Le module mesure la réponse de la monture aux impulsions de correction.

1. Une série fixe d'impulsions est envoyée à l'Ouest, puis à l'Est, puis au Nord, puis au Sud (`Pas de calibration` impulsions par direction).
2. Le déplacement image par impulsion de chaque passe est mesuré par rapport à l'image précédente.
3. **Le taux d'impulsion est tiré des combinaisons antisymétriques `(Ouest − Est) / 2` et `(Nord − Sud) / 2`.** Une passe unidirectionnelle est contaminée par la dérive de fond constante (erreur de suivi sidéral, erreur périodique, dérive de mise en station) ; la différence des deux passes opposées annule cette dérive et fournit un taux RA propre et un taux DEC propre. Les taux bruts par direction restent affichés, à titre de diagnostic.
4. L'orientation du capteur est déduite des deux axes (une estimation par l'axe RA et une par l'axe DEC, moyennées), l'ambiguïté à 180° étant levée par rapport à l'axe RA.
5. Le taux RA est ramené à l'équateur (`taux × cos(DEC de calibration)`) pour pouvoir être transposé à n'importe quelle déclinaison au moment du guidage.

Avant la série Nord et avant la série Sud, une impulsion optionnelle de *rattrapage de jeu DEC* prend d'abord le jeu mécanique de l'axe DEC, pour que les impulsions de mesure temporisées ne soient pas en partie perdues à réengrener. Son propre déplacement est exclu du calcul du taux.

Le résultat de calibration est enregistré et réutilisé d'une session à l'autre, tant que le côté du pied et l'orientation de la caméra n'ont pas changé.

{{% notice style="note" title="Qualité de la calibration" %}}
Après les quatre passes, le module calcule un *ratio de dérive* — l'ampleur de la dérive de fond constante rapportée au déplacement dû aux impulsions — et un voyant **Qualité de calibration** (vert / orange / rouge). Si le ratio dépasse *Ratio de dérive de fond max*, la calibration est rejetée et relancée, jusqu'à *Nombre de tentatives de calibration* fois, puis abandonnée avec un message invitant à vérifier la mise en station. Une calibration est également abandonnée si l'étoile s'écarte de plus de *Excursion d'étoile max* de sa position de départ, avant même de pouvoir quitter le capteur.
{{% /notice %}}

### Phase 3 — Guidage

Boucle de correction continue :

1. Acquérir une image.
2. Suivre le champ d'étoiles par rapport à la référence (voir *Suivi d'étoiles* ci-dessous).
3. Calculer la dérive, projetée sur les axes RA et DEC avec l'orientation de calibration.
4. **Régulation PI** — la correction vaut `agressivité × dérive + gain intégral × dérive accumulée`. Le terme proportionnel seul ne peut pas annuler une dérive constante (il se stabilise à `dérive / agressivité`) ; le terme intégral continue de croître jusqu'à ce que le résidu soit réellement nul. L'accumulateur est borné (*Borne de l'intégrale*) et gelé pour un axe dont l'impulsion est déjà au maximum.
5. Le taux RA est adapté à la déclinaison courante : `taux équatorial / cos(DEC courante)`.
6. Si la *Compensation de jeu DEC* est active et que la direction DEC s'est inversée depuis la dernière correction, l'estimation de compensation courante est ajoutée par-dessus.
7. Les impulsions de correction sont envoyées à la monture, bornées à *Impulsion min* / *Impulsion max*.
8. Les statistiques sont mises à jour (RMS RA, DEC, total).
9. Si une impulsion de compensation d'inversion a été envoyée au cycle précédent, la réduction de dérive réellement obtenue est comparée à la prédiction et l'estimation de compensation est ajustée à la hausse (sous-correction) ou à la baisse (sur-correction), dans les bornes configurées.

La **première image** d'une session est mesurée mais non corrigée — elle ne fait que fixer le zéro de travail, pour que quelques pixels de stabilisation après déplacement ne déclenchent pas une grosse première impulsion.

Une perte passagère de corrélation d'étoiles fait simplement **sauter** l'image (aucune impulsion) ; le guidage n'est abandonné qu'après *Images perdues consécutives max* d'affilée. Un **chien de garde matériel** abandonne la session si une exposition, une impulsion, une réinitialisation de trame ou une extraction d'étoiles ne se termine jamais dans le délai *Délai matériel*.

{{% notice style="note" title="Compensation de jeu DEC" %}}
Les engrenages de la monture ont un jeu mécanique : à chaque inversion de direction du guidage DEC, les premières impulsions prennent en partie ce jeu au lieu de produire un mouvement réel, d'où un pic de dérive temporaire. Le jeu réel varie avec la position de pointage et la charge, si bien qu'une valeur fixe sous-corrige ou sur-corrige. L'estimation adaptative s'auto-règle : chaque inversion est à la fois une correction et une mesure, elle converge donc vers le jeu réellement présent.
{{% /notice %}}

## Suivi d'étoiles

Entre deux images, le champ d'étoiles subit une translation quasi pure (la rotation de 180° d'un retournement au méridien est traitée à part par les indicateurs d'inversion). Le suivi cherche directement la **translation dominante** :

1. Chaque couple (étoile de référence, étoile courante) implique une translation candidate ; la vraie est celle sur laquelle le plus d'étoiles s'accordent (étape de vote / RANSAC).
2. Ses points cohérents sont affinés par une fenêtre à deux étages et un écrêtage sigma itératif, et la dérive est une moyenne des résidus pondérée par le flux — continue et sous-pixellique.

Cette méthode est insensible à un changement du classement des étoiles par flux et à l'apparition ou la disparition d'étoiles d'une image à l'autre. Les réglages associés sont sous *Appariement d'étoiles* dans les paramètres.

{{% notice style="note" title="Transposition en déclinaison du taux RA" %}}
Une impulsion déplace l'étoile de `taux × impulsion × cos(DEC)` sur le capteur, donc les ms/px varient en `1 / cos(DEC)`. Le taux RA est donc enregistré ramené à l'équateur au moment de la calibration et divisé par `cos(DEC courante)` au guidage, pour qu'une calibration faite à une déclinaison reste correcte à une autre.
{{% /notice %}}

## Paramètres

Les paramètres sont accessibles depuis le menu **Paramètres** du module et un jeu peut être enregistré comme profil.

### Équipements

| Paramètre | Description |
|---|---|
| Exposition | Durée d'exposition en secondes |
| Gain | Gain de la caméra |
| Offset | Offset de la caméra |

### Calibration

| Paramètre | Description |
|---|---|
| **Durée d'impulsion** | Durée en ms de chaque impulsion de calibration |
| **Pas de calibration** | Nombre d'impulsions par direction |
| **Rattrapage de jeu DEC** | Impulsion supplémentaire (ms) avant les séries Nord et Sud, pour prendre d'abord le jeu mécanique — 0 pour désactiver |
| **Ratio de dérive de fond max** | Rejeter la calibration si la dérive de fond constante dépasse cette fraction du déplacement dû aux impulsions — une valeur élevée traduit en général une mise en station médiocre. 0 pour désactiver le contrôle |
| **Nombre de tentatives de calibration** | Combien de fois relancer une calibration rejetée avant d'abandonner |
| **Excursion d'étoile max** | Abandonner la calibration si l'étoile s'écarte de plus de cette fraction de la plus petite dimension de l'image par rapport à son départ. 0 pour désactiver |

### Guidage

| Paramètre | Description |
|---|---|
| **Impulsion max** | Durée maximale d'une impulsion de correction (ms) |
| **Impulsion min** | Durée minimale d'une impulsion de correction (ms) |
| **Agressivité RA** | Fraction proportionnelle de la correction RA calculée qui est appliquée |
| **Agressivité DEC** | Fraction proportionnelle de la correction DEC calculée qui est appliquée |
| **Gain intégral RA** | Gain sur le résidu RA accumulé — annule l'écart permanent face à une dérive constante. 0 = purement proportionnel |
| **Gain intégral DEC** | Idem, pour l'axe DEC |
| **Borne de l'intégrale (px)** | Anti-emballement : amplitude à laquelle l'accumulateur intégral est borné. 0 pour désactiver la borne |
| **Seuil RMS** | Multiplicateur du RMS courant au-delà duquel une correction est écartée |
| **RMS calculé sur x** | Nombre d'images sur lequel le RMS est calculé |
| **Inversion selon côté du pied** | Inverser automatiquement les corrections RA/DEC quand le côté du pied diffère de celui de la calibration |
| **Amplitude de dither (px)** | Décalage aléatoire maximal appliqué lors d'un dither |
| **Images perdues consécutives max** | Sauter une image de guidage en cas de perte de corrélation ; abandonner seulement après ce nombre d'affilée |
| **Délai matériel (s)** | Abandonner si la caméra ou la monture ne répond plus pendant cette durée. 0 pour désactiver le chien de garde |

### Appariement d'étoiles

| Paramètre | Description |
|---|---|
| **Fenêtre d'appariement (px)** | Rayon de recherche lors de l'appariement d'une étoile de référence dans l'image courante |
| **Étoiles appariées min** | Nombre minimal d'étoiles de référence à retrouver pour que la dérive soit jugée fiable |
| **Étoiles utilisées max** | Plafond du nombre d'étoiles les plus brillantes retenues dans chaque image |

### Compensation de jeu DEC

| Paramètre | Description |
|---|---|
| **Activer** | Ajouter une impulsion supplémentaire adaptative à chaque inversion de direction du guidage DEC |
| **Compensation courante (ms)** | Estimation adaptative, ajustée après chaque inversion — fixer une valeur initiale ou laisser à 0 pour qu'elle s'apprenne |
| **Compensation minimale / maximale (ms)** | Bornes de l'estimation adaptative |
| **Pas d'ajustement (ms)** | De combien l'estimation est retouchée après chaque inversion, selon la sur/sous-correction mesurée |

### Inversions manuelles

| Paramètre | Description |
|---|---|
| **Inverser RA** | Inverser manuellement la direction de correction RA |
| **Inverser DEC** | Inverser manuellement la direction de correction DEC |

### Désactivation de corrections

| Paramètre | Description |
|---|---|
| **Désactiver RA+ / RA− / DEC+ / DEC−** | Désactiver les impulsions de correction dans cette direction |

### Aides au test

| Paramètre | Description |
|---|---|
| **SIM : faux retournement au méridien** | Ajoute 180° à l'angle de calibration au démarrage du guidage (l'effet qu'a un vrai retournement sur la correspondance dérive → RA/DEC), pour éprouver la gestion des inversions de correction avec le simulateur CCD, qui ne fait jamais tourner son propre champ. À laisser désactivé en usage réel |

## Valeurs de calibration

| Valeur | Description |
|---|---|
| **Qualité de calibration** | Voyant : vert (propre) / orange (exploitable, dérive de fond notable) / rouge (rejetée) |
| **Taux RA (ms/px)** | Taux RA équatorial utilisé pour le guidage (Ouest/Est combinés, dérive retirée) |
| **Taux DEC (ms/px)** | Taux DEC utilisé pour le guidage (Nord/Sud combinés) |
| **brut N / S / E / W (ms/px)** | Taux bruts par direction, diagnostic uniquement (portent encore la dérive de fond) |
| **Orientation CCD** | Angle de calibration (degrés) |
| **DEC monture à la calibration** | Déclinaison de la monture au moment de la calibration |
| **Inverser RA / DEC** | Inversions de correction enregistrées lors de la calibration |

## Valeurs en direct

| Valeur | Description |
|---|---|
| **Impulsion N / S / E / W** | Durée de la dernière impulsion dans chaque direction (ms) |
| **RMS RA / DEC / total** | Écart quadratique moyen de la dérive (pixels) |

## Graphiques

### Dérive (nuage de points)

Nuage XY de la dérive de chaque correction par rapport à la position de référence, avec cercles de tolérance.

### Guidage (série temporelle)

Graphique de style PHD2 montrant, par correction :

| Courbe | Description |
|---|---|
| Dérive RA / dérive DEC | Dérive mesurée sur chaque axe (pixels) |
| Impulsion RA / impulsion DEC | Durée de l'impulsion de correction (ms) |
| SNR | Rapport signal/bruit de la détection d'étoile |
| RMS | RMS total courant (pixels) |

## Actions

| Action | Description |
|---|---|
| **Calibrer** | Initialisation puis calibration seule |
| **Guider** | Démarrer le guidage — utilise la calibration enregistrée, ou calibre d'abord s'il n'y en a pas |
| **Abandonner** | Arrêter immédiatement la séquence en cours |
| **Réinitialiser la calibration** | Effacer les données de calibration enregistrées |
| **Dither** | Décaler aléatoirement la position de guidage jusqu'à *Amplitude de dither* |
