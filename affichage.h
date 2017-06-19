#ifndef __H_AFFICHAGE__
#define __H_AFFICHAGE__

#include "definitions.h"
#include "heuristique.h"

/**
 * Affiche le symbole de contrainte liant le slot et la caseB.
 * Si vertical = 1, affiche un ^ ou un v, sinon affiche un < ou un >
 *
 * Prérequis : la contrainte doit exister.
 */
//void printContrainte(Slot * slot, int caseB, int vertical);


/**
 * Affiche la contrainte par rapport à la case numéro actuel.
 * Par exemple si affichage est en haut à gauche et contrainte lie les deux premières cases
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
 * Affiche de nombreuses informations sur le slot. L'affichage est précédé de préface.
 * Sont affichées : la valeur actuelle, les valeurs du domaine, les slots avec des contraintes en commun ...
 */
void afficherCaseF(Slot * slot, char * preface);

/**
 * Affiche le slot via afficherCaseF
 */
//void afficherCase(Slot * slot);


/**
 * Affiche la grille de jeu avec les valeurs instanciées et les différentes contraintes de manière
 * alignée.
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
 * Affiche le temps pour résoudre la grille, le nombre de noeuds et le nombre de contraintes testées
 */
void afficher_performance(Performance * perf);

/**
 * Affiche de nombreuses informations (ceux de afficherCaseF) sur tous les slots de la grille.
 */
void afficherEtatGrille(CSP * jeu);
void afficherGrille    (CSP * jeu);

/**
 * Renvoie vrai si la grille possède des nombres différents sur ses lignes et colonnes.
 */
//int estValide(CSP * jeu)

/**
 * Affiche si la grille résolue possède des nombres différents sur ses lignes et colonnes.
 */
void doesItWork(CSP * jeu);


#endif
