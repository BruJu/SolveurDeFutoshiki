#include "lecture.h"

int nouvelleCase(CSP* jeu, int numeroCase, int nombre) {
	if (numeroCase < 0 || numeroCase >= jeu->N2) {
		return 0;
	}
	
	int i;
	
	Slot * slot = &(jeu->grille[numeroCase]);
	
	slot->valeur = 0;
	slot->debutcontraintes = 0;
	slot->nbDinferieur = 0;
	
	if (nombre == 0) {
		for (i = 0 ; i < jeu->N ; i++) {
			slot->domain[i] = i + 1;
		}
		slot->domain[jeu->N] = -1;
		slot->maxdomain = jeu->N;
	} else {
		slot->domain[0] = nombre;
		slot->domain[1] = -1;
		slot->maxdomain = 1;
	}
	
	return 1;
}

void addContrainteALaCase(Contrainte * contraint, Slot* slot) {
	slot->contraintes[slot->maxcontraintes] = contraint;
	slot->maxcontraintes = slot->maxcontraintes + 1;
	
	if (contraint->type == INF) {
		slot->nbDinferieur++;
	}
}

void majContrainte(Contrainte *contraint) {
	addContrainteALaCase(contraint, contraint->caseA);
	addContrainteALaCase(contraint, contraint->caseB);
	
#ifdef __OPT_FCDEDUCTIF
	if (contraint->type == INF) {
		contraint->caseA->slotInfSup[contraint->caseA->valeurMinimale++] = contraint->caseB;
		contraint->caseB->slotInfSup[contraint->caseB->valeurMaximale--] = contraint->caseA;
	}
#endif
	
}

int nouvelleContrainte(CSP* jeu, int numeroCase, int numeroCase2, char symbol) {
	Contrainte* newConstraint = &(jeu->contraintes[jeu->nb_contraintes]);
	
	if (symbol == '<' || symbol == '^') {
		newConstraint->type  = INF;
		newConstraint->caseA = &(jeu->grille[numeroCase]);
		newConstraint->caseB = &(jeu->grille[numeroCase2]);
	} else if (symbol == '>' || symbol == 'v') {
		newConstraint->type  = INF;
		newConstraint->caseB = &(jeu->grille[numeroCase]);
		newConstraint->caseA = &(jeu->grille[numeroCase2]);
	} else if (symbol == '.' || symbol == ' ') {
		newConstraint->type  = DIF;
		newConstraint->caseA = &(jeu->grille[numeroCase]);
		newConstraint->caseB = &(jeu->grille[numeroCase2]);
	} else {
		return 0;
	}	
	
	majContrainte(newConstraint);
	
	jeu->nb_contraintes ++;
	
	return 1;
}


void remplirUneAutreContrainte(CSP *jeu, int idslot) {
	int finLigne;
	int a;
	
	finLigne = idslot - (idslot % jeu->N) + jeu->N;
	
	// finir ligne
	for (a = idslot + 2 ; a < finLigne ; a++) {
		nouvelleContrainte(jeu, idslot, a, ' ');
	}
	
	// finir colonne
	for (a = idslot + 2 *jeu->N ; a < jeu->N2 ; a += jeu->N) {
		nouvelleContrainte(jeu, idslot, a, ' ');
	}
}

void remplirLesAutresContraintes(CSP *jeu) {
	for (int i = 0 ; i < jeu->N2 ; i++) {
		remplirUneAutreContrainte(jeu, i);
	}
}


int lireNombreSymbole(CSP* jeu, FILE* file, int * numCase) {
	int val_lue;
	char symbole_lu;
	
	if (fscanf(file, "%d%c", &val_lue, &symbole_lu) == 2) {
		nouvelleCase(jeu, *numCase, val_lue);
		(*numCase)++;
		nouvelleContrainte(jeu, (*numCase) - 1, *numCase, symbole_lu);
		
		return 1;
	} else {
		return 0;
	}
}

int lireLignePaire(CSP * jeu, FILE * file, int numCase) {
	int col;
	
	for (col = 0 ; col < (jeu->N-1) ; col++) {
		if (!lireNombreSymbole(jeu, file, &numCase)) {
			return 0;
		}
	}
	
	return 1;
}

CSP * remplirCSPF(FILE* file) {
	CSP * jeu = NULL;
	
	// Lecture du nombre de cases
	int N;
	
	if (fscanf(file, "%d", &N) != 1) {
		fprintf(stderr, "Format de fichier invalide");
		return NULL;
	}
	
	if (N <= 0) {
		fprintf(stderr, "Nombre de cases invalide\n");
		return NULL;
	}
	
	// Création de la structure
	jeu = newCSP(N);
	
	if (jeu == NULL)	return NULL;
	
	// Lecture des différentes cases
	int ligne = 0;		// Nombre de lignes lues
	
	int numCase = 0;	// id de la case suivant la dernière lue
	
	int val_lue;		// Valeure lue pour la dernière case de la ligne
	
	char symbole_lu;	// Symbole lu pour la lecture de contraintes
	int col;			// Colonnes correspondant à la contrainte lue.
	
	while (1) {		/* On break si la ligne de valeurs lue est la dernière avant de lire une ligne de contrainte */
		
		// Ligne de valeurs
		lireLignePaire(jeu, file, numCase);
		
		// Lecture de la dernière valeur de la ligne
		if (fscanf(file, "%d\n", &val_lue) == 1) {
			nouvelleCase(jeu, numCase - 1 + jeu->N, val_lue);
		} else {
			goto erreurLiberer;
		}
		
		// ne pas lire les contraintes de la dernière ligne
		if (ligne == (jeu->N - 1))
			break;
		
		// ligne de contraintes
		for (col = 0 ; col != (jeu->N) ; col++) {
			if (fscanf(file, "%c", &symbole_lu) == 1) {
				nouvelleContrainte(jeu, numCase, numCase + jeu->N, symbole_lu);
				
				numCase++;
			} else {
				goto erreurLiberer;
			}
		}
		ligne ++;
	}
	
	remplirLesAutresContraintes(jeu);
	
	return jeu;
	
erreurLiberer:
	libererCSP(jeu);
	return NULL;
}

CSP* lireFichier(char *fichier) {
	CSP  * jeu;
	FILE * file = fopen(fichier, "r");
	
	if (file == NULL) {
		perror("Ouverture du fichier");
		return NULL;	
	}
	
	jeu = remplirCSPF(file);
	
	fclose(file);
	return jeu;
}

