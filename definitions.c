#include "definitions.h"


int construireSlot(Slot* slot, int N) {
	slot->domain = (Domain) malloc(sizeof(int) * (N + 1));
	if (slot->domain == NULL) {
		free(slot);
		return 0;
	}

	slot->contraintes = (Contrainte**) malloc(sizeof(Contrainte*) * N * 2 + 1);
	if (slot->contraintes == NULL) {
		free(slot->domain);
		free(slot);
		return 0;
	}
	
	slot->maxcontraintes = 0;
	slot->maxdomain = 0;
	slot->debutcontraintes = 0;

#ifdef __OPT_FCDEDUCTIF
	for (int i = 0 ; i != 5 ; i++)
		slot->slotInfSup[i] = NULL;
	
	slot->valeurMinimale = 0;
	slot->valeurMaximale = 4;
#endif
	
	return 1;
}

void libererSlot(Slot *slot) {
	free(slot->contraintes);
	free(slot->domain);
}

void libererCSP(CSP* jeu) {
	int jeuN2 = jeu->N * jeu->N;
	
	for (int i = 0 ; i < jeuN2 ; i++) {
		libererSlot(&(jeu->grille[i]));
	}
	
	free(jeu->grille);
	free(jeu->contraintes);
	free(jeu);
}

CSP * newCSP(int N) {
	int maxContrainte = N*N*N - N*N + 1;
	int maxCase       = N * N;
	int i;
	CSP * jeu;
	
	jeu = (CSP*) malloc(sizeof(CSP));
	if (jeu == NULL) {
		return NULL;
	}
	
	jeu->N = N;
	jeu->N2 = N * N;
	jeu->nb_contraintes = 0;
	
	jeu->grille = (Slot*) malloc(maxCase * sizeof(Slot));
	if (jeu->grille == NULL) {
		perror("malloc jeu->grille");		goto erreurLibererJeu;
	}
	
	jeu->contraintes = (Contrainte*) malloc(maxContrainte * sizeof(Contrainte));
	if (jeu->contraintes == NULL) {
		perror("malloc jeu->contraintes");	goto erreurLibererGrille;
	}
	
	for (i = 0 ; i < maxCase ; i++) {
		if (!construireSlot(&(jeu->grille[i]), N)) {
			goto erreurLibererCases;
		}
		
		jeu->grille[i].id = i;
	}
	
	return jeu;

erreurLibererCases:
	for (i = i-1 ; i >= 0 ; i--) {
		libererSlot(&(jeu->grille[i]));
	}
	
	free(jeu->contraintes);
erreurLibererGrille:
	free(jeu->grille);
erreurLibererJeu:
	free(jeu);
	
	return NULL;
}



/* ============================================
==== MESURE DE PERFORMANCES
============================================ */

void init_performance(Performance * perf) {
	perf->nodes     = 0;
	perf->test_done = 0;
	perf->deduction = 0;
}





