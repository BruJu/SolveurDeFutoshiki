#include "affichage.h"
#include "options.h"
#include <time.h>


void afficherContrainte(Contrainte * contrainte, int actuel) {
	if (contrainte->caseA->id == actuel) {
		if (contrainte->type == DIF) {
			printf("<>%d ", contrainte->caseB->id);
		} else {
			printf(">%d ", contrainte->caseB->id);
		}
	} else {
		if (contrainte->type == DIF) {
			printf("<>%d ", contrainte->caseA->id);
		} else {
			printf("<%d ", contrainte->caseA->id);
		}
	}
}

void afficherCase(Slot * slot) {
	afficherCaseF(slot, "- Slot no ");
}

void afficherCaseF(Slot * slot, char * preface) {
	int i;
	
	printf("%s %d [val= %d] dom€", preface, slot->id, slot->valeur);
	
	for (i = 0 ; i < slot->maxdomain ; i++) {
		printf("%d ", slot->domain[i]);
	}
	
	printf(" cont");
	
	for (i = 0 ; i < slot->maxcontraintes ; i++) {
		if (i == slot->debutcontraintes)
			printf("/");
		
		afficherContrainte(slot->contraintes[i], slot->id);
	}
	
#ifdef __OPT_FCDEDUCTIF
	printf("\n\t");
	
	printf("min=%d max=%d ", slot->valeurMinimale, slot->valeurMaximale);

	for (i = 0 ; slot->slotInfSup[i]!=NULL ; i++) {
		printf("-%d ", slot->slotInfSup[i]->id);	
	}

	for (i = 4 ; slot->slotInfSup[i]!=NULL ; i--) {
		printf("+%d ", slot->slotInfSup[i]->id);	
	}
	
#endif
	
	printf("\n");
}


void afficherGrille(CSP * jeu) {
	int i = 0;
	
	int N2 = jeu->N * jeu->N;
	
	for (i = 0 ; i != N2 ; i ++) {
		afficherCase(&(jeu->grille[i]));
	}
}


/* ========================
 * 
 * AFFICHER GRILLE COMPLETE
 * 
 * ======================== */


void printContrainte(Slot * slot, int caseB, int vertical) {
	int i = 0;
	
	while (!(slot->contraintes[i]->caseA->id == caseB || slot->contraintes[i]->caseB->id == caseB)) {
		i++;
	}
	
	Contrainte * contrainte = slot->contraintes[i];
	
	if (vertical) {
		if (contrainte->type == DIF) {
			printf(".");
		} else if (contrainte->caseA->id < contrainte->caseB->id) {
			printf("^");
		} else {
			printf("v");
		}
	} else {
		if (contrainte->type == DIF) {
			printf(" ");
		} else if (contrainte->caseA->id < contrainte->caseB->id) {
			printf("<");
		} else {
			printf(">");
		}
	}
}

void afficherGrilleComplete(CSP * jeu) {
	// Affichage de N
	printf("%d\n", jeu->N);
	
	int i, j, offset;
	
	// Affichage de la grille
	for (i = 0 ; /*i < jeu->N */ 1 ; i++) {
		offset = i * jeu->N;
		
		// ligne de valeurs
		for (j = 0 ; /*j < jeu->N */ 1 ; j++) {
			printf("%d", jeu->grille[offset + j].valeur);
			
			if (j == jeu->N -1) /* Dernière colonne (pas de contrainte sur sa droite) */
				break;
			
			printContrainte(&(jeu->grille[offset + j]), offset + j + 1, 0);
		}
		
		printf("\n");
		
		if (i == jeu->N - 1) /* Dernière ligne (pas de contrainte en dessous) */
			break;
		
		// ligne d'indices
		for (j = 0 ; j < jeu->N ; j++) {
			printContrainte(&(jeu->grille[offset + j]), offset + j + jeu->N, 1);
			printf(" ");
		}
		printf("\n");
		
	}
}

void afficher_performance(Performance * perf) {
#ifdef __OPT_TIMER
	perf->temps = (double) (perf->fin - perf->debut) / CLOCKS_PER_SEC;
	printf("Temps  : %Lf s\n", perf->temps);
#endif

#ifdef __OPT_NODECOUNT
	printf("Noeuds : %llu\n", perf->nodes);
#endif

#ifdef __OPT_TESTCOUNT
	printf("Tests  : %llu\n", perf->test_done);
#endif

#ifdef __OPT_COMPTERDEDUCTION
	if(perf->deduction != 0) {
		printf("Test pour déduction  : %llu\n", perf->deduction);
	}
#endif
}

void afficherEtatGrille(CSP * jeu) {
	for (int i = 0 ; i < jeu->N2 ; i++) {
		afficherCaseF(&(jeu->grille[i]), "");
	}
}

int estValide(CSP * jeu) {
	int ligne;
	int col;
	int k;
	int val;
	
	int * cases;
	
	cases = malloc(sizeof(int) * jeu->N);
	if (cases == NULL) return 0;
	
	
	// Validité des lignes
	for (ligne = 0 ; ligne < jeu->N ; ligne++) {
		for (k = 0 ; k < jeu->N ; k++) {
			cases[k] = -1;
		}
		
		for (col = 0 ; col < jeu->N ; col++) {
			val = jeu->grille[ligne + col * jeu->N].valeur;
			
			for (k = 0 ; cases[k] != -1 ; k++) {
				if (cases[k] == val) {
					free(cases);
					return 0;
				}
			}
			
			cases[k] = val;
		}
	}
	
	// Validité des colonnes
	for (col = 0 ; col < jeu->N ; col++) {
		for (k = 0 ; k < jeu->N ; k++) {
			cases[k] = -1;
		}
		
		for (ligne = 0 ; ligne < jeu->N ; ligne++) {
			val = jeu->grille[ligne + col * jeu->N].valeur;
			
			for (k = 0 ; cases[k] != -1 ; k++) {
				if (cases[k] == val) {
					free(cases);
					return 0;
				}
			}
			
			cases[k] = val;
		}
	}
	
	free(cases);
	return 1;
}

void doesItWork(CSP * jeu) {
	if (estValide(jeu)) {
		printf("La grille a des nombres différents\n");
	} else {
		printf("Erreur : la grille est incorrecte.");
	}
}
