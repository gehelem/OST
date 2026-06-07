---
title: Module - Sequencer
weight : 20
---

{{% notice style="note" title="Attention" icon="exclamation-triangle" %}}
19/10/2025 - Note du développeur :
Cette documentation a été générée par l'IA Claude, comme un point de départ que je mettrai à jour lorsque je trouverai un peu de temps
{{% /notice %}}

# Module Sequencer

Le module **Sequencer** permet d'automatiser l'acquisition d'images astronomiques en définissant des séquences de poses avec différents paramètres (filtres, temps d'exposition, gain, offset, etc.).

## Vue d'ensemble

Le séquenceur est conçu pour gérer des sessions d'imagerie complexes en permettant :
- La définition de séquences multi-lignes avec différents paramètres par ligne
- L'intégration avec le module de mise au point automatique (autofocus)
- L'intégration avec le module de guidage
- Le suivi de la progression en temps réel
- La sauvegarde des configurations dans des profils

## Configuration de base

### Object (Objet cible)

Cette section permet de définir les coordonnées de l'objet observé :

| Paramètre | Description |
|-----------|-------------|
| **Name** | Nom de l'objet (ex: M31, NGC7000) |
| **RA** | Ascension droite de l'objet |
| **DEC** | Déclinaison de l'objet |

Ces informations sont utilisées pour nommer les fichiers acquis et peuvent être synchronisées avec le module navigator.

### Sequence (Séquence d'acquisition)

La séquence est définie sous forme de tableau où chaque ligne représente un ensemble de poses :

| Colonne | Description | Valeurs possibles |
|---------|-------------|-------------------|
| **Frame type** | Type de pose | Light (L), Bias (B), Dark (D), Flat (F) |
| **Filter** | Filtre à utiliser | Liste des filtres disponibles sur la roue |
| **Exposure** | Temps d'exposition en secondes | Nombre décimal (ex: 300.0 pour 5 minutes) |
| **Count** | Nombre de poses à réaliser | Nombre entier |
| **Gain** | Gain de la caméra | Valeur dépendant de la caméra |
| **Offset** | Offset de la caméra | Valeur dépendant de la caméra |
| **Progress** | Progression de la ligne | Indicateur visuel (automatique) |

#### Exemple de séquence

```
Frame Type | Filter | Exposure | Count | Gain | Offset
-----------|--------|----------|-------|------|-------
Light      | Ha     | 300      | 20    | 120  | 20
Light      | OIII   | 300      | 20    | 120  | 20
Light      | SII    | 300      | 20    | 120  | 20
Dark       | -      | 300      | 10    | 120  | 20
```

### Progress (Progression)

Cette section affiche l'avancement en temps réel :

- **Sequence** : Progression globale de toute la séquence
- **Current exposure** : Progression de la pose en cours

## Paramètres avancés

### Auto-focus at sequence start

**Type** : Booléen (case à cocher)
**Valeur par défaut** : Activé

Lorsque cette option est activée, le séquenceur déclenche automatiquement une mise au point avant de commencer la première ligne de la séquence. La mise au point utilise le filtre défini dans la première ligne.

**Utilisation recommandée** : Toujours activé pour garantir une mise au point optimale au début de la session.

### Auto-focus on filter change

**Type** : Booléen (case à cocher)
**Valeur par défaut** : Désactivé

Déclenche automatiquement une mise au point chaque fois que le filtre change entre deux lignes de la séquence.

**Utilisation recommandée** :
- Activé si votre système optique présente un décalage de mise au point entre filtres (chromatisme)
- Désactivé si vos filtres sont parfaitement compensés

### Focus module instance

**Type** : Sélection de module
**Valeur par défaut** : "focus"

Permet de sélectionner quelle instance du module focus utiliser pour les opérations d'autofocus. Utile si vous avez plusieurs modules focus chargés avec des configurations différentes.

### Suspend guiding during focus

**Type** : Booléen (case à cocher)
**Valeur par défaut** : Désactivé

Lorsque cette option est activée :
1. Le séquenceur suspend le guidage avant de lancer l'autofocus
2. L'autofocus s'exécute sans guidage actif
3. Le guidage reprend automatiquement après la mise au point
4. Un temps de stabilisation est appliqué (voir paramètre suivant)

**Utilisation recommandée** :
- Activé si le système de guidage perturbe la mesure de HFR pendant l'autofocus
- Activé pour les petits déplacements de mise au point qui pourraient déclencher des corrections de guidage parasites

### Guider module instance

**Type** : Sélection de module
**Valeur par défaut** : "guider"

Définit quelle instance du module guider contrôler pour les opérations de suspension/reprise du guidage.

### Resume guiding settle time

**Type** : Entier (secondes)
**Valeur par défaut** : 10 secondes

Temps d'attente après la reprise du guidage avant de continuer la séquence. Ce délai permet au système de guidage de se stabiliser et d'éviter de commencer une pose pendant que le guidage converge.

**Valeurs recommandées** :
- **5-10 secondes** : Configuration standard
- **15-20 secondes** : Monture avec beaucoup d'inertie ou guidage agressif
- **3-5 secondes** : Monture très stable avec guidage doux

## Utilisation pratique

### Démarrer une séquence

1. **Définir l'objet cible** dans la section "Object"
2. **Créer les lignes de séquence** en cliquant sur "+" pour ajouter des lignes
3. **Configurer chaque ligne** avec les paramètres souhaités
4. **Vérifier les paramètres** d'autofocus et de guidage
5. **Lancer la séquence** via le bouton d'action

### Arrêter une séquence

La séquence peut être interrompue à tout moment. L'arrêt se fait de manière propre :
- La pose en cours est terminée
- Les fichiers sont sauvegardés
- Les équipements sont remis en état sûr

### Reprendre une séquence interrompue

Le séquenceur conserve la progression de chaque ligne. Pour reprendre :
1. La séquence reprend automatiquement là où elle s'était arrêtée
2. Les lignes déjà complétées sont ignorées
3. La ligne en cours reprend au nombre de poses restantes

## Intégration avec d'autres modules

### Module Focus

Le séquenceur communique avec le module focus pour :
- Déclencher l'autofocus au début de la séquence
- Déclencher l'autofocus lors des changements de filtre
- Attendre la fin de l'autofocus avant de continuer

**Configuration requise** : Le module focus doit être chargé et configuré.

### Module Guider

Le séquenceur peut contrôler le module guider pour :
- Suspendre le guidage pendant l'autofocus
- Reprendre le guidage après l'autofocus
- Attendre la stabilisation du guidage

**Configuration requise** : Le module guider doit être chargé, calibré et en cours de guidage.

### Module Camera

Le séquenceur contrôle automatiquement :
- Les paramètres d'exposition (temps, gain, offset)
- Le type de pose (Light/Dark/Bias/Flat)
- L'enregistrement des fichiers FITS

### Module Filter Wheel

Si une roue à filtres est configurée :
- Le séquenceur change automatiquement de filtre entre les lignes
- Le changement de filtre peut déclencher un autofocus (si configuré)

## Profils

Les configurations suivantes sont sauvegardées dans les profils :
- Définition complète de la séquence (toutes les lignes)
- Coordonnées de l'objet
- Tous les paramètres d'autofocus et de guidage

Pour sauvegarder un profil :
1. Configurer la séquence et les paramètres
2. Utiliser la fonction de sauvegarde de profil du système
3. Donner un nom significatif (ex: "M31_HaOIIISII", "Broadband_LRGB")

## Conseils et bonnes pratiques

### Organisation des séquences

- **Grouper les poses similaires** : Toutes les poses Ha ensemble, puis OIII, etc.
- **Alterner si nécessaire** : Pour compenser la rotation du champ ou les conditions changeantes
- **Darks à la fin** : Acquérir les darks en fin de session quand la caméra est à température stable

### Gestion de l'autofocus

- **Toujours activer l'autofocus au démarrage** pour partir sur de bonnes bases
- **Tester le décalage entre filtres** avant d'activer l'autofocus sur changement de filtre
- **Monitorer les premières poses** de chaque filtre pour vérifier la qualité du focus

### Gestion du guidage

- **Calibrer le guidage avant** de démarrer la séquence
- **Activer la suspension pendant focus** si vous constatez des variations de HFR erratiques
- **Ajuster le settle time** selon votre monture (observer les premières poses après focus)

### Optimisation du temps

Le temps total de la séquence inclut :
- Temps d'exposition total (sum of all exposures)
- Temps de lecture de la caméra (readout time)
- Temps de changement de filtre (~2-5 secondes par changement)
- Temps d'autofocus (~1-3 minutes par autofocus)
- Temps de stabilisation du guidage (paramètre settle time)

**Astuce** : Minimiser le nombre de changements de filtre réduit le temps total.

### Gestion des erreurs

En cas de problème pendant la séquence :
- **Nuages/météo** : Arrêter proprement, la reprise sera possible
- **Perte de guidage** : Vérifier la configuration du guider avant de reprendre
- **Problème de focus** : Vérifier le module focus, recalibrer si nécessaire
- **Erreur caméra** : Vérifier la connexion INDI, redémarrer la caméra si nécessaire

## Dépannage

### La séquence ne démarre pas

- Vérifier que la caméra est connectée
- Vérifier que le module focus est chargé (si autofocus activé)
- Vérifier que tous les paramètres de la séquence sont valides
- Consulter les messages d'erreur dans les logs

### L'autofocus échoue

- Le séquenceur met la séquence en pause
- Corriger le problème dans le module focus
- Relancer manuellement l'autofocus
- Reprendre la séquence

### Le guidage ne reprend pas

- Vérifier que le module guider est toujours actif
- Vérifier que l'étoile guide est toujours visible
- Ajuster le paramètre settle time si nécessaire
- Recalibrer le guidage si nécessaire

### Les fichiers ne sont pas sauvegardés

- Vérifier les permissions du répertoire de destination
- Vérifier l'espace disque disponible
- Vérifier la configuration du module caméra

## Formats de fichiers

Les fichiers FITS générés contiennent les métadonnées suivantes dans leur en-tête :
- `OBJECT` : Nom de l'objet
- `OBJCTRA` : Ascension droite
- `OBJCTDEC` : Déclinaison
- `EXPTIME` : Temps d'exposition
- `GAIN` : Gain utilisé
- `OFFSET` : Offset utilisé
- `FILTER` : Filtre utilisé
- `IMAGETYP` : Type de pose (Light/Dark/Bias/Flat)

Ces métadonnées sont utilisées par les logiciels de traitement (PixInsight, Siril, etc.) pour organiser et calibrer les images.

## Conclusion

Le module Sequencer est l'outil central pour automatiser vos sessions d'imagerie. Une bonne maîtrise de ses paramètres et de son intégration avec les modules focus et guider permet d'obtenir des sessions d'acquisition fiables et de haute qualité, même sans surveillance.
