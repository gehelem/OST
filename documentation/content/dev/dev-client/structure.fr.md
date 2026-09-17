---
title: Modèle de données
weight: 10
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

À la connexion, envoyer une requête `DU` (voir [Client → Serveur]({{< relref "client-messages.fr.md" >}})).  
Le serveur répond avec un message dump complet (`d`) contenant tous les modules, les données du contrôleur et les listes de fichiers.

---

## Modules

Un module représente un sous-système fonctionnel (mise au point, guidage, caméra allsky, etc.).  
Son identifiant est son nom interne (sans espaces).

| Champ | Type | Description |
|---|---|---|
| `infos.label` | string | Nom affiché du module |
| `infos.name` | string | Nom interne du module |
| `infos.description` | string | Description du module |
| `infos.template` | string | Type de module (ex. `"focus"`, `"guider"`) |
| `properties` | object | Map nom de propriété → objet propriété |
| `globallovs` | object | Map clé LOV → objet LOV (listes de valeurs au niveau module) |
| `profile.name` | string | Nom du profil actif |
| `profile.changed` | bool | Indique si le profil a des modifications non sauvegardées |

{{% notice style="info" title="Format wire" %}}
Dans le format JSON sur le réseau, `properties` est abrégé en `p`, `globallovs` en `l`, et `profile` en `f`. Les clients doivent gérer ces clés abrégées.
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
| `hasGraph` | bool | La propriété contient un graphe |
| `graphType` | string | `"XY"` · `"DY"` (axe temporel) · `"PHD"` (guidage) |
| `graphParams` | object | Configuration du graphe |

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
| `directedit` | bool | `true` = envoyer `SV` à chaque modification ; `false` = grouper avec `SA` |
| `preicon` | string | Icône avant l'élément (optionnel) |
| `posticon` | string | Icône après l'élément (optionnel) |

{{% notice style="warning" title="Mises à jour partielles" %}}
Dans les messages de mise à jour `ee` et `ea`, les éléments ne contiennent **que leur valeur** — sans les champs de métadonnées. Les clients doivent conserver les métadonnées du dernier dump complet.
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
| `stars` | int | Nombre d'étoiles détectées |
| `issolved` | bool | Indique si la résolution de champ a réussi |
| `solverra` / `solverde` | float | Coordonnées AR/Déc résolues |
| `solverorientation` | float | Angle de rotation du champ résolu |
| `min` / `max` / `mean` / `median` / `stddev` | float[] | Statistiques par canal |
| `histogram` | array | Données d'histogramme par canal |
| `alternates` | string[] | Optionnel : versions alternatives de l'image |

{{% notice style="tip" title="URLs media" %}}
Tous les chemins image/vidéo sont relatifs. Préfixer avec `http(s)://hostname/ostmedia/` pour obtenir l'URL complète.  
Exemple : `"urljpeg": "Focus/frame.jpeg"` → `http://hostname/ostmedia/Focus/frame.jpeg`
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

Le LOV lui-même est défini au niveau module (`globallovs`) ou contrôleur (`controllerlovs`) :
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
