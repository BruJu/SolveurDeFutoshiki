#ifndef __H_LECTURE__
#define __H_LECTURE__

#include "definitions.h"

/**
 * Initialise le domaine de la case numeroCase dans le CSP.
 * Si nombre = 0, met toutes les valeurs de 1 à N dans le domaine
 * Si nombre != 0, met nombre dans le domaine
 * 
 * Renvoie faux si l'index est invalide
 */
int nouvelleCase(CSP* jeu, int numeroCase, int nombre);


/**
 * Ajoute la contrainte à la liste des contraintes du slot.
 * 
 * Si FCD est activé, ne met pas à jour le tableau permettant d'accéder
 * rapidement aux contraintes supérieur / inférieur.
 */
void addContrainteALaCase(Contrainte * contraint, Slot* slot);

/**
 * Ajoute la contrainte à la liste des contraintes des deux slots
 * auxquels elle est associée.
 * 
 * Si FCD est activé, met également à jour le tableau d'accés rapide.
 */
void majContrainte(Contrainte *contraint);

/**
 * Crée une nouvelle contrainte dans le CSP reliant les deux cases dont le numéro
 * est passé en paramètre.
 * 
 * Le symbol donné définit le type de contrainte :
 * - si < ou ^, numeroCase  < numeroCase2
 * - si > ou v, numeroCase2 < numeroCase
 * - si . ou  , numerocase != numeroCase2
 */
int nouvelleContrainte(CSP* jeu, int numeroCase, int numeroCase2, char symbol);

/**
 * Ajoute les contraintes de différences entre idSlot et tous les autres slots
 * d'id supérieurs qui ne sont pas directement à côté.
 * 
 * Exemple si idslot correspond au slot e :
 * a b c d
 * e f g h
 * i j k l
 * m n o p
 * 
 * Ajoute les contraintes de différence de e avec g, h et m.
 */
void remplirUneAutreContrainte(CSP *jeu, int idslot);

/**
 * Complète la liste des contraintes afin de prendre en compte les différences
 * entre cases de même ligne et cases de même colonne malgré le fait qu'elles
 * ne soient pas directement adjacente.
 */
void remplirLesAutresContraintes(CSP *jeu);

/**
 * Lit dans le fichier un nombre et un symbole.
 * 
 * Le nombre lu sert à mettre en place le domaine de la numCasième case du CSP.
 * Le symbole permet de créer la contrainte entre la case et celle à sa droite.
 * 
 * Renvoie 0 en cas d'erreur de lecture, 1 sinon.
 */
int lireNombreSymbole(CSP* jeu, FILE* file, int * numCase);

/**
 * Lit N-1 cases dans le fichier.
 * 
 * Renvoie 0 si échec, 1 si réussite.
 */
int lireLignePaire(CSP * jeu, FILE * file, int numCase);

/**
 * Crée un objet CSP (représentation du jeu de futoshiki sous forme CSP)
 * à partir des données du fichier.
 */
CSP * remplirCSPF(FILE * file);


/**
 * Ouvre le fichier ayant le nom passé en paramètre et crée un instance
 * CSP à partir des données du fichier.
 */
CSP * lireFichier(char *fichier);


#endif
