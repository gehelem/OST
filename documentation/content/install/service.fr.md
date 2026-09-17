---
title: Gérer le service ostserver
weight: 30
description: "Démarrer, arrêter, personnaliser les options de démarrage du service ostserver"
---

`ostserver` tourne en arrière-plan en tant que **service systemd**. Systemd est le gestionnaire de services de Linux : il démarre automatiquement `ostserver` au démarrage de la machine, le redémarre s'il plante, et permet de le contrôler avec quelques commandes simples.

---

## Démarrer, arrêter, redémarrer

```bash
# Démarrer le service
sudo systemctl start ostserver

# Arrêter le service
sudo systemctl stop ostserver

# Redémarrer le service (utile après avoir modifié les options)
sudo systemctl restart ostserver

# Voir l'état du service
sudo systemctl status ostserver
```

---

## Activer / désactiver le démarrage automatique

Par défaut, le service est **activé** : il démarre automatiquement à chaque démarrage de la machine.

```bash
# Désactiver le démarrage automatique
sudo systemctl disable ostserver

# Réactiver le démarrage automatique
sudo systemctl enable ostserver
```

---

## Consulter les logs

```bash
# Afficher les derniers logs
sudo journalctl -u ostserver

# Suivre les logs en temps réel
sudo journalctl -u ostserver -f

# Limiter aux 100 dernières lignes
sudo journalctl -u ostserver -n 100
```

---

## Personnaliser les options de démarrage

Lors de l'installation, un fichier de configuration est créé dans votre répertoire utilisateur :

```
~/.ost/ostserver.parms
```

Ce fichier contient **un argument par ligne**. Les lignes qui commencent par `#` sont des commentaires et sont ignorées. Les espaces dans les valeurs sont supportés nativement — aucun guillemet n'est nécessaire.

Pour l'éditer :

```bash
nano ~/.ost/ostserver.parms
```

Exemple de contenu :

```
# Web server media folder (must be writable)
--webroot=/var/lib/osterix/media

# Start embedded INDI server (Y/N)
--indiserver=Y

# Log level: 0=debug, 1=info, 2=warning, 3=error, 4=critical
--loglevel=1

# Security level: 0=disabled, 1=read only, 2=user grants
--grant=0

# Frontend banner text (spaces are allowed, no quoting needed)
--banner=Mon observatoire sans tête
```

Après toute modification, redémarrer le service pour qu'elle soit prise en compte :

```bash
sudo systemctl restart ostserver
```

{{% notice style="info" title="Fichier préservé lors des mises à jour" %}}
Le fichier `ostserver.parms` n'est **jamais écrasé** lors d'une mise à jour du paquet. Vos personnalisations sont conservées.
{{% /notice %}}

---

## Référence des options disponibles

| Option | Défaut | Description |
|---|---|---|
| `--webroot=<chemin>` | `/var/lib/osterix/media` | Dossier média servi par nginx |
| `--indiserver=Y\|N` | `N` | Démarrer le serveur INDI embarqué |
| `--loglevel=<0-4>` | `1` | Verbosité des logs : 0=debug, 1=info, 2=warning, 3=error, 4=critical |
| `--logfile=<chemin>` | *(horodaté dans `~/.ost/`)* | Chemin du fichier de log |
| `--grant=<0\|1\|2>` | — | Sécurité : 0=désactivée, 1=lecture seule, 2=grants utilisateur |
| `--lng=<fr\|en\|...>` | `fr` | Langue de l'interface (fr, en, de, it, es) |
| `--banner=<texte>` | `Observatoire Sans tête` | Bannière affichée dans le frontend |
| `--configuration=<nom>` | `default` | Nom de la configuration à charger au démarrage |
| `--libpath=<chemin>` | *(chemin système)* | Répertoire des modules `.so` |
| `--dbpath=<chemin>` | *(répertoire de travail)* | Emplacement de la base de données |
| `--systemwatchinterval=<s>` | `10` | Intervalle de surveillance système en secondes (0 pour désactiver) |
| `--minfree=<0-99>` | `10` | Espace disque libre minimum en % avant de sauter l'écriture d'une image |
| `--ssl=Y\|N` | `N` | Activer SSL pour le WebSocket |
| `--sslcert=<chemin>` | `/etc/ost/server.crt` | Certificat SSL |
| `--sslkey=<chemin>` | `/etc/ost/server.key` | Clé privée SSL |
| `--setadminpassword=<mdp>` | — | Définir le mot de passe administrateur (à supprimer après utilisation) |
