#ifndef __H_RESOLUTION__
#define __H_RESOLUTION__

#include "definitions.h"		// CSP, ...

/* ===============================
 *         TEST DE CONTRAINTES
 * =============================== */

/**
 * Renvoie :
 * 0 si la contrainte n'est pas respectée
 * 1 si elle est respectée
 * 2 si au moins une case n'a pas de valeur assignée
 * (pour Backtrack)
 */
//int testerContrainte(Contrainte *contrainte);

/**
 * Teste la contrainte. Renvoi vrai si elle est respectée, 0 sinon.
 * (pour Forward Checking)
 */
//int testerContrainteFC(Contrainte *contrainte);

/* ===============================
 *         REDUCTION DE DOMAINE
 * =============================== */

/**
 * Réduit le domaine du slot cSlot.
 * Les valeurs éliminées sont retenues dans emilinateur à partir de la position posEmilinateur.
 * Renvoie 0 si un domaine est vide
 */
//int fc_reduireDomaine(Emilienator * emilinateur, int * posEmilinateur, Slot * cSlot)

/* ===============================
 *         RESOLUTION
 * =============================== */


/** 
 * Résout la grille de Futoshiki grâce à l'algorithme Backtrack de base
 */
int backtrack(CSP * jeu);

/**
 *  Résout la grille de Futoshiki grâce à l'algorithme Forward Checking
 * de base
 */
int forwardChecking(CSP * jeu);

/* ===============================
 *         RESOLUTION HEURISTIQUE
 * =============================== */

/**
 *  Utilise BackTrack en appliquant l'heuristique passée en paramètre
 */
int backtrackHeuristique(CSP * jeu, int (*heuristique)(PileHeuristique *));


/**
 *  Utilise Forward Checking en appliquant l'heuristique passée en paramètre
 */
int forwardCheckingHeuristique(CSP * jeu, int (*heuristique)(PileHeuristique *));




#endif
