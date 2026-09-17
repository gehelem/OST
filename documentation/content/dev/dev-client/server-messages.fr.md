---
title: Messages Serveur → Client
weight: 20
description: "Tous les messages JSON envoyés par le serveur OST aux clients connectés"
---

## Format des messages

Chaque message est un objet JSON avec **une seule clé de premier niveau**. Cette clé encode le type d'événement et peut inclure une description lisible séparée par un tiret :

```
"aa-dump all data"   →  type d'événement = 2 premiers caractères = "aa"
"ea"                 →  type d'événement = "ea"
"d"                  →  type d'événement = "d" (caractère unique)
```

Les clients doivent comparer sur les **1 ou 2 premiers caractères** de la clé, et non sur la chaîne complète, car la partie description peut changer.

Toutes les clés d'événements envoyées par le serveur sont en **minuscules** (ex. `d`, `ea`, `ap`, `gc`).

---

## Index des types d'événements

| Clé | Description |
|---|---|
| [`d`](#d) | Dump initial complet (réponse à `DU`) |
| [`aa`](#aa) | Dump complet d'un module (module chargé/rechargé) |
| [`ap`](#ap) | Remplacement complet d'une propriété |
| [`ea`](#ea) | Mise à jour de toutes les valeurs d'éléments d'une propriété |
| [`ee`](#ee) | Mise à jour d'une seule valeur d'élément |
| [`ev`](#ev) | Mise à jour d'une valeur d'élément avec ses contraintes |
| [`ps`](#ps) | Mise à jour du statut et de l'état activé d'une propriété |
| [`dm`](#dm) | Module supprimé |
| [`dp`](#dp) | Propriété supprimée |
| [`gc`](#gc) | Ligne de grille créée |
| [`gu`](#gu) | Ligne de grille mise à jour |
| [`gd`](#gd) | Ligne de grille supprimée |
| [`gr`](#gr) | Réinitialisation de la grille (toutes les lignes effacées) |
| [`lc`](#lc) | LOV créé ou fusionné |
| [`lu`](#lc) | LOV remplacé |
| [`ld`](#lc) | LOV supprimé |
| [`fs`](#fs) | Profil sauvegardé |
| [`fl`](#fs) | Profil chargé |
| [`fc`](#fs) | Profil modifié (modifications non sauvegardées) |
| [`l`](#l) | Entrée de log |
| [`uc`](#uc) | Mise à jour des données du contrôleur |
| [`xx`](#xx) | Pong keepalive |

---

## `d` — Dump initial complet {#d}

Envoyé en réponse à une requête `DU`. Contient tout : tous les modules, les logs, les listes de fichiers et les données du contrôleur.

```json
{
  "d": {
    "grant-client": "1",
    "grant-server": "0",
    "serverlng": "en",
    "m": {
      "Focus": {
        "infos": { "label": "Focus", "name": "Focus", "description": "..." },
        "f": { "name": "default", "changed": false },
        "l": {
          "myLov": { "label": "My LOV", "type": "string", "values": { "a": "Option A" } }
        },
        "p": {
          "parameters": {
            "label": "Parameters",
            "order": "10",
            "level1": "Focus",
            "level2": "Config",
            "status": 0,
            "permission": 2,
            "enabled": true,
            "showElts": true,
            "hasGrid": false,
            "hasGraph": false,
            "rule": 0,
            "badge": false,
            "preicon1": "", "preicon2": "", "posticon1": "", "posticon2": "",
            "hasprofile": true,
            "freevalue": "",
            "e": {
              "iterations": {
                "type": "int",
                "label": "Iterations",
                "order": "10",
                "hint": "",
                "autoupdate": false,
                "badge": false,
                "directedit": false,
                "value": 5,
                "min": 1,
                "max": 20,
                "step": 1,
                "format": "",
                "slider": 0
              }
            }
          }
        }
      }
    },
    "files": {
      "folders": ["Allsky/archives"],
      "files": ["Allsky/archives/image.fits"],
      "selectedfolder": ""
    },
    "logs": [
      { "d": "2024-01-01T12:00:00.000", "c": "Focus", "t": "Module loaded", "l": 1 }
    ],
    "controllerdata": {
      "profiles": {
        "Focus": ["default", "test"]
      }
    },
    "lovs": {
      "sharedLov": { "label": "Shared LOV", "type": "string", "values": {} }
    }
  }
}
```

{{% notice style="warning" title="Abréviations du format wire" %}}
`m` = modules, `p` = properties, `e` = elements, `l` = globallovs, `f` = profile.
Normaliser ces abréviations vers leurs noms complets lors du stockage interne.
{{% /notice %}}

---

## `aa` — Dump complet d'un module (module chargé/rechargé) {#aa}

Envoyé lorsqu'un module est chargé ou rechargé à l'exécution. Contient l'état complet du module. La clé commence par `"aa"`.

```json
{
  "aa-dump all data": {
    "Guider": {
      "infos": { "label": "Guider", "name": "Guider", "description": "..." },
      "f": { "name": "default", "changed": false },
      "l": {},
      "p": {
        "status": { ... }
      }
    }
  }
}
```

---

## `ap` — Remplacement complet d'une propriété {#ap}

Remplace entièrement une ou plusieurs propriétés. La clé commence par `"ap"`.

```json
{
  "ap-dump all property data": {
    "Focus": {
      "p": {
        "parameters": {
          "label": "Parameters",
          "status": 0,
          "permission": 2,
          "enabled": true,
          "showElts": true,
          "hasGrid": false,
          "hasGraph": false,
          "rule": 0,
          "e": {
            "iterations": {
              "type": "int",
              "value": 7,
              "min": 1, "max": 20, "step": 1,
              "label": "Iterations", "order": "10", "hint": "",
              "autoupdate": false, "badge": false, "directedit": false,
              "format": "", "slider": 0
            }
          }
        }
      }
    }
  }
}
```

---

## `ea` — Mise à jour de toutes les valeurs d'éléments (niveau propriété) {#ea}

Le message de mise à jour le plus courant. Envoie les valeurs courantes de tous les éléments d'une propriété. Contient uniquement les valeurs — sans métadonnées. La clé est exactement `"ea"`.

```json
{
  "ea": {
    "Allsky": {
      "p": {
        "coming": {
          "e": {
            "sunrise": { "hh": 7, "mm": 6, "ss": 51 },
            "sunset":  { "hh": 21, "mm": 15, "ss": 1 }
          }
        }
      }
    }
  }
}
```

Pour les types simples (`int`, `float`, `bool`, `string`, `light`), la valeur est un scalaire :
```json
{
  "ea": {
    "Focus": {
      "p": {
        "parameters": {
          "e": {
            "iterations": 5,
            "exposure": 3.5,
            "active": true
          }
        }
      }
    }
  }
}
```

---

## `ee` — Mise à jour d'une seule valeur d'élément {#ee}

Met à jour la valeur d'un seul élément. Même format que `ea` mais pour un seul élément. La clé est exactement `"ee"`.

```json
{
  "ee": {
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
```

---

## `ev` — Mise à jour d'un élément avec métadonnées {#ev}

Met à jour la valeur d'un élément et ses contraintes (min, max, format). La clé commence par `"ev"`.

```json
{
  "ev-set one element value/min/max/format ": {
    "Focus": {
      "p": {
        "parameters": {
          "e": {
            "temperature": {
              "value": 12.5,
              "min": -40.0,
              "max": 80.0,
              "format": "%.1f"
            }
          }
        }
      }
    }
  }
}
```

---

## `ps` — Mise à jour du statut d'une propriété {#ps}

Met à jour uniquement les champs `status` et `enabled` d'une propriété. La clé commence par `"ps"`.

```json
{
  "ps-only property state": {
    "Focus": {
      "p": {
        "autofocus": {
          "status": 2,
          "enabled": true
        }
      }
    }
  }
}
```

Valeurs de statut : `0` Veille · `1` OK · `2` Occupé · `3` Erreur

---

## `dm` — Module supprimé {#dm}

Envoyé lorsqu'un module est déchargé. Le client doit supprimer toutes les données de ce module. La clé commence par `"dm"`.

```json
{
  "dm-delete/remove module": {
    "Focus": {}
  }
}
```

---

## `dp` — Propriété supprimée {#dp}

Envoyé lorsqu'une propriété est retirée d'un module. La clé commence par `"dp"`.

```json
{
  "dp-delete/remove property": {
    "Focus": {
      "p": {
        "oldProperty": ""
      }
    }
  }
}
```

---

## `gc` — Ligne de grille créée {#gc}

Envoyé après l'ajout d'une nouvelle ligne dans une propriété de type grille. La clé commence par `"gc"`.

```json
{
  "gc-grid new line ": {
    "Sequencer": {
      "p": {
        "sequence": {
          "i": 3,
          "values": {
            "target": "M31",
            "exposure": 120,
            "count": 10
          }
        }
      }
    }
  }
}
```

`i` est l'index de la ligne nouvellement créée. `values` est un objet indexé par nom d'élément.

---

## `gu` — Ligne de grille mise à jour {#gu}

Envoyé lorsqu'une ligne existante de la grille est modifiée. La clé commence par `"gu"`.

```json
{
  "gu-grid update line": {
    "Sequencer": {
      "p": {
        "sequence": {
          "i": 1,
          "values": {
            "target": "M42",
            "exposure": 60,
            "count": 20
          }
        }
      }
    }
  }
}
```

---

## `gd` — Ligne de grille supprimée {#gd}

Envoyé lorsqu'une ligne de la grille est retirée. La clé commence par `"gd"`.

```json
{
  "gd-grid delete line": {
    "Sequencer": {
      "p": {
        "sequence": {
          "i": 0
        }
      }
    }
  }
}
```

`i` est l'index (base zéro) de la ligne supprimée. Les lignes suivantes décalent d'un rang vers le haut.

---

## `gr` — Réinitialisation de la grille {#gr}

Efface l'intégralité de la grille d'une propriété. La clé commence par `"gr"`.

```json
{
  "gr-grid reset": {
    "Sequencer": {
      "p": {
        "sequence": {}
      }
    }
  }
}
```

---

## `lc` / `lu` / `ld` — Événements LOV {#lc}

**`lc` — LOV créé/fusionné** (clé commence par `"lc"`) :

LOV de module :
```json
{
  "lc-lov create": {
    "Focus": {
      "l": {
        "myLov": {
          "label": "My LOV",
          "type": "string",
          "values": { "a": "Option A", "b": "Option B" }
        }
      }
    }
  }
}
```

LOV du contrôleur (sans enveloppe module, utilise la clé `lovs`) :
```json
{
  "lc-lov create": {
    "lovs": {
      "sharedLov": {
        "label": "Shared LOV",
        "type": "string",
        "values": { "x": "Value X" }
      }
    }
  }
}
```

**`lu` — LOV remplacé** (même format que `lc`, remplacement complet).

**`ld` — LOV supprimé** (clé commence par `"ld"`) :
```json
{
  "ld-lov delete": {
    "lovs": {
      "sharedLov": {}
    }
  }
}
```

---

## `fs` / `fl` / `fc` — Événements de profil {#fs}

Envoyés lorsqu'un profil est sauvegardé, chargé ou modifié.

```json
{
  "fs-profile saved": {
    "Focus": {
      "f": { "name": "myProfile", "changed": false }
    }
  }
}
```

| Préfixe de clé | Événement |
|---|---|
| `fs` | Profil sauvegardé |
| `fl` | Profil chargé |
| `fc` | Profil avec modifications non sauvegardées |

---

## `l` — Entrée de log {#l}

Envoyé pour chaque message de log généré par le serveur. La clé est exactement `"l"`.

```json
{
  "l": {
    "d": "2024-01-01T12:00:00.000",
    "c": "Focus",
    "t": "Autofocus completed successfully",
    "l": 1
  }
}
```

| Champ | Description |
|---|---|
| `d` | Date/heure ISO 8601 avec millisecondes |
| `c` | Contexte/source (nom du module ou `"WS"`) |
| `t` | Texte du message (traduit dans la langue du client) |
| `l` | Niveau : `0` Debug · `1` Info · `2` Avertissement · `3` Erreur · `4` Critique |

---

## `uc` — Mise à jour des données du contrôleur {#uc}

Envoyé lorsque les données globales du contrôleur changent (ex. liste de fichiers, profils). La clé commence par `"uc"`.

```json
{
  "uc-update controller data": {
    "profiles": {
      "Focus": ["default", "highres"],
      "Guider": ["default"]
    }
  }
}
```

Le payload est `{"clé": valeur}` où `valeur` peut être une chaîne, un tableau ou un objet.

---

## `xx` — Pong keepalive {#xx}

Envoyé par le serveur en réponse à un heartbeat client (`XX`). Pas de payload utile ; les clients peuvent l'ignorer.

```json
{ "xx": {} }
```
