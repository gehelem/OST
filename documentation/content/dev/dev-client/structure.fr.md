---
title: Modèle de données
weight: 20
description: "Hiérarchie des modules, propriétés et éléments échangés via WebSocket"
---

## Vue d'ensemble

Toutes les données sont organisées selon une hiérarchie à trois niveaux :

```
Module
  └─ Property (une ou plusieurs par module)
       └─ Element (un ou plusieurs par propriété)
```

---

## Connexion et état initial

### Trouver le serveur

Un client **web** est servi *par* le serveur OST lui-même, il connaît donc déjà son hôte — l'URL WebSocket doit être dérivée de l'URL de la page, pas demandée à l'utilisateur :

| URL de la page | URL WebSocket |
|---|---|
| `http://<hôte>/` | `ws://<hôte>:9624` (direct, port fixe) |
| `https://<hôte>/` | `wss://<hôte>/ws/` (port 443 implicite, via un chemin proxy inverse — les installations SSL n'exposent pas directement le 9624) |

C'est une convention, pas quelque chose imposé par le protocole — mais tous les clients existants la suivent, et un client qui demande à l'utilisateur de saisir une URL WebSocket à la main résout un problème qui n'existe pas dans le cas web.

Un client **non-web** (mobile, desktop) n'a pas d'URL de page dont dériver quoi que ce soit, et a besoin que l'hôte lui soit fourni autrement :
- **mDNS/Zeroconf** : le serveur s'annonce sur le réseau local avec le type de service `_ostserver_ws._tcp` dans le domaine `local`, avec un enregistrement TXT `OstServer=Observatoire Sans Tete`. Le port annoncé est toujours `9624`. Cela permet à un client de **scanner les serveurs disponibles** sur le réseau plutôt que d'exiger que l'utilisateur connaisse une IP — utile pour une première connexion, ou dès qu'il peut y avoir plus d'un serveur OST à proximité.
- **Saisie manuelle, mémorisée** : qu'il soit découvert ou saisi, un client devrait garder un petit historique des serveurs déjà utilisés (hôte/port/sécurisé) pour que la reconnexion ultérieure ne nécessite pas de répéter la découverte ou la saisie. C'est un confort côté client, pas une partie du protocole.

Dans tous les cas, l'URL de base des **médias** (pour images/vidéos, voir la note « URLs media » plus bas) suit le même hôte, avec le port HTTP(S) standard (80/443) plutôt que 9624 — un client non-web a besoin de cet hôte aussi, exactement pour la même raison qu'il a besoin de l'hôte WebSocket.

À la connexion, envoyer une requête `DU` (voir [Client → Serveur]({{< relref "client-messages.fr.md" >}})).  
Le serveur répond avec un message dump complet (`d`) contenant tous les modules, les données du contrôleur et les listes de fichiers.

Les données propres au contrôleur (charge serveur, configurations de modules, drivers INDI, système de fichiers) ne font pas partie du modèle de modules décrit sur cette page — voir [Contrôleur, GlobalDatastore et fichiers]({{% relref "controller.fr.md" %}}).

---

## Modules

Un module représente un sous-système fonctionnel (mise au point, guidage, caméra allsky, etc.).  
Son identifiant est son nom interne (sans espaces).

| Champ | Type | Description |
|---|---|---|
| `infos.modulelabel` | string | Nom affiché du module |
| `infos.description` | string | Description du module |
| `infos.template` | string | Type de module (ex. `"focus"`, `"guider"`) — peut être absent |
| `properties` | object | Map nom de propriété → objet propriété |
| `globallovs` | object | Map clé LOV → objet LOV (listes de valeurs au niveau module) |
| `profile.name` | string | Nom du profil actif |
| `profile.changed` | bool | Indique si le profil a des modifications non sauvegardées |

{{% notice style="info" title="Format wire" %}}
Dans le format JSON sur le réseau, `properties` est abrégé en `p`, `globallovs` en `l`, et `profile` en `f`. Les clients doivent gérer ces clés abrégées.
{{% /notice %}}

{{% notice style="note" title="Champs internes" %}}
`infos` porte aussi plusieurs champs internes de build/version
(`baseGithash`, `thisversion`, `startdatetime`, et d'autres) — détail
d'implémentation, ne fait pas partie du contrat stable côté client,
pas documenté individuellement ici.
{{% /notice %}}

---

## Propriétés

Une propriété regroupe des éléments liés. Les propriétés sont affichées hiérarchiquement via `level1` et `level2`.

| Champ | Type | Description |
|---|---|---|
| `label` | string | Libellé affiché |
| `order` | string | Ordre de tri dans le même niveau |
| `level1` | string | Premier niveau hiérarchique (ex. nom d'onglet) |
| `level2` | string | Deuxième niveau hiérarchique (ex. nom de groupe) |
| `status` | int | `0` Veille · `1` OK (vert) · `2` Occupé (jaune) · `3` Erreur (rouge) |
| `permission` | int | `0` Lecture seule · `1` Écriture seule · `2` Lecture-écriture |
| `enabled` | bool | Indique si la propriété peut être interagie |
| `badge` | bool | Indique si la propriété est dans la liste des favoris |
| `preicon1` | string | Icône avant le libellé (nom d'icône Google Fonts) |
| `preicon2` | string | Deuxième icône avant le libellé |
| `posticon1` | string | Icône après le libellé |
| `posticon2` | string | Deuxième icône après le libellé |
| `showElts` | bool | Indique si les valeurs des éléments doivent être affichées en ligne |
| `hasprofile` | bool | Indique si la propriété est sauvegardée dans les profils |
| `freevalue` | string | Champ texte libre arbitraire |
| `rule` | int | Règle de groupement des éléments bool : `0` UnParmi (radio) · `1` AuPlusUn · `2` Quelconque |
| `elements` | object | Map nom d'élément → objet élément |

{{% notice style="tip" title="`preicon`/`posticon` : des actions cliquables, pas des décorations" %}}
Une valeur non vide de `preicon1`/`preicon2`/`posticon1`/`posticon2` est le nom d'icône Google Fonts (Material Icons) à afficher — et elle est cliquable : cliquer dessus envoie [`I1`/`I2`/`I3`/`I4`]({{% relref "client-messages.fr.md" %}}#i1) sans valeur ni paramètre, juste un payload vide ciblant la propriété. Une **chaîne vide signifie qu'il n'y a aucune icône** à cet emplacement — rien à afficher, rien à cliquer, rien à envoyer. Les `preicon`/`posticon` au niveau élément (plus bas) fonctionnent pareil, via [`J1`/`J2`]({{% relref "client-messages.fr.md" %}}#j1). Voir [Actions sur les icônes]({{% relref "client-messages.fr.md" %}}#icon-actions) pour la référence complète des messages.
{{% /notice %}}

{{% notice style="warning" title="`permission` régit les éléments éditables, pas l'affichage de la valeur" %}}
`permission: 1` (« écriture seule ») ne veut pas dire « masquer la valeur courante » — ça veut dire que la propriété n'a rien de significatif à relire comme entrée soumissible (une action à sens unique, sans retour). Une propriété dont le seul élément est d'un type non éditable (`img`, `video`, `light`, `prg`, `message`) peut légitimement être `permission: 1` — un aperçu caméra en direct en est un exemple réel — et doit quand même être restituée à partir de sa valeur, quelle que soit `permission`. Conditionner l'interactivité (afficher une saisie, activer une soumission) à `permission`, mais conditionner l'affichage au **type** de l'élément, pas à `permission`.
{{% /notice %}}

{{% notice style="note" title="`rule` n'a de sens que si tous les éléments sont `bool`" %}}
`rule` est présent sur chaque propriété (défaut `0`), mais il n'a de sens — et ne doit être pris en compte — que lorsque **tous** les éléments de la propriété sont de type `bool`. Il apparaît aussi, de façon inerte, sur des propriétés dont les éléments sont un mélange d'autres types (ex. une paire de coordonnées composée de deux éléments `float`) — un client doit l'ignorer dans ce cas plutôt que d'essayer de restituer, disons, deux floats comme un groupe de radios.
{{% /notice %}}

{{% notice style="note" title="`badge` est fixé par l'auteur du module, pas par un client à l'exécution" %}}
Aucun message client ne permet de basculer `badge` — il est fixé une fois pour toutes par propriété/par élément dans la définition même du module, ce n'est pas un favori par utilisateur, par session, qu'un client pourrait définir. Un écran « favoris » peut lire et agréger ce qui est déjà marqué, mais n'a aucun moyen de laisser un utilisateur marquer lui-même quelque chose comme favori.
{{% /notice %}}

{{% notice style="tip" title="`level1`/`level2`/`order` sont informatifs, pas un contrat" %}}
Ces champs sont purement informatifs — le backend ne les impose jamais, et un client est libre de les respecter partiellement ou pas du tout. Ils expriment seulement la façon dont le développeur d'un module a envisagé d'organiser ses propriétés. C'est précisément ce qui permet à des clients génériques (Rooster, ostnoctua) de s'adapter à n'importe quel module sans UI dédiée par module.

Le client de référence (Rooster) regroupe les propriétés ainsi, pour un client qui viserait une mise en page générique similaire :
1. `level1` vide → propriété affichée hors de tout onglet (rendue directement, triée par `order`).
2. `level1` renseigné → regroupée dans un onglet nommé `level1`. À l'intérieur, `level2` vide → affichée directement sous l'onglet ; `level2` renseigné → dans un groupe repliable nommé `level2`.
3. L'ordre des onglets (et des groupes à l'intérieur d'un onglet) n'est pas un champ séparé — il se déduit de la plus petite valeur `order` parmi les propriétés de ce groupe.
4. À l'intérieur d'un groupe, les propriétés sont triées par leur propre `order`.

Deux choses à connaître si vous construisez quelque chose de similaire :
- `level1`/`level2` forment une "fausse hiérarchie" par choix de conception, pas une structure à deux niveaux garantie — rien n'empêche un module de laisser `level1` vide tout en renseignant `level2` (Rooster ne gère même pas ce cas particulier : la propriété atterrit simplement dans le groupe "sans level1", `level2` étant ignoré).
- `level1`/`level2` sont des libellés traduits, passant par le même mécanisme que `label`/`hint`. Utiliser la valeur brute de la chaîne comme clé d'état d'interface (ex. un panneau replié/déplié) est fragile d'un changement de langue à l'autre côté client — pas un bug, juste à éviter.
{{% /notice %}}

**Propriétés avec grille** (quand `hasGrid: true`) :

| Champ | Type | Description |
|---|---|---|
| `hasGrid` | bool | La propriété contient des données tabulaires |
| `showGrid` | bool | La grille doit être affichée par défaut |
| `gridLimit` | int | Nombre maximum de lignes dans la grille |
| `gridheaders` | string[] | Ordre des colonnes (noms des éléments) |
| `grid` | tableau de tableaux | Lignes de la grille ; chaque ligne est un tableau de valeurs dans l'ordre de `gridheaders` |

**Propriétés avec graphe** (quand `hasGraph: true`) :

| Champ | Type | Description |
|---|---|---|
| `hasGraph` | bool | La propriété contient un graphe — **toujours présent**, même à `false` |
| `graphType` | string | `"XY"` · `"DY"` · `"SXY"` · `"SDY"` · `"PHD"` — voir plus bas |
| `graphParams` | object | Quel élément fournit quel rôle du graphe — voir plus bas |

Une propriété avec graphe est toujours aussi une propriété grille (`hasGrid: true`, voir ci-dessus) : chaque ligne de la grille est un point de données, et `graphParams` associe un rôle abstrait du graphe à la **clé d'élément** (l'un des `gridheaders`) dont la valeur, dans cette ligne, fournit ce rôle.

| `graphType` | Forme | Rôles de `graphParams` |
|---|---|---|
| `XY` | Une seule série en nuage de points, bornes d'axe fixes | `X`, `Y`, `Xmin`, `Xmax`, `Ymin`, `Ymax`, plus optionnellement `graphColor` (ci-dessous) |
| `DY` | Une seule série, date/heure en X | `D` (élément date/heure), `Y`, plus optionnellement `graphColor` (ci-dessous) |
| `SXY` | Plusieurs séries en nuage de points, regroupées par une clé de série | `S` (élément nom de série), `X`, `Y`, plus optionnellement `graphColors`/`axis`/`mapping` (ci-dessous) |
| `SDY` | Plusieurs séries, date/heure en X, regroupées par une clé de série | `S`, `D`, `Y`, plus optionnellement `graphColors`/`axis`/`mapping` |
| `PHD` | Graphe de guidage dédié façon PHD2 (dérive + SNR + RMS) | `D`, `RA`, `DE`, `pRA`, `pDE`, `RSB`, `RMS` — rôles fixes, spécifiques à cet usage |

Champs optionnels de `graphParams` :

| Champ | Utilisé par | Description |
|---|---|---|
| `graphColor` | `XY`, `DY` | `{"R": int, "G": int, "B": int}` — couleur de la série unique |
| `graphColors` | `SXY`, `SDY` | `{"<série>": {"R": int, "G": int, "B": int}}` — couleur par série |
| `axis` | `SXY`, `SDY` | `{"<nom>": {"min": number, "max": number, "pos": "left"\|"right", "text": string}}` — un ou plusieurs axes Y |
| `mapping` | `SXY`, `SDY` | `{"<série>": "<nom d'axe>"}` — sur quelle entrée de `axis` chaque série se trace |

{{% notice style="warning" title="`PHD` n'est pas un type de graphe générique" %}}
Contrairement à `XY`/`DY`/`SXY`/`SDY`, dont les rôles sont assez génériques pour piloter un seul composant de graphe réutilisable, `PHD` a des rôles fixes et spécifiques à un graphe de guidage façon PHD2 (utilisé par le module `guider`). Un client qui construit un composant de graphe générique pour les quatre autres types aura probablement besoin d'une implémentation dédiée pour `PHD`, ou devra le laisser de côté — c'est ce que fait actuellement Rooster (`SXY`/`SDY`/`XY`/`DY` seulement, `PHD` non géré).
{{% /notice %}}

{{% notice style="tip" title="Restituer les graphes comme des composants vivants, pas des images générées" %}}
Les données de grille d'une propriété graphe changent dans le temps (`gc`/`gu`/`gd`/`gr`, ou un `uc`/`ap`/`aa` frais) — à traiter comme n'importe quel autre élément d'UI vivant : le dessiner avec un composant graphique/canvas qui se redessine à chaque mise à jour, comme le fait Rooster avec Chart.js. Ne pas le transformer côté serveur ou côté client en image statique (capture PNG, graphe rendu serveur) pour ensuite juste échanger l'`<img>` — c'est visiblement moins bon qu'un vrai graphe (pas de mise à jour fluide, pas d'interactivité, scintillement/rechargement visible à chaque point), et ça jette tout l'intérêt d'avoir reçu les données de grille en direct sur le fil.
{{% /notice %}}

{{% notice style="info" title="Format wire" %}}
Dans le format JSON sur le réseau, `elements` est abrégé en `e`.
{{% /notice %}}

---

## Éléments

Un élément contient une valeur typée unique. Il existe 11 types d'éléments.

### Champs communs (tous les éléments, dans le dump complet)

| Champ | Type | Description |
|---|---|---|
| `type` | string | Type de l'élément (voir liste ci-dessous) |
| `label` | string | Libellé affiché |
| `order` | string | Ordre de tri au sein de la propriété |
| `hint` | string | Texte d'aide / infobulle |
| `autoupdate` | bool | Indicateur interne du backend ; ignoré par les clients |
| `badge` | bool | Indique si l'élément est dans la liste des favoris |
| `directedit` | bool | Indique si cet élément peut être écrit seul via `SV` — voir ci-dessous |
| `preicon` | string | Icône avant l'élément — action cliquable, voir la note ci-dessus (optionnel) |
| `posticon` | string | Icône après l'élément — action cliquable, voir la note ci-dessus (optionnel) |

{{% notice style="warning" title="Mises à jour partielles" %}}
Dans les messages de mise à jour `ee` et `ea`, les éléments ne contiennent **que leur valeur** — sans les champs de métadonnées. Les clients doivent conserver les métadonnées du dernier dump complet.
{{% /notice %}}

{{% notice style="tip" title="Omission de champ" %}}
`directedit` peut être totalement absent pour les éléments où il ne s'applique pas — considérer une absence comme `false`. Ça allège les dumps des propriétés qui n'en ont pas besoin.
{{% /notice %}}

{{% notice style="note" title="Signification de `directedit`" %}}
`true` signifie que cet élément peut être mis à jour seul, indépendamment des autres éléments de la propriété — envoyer `SV` pour lui (`SA` fonctionne aussi). `false` signifie que le client doit soumettre **tous** les éléments de la propriété ensemble — toujours utiliser `SA`, jamais `SV` pour cet élément seul. Exemple parlant : des coordonnées RA/Déc, où les deux valeurs doivent être données ensemble.

Le backend l'impose : un `SV` sur un élément à `directedit: false` est rejeté avec une erreur dans le log du module, et rien n'est appliqué. `SA` est toujours accepté.

Choisir la valeur selon le comportement de l'élément, pas selon qu'il « semble lié » à ses voisins :
- **`true` pour un élément indépendant** — un `bool` qui peut être vrai indépendamment de ses voisins (bouton d'action type start/abort/loop, interrupteur) doit avoir `directedit: true`. Une telle valeur reflète souvent une activité en cours, et plusieurs peuvent être vraies en même temps (ex. une boucle de capture en cours pendant qu'on demande un focus out). Le déclarer `false` prétendrait à tort que les éléments doivent être soumis en groupe, et un client qui suit cette page enverrait `SA`, contournant les modules qui ne traitent que `SV`.
- **`false` seulement pour un vrai groupe** — des éléments qui n'ont de sens qu'ensemble : coordonnées RA/Déc, ou un vecteur de switches INDI reproduit à l'identique.

Pour un groupe `false` d'éléments `bool` sous une règle exclusive (`OneOfMany` ou `AtMostOne`), sélectionner un élément revient à lui envoyer `true` et **tous ses voisins à `false`** dans le même `SA` — ne jamais renvoyer les valeurs courantes des voisins, sinon deux éléments finissent à `true`.
{{% /notice %}}

---

### Type : `int`

| Champ | Type | Description |
|---|---|---|
| `value` | int | Valeur entière courante |
| `min` | int | Valeur minimale autorisée |
| `max` | int | Valeur maximale autorisée |
| `step` | int | Pas d'incrément |
| `format` | string | Chaîne de formatage d'affichage |
| `slider` | int | `0` pas de slider · `1` slider seul · `2` slider + saisie de valeur |
| `listOfValues` | object | Optionnel : map `{"clé": "Libellé"}` des valeurs autorisées |
| `globallov` | string | Optionnel : clé référençant un LOV de module ou de contrôleur |
| `lovScope` | string | `"module"` ou `"controller"` — où est défini le globallov |
| `lovConstrained` | bool | Indique si la valeur doit appartenir au LOV |
| `nullable` | bool | Indique si l'élément peut être laissé vide (le frontend doit proposer un choix "aucun") |

### Type : `float`

Mêmes champs que `int`, avec `value`, `min`, `max`, `step` en virgule flottante.

### Type : `bool`

| Champ | Type | Description |
|---|---|---|
| `value` | bool | `true` ou `false` |

### Type : `string`

| Champ | Type | Description |
|---|---|---|
| `value` | string | Valeur texte |
| `listOfValues` | object | Optionnel : map `{"clé": "Libellé"}` des valeurs autorisées |
| `globallov` | string | Optionnel : clé référençant un LOV |
| `lovScope` | string | `"module"` ou `"controller"` |
| `lovConstrained` | bool | Indique si la valeur doit appartenir au LOV |
| `nullable` | bool | Indique si l'élément peut être laissé vide (le frontend doit proposer un choix "aucun") |

### Type : `date`

| Champ | Type | Description |
|---|---|---|
| `value` | object | `{"year": int, "month": int (1-12), "day": int (1-31)}` |

### Type : `time`

| Champ | Type | Description |
|---|---|---|
| `value` | object | `{"hh": int, "mm": int, "ss": int, "ms": int}` |
| `usems` | bool | Indique si les millisecondes doivent être affichées |

### Type : `datetime`

| Champ | Type | Description |
|---|---|---|
| `value` | object | `{"year": int, "month": int, "day": int, "hh": int, "mm": int, "ss": int, "ms": int}` |

### Type : `img`

Le champ `value` est un objet contenant les métadonnées de l'image :

| Champ | Type | Description |
|---|---|---|
| `urljpeg` | string | Chemin relatif vers l'image JPEG (à utiliser avec l'URL de base `/ostmedia/`) |
| `urlfits` | string | Chemin relatif vers le fichier FITS |
| `urlthumbnail` | string | Chemin relatif vers la vignette |
| `urloverlay` | string | Chemin relatif vers l'image de superposition |
| `channels` | int | Nombre de canaux de couleur |
| `width` / `height` | int | Dimensions de l'image en pixels |
| `snr` | float | Rapport signal sur bruit |
| `hfravg` | float | HFR moyen (rayon à mi-flux) des étoiles détectées |
| `hfravgdev` | float | Écart-type de `hfravg` |
| `stars` | int | Nombre d'étoiles détectées |
| `issolved` | bool | Indique si la résolution de champ a réussi |
| `solverra` / `solverde` | float | Coordonnées AR/Déc résolues |
| `solverorientation` | float | Angle de rotation du champ résolu |
| `min` / `max` / `mean` / `median` / `stddev` | float[] | Statistiques par canal (voir détail ci-dessous) |
| `histogram` | array | Données d'histogramme par canal (voir détail ci-dessous) |
| `alternates` | string[] | Optionnel : versions alternatives de l'image |
| `showstats` | bool | Fixé par le développeur du module : indique si les champs de statistiques ci-dessus ont un sens pour cette image et doivent être affichés (un module peut le mettre à `false` s'il ne les calcule pas, ou s'ils ne s'appliquent pas) |

**Statistiques par canal** : `min`/`max`/`mean`/`median`/`stddev` sont
des tableaux à **3 emplacements fixes**, quel que soit le nombre réel
de canaux — index `0` = canal unique (image mono) ou rouge (R), `1` =
vert (G), `2` = bleu (B). Seuls les `channels` premiers emplacements
sont significatifs ; au-delà, les valeurs sont à zéro et sans
signification (une image mono n'a que l'index `0` de renseigné).

{{% notice style="note" title="Histogramme : structure des bins" %}}
`histogram` suit la même logique de canaux (un tableau par canal,
seuls les `channels` premiers utilisés), mais avec deux particularités
à connaître avant de l'exploiter :

- **Chaque bin est lui-même encapsulé dans un tableau à un élément** :
  `histogram[canal][bin]` vaut `[fréquence]`, pas directement
  `fréquence` — un artefact de la sérialisation actuelle côté
  backend, pas une erreur de lecture côté client.
- **La valeur de pixel que représente chaque bin n'est pas transmise
  directement**, seule la fréquence l'est. Le nombre de bins varie
  d'une image à l'autre (`histogram[canal].length`) ; la valeur de
  pixel du bin `i` se recalcule à partir de `min`/`max` du même canal :
  ```
  largeur_bin = (max[canal] - min[canal]) / (nombre_de_bins - 1)
  valeur(i)   = min[canal] + i × largeur_bin
  ```
{{% /notice %}}

{{% notice style="tip" title="URLs media" %}}
Tous les chemins image/vidéo sont relatifs. Préfixer avec `http(s)://hostname/ostmedia/` pour obtenir l'URL complète.  
Exemple : `"urljpeg": "Focus/frame.jpeg"` → `http://hostname/ostmedia/Focus/frame.jpeg`
{{% /notice %}}

{{% notice style="warning" title="Les médias sont servis séparément du port WebSocket" %}}
Ce `hostname` n'est **pas déductible du protocole** — aucun champ du dump ne le donne, et ce n'est **pas** l'hôte de la connexion WebSocket que vous venez d'établir. Dans le déploiement de référence (voir [Installation propre]({{% relref "fresh.fr.md" %}})), les médias sont servis par un serveur web séparé (nginx, port 80 par défaut, 443 derrière SSL) — un port complètement différent du serveur WebSocket (9624). Un client doit déjà connaître, ou faire configurer par l'utilisateur, le hostname du serveur OST indépendamment de l'URL WebSocket, et construire les URLs media à partir du port HTTP de ce hostname — pas de l'origine de ses propres fichiers, ni du port WebSocket. Un client qui se contente de préfixer un chemin `/ostmedia/` nu (résolu par rapport à l'endroit où ses propres fichiers sont servis) obtiendra des images cassées.
{{% /notice %}}

### Type : `video`

| Champ | Type | Description |
|---|---|---|
| `value` | object | `{"url": "chemin/relatif/vers/video.mp4"}` |

Dans les messages de mise à jour partielle (`ea`), l'élément vidéo peut contenir `url` directement au niveau de l'élément plutôt qu'imbriqué dans `value`.

### Type : `light`

| Champ | Type | Description |
|---|---|---|
| `value` | int | `0` Veille · `1` OK · `2` Avertissement · `3` Erreur |

### Type : `prg`

| Champ | Type | Description |
|---|---|---|
| `value` | object | `{"value": float (0–100), "dynlabel": string}` |
| `prgtype` | string | `"bar"` ou `"spinner"` |

### Type : `message` (non encore implémenté dans le backend actuel)

Réservé pour l'affichage de messages/logs.

---

## Listes de valeurs (LOV)

Les LOV fournissent un ensemble de valeurs autorisées pour les éléments de type `int`, `float` et `string`.

**LOV locale** — intégrée dans l'élément :
```json
"listOfValues": {
  "1": "Option Un",
  "2": "Option Deux"
}
```

**LOV globale** — référencée par clé :
```json
"globallov": "myCameraModels",
"lovScope": "module"
```

Le LOV lui-même est défini au niveau module (`globallovs`) ou contrôleur (`lovs`) :
```json
"myCameraModels": {
  "label": "Modèles de caméra",
  "type": "string",
  "values": {
    "ASI294MC": "ZWO ASI 294 MC",
    "ASI533MC": "ZWO ASI 533 MC"
  }
}
```

**LOV intégrés du contrôleur** (toujours disponibles) :

| Clé | Contenu |
|---|---|
| `loadedModules` | Tous les modules chargés : `{nomModule: libellé}` |
| `loadedModules-<template>` | Modules filtrés par type (ex. `loadedModules-focus`) |
| `profiles-<template>` | Profils disponibles pour un type de module |

---

## Contrôle d'accès

La réponse dump contient les valeurs `grant-server` et `grant-client` :

| Valeur | Signification |
|---|---|
| `"1"` | Accès complet lecture-écriture |
| `"0"` | Lecture seule |
| `"-1"` | Accès refusé (authentification requise) |

Si `grant-client` vaut `"-1"`, aucune donnée de module n'est envoyée. Utiliser le message `LO` (login) pour s'authentifier.
