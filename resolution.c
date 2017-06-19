#include "resolution.h"
#include "options.h"		// Performance
#include "gestHeuristique.h"

extern Performance performance;
extern int heuristiqueSurDegre;


/* ==========================================
 * ==============    INLINE    ==============
 * ========================================== */

/**
 * Enlève la valeur donnée au slot
 */
static inline void desinstancier(Slot * slot) {
	slot->valeur = 0;
}

/**
 * Donne au slot la valeur valeur
 */
static inline void instancier(Slot * slot, int valeur) {
	slot->valeur = valeur;
}

/**
 *  Empile au dessus de la pile d'élimination l'élimination d'une valeur
 * de rSlot à cause de l'instanciation de rSlot
 */
static inline void emiliner(Emilienator * emilinateur, int * posEmilinateur,
							Slot * cSlot, Slot * rSlot, int valeur) {
	emilinateur[*posEmilinateur].elimineur      = cSlot;
	emilinateur[*posEmilinateur].victime        = rSlot;
	emilinateur[*posEmilinateur].valeur_retiree = valeur;
	(*posEmilinateur) ++;
}


/* ==========================================
 * ===========    CONTRAINTES    ============
 * ========================================== */

int testerContrainte(Contrainte *contrainte) {
	if (contrainte->caseA->valeur == 0 || contrainte->caseB->valeur == 0)
		return 2;

#ifdef __OPT_TESTCOUNT
	performance.test_done ++;
#endif
	
	if (contrainte->type == DIF) {
		return (contrainte->caseA->valeur != contrainte->caseB->valeur) ? 1 : 0;
	} else {
		return (contrainte->caseA->valeur < contrainte->caseB->valeur) ? 1 : 0;
	}
}

int testerContrainteFC(Contrainte *contrainte) {
#ifdef __OPT_TESTCOUNT
	performance.test_done ++;
#endif
	
	if (contrainte->type == DIF) {
		return (contrainte->caseA->valeur != contrainte->caseB->valeur);
	} else {
		return (contrainte->caseA->valeur < contrainte->caseB->valeur);
	}
}

/* ==========================================
 * =======    REDUCTION DE DOMAINE    =======
 * ========================================== */


int fc_reduireDomaine(Emilienator * emilinateur, int * posEmilinateur, Slot * cSlot) {
	int iCont;			// numéro de la contrainte
	
	Slot * rSlot;		// slot sur lequel on vérifie actuellement le domaine
	int d;				// exploration du domaine de rslot
	
	for (iCont = cSlot->debutcontraintes ; iCont < cSlot->maxcontraintes ; iCont++) {
		// Détermination de rSlot
		if (cSlot == cSlot->contraintes[iCont]->caseA) {
			rSlot = cSlot->contraintes[iCont]->caseB;
		} else {
			rSlot = cSlot->contraintes[iCont]->caseA;
		}
		
		// Ne pas réduire le domaine d'une variable déjà instanciée
		if (rSlot->valeur != 0) {
			continue;
		}
		
		// Exploration du domaine de rslot
		d = 0;
		while (d < rSlot->maxdomain) {
			rSlot->valeur = rSlot->domain[d];
			
			if (!testerContrainteFC(cSlot->contraintes[iCont])) {
				// Contrainte violée
				emiliner(emilinateur, posEmilinateur, cSlot, rSlot, rSlot->valeur);
				
				// Reduction du domaine en swappant la valeur éliminée et la dernière valeur
				rSlot->maxdomain--;
				rSlot->domain[d] = rSlot->domain[rSlot->maxdomain];
			} else {
				d++;
			}
		}
		
		rSlot->valeur = 0;
		
		// Domaine de rSlot vide : l'instanciation courante en prenant cSlot->valeur ne mène pas à une solution
		if (rSlot->maxdomain == 0) {
			return 1;
		}
	}
	
	return 0;
}


/* ===============================
 *         RESOLUTION DE BASE
 * =============================== */


int backtrack(CSP * jeu) {
	int * iDom;					// Exploration du domaine de chaque variable dans l'ordre
	int N2 = jeu->N * jeu->N;	// Nombre de slots à parcourir
	int iVar;					// Numéro de la variable en cours d'instanciation
	int vientDEchouer;
	int j;						// Parcours de contrainte
	
	Slot * currentSlot;
	
#ifdef __OPT_TIMER
	performance.debut = clock();
#endif
	
	// Allocation mémoire
	iDom = malloc(sizeof(int) * N2);
	
	if (iDom == NULL) {
		perror("malloc iDom");
		return 0;
	}
	
	iVar = 0;
	vientDEchouer = 0;
	
	
	while (iVar != N2 && iVar != -1) {
		currentSlot = &(jeu->grille[iVar]);
		
		if (!vientDEchouer) {	// On ne vient pas d'un échec
			iDom[iVar] = 0;
		} else {				// Essai d'une nouvelle valeur
			iDom[iVar] ++;
		}
		
		if (currentSlot->domain[iDom[iVar]] == -1) { // Fin de l'exploration du domaine
			currentSlot->valeur = 0;
			vientDEchouer = 1;
			iVar--;
		} else { // Exploration du domaine de la variable courante
			currentSlot->valeur = currentSlot->domain[iDom[iVar]];
			
#ifdef __OPT_NODECOUNT
			performance.nodes ++;
#endif
			
			vientDEchouer = 0;
			
			// Tester la consistance
			for (j = 0 ; j < currentSlot->maxcontraintes ; j++) {
				if (! testerContrainte(currentSlot->contraintes[j])) {
					vientDEchouer = 1; // echec immédiat
					break;
				}
			}
			
			if (vientDEchouer == 0) {
				// Exploration d'une nouvelle variable
				iVar++;
			}
		}
	}
	
	free(iDom);

#ifdef __OPT_TIMER
	performance.fin = clock();
#endif
	
	return iVar != -1;
}

int forwardChecking(CSP * jeu) {
	int * iDom;						// Exploration du domaine de chaque variable dans l'ordre
	
	Emilienator * emilinateur;		// Pile de valeurs éliminées
	int posEmilinateur;				// Sommet de pile
	
	int N2 = jeu->N2;				// Bombre de variables à instacier
	
	
	int    iSlot;					// Numéro de la variable en cours d'instanciation
	Slot * cSlot;					// Variable en cours d'instanciation
	Slot * rSlot;					// Variable dont le domaine a été réduit à cause de cSlot
	
	int provenance;					// Souvenir de l'échec ou non
	
#ifdef __OPT_TIMER
	performance.debut = clock();
#endif
	
	// Allocation mémoire
	iDom = malloc(sizeof(int) * N2);
	if (iDom == NULL) {
		perror("malloc iDom");
		return 0;
	}
	
	/* 
	 * Emilinateur est une pile de valeurs éliminées
	 *  ie une pile de (ensemble de (variable instanciée, variable dont le domaine est réduit, valeur retirée du domaine) )
	 * > On peut borner le nombre de valeurs éliminées en supposant que toute la première instanciation 
	 * supprime toutes les valeurs des domaines des autres variables
	 * Taille d'un domaine : N, nombre d'autres variables : NxN - 1
	 * donc taille requise <= N x (NxN - 1)
	 * */
	emilinateur = malloc(sizeof(Emilienator) * (N2 -1) * jeu->N);
	if (emilinateur == NULL) {
		perror("malloc emilinateur");
		free(iDom);
		return 0;
	}
	
	// Initialisation
	iSlot = 0;
	provenance = 0;
	posEmilinateur = 0;

	while (iSlot != -1 && iSlot != N2) {
		cSlot = &(jeu->grille[iSlot]);
		
		if (!provenance) {
			iDom[iSlot] = 0;
		} else {
			// La valeur actuelle de cSlot est mauvaise : on restaure les domaines supprimé jusque là par cSlot
			while (posEmilinateur > 0 && emilinateur[posEmilinateur-1].elimineur == cSlot) {
				rSlot = emilinateur[posEmilinateur-1].victime;
				rSlot->domain[rSlot->maxdomain++] = emilinateur[posEmilinateur-1].valeur_retiree;
				
				posEmilinateur --;
			}
			
			iDom[iSlot] ++;
		}
		
		
		if (iDom[iSlot] == cSlot->maxdomain) { // Fin de l'exploration du domaine
			cSlot->valeur = 0;
			provenance = 1;
			iSlot--;
		} else { // Exploration du domaine de la cSlot (variable en train d'être instanciée)
			cSlot->valeur = cSlot->domain[iDom[iSlot]];
			
#ifdef __OPT_NODECOUNT
			performance.nodes ++;
#endif
			
			// Réduction du domaine des autres variables
			provenance = fc_reduireDomaine(emilinateur, &posEmilinateur, cSlot);
			
			if (provenance == 0) {
				iSlot ++;
			}
		}
	}
	
	free(iDom);
	free(emilinateur);
	
#ifdef __OPT_TIMER
	performance.fin = clock();
#endif
	
	return iSlot != -1;
}



/* ===============================
 *         RESOLUTION AVEC HEURISTIQUE
 * =============================== */


int backtrackHeuristique(CSP * jeu, int (*heuristique)(PileHeuristique *)) {
	int vientDEchouer;						// Etat de l'exploration
	int j;									// Parcours de contrainte
	
	PileHeuristique * piles;				// Gestion des variables déjà instanciées / à instancier
	
	Slot * currentSlot;						// Variable en cours d'instanciation
	ExplorationDeVariable * currentInst;	// Couple (Slot, position dans le domaine)
	
#ifdef __OPT_TIMER
	performance.debut = clock();
#endif
	
	// Initialisation de la structure nous permettant de gérer les heuristiques
	piles = preparerExploitationHeuristique(jeu);
	if (piles == NULL)
		return 0;
	
	initialiser_heuristique(heuristique, jeu, piles);
	
	// Instancier une variable
	vientDEchouer = 0;
	currentInst = pileHeuriExtraireVariable(piles, heuristique(piles));
	
	while (currentInst != NULL) {
		currentSlot = currentInst->slot;
		
		if (currentSlot->maxdomain == currentInst->iDom) {
			// Fin de l'exploration du domaine
			desinstancier(currentSlot);
			currentInst = retourArriere(piles);
			currentInst->iDom++;
		} else {
			// Choix d'une nouvelle valeur
			instancier(currentSlot, currentSlot->domain[currentInst->iDom]);
			
#ifdef __OPT_NODECOUNT
			performance.nodes ++;
#endif
			
			vientDEchouer = 0;
			
			// tester la consistance
			for (j = 0 ; j < currentSlot->maxcontraintes ; j++) {
				if (!testerContrainte(currentSlot->contraintes[j])) {
					vientDEchouer = 1; // echec immédiat
					break;
				}
			}
			
			// Préparation du noeud suivant
			if (!vientDEchouer) {
				currentInst = pileHeuriExtraireVariable(piles, heuristique(piles));
			} else {
				currentInst->iDom ++;
			}
		}
	}

#ifdef __OPT_TIMER
	performance.fin = clock();
#endif
	
	liberer_preparationHeuristique(heuristique);
	libererPileHeuristique(piles);
	
	return !vientDEchouer;
}


int forwardCheckingHeuristique(CSP * jeu, int (*heuristique)(PileHeuristique *)) {
	PileHeuristique * piles;				// Gestion des variables
	
	Emilienator * emilinateur;				// Eliminations
	int posEmilinateur;
	Slot * rSlot;
	
	ExplorationDeVariable * currentInst;	// Variable instanciée
	Slot * cSlot;
	
	int vientDunEchec;
	
#ifdef __OPT_TIMER
	performance.debut = clock();
#endif
	
	// Allocations de mémoire pour les structures de pseudo recursion
	piles = preparerExploitationHeuristique(jeu);
	if (piles == NULL) return 0;
	
	emilinateur = malloc(sizeof(Emilienator) * (jeu->N2 -1) * jeu->N);
	if (emilinateur == NULL) {
		perror("malloc emilinateur");
		libererPileHeuristique(piles);
		return 0;
	}
	
	initialiser_heuristique(heuristique, jeu, piles);
	
	// Initialisation
	posEmilinateur = 0;
	vientDunEchec = 0;
	
	currentInst = pileHeuriExtraireVariable(piles, heuristique(piles));
	
	while (currentInst != NULL) {
		cSlot = currentInst->slot;
		
		if (vientDunEchec) {
			// Restauration des valeurs precedemment retirées
			while (posEmilinateur > 0 && emilinateur[posEmilinateur-1].elimineur == cSlot) {
				rSlot = emilinateur[posEmilinateur-1].victime;
				rSlot->domain[rSlot->maxdomain++] = emilinateur[posEmilinateur-1].valeur_retiree;
				
				posEmilinateur --;
			}
			
			// Avance dans le domaine
			currentInst->iDom++;
		}
		
		
		if (currentInst->iDom == cSlot->maxdomain) { // Fin de l'exploration du domaine
			desinstancier(cSlot);
			currentInst = retourArriere(piles);
			vientDunEchec = 1;
		} else { // Exploration du domaine de la cSlot
			instancier(cSlot, cSlot->domain[currentInst->iDom]);
			
#ifdef __OPT_NODECOUNT
			performance.nodes ++;
#endif
			
			vientDunEchec = fc_reduireDomaine(emilinateur, &posEmilinateur, cSlot);
			
			if (!vientDunEchec) {
				currentInst = pileHeuriExtraireVariable(piles, heuristique(piles));
			}
		}
	}
	
	free(emilinateur);
	liberer_preparationHeuristique(heuristique);
	libererPileHeuristique(piles);
	
#ifdef __OPT_TIMER
	performance.fin = clock();
#endif
	
	return !vientDunEchec;
}
