---
title: Principes d'architecture client
weight: 50
description: "Comment structurer un client OST : datastore unique, écriture via WS uniquement"
---

Cette page décrit **comment un client doit être structuré**, pas le format
des messages échangés (voir [Structure des données]({{% relref "structure.fr.md" %}})
et les pages précédentes pour ça).

## Principe : un datastore unique détient la vérité

Le client maintient **une seule structure de données** représentant l'état
courant (modules → propriétés → éléments). Toute l'UI **lit** cette
structure ; rien d'autre ne la modifie que les messages reçus du serveur.

```
        lit                envoie SV/SA
  UI ────────► Datastore         UI ────────► Serveur
        ▲                                        │
        │              met à jour                │
        └──────── Datastore ◄─────────────────────┘
                     (uniquement sur retour serveur)
```

- **Pas de duplication** : si un composant a besoin d'une valeur, il la lit
  dans le datastore, il ne la stocke pas de son côté.
- **Pas d'écriture directe** : l'UI ne modifie jamais le datastore
  elle-même, même pour un affichage optimiste.

{{% notice style="warning" title="Écriture : toujours via le serveur" %}}
Une action utilisateur (changer une valeur, cliquer un bouton) déclenche
l'envoi d'un message au serveur (`SV`/`SA` — voir
[Client → Serveur]({{% relref "client-messages.fr.md" %}})). Le datastore
local **ne change qu'en retour**, quand le serveur répond avec l'update
confirmé. Le client ne présume jamais du résultat.
{{% /notice %}}

## Dump complet vs mises à jour partielles

À la connexion, le client envoie `DU` ; le serveur répond avec un dump
complet (`d`) contenant tous les modules avec leurs métadonnées
complètes (label, type, min/max, LOV, etc.).

{{% notice style="tip" title="Les mises à jour partielles ne portent que la valeur" %}}
Les messages qui arrivent ensuite (`ee`, `ea`) ne contiennent **que la
valeur**, pas les métadonnées. Le client doit :
1. Conserver les métadonnées du dernier dump complet.
2. Sur une mise à jour partielle, ne patcher **que** la valeur, sans
   jamais supposer que les métadonnées ont changé.

Si les métadonnées elles-mêmes changent (min/max/format d'un élément),
le serveur le signale explicitement via un message distinct, `ev`
(voir [Serveur → Client]({{% relref "server-messages.fr.md" %}})) — ce
n'est jamais silencieux. Le client n'a donc qu'à réagir au type de
message reçu, sans avoir à deviner ce qui a changé.
{{% /notice %}}

{{% notice style="warning" title="Piège connu : les mises à jour de GlobalLOV" %}}
Quand une propriété référence une LOV globale et que celle-ci change, le
serveur envoie la mise à jour **sans le champ `value`**. Le client ne doit
**jamais préserver l'ancienne valeur** dans ce cas précis — il faut
appliquer ce que le serveur envoie tel quel, y compris l'absence de
valeur. Le serveur fait foi ; ne jamais tenter de "combler" un champ
manquant avec l'ancien état local.
{{% /notice %}}

## Heartbeat

{{% notice style="note" title="Facultatif" %}}
Le heartbeat n'est pas imposé par le protocole. C'est un mécanisme que
le client met en place de son côté, seulement si sa technologie
l'exige, pour garder la connexion WebSocket active (certains
navigateurs, proxys ou plateformes ferment une connexion jugée
inactive). Un client qui n'en a pas besoin peut s'en passer entièrement.

Le mécanisme existe côté protocole pour qui en a besoin : envoyer `XX`
toutes les 30 secondes, le serveur répond `xx` (sans payload utile).
```json
→ {"XX": {}}
← {"xx": {}}
```
{{% /notice %}}

## À vérifier avant publication

- Comportement à la reconnexion après une coupure réelle (redemander
  `DU` ? état côté serveur conservé ou non ?) — rien trouvé de
  documenté sur ce point précis, contrairement au reste de cette page.
- Exemple de code minimal (pseudo-code ou extrait réel d'OstErix) pour
  illustrer ces principes, pas juste les décrire en prose
