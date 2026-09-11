# BlindPEC — notes de conception (reprise du vieux module)

Notes de travail pour pérenniser `OST-modules/src/modules/blindpec/` et le porter
sur les derniers headers/bibliothèques OST. On complète ce fichier au fur et à
mesure.

---

## 1. Principe fonctionnel

Ce n'est **pas** du guidage stellaire. Un petit **microscope USB** (reconnu comme
webcam) filme la **surface de l'axe d'ascension droite** de la monture, sur un
diamètre suffisant (rayon R ≈ 100–150 mm). OpenCV mesure le défilement de cette
surface texturée : ce défilement **est** la rotation de l'axe.

→ On obtient un **encodeur optique haute résolution sur l'axe d'AD** pour ~20 €.

On peut alors :
1. constater les écarts d'erreur périodique (vis sans fin) ;
2. corriger la dérive en ascension droite par pulses E/W.

Équivaut à un encodeur haute résolution, à coût ridicule.

**DEC non corrigée** : monture bien mise en station, ce qu'un guider classique
verrait en DEC dans cette situation n'est que de la turbulence. BlindPEC ne
traite qu'un seul axe (AD).

Résultats déjà obtenus avec le prototype : **très prometteurs**. Objectif :
fiabiliser et rendre réellement utilisable.

---

## 2. Modèle mathématique

Position mesurée dans l'image du microscope :

```
p(t) = V·t + e(t) + n(t)
```

| terme | sens |
|---|---|
| `V·t`  | rampe constante = rotation horaire voulue (sidérale). C'est la constante `pixsec`. |
| `e(t)` | erreur périodique (vis sans fin), **moyenne nulle sur une période de VSF `T_vsf`**. |
| `n(t)` | bruit : corrélation + turbulence résiduelle + marche aléatoire des ré-ancrages. |

Guider en aveugle = tenir `e(t) + n(t) → 0`, c.-à-d. asservir `dp/dt` sur `V`.

---

## 3. Calibration initiale

### Deux constantes distinctes — ne pas confondre

**G — le gain** : px de l'image par ms de pulse (ou px/arcsec).
- Mesurable en aveugle, trivial : pulse E de P ms → saut Δ px mesuré → `G = Δ/P`.
- = calibration 1 axe d'un guider classique, mais sur l'image du microscope.

**V — la pente de consigne** : px/s quand le suivi est parfait.
- La partie difficile : **toute mesure courte est polluée par `e(t)`**. Ajuster
  la pente sur un nombre non entier de périodes de VSF → PE non compensée →
  pente biaisée. (C'est ça qui a toujours été fait à tâtons via `pixsec`.)

### Méthodes pour V (du plus simple au meilleur)

1. **La déduire de G.** Si la monture expose `GUIDE_RATE` (k × sidéral, souvent
   0.5) : un pulse P ms tourne l'axe de `k·ω_sid·P`, déplace l'image de Δ px.
   ```
   V = ω_sid · Δ/(k·ω_sid·P) = Δ / (k·P)
   ```
   V sort de la **même** mesure que G, sans jamais observer de rampe libre. Il
   faut juste `k`. `ω_sid = 2π / 86164.09 s = 7.2921e-5 rad/s`.

2. **Ajustement robuste sur rampe libre.** Capture non guidée ≥ 3–5 périodes de
   VSF ; régression de `p(t)` cumulé vs `t` par Theil–Sen ou moindres carrés
   totaux (robuste aux frames où la corrélation a sauté). Fenêtre idéale =
   multiple entier de `T_vsf`. Biais PE décroît en `1/N_périodes`.

3. **Moyenne sur période de VSF.** `V = Δp sur N·T_vsf / (N·T_vsf)`. La PE de
   moyenne nulle s'annule exactement. `T_vsf` : doc monture, ou
   autocorrélation / FFT de `p(t)` détendancé.

4. **Adaptatif — supprime le tâtonnement.** Amorce avec (1) ou une louche.
   Lancer la boucle. Surveiller le **biais moyen de correction (E−W)** sur une
   fenêtre glissante d'une période de VSF : biais persistant `b` → `V += α·b`.
   Convergence en quelques périodes. Le feed-forward `V` s'auto-cale ;
   l'intégrateur ne se bat plus que contre la PE.

**Retenu en pratique : (1) pour l'amorce + (4) pour l'entretien.**

### Piège de fond : axe ≠ ciel

Le microscope voit l'**axe**, pas le ciel. Il ne peut tenir que le taux moyen
*propre* de la monture. Si ce taux moyen ≠ sidéral vrai (quartz, réglage
King/lunaire, réfraction), l'aveugle ne le verra jamais.
→ **une seule** mesure référencée ciel (drift sur étoile ~10 min, ou courte
session guidée classique) pour fixer l'absolu ; ensuite aveugle indéfiniment,
tant que la mécanique ne bouge pas.

### Bonus : table PEC verrouillée en phase

Résidu `e(t)` détendancé → FFT → période de VSF + harmoniques → table PEC
verrouillée en phase. La boucle devient prédictive (rejouer la PE connue) +
petite correction de trim. Nécessite une référence de phase de la VSF (index INDI
`PEC`, ou phase libre asservie sur la composante périodique).

---

## 4. Flux d'image

### Décision : photos séquencées via INDI

Le prototype lit un **flux vidéo natif OpenCV en dur** (`VideoCapture`, thread
`TAcquisitionVideo`), sans passer par INDI (les essais INDI sont en commentaire).
Pour pérenniser : **INDI obligatoire** (support futur des caméras astro
sensibles / petits pixels).

Choix arrêté : **capture photo séquencée** (`requestCapture` → BLOB), pas le flux
vidéo INDI (`CCD_VIDEO_STREAM`).

### Le vrai enjeu : le temps, pas le débit

L'estimation repose sur `p(t) = V·t + e(t)` → connaître `t` de chaque frame au
ms près est vital.

**Trois poisons du flux vidéo pour ce modèle :**
1. **Frames droppées silencieuses** : `Δt` cru faux → saut dans `V·t` → faux pic
   de PE.
2. **Timestamp = réception, pas intégration** : gigue de dizaines à centaines de
   ms (buffering USB, file driver), non mesurable a posteriori.
3. **Expo auto** : la corrélation rend la position au **milieu** de pose, instant
   effectif = `t_start + t_exp/2` ; si `t_exp` varie, les timestamps effectifs
   ondulent. → tout en manuel : expo fixe, gain fixe, aucune régulation auto.

**Ce que la photo séquencée donne :**
- chaque capture = événement explicite ; un échec est une **erreur gérée** (trou
  franc), pas un `Δt` corrompu en douce ;
- `DATE-OBS` (début de pose) dans l'en-tête FITS + `t_exp` connu →
  **instant effectif de l'échantillon = `DATE-OBS + t_exp/2`**, jamais une
  cadence supposée ;
- FITS brut 16 bits, pas de MJPEG (le flux vidéo webcam passe souvent par JPEG →
  grille 8×8 d'artefacts mobiles → plancher de bruit de corrélation).

### Flou de bougé (mineur)

Surface à ~9 µm/s : pose 30 ms → 0,27 µm ; pose 500 ms → 4,5 µm (~2 px à
2,4 µm/px). Filé symétrique dans l'axe du mouvement : élargit le pic de
corrélation, dégrade un peu le sous-pixel dans cette direction, sans casser la
mesure. → poses courtes **10–50 ms**.

### Débit / ROI / binning

PE : période ≥ 100 s, spectre utile < ~0,2 Hz. **1–5 Hz d'échantillonnage
suffit largement**, même pour bâtir une table PEC. Aucun besoin de 30–60 fps.

Caméra astro petits pixels en vidéo pleine trame ≈ 240 Mo/s → USB mort pour
rien. Réponse : **ROI configurable (256²–512²) + binning 2×2 / 3×3** sur la zone
texturée. Binné à 1–2 µm effectifs → toujours dix fois plus fin qu'un encodeur
de monture, meilleure SNR, volume écroulé.

### Architecture d'acquisition retenue

- **Boucle = photo séquencée INDI.** Expo courte fixe, gain fixe, ROI + binning
  en paramètres, timestamp pris dans l'en-tête **frame par frame**.
- **Vidéo = backend optionnel #2**, derrière la même interface, réservé à la
  caractérisation large bande de la PE / diagnostic vibrations, et seulement avec
  un driver qui streame en brut avec de vrais timestamps par frame. L'asservissement
  ne se construit pas dessus.
- **Découpler mesure et commande** : pipeline capture→corrélation→`(t, p)` aussi
  rapide que possible ; loi de commande envoie les pulses E/W à 0,2–0,5 Hz
  seulement ; **blanker 1–2 échantillons** pendant/après un pulse pour ne pas
  mesurer le transitoire.
- **`p(t)` par frame d'ancrage**, pas par intégration frame-à-frame (marche
  aléatoire). Ré-ancrage sous-pixel quand le recouvrement passe sous un seuil,
  avec report de la fraction, et **vérification croisée** de chaque ré-ancrage
  (corréler ancienne vs nouvelle ancre dans le recouvrement, comparer au delta
  accumulé ; désaccord > seuil → frame rejetée).

---

## 5. Mesure du déplacement

### Subpixel : OpenCV le fait, selon la fonction

| Fonction | Subpixel |
|---|---|
| `matchTemplate` + `minMaxLoc` | **Non** (grille entière). Fit parabolique 3 pts/axe requis → c'est `CV_SubPix.cpp`. Pic `TM_CCOEFF` plat/asymétrique sur données entières = source des échecs passés. |
| `phaseCorrelate` (+ `createHanningWindow`) | **Oui, natif.** Cross-power spectrum → IFFT → centroïde pondéré 5×5 autour du pic → `Point2d` + `response`. Fait pour la translation rigide plein champ. |
| `findTransformECC` (`MOTION_TRANSLATION`) | **Oui, natif.** Optim. itérative du coeff. de corrélation, **normalisé → robuste éclairage**. Extensible `MOTION_EUCLIDEAN` si axe non aligné au capteur. |
| `calcOpticalFlowPyrLK`, `cornerSubPix` | Oui, mais suivi de points, pas translation globale. |

### Précision atteignable

Registration subpixel sur image bien texturée + bonne SNR : **0,01–0,05 px RMS**
réaliste.

```
1 µm/px · 0,02 px = 0,02 µm  ;  R ≈ 125 mm
→ 0,02 µm / 125 000 µm = 1,6e-7 rad ≈ 0,033″
```
Conservateur (0,05 px, 2 µm/px, R = 100 mm) → ~0,2″. **Classe encodeur**
(Renishaw haut de gamme ~0,04″, retrofits éco ~0,3–1″). PE à compenser : 5–30″.

### Géométrie retenue : surface perpendiculaire à l'axe

Le microscope regarde une **face plane perpendiculaire à l'axe AD** (un CD
enfoncé en force sur l'axe, poncé au papier grain 320 pour une texture
analysable), **pas** la surface cylindrique.

Conséquences :

- **Faux-rond radial** : hors sujet (translate le motif de quelques µm, pas de
  couplage à la rotation).
- **Planéité + orthogonalité surface/axe = nuisance dominante, mais
  spectralement quarantainée de la PE.** Le disque tourne à la sidérale →
  **1 tour = 1 jour sidéral (86 164 s)**. Défaut de perpendicularité (voile α) →
  oscillation axiale `r·α·sin(φ)` **à 1/tour** (défoc + respiration de
  grandissement + léger wobble plan) → période ~24 h. Défaut de planéité
  (gauchissement) → **2/tour**, période ~12 h. Sur une session de quelques
  heures : **dérive lente légèrement courbée** → pollue l'estimation de `V`, pas
  la PE (100–600 s). **Le `V` adaptatif (§3 méthode 4) les absorbe** si sa
  constante de temps ≈ quelques périodes de VSF. Atout de conception.
- **Mouvement tangentiel en arc** : sur une période de VSF, flèche `r·θ²/8`
  < nm → **localement une droite**, aucune correction géométrique.
- **Gain** = `r · θ · (échelle px)`. `r` grand → meilleure résolution ;
  `r ≲ 50 mm` sur un CD (FOV doit rester sur le disque). À `r = 50 mm` +
  sidéral : vitesse surface ≈ **3,6 µm/s** → **moins de ré-ancrages** que le cas
  cylindre.
- **Texture grain 320** ≈ rayures 40–50 µm : viser des features de **3–10 px**
  dans l'image (grain 600–800 si trop gros, ou baisser le grandissement).

**Watch-items résiduels :**
1. **Jeu axial (end-float) de l'axe AD** : une composante axiale *à la période de
   la VSF* aliaserait dans la PE. À vérifier sur la monture cible.
2. **Adhérence du CD sur l'axe** : serrage force → zéro glissement sous couple
   (sinon fausse rotation injectée directement). Goutte de colle en sécurité.

### Le vrai plancher n'est pas l'algo

1. **Couplage mécanique** : flexion du support microscope, vibration, respiration
   MAP, dilatation. Tout micro-mouvement microscope↔axe est indiscernable d'une
   rotation. Les défauts disque (ci-dessus) sont lents → absorbés par `V`. Le
   couplage support/vibration reste **LA limite** dans la bande PE.
2. **Marche aléatoire des ré-ancrages** : 0,02 px/frame sur 1000 frames → 0,6 px.
   Approche par ancre, mais chaque ré-ancrage réinjecte ~0,02 px ; sur une
   période de VSF, quelques à quelques dizaines de ré-ancrages → bruit cumulé
   ~0,05–0,15 px. **Terme d'erreur dominant côté algo.**
3. Texture (features 3–10 px ≈ optimal), éclairage stable, filé / rolling shutter
   (planchers mineurs).

### Pixel-locking — constaté sur le banc (22:22), corrigé

Le sous-pixel de `phaseCorrelate` = **centroïde pondéré 5×5** du pic de
corrélation → **biais en S (peak-locking)**, fonction de la partie fractionnaire
du décalage, période 1 px. Comme le décalage vs l'ancre croît de ~1 px/frame, ce
biais **change de signe à chaque frame** → créneau alterné ±0,25 px. Sur la
courbe PE `observeonly` : `-2,13 / +1,38 / -2,73 / +1,29 …` une frame sur deux.
La vraie PE de la monture (±20") est **complètement noyée** à ce timescale. Le
« peak-to-peak 0,85 px » mesuré = uniquement l'artefact.

**Correctif appliqué** (`pecmeter`, params dans `measParams`) :
1. **pré-flou gaussien** `preblur` (σ défaut 1 px) — élargit le pic, coupe le
   pixel-locking de tout centroïde ;
2. `phaseCorrelate` ne sert plus qu'au **décalage grossier** (amorce) ;
3. **`cv::findTransformECC` `MOTION_TRANSLATION`** amorcé avec ce décalage → sous-
   pixel par critère continu + interpolation bilinéaire → biais en S bien plus
   doux. `response` = coeff. ECC ∈ [0,1]. `eccrefine`/`ecciters`/`ecceps`/`eccgauss`.
   ECC reçoit les images **non fenêtrées** (pas de FFT). Fallback `phaseCorrelate`
   si ECC diverge.
4. **Calibration de la S-curve résiduelle** (`scurve`, défaut on) : **une fois**,
   au 1ᵉʳ dépôt d'ancre, `pecmeter` génère `scurvepoints` copies de l'ancre
   décalées de fractions de pixel connues par **décalage de Fourier** (exact, sans
   biais d'interpolation), mesure `biais = estimé − vrai` vs `frac`, l'ajuste en
   `scurveharm` harmoniques (`Σ aₖ·sin(2πkf) + cₖ·cos(2πkf)`), et **soustrait**
   `biais(frac(shift))` de chaque mesure. Fonctionne aussi bien (mieux) avec
   `eccrefine=off` → c'est la méthode « estimateur simple + courbe de réponse ».
   Coût : ~1–2 s sur la 1ʳᵉ frame. Loggé : peak-to-peak de la courbe ajustée.
   Désactivé auto si l'ajustement est dégénéré ou > 0,8 px.

Reste en réserve si insuffisant : DFT sur-échantillonnée (Guizar-Sicairos),
mesure **frame-à-frame** (le biais devient un quasi-constant absorbé par `V`).

**Piste : estimateur sélectionnable.** La calibration S-curve est **agnostique
de l'estimateur** — elle calibre ce que fait `measureShift()`. On pourrait
exposer `measParams/estimator` (`phasecorr` / `phasecorr+ecc` / `matchtemplate`)
et garder la même S-curve par-dessus dans tous les cas. Ça permettrait de
retrouver l'approche d'origine (`matchTemplate TM_CCOEFF` + parabole `CV_SubPix`
+ courbe de réponse) et de la comparer. ~20 lignes (un `switch` dans
`measureShift`, `CV_SubPix.{h,cpp}` déjà présents dans le vieux module à
récupérer). Non fait — à voir si le duo ECC + S-curve ne suffit pas.
- `matchTemplate` + `CV_SubPix` gardé seulement si compétitif au bench — alors
  `TM_CCOEFF_NORMED` + upsampling ×4–8 de la ROI avant le fit.
- **Mesurer le plancher réel** : axe bloqué, 500 frames, RMS du déplacement
  rapporté → plancher réel (microscope + éclairage inclus).
- Texture de l'axe : mouchetis imprimé / speckle, **pas du métal poli**.
- Éclairage : LED **DC**, pas de 50 Hz qui bat avec la cadence.

---

## 6. État du vieux code / portage

### Code mort à supprimer (relevé à la lecture)

- Toute la voie « machine à états / pose INDI » : `SMRequestFrameReset`,
  `SMRequestExposure`, `SMAbort` (**corps vide**), `onTimer`, `mytimer`
  (`.start()` commenté). Aucun `QStateMachine` n'est instancié.
- `newBLOB` + `updateProperty` + lecture CFITSIO memfile manuelle → `cv::Mat`
  depuis BLOB → `emit ExposureDone()` : jamais déclenché. `boolloadBlobToCV`
  déclaré, jamais défini.
- 9 signaux déclarés, jamais connectés (`InitDone`, `ExposureDone`, `PulsesDone`…).
- Membres inutilisés en masse : `_pulse/_pulseMax/_pulseMin`, `_raAgr/_deAgr`,
  tous les `_calPulse*`, `_calState/_calStep/_calSteps`,
  `_dxFirst/_dyFirst/_dxPrev/_dyPrev`, `_mountDEC/_mountRA`,
  `_mountPointingWest/_calMountPointingWest`, `_ccdOrientation/_calCcdOrientation`,
  `_ccdSampling`, `_itt`, `subPixTempl/subPixPrev/subPixDev`, `dtPrev`,
  `firstFrame`, `previousFrame/currentFrame`, `gray`, `resolutions`,
  `framecountavg`, `countpulse`, `_camera/_mount` (chaînes en dur), membres PID
  `setpoint/processVariable/output` (redondants avec des locales).
- Props JSON `drift`, `snr`, `values.calPulse*` : déclarées, jamais poussées.
  `revCorrections`, `disDE+/-` : hors sujet (pas de DEC).
- `TAcquisitionVideo::stop()` = `delete this` (dangereux) ; `getProprietes` /
  `setProprietes` quasi tout commenté.
- Handlers `calguide` / `calibrate` / `abortguider` vides ou quasi (`abortguider`
  supprime le thread mais n'arrête pas les pulses en cours).
- Constructeur : écrit `test.jpg` dans le cwd + dump `QCameraInfo` en `qDebug`.
- `#define PI`, `square()` : inutilisés.

### À moderniser (headers actuels)

- `OnMyExternalEvent(OST::Event)` → `onExternalEvent(OST::ExtEvent)`.
- `updateProperty` override → hook `onUpdateProperty`.
- `#include <QtWidgets>` dans un module → à retirer.
- `QCameraInfo` / `QtMultimedia` Qt5 → suppression (plus de flux natif dans la
  boucle).
- `setModuleVersion` / `getEltString("thisGit"…)` → `setMetadata(...)` comme dans
  le guider actuel.
- S'aligner sur les patterns du `guider` actuel : watchdog single-shot réarmé à
  chaque attente hardware, garde `_expectingFrame` sur le BLOB,
  `atomicSaveJpeg`.

---

## 7. Options structurantes tranchées

### Module séparé `blindpec` vs flag dans `guider` → **module séparé** (A)

Deux pistes envisagées :
- **A** — nouveau module `blindpec` ; le profil utilise soit `guider`, soit
  `blindpec` comme guider.
- **B** — adapter `guider` avec un mode « blindpec on/off ».

**Retenu : A.** Simplicité (on ne mélange rien), testable en isolation, plus
facile à débugger.

Idée de départ : `blindpec` déclare `defineMeAsGuider()` comme `guider`, donc
pour le sequencer c'est « un guider ».

**Nuance importante (lacune connue du core).** Ce n'est *pas* aussi propre :
le sequencer dialogue avec un module par **nom de classe** (`"guider"`), pas par
une notion de **famille / type** de module. `defineMeAsGuider()` laisse croire
que le rôle suffit, mais il reste des couplages en dur sur le nom `guider` et
plein de conséquences non gérées. Un vrai « type de module » dans le core n'est
qu'ébauché.

Conséquence pratique pour BlindPEC : ne pas supposer que déclarer le rôle
suffit. À l'intégration il faudra vérifier concrètement chaque point de contact
sequencer ↔ guider (dither, timeout, suspend/resume, abort, lecture des
résultats) et voir ce qui casse quand la classe s'appelle `blindpec` et non
`guider`. Options de contournement possibles le moment venu : garder la classe
nommée de façon à matcher, ou faire avancer la notion de type/famille dans le
core (**sujet à part, plus tard**).

Seule règle claire dès maintenant : **un profil contient soit `guider`, soit
`blindpec`**, jamais les deux.

### Comment démarrer le module

Ne pas ressusciter le vieux `blindpec` tel quel. Le composer de deux sources :

- **Plomberie = squelette du `guider` actuel** (headers récents, éprouvé) :
  `SMRequestExposure` + BLOB avec garde `_expectingFrame`, `SMRequestPulses`,
  watchdog single-shot, `atomicSaveJpeg`, push des grilles `guiding` / `drift`.
  On retire calibration d'orientation, DEC, `startracker`, backlash.
- **Algorithme = vieux `blindpec`** : `matchTemplate` + `CV_SubPix`, ré-ancrage
  ROI + `offset`, soustraction `V·t`, PID, GuideLog PHD2.

→ module court, une seule machine à états linéaire (capture → mesure → commande →
capture), un seul axe.

### Conséquences à garder en tête

- **Duplication de plomberie** avec `guider` : assumée au début. Factorisation
  d'une base commune (boucle capture INDI + envoi pulses + log PHD2) *après*,
  quand la forme de `blindpec` est stable — pas avant.
- **Dither du sequencer** : envoyé à « the guider ». RA seul = OK, composante DEC
  = no-op. À documenter.
- **Nom** : `blindpec` conservé (parlant, déjà existant). Label UI plus explicite,
  p. ex. « Optical axis guider (BlindPEC) ».

---

## 8. Autres points identifiés

### Propriétés favorables à exploiter

- **Le retournement au méridien est transparent.** BlindPEC ferme la boucle sur
  **l'axe**, pas sur le ciel. Les pulses E/W pilotent des sens moteur fixes ;
  leur effet *sur l'axe* ne s'inverse pas au flip (seul l'effet *sur le ciel*
  s'inverse). → aucune gymnastique pier-side comme dans le `guider`.
  **Corollaire** : le microscope doit être fixé sur le **corps de monture
  porteur du palier d'AD** (partie qui ne tourne pas avec l'axe), jamais sur
  quelque chose de solidaire de l'axe.
- **Incrémental, pas d'index.** On mesure une variation, pas l'angle absolu.
  Parfait pour la correction PE en **feedback**. Pour du **feed-forward**
  (rejouer une table PE) il faut une **phase de la VSF** : index PEC de la monture
  (propriété INDI `PEC` si dispo) ou estimateur de phase verrouillé sur la
  composante périodique (valide après >1 période, dérive si la VSF n'est pas
  stable). → contrainte sur l'ambition table PEC.

### Domaine de validité (à écrire noir sur blanc dans la doc module)

BlindPEC **ne corrige pas** la dérive DEC ni la rotation de champ. Hypothèse
d'exploitation explicite : **MES assez bonne pour que la dérive DEC reste sous la
tolérance de traînage sur la durée de pose visée**. Se marie avec bonne MES /
poses pas trop longues / recentrage occasionnel par plate-solve. Limite, pas bug.
Attention : la réfraction ajoute une vraie dérive DEC lente près de l'horizon
(cf. §8 « DEC et réfraction ») — l'hypothèse « dérive DEC = turbulence » ne vaut
qu'à haute altitude.

### Pourquoi le `V` adaptatif n'est pas optionnel (chiffré)

Boucle active, `V` statique faux de 0,1 % : le résidu *contrôlé* ne s'emballe pas
(la boucle le tient à ~0), **mais** :
- biais de pulse permanent (filet constant E ou W) ;
- l'intégrateur s'assoit sur une valeur permanente → **mange la réserve
  anti-windup d'un côté** ;
- le résidu *brut* loggé (`p − V·t`) rampe et perd son sens physique.

Dérive thermique (graisse qui refroidit), terme disque à 1/tour, erreur de taux
moyen : **tout tombe dans la même bande lente** → un seul mécanisme (`V`
adaptatif, τ ≈ quelques périodes de VSF) traite l'ensemble.

### Robustesse

- **Phase de caractérisation au démarrage** : suivi seul, mesure ≥ dizaines de s
  (idéalement 1 période de VSF) **avant** de corriger → amorce `V`, amorce la
  phase PE si feed-forward. Pas de settle stellaire (pas de slew) mais ce
  warm-up est nécessaire.
- **Gate anti-outlier / vibration** : résonance du support + vent + pas au sol →
  fausse « PE » HF → « corrigée » → injecte une vraie erreur dans la monture.
  Rejeter les frames avec `|Δ|` ≫ attendu ou `response` bas. Mécanique : support
  **court et rigide**, masse.
- **Composante d'axe croisé = signal de santé.** Microscope jamais parfaitement
  tangentiel → projeter la mesure sur la direction calibrée ; la composante
  orthogonale doit rester ~0. Dérive → glissement du CD, basculement,
  desserrage.
- **Remontage du CD = recalibration.** `r`, voile, patch de texture changent à
  chaque montage → calibration de `G` **rapide et automatique en début de
  session** (quelques pulses, ~10 s), pas une corvée manuelle.

### Télémétrie à concevoir dès le départ

Douleur récurrente du `guider` : « on ne sait pas pourquoi ça a aborté ».
Publier par frame : `response` de corrélation, résidu d'axe croisé, flag de
ré-ancrage, `Δt` mesuré vs nominal (détection de frame lente même en pose
séquencée), pulse émis, RMS du résidu **dans la bande PE**. Garder le GuideLog
PHD2 + un CSV brut `(t, p_raw, V_est, résidu, response, pulse)` pour le tuning
offline.

### DEC et réfraction atmosphérique (à mettre de côté, mais noté)

La vitesse de dérive cible `V` n'est **pas** une pure constante sidérale : la
réfraction atmosphérique déforme la trajectoire apparente de l'objet, d'autant
plus qu'on est bas sur l'horizon. `V` doit être adaptée en fonction des
coordonnées **ALT/AZ** (et pression / température).

- **RA** : le taux apparent réel dévie du sidéral selon ALT/AZ. Fonction connue
  et calculable (cf. « King rate » / tracking corrigé réfraction). → `V` devrait
  porter un terme correctif dépendant du pointage courant. Le `V` adaptatif en
  absorbe déjà une partie (variation lente, sauf près de l'horizon) ; mieux vaut
  feed-forward le terme calculé et ne laisser au `V` adaptatif que le résidu.
- **DEC** : la réfraction induit aussi une **dérive DEC apparente lente**
  dépendant d'ALT/AZ → « avec bonne MES la dérive DEC n'est que de la
  turbulence » est **faux près de l'horizon**. BlindPEC ne peut pas la corriger
  (pas d'axe DEC) → fait partie du domaine de validité ci-dessus.

À mettre de côté pour une v1 (viser haute altitude, loin de l'horizon), à
reprendre ensuite. (Piste matérielle alternative pour la DEC un jour : 2ᵉ
microscope sur l'axe DEC → encodeur aveugle 2 axes ; garder la gestion d'axe
factorisée si peu coûteux.)

---

## 9. État d'avancement — v0 (squelette qui compile)

Créé dans `src/modules/blindpec/` (build OK : `libostblindpec.so`, warnings
unused-param seulement, comme la classe de base) :

| Fichier | Rôle |
|---|---|
| `pecmeter.{h,cpp}` | **Cœur de mesure isolé** (pas de Qt/INDI, testable seul). `cv::phaseCorrelate` + fenêtre de Hann, mesure vs frame d'**ancre**, ré-ancrage sous-pixel quand le shift dépasse `reanchorfrac·½·min(w,h)`, gates `minResponse` / `maxStepPx`. Rend un `Sample {ok, x, y, stepX/Y, response, reanchored}`. Analogue de `startracker` pour le guider. |
| `blindpec.{h,cpp}` | Module. Plomberie calquée sur le `guider` actuel : `_expectingFrame` sur le BLOB, watchdog single-shot réarmé à chaque attente hardware, `atomicSaveJpeg`, grilles `guiding`/`drift`, `setMetadata("template","guider")`. **Une seule** `QStateMachine` linéaire : `InitInit → ReqFrameReset → … Exposure → Measure → Compute → ReqPulses → …` en boucle. RA seul (`TELESCOPE_TIMED_GUIDE_WE`). |
| `blindpec.json`, `blindpec.qrc` | Props : `values`, `calibrationvalues` (G, V, arcsecPerPx, guideRateK, revRA), `drift`, `guiding`, `calParams` (calpulse/calsteps), `pid` (kp/ki/kd), `guideParams`, `revCorrections`, `disCorrections`. |
| `CMakeLists.txt` | Cible `ostblindpec` (lie OpenCV comme `inspector` + `Qt::StateMachine`) ajoutée à la liste `install`. |

### Phases implémentées (enum `Phase` dans `SMCompute`) — calibration 2 étapes

Ordre : **Init → Characterize (étape 1) → GainCal (étape 2) → Guide.**
Hypothèse : **la monture suit au sidéral** pendant toute la calibration.

1. **PhCharacterize (étape 1)** — dérive libre, aucun pulse, pendant
   `calParams/chardur` s (défaut 60 ; « quelques dizaines de s » = `V` grossier,
   ≥ 1 période de VSF = `V` propre + vraie mesure de PE). `fitDriftLine()` =
   pentes MCO de `x(t)` et `y(t)` → **`θ` = `atan2(sy, sx)`** (orientation de
   l'axe AD dans l'image) et **`V` = `hypot(sx, sy)`** (px/s). Résidu autour de
   la droite décomposé en **along-axis** (= erreur périodique + bruit) et
   **perpendiculaire** (doit rester petit). Loggé : PE peak-to-peak + RMS (arcsec)
   + courbe PE décimée (`PE t=.. along=.. arcsec`, ≤ 40 lignes).
   **`calParams/observeonly`** : fait l'étape 1 seule puis s'arrête proprement
   (pas de gain-cal, pas de guidage) → pour mesurer/visualiser la PE.
   En guidage, **`disRA+` ET `disRA-` cochés = mode observation** : la boucle
   mesure et logge `resid` (= PE) mais n'envoie rien, n'adapte rien (intégrale +
   `V` gelés).
   **Échelle** : l'axe AD tourne au taux sidéral (~15,041 arcsec/s *de rotation
   d'axe*), donc **`arcsecPerPx = 15,041 / V`**. C'est la **seule** source de
   l'échelle — aucune focale / grandissement / taux de guidage monture : tout ça
   est déjà encapsulé dans `V` (px/s). Ex. 30 px en 10 s → `V = 3 px/s` →
   `150 arcsec / 30 px = 5 arcsec/px` (diviser par les **pixels**, pas les
   secondes). Le résidu autour de la droite = courbe de PE (pas encore exploité).
   Persiste `V`, `theta`, `arcsecPerPx`.
2. **PhGainCal (étape 2)** — `calsteps` pulses W **et** `calsteps` pulses E,
   alternés. Pour chaque pulse : `effet = Δp_proj − V·Δτ` (projection sur `θ`,
   dérive sidérale retirée grâce à `V`). `moveW = moy(effetsW)`,
   `moveE = moy(effetsE)` (signes opposés). `Gpx = (|moveW|+|moveE|)/2` →
   **`G = calpulse / Gpx`** (ms/px), **`wDir = signe(moveW)`**. Warning si
   asymétrie `|moveW|` vs `|moveE|` > 50 % (backlash ou `V` faux). Le taux de
   guidage monture `k` (`GUIDE_RATE`, fallback 0,5) ne sert **plus qu'à un log de
   contrôle croisé** (px/pulse mesuré vs attendu), pas au calcul. Persiste `G`,
   `wdir`, `guideRateK`.

**Calibration = une fois (montage physique).** `θ`, `V`, `arcsecPerPx` sont
**géométriques** (orientation du microscope, rayon × grandissement) et `V` est
lié au sidéral, pas à la thermique — donc constants tant que le microscope ne
bouge pas. Flux :
- **`calibrate`** → étape 1 + étape 2, persiste les 5 valeurs, s'arrête.
- **`guide`** avec calibration complète stockée (`arcsecPerPx>0 && V>0 && G>0`) →
  charge `θ/V/échelle/G/wDir` et va **directement au guidage** (pas d'étape 1 ni
  2). `SMInitInit` appelle `enterGuide()` d'emblée.
- **`guide`** sans calibration → étape 1 + étape 2 + guidage.
- **`resetcalibration`** efface les 5 → force une re-calibration complète.
La dérive thermique résiduelle en session est absorbée par `alphaV` (adaptatif),
qui part de la `V` stockée.
3. **PhGuide** — mesure projetée sur `θ` : `p = projRA`, `cross = projCross`
   (signal de santé). **Consigne incrémentale** : `setpoint += V·dt` à chaque
   frame (1ʳᵉ frame → `setpoint = p`), `résidu = p − setpoint`. (Pas `p − V·t` :
   changer `V` réécrivait rétroactivement toute la ligne de base.) RMS calculé à
   **chaque** frame (blank incluses — c'est un vrai résidu, juste pas corrigé ;
   avant, les blank poussaient `RMS = 0` dans le graphe → yoyo). Dither RA seul
   en biais de consigne ; `blankframes` après un pulse ; P+I(+D) → effort `u` →
   `needPx = −u` (flip par `revRA` manuel) → sens W/E via `wDir`, magnitude via
   `G` ; clamp `pulsemin/pulsemax` ; masque `disCorrections` ; anti-windup
   (`intmax`) ; RMS glissant.
   **`V` adaptatif = PI sur `V`** : `V += alphaV·résidu` chaque frame, borné à
   ±20 % du `V` caractérisé (`_V0`). Piloté par la *valeur* du résidu (un résidu
   soutenu déplace `V`, un résidu de moyenne nulle le laisse) → contre-réaction
   négative stable. Corrige un `V` biaisé par une caractérisation trop courte.
   Défaut `alphaV = 0.005` (**activé**). L'ancienne loi `(err − errPrev)/dt`
   (dérivée) chassait le bruit → supprimée.
   **`kp` défaut 0,5**, **`kd` défaut 0** (le terme D brut n'amplifie que du bruit).
   `kp = 1` = deadbeat → sonne avec la période de boucle.
   **Compensation par anticipation (lead)** — 2026-09-08, sans paramètre exposé.
   La boucle est limitée par la **latence** (~1 s mesure → effet du pulse) : elle
   corrige où l'erreur *était*. On lisse `d(err)/dt` sur ~1 s (`_errRate`, EMA
   indépendante de la cadence) et on corrige sur `errLead = err + _errRate·LEAD`
   avec `LEAD = 1 s` codé en dur (= la latence). `u = kp·errLead + ki·I + kd·_errRate`
   (le terme `kd` agit maintenant sur le taux *lissé*, plus sur `err − errPrev`
   brut). **La consigne avance toujours à `V` fixe** → `err` reste la vraie
   erreur d'axe : pas de piège « suivre la PE » (contrairement à un FF de vitesse
   sur la consigne, qui serait le même défaut que `alphaV`). Loggé : `eR=` (px/s).

**Diag banc (21:24)** : caractérisation 30 s → `V = 2.2249` biaisé haut de
~7,5 % (phase de PE). Symptômes : gain-cal asym W/E 56 % (`W = g−δ`, `E = g+δ` ;
la moyenne `(|W|+|E|)/2` annule `δ` → `G` reste bon), et en guidage l'intégrale
qui s'accumule (`I : −0.08 → −1.66`) + pulses W permanents. → c'est ce que le
`V` adaptatif corrige.

### Pas d'hypothèse horizontale/verticale

`fitDriftLine()` ajuste les pentes de `x(t)` **et** `y(t)` → `θ = atan2(sy, sx)`
= la vraie direction de dérive (angle quelconque). Toutes les mesures suivantes
sont **projetées sur `θ`** (`projRA` / `projCross`). L'axe AD dans l'image est
calibré, on ne suppose ni X ni Y. Seul le vecteur de déplacement dessiné dans le
preview est encore en X/Y bruts (cosmétique).

### Affichage : mono-axe

`drift` (scatter XY) et `guiding` (PHD) sont réutilisés du template `guider` mais
BlindPEC n'a qu'un axe : `DEC` / `DE` / `pDE` sont **forcés à 0** (rien ne doit
se lire comme un 2ᵉ axe asservi). La composante perpendiculaire (`cross`) reste
publiée **uniquement** dans `values/crossaxis` — lecture de santé numérique
(glissement / basculement du CD, turbulence), pas une trace de guidage.

### Bring-up

- `_trace` dans `blindpec.h` : **`false` par défaut** (le per-frame floodait).
  Passe-le à `true` pour ré-activer les traces verbeuses (`prop <-`, `newBLOB`,
  `SM: Request/Wait/Measure/Compute`, pulses). Restent visibles sans `_trace` :
  transitions de phase, résultats de calibration, warnings/errors, abort.
- **Frame-reset** : `CCD_FRAME_RESET` ne repasse pas toujours à `IPS_OK` selon le
  driver. `SMRequestFrameReset` a maintenant un `QTimer::singleShot` de 1,5 s qui
  émet `FrameResetDone` en filet de sécurité (le `guider` a le même point faible,
  non traité chez lui). Sur le banc (driver **V4L2 CCD**) le vrai `IPS_OK`
  arrive, donc OK.
- **Pose rejetée par le driver V4L2 CCD** : `CCD_EXPOSURE -> IPS_ALERT` ~80 ms
  après `requestCapture` (expo 0,1 s), aucun BLOB. `onUpdateProperty` détecte
  maintenant l'alerte et abort tout de suite au lieu d'attendre 30 s.
  **Cause probable = config driver/caméra, pas le code** : auto-exposure V4L2
  active, format de capture / résolution non sélectionné, ou valeur d'expo hors
  plage supportée. À vérifier : reproduire une pose 0,1 s directement dans le
  panneau INDI pour « V4L2 CCD » ; si ça alerte aussi là → régler le driver
  (désactiver auto-exposure, choisir un format, essayer 0,5–1 s). Si le driver
  s'avère *streaming only*, basculer sur le backend vidéo (#2 du §4).
- `HORIZONTAL_COORD` / `GUIDE_RATE` absents sur certaines montures → lecture best
  effort, warning + fallback `k = 0.5`, pas d'abort.
- **Driver « INDI Webcam »** : frames OK (640×480, `bpp=1 ch=3` → on prend le
  plan R, corrélation `resp≈0.999`, très bonne texture).
- **Caractérisation trop rapide** (v0 initiale) : 60 frames en < 1 s → `V ≈ 0`.
  **Refondue** en calibration 2 étapes (cf. § « Phases implémentées ») : étape 1
  = dérive libre pendant `calParams/chardur` s → `θ` + `V` ; étape 2 = pulses
  W/E décontaminés par `V`.

### Sortie GuideLog PHD2

`enterGuide()` ouvre `getWebroot()/PHD2_GuideLog_<yyyy-MM-dd_HHmmss>.txt` (même
répertoire que le JPEG `<module>.jpeg`), y écrit un en-tête PHD2 minimal
(`Guiding Begins at`, `Pixel scale = <arcsecPerPx> arc-sec/px`, ligne
`Mount = "Mount", …, xAngle = θ°, xRate = 1000/G px/s, …`, puis la ligne des
18 colonnes). `computeGuide()` ajoute une ligne par frame de guidage (frames
`blank` et mode *observe* inclus → une session *observe* produit une trace PE
pure ouvrable dans PHD Log Viewer). `SMAbort()` écrit `Guiding Ends at` +
`Log closed at` et ferme le fichier. Toujours actif, pas de paramètre.

Conventions : axe unique RA, toutes les colonnes DEC à `0` / vide. Signe PHD
respecté : `RARawDistance > 0 ⇔ pulse W`, dérivé de la direction qui annule
l'erreur courante (donc défini même sur les frames `blank`), avec les mêmes
bascules `wDir` / `revRA` que la boucle. `dx,dy` = résidu reprojeté en repère
caméra (pour le nuage de points PHD). `SNR` = métrique NCC ×100 ; `StarMass`
sans équivalent (constante). `Frame`/`Time` repartent de 0 à chaque session.

### Piste explorée et abandonnée : projection 1D avant corrélation

**Idée (branche `blindpec-1dproj`, hors build, non câblée) :** puisque seul le
déplacement le long de `θ` compte, sommer les lignes perpendiculaires à l'axe
pour obtenir un profil 1D avant corrélation - gain de SNR "gratuit" façon
binning logiciel, plus un zoom DFT suréchantillonné 1D bien moins cher que la
version 2D (`measParams/dftshift`, coûteuse à `-O0`). Code : `pecmeter1d.h/.cpp`
(`project()` + `upsampledShift()` façon Guizar-Sicairos 1D), isolé, sans
dépendance Qt/INDI, même convention que `pecmeter.cpp`.

**Verdict : idée fausse, invalidée par le banc, deux fois.**
1. Banc synthétique (texture isotrope, même recette que le banc DFT 2D) :
   le 1D est **2,6 à 130× pire** que le 2D selon bruit/angle, biais dès θ≠0
   même sans bruit (artefact de rotation `warpAffine`).
2. Banc sur **vraies frames BlindPEC** (2 frames indépendantes, 320×240,
   bandeau overlay retiré) : confirmé et pire - **2,3 à 32× pire**, jusqu'à
   ~90× à certains angles, y compris à bruit nul.

**Pourquoi :** le raisonnement de départ suppose la texture invariante selon
l'axe perpendiculaire (alors sommer les lignes accumule le signal en cohérent
pendant que le bruit s'annule en `√N` - un vrai filtre adapté). C'est faux
pour une surface poncée : l'autocorrélation directionnelle mesurée sur les
vraies frames tombe de ~0,75-0,82 à 1 px à ~0,2-0,3 dès 8-12 px - le grain
n'est cohérent que sur quelques pixels, pas sur les ~100 lignes sommées.
Sommer moyenne le signal aussi vite que le bruit ; le pic de corrélation
s'aplatit au lieu de se renforcer. L'estimateur 2D existant (ECC+S-curve ou
DFT 2D) exploite déjà toute l'information utile de l'image - la réduire en 1D
en jette une partie.

**Conclusion pratique :** on a fait le tour raisonnable des méthodes de
mesure sub-pixel pour ce capteur/cette texture (cf. §5 pixel-locking, et la
branche `blindpec-dftshift`). Le plancher de mesure actuel (~0,03 px, S-curve
ECC) n'est pas là où chercher du gain. `pecmeter1d.h/.cpp` reste dans le repo
comme trace documentée de ce cul-de-sac (non compilé, hors CMake) ; les bancs
(`bench1d.cpp`, `bench1d_real.cpp`) sont restés en scratchpad, comme pour
`dfttest.cpp`.

### Raccourcis / dette assumée (à reprendre)

- **Timestamp = `QDateTime::currentDateTime()` à l'arrivée du BLOB**, pas
  `DATE-OBS + t_exp/2` de l'en-tête. → à câbler (cf. §4, c'est *le* point dur).
- `fitDriftLine()` = MCO simple sur `x(t)` / `y(t)`, **pas** Theil–Sen ni
  fenêtrage sur période de VSF entière → biais PE résiduel sur `θ` et `V`.
- `chardur` : paramètre fixe utilisateur ; pas de détection auto de `T_vsf` ni
  de fenêtrage entier (prévu plus tard).
- Ré-ancrage sans **vérification croisée** (corréler ancienne vs nouvelle ancre).
- Pas de gestion multi-canal FITS (prend le 1ᵉʳ plan si `channels==3`).
- Bouton *calibrate*-only : réutilise `emit Abort()` pour s'arrêter (log
  « Aborting ») au lieu d'un arrêt propre.
- ROI / binning : **pas encore** de pilotage `CCD_FRAME` / `CCD_BINNING` — on
  prend la trame telle que la donne le driver.
- Réfraction (terme `V(ALT/AZ)`) : ALT/AZ lus et stockés, **pas utilisés**.

---

## 10. Points ouverts

- [ ] Timestamp `DATE-OBS + t_exp/2` par frame (remplacer `nowMs()`).
- [ ] `matchTemplate`+`CV_SubPix` vs `phaseCorrelate` vs `findTransformECC` :
      bench sur images réelles (le code part sur `phaseCorrelate`).
- [ ] `fitDriftLine` robuste : Theil–Sen + fenêtre = multiple entier de `T_vsf`.
- [ ] Estimation de `T_vsf` : détection auto (FFT/autocorr) du résidu étape 1.
- [ ] Pilotage ROI + binning (`CCD_FRAME`, `CCD_BINNING`).
- [ ] Revoir les défauts `pid` (kp/ki) sur le banc.
- [ ] Vérification croisée du ré-ancrage.
- [ ] Exploiter la composante d'axe croisé (`crossaxis`) comme diagnostic actif.
- [ ] Arrêt propre pour *calibrate*-only (ne pas réutiliser `Abort`).
- [ ] Référence de phase de la VSF pour la table PEC : dispo côté INDI monture ?
- [ ] Audit des points de contact sequencer ↔ « guider » avec la classe
      `blindpec` (cf. §7).
- [ ] Test unitaire `pecmeter` (2 `cv::Mat` synthétiques translatées → `dx/dy`).
