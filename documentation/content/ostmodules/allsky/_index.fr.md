---
title: Allsky
weight: 90
description: Caméra allsky avec keogramme, stack, timelapse et données météo
---

## Rôle

Le module Allsky pilote une caméra grand-angle (allsky) en boucle continue pour documenter la nuit. À chaque image, il génère une superposition des données météo, construit un keogramme et un stack en temps réel, puis produit un timelapse vidéo en fin de session. Les sessions passées sont archivées et consultables.

![Capture d'écran du module Allsky](/images/modules/allsky.png)

## Appareils requis

| Appareil | Rôle |
|---|---|
| Caméra | Acquisition des images allsky |
| GPS | Optionnel — position géographique pour le calcul des heures de lever/coucher du soleil |
| Station météo | Optionnel — données météo affichées en superposition et dans le keogramme |

## Paramètres

### Acquisition

| Paramètre | Description |
|---|---|
| **Exposition** | Durée d'exposition en secondes |
| **Gain** | Gain de la caméra |
| **Offset** | Offset de la caméra |
| **Délai (s)** | Intervalle en secondes entre le démarrage de deux acquisitions consécutives (doit être supérieur à l'exposition) |

### Planification

| Paramètre | Description |
|---|---|
| **Manuel** | Démarre immédiatement et fonctionne en continu tant que l'action Play est active |
| **Heure fixe** | Acquisition entre une heure de début et une heure de fin (jour ou nuit selon l'ordre des heures) |
| **Coucher du soleil** | Démarre automatiquement au coucher du soleil et s'arrête au lever du soleil |

Pour les modes *Heure fixe* et *Coucher du soleil*, les heures de prochain lever et coucher de soleil sont calculées automatiquement à partir de la position géographique configurée (libnova).

### Position géographique

La position géographique (latitude, longitude) est nécessaire pour le calcul des heures solaires. Elle peut être saisie manuellement ou importée depuis un appareil GPS connecté.

### Exposition/gain automatique

| Paramètre | Description |
|---|---|
| **Activé** | Active l'ajustement automatique |
| **Modifier exposition ou gain** | Choisit le paramètre à ajuster |
| **Valeur minimum** | Limite basse de l'ajustement |
| **Valeur maximum** | Limite haute de l'ajustement |
| **Mesure** | Statistique utilisée comme référence (Moyenne ou Médiane) |
| **Valeur cible** | Valeur cible de la statistique |
| **Seuil (%)** | Non utilisé actuellement |

Le coefficient de correction est calculé par : `nouvelle_valeur = valeur_actuelle × (cible / mesure_actuelle)`.

### Archives

| Paramètre | Description |
|---|---|
| **Conserver les images individuelles** | Si activé, les JPEG individuels de chaque session sont conservés dans les archives ; sinon seuls le keogramme, le stack et le timelapse sont gardés |

## Traitement de chaque image

Pour chaque image acquise :

1. **Sauvegarde FITS** de l'image brute
2. **Superposition** de la date/heure et des données météo (température, humidité, pression) en couleur sur le JPEG
3. **Stack par maximum** : chaque pixel conserve la valeur maximale de toutes les images de la session
4. **Keogramme** : une bande verticale de 1 pixel prélevée au centre de chaque image est accolée aux précédentes pour former un graphique temporel de la nuit ; les données météo y sont représentées par des points colorés (rouge = température, vert = humidité, bleu = pression)
5. **Journal SNR** : le rapport signal/bruit de l'image est enregistré avec l'horodatage

## Timelapse

En fin de session (arrêt ou fin de planification), le module génère un timelapse vidéo (MP4 H.264 à 30 fps) à partir de tous les JPEG de la session, via ffmpeg.

## Archives

Chaque session terminée est déplacée dans un dossier d'archives horodaté. Les archives sont consultables directement depuis l'interface avec, pour chaque session :

| Élément | Description |
|---|---|
| **Date** | Identifiant horodaté de la session |
| **Keogramme** | Bande temporelle de la nuit |
| **Stack** | Image empilée (maximum) de la nuit |
| **Timelapse** | Vidéo de la nuit |

## Données météo

Si un appareil météo INDI est configuré, le module lit les propriétés `WEATHER_PARAMETERS` pour récupérer température, humidité et pression. Ces valeurs sont :
- Affichées en temps réel dans la section **Mesures**
- Superposées sur chaque image
- Tracées dans le keogramme
- Historisées dans un graphique temporel

## Actions

| Action | Description |
|---|---|
| **Play** | Démarre la boucle d'acquisition selon le mode de planification actif |
| **Pause** | Suspend temporairement les acquisitions sans clore la session |
| **Stop** | Arrête la session, génère le timelapse et archive la session |
