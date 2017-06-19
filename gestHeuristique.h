#ifndef __H_GESTHEURISTIQUE__
#define __H_GESTHEURISTIQUE__

#include "definitions.h"
#include "heuristique.h"


/* ==============================================
	CHOIX DES HEURISTIQUE
============================================== */

/**
 * Renvoie la liste des heuristiques.
 * 
 * Il faut free la liste une fois utilisée.
 */
Heuristique * avoirLesHeuristiques();


/**
 * Renvoie la structure donnant les informations sur l'heuristique dont
 * le nom est donné en paramètre
 */

Heuristique trouverHeuristique(char * chaine);


/**
 * Rempli l'heuristique h afin de lui mettre son nom, sa description,
 * lui attribuer la fonction associée et si elle travaille sur
 * l'organisation des contraintes
 */
void remplirHeuristique(Heuristique * h, char * nom, int surDegre, 
						int (*fonction)(PileHeuristique *), char * description);


/**
 * Affiche la liste des heuristiques dans la console
 */
void listeLesHeuristiques();



/* ==============================================
	PREPARATION
============================================== */

/**
 * Fait des appels aux fonctions qui permettent de préparer la mémoire
 * à l'utilisation d'une heuristique
 */
void initialiser_heuristique(int (*fonction)(PileHeuristique *), CSP * jeu, PileHeuristique * pile);

/**
 * Permet de libérer la mémoire allouée par l'utilisation de certaines
 * fonctions heuristiques
 */
void liberer_preparationHeuristique(int (*heuristique)(PileHeuristique *));


/* ==============================================
	UTILISATION DES HEURISTIQUES
============================================== */

/* ========= ALLOCATION MEMOIRES */

/**
 * Alloue la structure de pile + liste pour la gestion des variables
 * instanciées
 */
PileHeuristique * preparerExploitationHeuristique(CSP * jeu);

/**
 * Libère la mémoire utilisée par la structure
 */
void libererPileHeuristique(PileHeuristique * h);


/* ========= UTILISATION */

/**
 * Dans la structure slot, pour les contraintes utilisant le degre, la
 * liste des contraintes est séparée en deux :
 *  Entre 0 et debutcontraintes, on a les contraintes avec des slots
 * instanciés.
 *  Entre debutcontraintes+1 et maxcontraintes, on a les contraintes avec
 * des slots non instanciés.
 * 
 *  On appelle cette fonction qunad on désintancie cSlot.
 *  Son but est de décrémenter les debutcontraintes des slots non
 * instanciés ayant une contrainte en commun avec cSlot.
 */
void decrementerLesContraintesCommunes(Slot * cSlot);

/**
 *  Extrait le slot ayant le numéro passé en paramètre dans la liste des
 * variables non instanciées.
 *  Le slot est enlevé de la liste des variables non instanciées et
 * empilé en haut de la pile de variables instanciées
 */
ExplorationDeVariable * pileHeuriExtraireVariable(PileHeuristique *h, int numero);

/**
 * Desempile la dernière variable instanciée et renvoie l'avant dernière
 */
ExplorationDeVariable * retourArriere(PileHeuristique * h);

/* ========= AFFICHAGE */

/**
 * Affiche la liste des variables instanciées dans l'ordre d'instanciation
 */
void printPile(PileHeuristique * h);

#endif
