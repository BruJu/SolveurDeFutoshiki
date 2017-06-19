#ifndef __H_AFFICHAGE__
#define __H_AFFICHAGE__

#include "definitions.h"
#include "heuristique.h"

/**
 * Affiche le symbole de contrainte liant le slot et la caseB.
 * Si vertical = 1, affiche un ^ ou un v, sinon affiche un < ou un >
 *
 * Pr�requis : la contrainte doit exister.
 */
//void printContrainte(Slot * slot, int caseB, int vertical);


/**
 * Affiche la contrainte par rapport � la case num�ro actuel.
 * Par exemple si affichage est en haut � gauche et contrainte lie les deux premi�res cases
 * (typiquement la grille est de cette forme :
 * 2
 * 1>2
 * 2 1
 * )
 *
 * Pour 1>2 affiche : <2
 */
//void afficherContrainte(Contrainte * contrainte, int actuel);


/**
 * Affiche de nombreuses informations sur le slot. L'affichage est pr�c�d� de pr�face.
 * Sont affich�es : la valeur actuelle, les valeurs du domaine, les slots avec des contraintes en commun ...
 */
void afficherCaseF(Slot * slot, char * preface);

/**
 * Affiche le slot via afficherCaseF
 */
//void afficherCase(Slot * slot);


/**
 * Affiche la grille de jeu avec les valeurs instanci�es et les diff�rentes contraintes de mani�re
 * align�e.
 * Exemple d'affichage :
 * 3
 * 1 2 3
 * ^   v
 * 2 3 1
 *
 * 3 1 2
 */
void afficherGrilleComplete(CSP * jeu);

/**
 * Affiche le temps pour r�soudre la grille, le nombre de noeuds et le nombre de contraintes test�es
 */
void afficher_performance(Performance * perf);

/**
 * Affiche de nombreuses informations (ceux de afficherCaseF) sur tous les slots de la grille.
 */
void afficherEtatGrille(CSP * jeu);
void afficherGrille    (CSP * jeu);

/**
 * Renvoie vrai si la grille poss�de des nombres diff�rents sur ses lignes et colonnes.
 */
//int estValide(CSP * jeu)

/**
 * Affiche si la grille r�solue poss�de des nombres diff�rents sur ses lignes et colonnes.
 */
void doesItWork(CSP * jeu);


#endif
