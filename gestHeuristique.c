#include "gestHeuristique.h"

#include <string.h>			//strcpy

int heuristiqueSurDegre = 0;
int etageValide = 0;
extern int optionHeuristique_groupesReels;
extern int * nbDeMembresTruePotential;
extern int * groupeSurLigne;
extern int * groupeSurColonne;


/* ==============================================
	CHOIX DES HEURISTIQUE
============================================== */

Heuristique * avoirLesHeuristiques() {
	int i = 0;
	
	Heuristique * h = malloc(sizeof(Heuristique) * 100);
	
	remplirHeuristique(&(h[i++]), "Ordre"   , 0, heuristiqueOrdre
								, "Dans l'ordre des cases");
							   
	remplirHeuristique(&(h[i++]), "Domaine" , 0, heuristiqueTailleDomaine
								, "Selon la taille du domaine");
							   
	remplirHeuristique(&(h[i++]), "Degre"   , 1, heuristiqueNbContraintesRest
								, "Selon le nombre de contraintes avec des cases non instanciÃ©es");
								
	remplirHeuristique(&(h[i++]), "Dom+Deg" , 1, heuristiqueDomPlusDeg
								, "Prend la plus petite somme Domaine + Degré");
								
	remplirHeuristique(&(h[i++]), "Dom/Deg" , 1, heuristiqueDomDeg
								, "Domaine / Degré");
								
	remplirHeuristique(&(h[i++]), "Dom/Deg2", 1, heuristiqueDomSurDeg2
								, "Degré en valorisant les contraintes infÃ©rieur");
	
								
	remplirHeuristique(&(h[i++]), "Groupe", 1, heuristiqueGroupe
								, "Instancie les cases qui sont le plus liées à d'autres cases par des <");
	
	remplirHeuristique(&(h[i++]), "Potentiel", 1, heuristiquePotentiel
								, "Valorise les cases pouvant le plus supprimer dans des domaines");
	
	remplirHeuristique(&(h[i++]), "GroupesReels", 1, heuristiqueGroupesReels
								, "Instancie les cases ayant le plus de cases atteignables via des groupes sur la ligne / colonne");
								
	remplirHeuristique(&(h[i++]), "ValVar", 0, heuristiqueVarVal, "Instancie les cases ayant des domaines avec des valeurs uniques");
	

	h[i].fonction = NULL;
	
	return h;
}

Heuristique trouverHeuristique(char * chaine) {
	Heuristique * listeHeuri = avoirLesHeuristiques();
	Heuristique h;
	int i;
	
	for (i = 0 ; listeHeuri[i].fonction != NULL ; i++) {
		if (!strcmp(chaine, listeHeuri[i].nom)) {
			break;
		}
	}
	
	h = listeHeuri[i];
	free(listeHeuri);
	return h;
}

void remplirHeuristique(Heuristique * h, char * nom, int surDegre, 
						int (*fonction)(PileHeuristique *), char * description) {
	strcpy(h->nom  , nom);
	strcpy(h->descr, description);
	h->surDegre    = surDegre;
	h->fonction    = fonction;
}

void listeLesHeuristiques() {
	int i;
	Heuristique * listeHeuri = avoirLesHeuristiques();
	
	for (i = 0 ; listeHeuri[i].fonction != NULL ; i++) {
		printf("- %s : %s\n", listeHeuri[i].nom, listeHeuri[i].descr);
	}
	
	free(listeHeuri);
}


/* ==============================================
	PREPARATION
============================================== */

void initialiser_heuristique(int (*fonction)(PileHeuristique *), CSP * jeu, PileHeuristique * pile) {
	for (int i = 0 ; i != jeu->N2 ; i++) {
		jeu->grille[i].statut = 0;
	}
	
	if (fonction == heuristiqueOrdre) {
		// Pour que l'heuristique dans l'ordre et sans heuristique
		// aient les memes résultats, il faut instancier en premier
		// les plus petits id, ie qu'ils soient en haut de la pile.
		// Or actuellement ils sont en bas.
		inverserVariablesRestantes(pile);
	} else if (fonction == heuristiqueGroupe) {
		preparerHeuriGroupes(jeu);
	} else if (fonction == heuristiqueGroupesReels) {
		preparerHeuriGroupeReels(jeu);
	} else if (fonction == heuristiqueVarVal) {
		preparerVarVal(jeu);
		pile->jeu = jeu;
	}
	
}

void liberer_preparationHeuristique(int (*fonction)(PileHeuristique *)) {
	if (fonction == heuristiqueGroupesReels) {
		free(nbDeMembresTruePotential);
		free(groupeSurLigne);
		free(groupeSurColonne);
	} else if (fonction == heuristiqueVarVal) {
		libererVarVal();
	}
}



/* ==============================================
	UTILISATION DES HEURISTIQUES
============================================== */

/* ========= ALLOCATION MEMOIRES */

PileHeuristique * preparerExploitationHeuristique(CSP * jeu) {
	int i;
	
	PileHeuristique * pile = malloc(sizeof(PileHeuristique));
	if (pile == NULL) {
		perror("malloc PileHeuristique");
		return NULL;
	}
	
	pile->instanciee = malloc(sizeof(ExplorationDeVariable) * jeu->N2);
	if (pile->instanciee == NULL) {
		perror("malloc pile instanciee");
		free(pile);
		return NULL;
	}
	
	pile->listeRestante = malloc(sizeof(VariableNonInstanciee) * jeu->N2);
	if (pile->listeRestante == NULL) {
		perror("pile->lireRestante");
		free(pile->instanciee);
		free(pile);
		return NULL;
	}
	
	pile->maxInstanciee = 0;
	pile->maxListeRestante = jeu->N2;
	for (i = 0 ; i < jeu->N2 ; i++) {
		pile->listeRestante[i].slot = &(jeu->grille[i]);
	}
	
	pile->N = jeu->N;
	
	return pile;
}

void libererPileHeuristique(PileHeuristique * h) {
	free(h->listeRestante);
	free(h->instanciee);
	free(h);
}


/* ========= UTILISATION */

void decrementerLesContraintesCommunes(Slot * cSlot) {
	int iCont;
	Slot * rSlot;
	
	for (iCont = cSlot->debutcontraintes ; iCont != cSlot->maxcontraintes ; iCont ++) {
		if (cSlot == cSlot->contraintes[iCont]->caseA) {
			rSlot = cSlot->contraintes[iCont]->caseB;
		} else {
			rSlot = cSlot->contraintes[iCont]->caseA;
		}
		
		rSlot->debutcontraintes --;
	}
}

ExplorationDeVariable * pileHeuriExtraireVariable(PileHeuristique * h, int numero) {
	if (numero == -1)
		return NULL;
		
		
	int i;			// Parcours des contraintes de slot
	int j;			// Parcours des contraintes de rSlot
	Slot * slot;	// Slot empilé
	Slot * rSlot;	// Autre slot que le slot empilé sur la contrainte i
	int option;		// contrainte sur degre - Si option = 1, il y a déjà en mémoire le slot à empiler

	ExplorationDeVariable * variableInstanciee = &(h->instanciee[h->maxInstanciee++]);

	if (heuristiqueSurDegre) {
		// En cas d'heuristique sur le degré, nous devons faire deux choses :
		// 1/ Empiler
		// 2/ Pour tous les slots qui ont une contrainte en commun avec le slot empilé,
		//   mettre de côté la contrainte avec le slot empilé (juste avant debutcontraintes)
		//    Pour des raisons de performances, si on instancie plusieurs fois des slots dans
		//   le meme ordre, on ne fait pas le travail de recherche de la contrainte à mettre de côté
		option = 0;
		
		option = etageValide >= h->maxInstanciee
				&& variableInstanciee->slot == h->listeRestante[numero].slot;
				
		if (option) { // Instanciation dans le même ordre
			slot = variableInstanciee->slot;
			
			for (i = slot->debutcontraintes ; i != slot->maxcontraintes ; i++) {
				Contrainte * contrainte = slot->contraintes[i];
				
				// Mettre de côté la première contrainte
				if (slot == contrainte->caseA) {
					contrainte->caseB->debutcontraintes++;
				} else {
					contrainte->caseA->debutcontraintes++;
				}
			}
		} else { // Ordre différent
			/* ====== METTRE EN TETE LES CONTRAINTES COMMUNES ====== */
			variableInstanciee->slot = h->listeRestante[numero].slot; // Empiler
			
			slot = variableInstanciee->slot;
			
			// Parcours de chaque contrainte
			for (i = slot->debutcontraintes ; i != slot->maxcontraintes ; i++) {
				Contrainte * contrainte = slot->contraintes[i];
				// Récupération de l'autre slot
				if (slot == contrainte->caseA) {
					rSlot = contrainte->caseB;
				} else {
					rSlot = contrainte->caseA;
				}
				
				// Recherche de la contrainte en commun
				for (j = rSlot->debutcontraintes ; rSlot->contraintes[j] != contrainte ; j++);
				
				// Echange pour mettre de côté la contrainte du point de vue de rSlot
				rSlot->contraintes[j] = rSlot->contraintes[rSlot->debutcontraintes];
				rSlot->contraintes[rSlot->debutcontraintes++] = contrainte;
			}
		}
	} else { // heuristiqueSurDegre = 0 : On se contente d'empiler le slot instancié
		variableInstanciee->slot = h->listeRestante[numero].slot;
	}

	if (optionHeuristique_groupesReels) {
		if (variableInstanciee->slot->statut != -1) {
			nbDeMembresTruePotential[(int) variableInstanciee->slot->statut] --;
		}
	}

	// Depiler des variables à instancier
	if (numero != h->maxListeRestante-1) {
		h->listeRestante[numero].slot = h->listeRestante[h->maxListeRestante-1].slot;
	}

	h->maxListeRestante --;


	// Mettre à 0 l'exploration du domaine
	variableInstanciee->iDom = 0;

	return variableInstanciee;
}

ExplorationDeVariable * retourArriere(PileHeuristique * h) {
	// Retour arriere avec la premiere variable instanciee = pas de solution
	if (h->maxInstanciee <= 1) {
		return NULL;
	}
	
	h->maxInstanciee--;
	
	// remettre la variable precedement instancieee dans la liste des variables non instanciees
	h->listeRestante[h->maxListeRestante].slot = h->instanciee[h->maxInstanciee].slot;

	if (heuristiqueSurDegre)
		decrementerLesContraintesCommunes(h->listeRestante[h->maxListeRestante].slot);
	
	if (optionHeuristique_groupesReels) {
		if (h->listeRestante[h->maxListeRestante].slot->statut != -1) {
			nbDeMembresTruePotential[(int) h->listeRestante[h->maxListeRestante].slot->statut] ++;
		}
	}
	
	
	h->maxListeRestante++;
	
	return &(h->instanciee[h->maxInstanciee-1]);
}


/* ========= AFFICHAGE */

void printPile(PileHeuristique * h) {
	for (int i = 0 ; i < h->maxInstanciee ; i++) {
		printf(" : %d prend la valeur %d\n", h->instanciee[i].slot->id, h->instanciee[i].slot->valeur);
	}
}
