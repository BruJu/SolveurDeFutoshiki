#ifndef __H_DEFINITIONS__
#define __H_DEFINITIONS__

#include <stdlib.h> 
#include <stdio.h>
#include <time.h>
#include "options.h"


/* ============================================
==== DEFINITIONS DE TYPE
============================================ */

/**
 * Un domaine est une liste de valeur
 */
typedef int * Domain;

/**
 * Deux types de contraintes : inférieur et différent
 * Supérieur est équivalent à inférieur
 */
typedef enum {
	DIF, INF
#ifdef __OPT_FCDEDUCTIF
	, SUP		// Utilisé pour optimiser la réduction de valeurs
#endif
} TypeContrainte;

struct Slot;

/**
 * Définition d'une contrainte (le fait d'être sur la même ligne ou colonne)
 */
typedef struct {
	TypeContrainte type;
	struct Slot*   caseA;
	struct Slot*   caseB;
} Contrainte;

/**
 * Définition d'une case dans un jeu de Futoshiki (nos variables)
 */
typedef struct Slot {
	short id;					// Numéro de la case
	int valeur;					// Valeur instanciée
	Domain domain;				// Domaine de valeurs
	Contrainte** contraintes;	// Liste des contraintes affectant la case
	int maxcontraintes;			// Nombre de contraintes
	int maxdomain;				// Taille du domaine
	short debutcontraintes;		// Premiere contrainte avec une variable non instanciée (si l'heuristique est sur le degré)
	short nbDinferieur;			// Nombre de contraintes "inférieur"
	char statut;				// Usage dépendant de l'heuristique
#ifdef __OPT_FCDEDUCTIF
	struct Slot * slotInfSup[5];	// Pointeur vers les slots avec des contraintes < et > en commun
	int valeurMinimale;				// Lors de la création de la structure : représente le pointeur sur la fin de la liste slotInfSup 
	int valeurMaximale;				// Lors de Forward Checking Deductif, sauvegarde les valeurs minimales et maximales du domaine
#endif
} Slot;


/**
 * Définition de la représentation d'un jeu de Futoshiki avec CSP
 */
typedef struct {
	Slot*		grille;			// Liste des cases
	Contrainte*	contraintes;	// Liste des contraintes
	int			N;				// Nombre de ligne / colonnes
	
	int			N2;				// Nombre de cases
	int			nb_contraintes;	// Nombre de contraintes
} CSP;


/**
 * Structure pour la retenue des valeurs supprimées lors de Forward Checking
 */
typedef struct {
	Slot *	elimineur;		// Variable instanciée
	Slot *	victime;		// Variable dont le domaine est réduit
	int		valeur_retiree;	// Valeur supprimée du domaine
} Emilienator;


/**
 * Pour les déductions, savoir si on doit réduire par rapport à la borne
 * maximale ou minimale
 * 
 * PERE / FRERE permet de gérer la recurrence pour l'analsye preliminaire
 * de la grille afin de voir si elle est resoluble
 */
typedef enum {
	SENS_INF, SENS_SUP, SENS_DIF,
	PERE, FRERE
} Sens;

/**
 * Deduction de réduction de domaines depuis une case qui a été victime
 * de réduction de domaine et dans un sens
 */
typedef struct {
	Slot *	victime;
	Sens	sens;
} EmilieDeduit;

/**
 * Algorithmes disponibles pour résoudre la grille
 */
typedef enum {
	Backtrack,
	ForwardChecking,
#ifdef __OPT_FCDEDUCTIF
	ForwardCheckingDeductif
#endif
} MethodeDeResolution;



/* ============================================
==== FONCTIONS
============================================ */


/**
 * Construit un slot pouvant accueillir N valeurs (requiert un appel à
 * libererSlot à la fin)
 * Renvoie vrai si réussite, faux si échec
 */
int construireSlot(Slot* slot, int N);

/**
 *  Libère la mémoire allouée à un slot
 */
void libererSlot(Slot *slot);

/**
 *  Libère la mémoire allouée au jeu
 */
void libererCSP(CSP* jeu);

/**
 * Alloue la mémoire pour accueillir une instance CSP représentant le fukushiki de taille N
 */
CSP * newCSP(int N);


/* ============================================
==== MESURE DE PERFORMANCES
============================================ */

/**
 * Mesure des performances de chaque algorithme.
 */
typedef struct {
	clock_t debut;
	clock_t fin;
	long double temps;				// Temps d'exécution
	
	unsigned long long nodes;		// Nombre de noeuds explorés
	unsigned long long test_done;	// Nombre de contraintes testées
	
	unsigned long long deduction;	// Nombre de tests de contrainte < et > pour les déductions
} Performance;

/**
 * Initialise la structure performance à 0
 */
void init_performance(Performance * perf);


/* ============================================
==== HEURISTIQUES
============================================ */

/**
 * Variable instanciée : on retient la position courante dans le domaine
 */
typedef struct {
	Slot * slot;
	int    iDom;
} ExplorationDeVariable;

/**
 * Variable qui n'a pas encore été instanciée
 */
typedef struct {
	Slot * slot;
} VariableNonInstanciee;

/**
 * Gère les variables instanciées et les variables non instanciées
 */
typedef struct {
	ExplorationDeVariable *	instanciee;			// Pile de variables instanciees
	VariableNonInstanciee *	listeRestante;		// Liste de variables non instanciees
	
	int						maxInstanciee;
	int						maxListeRestante;
	
	int						N;
	CSP *					jeu;
} PileHeuristique;

/**
 * Structure permettant de manipuler les heuristiques plus facilement
 * pour l'affichage et le choix
 */
typedef struct Heuristique {
	char nom[100];						// Nom
	char descr[100];					// Description
	int surDegre;						// Vrai si l'heuristique provoque la séparation de contraintes sur des variables instanciées et non instanciées
	int (*fonction)(PileHeuristique *);	// Fonction permettant de choisir une variable dans la liste des variables non instanciées
} Heuristique;


#endif
