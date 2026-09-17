---
title: IndiPanel
weight: 80
description: Panneau de contrôle brut de tous les appareils INDI connectés
---

## Rôle

Le module IndiPanel est un panneau de contrôle transparent donnant accès à toutes les propriétés INDI de tous les appareils connectés au serveur INDI. Contrairement aux autres modules OST, il ne remplit pas une fonction astrophotographique spécifique : il expose directement l'intégralité du protocole INDI dans l'interface OST.

![Capture d'écran du module IndiPanel](/images/modules/indipanel.png)

{{% notice style="note" title="Module sans configuration" %}}
IndiPanel n'a pas de fichier de configuration ni de paramètres à définir. Son contenu est entièrement généré à la volée à partir des appareils et propriétés découverts sur le serveur INDI.
{{% /notice %}}

## Fonctionnement

Dès le démarrage, le module se connecte au serveur INDI et écoute tous les événements de découverte d'appareils et de propriétés. Pour chaque propriété INDI reçue, une entrée correspondante est créée automatiquement dans l'interface OST :

| Type INDI | Représentation OST |
|---|---|
| Number | Valeur numérique |
| Switch | Bouton on/off ou groupe de boutons (OneOfMany, AnyOfMany) |
| Text | Champ texte |
| Light | Indicateur d'état |
| BLOB | Image (JPEG + FITS) |

Les propriétés sont organisées par appareil INDI et par groupe, exactement comme dans tout client INDI standard.

## Interaction bidirectionnelle

- **INDI → OST** : toute mise à jour d'une propriété INDI (valeur, état) se répercute immédiatement dans l'interface
- **OST → INDI** : toute modification effectuée par l'utilisateur est renvoyée au pilote INDI correspondant (`sendNewSwitch`, `sendNewNumber`, `sendNewText`)

Les images (BLOBs) sont automatiquement affichées dès qu'un pilote envoie une image — y compris depuis des caméras non configurées dans d'autres modules.

## Cas d'usage

- Accès direct aux propriétés avancées d'un pilote INDI non exposées par les autres modules
- Débogage d'un appareil ou d'un pilote
- Contrôle d'un appareil exotique sans module OST dédié
- Consultation des messages d'état émis par les pilotes INDI

## Appareils

Aucun appareil n'est à configurer. Le module se connecte automatiquement au serveur INDI et découvre tous les appareils présents.
