#ifndef __H_HEURISTIQUE__
#define __H_HEURISTIQUE__

#include "definitions.h"




/* ==============================================
	HEURISTIQUE ORDRE
============================================== */

/**
 * Inverse les variables dans la liste des variables � instancier.
 * Utilis� pour que l'instanciation soit le m�me sans heuristique et
 * avec heuristique d'ordre
 */
void inverserVariablesRestantes(PileHeuristique * h);

/**
 * Choisis le slot � la fin de la liste
 */
int heuristiqueOrdre(PileHeuristique * h);


/* ==============================================
	HEURISTIQUE SUR GROUPE
============================================== */

/**
 * Fonction r�cursive qui donne � statut la valeur groupe pour tous les
 * slot num�ro pos et � tous les autres slots ayant une contrainte < ou
 * > en commun dont l'id est sup�rieur ou �gal � groupe.
 */
void mettreEnPlaceLesGroupes_setGroupe(CSP * jeu, int pos, int groupe);

/**
 * Donne des numeros de groupe � toutes les cases et renvoie un tableau
 * avec le nombre de membre dans le groupe de chaque slot.
 * Les numeros de groupe correspondent au plus petit id du membre du
 * groupe
 * Exemple pour la grille suivante :
 * 3
 * 0<0<0
 *   ^ 
 * 0 0 0
 * 
 * 0<0 0
 * 
 * Attribue cette division :		Et dans le tableau d'int renvoy� :
 * 0 0 0							4 4 4
 * - 0 -							1 4 1
 * 6 6 -							2 2 1
 * 
 * (- = -1, pas de groupe)
 */
int * mettreEnPlaceLesGroupes(CSP * jeu);

/**
 * Met dans statut le nombre de membres dans le m�me groupe de chaque
 * slot.
 */
void preparerHeuriGroupes(CSP * jeu);

/**
 * min ( dom/(2*deg) - tailleDuGroupe )
 */
int heuristiqueGroupe(PileHeuristique * h);


/* ==============================================
	HEURISTIQUE GROUPE REELS
============================================== */

/**
 * Rempli les variables globales
 * nbDeMembresTruePotential : Nombre de membres non instanci�s dans chaque groupe
 * groupeSurLigne           : Liste des groupes pr�sents sur chaque ligne
 * groupeSurColonne			: Liste des groupes pr�sents sur chaque colonne
 */
void preparerHeuriGroupeReels(CSP * jeu);

/**
 * Si un quart des slots ont �t� instanci�s :
 * min(domaine)
 * 
 * Sinon :
 * 
 * min(Domaine) > max(Somme du nombre de membres non instanci�s
 *         dans les groupes pr�sents sur la m�me ligne / colonne)
 * 
 */
int heuristiqueGroupesReels (PileHeuristique * h);

/* ==============================================
	HEURISTIQUE VARVAL
============================================== */

/**
 * Alloue des tableaux globaux de taille N�
 */
void preparerVarVal(CSP * jeu);

/**
 * Renvoie :
 * - une variable qui a une valeur dans son domaine dont elle est la seule
 * � l'avoir sur sa ligne ou colonne si elle existe
 * - la variable ayant le plus petit domaine si une telle variable n'existe pas
 */
int heuristiqueVarVal(PileHeuristique * h);


/**
 * Lib�re les tableaux globaux allouer pour exploiter VarVal
 */
void libererVarVal();



/* ==============================================
	HEURISTIQUES SANS PREPARATION
============================================== */

/**
 * Choisis le slot ayant le plus petit domaine
 */
int heuristiqueTailleDomaine(PileHeuristique * h);


/**
 * Renvoie le slot avec le plus petit degr�
 */
int heuristiqueNbContraintesRest(PileHeuristique * h);


/**
 * Choisis le slot qui a la plus petite valeur domaine / degre
 */
int heuristiqueDomDeg(PileHeuristique * h);

/**
 * min(Domaine/(2*Contraintes - ContraintesInf))
 */
int heuristiqueDomSurDeg2(PileHeuristique * h);

/**
 * min(Dom+Deg)
 */
int heuristiqueDomPlusDeg(PileHeuristique * h);


/**
 * Prend le maximum de :
 * - Du degre
 * - Auquel on ajoute pour chaque slot ayant une contrainte > ou < en
 * commun avec le slot en cours d'�valuation, la moiti� de la taille du
 * domaine
 */
int heuristiquePotentiel(PileHeuristique * h);






#endif
