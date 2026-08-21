# Module Collimator — OST

## Contexte projet
OST (Observatoire Sans Tête) est une plateforme d'astrophotographie autonome headless.
- **ostserver** : backend C++17/Qt6, pilote les appareils via INDI (libindi, libnova, cfitsio, StellarSolver)
- **osterix** : frontend Angular/TypeScript, communication temps réel via WebSocket (messages JSON)
- Modules existants : Focuser (mise au point auto par ajustement polynomial degré 2 sur courbe HFR, zoning jusqu'à 9×9), Guider (guidage maison sans PHD2), etc.

## Objectif du module
Aider à la collimation optique (alignement des miroirs) des instruments **Newton, RC (Ritchey-Chrétien) et SC (Schmidt-Cassegrain)**, en semi-manuel : le logiciel analyse et affiche l'écart de collimation, l'utilisateur agit physiquement sur les vis (non motorisées), le logiciel réanalyse en boucle jusqu'à convergence.

## Instruments couverts (v1)
Newton, RC (Ritchey-Chrétien), SC (Schmidt-Cassegrain). La méthode d'analyse est **strictement identique pour les trois formules** : les trois présentent une ombre du secondaire (obstruction centrale) visible dans le donut défocalisé. Pas de sélection ni d'adaptation par type d'instrument nécessaire dans le module — seul le rapport F/D (déjà connu via les infos du télescope ailleurs dans OST) intervient dans le calcul, pas la formule optique en tant que telle.

## Convention mécanique
**Fixe** : 3 vis de collimation à 120° les unes des autres, quel que soit l'instrument (pas de config manuelle en v1, pas de détection auto).

## Principe retenu pour l'algo d'analyse : convergence du champ de déformation

Une étoile défocalisée apparaît comme un disque/anneau ("donut" ou "pseudo-donut") dont la forme renseigne sur la collimation :
- Si l'instrument est bien collimaté, une étoile **centrée dans le champ** produit un donut parfaitement concentrique. Une étoile **excentrée** dans le champ montre malgré tout un donut déformé (asymétrique, forme en virgule) à cause du **coma de champ**, une aberration optique normale et attendue, pas un défaut de collimation — cette déformation pointe vers le centre de l'image et croît avec la distance au centre et avec l'ouverture (1/(F/D)²).
- Si mal collimaté, un vecteur de décentrage **constant, indépendant de la position dans le champ**, s'ajoute à cet effet de coma pour chaque étoile.

Concrètement, le vecteur de déformation observé pour une étoile à la position `P` (repère image, centre = origine) suit un modèle linéaire :

```
D(P) = C - k·P
```

où `C` est le vecteur de collimation recherché (constant) et `k` regroupe le coefficient de coma (dépend du F/D de l'instrument). Un champ vectoriel de cette forme a toujours un **point de convergence unique** `P* = C/k` — le point où les vecteurs de déformation, prolongés en droites, se croisent tous. Avec `k` connu (F/D disponible dans les paramètres du télescope), quelques étoiles suffisent à localiser ce point et à en déduire `C` directement. Si `k` n'est pas connu a priori, il peut être résolu simultanément avec `C` par régression, à condition d'avoir des étoiles à des positions de champ suffisamment variées.

**Conséquence importante : pas besoin de comparaison intra/extra-focale dans l'algo automatique.** Une seule capture, une seule position de défocalisation, suffit à déterminer le vecteur de collimation. L'utilisateur reste libre de faire lui-même des allers-retours intra/extra en usage manuel pour affiner ou vérifier visuellement (cf. boutons **Go intra**/**Go extra**), mais ce n'est pas une étape du flux automatisé.

**Étapes de l'algo :**
1. Capture d'une image défocalisée (intra ou extra, peu importe le sens — l'utilisateur choisit ou l'auto-défocalisation par défaut)
2. Détection de **toutes** les étoiles/donuts exploitables du champ (pas une seule cible)
3. Pour chaque donut détecté : calcul d'un vecteur de déformation local — centroïde pondéré par intensité de l'anneau (en excluant l'ombre du secondaire) par rapport au centre théorique du donut, plus robuste qu'un simple écart entre centroïdes d'ellipses quand la déformation est une forme en virgule prononcée plutôt qu'un cercle simplement décentré
4. Régression sur l'ensemble des vecteurs (position de champ → vecteur de déformation) pour déterminer `C` (et `k` si non connu a priori)
5. Conversion du vecteur `C` (repère image, en x/y pixels) vers un repère angulaire aligné sur les 3 vis à 120° → détermination du sens et de l'amplitude de rotation à appliquer par vis
6. Le critère de convergence : `|C|` proche de 0 — c'est l'état cible à atteindre par ajustement des vis

**Bibliothèque retenue : OpenCV (module `imgproc`)**
- `cv::threshold` / `cv::adaptiveThreshold` pour isoler chaque donut du fond de ciel
- `cv::findContours` pour extraire les contours (externe de chaque donut + ombre du secondaire)
- `cv::moments` pour le centroïde pondéré par intensité de l'anneau (méthode principale, cf. étape 3 ci-dessus)
- `cv::fitEllipse` en complément sur les donuts peu déformés (proches du centre), pour une mesure de précision quand la forme reste proche d'un cercle/ellipse propre
- `cv::HoughCircles` en option si les contours s'avèrent bruités
- Choisi pour la rapidité de développement et la robustesse en v1, au détriment d'une empreinte plus légère (bien packagée sur Debian/Raspberry Pi OS, donc acceptable pour un usage headless)
- Alternative écartée pour la v1 mais à garder en tête si le poids pose problème sur Raspberry Pi : seuillage + fit de cercle maison en C++/Eigen, plus léger mais plus de travail
- `cfitsio` (déjà utilisé dans le projet) reste la brique de lecture/écriture FITS en amont

## Flux fonctionnel (wizard semi-manuel)
1. **Défocalisation** : pilotage automatique du focuser existant vers une position cible définie (avec possibilité d'override manuel par l'utilisateur — la position n'est pas figée si l'utilisateur préfère ajuster lui-même)
2. Capture d'image via la caméra déjà pilotée par OST — un champ suffisamment riche en étoiles, pas une cible unique (cf. section Cible)
3. **Analyse** (nouvel algo dédié, indépendant de StellarSolver, cf. section dédiée ci-dessus) : détection multi-étoiles, calcul du vecteur de collimation `C` par convergence du champ de déformation
4. **Affichage double** :
   - Overlay graphique sur l'image live (contours de chaque donut détecté, petit vecteur de déformation par étoile, point de convergence calculé, indication du vecteur de correction global)
   - Jauges/flèches numériques par vis (les 3 vis, amplitude + sens de rotation à appliquer)
5. **Boucle temps quasi-réel** : nouvelle capture → nouvelle analyse → mise à jour de l'affichage, répété jusqu'à convergence (`|C|` proche de 0)
6. Fin de session : pas de persistance/historique en v1 — état valable uniquement pour la session en cours

## Hors périmètre v1
- Historique/log des sessions (reporté à une version ultérieure)
- Détection automatique du nombre/position des vis (convention fixe 3×120° uniquement)
- Réutilisation de StellarSolver pour l'analyse (nouvel algo dédié à écrire)
- Motorisation des vis (aucun asservissement, action physique humaine uniquement)
- Recentrage automatique par slew monture (plus nécessaire — l'algo réanalyse le champ disponible à chaque capture, sans avoir besoin de garder une étoile particulière centrée)
- Comparaison intra/extra automatisée dans la boucle (laissée à l'usage manuel de l'utilisateur, cf. boutons Go intra/Go extra)

## Interface utilisateur

### Écran de sélection des périphériques INDI
- **Caméra** (obligatoire) : source des captures
- **Monture** (facultative) : utile pour un pointage initial vers un champ riche en étoiles, mais plus indispensable en continu — l'algo ne dépend plus du recentrage d'une étoile particulière pendant la boucle d'ajustement
- **Focuser** (facultatif) : si présent, pilotage automatique de la défocalisation (position intra/extra-focale) ; si absent, l'utilisateur doit défocaliser manuellement (molette de mise au point) avant chaque capture, le module se contentant alors d'attendre une confirmation de l'utilisateur pour déclencher la capture

### Overlay d'affichage (image live)
Affichage continu de l'image caméra, avec overlay graphique en superposition :
- **Contours détectés** de chaque donut exploitable du champ (contour externe + ombre du secondaire) tracés sur l'image en temps réel
- **Vecteur de déformation** par étoile détectée : petite flèche indiquant la direction/amplitude locale de l'asymétrie
- **Point de convergence** calculé (`P*`) matérialisé sur l'image, et **ligne** reliant ce point au centre théorique de l'image — la longueur donne une lecture immédiate de l'amplitude de l'écart de collimation
- **Bullseye** : anneaux concentriques fixes matérialisant la zone de tolérance de convergence, centrés sur le centre théorique de l'image
- **Code couleur progressif** (rouge → orange → vert) appliqué au point de convergence et/ou aux contours, selon la proximité avec la cible, pour un feedback instantané sans lecture de valeur numérique
- Complété par les jauges/flèches numériques par vis (cf. section Flux fonctionnel) pour la précision chiffrée

### Boutons d'action
Cohérent avec les autres modules du projet (ex. Focuser) :
- **Start** : démarre la boucle continue (capture → analyse → affichage overlay/jauges)
- **Stop** : arrête la boucle continue
- **Go home** : ramène le focuser à la position de départ (mise au point d'origine, avant toute défocalisation)
- **Go intra** : déplace le focuser de -XX pas (offset défini en paramètre) pour atteindre la position intra-focale
- **Go extra** : déplace le focuser de +XX pas (offset défini en paramètre) pour atteindre la position extra-focale

`Go intra`/`Go extra` restent utiles pour l'utilisateur qui veut vérifier ou affiner manuellement par va-et-vient intra/extra (comparaison visuelle), mais ce n'est plus une étape pilotée par l'algo automatique.

### Paramètres du module (v1)
- **Temps de pose, gain, offset** : gérés via les propriétés génériques caméra déjà utilisées par les autres modules du projet (pas de réinvention, cohérence avec le reste d'OST)
- **Offset du focuser pour défocaliser** : décalage (en pas de focuser) à appliquer par rapport à la position actuelle pour atteindre l'intra/extra-focale ; on postule que la mise au point de départ est déjà à peu près correcte, donc un simple offset relatif suffit (pas besoin de retrouver le focus absolu)
- **Facteurs de vitesse de slew/recentrage** : x2, x4, x8... pour les commandes de pointage initial manuel, cohérent avec les contrôles de raquette classiques

Pas de paramètre dédié pour le F/D : réutilise le rapport focale/ouverture déjà renseigné dans les infos télescope existantes d'OST, pas de duplication.

Pas de paramètres supplémentaires prévus pour la v1 (seuils de convergence, ROI, binning, etc. laissés pour une itération ultérieure).

### Déclenchement de la capture
**Boucle continue automatique** : le module capture, analyse et met à jour l'affichage (overlay + jauges) en continu, sans action de déclenchement manuelle. L'utilisateur peut ainsi tourner une vis tout en observant le retour visuel se mettre à jour en quasi temps réel, comme un feedback live plutôt qu'un cycle capture → pause → capture.

**Cible** : un champ suffisamment riche en étoiles (quelques-unes suffisent, réparties à des positions de champ variées), pas une étoile unique — l'algo a besoin de plusieurs points de mesure à des rayons de champ différents pour déterminer le point de convergence. Contrairement à une approche mono-étoile, il n'est plus nécessaire que chaque étoile soit très brillante individuellement (l'algo agrège plusieurs mesures), mais chaque donut détecté doit rester suffisamment exploitable (seuil de SNR minimal à définir).

Comme chaque capture est analysée indépendamment (pas de suivi d'une étoile particulière d'une image à l'autre), la dérive du champ pendant les réglages de vis (le tube n'étant pas un point fixe) n'est plus un problème : l'algo redétecte simplement les étoiles disponibles dans le champ courant à chaque nouvelle capture, quelles qu'elles soient.

Le CCD Simulator INDI ne convenait pas nativement : il ne génère que des étoiles gaussiennes ponctuelles, pas de patron de diffraction défocalisé (donut). **Mise à jour** : une branche de développement (`ccdsimdonuts`) a depuis ajouté cette capacité au simulateur CCD lui-même (défocus progressif, obstruction du secondaire, décentrage de collimation modélisé en cônes coaxiaux avec flip intra/extra naturel, coma de champ par tracé de rayons géométrique façon Seidel) — utile pour tester le flux complet du wizard en conditions quasi réelles (focuser + monture + caméra simulés). Ça complète, sans le remplacer, le plan ci-dessous pour les tests unitaires/non-régression rapides de l'algo d'analyse :

**1. Génération procédurale de donuts synthétiques (prioritaire)**
Générateur d'images synthétiques indépendant du simulateur INDI, avec vérité terrain connue :
- Anneau externe (cercle/ellipse) de rayon R, centré en (cx, cy)
- Ombre centrale du secondaire : cercle concentrique de rayon r < R, décentrage indépendant possible pour simuler un défaut de collimation
- Déformation de type coma (asymétrie croissante avec la distance au centre du champ simulé) pour générer des champs multi-étoiles réalistes avec un point de convergence connu à l'avance
- Araignée (spider) en option : 2-4 branches fines traversant l'anneau
- Bruit gaussien + fond de ciel ajoutés par-dessus
- Réalisable avec OpenCV (`cv::circle`/`cv::ellipse` remplis + flou gaussien) directement en C++ dans les tests, ou via un script Python séparé produisant des fixtures FITS
- Intérêt : le centre exact et le point de convergence étant connus à l'injection, on vérifie que l'algo retrouve le bon vecteur de collimation à une tolérance donnée → tests de non-régression rapides, sans dépendance matérielle

**2. Fixtures réelles (complément)**
Quelques vraies images capturées sur setup réel avec un champ riche en étoiles, bien collimaté puis volontairement décollimaté à un niveau connu, conservées comme fixtures FITS dans le repo de tests. Valide l'algo face à du bruit/artefacts réels non reproductibles synthétiquement (poussière, vignettage, défauts optiques).

**Répartition :**
- Tests unitaires rapides et nombreux → générateur synthétique paramétrique (variations de décentrage, rayon, bruit, ellipticité, nombre/répartition d'étoiles dans le champ)
- Tests d'intégration/non-régression, moins nombreux → fixtures réelles, tolérance plus large

## Points d'intégration à identifier dans le code existant
- API/service Focuser (ostserver) pour piloter le déplacement vers la position défocalisée
- Pipeline caméra existant pour la capture d'image (format, cfitsio)
- Infos télescope existantes (focale, ouverture) pour le calcul du F/D utilisé dans le modèle de coma
- Protocole WebSocket/JSON existant pour définir les nouveaux messages du module (namespace `collimator`, ex. `collimator:analyze`, `collimator:screwOffsets`, `collimator:overlayData`)
- Frontend Angular : nouveau composant de wizard + composant overlay canvas/SVG sur le flux image live + composant jauges

## Notes techniques à creuser avec Claude Code
- Détection robuste du contour de chaque donut (seuillage adaptatif + détection de cercle/ellipse, ex. transformée de Hough ou fit par moindres carrés), avec séparation propre entre étoiles voisines dans un champ à plusieurs donuts
- Extraction du vecteur de déformation par étoile : valider en pratique (sur données synthétiques puis réelles) que le centroïde pondéré par intensité de l'anneau est un proxy fiable de la direction de déformation pour les donuts fortement comatiques (forme en virgule), pas seulement pour les cas proches du cercle simple
- Robustesse de la régression du point de convergence avec peu d'étoiles (3-5) et/ou mal réparties en champ (toutes proches du centre, ou alignées) — cas où le système est mal conditionné
- Faut-il estimer `k` (coefficient de coma) à chaque session, ou le calibrer une fois par instrument et le réutiliser (F/D fixe pour un setup donné) ?
- Gestion du bruit de fond de ciel et des étoiles voisines parasites dans le champ
- Choix de l'amplitude de défocalisation (assez pour des donuts nets et exploitables, pas trop pour rester dans un temps de pose raisonnable)
- Les trois formules (Newton, RC, SC) présentent une ombre du secondaire à l'intérieur du donut, à traiter comme un second contour concentrique attendu ; le contour externe et l'ombre centrale doivent être analysés conjointement dans tous les cas
