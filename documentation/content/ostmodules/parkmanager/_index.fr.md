---
title: Park Manager
weight: 70
description: Automatisation de l'ouverture et de la fermeture de l'observatoire
---

## Rôle

Le module Park Manager orchestre l'ouverture et la fermeture complète de l'observatoire : ouverture du volet de la coupole, déparquage de la monture, démarrage du suivi — et dans l'ordre inverse pour la fermeture. Il peut fonctionner en mode manuel (action ponctuelle) ou en mode automatique piloté par des règles horaires et météo.

![Capture d'écran du module Park Manager](/images/modules/parkmanager.png)

## Appareils requis

| Appareil | Rôle |
|---|---|
| Coupole | Ouverture et fermeture du volet, parquage |
| Monture | Déparquage, suivi, retour au home, parquage |
| Station météo | Surveillance des conditions pour le mode automatique (optionnel) |

## Paramètres

### Règles météo

| Paramètre | Description |
|---|---|
| **Must be OK** | Si activé, l'ouverture est refusée si la météo n'est pas au vert. En cours de session, une dégradation météo déclenche automatiquement la fermeture |
| **Disabled** | Désactive la surveillance météo (la station météo est ignorée) |

### Modules esclaves

| Paramètre | Description |
|---|---|
| **Planner instance** | Nom de l'instance du module Planner à arrêter au démarrage de la séquence de fermeture |

### Règles horaires

Ces règles définissent la fenêtre d'ouverture automatique en mode **Automate**. Elles sont mutuellement exclusives.

| Paramètre | Description |
|---|---|
| **Fixed time** | Fenêtre horaire fixe définie par une heure de début et une heure de fin |
| **Fixed time : start** | Heure de début de la fenêtre fixe |
| **Fixed time : end** | Heure de fin de la fenêtre fixe (peut être le lendemain — gestion du passage minuit) |
| **Dusk and dawn** | Fenêtre automatique du coucher au lever du soleil (calculé à partir des coordonnées GPS) |
| **Anytime** | Ouverture permanente : le module ouvre dès qu'il est en mode Auto et reste ouvert |

## Séquences

### Séquence d'ouverture

1. Vérification météo (si la règle *Must be OK* est active)
2. Ouverture du volet de la coupole
3. Attente confirmation ouverture volet
4. Déparquage de la monture
5. Attente confirmation déparquage
6. Démarrage du suivi sidéral
7. Entrée en surveillance (*Open Monitoring*)

### Séquence de fermeture

1. Coupure du refroidissement sur tous les appareils connectés exposant un switch `CCD_COOLER` (pas seulement la caméra utilisée par le Séquenceur), et demande d'arrêt à l'instance du Planner
2. Arrêt du suivi sidéral
3. Envoi de la monture en position home
4. Attente arrivée en position home
5. Parquage de la monture
6. Attente confirmation parquage
7. Fermeture du volet de la coupole
8. Attente confirmation fermeture volet

{{% notice style="warning" title="Ordre de fermeture" %}}
La monture est toujours parquée **avant** la fermeture de la coupole. Ne jamais interrompre manuellement la séquence entre ces deux étapes.
{{% /notice %}}

{{% notice style="tip" title="Propagation de l'arrêt" %}}
Arrêter le Planner interrompt à son tour le Séquenceur (qui interrompt lui-même le Guider et le Focuser) ainsi que le Navigator — voir la documentation [Planner](../planner/). Cela garantit que toute la chaîne d'acquisition s'arrête proprement avant la fermeture physique de l'observatoire.
{{% /notice %}}

### Arrêt d'urgence

L'action **Abort** envoie simultanément un ordre d'arrêt à la monture (`TELESCOPE_ABORT_MOTION`) et à la coupole (`DOME_ABORT_MOTION`), puis repasse en mode Idle.

## Mode automatique

En mode **Automate**, le module évalue toutes les 2 secondes si l'observatoire doit être ouvert ou fermé selon les règles horaires et météo configurées.

{{< mermaid >}}
flowchart TB
    ATTENTE([Attente]) -->|"fenêtre active & météo OK"| OD

    subgraph OUV ["Séquence d'ouverture"]
        direction LR
        OD[Ouvre coupole] --> OW[Attend ouverture] --> UD[Déparque monture] --> UW[Attend déparquage] --> TR[Démarre suivi]
    end

    TR --> SURV([Surveillance])
    SURV -->|"fenêtre terminée ou météo dégradée"| ST

    subgraph FER ["Séquence de fermeture"]
        direction LR
        ST[Arrête suivi] --> GH[Goto home] --> WH[Attend home] --> PM[Parque monture] --> WP[Attend parquage] --> CD[Ferme coupole]
    end

    CD --> ATTENTE
    ATTENTE -->|Abort| IDLE([Idle])
    SURV -->|Abort| IDLE
{{< /mermaid >}}

| Situation | Action |
|---|---|
| Fenêtre ouverte + observatoire fermé | Lancement de la séquence d'ouverture |
| Fenêtre fermée + observatoire ouvert | Lancement de la séquence de fermeture |
| Météo dégradée en cours de session | Lancement de la séquence de fermeture |
| Fin de séquence | Retour en attente — le mode Auto est conservé |

{{% notice style="tip" title="Persistance du mode Auto" %}}
Après une séquence d'ouverture ou de fermeture, le module reste en mode **Auto** et continue de surveiller les conditions. Il n'est pas nécessaire de le réactiver manuellement.
{{% /notice %}}

## Actions

| Action | Description |
|---|---|
| **Open all** | Lance la séquence d'ouverture complète |
| **Close all** | Lance la séquence de fermeture complète |
| **Automate** | Active le mode automatique piloté par les règles |
| **Abort motion** | Arrêt d'urgence de tous les mouvements |
| **Idle** | Repasse en mode inactif sans action |

## État affiché

### Monture

| Valeur | Description |
|---|---|
| **Mount RA** | Ascension droite courante |
| **Mount DEC** | Déclinaison courante |
| **Mount is tracking** | Suivi sidéral actif |
| **Mount is parked** | Monture en position de parquage |
| **Mount is at home** | Monture en position home |

### Coupole

| Valeur | Description |
|---|---|
| **Dome is parked** | Coupole en position de parquage |
| **Dome shutter is closed** | Volet fermé |

### Météo et événements

| Valeur | Description |
|---|---|
| **Global** | État global de la météo (vert / orange / rouge) |
| **Next sunset** | Heure du prochain coucher de soleil |
| **Next sunrise** | Heure du prochain lever de soleil |
