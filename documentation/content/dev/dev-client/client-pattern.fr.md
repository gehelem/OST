---
title: Principes d'architecture client
weight: 10
description: "Comment structurer un client OST : datastore unique, écriture via WS uniquement"
---

Cette page décrit **comment un client doit être structuré**, pas le format
des messages échangés (voir [Structure des données]({{% relref "structure.fr.md" %}})
et les pages suivantes pour ça).

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

## Principe : aucune dépendance à internet

Un serveur OST tourne souvent sur un réseau local isolé — un site d'observatoire distant sans accès à internet est un déploiement normal, pas un cas limite. Un client doit fonctionner correctement dans cette situation : **toute ressource dont il a besoin doit être embarquée avec le client lui-même**, pas récupérée depuis un CDN ou un autre service externe au moment de l'exécution.

Ça concerne directement le rendu des icônes : [`preicon1`/`preicon2`/`posticon1`/`posticon2`/`preicon`/`posticon`]({{% relref "structure.fr.md" %}}) donnent un *nom* d'icône Google Fonts (Material Icons), pas une URL — un client doit embarquer localement la police/les glyphes d'icônes (vendorisés, auto-hébergés, ou empaquetés à la construction) plutôt que de charger la feuille de style du CDN Google, qui casse silencieusement dès qu'il n'y a plus de route vers internet.

Ça vaut aussi pour tout ce qu'un client pourrait être tenté de tirer d'un CDN : un framework JS, une bibliothèque de graphes, une police web pour l'UI elle-même. Si le client en a besoin pour fonctionner, ça part avec le client.

## Recommandation forte : proposer au moins un thème sombre et un thème clair

Les clients OST sont couramment utilisés à l'oculaire ou à côté d'un télescope, de nuit — une interface pleine luminosité à fond blanc n'est pas qu'un décalage de style dans ce contexte, elle détruit purement et simplement la vision nocturne de l'opérateur. Un client doit proposer **au minimum un thème sombre et un thème clair**, et non figer l'un ou l'autre en dur.

Ce n'est pas une exigence du protocole filaire — rien ici n'est imposé ni même visible côté serveur — mais c'est une recommandation forte pour tout ce qui est destiné à être utilisé lors de vraies sessions d'astrophotographie, pas seulement comme démo ou outil de test de bureau.

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

## Rendu : ne rafraîchir que la propriété modifiée

{{% notice style="warning" title="Impératif : ne jamais re-render plus large que la propriété modifiée" %}}
Certains modules peuvent envoyer plusieurs mises à jour `ee`/`ea` **par
seconde** (par exemple un module qui interroge en continu un
périphérique). Un client qui réagit à chaque mise à jour reçue en
redessinant toute son UI — ou même juste tout le module affiché —
devient inutilisable sous cette charge : perte du focus de saisie,
clignotement, et dans le pire cas l'UI cesse totalement de répondre
aux interactions.

Ce n'est pas un cas limite à anticiper "par précaution" : c'est un
comportement réel et actuel du backend, et les clients **doivent**
être construits pour l'encaisser dès le départ :
1. À chaque mise à jour, seul l'objet JS/données de la propriété
   modifiée doit être remplacé/muté. La référence de toute autre
   propriété doit rester strictement identique tant qu'elle n'est pas
   concernée.
2. Le rendu de l'UI doit être indexé par l'identité de la propriété
   (ou du module) — une liste de propriétés doit réutiliser le même
   élément d'UI pour la même propriété d'un rendu à l'autre, jamais le
   détruire et le recréer, afin qu'un champ de saisie non concerné ne
   soit jamais touché, quoi qu'il se passe par ailleurs.
3. Limiter la **fréquence** des rendus (debounce/throttle) ne suffit
   pas si le rendu lui-même reconstruit de l'UI non concernée — le
   correctif est dans le **périmètre** de ce qui est reconstruit, pas
   seulement dans sa fréquence.

Rooster (le frontend de production) implémente ce principe par
construction : une mise à jour entrante ne remplace que la référence
de la propriété concernée dans le store, et toute liste de propriétés
est rendue avec une clé stable par propriété. Résultat : l'UI des
propriétés non concernées n'est jamais perturbée, quelle que soit la
vitesse à laquelle une autre propriété se met à jour en parallèle.
{{% /notice %}}

## Templates : écrans par type de module, avec un repli générique

Chaque instance de module peut annoncer `infos.template` (voir
[Modèle de données]({{% relref "structure.fr.md" %}})) — une chaîne
nommant son type (`"focus"`, `"guider"`, `"sequencer"`...). C'est
purement indicatif, et **son absence est autorisée**.

{{% notice style="tip" title="Template absent = l'écran par défaut" %}}
Un client qui gère des écrans dédiés par template doit traiter un
`infos.template` absent (ou non reconnu) exactement comme la valeur
littérale `"default"` — pas comme une erreur. Rooster procède ainsi
avec un petit registre associant un nom de template en minuscules à un
composant d'écran, avec repli sur son écran générique/par défaut quand
le nom est absent ou absent du registre :
```
template = infos.template?.toLowerCase() || "default"
screen = registry[template] ?? registry["default"]
```
C'est ce qui permet à un client de rester pleinement fonctionnel face
à un type de module qu'il n'a jamais vu — un module nouveau ou tiers
s'affiche toujours via l'écran générique — tout en offrant un écran
soigné et dédié pour les types de modules qui le méritent.

L'écran `"default"` n'est pas un repli minimal symbolique : c'est le
**seul** écran que certains types de modules auront jamais, il doit
donc rendre tout ce qu'une propriété/élément générique peut porter —
status, `hint`, pré/posticônes, grilles, graphes, LOV, tout — pas
seulement les valeurs. Un écran dédié par template peut être plus
sélectif (masquer des champs, réorganiser la mise en page), mais
l'écran par défaut ne doit pas faire disparaître silencieusement de
l'information faute d'un écran dédié écrit pour ce module.
{{% /notice %}}

{{% notice style="tip" title="Recommandé : un seul composant de rendu de propriété, réutilisé partout" %}}
Pas obligatoire, mais fortement conseillé : construire un composant
**unique**, responsable du rendu d'une propriété (ou d'un élément) —
gérant `directedit`, `permission`, les trois portées de LOV, les
règles de groupe bool, `autoupdate`, les grilles, les graphes — et
faire en sorte que **tous** les écrans l'utilisent pour leurs
propriétés individuelles, y compris les écrans dédiés par template, pas
seulement le repli générique.

L'alternative — chaque écran réimplémentant son propre rendu de
champs — duplique chaque nuance du format d'échange dans toute la base
de code, et un correctif ou une nouvelle capacité backend n'atteint
que les écrans dont quelqu'un s'est souvenu. Les écrans dédiés de
Rooster (`GuiderComponent`, `FocusComponent`...) ont leur propre mise
en page, mais délèguent chaque propriété individuelle au même
composant de propriété générique que l'écran de repli — un correctif
fait une seule fois à cet endroit (par exemple gérer correctement un
groupe de switches exclusifs) s'applique partout automatiquement,
écrans dédiés compris.
{{% /notice %}}

## Formulaires de propriété : `SA`, obligatoire pour `directedit: false`

Au-delà du `SV` champ par champ, une propriété éditable a besoin d'un **chemin formulaire** : un écran listant tous ses éléments éditables à la fois, soumis ensemble via [`SA`]({{% relref "client-messages.fr.md" %}}#sa).

{{% notice style="warning" title="Pas optionnel pour un élément `directedit: false`" %}}
`SV` est rejeté par le backend pour tout élément avec `directedit: false` (voir [Modèle de données]({{% relref "structure.fr.md" %}})). `SA` est le **seul** moyen de jamais mettre à jour un tel élément — un client sans chemin formulaire ne peut tout simplement pas écrire sur cet élément, quelle que soit la qualité du reste de son UI.
{{% /notice %}}

Seuls les éléments **éditables** ont leur place dans ce formulaire — les types d'affichage seul (`img`, `video`, `light`, `prg`, `message`) sont rendus, jamais soumis ; à exclure complètement du payload plutôt que d'y envoyer une valeur sans sens.

La même forme « soumettre un ensemble d'éléments à la fois » est réutilisée par la création de ligne de grille ([`GC`]({{% relref "client-messages.fr.md" %}}#gc)) et sa mise à jour ([`GU`]({{% relref "client-messages.fr.md" %}}#gu)) — les deux prennent une map `e` exactement comme `SA`. Ça vaut le coup de construire **un seul** composant formulaire réutilisable qui serve à la fois le formulaire de propriété, la boîte de création de ligne de grille et celle d'édition de ligne, plutôt que trois implémentations séparées d'une logique essentiellement identique (« rendre des éléments éditables, collecter leurs valeurs »). C'est ce que fait Rooster.

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

## Exemple : lecture et écriture dans Rooster

Extraits réels illustrant la séparation lecture/écriture de tête de
cette page, une fois qu'un vrai client (ici Rooster, en Angular) est
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

