#ifndef __H_PRINCIPAL__
#define __H_PRINCIPAL__

#include "definitions.h"
#include "affichage.h"

/* ==== SIGNAUX ==== */

#ifdef __linux__
// Lorsque le programme reçoit le signal SIGUSR1, il affiche l'état actuel de sa progression
void signal_recu(int sig);

/**
 * Met en place la captation du signal sig.
 * Appel la fonction h lorsqu'il est capté.
 * Auteur : Edouard Thiel
 */
int bor_signal (int sig, void (*h)(int), int options);

#endif


#endif
