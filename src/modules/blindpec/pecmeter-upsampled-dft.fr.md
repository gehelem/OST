# pecmeter — remplacer l'estimateur sous-pixel par une DFT suréchantillonnée

*Note de conception. Cible : `src/modules/blindpec/pecmeter.{h,cpp}`.*

---

## 0. État — implémenté sur la branche `blindpec-dftshift` (2026-09-07)

`Meter::upsampledShift()` ajoutée à `pecmeter.cpp`, sélectionnable par
`measParams/dftshift` (défaut **ON** sur la branche) + `measParams/upsamplefactor`
(κ, défaut **50**). Chemin legacy (`phaseCorrelate` + ECC + S-curve) conservé
derrière `dftshift=false`. S-curve court-circuitée quand `dftshift=on`.

**Écart avec le §5 :** l'esquisse `+i` est abandonnée. On suit `dftups` /
skimage à la lettre — noyaux `−i` (`phaseKernel`) — **et** on passe `conj(R)` au
zoom matriciel (`R = F_cur·conj(F_ref)` pour le pic grossier, `conj(R)` pour le
zoom, sinon le pic sort de la grille). Convention module respectée : `shift` =
déplacement du contenu ancre→courante (positif quand la scène va vers +X).

**Banc de validation** (`scratchpad/dfttest.cpp`, hors dépôt) : image bruitée
band-limitée, décalages de Fourier connus.
- propre, κ=50 : erreur pire **0.014 px** (= la quantification `1/κ`), signe et
  échelle corrects.
- + bruit gaussien σ=8 (frais à chaque frame) : |err| moyen **0.072 px**, **biais
  signé +0.018 px** ≈ 0 → pas de pixel-locking résiduel (S-curve `phaseCorrelate`
  brute ≈ 0.66 px p2p pour comparaison).

Build OK (`libostblindpec.so`). Reste à valider sur le banc réel (hardware) :
`observeonly` — `along` lisse ?, corrélation avec `sfrac` ?, RMS, débit frames.

---

## 1. Le problème

L'estimateur actuel empile trois mécanismes qui luttent tous contre le même
défaut — le **pixel-locking** (biais d'attraction du décalage estimé vers les
valeurs entières) :

| Couche | Rôle | Coût |
|---|---|---|
| `preBlur` | élargit le pic de corrélation | 1 paramètre, flou destructeur |
| ECC (`findTransformECC`) | raffinement par interpolation | 4 paramètres, itératif, peut diverger |
| S-curve | mesure le biais résiduel et le soustrait | 3 paramètres, ~155 lignes, calibration au démarrage |

Les trois se règlent **ensemble** : changer `preBlur` invalide la S-curve, activer
ECC change la forme de la S-curve, `eccGaussFilt` se cumule avec `preBlur`. C'est
la source réelle de la complexité, pas le nombre de lignes.

Et le résultat reste un correctif *a posteriori* : on mesure une erreur
systématique, puis on la retranche. Si la texture du CD change (poussière,
condensation, remise en place du microscope), la courbe calibrée devient fausse
et rien ne le signale.

## 2. Le principe de la DFT suréchantillonnée

Référence : Guizar-Sicairos, Thurman & Fienup, *« Efficient subpixel image
registration algorithms »*, Optics Letters 33(2), 2008.
Implémentation de référence : `skimage.registration.phase_cross_correlation`.

L'idée tient en une phrase : **la corrélation croisée n'a pas besoin d'être
calculée sur une grille entière.** On peut l'évaluer directement à des positions
fractionnaires, exactement, par une DFT inverse écrite sous forme de produit
matriciel — sans interpoler quoi que ce soit.

Le pixel-locking vient précisément de l'interpolation : ajuster une parabole sur
trois échantillons entiers, ou raffiner par ECC, ce sont des approximations dont
l'erreur dépend de la partie fractionnaire du décalage. Ici on ne fait aucune
approximation : on échantillonne la vraie corrélation là où on veut, aussi finement
qu'on veut.

### L'algorithme, en trois étapes

1. **Spectre de puissance croisé.** `R = conj(F_ancre) · F_courant`, normalisé
   par son module. C'est exactement ce que fait déjà `phaseCorrelate` en interne.

2. **Pic grossier.** IDFT complète de `R`, `minMaxLoc` → décalage entier
   `(dx₀, dy₀)`. Une seule FFT, comme aujourd'hui.

3. **Zoom local exact.** On évalue la corrélation sur une petite grille de
   `S × S` points (typiquement `S = ceil(1.5 · κ)`) centrée sur `(dx₀, dy₀)`,
   avec un pas de `1/κ` pixel. Le maximum de cette grille donne le décalage à
   `1/κ` près. Avec `κ = 100`, c'est 0.01 px, sans biais fractionnaire.

L'étape 3 s'écrit comme deux produits matriciels :

```
CC = K_lignes · R · K_colonnes
```

avec des noyaux de phase de tailles `S×M` et `N×S`. Le coût est
`O(S·M·N)` au lieu de `O(S²·M·N)` en naïf — négligeable devant la FFT initiale.

## 3. Pourquoi ça convient à BlindPEC

- **Régime facile.** Translation rigide d'une texture statique, images espacées
  d'environ une seconde, corrélation quasi parfaite. C'est le cas nominal de la
  méthode. IPC, à l'inverse, est conçu pour des images *dissemblables*
  (granulation solaire évoluant entre deux poses SDO) — sa complexité
  supplémentaire ne rachète rien ici.
- **Aucune dépendance nouvelle.** OpenCV suffit (`dft`, `idft`, `mulSpectrums`,
  `gemm`). Pas de vendoring, pas de monorepo à découper, pas de question de
  licence.
- **Un seul paramètre de précision.** `upsampleFactor` remplace sept réglages
  couplés. On le monte jusqu'à ce que le banc montre un plateau.
- **Déterministe.** Pas d'itérations, pas de convergence, pas de `try/catch`
  autour d'un solveur qui peut échouer.

## 4. Impact concret sur le code

### 4.1 Ce qui disparaît de `pecmeter.cpp`

| Symbole | Lignes ≈ | Devenir |
|---|---|---|
| `fitHarmonics` | 28 | supprimé |
| `biasEval` | 12 | supprimé |
| `curvePeakToPeak` | 15 | supprimé |
| `Meter::calibrateSCurve` | 55 | supprimé |
| bloc ECC dans `measureShift` | 25 | remplacé |
| `fourierShift` | 45 | **déplacé dans les tests** (voir §6) |

### 4.2 Ce qui reste inchangé

Toute la logique qui fait la valeur du module ne bouge pas :

- ancre / ré-ancrage avec report de l'origine (`dropAnchor`, `_anchorOriginX/Y`)
- portes de confiance (`minResponse`, `maxStepPx`) et non-avancement du cumul
  sur frame rejetée
- `nccAtShift` — le score de confiance reste indépendant de l'estimateur, c'est
  un bon choix, on le garde tel quel
- fenêtre de Hann : **toujours nécessaire** pour tuer le repliement de bord
- l'API publique `update()` / `Sample` / `cumX()` / `cumY()`

### 4.3 En-tête `pecmeter.h`

Champs à retirer : `_biasCoefX`, `_biasCoefY`, `_haveCurve`, `_sCurvePPx`,
`_sCurvePPy`. Méthodes publiques à retirer : `haveSCurve()`,
`sCurvePeakToPeakX()`, `sCurvePeakToPeakY()`.

`_anchorRaw` est **conservé** : il ne servait pas qu'à ECC, `nccAtShift` en a besoin.

### 4.4 Paramètres

```
SUPPRIMÉS (7) :  eccRefine, eccIters, eccEps, eccGaussFilt,
                 sCurve, sCurvePoints, sCurveHarm

AJOUTÉ (1)    :  upsampleFactor   (défaut 100)

CONSERVÉS (5) :  hann (true), preBlur (défaut → 0.0), reanchorFrac,
                 maxStepPx, minResponse
```

**11 → 6 paramètres.** `preBlur` passe à 0 par défaut : il n'est plus là pour
combattre le pixel-locking, seulement pour remonter le SNR sur une image bruitée.
On le laisse exposé, mais neutre.

### 4.5 Répercussions dans `blindpec.cpp` / `blindpec.json`

- `meterParams()` (lignes 130–136) : supprimer les sept mappings ECC/S-curve,
  ajouter `upsamplefactor`.
- Lignes 497–503 : le bloc de log S-curve tombe. Le flag `_sCurveLogged`
  (`blindpec.h`, ligne 97) et sa remise à zéro (ligne 167) disparaissent aussi.
- `blindpec.json` : retirer les blocs `eccrefine`, `ecciters`, `ecceps`,
  `eccgauss`, `scurve`, `scurvepoints`, `scurveharm` ; ajouter `upsamplefactor`
  (int, 10–500, défaut 100) ; corriger le `hint` de `preblur` qui mentionne le
  pixel-locking.

## 5. Code de référence

> Esquisse à valider sur le banc avant intégration. **Le piège classique est la
> convention de signe** des noyaux de phase et l'ordre `conj(ancre)·courant` vs
> l'inverse : une erreur donne un décalage exact mais opposé. Le banc du §6 le
> détecte immédiatement — un signe inversé produit une droite de pente −1.
> Deux implémentations de référence permettent de trancher sans tâtonner :
> `_upsampled_dft` de scikit-image et `dftups.h` de `aystarik/splitrad` (§9).

```cpp
namespace {

/// Noyau de phase pour l'évaluation de la DFT inverse à des positions
/// fractionnaires. Renvoie une matrice CV_64FC2 de taille (S x n).
///   K[s, k] = exp( +i·2π · f(k) · (s - offset) / (n · kappa) )
/// où f(k) est l'indice de fréquence signé de la colonne/ligne k.
cv::Mat phaseKernel(int n, int S, double kappa, double offset)
{
    cv::Mat K(S, n, CV_64FC2);
    for (int s = 0; s < S; ++s)
    {
        cv::Vec2d *row = K.ptr<cv::Vec2d>(s);
        const double pos = s - offset;
        for (int k = 0; k < n; ++k)
        {
            const double f  = (k <= n / 2) ? k : k - n;   // fréquence signée
            const double ph = 2.0 * CV_PI * f * pos / (n * kappa);
            row[k] = cv::Vec2d(std::cos(ph), std::sin(ph));
        }
    }
    return K;
}

/// Produit matriciel complexe A(p x q) · B(q x r) -> (p x r), tout en CV_64FC2.
cv::Mat cgemm(const cv::Mat &A, const cv::Mat &B)
{
    cv::Mat out;
    cv::gemm(A, B, 1.0, cv::noArray(), 0.0, out);   // gemm gère CV_64FC2
    return out;
}

} // namespace

/// Décalage sous-pixel de `cur` par rapport à `ref`, par DFT suréchantillonnée.
/// `ref` et `cur` : CV_32F, même taille, déjà fenêtrés (Hann).
cv::Point2d Meter::measureShift(const cv::Mat &ref, const cv::Mat &cur) const
{
    const int M = ref.rows, N = ref.cols;

    // --- 1. spectre de puissance croisé, normalisé -------------------------
    cv::Mat Fr, Fc;
    cv::dft(ref, Fr, cv::DFT_COMPLEX_OUTPUT);
    cv::dft(cur, Fc, cv::DFT_COMPLEX_OUTPUT);

    cv::Mat R;
    cv::mulSpectrums(Fc, Fr, R, 0, /*conjB=*/true);   // conj(ref) * cur

    // normalisation par le module -> corrélation de phase pure
    {
        std::vector<cv::Mat> ch(2);
        cv::split(R, ch);
        cv::Mat mag;
        cv::magnitude(ch[0], ch[1], mag);
        mag += 1e-12;
        cv::divide(ch[0], mag, ch[0]);
        cv::divide(ch[1], mag, ch[1]);
        cv::merge(ch, R);
    }
    R.convertTo(R, CV_64FC2);

    // --- 2. pic grossier, précision entière --------------------------------
    cv::Mat cc;
    cv::idft(R, cc, cv::DFT_REAL_OUTPUT | cv::DFT_SCALE);
    cv::Point peak;
    cv::minMaxLoc(cc, nullptr, nullptr, nullptr, &peak);

    // ramener dans [-M/2, M/2) x [-N/2, N/2)
    double dy0 = peak.y, dx0 = peak.x;
    if (dy0 > M / 2) dy0 -= M;
    if (dx0 > N / 2) dx0 -= N;

    // --- 3. zoom local exact autour du pic ---------------------------------
    const double kappa = std::max(1.0, _params.upsampleFactor);
    const int    S     = static_cast<int>(std::ceil(1.5 * kappa));
    const double dc    = std::floor(S / 2.0);          // centre de la fenêtre

    // On veut échantillonner autour de (dx0, dy0) : l'offset place le pic
    // grossier au centre de la grille suréchantillonnée.
    const cv::Mat Krow = phaseKernel(M, S, kappa, dc + dy0 * kappa);
    const cv::Mat Kcol = phaseKernel(N, S, kappa, dc + dx0 * kappa);

    cv::Mat ccUp = cgemm(Krow, cgemm(R, Kcol.t()));    // S x S, complexe

    cv::Mat ccAbs;
    {
        std::vector<cv::Mat> ch(2);
        cv::split(ccUp, ch);
        cv::magnitude(ch[0], ch[1], ccAbs);
    }
    cv::Point up;
    cv::minMaxLoc(ccAbs, nullptr, nullptr, nullptr, &up);

    return cv::Point2d(dx0 + (up.x - dc) / kappa,
                       dy0 + (up.y - dc) / kappa);
}
```

**Raffinement optionnel, plus tard :** ajuster une parabole 2D sur le voisinage
3×3 de `ccAbs` autour de `up` pour descendre sous `1/κ`. Inutile si `κ = 100` :
0.01 px est déjà loin sous le bruit.

**Passe-bande optionnelle, plus tard :** l'idée que IPC apporte réellement est le
filtrage passe-bande de `R` avant l'étape 2 (`stdevL` / `stdevH`), qui rejette le
bruit haute fréquence et les gradients d'éclairage basse fréquence. C'est un
masque gaussien de dix lignes appliqué sur `R`. À garder en réserve **si et
seulement si** le banc montre que le bruit domine.

## 6. Le banc de validation — à écrire avant tout le reste

`fourierShift()` ne doit surtout pas être supprimé. C'est un générateur de
décalages sous-pixel **exacts et sans biais d'interpolation** (multiplication du
spectre par une rampe de phase linéaire). C'est l'instrument de mesure idéal pour
départager les estimateurs. Il migre de `pecmeter.cpp` vers un fichier de test.

### Protocole

1. Prendre **une vraie frame microscope** (pas une image de synthèse : la
   statistique de texture du CD poncé compte).
2. Générer les copies décalées de `d = 0.00, 0.02, … 0.98` px sur X, puis sur Y.
3. Pour chaque estimateur, tracer l'**erreur** `mesuré − d` en fonction de `d`.
4. Reporter : amplitude crête-à-crête du biais, écart-type résiduel, temps par
   frame.

### Estimateurs à comparer

| # | Configuration |
|---|---|
| A | `phaseCorrelate` seul (baseline, pixel-locking maximal) |
| B | A + `preBlur = 1.0` |
| C | B + ECC (la configuration actuelle par défaut) |
| D | C + S-curve (l'état de l'art du module aujourd'hui) |
| E | **DFT suréchantillonnée, κ = 10 / 50 / 100** |

### Critère de décision

Le remplacement est justifié si **E ≥ D** en biais crête-à-crête, à coût CPU
comparable ou inférieur. Si E domine, on supprime B/C/D avec les 155 lignes qui
vont avec. Si E est seulement équivalent, on remplace quand même : six paramètres
au lieu de onze et aucune calibration au démarrage, c'est déjà le gain
recherché.

### Le test qui compte vraiment

Ajouter un bruit gaussien réaliste aux copies décalées (estimé sur deux frames
consécutives réelles, microscope immobile). Un estimateur parfait sur image propre
et fragile sur image bruitée ne sert à rien en conditions nocturnes.

### Question ouverte à trancher en même temps

**Quel est le budget d'erreur réel ?** Si `arcsecPerPx ≈ 0.5"/px`, alors 0.05 px
de pixel-locking font 0.025" — largement sous le seeing et sous le bruit mécanique
de la monture. Dans ce cas l'estimateur n'est pas le goulot d'étranglement, et le
banc le montrera : les courbes A à E se ressembleront toutes une fois le bruit
ajouté. Les suspects seraient alors ailleurs — jeu de la vis sans fin, planéité et
excentricité du CD, vibrations, latence entre la mesure et l'impulsion. Le
remplacement resterait justifié pour la simplification, mais il ne faudrait pas
en attendre un gain de RMS.

## 7. Plan de migration

| Étape | Contenu | Réversible |
|---|---|---|
| 1 | Extraire `fourierShift` vers `test/pecmeter_bench.cpp`, écrire le banc, mesurer A→D | oui, aucun changement fonctionnel |
| 2 | Implémenter `upsampledShift()` **à côté** de `measureShift()`, l'ajouter au banc comme configuration E | oui |
| 3 | Décider sur les chiffres | — |
| 4 | Basculer `measureShift()`, garder l'ancien chemin derrière un booléen `legacyEstimator` | oui |
| 5 | Une nuit de guidage réel en comparant les RMS | oui |
| 6 | Supprimer le chemin legacy, les 7 paramètres, les blocs JSON, le log S-curve | non |

Ne pas sauter l'étape 1. Le banc est réutilisable indéfiniment et c'est lui qui
transforme le choix d'estimateur en question tranchée plutôt qu'en intuition.

## 8. Risques

- **Convention de signe / `fftshift`.** Le piège n°1. Détecté immédiatement par
  le banc (pente −1 au lieu de +1, ou décalage constant de `M/2`).
- **Mémoire.** `κ = 100` sur une frame 640×480 donne des noyaux de
  150×480 et 150×640 en `CV_64FC2` — quelques Mo, sans problème. Attention si la
  résolution du microscope monte beaucoup.
- **`cv::gemm` et `CV_64FC2`.** Vérifier le support des matrices complexes dans
  la version d'OpenCV visée ; sinon, écrire le produit complexe à la main
  (la boucle est triviale et les tailles sont petites).
- **Ce qui ne changera pas.** Les portes de confiance, le ré-anchorage et le
  `TODO` non résolu du contrôle croisé de ré-anchorage (`pecmeter.cpp`, fin de
  `update()`) restent exactement les mêmes problèmes qu'aujourd'hui.

## 9. Implémentations de référence

Aucune de ces bibliothèques n'est à intégrer. Elles servent d'**oracles** : à
consulter pendant l'écriture de `phaseKernel()` et à comparer numériquement sur
le banc.

| Projet | Nature | Verdict |
|---|---|---|
| `skimage.registration.phase_cross_correlation` | Python/NumPy, BSD-3 | **Oracle principal.** Lire `_upsampled_dft`. Permet de générer des valeurs attendues pour les tests unitaires. |
| `aystarik/splitrad` (`dftups.h`) | C++ pur, FFT maison | **Oracle C++.** Même algorithme, écrit indépendamment. Non intégrable (voir ci-dessous). |
| `zdenyhraz/shenanigans` (IPC) | C++/OpenCV, GPL-3.0 | Réserve pour la passe-bande uniquement, si le bruit domine. |
| `zdenyhraz/IPC` | Exécutable Windows 2019 | Inexploitable : pas de build system, pas de licence. |

### `aystarik/splitrad` — pourquoi le lire, pourquoi ne pas le prendre

Le fichier `dftups.h` porte le nom même de la fonction du code MATLAB de référence
de Guizar-Sicairos. C'est exactement l'algorithme du §5, écrit indépendamment,
en C++ sans dépendance externe.

**À lire pour :**

- **La convention de signe et l'ordre des indices** dans la construction des
  noyaux de phase. C'est le point de blocage annoncé au §5 ; ce fichier le
  tranche.
- **`matmultmax()`** — fusionne le second produit matriciel et l'argmax : la
  grille suréchantillonnée n'est jamais matérialisée en mémoire, seul le maximum
  courant est retenu. Optimisation à envisager en phase 2 si le profilage le
  justifie.
- **La table `twiddle`** — LUT sin/cos sur `N·K` points exploitant les symétries
  de quadrant, avec une indexation par masquage de bits sans branchement.
  Élégant, mais sans objet ici : OpenCV fait déjà ce travail.

**À ne pas intégrer parce que :**

- **Pas de fichier de licence.** Quatre commits, README d'une ligne. Sans licence
  explicite, tous droits réservés — incompatible avec la GPL-3.0 d'ostserver.
  Bloquant, sauf accord écrit de l'auteur.
- **`template <int N, int K>`** — la taille d'image est une constante de
  compilation, **carrée et puissance de deux** (le masquage `maxi & (N-1)` et la
  FFT split-radix en dépendent). Imposerait un recadrage des frames microscope.
- **Région suréchantillonnée en N×N** au lieu de `ceil(1.5·κ)` : `matmult<N>` est
  en O(N³), soit ~33 M multiplications complexes par frame à N=256. Tenable à
  1 Hz, mais deux ordres de grandeur au-dessus du nécessaire.
- **Allocation pile** — `std::complex<T> kern[N*N], temp[N*N]` dans `apply()` :
  1 Mo à N=256, 4 Mo à N=512. Dangereux dans un thread Qt à pile réduite.
- **Simple précision** par défaut, et aucune logique d'ancre, de porte de
  confiance ni de score — c'est l'algorithme nu, pas un `Meter`.

**Conclusion :** OpenCV est déjà une dépendance dure de `ostblindpec`
(`CMakeLists.txt`, `find_package(OpenCV REQUIRED)`, lignes 33 et 440–448). La FFT
maison de splitrad résout un problème que nous n'avons pas. On garde
l'implémentation OpenCV à région réduite du §5, avec `dftups.h` ouvert à côté
pendant l'écriture.

## 10. Références

- Guizar-Sicairos, Thurman, Fienup (2008), *Efficient subpixel image registration
  algorithms*, Optics Letters 33(2), 156–158.
- `skimage.registration.phase_cross_correlation` — implémentation de référence
  lisible, notamment la fonction interne `_upsampled_dft`.
- `github.com/aystarik/splitrad`, fichier `dftups.h` — implémentation C++ du même
  algorithme, sans dépendance externe. Oracle pour les conventions de signe.
  Voir §9 pour les réserves.
- Hrazdira, Druckmüller, Habbal (2020), *Iterative Phase Correlation Algorithm for
  High-precision Subpixel Image Registration*, ApJS 247(1). Source à jour dans
  `github.com/zdenyhraz/shenanigans` (GPL-3.0, compatible avec ostserver). À
  consulter pour la partie passe-bande si le bruit s'avère dominant. Le dépôt
  `zdenyhraz/IPC` est un instantané de 2019 (exécutable Windows + archives, sans
  build system ni licence) et n'est pas exploitable comme bibliothèque.
