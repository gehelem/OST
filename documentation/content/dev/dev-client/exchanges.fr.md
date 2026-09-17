---
title: Exemple de session
weight: 40
description: "Échanges WebSocket annotés illustrant une session client complète"
---

Cette page détaille une session WebSocket réaliste : connexion, dump, login, réception des mises à jour, envoi de commandes et keepalive.

Notation :
- **`→`** Client → Serveur
- **`←`** Serveur → Client

---

## 1. Connexion

Ouvrir une connexion WebSocket vers le serveur.

```
→ WS connect to ws://server:9624
← (connexion acceptée)
```

---

## 2. Demander le dump complet

À envoyer immédiatement après la connexion. Préciser la langue souhaitée pour les labels traduits.

```json
→ {"DU": {"language": "fr"}}
```

Le serveur répond avec `d`, un message unique contenant tous les modules, leurs propriétés et éléments, la liste de fichiers, les logs, les LOV du contrôleur et les droits d'accès.

```json
← {
  "d": {
    "grant-client": "1",
    "grant-server": "0",
    "serverlng": "fr",
    "m": {
      "MyFocuser": {
        "infos": {"label": "MyFocuser", "name": "MyFocuser", "description": ""},
        "f": {"name": "default", "changed": false},
        "l": {},
        "p": {
          "parameters": {
            "label": "Paramètres", "order": "10",
            "level1": "MyFocuser", "level2": "Config",
            "status": 0, "permission": 2, "enabled": true,
            "showElts": true, "hasGrid": false, "hasGraph": false,
            "rule": 0, "badge": false,
            "preicon1": "", "preicon2": "", "posticon1": "", "posticon2": "",
            "hasprofile": true, "freevalue": "",
            "e": {
              "exposure": {
                "type": "float", "label": "Exposition", "order": "10",
                "hint": "", "autoupdate": false, "badge": false,
                "directedit": true,
                "value": 3.5, "min": 0.1, "max": 30.0, "step": 0.1,
                "format": "%.1f", "slider": 0
              },
              "abortfocus": {
                "type": "bool", "label": "Annuler", "order": "90",
                "hint": "", "autoupdate": false, "badge": false,
                "directedit": true, "value": false
              }
            }
          }
        }
      }
    },
    "files": {"folders": [], "files": [], "selectedfolder": ""},
    "logs": [],
    "controllerdata": {"profiles": {"MyFocuser": ["default"]}},
    "lovs": {}
  }
}
```

{{% notice style="info" title="grant-client" %}}
Si `grant-client` vaut `"-1"`, le serveur exige une authentification avant d'accepter des commandes. Voir étape 3.
S'il vaut `"0"` ou `"1"`, les commandes sont acceptées directement.
{{% /notice %}}

---

## 3. Login (si requis)

Nécessaire uniquement si `grant-client` vaut `"-1"` dans le dump.

```json
→ {"LO": {"user": "ADMIN", "pw": "admin", "language": "fr"}}
```

En cas de succès, le serveur renvoie un nouveau dump (même format que `d`) avec les droits mis à jour.

---

## 4. Réception des mises à jour en temps réel

Le serveur envoie des mises à jour à tout moment. Aucun polling n'est nécessaire.

### Mise à jour des valeurs d'éléments (`ea`)

Le message le plus fréquent. Le serveur envoie les valeurs courantes dès qu'un changement survient.

```json
← {
  "ea": {
    "MyFocuser": {
      "p": {
        "parameters": {
          "e": {
            "exposure": 3.5,
            "abortfocus": false
          }
        }
      }
    }
  }
}
```

### Changement de statut d'une propriété (`ps`)

Envoyé quand une propriété passe en mode occupé, inactif ou désactivé.

```json
← {
  "ps-only property state": {
    "MyFocuser": {
      "p": {
        "parameters": {
          "status": 2,
          "enabled": true
        }
      }
    }
  }
}
```

Valeurs de statut : `0` Veille · `1` OK · `2` Occupé · `3` Erreur

### Entrée de log (`l`)

```json
← {"l": {"d": "2026-06-25T10:14:08.000", "c": "MyFocuser", "t": "Autofocus démarré", "l": 1}}
```

---

## 5. Envoyer une commande

### Annuler une action en cours

Les éléments avec `directedit: true` envoient leur valeur immédiatement via `SV`.

```json
→ {"SV": {"m": {"MyFocuser": {"p": {"actions": {"e": {"abortfocus": true}}}}}}}
```

Le serveur accuse réception en envoyant des mises à jour de statut et de valeurs :

```json
← {"ps-only property state": {"MyFocuser": {"p": {"actions": {"status": 2, "enabled": true}}}}}
← {"ea": {"MyFocuser": {"p": {"actions": {"e": {"abortfocus": false}}}}}}
← {"ps-only property state": {"MyFocuser": {"p": {"actions": {"status": 0, "enabled": true}}}}}
```

### Démarrer une action

```json
→ {"SV": {"m": {"MyPlanner": {"p": {"actions": {"e": {"start": true}}}}}}}
```

### Envoyer plusieurs valeurs à la fois

Utiliser `SA` pour les propriétés où `directedit` est `false` (édition par formulaire).

```json
→ {
  "SA": {
    "m": {
      "MyFocuser": {
        "p": {
          "parameters": {
            "e": {
              "exposure": 5.0,
              "gain": 100
            }
          }
        }
      }
    }
  }
}
```

---

## 6. Keepalive

Envoyer `XX` toutes les 30 secondes pour maintenir la connexion active.

```json
→ {"XX": {}}
← {"xx": {}}
```

---

## Séquence complète (résumé)

```
→ WS connect
→ {"DU": {"language": "fr"}}
← {"d": { ... dump complet ... }}

← {"ea": { ... mises à jour périodiques ... }}

→ {"SV": {"m": {"MyFocuser": {"p": {"actions": {"e": {"abortfocus": true}}}}}}}
← {"ps-...": { ... statut occupé ... }}
← {"ea": { ... valeurs ... }}
← {"ps-...": { ... statut veille ... }}

→ {"XX": {}}
← {"xx": {}}
```
