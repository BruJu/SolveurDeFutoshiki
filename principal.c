#ifdef __linux__
#include <signal.h>			// sigaction
#include <unistd.h>			// getpid
#endif
#include <string.h>			//strcmp

#include "principal.h"

#include "lecture.h"
#include "options.h"
#include "resolution.h"
//#include "heuristique.h"
#include "gestHeuristique.h"

#ifdef __OPT_FCDEDUCTIF
#include "resolutionDeductive.h"
#endif


extern int heuristiqueSurDegre;
Performance performance;

#ifdef __linux__
CSP * game;

void signal_recu(int sig) {
	if (sig == SIGUSR1) {
		printf("--> Sinal SIGUSR1 reçu\n");
		afficherGrilleComplete(game);
		afficherEtatGrille(game);
#ifdef __OPT_TIMER
		performance.fin = clock();
#endif
		afficher_performance(&performance);
	} else if (sig == SIGINT) {
		printf("--> Resolution interrompue, état actuel de la résolution\n");
		afficherGrilleComplete(game);
#ifdef __OPT_TIMER
		performance.fin = clock();
#endif
		afficher_performance(&performance);
		exit(0);
	}
}


int bor_signal (int sig, void (*h)(int), int options) {
    int r;
    struct sigaction s;
    s.sa_handler = h;
    sigemptyset (&s.sa_mask);
    s.sa_flags = options;
    r = sigaction (sig, &s, NULL);
    if (r < 0) perror (__func__);
    return r;
}


#endif



void lancer_analyse(CSP * jeu, MethodeDeResolution methode, Heuristique heuri) {
	int resultat = -1;
	
	init_performance(&performance);
	
	switch (methode) {
		case Backtrack:
			if (heuri.fonction == NULL) {
				printf("ALGO : Backtrack sans heuristique\n");
				resultat = backtrack(jeu);
			} else {
				printf("ALGO : Backtrack - %s\n", heuri.descr);
				resultat = backtrackHeuristique(jeu, heuri.fonction);
			}
			break;
		case ForwardChecking:
			if (heuri.fonction == NULL) {
				printf("ALGO : Forward Checking sans heuristique\n");
				resultat = forwardChecking(jeu);
			} else {
				printf("ALGO : Forward Checking - %s\n", heuri.descr);
				resultat = forwardCheckingHeuristique(jeu, heuri.fonction);
			}
			break;
#ifdef __OPT_FCDEDUCTIF
		case ForwardCheckingDeductif:
			printf("ALGO : Forward Checking Deductif - %s\n", heuri.descr);
			resultat = forwardCheckingDeductif(jeu, heuri.fonction);
			break;
#endif // __OPT_FCDEDUCTIF
		default :
			break;
	}
	
	if (resultat == -1) {
		printf("Algorithme non implémenté\n");
		return;
	}
	
	if (resultat == 1) {
		printf("--> Grille résolue");
		printf("Solution : \n");
		afficherGrilleComplete(jeu);
	} else {
		printf("Pas de solution\n");
	}
	
	afficher_performance(&performance);
}

int main (int argc, char* argv[]) {
	char * fichier;
	CSP * jeu;
	MethodeDeResolution methode; Heuristique heuri;

	// Analyse des arguments
    if (argc < 2) {
		printf("Usage : %s fichier ALGO HEURI\n", argv[0]);
		printf("ALGO : \n");
		printf(" - BT   : Backtrack \n");
		printf(" - FC   : Forward Checking \n");
		printf(" - FCD  : Forward Checking Deductif\n");
		printf("HEURI : \n");
		listeLesHeuristiques();
		printf("== About the authors ==\n");
		printf("- Date    : Printemps 2017\n");
		printf("- Auteurs : Julian BRUYAT, Meldrick FERRARO\n");
		exit(EXIT_SUCCESS);
	}
	
    fichier = argv[1];
    
    
    
    if (argc < 3) {
		methode = ForwardChecking;
		heuri.fonction = NULL;
	} else {
		if (strcmp(argv[2], "BT") == 0) {
			methode = Backtrack;
		} else if (strcmp(argv[2], "FC") == 0) {
			methode = ForwardChecking;
#ifdef __OPT_FCDEDUCTIF
		} else if (strcmp(argv[2], "FCD") == 0) {
			methode = ForwardCheckingDeductif;
			heuri.fonction = heuristiqueOrdre;
			heuri.nom[0] = 0;
			heuri.descr[0] = 0;
#endif	// __OPT_FCDEDUCTIF
		} else {
			fprintf(stderr, "Méthode de résolution non reconnue\n");
			return EXIT_FAILURE;
		}
		
		
#ifdef __OPT_FCDEDUCTIF
		if (methode != ForwardCheckingDeductif || argc > 3)
#endif	// __OPT_FCDEDUCTIF
		{
			if (argc < 4) {
				heuri.fonction = NULL;
			} else {
				heuri = trouverHeuristique(argv[3]);
				heuristiqueSurDegre = heuri.surDegre;
				
				if (heuri.fonction == NULL) {
					fprintf(stderr, "Heuristique non reconnue\n");
					return EXIT_FAILURE;
				}
				
			}
		}
	}
    
	// Drôme des collines
	printf("GRILLE OUVERTE : %s\n", fichier);

#ifdef __linux__
	printf("PID : %d\n", (int)getpid());
	printf("A la reception du signal %d, la grille en cours sera affichée\n", SIGUSR1);	
#endif
	
	// Chargement
	jeu = lireFichier(fichier);
	if (jeu == NULL) {
		fprintf(stderr, "Erreur dans la lecture de la grille\n");
		exit(-1);
	}
	
	printf("--> Grille à résoudre");
	afficherGrilleComplete(jeu);
	
	// Signaux
#ifdef __linux__
	game = jeu;
	
	bor_signal(SIGUSR1, signal_recu, SA_RESTART);
	bor_signal(SIGINT, signal_recu, 0);
#endif
	
	// Resolution
	lancer_analyse(jeu, methode, heuri);
	
	doesItWork(jeu);
	
	// Fin
	printf("== FIN DU PROGRAMME ==\n");
	
	libererCSP(jeu);
	
	return EXIT_SUCCESS;
}

