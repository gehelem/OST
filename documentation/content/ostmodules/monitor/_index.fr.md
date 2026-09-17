---
title: Monitor
weight: 75
description: Surveillance et historisation des événements de session
---

## Rôle

Le module Monitor enregistre les événements émis par les modules esclaves pendant une session d'observation et les restitue sous forme graphique. Il permet de visualiser a posteriori la qualité du guidage, l'évolution du HFR des images et les interventions de mise au point, sur un axe temporel commun.

![Capture d'écran du module Monitor](/images/modules/monitor.png)

## Principe de fonctionnement

Le Monitor observe la propriété `signals` de chaque module esclave configuré. Chaque fois qu'un module émet un événement (via `setStateEvent`), le Monitor l'enregistre avec un horodatage et les valeurs numériques associées.

Les événements sont stockés en mémoire pendant toute la session. À l'arrêt, ils sont exportés dans un fichier JSON horodaté sous `<webroot>/monitor/`.

## Modules surveillés

| Module | Événements captés |
|---|---|
| Guider | `guideRMS` — RMS total du guidage (arcsec) |
| Guider | `guideSNR` — rapport signal/bruit de la détection d'étoiles |
| Focus | `focusdone` — HFR final après mise au point (pixels) |
| Séquenceur | `imagehfr` — HFR de l'image enregistrée (pixels) |

## Graphe de session

Le graphe principal affiche l'ensemble de la session sur un axe temporel commun.

| Courbe | Description |
|---|---|
| Ligne orange | RMS du guidage (axe gauche, arcsec) |
| Points bleus | HFR des images acquises (axe droit, pixels) |
| Triangles verts | HFR mesuré en fin de mise au point (axe droit, pixels) |
| Ligne rouge pointillée | SNR du guidage (masquée par défaut, activable via la légende) |

Le slider double sous le graphe permet de restreindre la fenêtre temporelle affichée sans perte des données en mémoire.

## Actions

| Action | Description |
|---|---|
| **Start** | Démarre une nouvelle session d'enregistrement (efface les données précédentes) |
| **Stop** | Arrête l'enregistrement et exporte les données en JSON |

## Paramètres

### Parms

| Paramètre | Description |
|---|---|
| **Auto-start** | Démarre automatiquement l'enregistrement au chargement du module (sauvegardé dans le profil) |

### Filtrage de la vue

| Paramètre | Description |
|---|---|
| **From** | Borne inférieure de l'affichage (filtre les événements antérieurs à cette heure) |
| **To** | Borne supérieure de l'affichage (filtre les événements postérieurs à cette heure) |
| **Max rows** | Nombre maximum de lignes conservées dans la vue live (défaut : 200) |

### Modules surveillés (Slaves)

Chaque entrée permet de sélectionner l'instance de module à surveiller pour chaque type (guider, séquenceur, focus, etc.).

## Persistance JSON

À chaque arrêt de session, un fichier est exporté sous :

```
<webroot>/monitor/YYYYMMDD-HHmmss.json
```

Le fichier contient les en-têtes de colonnes (`gridheaders`) et la grille de données (`grid`) au format tableau de tableaux, réutilisable directement pour une analyse externe.

{{% notice style="note" title="Démarrage automatique" %}}
Avec **Auto-start** activé dans le profil, le module lance l'enregistrement une seconde après son chargement, sans intervention manuelle.
{{% /notice %}}
