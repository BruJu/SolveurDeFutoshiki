# SolveurDeFutoshiki

Il s'agit d'un solveur de futoshiki (jeu proche du Sudoku) réalisé à l'occasion
de ma L3 Informatique dans le cours d'Intelligence Artificielle dispensé par
Cyril Terrioux.

Vous trouverez le sujet d'origine dans le fichier projet.pdf

## Utilisation (sous Linux)

Compilation : `make all`

Utilisation : `./solver [nom du fichier] (BT/FC/FCD) (Nom d'une heuristique)`

Afficher l'aide avec toutes les heuristiques gérées : `./solver`


## Performances

Une modélisation très basique a été rapidement faite du problème avec le solveur Choco en Java.

La source est disponible dans le fichier `Futoshiki.java`.

Le code n'étant pas documenté et écrit rapidement, voici comment l'utiliser en une ligne Java :

`new Futoshiki().resoudreGrille("/path/to/file");`

Les performances avec Choco sont meilleures que avec le programme écrit en C.
(résolution plus rapide et les grilles qui ne sont pas résolues par le solveur
le sont par Choco).