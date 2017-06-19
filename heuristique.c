#include "heuristique.h"
#include "gestHeuristique.h"
#include "affichage.h"
#include "string.h"			// strcpy



/* FULL POTENTIAL */
int optionHeuristique_groupesReels = 0;
int * nbDeMembresTruePotential;
int * groupeSurLigne;
int * groupeSurColonne;

/**
 * Renvoie le degré
 * Ne fonctionne que si heuristiqueSurDegre = 1
 */
static inline int calculerLeDegre(Slot * slot) {
	return slot->maxcontraintes - slot->debutcontraintes;
}

/**
 * Renvoie le nombre de contraintes > ou < du slot avec des slots
 * non instanciés
 */
static inline int calculerLeDegreInf(Slot * slot) {
	int ii;
	int nbDeContraintesInf = 0;

	for (ii = 0 ; slot->slotInfSup[ii] != NULL ; ii++) {
		if (slot->slotInfSup[ii]->valeur == 0) {
			nbDeContraintesInf++;
		}
	}
	
	for (ii = 4 ; slot->slotInfSup[ii] != NULL ; ii--) {
		if (slot->slotInfSup[ii]->valeur == 0) {
			nbDeContraintesInf++;
		}
	}
	
	return nbDeContraintesInf;
}




/* ============================ */
/* ============================ */
/* GESTION DES PILE HEURISTIQUE */
/* ============================ */

int caseChoisie = 0;
int debutLigne = 0;
int moduloCol = 0;




/* Schéma général poour connaître le numéro de la variable à instancier
 * - Cette fonction sera ensuite copier collé pour chaque heuristique
 * - Nous faisons un pointeur de fonction à ce niveau afin de ne pas parcourir toutes les variables
 * quand cela n'est pas nécessaire (par exemple quand on prend la première qui vient, ou quand on veut
 * instancier toute la ligne d'un coup)
 *
 * On met la valeurMinimaleTrouvee à une grande valeur afin de ne pas répéter deux fois le code
 * de l'évaluation de l'heuristique
 * 
 * Usage final : pileHeuriExtraireVariable(&pile, schemaExtraireHeuristique(&pile));
 *  */
int schemaExtraireHeuristique(PileHeuristique * h) {
	int valeurMinimaleTrouvee = 19999999;
	int indiceMinimal         = -1;
	int i;
	int valeurHeuristique;

	for (i = 0 ; i != h->maxListeRestante ; i++ ) {
		valeurHeuristique = 0;

		if (valeurHeuristique <= valeurMinimaleTrouvee) {
			valeurMinimaleTrouvee = valeurHeuristique;
			indiceMinimal = i;
		}
	}
	
	return indiceMinimal;
}



/* ============================ */
/* ============================ */
/*         HEURISTIQUE          */
/* ============================ */



/* ==============================================
	HEURISTIQUE ORDRE
============================================== */


int heuristiqueOrdre(PileHeuristique * h) {
	return h->maxListeRestante - 1;
}

void inverserVariablesRestantes(PileHeuristique * h) {
	VariableNonInstanciee * nouvelleListe = malloc(sizeof(VariableNonInstanciee) * h->maxListeRestante);
	if (nouvelleListe == NULL) {
		perror("malloc inversion");
		return;
	}
	
	int i;
	
	for (i = 0 ; i != h->maxListeRestante ; i++) {
		nouvelleListe[i].slot = h->listeRestante[h->maxListeRestante - 1 - i].slot;
	}
	
	free(h->listeRestante);
	h->listeRestante = nouvelleListe;
}



/* ==============================================
	HEURISTIQUE SUR GROUPE
============================================== */


void mettreEnPlaceLesGroupes_setGroupe(CSP * jeu, int pos, int groupe) {
	jeu->grille[pos].statut = groupe;
	
	for (int i = 0 ; i != 5 ; i++) {
		if (jeu->grille[pos].slotInfSup[i] == NULL)
			continue;
		
		if (jeu->grille[pos].id < groupe)
			continue;
			
		if (jeu->grille[pos].slotInfSup[i]->statut != -1) {
			// statut == groupe, un autre appel a donné juste avant
			// le numéro du groupe
			continue;
		}
		
		jeu->grille[pos].slotInfSup[i]->statut = groupe;
		
		mettreEnPlaceLesGroupes_setGroupe(jeu, jeu->grille[pos].slotInfSup[i]->id, groupe);
	}
}

int * mettreEnPlaceLesGroupes(CSP * jeu) {
	int * nbDeMembresDansLeGroupe;
	
	int i;
	int numSlot;
	
	nbDeMembresDansLeGroupe = malloc(sizeof(int) * jeu->N2);
	
	// Statut = numéro du groupe
	for (i = 0 ; i != jeu->N2 ; i++) {
		jeu->grille[i].statut = -1;			// Pas de groupe encore attribué
		nbDeMembresDansLeGroupe[i] = 0;		// Pas de membre dans des groupes non formés
	}
	
	for (numSlot = 0 ; numSlot != jeu->N2 ; numSlot++) {		
		if (jeu->grille[numSlot].statut != -1) {
			// Un groupe a déjà été attribué
			continue;
		}
		
		// Affectation de l'id du groupe (le numéro de la case) à la case
		// et à toutes les cases qui sont reliées par des < ou des >
		mettreEnPlaceLesGroupes_setGroupe(jeu, numSlot, numSlot);
	}
	
	// Décompte du nombre de membres dans chaque groupe
	for (numSlot = 0 ; numSlot != jeu->N2 ; numSlot++) {
		nbDeMembresDansLeGroupe[(int) jeu->grille[numSlot].statut] ++;
	}
	
	// Enlève le groupe des slots qui sont tous seuls
	for (numSlot = 0 ; numSlot != jeu->N2 ; numSlot++) {
		if (nbDeMembresDansLeGroupe[(int) jeu->grille[numSlot].statut] == 1) {
			jeu->grille[numSlot].statut = -1;
		}
	}
	
	return nbDeMembresDansLeGroupe;
}

void preparerHeuriGroupes(CSP * jeu) {
	int * nbDeMembres = mettreEnPlaceLesGroupes(jeu);
	
	// Remplacer les numeros de groupe par le nombre de membres dans le groupe
	for (int numSlot = 0 ; numSlot != jeu->N2 ; numSlot++) {
		if (jeu->grille[numSlot].statut == -1) {
			jeu->grille[numSlot].statut = 1;
		} else {
			jeu->grille[numSlot].statut = nbDeMembres[(int) jeu->grille[numSlot].statut];
		}
	}
	
	free(nbDeMembres);
}

int heuristiqueGroupe(PileHeuristique * pile) {
	int valeurMinimaleTrouvee = 19999999;
	int indiceMinimal         = -1;
	int i;
	int valeurHeuristique;
	
	int degre;

	for (i = 0 ; i != pile->maxListeRestante ; i++ ) {
		if (pile->listeRestante[i].slot->maxdomain == 1)
			return i;
		
		
		degre = calculerLeDegre(pile->listeRestante[i].slot);
		if (degre == 0) {	// BT, division par 0
			if (indiceMinimal == -1)
				indiceMinimal = i;
			continue;
		} else {
			valeurHeuristique = (0x80*pile->N*pile->listeRestante[i].slot->maxdomain /degre)
							- 0x100*pile->listeRestante[i].slot->statut;
		}
		

		if (valeurHeuristique <= valeurMinimaleTrouvee) {
			valeurMinimaleTrouvee = valeurHeuristique;
			indiceMinimal = i;
		}
	}

	return indiceMinimal;
}


/* ==============================================
	HEURISTIQUE GROUPES REELS
============================================== */

int heuristiqueGroupesReels (PileHeuristique * h) {
	int valeurMinimaleTrouvee = 19999999;
	int indiceMinimal         = -1;
	int i;
	int valeurHeuristique;

	Slot * slot;	int idSlot;		int parcours;

	for (i = 0 ; i != h->maxListeRestante ; i++ ) {
		slot = h->listeRestante[i].slot;
		
		if (slot->maxdomain == 1) {
			return i;
		} else {
			valeurHeuristique = -100 * slot->maxdomain;
		}
		
		if (h->maxInstanciee > (h->N * h->N) / 4)
			goto skipperCalculLong;
		
		idSlot = slot->id;
		
		parcours = (idSlot % h->N) * h->N;
		for ( ; groupeSurColonne[parcours] != -1 ; parcours++) {
			if (groupeSurColonne[parcours] != slot->statut)
				valeurHeuristique += nbDeMembresTruePotential[groupeSurColonne[parcours]];
		}
		
		
		parcours = (idSlot / h->N) * h->N;
		for ( ; groupeSurLigne[parcours] != -1 ; parcours++) {
			if (groupeSurLigne[parcours] != slot->statut)
				valeurHeuristique += nbDeMembresTruePotential[groupeSurLigne[parcours]];
		}
		
		skipperCalculLong:
		
		valeurHeuristique *= -1;
		
		if (valeurHeuristique <= valeurMinimaleTrouvee) {
			valeurMinimaleTrouvee = valeurHeuristique;
			indiceMinimal = i;
		}
	}

	return indiceMinimal;
}

void preparerHeuriGroupeReels(CSP * jeu) {
	nbDeMembresTruePotential = mettreEnPlaceLesGroupes(jeu);
	int numCase;
	int ligne, colonne;
	
	groupeSurLigne = malloc(sizeof(int) * jeu->N2);
	groupeSurColonne = malloc(sizeof(int) * jeu->N2);
	
	for (numCase = 0 ; numCase != jeu->N2 ; numCase ++) {
		groupeSurLigne  [numCase] = -1;
		groupeSurColonne[numCase] = -1;
	}
	
	// Mise en place de la retenue des groupes sur chaque ligne / colonne
	for (numCase = 0 ; numCase != jeu->N2 ; numCase ++) {
		// Pas de groupe
		if (jeu->grille[numCase].statut == -1)
			continue;
		
		// Calcul des coordonnées
		ligne = numCase / jeu->N;
		colonne = numCase % jeu->N;
		
		ligne   *= jeu->N;
		colonne *= jeu->N;
		
		// Inscription du groupe
		for ( ; groupeSurLigne[ligne] != -1
				&& groupeSurLigne[ligne] != jeu->grille[numCase].statut 
				; ligne++);
				
		if (groupeSurLigne[ligne] == -1)
			groupeSurLigne[ligne] = jeu->grille[numCase].statut;
		
		
		for ( ; groupeSurColonne[colonne] != -1
				&& groupeSurColonne[colonne] != jeu->grille[numCase].statut
				; colonne++);
		
		if (groupeSurColonne[colonne] == -1)
			groupeSurColonne[colonne] = jeu->grille[numCase].statut;
	}
	
	optionHeuristique_groupesReels = 1;
}




/* ==============================================
	HEURISTIQUES SANS PREPARATION
============================================== */

int heuristiqueTailleDomaine(PileHeuristique * h) {
	int valeurMinimaleTrouvee = 19999999;
	int indiceMinimal         = -1;
	int i;
	int valeurHeuristique;

	for (i = 0 ; i != h->maxListeRestante ; i++ ) {
		valeurHeuristique = h->listeRestante[i].slot->maxdomain;

		if (valeurHeuristique <= valeurMinimaleTrouvee) {
			valeurMinimaleTrouvee = valeurHeuristique;
			indiceMinimal = i;
		}
	}
	
	return indiceMinimal;
}

int heuristiqueDomDeg(PileHeuristique * h) {
	int valeurMinimaleTrouvee = 19999999;
	int indiceMinimal         = -1;
	int i;
	int valeurHeuristique;

	for (i = 0 ; i != h->maxListeRestante ; i++ ) {
		valeurHeuristique = calculerLeDegre(h->listeRestante[i].slot);
		
		if (valeurHeuristique == 0) {
			// Si il y n'a plus de contrainte, il reste de part la nature de notre problème seulement 
			// qu'une valeur dans le domaine. On décide d'instancier directement cette case afin de ne
			// plus en recalculer la valeur heuristique.
			//
			// Dans un autre problème, on attendrait d'avoir instancié toutes les autres valeurs pour ne
			// pas multiplier des branches d'arbres similaires.
			return i;
		}
		
		valeurHeuristique = (1000 * h->listeRestante[i].slot->maxdomain) / valeurHeuristique;
		
		if (valeurHeuristique <= valeurMinimaleTrouvee) {
			valeurMinimaleTrouvee = valeurHeuristique;
			indiceMinimal = i;
		}
	}

	return indiceMinimal;
}

int heuristiqueDomPlusDeg(PileHeuristique * h) {
	int valeurMinimaleTrouvee = 19999999;
	int indiceMinimal         = -1;
	int i;
	int valeurHeuristique;

	for (i = 0 ; i != h->maxListeRestante ; i++ ) {
		valeurHeuristique = calculerLeDegre(h->listeRestante[i].slot);
		valeurHeuristique += h->listeRestante[i].slot->maxdomain;
		
		if (valeurHeuristique <= valeurMinimaleTrouvee) {
			valeurMinimaleTrouvee = valeurHeuristique;
			indiceMinimal = i;
		}
	}

	return indiceMinimal;
}

int heuristiqueNbContraintesRest(PileHeuristique * h) {
	int valeurMinimaleTrouvee = 19999999;
	int indiceMinimal         = -1;
	int i;
	int valeurHeuristique;
		

	for (i = 0 ; i != h->maxListeRestante ; i++ ) {
		valeurHeuristique = calculerLeDegre(h->listeRestante[i].slot);

		if (valeurHeuristique <= valeurMinimaleTrouvee) {
			valeurMinimaleTrouvee = valeurHeuristique;
			indiceMinimal = i;
		}
	}

	return indiceMinimal;
}

int heuristiqueDomSurDeg2(PileHeuristique * h) {
	int valeurMinimaleTrouvee = 19999999;
	int indiceMinimal         = -1;
	int i;
	int valeurHeuristique;


	int nbDeContraintesTotales;
	int nbDeContraintesInf;
	int tailleDomaine;

	for (i = 0 ; i != h->maxListeRestante ; i++ ) {
		nbDeContraintesTotales = calculerLeDegre   (h->listeRestante[i].slot);
		nbDeContraintesInf     = calculerLeDegreInf(h->listeRestante[i].slot);
	
		nbDeContraintesTotales = 2*nbDeContraintesTotales - nbDeContraintesInf;
		
		if (nbDeContraintesTotales == 0) {
			return i;
		}
		
		tailleDomaine = h->listeRestante[i].slot->maxdomain;
		
		valeurHeuristique = (0x100 * tailleDomaine) / nbDeContraintesTotales;

		if (valeurHeuristique <= valeurMinimaleTrouvee) {
			valeurMinimaleTrouvee = valeurHeuristique;
			indiceMinimal = i;
		}
	}

	return indiceMinimal;
}

int heuristiqueDegreInferieur(PileHeuristique * h) {
	int valeurMinimaleTrouvee = 19999999;
	int indiceMinimal         = -1;
	int i;
	int valeurHeuristique;
	int multiplicateur;

	for (i = 0 ; i != h->maxListeRestante ; i++ ) {
		valeurHeuristique = calculerLeDegreInf(h->listeRestante[i].slot);
		
		if (valeurHeuristique == 0) {
			valeurHeuristique = calculerLeDegre(h->listeRestante[i].slot);
			multiplicateur = 10000;
		} else {
			multiplicateur = 1000;
		}
		
		valeurHeuristique = (multiplicateur * h->listeRestante[i].slot->maxdomain) / valeurHeuristique;

		if (valeurHeuristique <= valeurMinimaleTrouvee) {
			valeurMinimaleTrouvee = valeurHeuristique;
			indiceMinimal = i;
		}
	}

	return indiceMinimal;
}

int heuristiquePotentiel(PileHeuristique * h) {
	int valeurMinimaleTrouvee = 19999999;
	int indiceMinimal         = -1;
	int i;
	int valeurHeuristique;

	Slot * slot;
	int c;

	for (i = 0 ; i != h->maxListeRestante ; i++ ) {	
		slot = h->listeRestante[i].slot;
		
		valeurHeuristique = calculerLeDegre(slot);
		
		for (c = 0 ; c != 5 ; c++) {
			if (slot->slotInfSup[c] == NULL)
				continue;
			
			if (slot->slotInfSup[c]->valeur != 0)
				continue;
			
			valeurHeuristique += slot->slotInfSup[c]->maxdomain/2;
		}
		
		valeurHeuristique = - valeurHeuristique;
		
		if (valeurHeuristique <= valeurMinimaleTrouvee) {
			valeurMinimaleTrouvee = valeurHeuristique;
			indiceMinimal = i;
		}
	}

	return indiceMinimal;
}


/* ==============================================
	HEURISTIQUE VARVAL
============================================== */

int * ocurrencesChiffreParLigne;
int * occurencesChiffreParColonne;

void preparerVarVal(CSP * jeu) {
	/* Indexation : 
	 * [numLigne][valeur-1] <=> numLigne * jeu->N + valeur-1
	 * 
	 * (-1 car les tableaux commencent à 0, or nos valeurs commencent à 1)
	 */
	ocurrencesChiffreParLigne = malloc(sizeof(int) * jeu->N2);
	occurencesChiffreParColonne = malloc(sizeof(int) * jeu->N2);
}

int heuristiqueVarVal(PileHeuristique * h) {
	int i;						// Parcours divers
	int ligne;					// Ligne du slot
	int colonne;				// colonne du slot
	int d;						// Parcours du domaine de slot
	Slot * slot;				// Case du jeu en cours de traitement
	int minimumTrouveLigne;		// Plus petit nombre d'occurences sur une ligne
	int positionLigne;			// Numéro de la ligne + nombre concerné
	int minimumTrouveColonne;	// idem sur colonne
	int positionColonne;		// idem sur colonne
	int valeurUnique;	int pas;	int borne;		int actuel;
	
	if (h->maxListeRestante == 0) {
		return -1;
	}
	
	// Mise à zéro des tableaux	
	for (i = 0 ; i < h->N * h->N ; i++) {
		ocurrencesChiffreParLigne[i] = 0;
		occurencesChiffreParColonne[i] = 0;
	}
	
	// Remplissage du tableau avec le nombre d'occurences actuel
	for (i = 0 ; i < h->maxListeRestante ; i++) {
		slot = h->listeRestante[i].slot;
		
		// Si le domaine est de taille 1, on instancie directement
		if (slot->maxdomain == 1)
			return i;
		
		// Decompte des occurences de chaque valeur du domaine
		ligne = slot->id;
		colonne = slot->id;
		
		ligne = ligne / h->N;
		colonne = colonne % h->N;
		
		for (d = 0 ; d != slot->maxdomain ; d++) {
			ocurrencesChiffreParLigne[ligne*h->N + slot->domain[d]-1]++;
			occurencesChiffreParColonne[colonne*h->N + slot->domain[d]-1]++;
		}
	}
	
	// Recherche de la valeur apparaissant le moins souvent (initialisation)
	minimumTrouveLigne = h->N+1;
	minimumTrouveColonne = h->N +1;
	
	positionLigne = positionColonne = 0;	// Enlever warning
	
	// Recherche de la ligne ou colonne ayant le plus petit nombre
	// d'occurences d'un chiffre
	for (i = 0 ; i < h->N*h->N ; i++) {
		if (ocurrencesChiffreParLigne[i] != 0) {
			if (ocurrencesChiffreParLigne[i] < minimumTrouveLigne) {
				positionLigne = i;
				minimumTrouveLigne = ocurrencesChiffreParLigne[i];
				
				if (minimumTrouveLigne == 1)
					break;
			}
		}
		
		if (occurencesChiffreParColonne[i] != 0) {
			if (occurencesChiffreParColonne[i] < minimumTrouveColonne) {
				positionColonne = i;
				minimumTrouveColonne = occurencesChiffreParColonne[i];
				
				if (minimumTrouveColonne == 1) {
					break;
				}
			}
		}	
	}
	
	// si il n'y a pas de valeur unique, heuristique sur taille des domaines
	if (minimumTrouveLigne != 1 && minimumTrouveColonne != 1) {
		return heuristiqueTailleDomaine(h);
	}
	
	// Mise en place des paramètres pour la recherche de la variable
	// ayant valeurUnique dans son domaine.
	if (minimumTrouveLigne < minimumTrouveColonne) {
		valeurUnique = (positionLigne % h->N)+1;
		pas = 1;						// Sauts horizontaux
		actuel = positionLigne / h->N;	// Du début de la ligne
		actuel = actuel * h->N;
		borne = actuel + h->N;			// Jusqu'à la fin
	} else {
		valeurUnique = (positionColonne % h->N)+1;
		pas = h->N;							// Sauts verticaux
		actuel = positionColonne / h->N;	// Du haut de la colonne
		borne = h->N * h->N;				// Jusqu'à la fin de la grille
	}
	
	
	// Rechercher un slot qui a la valeur dans son domaine
	for ( ; actuel < borne ; actuel += pas) {
		slot = &(h->jeu->grille[actuel]);
		
		if (slot->valeur != 0)
			continue;
		
		for (d = 0 ; d != slot->maxdomain ; d++) {
			if (slot->domain[d] == valeurUnique)
				goto slotTrouve;
		}
	}
	
	fprintf(stderr, "Erreur dans l'heuristique varval\n");
	exit(1);
	
slotTrouve:
	
	// On met la valeur unique trouvée au début en espérant que
	// l'instanciation courante mènera à la solution
	slot->domain[d] = slot->domain[0];
	slot->domain[0] = valeurUnique;
	
	// recherche de la position du slot dans la liste des variables
	// non instanciées
	for (i = 0 ; i < h->maxListeRestante ; i++) {
		if (h->listeRestante[i].slot == slot)
			return i;
	}
	
	fprintf(stderr, "Erreur dans l'heuristique varval (2)\n");
	exit(1);
	
}

void libererVarVal() {
	free(occurencesChiffreParColonne);
	free(ocurrencesChiffreParLigne);
}



