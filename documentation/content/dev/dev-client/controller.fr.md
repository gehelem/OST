---
title: Contrôleur, GlobalDatastore et fichiers
weight: 30
description: "Ce qui n'est pas un module ordinaire : données du contrôleur, charge système, configurations, module GlobalDatastore, système de fichiers"
---

Cette page décrit ce qui sort du schéma « modules → propriétés → éléments » des pages précédentes : les **données du contrôleur**, le module central **`GlobalDatastore`** et le **système de fichiers** du serveur. Un client qui se limite à afficher les modules n'en a pas besoin ; un client qui veut offrir ce que propose une page de paramètres globaux (configurations, drivers INDI, charge du serveur, équipement, images) doit les traiter.

## Les données du contrôleur (`controllerdata`)

Le contrôleur est le composant du serveur qui charge les modules et pilote le serveur INDI embarqué. Il expose ses propres données dans le dump, sous la clé `controllerdata` (voir [`d`]({{% relref "server-messages.fr.md" %}}#d)), puis les tient à jour avec des messages [`uc`]({{% relref "server-messages.fr.md" %}}#uc).

| Clé | Contenu |
|---|---|
| `availableconfs` | Configurations de modules sauvegardées : objet `nom de configuration → liste de {modulelabel, moduletype, profilename}` |
| `currentconf` | Nom de la configuration courante |
| `banner` | Bannière du serveur (texte libre, option `--banner`) |
| `git` | Version du serveur : `Gitdate`, `Githash`, `Gitmessage`, `Gittag` |
| `libraries` | Bibliothèques de modules disponibles, un objet de métadonnées par bibliothèque |
| `profiles` | Profils disponibles : objet `nom de bibliothèque transformé → liste de noms de profils` — voir la convention de clé ci-dessous |
| `indiserver` | `"Y"` si le serveur INDI embarqué est activé, `"N"` sinon |
| `indidrivers` | Catalogue des drivers INDI installés : liste de `{binary, family, label, mdpd}` |
| `indiactivedrivers` | Drivers INDI actuellement lancés |
| `systemwatcher` | Charge du serveur, voir plus bas |
| `files`, `folders` | Copie de `d.files.files` et `d.files.folders`, voir [Système de fichiers](#fichiers) |

{{% notice style="warning" title="Clé de `profiles` : ni le nom de bibliothèque tel quel, ni `infos.template`" %}}
`profiles` est indexé par le **nom de bibliothèque** du module (la même chaîne passée comme `lib` à [`ML`]({{% relref "client-messages.fr.md" %}}#ml), c'est-à-dire la clé de `controllerdata.libraries` avec le préfixe `libost` retiré) — mais transformée : **première lettre en majuscule, le reste en minuscules** (`"focus"` → `"Focus"`, pas une simple capitalisation — un nom de bibliothèque contenant une autre majuscule serait quand même aplati, ex. un hypothétique `"myFocus"` → `"Myfocus"`). Cette clé n'est ni le nom d'instance/libellé du module, ni forcément identique à `infos.template` (les deux coïncident généralement en pratique, mais sont réglés indépendamment et rien dans le protocole ne le garantit).

Cette convention n'est documentée nulle part ailleurs et n'est pas imposée par le backend — elle a été rétro-ingénierée depuis la logique de recherche de Rooster lui-même. Deux clients indépendants construits uniquement à partir de cette doc s'y sont fait piéger (un sélecteur de profil se rendant vide en silence). Si tu as besoin de retrouver les profils disponibles d'un module, reproduis exactement cette transformation sur le nom de bibliothèque plutôt que de deviner.
{{% /notice %}}

{{% notice style="warning" title="`uc` : une clé par message, valeur remplacée en bloc" %}}
Un message `uc` porte **une seule clé** de `controllerdata`, et sa valeur remplace **entièrement** l'ancienne (pas de fusion). Un client qui ne lit que la première clé du payload (comme le fait Rooster) est donc correct tant que le serveur n'envoie qu'une clé par message, ce qui est le cas aujourd'hui.
{{% /notice %}}

### `systemwatcher` : charge du serveur

Le serveur mesure sa propre charge à intervalle régulier et la diffuse à tous les clients. L'intervalle se règle avec l'option `--systemwatchinterval` (secondes, **10 par défaut**, `0` désactive). Le message est un `uc` ordinaire, et la dernière valeur est aussi présente dans `controllerdata.systemwatcher` du dump.

```json
{
  "uc-update controller data": {
    "systemwatcher": {
      "cpu": { "load_1m": 10.42, "load_5m": 9.79, "load_15m": 4.51 },
      "ram": { "used_mb": 12194, "total_mb": 31941 },
      "disks": [
        { "device": "/dev/nvme0n1p2", "mountpoint": "/", "used_gb": 789.4, "total_gb": 915.3 }
      ],
      "network": [
        { "iface": "enp4s0", "rx_kbps": 16.27, "tx_kbps": 448.8 }
      ],
      "temperatures": [
        { "type": "Tctl", "temp_c": 75.375 }
      ]
    }
  }
}
```

| Champ | Signification |
|---|---|
| `cpu.load_1m` / `load_5m` / `load_15m` | **Charge moyenne** du système (comme `/proc/loadavg`), **pas un pourcentage d'utilisation** : 1.0 = un cœur saturé en moyenne |
| `ram.used_mb` / `total_mb` | Mémoire utilisée (total − disponible) et totale, en Mo |
| `disks[]` | Un élément par périphérique de stockage monté (`/dev/*`, hors `loop`) : `device`, `mountpoint`, `used_gb`, `total_gb` |
| `network[]` | Un élément par interface (hors `lo`) : débits `rx_kbps` / `tx_kbps` mesurés entre deux messages ; la première mesure d'une interface vaut 0 |
| `temperatures[]` | Capteurs matériels du processeur : `type`, `temp_c`. **La liste peut être vide** selon la machine (seuls les capteurs de type Intel `coretemp` et AMD `k10temp` sont lus) |

{{% notice style="tip" title="Ne pas supposer que les tableaux sont remplis" %}}
`disks`, `network` et surtout `temperatures` peuvent être vides. Un client doit masquer la section correspondante plutôt que d'afficher un tableau vide. Le protocole ne donne pas le nombre de cœurs du serveur : une jauge de CPU ne peut donc pas être un vrai pourcentage, il faut afficher la valeur comme une charge.
{{% /notice %}}

## Configurations et cycle de vie des modules

Une **configuration** est un ensemble nommé de modules (bibliothèque, libellé, profil) que le serveur peut recharger d'un coup. La liste est dans `controllerdata.availableconfs`, la configuration active dans `controllerdata.currentconf`.

| Action | Commande | Détail |
|---|---|---|
| Charger une configuration | [`CL`]({{% relref "client-messages.fr.md" %}}#cl) | Charge tous les modules de la configuration |
| Sauvegarder la configuration courante | [`CS`]({{% relref "client-messages.fr.md" %}}#cs) | Sous le nom donné |
| Charger un module | [`ML`]({{% relref "client-messages.fr.md" %}}#ml) | Bibliothèque, libellé, profil optionnel |
| Décharger un module | [`MK`]({{% relref "client-messages.fr.md" %}}#mk) | Par nom de module |

Les modules effectivement chargés sont ceux du dump (`d.m`) ; ils apparaissent et disparaissent par [`aa`]({{% relref "server-messages.fr.md" %}}#aa) et [`dm`]({{% relref "server-messages.fr.md" %}}#dm). La configuration ne contient **jamais** `GlobalDatastore` (voir ci-dessous), qui est toujours présent.

### Serveur INDI et drivers

Si `controllerdata.indiserver` vaut `"Y"`, le serveur pilote une instance INDI embarquée. Les commandes [`YA`/`YZ`/`YL`/`YR`/`YS`]({{% relref "client-messages.fr.md" %}}#indi) la démarrent, l'arrêtent et gèrent les drivers ; `controllerdata.indidrivers` donne le catalogue disponible et `controllerdata.indiactivedrivers` ce qui tourne. `binary` est la valeur à passer comme `driver` dans `YL`, `YR` et `YS`.

## Le module `GlobalDatastore` {#globaldatastore}

`GlobalDatastore` est un module **comme les autres dans le dump** (`d.m.GlobalDatastore`, avec propriétés, éléments, grilles, profils), mais son rôle est central :

- il est créé par le serveur **avant tous les autres modules** et **toujours présent** : `MK` sur lui est refusé, et il n'est jamais inclus dans une configuration ;
- il porte le **référentiel partagé de l'équipement**, que tous les autres modules consultent : optiques, emplacements, équipements (jeux d'appareils) et serveurs INDI.

Ses grilles servent de source à des listes de choix (LOV) du même nom, présentes dans `d.lovs` (`optics`, `locations`, `equipments`, `servers`) et reconstruites à chaque modification de la grille. Les autres modules s'en servent ainsi : choisir une optique remplit la focale et le diamètre, choisir un emplacement remplit latitude, longitude et altitude, choisir un jeu d'équipements résout les appareils (caméra, monture, focuser, roue à filtres, caméra de guidage, GPS, dôme, météo) et le serveur INDI à contacter.

Propriétés exposées :

| Propriété | Rôle |
|---|---|
| `devices` | Jeu d'équipements et GPS sélectionnés (`equipments`, `gps`) |
| `devicesactions` | Boutons : connecter/déconnecter les appareils, charger les configurations (`condevs`, `discondevs`, `loadconfs`) |
| `server` | Hôte et port du serveur INDI (`host`, `port`) |
| `serveractions` | Boutons : connecter/déconnecter le serveur INDI (`conserv`, `disconserv`) |
| `startup` | Cases à cocher de démarrage automatique (`indiatstart`, `devatstart`, `confsatstart`) |
| `equipments` | Grille des jeux d'équipements |
| `locations` | Grille des emplacements (icônes `refresh` = lire le GPS, `add` = ajouter la ligne courante) |
| `optics` | Grille des optiques |
| `servers` | Grille des serveurs INDI |

Toutes ses propriétés se manipulent avec les messages standard (`SV`, `SA`, `GC`…, `I3`/`I4` sur `locations`) ; un client générique les affiche comme n'importe quel module. Rooster les présente dans un onglet dédié de sa page de paramètres, en retenant les propriétés qui ont un profil (`hasprofile`) et en les regroupant par `level1`/`level2`.

## Système de fichiers {#fichiers}

Le serveur publie la liste des fichiers produits (images, séquences, archives) dans le dossier `--webroot`, sans que le client ait à les interroger. Ce mécanisme est indépendant des modules.

### Dans le dump : `d.files`

```json
"files": {
  "folders": ["", "/Allsky", "/Sequencer/gam Cyg/LIGHT/Luminance"],
  "files": ["/image1.jpeg", "/rapport.txt"],
  "selectedfolder": "/home/gilles/ostmedia"
}
```

| Champ | Contenu |
|---|---|
| `folders` | **Tous** les sous-dossiers du webroot, récursivement, chemins relatifs au webroot commençant par `/`. La racine est la chaîne vide `""`. Non trié. |
| `files` | Les fichiers **du seul dossier sélectionné** (pas récursif), chemins relatifs au webroot commençant par `/`. Tous les fichiers, sans filtre d'extension. |
| `selectedfolder` | Le dossier sélectionné, en **chemin absolu du système de fichiers du serveur** (et non relatif). |

Il n'y a ni pagination ni limite de taille : sur un webroot très rempli, la liste peut être longue.

### Choisir le dossier : `FS`

[`FS`]({{% relref "client-messages.fr.md" %}}#fs) désigne le dossier dont on veut la liste de fichiers. Le serveur ne surveille qu'**un seul** dossier à la fois.

{{% notice style="warning" title="Le dossier sélectionné est global au serveur" %}}
Il n'y a pas de dossier par client : un `FS` change la liste de fichiers vue par **tous** les clients connectés (les événements ci-dessous sont diffusés à tous). Un client ne doit pas s'en servir pour une simple navigation « locale » si d'autres clients sont susceptibles d'être connectés.

Le serveur **ne valide pas** le chemin : un dossier inexistant renvoie simplement une liste vide, sans erreur.
{{% /notice %}}

### Mises à jour : `af` / `df` / `ad` / `dd`

Quand le contenu du dossier sélectionné change (fichier créé, supprimé ou renommé), ou après un `FS`, le serveur envoie un message par chemin concerné — la valeur est le chemin brut lui-même, sans être enveloppé dans un objet ou un tableau :

```json
{ "af": "/Sequencer/gam Cyg/LIGHT/Luminance/img_001.FITS" }
{ "df": "/rapport.txt" }
```

| Clé | Signification |
|---|---|
| `af` | Fichier apparu dans le dossier sélectionné |
| `df` | Fichier disparu du dossier sélectionné |
| `ad` | Sous-dossier créé dans le webroot |
| `dd` | Sous-dossier supprimé du webroot |

Chaque message porte exactement un chemin (N fichiers = N messages) — voir [`af`/`df`/`ad`/`dd`]({{% relref "server-messages.fr.md" %}}#af) pour la référence complète. Après un `FS`, le client reçoit un `df` pour chaque fichier de l'ancien dossier puis un `af` pour chaque fichier du nouveau : il n'y a **pas** de message annonçant le nouveau `selectedfolder`, qui n'existe que dans le dump.

Le serveur envoie en plus, à chaque passage, des messages [`uc`]({{% relref "server-messages.fr.md" %}}#uc) `files` et `folders` contenant les listes **complètes** : un client peut donc soit appliquer les événements un par un, soit simplement remplacer ses listes avec ces `uc`.

### Construire l'URL d'un fichier

Les chemins de `files` sont relatifs au webroot et commencent par `/`. L'URL d'un fichier est l'URL de base des médias du serveur (`http(s)://hostname/ostmedia/`, voir la note « URLs media » du [Modèle de données]({{% relref "structure.fr.md" %}})) suivie du chemin. Attention au `/` initial : le concaténer à une base qui se termine déjà par `/` donne un double `//`.

### Ce que ne fait pas l'événement

Le serveur détecte l'**ajout, la suppression et le renommage** de fichiers, pas la modification du contenu d'un fichier existant : une image réécrite sous le même nom ne produit aucun événement. C'est le cas des images de prévisualisation des modules, dont l'URL reste identique à chaque prise de vue : un client doit ajouter un paramètre de cache-busting (`?t=` + horodatage) quand il recharge une image après une mise à jour de sa propriété.
