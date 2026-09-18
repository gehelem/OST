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
Lecture  :  Datastore ────────────► UI

Écriture :  UI ─── SV/SA ───► Serveur ─── update ───► Datastore
```

- **Pas de duplication** : si un composant a besoin d'une valeur, il la lit
  dans le datastore, il ne la stocke pas de son côté.
- **Pas d'écriture directe** : l'UI ne modifie jamais le datastore
  elle-même, même pour un affichage optimiste.

{{% notice style="tip" title="Écriture : toujours via le serveur" %}}
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

## Reconnexion

{{% notice style="tip" title="Toujours repartir d'un dump complet" %}}
À la moindre coupure, le client doit **vider entièrement son
datastore** — aucun état partiel ne peut être considéré fiable après
une déconnexion.

Ce que le client fait ensuite (retenter automatiquement N fois,
proposer à l'utilisateur de relancer la connexion, abandonner...) ne
regarde que lui : ce n'est pas spécifié par le protocole, chaque
client est libre de sa propre stratégie.

Mais une fois (re)connecté, renvoyer `DU` pour obtenir un nouveau dump
complet n'est quasiment pas optionnel : reconstruire un état exploitable
à partir des seules mises à jour partielles qui arriveraient ensuite
(`ee`/`ea`/`ev`) est impossible — elles ne donnent qu'une vue
fragmentaire, jamais un état complet. Côté serveur, rien n'est à
préserver entre deux connexions d'un même client : c'est une
contrainte purement côté client.
{{% /notice %}}

## Exemple minimal : premier échange

Sans framework, juste le tout premier échange (connexion → `DU` →
écoute) — extrait de `test-ws-advanced.js`, le script utilisé pour le
health-check horaire du serveur :

```javascript
const ws = new WebSocket("ws://127.0.0.1:9624");

ws.on("open", () => {
  ws.send(JSON.stringify({ "DU": { "language": "en" } }));
});

ws.on("message", (data) => {
  console.log("received:", data.toString());
});
```

## Exemple : lecture et écriture dans OstErix

Extraits réels illustrant la séparation lecture/écriture de tête de
cette page, une fois qu'un vrai client (ici OstErix, en Angular) est
construit autour de ce principe.

**Lecture** — `datastore.service.ts`, un simple accès à l'état interne :
```typescript
getProperty(moduleName: string, propertyName: string): Observable<any> {
  return new Observable(subscriber => {
    const subscription = this.changed$.subscribe(() => {
      const module = this.modules[moduleName];
      const prop = (module as any)?.properties?.[propertyName] || null;
      subscriber.next(prop);
    });
    subscriber.next(this.modules[moduleName]?.properties?.[propertyName] || null);
    return () => subscription.unsubscribe();
  });
}
```

**Écriture** — `websocket.service.ts`, aucune mutation locale, juste
l'envoi du message :
```typescript
setPropertyOneElement(module: string, property: string, elements: { [key: string]: any }): void {
  this.send({
    SV: { m: { [module]: { p: { [property]: { e: elements } } } } }
  });
}
```

