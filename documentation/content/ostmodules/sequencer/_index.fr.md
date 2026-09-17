---
title: Séquenceur
weight: 30
description: Acquisition automatique de séquences d'images
---

## Rôle

Le module Séquenceur orchestre l'acquisition automatique lors d'une session d'astrophotographie. Il exécute une liste de lignes de séquence, chacune définissant le filtre, la durée de pose et le nombre d'images à acquérir. Le type de trame, le gain et l'offset sont désormais partagés par toute la séquence plutôt que définis ligne par ligne. Les fichiers FITS sont enregistrés automatiquement dans une arborescence organisée par objet et par filtre, et le module peut demander le refroidissement de la caméra, la mise au point automatique et le guidage autour de chaque ligne selon les besoins.

![Capture d'écran du module Séquenceur](/images/modules/sequencer.png)

## Appareils requis

| Appareil | Rôle |
|---|---|
| Caméra | Acquisition des images, et refroidissement si activé |
| Roue à filtres | Optionnelle — la séquence fonctionne sans elle ; le positionnement et les profils de mise au point par filtre sont simplement ignorés |

## Structure d'une séquence

Une séquence est composée de plusieurs lignes. Chaque ligne définit :

| Colonne | Description |
|---|---|
| **Filtre** | Filtre sélectionné sur la roue à filtres |
| **Exposition** | Durée d'exposition en secondes |
| **Nombre** | Nombre d'images à acquérir pour cette ligne |

Les lignes sont exécutées dans l'ordre, de la première à la dernière. Le type de trame, le gain et l'offset s'appliquent à toutes les lignes (voir *Paramètres de séquence* ci-dessous).

## Paramètres

Les réglages sont répartis en trois groupes, afin de ne pas mélanger le comportement de séquence, de guidage et de mise au point.

### Objet

| Paramètre | Description |
|---|---|
| **Nom** | Nom de l'objet imagé — utilisé pour nommer le dossier d'enregistrement |
| **AD** | Ascension droite de l'objet |
| **DEC** | Déclinaison de l'objet |

### Modules esclaves

Le séquenceur interagit avec d'autres modules pour la mise au point et le guidage.

| Paramètre | Description |
|---|---|
| **Instance du module de mise au point** | Nom de l'instance du module Focuser à utiliser (ex. `focus`) |
| **Instance du module de guidage** | Nom de l'instance du module Guider à utiliser (ex. `guider`) |

### Paramètres de séquence

| Paramètre | Description |
|---|---|
| **Activer le refroidissement caméra** | Demande le refroidissement à la température cible avant de démarrer la séquence, et attend qu'elle soit atteinte |
| **Température cible (°C)** | Température cible du capteur |
| **Type de trame** | Light (L), Bias (B), Dark (D), ou Flat (F) — s'applique à toutes les lignes |
| **Gain** | Gain de la caméra — s'applique à toutes les lignes |
| **Offset** | Offset de la caméra — s'applique à toutes les lignes |

### Paramètres de guidage

| Paramètre | Description |
|---|---|
| **Utiliser le guidage** | Active l'intégration avec le guideur (dithering, stabilisation, surveillance du RMS, suspension pendant la mise au point). Le guideur doit déjà guider, ou être démarré par le séquenceur si nécessaire. |
| **Temps de stabilisation après reprise du guidage (s)** | Délai d'attente en secondes après la reprise du guidage, avant de poursuivre les acquisitions |
| **Dithering toutes les N poses (0=désactivé)** | Demande un dithering toutes les N images capturées |
| **Seuil de recalibration RMS (0=désactivé)** | Interrompt l'exposition en cours et déclenche une recalibration du guideur lorsque le RMS maximal pendant cette exposition dépasse cette valeur |

### Paramètres de mise au point

| Paramètre | Description |
|---|---|
| **Mise au point auto au changement de filtre** | Déclenche une mise au point à chaque changement de filtre — cela couvre également la toute première ligne de la séquence, puisqu'aucune mise au point n'a encore eu lieu dans cette session du module. Relancer la même séquence évite une mise au point redondante si la dernière effectuée correspond déjà au filtre courant. |
| **Suspendre le guidage pendant la mise au point** | Met en pause le guidage avant la mise au point et le reprend ensuite |
| **Seuil de re-mise au point HFR (0=désactivé)** | Déclenche une mise au point supplémentaire en cours de ligne lorsque le HFR mesuré dépasse ce seuil |
| **Profil de mise au point à charger** | Profil à charger sur le module de mise au point avant de demander l'autofocus ; laisser vide pour conserver le profil actuellement chargé |

### Profil de mise au point par filtre

Une grille dédiée permet de surcharger le profil de mise au point pour des filtres spécifiques — utile par exemple lorsque les filtres à bande étroite nécessitent des réglages de mise au point différents des filtres LRGB.

| Colonne | Description |
|---|---|
| **Filtre** | Filtre auquel s'applique cette ligne |
| **Profil de mise au point** | Profil à charger lors de la mise au point sur ce filtre |

Si le filtre courant possède une ligne dans cette grille, son profil est utilisé. Sinon, le séquenceur se rabat sur *Profil de mise au point à charger* ci-dessus. Si celui-ci est également vide, le module de mise au point conserve le profil qu'il a actuellement chargé.

## Algorithme

### Démarrage de la séquence

1. Connexion à la caméra et remise à zéro de la trame
2. Si *Activer le refroidissement caméra* est actif, demande la température cible et attend qu'elle soit atteinte (ignoré immédiatement si désactivé ou si la caméra n'est pas configurée)
3. Démarrage de la première ligne

### Exécution d'une ligne

Pour chaque ligne de la séquence :

1. Sélection du filtre sur la roue à filtres (ignoré si aucune roue à filtres n'est configurée)
2. Si *Mise au point auto au changement de filtre* est activé et que le filtre diffère de celui utilisé lors de la dernière mise au point de cette session : déclenchement de la mise au point automatique (en chargeant d'abord un profil — surcharge par filtre, puis repli général — et en suspendant/reprenant le guidage si configuré)
3. Démarrage ou confirmation du guidage si *Utiliser le guidage* est activé
4. Dithering si l'intervalle de poses configuré est atteint
5. Acquisition du nombre d'images défini une par une, avec re-mise au point en cours de ligne si le seuil HFR est dépassé

### Enregistrement des fichiers

Les fichiers FITS sont enregistrés dans l'arborescence suivante (les segments objet et filtre sont omis lorsqu'ils ne sont pas définis, plutôt que de laisser des séparateurs superflus) :

```
<objet>/
  LIGHT/
    <filtre>/   ← Trames Light
  FLAT/
    <filtre>/   ← Trames Flat
  BIAS/         ← Trames Bias
  DARK/         ← Trames Dark
```

Chaque nom de fichier comprend le nom de l'objet, le type de trame, le filtre et un horodatage.

### Intégration avec le module de mise au point

Lorsqu'une mise au point automatique est demandée :

1. Si un profil s'applique (surcharge par filtre, ou repli général), le séquenceur demande au module Focuser de le charger
2. Le séquenceur envoie l'action `autofocus` au module Focuser désigné
3. Il met en pause les acquisitions et attend la fin de la mise au point
4. Si *Suspendre le guidage* est activé, il envoie `abortguider` au module Guider avant la mise au point, puis `guide` ensuite
5. Si un temps de stabilisation est configuré, il attend ce délai avant de reprendre

### Intégration avec le module de guidage

- Le guidage est démarré (ou sa présence confirmée) avant le début des expositions, lorsque *Utiliser le guidage* est activé
- Un dithering est demandé toutes les N poses, puis le séquenceur attend le temps de stabilisation configuré
- Si le RMS maximal pendant une exposition dépasse le seuil configuré, l'exposition est interrompue et le guideur est recalibré avant de reprendre

## Progression

La progression est suivie à la fois en direct (mise à jour pendant l'acquisition) et sous forme de moyennes/durées cumulées depuis le chargement du module.

| Indicateur | Description |
|---|---|
| **Séquence (poses)** | Nombre de poses terminées sur le total de la séquence entière |
| **Séquence (temps)** | Temps d'exposition écoulé sur le temps d'exposition total estimé, mis à jour en direct pendant l'exposition en cours |
| **Exposition en cours** | Progression de l'exposition en cours (0 à 100 %) |
| **Durée totale théorique** | Somme de nombre × exposition sur toutes les lignes, recalculée à chaque modification de la grille — même avant le démarrage de la séquence |
| **Durée restante théorique** | Durée totale théorique moins le temps déjà écoulé |
| **Durée réelle écoulée** | Chronomètre réel, remis à zéro à chaque démarrage de la séquence |
| **Durée moyenne de mise au point** | Moyenne glissante du temps passé à attendre l'autofocus, depuis le chargement du module |
| **Durée moyenne de démarrage du guidage** | Moyenne glissante du temps passé à attendre le démarrage du guideur, depuis le chargement du module |

## Actions

| Action | Description |
|---|---|
| **Démarrer la séquence** | Démarre l'exécution de la séquence depuis la première ligne |
| **Interrompre** | Arrête immédiatement la séquence en cours, et propage aussi une demande d'interruption aux instances Guider et Focuser configurées dans *Modules esclaves*, afin d'arrêter également le guidage et une éventuelle mise au point en cours |
