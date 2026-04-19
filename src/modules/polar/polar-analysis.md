# Analyse : alignement polaire à trois clichés

Comparaison entre l'implémentation KStars/Ekos (`polaralign.cpp`, `poleaxis.cpp`, `rotations.cpp`)
et l'implémentation OST (`polar.cpp`).

---

## 1. Algorithme correct (KStars/Ekos)

### Principe géométrique

On prend trois clichés du ciel avec une rotation de la monture en RA entre chaque.
Le centre de chaque image trace un arc de cercle sur la sphère céleste.
Le plan de cet arc est perpendiculaire à l'axe RA de la monture.
La normale à ce plan est donc la direction de l'axe RA.

Si la monture est bien alignée, cet axe pointe vers le pôle céleste, qui est lui-même
dans la direction du pôle géographique (azimuth=0°, altitude=latitude).

### Chaîne de conversions (critique)

Pour chaque cliché, le chemin est :

```
Pixel image
  → WCS (astrométrie / StellarSolver)
  → RA/DEC J2000 (degrés, repère inertiel)
  → RA/DEC apparent (époque actuelle, J2000 → observé via précession/nutation)
  → Azimuth/Altitude (repère horizontal, lié à la Terre)
  → Vecteur unitaire xyz  [azAlt2xyz]
```

Les étapes de conversion sont **indispensables**. Travailler en RA/DEC J2000 est incorrect
parce que ce repère n'est pas lié à la rotation de la Terre. L'axe RA de la monture
est fixe dans le repère horizontal (Az/Alt), pas dans le repère céleste J2000.

### Calcul de l'axe et de l'erreur

```
p0, p1, p2 = trois vecteurs xyz (Az/Alt)
axis = getAxis(p0, p1, p2)   → normale au plan = direction de l'axe RA
   (= produit vectoriel normalisé)

azAlt_axis = xyz2azAlt(axis)
  → azAlt_axis.x() = azimuth de l'axe  (erreur azimutale, devrait être 0°)
  → azAlt_axis.y() = altitude de l'axe (devrait être égale à la latitude géographique)

erreur_azimut   = azAlt_axis.x()
erreur_altitude = azAlt_axis.y() - latitude_observateur
```

L'erreur en altitude est la différence entre l'altitude réelle de l'axe et la latitude,
pas le complément à 90°.

### Hémisphère

- Hémisphère nord : axe.x() doit être > 0 (pointe vers le nord). Si < 0, inverser le vecteur.
- Hémisphère sud : axe.x() doit être < 0 (pointe vers le sud). Si > 0, inverser le vecteur.

---

## 2. Analyse critique de polar.cpp

### Bug 1 — Mauvais espace de coordonnées dans SMCompute (lignes 377-401)

**Ce que fait le code :**
```cpp
_ra0 = _solver.stellarSolver.getSolution().ra;    // J2000, degrés (0..360)
_de0 = _solver.stellarSolver.getSolution().dec;   // J2000, degrés
```

**Ce que SMComputeFinal en fait (lignes 442-444) :**
```cpp
Rotations::V3 p0(Rotations::azAlt2xyz(QPointF(_ra0, _de0)));
```

**Problème :** `azAlt2xyz` attend un azimuth et une altitude en degrés dans le repère horizontal.
On lui passe à la place RA et DEC J2000. Ce sont deux repères complètement différents :
- RA J2000 est une coordonnée céleste inertielle (ne tourne pas avec la Terre)
- Azimuth dépend de la position de l'observateur et de l'heure

Le résultat est un vecteur xyz sans signification physique pour le calcul d'axe.

### Bug 2 — Absence de conversion de coordonnées

La chaîne correcte est : J2000 → apparent (époque actuelle) → Az/Alt.

Le code commenté (lignes 374, 385, 395) fait `ObservedToJ2000`, c'est-à-dire
la **conversion inverse** (EOD→J2000), alors qu'il faudrait `J2000toObserved` puis
la conversion équatoriale→horizontal.

**Ce qu'il faudrait faire pour chaque point :**
```cpp
// 1. Solver donne J2000 en degrés
double ra_j2000_deg  = _solver.stellarSolver.getSolution().ra;
double dec_j2000_deg = _solver.stellarSolver.getSolution().dec;

// 2. J2000 → apparent (RA en heures pour INDI)
INDI::IEquatorialCoordinates j2000_coord;
j2000_coord.rightascension = ra_j2000_deg * 24.0 / 360.0;  // heures
j2000_coord.declination    = dec_j2000_deg;

INDI::IEquatorialCoordinates eod_coord;
double jd = ln_get_julian_from_sys();
INDI::J2000toObserved(&j2000_coord, jd, &eod_coord);
// eod_coord.rightascension = RA apparent (heures), eod_coord.declination = DEC apparent (degrés)

// 3. Équatorial → Horizontal (libnova)
ln_equ_posn equ;
equ.ra  = eod_coord.rightascension * 360.0 / 24.0;  // degrés pour libnova
equ.dec = eod_coord.declination;

ln_hrz_posn hrz;
double lst = ... ; // temps sidéral local
double ha  = lst - equ.ra;
ln_get_hrz_from_equ(&equ, &observer_location, ln_get_julian_from_sys(), &hrz);
// hrz.az = azimuth (degrés), hrz.alt = altitude (degrés)

// 4. Maintenant on peut appeler azAlt2xyz
Rotations::V3 p(Rotations::azAlt2xyz(QPointF(hrz.az, hrz.alt)));
```

*Note : libnova est disponible dans l'environnement de build (libindi dépend de libnova).*

### Bug 3 — Formule d'erreur en altitude (ligne 472)

**Code actuel :**
```cpp
_erralt = 90 - azAlt.y();   // FAUX
```

**Correct :**
```cpp
double latitude = getObserverLatitude();  // latitude de l'observateur en degrés
_erralt = azAlt.y() - latitude;           // différence entre altitude de l'axe et latitude
```

`90 - azAlt.y()` donne la co-altitude de l'axe, c'est-à-dire l'angle entre l'axe et le
zénith. Ce n'est pas l'erreur d'alignement. Une monture parfaitement alignée a son axe
pointant vers l'altitude = latitude (pas altitude = 90°).

### Bug 4 — Détection d'hémisphère incomplète (ligne 460)

**Code actuel :**
```cpp
if (axis.x() < 0)
{
    axis = Rotations::V3(-axis.x(), -axis.y(), -axis.z());
}
```

**Correct (KStars) :**
```cpp
bool northernHemisphere = (latitude >= 0);
if ((northernHemisphere && axis.x() < 0) || (!northernHemisphere && axis.x() > 0))
{
    axis = Rotations::V3(-axis.x(), -axis.y(), -axis.z());
}
```

En hémisphère sud, l'axe RA pointe vers le pôle sud, dont la composante x dans le
repère de `azAlt2xyz` est positive (l'axe x pointe vers l'avant / nord). Il faut donc
inverser le test.

### Problème mineur — Timestamps non utilisés

Les timestamps `_t0`, `_t1`, `_t2` sont enregistrés mais jamais utilisés dans les calculs.
En principe on pourrait corriger la rotation sidérale entre les trois poses (la Terre tourne
pendant le temps d'acquisition), mais c'est une correction de second ordre, négligeable
pour des temps typiques de quelques minutes. KStars ne le fait pas non plus.

---

## 3. Résumé des corrections à apporter

| # | Où | Problème | Correction |
|---|---|---|---|
| 1 | `SMComputeFinal` lignes 442-444 | `azAlt2xyz` reçoit RA/DEC J2000 | Passer Az/Alt après conversion complète |
| 2 | `SMCompute` lignes 377-401 | Pas de conversion J2000 → Az/Alt | Ajouter J2000 → apparent → Az/Alt |
| 3 | `SMComputeFinal` ligne 472 | `90 - azAlt.y()` | `azAlt.y() - latitude` |
| 4 | `SMComputeFinal` ligne 460 | Test hémisphère incomplet | Conditionner sur `latitude >= 0` |

### Dépendances nécessaires

- `ln_get_julian_from_sys()` — libnova (déjà disponible)
- `INDI::J2000toObserved()` — libindi (déjà disponible)
- `ln_get_hrz_from_equ()` — libnova, pour la conversion équatorial → horizontal
- Latitude/longitude de l'observateur — à récupérer depuis la monture (`GEOGRAPHIC_COORD`)

---

## 4. Architecture suggérée

Créer une méthode `solverToAzAlt(double ra_j2000_deg, double dec_j2000_deg, double jd) → QPointF(az, alt)` qui encapsule toute la chaîne de conversion, puis l'appeler dans `SMCompute` à la place de la copie directe des coordonnées solver.

```cpp
QPointF Polar::solverToAzAlt(double ra_j2000_deg, double dec_j2000_deg, double jd)
{
    // J2000 → apparent
    INDI::IEquatorialCoordinates j2000, eod;
    j2000.rightascension = ra_j2000_deg * 24.0 / 360.0;
    j2000.declination    = dec_j2000_deg;
    INDI::J2000toObserved(&j2000, jd, &eod);

    // Apparent → horizontal
    ln_equ_posn equ { eod.rightascension * 360.0 / 24.0, eod.declination };
    ln_lnlat_posn obs { _observerLongitude, _observerLatitude, 0 };
    ln_hrz_posn  hrz;
    ln_get_hrz_from_equ(&equ, &obs, jd, &hrz);

    return QPointF(hrz.az, hrz.alt);
}
```

Et stocker `_az0/_alt0`, `_az1/_alt1`, `_az2/_alt2` à la place de `_ra0/_de0` etc.,
ou conserver les coordonnées J2000 pour l'affichage et calculer Az/Alt à la demande dans
`SMComputeFinal`.
