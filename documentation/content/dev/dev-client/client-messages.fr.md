---
title: Messages Client → Serveur
weight: 30
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

## Actions sur les icônes

Les icônes de propriété (preicon / posticon) déclenchent des actions backend telles que la sauvegarde d'un profil, le chargement d'un profil ou le lancement d'un processus. Les événements de clic sont envoyés via les commandes suivantes.

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

{{% notice style="note" title="Les éléments de base se mettent à jour avant la ligne de grille" %}}
Créer une ligne de grille applique d'abord les valeurs soumises aux éléments de base de la propriété (chacun émettant son propre `ea`), puis ajoute la ligne et émet `gc`. Si une propriété affiche à la fois ses éléments de base et sa grille (`showElts` et `showGrid` tous les deux actifs — combinaison rare), un client qui affiche en direct verra brièvement les éléments de base refléter les valeurs de la nouvelle ligne avant que la ligne elle-même n'apparaisse. C'est le vrai état du datastore, pas un bug — adaptez votre affichage à ce comportement plutôt que de chercher à le filtrer, et surtout ne faites jamais de polling pour le contourner.
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
