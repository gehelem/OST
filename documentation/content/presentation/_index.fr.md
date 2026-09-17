---
title: Présentation
weight : 10
---

## Pourquoi "Observatoire Sans Tête" ?

Ce nom vient d'une blague avec mon ami Bob, qui consiste à traduire mot à mot "Headless Observatory".
Ce terme donne bien le but recherché : un serveur autonome qu'on peut controler à distance.

## Qu'est-ce que OST ?

OST est une plateforme d'astrophotographie autonome. Elle tourne en tâche de fond sur une machine dédiée — Raspberry Pi, mini-PC ou machine virtuelle — et s'utilise depuis n'importe quel navigateur web sur le réseau local, sans écran ni clavier branchés sur la machine de contrôle.

L'objectif est de pouvoir piloter une session complète d'observation depuis un canapé, un autre bâtiment, ou à distance via VPN : mise en station, pointage, mise au point, guidage, acquisition, et documentation de la nuit.

## Architecture

OST repose sur une architecture client-serveur :

| Composant | Rôle |
|---|---|
| **Serveur OST** | Application C++/Qt tournant en arrière-plan, pilote les appareils via le protocole INDI |
| **Interface web** | Client Angular servi par Nginx, communique avec le serveur via WebSocket |
| **Serveur INDI** | Couche d'abstraction matérielle — pilotes pour caméras, montures, focuseurs, roues à filtres, etc. |

La communication entre le client et le serveur est entièrement basée sur des messages JSON échangés via WebSocket, ce qui permet de contrôler OST depuis n'importe quel appareil disposant d'un navigateur moderne.

## Modules

OST est organisé en modules indépendants, chacun dédié à une fonction spécifique :

| Module | Fonction |
|---|---|
| **Séquenceur** | Acquisition automatisée : séquences de poses par type, filtre, gain et durée |
| **Guidage** | Autoguidage via PHD2, contrôle de l'agitation et des corrections |
| **Navigateur** | Résolution de champ (plate solving) et centrage automatique sur une cible |
| **Planificateur** | Orchestration d'une liste d'objets : centrage puis acquisition pour chacun |
| **Polar** | Alignement polaire par la méthode des 3 images |
| **Mise au point** | Autofocus par courbe en V avec minimisation du HFR |
| **Inspector** | Analyse de la qualité optique : carte HFR, carte des aberrations, mosaïque des coins |
| **IndiPanel** | Panneau de contrôle brut de tous les appareils INDI connectés |
| **Allsky** | Caméra grand-angle : keogramme, stack, timelapse et superposition météo |

## Inspirations

Sur sa forme, OST s'inspire de nombreux autres projets opensource :
- https://github.com/GuLinux/AstroPhoto-Plus
- https://github.com/pludov/mobindi
- https://www.indigo-astronomy.org/for-users.html

Bien sûr l'inspiration vient aussi de solutions commerciales :
- ASIAIR
- Stellarmate

## Technologies utilisées

| Domaine | Technologie |
|---|---|
| Serveur | C++17, Qt6, libnova, cfitsio, libindi, StellarSolver |
| Interface web | Angular, TypeScript |
| Serveur web | Nginx |
| Protocole | INDI, WebSocket, JSON |
| Vidéo | ffmpeg (timelapse allsky) |
| Packaging | PPA Ubuntu (Launchpad) |
