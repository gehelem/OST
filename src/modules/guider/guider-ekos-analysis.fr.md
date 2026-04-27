---
title: Module - Guider
weight : 20
--- 

# Documentation du module Guider

## Référence : autoguidage interne Ekos (KStars)

Cette analyse décrit le fonctionnement de l'autoguidage interne d'Ekos, utilisé comme référence pour le développement du module guider d'OST. Les fichiers sources analysés se trouvent dans `kstars/ekos/guide/`.

---

## Fichiers clés dans Ekos

| Fichier | Rôle |
|---|---|
| `calibration.h/cpp` | Structure, calculs d'angles, matrices, taux de pulsation |
| `calibrationprocess.cpp` | Machine d'état de la séquence de calibration |
| `gmath.h/cpp` | Cœur du traitement : détection d'étoile, drift, corrections |
| `internalguider.h/cpp` | Orchestration : pier flip, changement de cible |
| `vect.h`, `matr.h` | Algèbre vectorielle et matricielle |

---

## 1. Calibration

### Séquence (machine d'état `CalibrationProcess`)

5 étapes dans l'ordre :
1. **RA Outward** — pulse RA+ jusqu'à dérive > 15 px (ou max itérations)
2. **RA Inward** — pulse RA− pour revenir à la position de départ
3. **DEC Backlash** (optionnel) — 5 pulses DEC+ pour mesurer le jeu mécanique
4. **DEC Outward** — idem RA+ mais pour DEC
5. **DEC Inward** — retour à la position de départ

### Calculs dans `calculate2D()`

**Conversion pixels → arcseconde :**
```
arcsec_par_pixel = binning × 206264.806 × taille_pixel_mm / focale_mm
```

**Angles de rotation pour chaque axe :**
```
phi_RA  = atan2(-dy_RA,  dx_RA)  × (180/π)   [degrés, 0–360°]
phi_DEC = atan2(-dy_DEC, dx_DEC) × (180/π)
```
L'axe DEC est orienté perpendiculairement au RA : les deux rotations ±90° sont testées et la meilleure est choisie par produit scalaire.

**Angle final retenu :**
```
phi = (phi_RA + phi_DEC) / 2
```

**Matrice de rotation Z construite à partir de phi :**
```
ROT_Z = [ cos(-φ)  -sin(-φ)  0 ]
        [ sin(-φ)   cos(-φ)  0 ]
        [    0         0     1 ]
```

**Taux de pulsation (valeurs fondamentales) :**
```
raPulseMsPerArcsecond  = durée_totale_pulse_RA  / déplacement_RA_en_arcsec
decPulseMsPerArcsecond = durée_totale_pulse_DEC / déplacement_DEC_en_arcsec
```

### Valeurs persistées après calibration

- Angle φ (`calibrationAngle`)
- `raPulseMsPerArcsecond` et `decPulseMsPerArcsecond`
- `calibrationDecSwap` (si l'axe DEC est inversé)
- Position RA/DEC du monture au moment de la calibration
- Pier side au moment de la calibration
- Binning utilisé

---

## 2. Calcul des corrections (`gmath.cpp`)

### Pipeline par frame

**1. Détection de l'étoile** → position en pixels (centroïde ou SEP)

**2. Drift brut en arcseconde :**
```
drift_brut = (pos_étoile − pos_réticule) × arcsec_par_pixel
```

**3. Rotation dans le repère RA/DEC :**
```
in.x =  drift_brut.x
in.y = −drift_brut.y   ← inversion Y (repère image vs repère céleste)
drift_ra_dec = in × ROT_Z
```

**4. Buffer circulaire :** les 50 derniers drifts sont stockés pour le terme intégral.

**5. Correction PID simplifiée (par axe) :**
```
response_P = drift_actuel   × gain_P × ms_par_arcsec
response_I = moy(50_derniers) × gain_I × ms_par_arcsec
durée_pulse = min(|P + I|, max_pulse_ms)
```
Si la correction est inférieure au seuil minimum, aucun pulse n'est envoyé.

**6. GPG (optionnel pour RA) :** si activé, remplace le PID par un modèle prédictif Gaussian Process.

---

## 3. Application des corrections

### Fonctionnement normal

- Les pulses sont émis en millisecondes via le pilote INDI du monture.
- **Premier frame** après démarrage ou slew : pas de pulse, le réticule est centré sur l'étoile (`m_isFirstFrame = true`).

### Changement de cible (slew)

- La calibration est **réutilisée** telle quelle.
- `m_isFirstFrame = true` → première capture sans correction, réticule recentré.
- Guidage reprend normalement dès la 2e frame.

### Flip de méridien (`guideAfterMeridianFlip()`)

**Option 1 — Recalibration complète** (si `resetGuideCalibration` = true) :
- La calibration est effacée, une nouvelle calibration est requise.

**Option 2 — Adaptation de la calibration** (comportement par défaut) :
```
angle_nouveau = angle_calibration + 180°  (modulo 360°)
ROT_Z recalculée avec le nouvel angle
```
Pour le DEC swap :
- Si `NOT reverseDecOnPierChange` → `decSwap = !calibrationDecSwap`
- Sinon → decSwap conservé

### Corrections automatiques de la calibration restaurée

**Déclinaison** si la position actuelle diffère de la position de calibration :
```
ms_par_arcsec_corrigé = ms_par_arcsec_original × cos(dec_actuelle) / cos(dec_calibration)
```

**Binning** si le binning a changé depuis la calibration :
```
binFactor = subBinX_actuel / subBinX_calibration
arcsec_par_pixel_corrigé = arcsec_par_pixel_base × binFactor
```

---

## Tableau récapitulatif

| Étape | Ce qui est calculé | Ce qui est retenu |
|---|---|---|
| **Calibration** | Déplacement angulaire sous pulse | Angle φ, `raPulseMsPerArcsec`, `decPulseMsPerArcsec`, `decSwap`, pier side, RA/DEC, binning |
| **Guidage** | Drift pixel → arcsec → repère RA/DEC via ROT_Z → PID | Pulse en ms envoyé au monture |
| **Pier flip** | angle + 180°, decSwap éventuellement inversé | Calibration mise à jour en mémoire |
| **Slew** | Aucun recalcul | Calibration réutilisée, réticule recentré |
