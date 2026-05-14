# Module Template

## Description

Ce module est un squelette/template pour créer de nouveaux modules OST. Il démontre toutes les fonctionnalités de base :

- Gestion des périphériques INDI
- Création et mise à jour de propriétés
- Gestion des images (BLOB)
- Sauvegarde/chargement de profils
- Barre de progression
- Grille de résultats

## Utilisation

### Paramètres

- **Iterations** : Nombre d'itérations à effectuer (1-100)
- **Threshold** : Seuil de détection (0.1-10.0)
- **Enable Option** : Active/désactive une option de traitement

### Périphériques requis

- **Camera** : Caméra pour acquisition d'images

### Actions

- **Start** : Démarre l'opération
- **Abort** : Annule l'opération en cours

### Résultats

Les résultats sont affichés dans une grille avec :
- Numéro d'itération
- Valeur mesurée
- Horodatage

## Profils

Les paramètres peuvent être sauvegardés et rechargés via les profils :

1. Ajustez les paramètres
2. Entrez un nom de profil
3. Cliquez sur l'icône de sauvegarde
4. Pour charger : sélectionnez le profil et cliquez sur l'icône de chargement

## Adaptation du module

Pour créer votre propre module :

1. Copiez le dossier `template` avec un nouveau nom
2. Renommez tous les fichiers (template.h → votremodule.h, etc.)
3. Dans les fichiers .h et .cpp :
   - Remplacez "Template" par "VotreModule"
   - Remplacez "template" par "votremodule"
4. Adaptez template.json à vos besoins
5. Implémentez votre logique dans startOperation() et processImage()
6. Ajoutez le module dans CMakeLists.txt

## Notes techniques

- Le module hérite de `IndiModule` pour l'accès INDI
- Les propriétés sont chargées depuis template.json
- Les profils sont automatiquement gérés via la classe de base
- Les images sont traitées via la classe `fileio`
