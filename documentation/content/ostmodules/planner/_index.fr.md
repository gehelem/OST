---
title: Planner
weight: 50
description: Planification multi-cibles et orchestration automatique de la session
---

## Rôle

Le module Planner orchestre une session d'astrophotographie multi-cibles. Il exécute une liste de lignes de planning, en déléguant successivement le pointage au module Navigator et l'acquisition au module Séquenceur. Pour chaque cible, il charge automatiquement un profil de séquence. Avant de pointer une cible, il vérifie aussi qu'elle reste au-dessus d'une élévation minimale pendant toute la durée estimée de la séquence et qu'elle est suffisamment éloignée de la lune, en sautant puis en retentant plus tard les cibles qui ne respectent pas ces contraintes.

![Capture d'écran du module Planner](/images/modules/planner.png)

## Appareils requis

| Appareil | Rôle |
|---|---|
| GPS | Fourniture des coordonnées géographiques et de l'heure |

## Structure d'un planning

Un planning est composé de plusieurs lignes. Chaque ligne définit :

| Colonne | Description |
|---|---|
| **Objet** | Nom de la cible |
| **RA** | Ascension droite de la cible (J2000) |
| **DEC** | Déclinaison de la cible (J2000) |
| **Profil** | Profil de séquence à charger dans le Séquenceur pour cette cible |
| **Progression** | Avancement de la ligne en cours |

Les lignes ne sont pas forcément exécutées strictement de haut en bas : une ligne qui échoue au contrôle de visibilité/lune est sautée puis retentée plus tard, une fois que toutes les autres lignes sont passées (voir *États d'une ligne* ci-dessous). Elles peuvent être ajoutées manuellement ou envoyées depuis le Navigator via l'action *Ajouter au planning*.

## Paramètres

### Modules esclaves

| Paramètre | Description |
|---|---|
| **Instance du Séquenceur** | Nom de l'instance du module Séquenceur à piloter |
| **Instance du Navigator** | Nom de l'instance du module Navigator à piloter |

### Contraintes de visibilité

| Paramètre | Description |
|---|---|
| **Minimum elevation (°)** | La cible doit rester au-dessus de cette élévation pendant toute la durée estimée de la séquence, sinon la ligne est sautée |
| **Moon illumination threshold (%)** | L'évitement de la lune ne s'applique que lorsque sa fraction illuminée dépasse ce pourcentage |
| **Moon separation threshold (°)** | La ligne n'est rejetée que si la cible est plus proche que cette distance **et** que l'illumination de la lune dépasse le seuil ci-dessus |
| **Max retries before giving up** | Une ligne qui échoue au contrôle de visibilité/lune ce nombre de fois passe en état *Failed* et n'est plus jamais retentée |

## Algorithme

### Démarrage

1. Vérification de la connexion INDI et des modules esclaves
2. Marquage de toutes les lignes à l'état *En attente* et réinitialisation des compteurs de tentatives
3. Démarrage de la première ligne

### Exécution d'une ligne

Pour chaque ligne du planning :

1. Le Planner demande au Séquenceur la durée théorique du profil demandé (sans le charger), via une requête générique inter-module
2. Il vérifie que la cible reste au-dessus de l'élévation minimale configurée pendant toute cette durée, et suffisamment loin de la lune (seuils d'illumination/distance) — si l'un des deux contrôles échoue, la ligne est sautée (voir *États d'une ligne*) et le Planner passe à la suivante
3. Sinon, il configure la cible du Navigator (RA, DEC, nom)
4. Il charge le profil de séquence demandé dans le Séquenceur et configure l'objet correspondant
5. Il déclenche l'action **Goto cible** du Navigator
6. Il attend la fin du centrage (événement de succès du Navigator)
7. Il déclenche l'action **Lancer la séquence** du Séquenceur
8. Il suit la progression de la séquence et attend sa fin
9. La ligne est marquée *Terminée* et le Planner passe à la prochaine ligne exploitable

### États d'une ligne

| État | Signification | Retentée plus tard ? |
|---|---|---|
| **Queued** | Pas encore tentée | — |
| **Checking visibility** | Contrôle élévation/lune en cours | — |
| **Slewing** | Centrage du Navigator sur la cible | — |
| **Skipped : \<raison\>** | Échec du contrôle de visibilité/lune | Oui, une fois que toutes les autres lignes sont passées |
| **Failed : \<raison\>** | Échec du contrôle de visibilité/lune trop de fois (*Max retries* atteint) | Non — exclue définitivement |
| **Cancelled** | La séquence a été interrompue manuellement en cours d'exécution (par opposition à une fin normale) | Non — exclue définitivement |
| **Finished** | Séquence terminée avec succès | Non |

{{% notice style="tip" title="Annulation manuelle vs fin normale" %}}
Le Planner distingue les deux cas : si tu interromps directement le Séquenceur pendant qu'il exécute une ligne du planning, cette ligne est marquée *Cancelled* et le Planner passe à la prochaine ligne exploitable — elle n'est pas confondue avec une réussite, et n'est plus retentée ensuite.
{{% /notice %}}

### Fin du planning

Le Planner continue de boucler sur les lignes, en retentant celles encore *Skipped*, jusqu'à ce que chaque ligne soit soit *Finished*, soit *Failed*, soit *Cancelled*. À ce moment-là, la progression globale passe à 100 % et le module s'arrête.

### Propagation de l'arrêt

Arrêter le Planner (que ce soit via le bouton **Arrêter**, ou parce qu'un module parent comme le Park Manager le demande) interrompt aussi ses modules esclaves : il envoie l'action **Interrompre** du Séquenceur et l'action **Interrompre** du Navigator, afin qu'un planning interrompu ne laisse pas une exposition ou un pointage se poursuivre sans supervision.

## Progression

| Indicateur | Description |
|---|---|
| **Progression globale** | Nombre de lignes traitées sur le total |
| **Progression par ligne** | Avancement de l'acquisition en cours, relayé depuis le Séquenceur |

## Actions

| Action | Description |
|---|---|
| **Démarrer** | Lance l'exécution du planning depuis la première ligne |
| **Arrêter** | Interrompt le planning en cours |
