#ifndef OPT__H
#define OPT__H

#include <stdlib.h>

// usage : affiche l'aide et termine le programme en notifiant un succès à
// l'environnement d'exécution
extern void show_help(char *argv[]);

// options :
extern int options(int argc, char *argv[], bool *b, size_t *i,
    bool *p, bool *l, bool *S, bool *R, int *op);

#endif
