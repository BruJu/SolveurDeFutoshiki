#ifndef __H_RESOLUTIONDEDUCTIVE__
#define __H_RESOLUTIONDEDUCTIVE__

#include "definitions.h"
#include "affichage.h"


/* ===============================
 *         AMELIORATION DE FC
 * =============================== */
 
 #ifdef __OPT_FCDEDUCTIF
 
/**
 * Complète l'instance CSP afin d'avoir la structure de graphe pour les déductions
 * et effectue les premières déductions
 * 
 * Renvoie 0 si il y a des contraintes successives qui rendent la résolution impossible
 * Renvoie 0 si l'allocation de la pile de déduction a produit une erreur
 * Renvoie 0 si il y a une chaine de contraintes plus longues que la taille du jeu
 * 
 * Exemple sur un extrait de grille
 * a < b   a = 1, b = 2, d = 3, c = 4, a = 5, impossible car a a deux valeurs différentes
 * v   ^
 * c > d
 * 
 * Exemple de déduction possible dès le début sur un extrait de grille 2x2 :
 * a < b  a < b, donc a inférieur à max de b (2), donc a n'a pas 2 dans son domaine
 *               donc b supérieur à min de a (1), donc b n'a pas 1 dans son domaine
 * c   d
 * 
 * 
 * Cette fonction n'est pas optimisée complètement car elle n'est appellée qu'une fois.
 */
int fcd_completerCSP(CSP * jeu, EmilieDeduit ** emilieDeduit, Emilienator * emilinateur, int * posEmilinateur);

/**
 * Vide emilieDeduit en réduisant les domaines qu'il faut réduire pour faire
 * des déductions
 * 
 * 
 * Exemple : a < b < c
 *               v
 *               d < e
 * On instancie a (EmilieSlot). fcd_reduireDomaine va réduire le domaine de b
 * et empile b dans emilieDeduit.
 * Cette fonction se charge de réduire les domaines de c, d et e.
 * 
 * Renvoie 0 si un domaine est vide
 */
int fcd_deduction(Emilienator * emilinateur, int * posEmilinateur,
					EmilieDeduit * emilieDeduit , int * posEmilieDeduit,
					Slot * EmilieSlot);

/**
 * Copier coller de fc_reduireDomaine
 * La seule différence étant que lorsqu'un domaine est réduit, le slot est
 * empilé dans emilieDeduit
 */
int fcd_reduireDomaine(Emilienator * emilinateur, int * posEmilinateur,
						EmilieDeduit * emilieDeduit , int * posEmilieDeduit,
						Slot * cSlot);

/**
 * Résout la grille de futoshiki grâce à Forward Checking amélioré via deux aspects :
 * - Tente d'appliquer une pseudo arc consistance via les bornes de chaque case et
 *  de ses voisines ayant une contrainte < ou > en commun
 * - Vérifie plus efficacement les contraintes
 */
int forwardCheckingDeductif(CSP * jeu, int (*heuristique)(PileHeuristique *));

#endif // __OPT_FCDEDUCTIF

#endif
