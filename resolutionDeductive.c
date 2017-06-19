#include "resolutionDeductive.h"
#include "options.h"				// Performances
#include "gestHeuristique.h"

extern Performance performance;
extern int heuristiqueSurDegre;


void razTableau (int * tableau, int N) {
	for (int i = 0 ; i < N ; i++) {
		tableau[i] = 0;
	}
}

void afficherTableau(char * pref, int ligne, int * tableau, int N) {
	printf("%s %d : ", pref, ligne);
	
	for (int i = 0 ; i < N ; i ++) {
		printf("%d ", tableau[i]);
	}
	
	printf("\n");
}

void incrementerViaSlot(int * tableau, Slot * slot) {
	for (int d = 0 ; d != slot->maxdomain ; d++) {
		tableau[slot->domain[d]-1] ++;
	}
}

void afficherNbOccurrences (CSP * jeu) {
	int ligne;
	int colonne;
	int tableau[jeu->N];
	
	
	for (ligne = 0 ; ligne < jeu->N ; ligne++) {
		razTableau(tableau, jeu->N);
		
		for (colonne = 0 ; colonne < jeu->N ; colonne++) {
			incrementerViaSlot(tableau, &(jeu->grille[ligne*jeu->N + colonne]));
		}
		
		afficherTableau("ligne", ligne+1, tableau, jeu->N);
	}
	
	for (ligne = 0 ; ligne < jeu->N ; ligne++) {
		razTableau(tableau, jeu->N);
		
		for (colonne = 0 ; colonne < jeu->N ; colonne++) {
			incrementerViaSlot(tableau, &(jeu->grille[ligne + colonne*jeu->N]));
		}
		
		afficherTableau("colonne", ligne+1, tableau, jeu->N);
	}
	
}

/* ===============================
 *         INLINE
 * =============================== */
 
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
 *  Emilie rempli une feuille avec l'élimination de valeur du domaine
 * de rSlot à cause de cSlot et la met en haut de la pile des autres
 * éliminations
 */
static inline void emiliner(Emilienator * emilinateur, int * posEmilinateur,
							Slot * cSlot, Slot * rSlot, int valeur) {
	emilinateur[*posEmilinateur].elimineur      = cSlot;
	emilinateur[*posEmilinateur].victime        = rSlot;
	emilinateur[*posEmilinateur].valeur_retiree = valeur;
	(*posEmilinateur) ++;
}

/* ===============================
 *         FORWARD CHECKING DEDUCTIF
 * =============================== */

#ifdef __OPT_FCDEDUCTIF

int fcd_completerCSP(CSP * jeu, EmilieDeduit ** emilieDeduit, Emilienator * emilinateur, int * posEmilinateur) {
	int numCase;					// Numéro de la case en cours de traitement
	
	EmilieDeduit * emDed;			// Enlever une * à emilieDeduit
	int pos_emDed; 					// Haut de la pile emilieDeduit

	Slot * slot;					// jeu->grille[numCase]
#ifndef __OPT_FCDEDUCTIF_BYPASS_VALIDITE
	Slot *aSlot;					// Slot dépilé
	int niveau;						// Nombre de < séparant le aSlot et slot
	Sens sensObserve = SENS_INF;	// Sens des indices observés
#endif
	
	/* ===========================
	 * REMPLISSAGE DE LA STRUCTURE
	 * =========================== */
	for (numCase = 0 ; numCase != jeu->N2 ; numCase++) {
		slot = &(jeu->grille[numCase]);
		
		slot->valeurMinimale = 0;			// On met des valeurs plus petites / grandes 
		slot->valeurMaximale = jeu->N+1;	// afin que la déduction se fasse.
	}
	
	
	/* ===================================================
	 * NON RECURSIVITE ET ABSENCE DE CHAINES TROP LONGUES
	 * =================================================== */
	 
	/*
	 * Pour la vérification de non recursivité, on prend chaque case et on regarde si on peut y retourner
	 *  Nombre de cases à regarder : N² x 2 sens
	 *  Borne supérieure que cela va entrainer de cases intermediaires : 4 x (N+1)
	 *   si on arrive à N+1 la grille ne peut pas être résolue.
	 * 
	 * Total : 2 N² + 4N + 4
	 * 
	 * 
	 * Par la suite quand on va réduire des domaines, la reduction peut demander
	 * (n-1)x2 cases x 2 sens déduction
	 * Une déduction peut empiler 4 autres déductions à faire : + 4 x N
	 * Total : 4N - 4 + 4N x N = 4N² + 4N - 4
	 * 
	 * TODO : mettre ça dans le rapport et vérifier les calculs
	 */
	
	// Allocation de mémoire pour les déductions
	*emilieDeduit = malloc(sizeof(EmilieDeduit) * (4*jeu->N2 + 4*jeu->N));
	if (*emilieDeduit == NULL) {
		perror("Emilie deduit");
		return 0;
	}
	emDed = *emilieDeduit;
	
#ifndef __OPT_FCDEDUCTIF_BYPASS_VALIDITE
	
	{
		/* La manière "jolie" de le faire serait :
		 * sensPossibles[2] = {SENS_INF, SENS_SUP}
		 * for (int i = 0 ; i < 2 ; i++) {
		 * 	sensObserve = sensPossible[i]
		 *  [code]
		 * }
		 *  */
		 
		sensObserve = SENS_INF;
recommencerAnalyseNonRec:
		
		for (numCase = 0 ; numCase != jeu->N2 ; numCase++) {
			slot = &(jeu->grille[numCase]);
			
			// slot de départ : le slot que l'on souhaite vérifier
			emDed[0].victime = slot;
			emDed[0].sens    = FRERE;
			
			pos_emDed = 1;
			niveau = 0;
			
			/* On empile chaque slot qui suit le slot en haut de la pile.
			 * Lorsqu'il n'y a plus de slot qui suit (ou qu'on vient de les dépiler),
			 * on dépile le slot.
			 * 
			 * Si la chaîne dépasse jeu->N slots ou si on retombe sur slot, la grille
			 * ne peut pas être résolue.
			 * 
			 * Si on arrive à une pile vide, c'est qu'il est possible de donner une
			 * valeur au slot.
			 * */
			while (pos_emDed != 0) {
				if (niveau >= jeu->N) {	// Chaîne de contraintes trop longues
					return 0;
				}
				
				// Slot du haut de la pile.
				aSlot = emDed[pos_emDed-1].victime;
				
				
				// Slot non visité : empiler tous les sommets suivants
				if (emDed[pos_emDed-1].sens == FRERE) {
					emDed[pos_emDed-1].sens = PERE;		// marqueur disant que le slot a été visité
					niveau++;
					
					if (sensObserve == SENS_INF) {
						if (aSlot->slotInfSup[0] != NULL) {
							// Empiler tous les suivants
							for (int i = 0 ; aSlot->slotInfSup[i] != NULL ; i++) {
								if (aSlot->slotInfSup[i] == slot)	// recursivité
									return 0;
								
								emDed[pos_emDed].victime = aSlot->slotInfSup[i];
								emDed[pos_emDed++].sens  = FRERE;
							}
							
							continue;
						}
					} else {
						if (aSlot->slotInfSup[4] != NULL) {
							for (int i = 4 ; aSlot->slotInfSup[i] != NULL ; i--) {
								if (aSlot->slotInfSup[i] == slot)
									return 0;
									
								emDed[pos_emDed].victime = aSlot->slotInfSup[i];
								emDed[pos_emDed++].sens  = FRERE;
							}
							
							continue;
						}
					}
				}
				
				// Le slot courant a été visité
				niveau--;
				pos_emDed--;
			}
		}
		
		if (sensObserve == SENS_INF) {
			sensObserve = SENS_SUP;
			goto recommencerAnalyseNonRec;
		}
		
	}
	
	
#endif


	/* ===================================================
	 * DEDUCTIONS DE BASE SUR LE PLATEAU DE JEU
	 * =================================================== */

	// Empiler des demandes de déduction à partir de toutes les cases
	//qui ont une contrainte < ou >
	pos_emDed = 0;
	for(numCase = 0 ; numCase != jeu->N2 ; numCase++) {
		slot = &(jeu->grille[numCase]);
		
		if(slot->slotInfSup[0] != NULL) {
			emDed[pos_emDed  ].victime = slot;
			emDed[pos_emDed++].sens    = SENS_INF;
		}
		
		if (slot->slotInfSup[4] != NULL) {
			emDed[pos_emDed  ].victime = slot;
			emDed[pos_emDed++].sens    = SENS_SUP;
		}
	}
	
	return fcd_deduction(emilinateur, posEmilinateur, emDed, &pos_emDed, NULL);
}

int fcd_deduction(Emilienator * emilinateur, int * posEmilinateur,
					EmilieDeduit * emilieDeduit , int * posEmilieDeduit,
					Slot * EmilieSlot) {
	Slot * cSlot;		// Slot en haut de emilieDeduit
	Slot * aSlot;		// Slot ayant une contrainte en commun avec cSlot
	Sens sens;			// Sens s'observation (slots inférieurs ou supérieurs)
	int i;				// Parcours des aSlot
	int iDom;			// Parcours du domaine de cSlot à la recherche de la nouvelle borne
	int nouvelleBorne;	// Sauvegarde temporaire de la borne trouvée
	int ii;				// Parcours du domaine de aSlot et non empilement de aSlot déjà empilés.
	
	
	while(*posEmilieDeduit != 0) {
		(*posEmilieDeduit) --;
		
		cSlot = emilieDeduit[*posEmilieDeduit].victime;
		sens  = emilieDeduit[*posEmilieDeduit].sens;
		
		if (sens == SENS_INF) {
			// Actualisation de la borne
			nouvelleBorne = cSlot->domain[0];
			if (nouvelleBorne == cSlot->valeurMinimale) {
couperLaBoucleInf:
				continue;
			}
			
			for (iDom = 1 ; iDom != cSlot->maxdomain ; iDom++) {
				if (cSlot->valeurMinimale == cSlot->domain[iDom]) {
					goto couperLaBoucleInf;	// break + continue;
				}
				
				if (cSlot->domain[iDom] < nouvelleBorne) {
					nouvelleBorne = cSlot->domain[iDom];
				}
			}
			
			cSlot->valeurMinimale = nouvelleBorne;
			
			// Parcours des slots inférieurs voisins
			for (i = 0 ; cSlot->slotInfSup[i] != NULL ; i++) {
				aSlot = cSlot->slotInfSup[i];
				
				// aSlot instancié : inutile de déduire
				if (aSlot->valeur != 0) {
					continue;
				}
				
				// == Reduction du domaine
				// Il n'y aura plus de valeur dans le domaine (on va tout couper)
				// [Exemple : min de cSlot = était 1, est désormais 4 ; domaine de aSlot : 2 3]
#ifdef __OPT_COMPTERDEDUCTION
				performance.deduction++;
#endif
				if (cSlot->valeurMinimale > aSlot->valeurMaximale) {
					return 0;
				}
				
				// Seulement si il faut réduire le domaine
				// [Exemple : cSlot avait pour domaine 1 2, aSlot a pour domaine 3 4
				//       il se trouve que cSlot a désormais pour min 2 qui est plus petit que 3
				//       ça sert à rien de tenter d'éliminer]

#ifdef __OPT_COMPTERDEDUCTION
				performance.deduction++;
#endif
				if (cSlot->valeurMinimale >= aSlot->valeurMinimale) {
					for (ii = 0 ; ii < aSlot->maxdomain ; ii++) {
#ifdef __OPT_COMPTERDEDUCTION
						performance.deduction++;
#endif
						if (aSlot->domain[ii] <= cSlot->valeurMinimale) {
							emilinateur[*posEmilinateur].elimineur      = EmilieSlot;
							emilinateur[*posEmilinateur].victime        = aSlot;
							emilinateur[*posEmilinateur].valeur_retiree = aSlot->domain[ii];
							(*posEmilinateur) ++;
							
							aSlot->maxdomain--;
							aSlot->domain[ii] = aSlot->domain[aSlot->maxdomain];
						}
					}
					
					if (aSlot->maxdomain == 0) {
						return 0;
					}
				}
				
				
				// Appel Reccursif : on peut peut être déduire des choses de la case qu'on vient de réduire.
				if (aSlot->slotInfSup[0] != NULL) {
					// Ne pas empiler deux fois
					for (ii = 0 ; ii < *posEmilieDeduit ; ii++) {
						if (emilieDeduit[ii].victime == aSlot && emilieDeduit[ii].sens == SENS_INF) {
							break;
						}
					}
					
					// empiler
					if (ii == *posEmilieDeduit) {
						emilieDeduit[*posEmilieDeduit].victime = aSlot;
						emilieDeduit[*posEmilieDeduit].sens = SENS_INF;
						(*posEmilieDeduit) ++;
					}
				}
				
			}
			
		} else { /* SENS_SUP : le code est le même */
			nouvelleBorne = cSlot->domain[0];
			if (nouvelleBorne == cSlot->valeurMaximale) {
couperLaBoucleSup:
				continue;
			}
			
			for (iDom = 1 ; iDom != cSlot->maxdomain ; iDom++) {
				if (cSlot->valeurMaximale == cSlot->domain[iDom]) {
					goto couperLaBoucleSup;
				}
				
				if (cSlot->domain[iDom] > nouvelleBorne) {
					nouvelleBorne = cSlot->domain[iDom];
				}
			}
			
			cSlot->valeurMaximale = nouvelleBorne;
			
			for (i = 4 ; cSlot->slotInfSup[i] != NULL ; i--) {
				aSlot = cSlot->slotInfSup[i];
				
				if (aSlot->valeur != 0) {
					continue;
				}

#ifdef __OPT_COMPTERDEDUCTION
				performance.deduction++;
#endif
				if (cSlot->valeurMaximale < aSlot->valeurMinimale) {
					return 0;
				}


#ifdef __OPT_COMPTERDEDUCTION
				performance.deduction++;
#endif
				if (cSlot->valeurMinimale >= aSlot->valeurMinimale) {
					for (ii = 0 ; ii < aSlot->maxdomain ; ii++) {
						
#ifdef __OPT_COMPTERDEDUCTION
						performance.deduction++;
#endif
						
						if (aSlot->domain[ii] >= cSlot->valeurMaximale) {
							emilinateur[*posEmilinateur].elimineur      = EmilieSlot;
							emilinateur[*posEmilinateur].victime        = aSlot;
							emilinateur[*posEmilinateur].valeur_retiree = aSlot->domain[ii];
							
							(*posEmilinateur) ++;
							
							aSlot->maxdomain--;
							aSlot->domain[ii] = aSlot->domain[aSlot->maxdomain];
						}
					}
					
					if (aSlot->maxdomain == 0) {
						return 0;
					}
				}
				
				if (aSlot->slotInfSup[4] != NULL) {
					for (ii = 0 ; ii < *posEmilieDeduit ; ii++) {
						if (emilieDeduit[ii].victime == aSlot && emilieDeduit[ii].sens == SENS_SUP) {
							break;
						}
					}
					if (ii == *posEmilieDeduit) {
						emilieDeduit[*posEmilieDeduit].victime = aSlot;
						emilieDeduit[*posEmilieDeduit].sens = SENS_SUP;
						(*posEmilieDeduit) ++;
					}
				}
				
			}
			
		}
		
		
	}
	
	return 1;
}

int fcd_reduireDomaine(Emilienator * emilinateur, int * posEmilinateur,
						EmilieDeduit * emilieDeduit , int * posEmilieDeduit,
						Slot * cSlot) {
	int iCont;							// Parcours des contraintes de cSlot
	Contrainte * contrainte;			// Contrainte iCont
	
	TypeContrainte sens;				// DIF si cSlot != rSlot
										// INF si cSlot < rSlot, SUP si cSlot > rSlot
	
	Slot * rSlot;						// Autre slot que cSlot dans la contrainte iCont
	int tailleOriginelle;				// Taille d'origine du domaine de rSlot
	int d;								// Parcours du domaine de rSlot
	int maxMod;		int minMod;			// Min ou Max de rSlot éliminé
	
	// Sauvegarde de la valeur choisie pour cSlot
	int valeurDansLaCase = cSlot->valeur;
	
	for (iCont = cSlot->debutcontraintes ; iCont < cSlot->maxcontraintes ; iCont++) {
		contrainte = cSlot->contraintes[iCont];
		
		if (cSlot == contrainte->caseA) {
			rSlot = contrainte->caseB;
		
			// Ne pas aller plus loin si l'autre slot est instancié
			if (rSlot->valeur != 0) {
				continue;
			}
			
			sens  = contrainte->type;
		} else {
			rSlot = contrainte->caseA;
			
			if (rSlot->valeur != 0) {
				continue;
			}
			
			sens = (contrainte->type == DIF) ? DIF : SUP;
		}
		
		
		tailleOriginelle = rSlot->maxdomain;		// Retenue de la taille initiale du domaine
		
		d = 0; minMod = 0; maxMod = 0;

		if (sens == DIF) {
			// Parcours du domaine
			while (d < rSlot->maxdomain) {
#ifdef __OPT_TESTCOUNT
	performance.test_done ++;
#endif
				// Eliminer la valeur du domaine ayant la même valeur
				if (rSlot->domain[d] == valeurDansLaCase) {
					emiliner(emilinateur, posEmilinateur, cSlot, rSlot, rSlot->domain[d]);
					
					// Retenue de si on a éliminé le minimum ou le maximum pour les déductions.
					if (rSlot->domain[d] == rSlot->valeurMinimale)
						minMod = 1;
					else if (rSlot->domain[d] == rSlot->valeurMaximale)
						maxMod = 1;
					
					// Remplacement de la valeur supprimée par la dernière valeur du domaine
					rSlot->maxdomain--;
					rSlot->domain[d] = rSlot->domain[rSlot->maxdomain];
					
					break; // Une seule valeur du domaine à éliminer pour une contrainte DIF
				} else {
					d++;
				}
			}
		} else if (sens == INF) {
			while (d != rSlot->maxdomain) {
				
#ifdef __OPT_TESTCOUNT
	performance.test_done ++;
#endif
				// Eliminer les valeur du domaine ayant une valeur plus grande
				if (rSlot->domain[d] <= valeurDansLaCase) {
					emiliner(emilinateur, posEmilinateur, cSlot, rSlot, rSlot->domain[d]);
					
					if (rSlot->domain[d] == rSlot->valeurMinimale)
						minMod = 1;
					else if (rSlot->domain[d] == rSlot->valeurMaximale)
						maxMod = 1;
					
					rSlot->maxdomain--;
					rSlot->domain[d] = rSlot->domain[rSlot->maxdomain];
				} else {
					d++;
				}
			}
			
		} else { // SENS_SUP
			while (d < rSlot->maxdomain) {
#ifdef __OPT_TESTCOUNT
	performance.test_done ++;
#endif
				// Eliminer les valeur du domaine ayant une valeur plus petite
				if (rSlot->domain[d] >= valeurDansLaCase) {
					emiliner(emilinateur, posEmilinateur, cSlot, rSlot, rSlot->domain[d]);

					
					if (rSlot->domain[d] == rSlot->valeurMinimale)
						minMod = 1;
					else if (rSlot->domain[d] == rSlot->valeurMaximale)
						maxMod = 1;
					
					rSlot->maxdomain--;
					rSlot->domain[d] = rSlot->domain[rSlot->maxdomain];
				} else {
					d++;
				}
			}
		}
		
		
		// Domaine vide : échec
		if (rSlot->maxdomain == 0) {
			return 1;
		}
		
		
		// En cas de modification du domaine, on empile dans la liste des déductions potentielles
		if (tailleOriginelle != rSlot->maxdomain) {
			/* Remarque : empiler plusieurs fois est plus efficace
			 * que de regarder si le slot est déjà présent          */
			
			// Déduction sur des cases inférieures
			if (minMod && rSlot->slotInfSup[0] != NULL) {
				emilieDeduit[ *posEmilieDeduit   ].victime = rSlot;
				emilieDeduit[(*posEmilieDeduit)++].sens = SENS_INF;
			}
			
			// Déduction sur des cases supérieures
			if (maxMod && rSlot->slotInfSup[4] != NULL) {
				emilieDeduit[ *posEmilieDeduit   ].victime = rSlot;
				emilieDeduit[(*posEmilieDeduit)++].sens = SENS_SUP;
			}
		}
	}

	
	return 0;
}


int forwardCheckingDeductif(CSP * jeu, int (*heuristique)(PileHeuristique *)) {
	PileHeuristique * piles;
	
	Emilienator * emilinateur;		int posEmilinateur;
	Slot * rSlot;
	
	// Structure permettant de sauvegarder les déductions à faire
	EmilieDeduit * emilieDeduit;	int posDeduction;
	
	Slot * cSlot;	
	ExplorationDeVariable * currentInst;
	
#ifdef __OPT_TIMER
	performance.debut = clock();
#endif
	
	piles = preparerExploitationHeuristique(jeu);
	if (piles == NULL) return 0;
	
	initialiser_heuristique(heuristique, jeu, piles);
	
	emilinateur = malloc(sizeof(Emilienator) * (jeu->N2 -1) * jeu->N);
	if (emilinateur == NULL) {
		perror("malloc emilinateur");
		libererPileHeuristique(piles);
		return 0;
	}

	posEmilinateur = 0;
	posDeduction = 0;

	// Fait les premières déductions et essaie de vérifier que la grille n'est pas impossible
	if (!fcd_completerCSP(jeu, &emilieDeduit, emilinateur, &posEmilinateur)) {
		free(emilinateur);
		libererPileHeuristique(piles);
		return 0;
	}

	int vientDunEchec = 0;
	
	currentInst = pileHeuriExtraireVariable(piles, heuristique(piles));

	while (currentInst != NULL) {
		cSlot = currentInst->slot;
		
		/* Vient d'échouer */
		if (vientDunEchec) {
			while (posEmilinateur > 0 && emilinateur[posEmilinateur-1].elimineur == cSlot) {
				// Dépiler les valeurs
				rSlot = emilinateur[posEmilinateur-1].victime;
				rSlot->domain[rSlot->maxdomain] = emilinateur[posEmilinateur-1].valeur_retiree;
				posEmilinateur --;
				
				// Restaure la connaissance des valeurs minimales et maximales
				if (rSlot->domain[rSlot->maxdomain] > rSlot->valeurMaximale ) {
					rSlot->valeurMaximale = rSlot->domain[rSlot->maxdomain];
				} else if (rSlot->domain[rSlot->maxdomain] < rSlot->valeurMinimale) {
					rSlot->valeurMinimale = rSlot->domain[rSlot->maxdomain];
				}
				
				rSlot->maxdomain ++;
				
			}
			
			// Instancier la suivante
			currentInst->iDom++;
		}
		
		
		if (currentInst->iDom == cSlot->maxdomain) {
			// Fin d'exploration : le neoud mène à une inconsistance
			desinstancier(cSlot);
			currentInst = retourArriere(piles);
			vientDunEchec = 1;
		} else {
			// Nouveau noeud
			instancier(cSlot, cSlot->domain[currentInst->iDom]);
			
#ifdef __OPT_NODECOUNT
			performance.nodes ++;
#endif
			posDeduction = 0;
			
			// Réduit le domaine
			vientDunEchec = fcd_reduireDomaine(emilinateur, &posEmilinateur, emilieDeduit, &posDeduction, cSlot);
			
			
			if (!vientDunEchec) {
				// Fait des déductions
				vientDunEchec = !fcd_deduction(emilinateur, &posEmilinateur, emilieDeduit, &posDeduction, cSlot);
				
				if (!vientDunEchec) {
					// Aucun domaine n'est vide : on continue
					currentInst = pileHeuriExtraireVariable(piles, heuristique(piles));
				}
			}
		}
	}
	
	free(emilieDeduit);
	free(emilinateur);
	libererPileHeuristique(piles);
	
	liberer_preparationHeuristique(heuristique);
	
#ifdef __OPT_TIMER
	performance.fin = clock();
#endif
	
	return !vientDunEchec;
}

#endif // __OPT_FCDEDUCTIF
