---
title: Messages Client → Serveur
weight: 50
description: "Tous les messages JSON que les clients peuvent envoyer au serveur OST"
---

## Format des messages

Chaque message est un objet JSON avec **une seule clé de premier niveau** identifiant la commande. La valeur est un objet dont la structure dépend de la commande.

Toutes les clés de commandes client sont en **MAJUSCULES** (ex. `DU`, `SV`, `SA`, `GC`).

---

## Index des commandes

| Clé | Description |
|---|---|
| [`DU`](#du) | Demander le dump complet |
| [`XX`](#xx) | Heartbeat / keepalive |
| [`LO`](#lo) | Connexion |
| [`IL`](#il) | Changer la langue |
| [`SV`](#sv) | Envoyer la valeur d'un seul élément |
| [`SA`](#sa) | Envoyer toutes les valeurs d'éléments |
| [`DP`](#dp) | Demander le dump d'une propriété |
| [`I1`](#i1) | Clic sur preicon1 de propriété |
| [`I2`](#i2) | Clic sur preicon2 de propriété |
| [`I3`](#i3) | Clic sur posticon1 de propriété |
| [`I4`](#i4) | Clic sur posticon2 de propriété |
| [`J1`](#j1) | Clic sur preicon d'élément |
| [`J2`](#j2) | Clic sur posticon d'élément |
| [`PL`](#pl) | Charger un profil |
| [`PS`](#ps) | Sauvegarder un profil |
| [`GC`](#gc) | Créer une ligne de grille |
| [`GU`](#gu) | Mettre à jour une ligne de grille |
| [`GD`](#gd) | Supprimer une ligne de grille |
| [`GF`](#gf) | Sélectionner une ligne de grille |
| [`GH`](#gh) | Monter une ligne de grille |
| [`GB`](#gb) | Descendre une ligne de grille |
| [`GR`](#gr) | Réinitialiser (vider) la grille |
| [`CL`](#cl) | Charger une configuration de modules |
| [`CS`](#cs) | Sauvegarder la configuration courante |
| [`FS`](#fs) | Choisir le dossier de fichiers surveillé |
| [`ML`](#ml) | Charger un module |
| [`MK`](#mk) | Décharger un module |
| [`YA`](#indi) | Démarrer le serveur INDI embarqué |
| [`YZ`](#indi) | Arrêter le serveur INDI embarqué |
| [`YL`](#indi) | Charger un driver INDI |
| [`YR`](#indi) | Recharger un driver INDI |
| [`YS`](#indi) | Arrêter un driver INDI |

---

Tous les messages ciblant un module ou une propriété utilisent l'enveloppe suivante :

```json
{
  "COMMANDE": {
    "m": {
      "NomDuModule": {
        "p": {
          "nomDeLaPropriété": { ... }
        }
      }
    }
  }
}
```

où `m` désigne *modules* et `p` désigne *properties*.

---

## Gestion de session

### `DU` — Demander le dump complet {#du}

Doit être envoyé immédiatement après la connexion. Le serveur répond avec un message `d` contenant tous les modules.

```json
{ "DU": { "language": "fr" } }
```

| Champ | Description |
|---|---|
| `language` | Langue préférée pour les libellés traduits (`"en"`, `"fr"`, etc.) |

---

### `XX` — Heartbeat {#xx}

À envoyer toutes les 30 secondes pour maintenir la connexion active. Le serveur répond avec `xx`.

```json
{ "XX": {} }
```

---

### `LO` — Connexion {#lo}

Requis lorsque le serveur a le contrôle d'accès activé (`grant-client` vaut `"-1"` dans la réponse dump).

```json
{
  "LO": {
    "user": "username",
    "pw": "password",
    "language": "fr"
  }
}
```

---

### `IL` — Changer la langue {#il}

Modifie la langue des libellés traduits sans se reconnecter.

```json
{ "IL": { "language": "fr" } }
```

---

## Mise à jour des valeurs de propriété

### `SV` — Envoyer la valeur d'un seul élément {#sv}

À utiliser lorsque `directedit` est `true` sur l'élément — envoie immédiatement la valeur d'un seul élément. Sur un élément à `directedit: false`, le backend le rejette (erreur dans le log du module, rien n'est appliqué) : utiliser `SA` à la place.

```json
{
  "SV": {
    "m": {
      "Focus": {
        "p": {
          "parameters": {
            "e": {
              "iterations": 7
            }
          }
        }
      }
    }
  }
}
```

`e` contient une seule paire `nomÉlément: valeur`.

---

### `SA` — Envoyer toutes les valeurs d'éléments {#sa}

Utilisé pour soumettre tous les éléments éditables d'une propriété en une fois (ex. depuis un formulaire). Également utilisé lorsque `directedit` est `false`.

```json
{
  "SA": {
    "m": {
      "Focus": {
        "p": {
          "parameters": {
            "e": {
              "iterations": 7,
              "exposure": 3.5,
              "active": true
            }
          }
        }
      }
    }
  }
}
```

{{% notice style="warning" title="Toujours envoyer tous les éléments" %}}
Un `SA` doit contenir tous les éléments éditables de la propriété. Si un seul manque, le backend n'applique rien du tout — pas même les éléments présents — et journalise un avertissement côté serveur (visible côté client via un message `l`). Ce n'est pas un cas à gérer avec une dégradation gracieuse : c'est le signe d'un bug dans le client, qui doit toujours envoyer l'ensemble complet des éléments.
{{% /notice %}}

---

### `DP` — Demander le dump d'une propriété {#dp}

Demande au serveur de renvoyer l'état complet d'une propriété, comme si elle venait d'être (re)chargée — utile pour forcer le rafraîchissement d'une propriété qu'un client soupçonne d'être désynchronisée, sans recharger tout le module. Le serveur répond par un message [`ap`]({{% relref "server-messages.fr.md" %}}#ap) pour cette seule propriété.

```json
{
  "DP": {
    "m": {
      "Focus": {
        "p": {
          "parameters": {}
        }
      }
    }
  }
}
```

Pas de champ `e` : `DP` cible la propriété entière, pas un élément particulier.

---

## Actions sur les icônes {#icon-actions}

`preicon1`/`preicon2`/`posticon1`/`posticon2` (propriétés) et `preicon`/`posticon` (éléments, voir [Modèle de données]({{% relref "structure.fr.md" %}})) ne sont pas des décorations : chacune est une **action cliquable**. Une valeur non vide est le nom de l'icône Google Fonts (Material Icons) à afficher ; cliquer sur l'icône affichée envoie la commande correspondante ci-dessous **sans aucune valeur ni paramètre** — un payload vide qui cible simplement la propriété (ou l'élément). Ce que fait réellement l'action côté serveur (sauvegarder un profil, charger un profil, lancer un processus...) dépend entièrement du module ; le seul rôle du client est d'afficher l'icône et de transmettre le clic tel quel.

Une **chaîne vide signifie qu'il n'y a aucune icône dans cet emplacement** — rien à afficher, et donc rien à cliquer ni à envoyer. Ne pas afficher de substitut ni brancher de gestionnaire de clic pour un champ icône vide.

### `I1` — Clic sur preicon1 de propriété {#i1}

```json
{
  "I1": {
    "m": {
      "Focus": {
        "p": {
          "saveprofile": {}
        }
      }
    }
  }
}
```

### `I2` — Clic sur preicon2 de propriété {#i2}

```json
{
  "I2": {
    "m": {
      "Focus": {
        "p": {
          "loadprofile": {}
        }
      }
    }
  }
}
```

### `I3` — Clic sur posticon1 de propriété {#i3}

Même structure que `I1` / `I2`.

### `I4` — Clic sur posticon2 de propriété {#i4}

Même structure que `I1` / `I2`.

---

### `J1` — Clic sur preicon d'élément {#j1}

```json
{
  "J1": {
    "m": {
      "Focus": {
        "p": {
          "devices": {
            "e": {
              "myDevice": {}
            }
          }
        }
      }
    }
  }
}
```

### `J2` — Clic sur posticon d'élément {#j2}

Même structure que `J1`.

---

## Gestion des profils

### `PL` — Charger un profil {#pl}

```json
{
  "PL": {
    "m": {
      "Focus": {
        "profile": "myProfile"
      }
    }
  }
}
```

Le serveur répond avec un message `fl` (profil chargé) puis envoie les valeurs de propriétés mises à jour.

{{% notice style="note" title="Un `fc` transitoire précède `fl`" %}}
Le serveur envoie un message [`fc`]({{% relref "server-messages.fr.md" %}}#fs) (modifications non sauvegardées) juste avant `fl`, comme étape intermédiaire du chargement du profil, avant que les valeurs des propriétés ne changent réellement. Un client qui réagit à `fc` en affichant un indicateur « modifications non sauvegardées » le verra clignoter le temps d'un message et se corriger de lui-même dès que `fl` arrive juste après — c'est attendu, pas un bug.
{{% /notice %}}

---

### `PS` — Sauvegarder un profil {#ps}

```json
{
  "PS": {
    "m": {
      "Focus": {
        "profile": "myProfile"
      }
    }
  }
}
```

Le serveur répond avec un message `fs` (profil sauvegardé).

---

## Opérations sur les grilles

Les commandes de grille ciblent une propriété spécifique au sein d'un module. Toutes utilisent l'enveloppe `m` / `p`.

### `GC` — Créer une ligne de grille {#gc}

Ajoute une nouvelle ligne à la grille avec les valeurs d'éléments fournies.

```json
{
  "GC": {
    "m": {
      "Sequencer": {
        "p": {
          "sequence": {
            "e": {
              "target": "M31",
              "exposure": 120,
              "count": 10
            }
          }
        }
      }
    }
  }
}
```

Le serveur répond avec un message `gc` confirmant l'index de la nouvelle ligne.

{{% notice style="note" title="Applique chaque `ea` tel qu'il arrive — rien à se demander" %}}
Créer une ligne de grille applique d'abord les valeurs soumises aux éléments de base de la propriété, puis ajoute la ligne et émet `gc`. Ça peut prendre plusieurs messages `ea` (un par élément de base modifié, envoyés l'un après l'autre — soumettre `target`/`exposure`/`count` comme ci-dessus produit trois messages `ea` distincts avant le `gc` final), mais un client n'a jamais besoin de connaître ou de prévoir ce nombre : le serveur n'envoie jamais que le strict nécessaire pour garder le datastore du client exact, ni plus ni moins (voir [Principes d'architecture client]({{% relref "client-pattern.fr.md" %}})). Applique chaque message tel qu'il se présente, comme d'habitude, et le résultat est correct quel que soit le nombre de messages reçus.

Le seul effet visible : une propriété qui affiche à la fois ses éléments de base et sa grille (`showElts` et `showGrid` tous les deux actifs — combinaison rare) clignotera à travers ces états intermédiaires avant que la nouvelle ligne apparaisse. C'est le vrai état du datastore, pas un bug, et ça se résout tout seul — pas besoin de le filtrer ni de faire du polling pour le contourner.
{{% /notice %}}

---

### `GU` — Mettre à jour une ligne de grille {#gu}

Met à jour une ligne spécifique (index base zéro `i`) avec de nouvelles valeurs d'éléments.

```json
{
  "GU": {
    "m": {
      "Sequencer": {
        "p": {
          "sequence": {
            "i": 1,
            "e": {
              "target": "M42",
              "exposure": 60
            }
          }
        }
      }
    }
  }
}
```

---

### `GD` — Supprimer une ligne de grille {#gd}

Supprime la ligne à l'index `i` (base zéro).

```json
{
  "GD": {
    "m": {
      "Sequencer": {
        "p": {
          "sequence": {
            "i": 0
          }
        }
      }
    }
  }
}
```

---

### `GF` — Sélectionner une ligne de grille {#gf}

Demande au serveur de charger une ligne de la grille dans les éléments éditables de la propriété (utilisé lors du clic sur une ligne pour l'éditer).

```json
{
  "GF": {
    "m": {
      "Sequencer": {
        "p": {
          "sequence": {
            "i": 2
          }
        }
      }
    }
  }
}
```

---

### `GH` — Monter une ligne de grille {#gh}

Déplace la ligne à l'index `i` d'une position vers le haut.

```json
{
  "GH": {
    "m": {
      "Sequencer": {
        "p": {
          "sequence": {
            "i": 3
          }
        }
      }
    }
  }
}
```

---

### `GB` — Descendre une ligne de grille {#gb}

Déplace la ligne à l'index `i` d'une position vers le bas.

```json
{
  "GB": {
    "m": {
      "Sequencer": {
        "p": {
          "sequence": {
            "i": 2
          }
        }
      }
    }
  }
}
```

---

### `GR` — Réinitialiser la grille {#gr}

Vide entièrement la grille d'une propriété (toutes les lignes supprimées) — contrairement à `GD` qui ne supprime qu'une ligne. Le serveur diffuse [`gr`]({{% relref "server-messages.fr.md" %}}#gr) en réponse.

```json
{
  "GR": {
    "m": {
      "Sequencer": {
        "p": {
          "sequence": {}
        }
      }
    }
  }
}
```

Pas de champ `i` : `GR` cible la grille entière, pas une ligne particulière.

---

## Cycle de vie des modules

### `CL` — Charger une configuration {#cl}

Charge tous les modules d'une configuration sauvegardée (liste dans `controllerdata.availableconfs`, voir [Contrôleur]({{% relref "controller.fr.md" %}})).

```json
{ "CL": { "name": "modules" } }
```

---

### `CS` — Sauvegarder la configuration courante {#cs}

Enregistre l'ensemble des modules actuellement chargés sous le nom donné (`GlobalDatastore` n'est jamais inclus).

```json
{ "CS": { "name": "modules" } }
```

---

### `ML` — Charger un module {#ml}

Demande au serveur de charger dynamiquement un module. Ces commandes visent le contrôleur : le payload n'utilise **pas** l'enveloppe `m`/`p`.

```json
{ "ML": { "lib": "focus", "label": "MyFocuser", "profile": "default" } }
```

| Champ | Description |
|---|---|
| `lib` | Bibliothèque du module, **sans** le préfixe `libost` (le serveur l'ajoute) : `focus` pour `libostfocus`. Les bibliothèques disponibles sont les clés de `controllerdata.libraries` |
| `label` | Libellé de cette instance ; le **nom du module** est ce libellé sans les espaces. Si un module de ce nom est déjà chargé, la demande est refusée |
| `profile` | Nom de profil à charger (optionnel) |

{{% notice style="warning" title="Ne pas passer telle quelle une clé de controllerdata.libraries" %}}
Les clés de `controllerdata.libraries` sont déjà préfixées (`libostfocus`, `libostdummy`...). Le serveur ajoute `libost` une nouvelle fois, sans condition : transmettre la clé telle quelle double le préfixe et échoue :

```json
{ "ML": { "lib": "libostdummy", "label": "MyDummy" } }
```
```
Error loading library MyDummy - Cannot load library libostlibostdummy: cannot open shared object file: No such file or directory
```

Retirer d'abord le préfixe `libost` de la clé (même transformation que celle sur laquelle s'appuie la convention de clé de `profiles`, voir [Données du contrôleur]({{% relref "controller.fr.md" %}})) :

```json
{ "ML": { "lib": "dummy", "label": "MyDummy" } }
```
{{% /notice %}}

---

### `MK` — Décharger un module {#mk}

Demande au serveur de décharger un module en cours d'exécution, désigné par son nom.

```json
{ "MK": { "name": "MyFocuser" } }
```

`GlobalDatastore` ne peut pas être déchargé : la demande est refusée (message d'erreur dans les logs).

---

## Système de fichiers

### `FS` — Choisir le dossier surveillé {#fs}

Désigne le dossier dont le serveur publie la liste de fichiers (voir [Système de fichiers]({{% relref "controller.fr.md" %}}#fichiers)). Le chemin est **relatif au webroot** ; la chaîne vide désigne la racine.

```json
{ "FS": { "folder": "/Sequencer/gam Cyg/LIGHT/Luminance" } }
```

Le changement est **global au serveur** (tous les clients voient la nouvelle liste). Le chemin n'est pas validé : un dossier inexistant donne une liste vide.

---

## Contrôle du serveur INDI embarqué {#indi}

Ces commandes ne sont disponibles que lorsque le serveur dispose d'une instance INDI embarquée.

| Commande | Description |
|---|---|
| `YA` | Démarrer le serveur INDI embarqué |
| `YZ` | Arrêter le serveur INDI embarqué |
| `YL` | Charger un driver INDI (`{"YL": {"driver": "<binary>"}}`) |
| `YR` | Recharger un driver INDI (`{"YR": {"driver": "<binary>"}}`) |
| `YS` | Arrêter un driver INDI (`{"YS": {"driver": "<binary>"}}`) |

`YA` et `YZ` n'ont pas de payload (`{"YA": {}}`). Pour `YL`, `YR` et `YS`, `driver` est le champ `binary` d'une entrée de `controllerdata.indidrivers`.

---

## Récapitulatif du contrôle d'accès

| Valeur grant serveur | Commandes autorisées |
|---|---|
| `grant-client: "1"` | Toutes les commandes |
| `grant-client: "0"` | `DU`, `LO`, `IL` uniquement |
| `grant-client: "-1"` | `LO` uniquement (authentification requise) |

Les commandes envoyées sans droits suffisants sont silencieusement ignorées par le serveur.
