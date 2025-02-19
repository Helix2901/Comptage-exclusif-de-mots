//  Affiche sur la sortie standard la liste des différents mots lus sur
//    l'entrée standard et/ou dans des fichiers de maniere exclusive chaque mot
// étant suivi de son
//    nombre d'occurrences.
//  Limitations :
//  - les mots sont obtenus par lecture sur l'entrée des suites consécutives
//    de longueur maximale mais majorée WORD_LENGTH_MAX de caractères qui ne
//    sont pas de la catégorie isspace ;
//  - toute suite de tels caractères de longueur strictement supérieure à
//    WORD_LENGTH_MAX se retrouve ainsi découpée en plusieurs mots.
//  Attention ! Le point suivant est à retravailler. Le laisser en l'état est
//    contraire aux exigences prônées :

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <ctype.h>
#include <locale.h>
#include <unistd.h>

#include "hashtable.h"
#include "holdall.h"
#include "opt.h"

//--- Définition wrdx ----------------------------------------------------------

typedef struct wrdx wrdx;
struct wrdx {
  size_t count;
  int f_idx;
  int nfiles;
};

//  Définition de codes d'échappement ANSI pour colorer le texte.
#define ANSI_BLUE_ON_WHITE "\033[1;94;47m"
#define ANSI_RESET_COLOR "\033[0m"

//   str_hashfun : l'une des fonctions de pré-hachage conseillées par Kernighan
//     et Pike pour les chaines de caractères.
static size_t str_hashfun(const char *s);

//  scptr_display : affiche sur la sortie standard *cptr, le caractère
//    tabulation, la chaine de caractères pointée par s et la fin de ligne.
//    Renvoie zéro en cas de succès, une valeur non nulle en cas d'échec.
static int scptr_display(const char *s, wrdx *cptr);

static int reverse_strcoll(const char *a, const char *b);

//  rfree : libère la zone mémoire pointée par ptr et renvoie zéro.
static int rfree(void *ptr);

int main(int argc, char *argv[]) {
  int optind = 0;
  int n = 0;
  bool lexic = false;
  size_t i = 0;
  int index = 1;
  bool hs = false;
  bool ponct = false;
  bool nosort = false;
  bool reverse = false;
  int r = EXIT_SUCCESS;
  hashtable *ht = hashtable_empty((int (*)(const void *, const void *))strcmp,
      (size_t (*)(const void *))str_hashfun);
  holdall *has = holdall_empty();
  holdall *hacptr = holdall_empty();
  if (ht == NULL || has == NULL || hacptr == NULL) {
    goto error_capacity;
  }
  if (options(argc, argv, &hs, &i, &ponct, &lexic, &reverse,
      &nosort, &optind) != 0) {
    goto error_option;
  }
  n = argc - optind;
  if (n == 0) {
    optind = 0;
  }
  int argc1 = argc;
  if (optind == 0) {
    argc1 = 1;
  }
  if (optind > argc) {
    return EXIT_SUCCESS;
  }
  for (int k = optind; k < argc1; ++k) {
    char *filename = argv[k];
    FILE *f;
    if (optind == 0) {
      f = stdin;
      printf(
          ANSI_BLUE_ON_WHITE "--- starts reading for #%d FILE"
          ANSI_RESET_COLOR "\n",
          index);
      strcpy(filename, "standard input");
    } else if (strcmp(filename, "-") == 0) {
      printf(
          ANSI_BLUE_ON_WHITE "--- starts reading for #%d FILE"
          ANSI_RESET_COLOR "\n",
          index);
      f = stdin;
      rewind(f);
    } else {
      f = fopen(filename, "r");
      if (f == NULL) {
        goto error_file;
      }
    }
    int c;
    size_t ligne = 1;
    while ((c = fgetc(f)) != EOF) {
      size_t limite = BUFSIZ;
      if (i != 0) {
        limite = i;
      }
      char word[limite + 1];
      char word1[BUFSIZ] = {
        '\0'
      };
      size_t taille = 0;
      while (c != EOF && !isspace(c) && (isalnum(c) || c >= 128)) {
        int cpt = 0;
        if (taille > limite) {
          fprintf(stderr,
              "xwc: Word from file '%s' at line %ld cut: '%s....'.\n",
              filename, ligne, word1);
          if (ponct) {
            while (c != EOF && (c >= 0 && c <= 127) && !isspace(c)) {
              c = fgetc(f);
              cpt += 1;
            }
          } else {
            while (c != EOF && !isspace(c)) {
              c = fgetc(f);
              cpt += 1;
            }
          }
        }
        if (cpt == 0) {
          if (ponct) {
            if ((c >= 0 && c <= 127) && !isspace(c)) {
              word1[taille] = (char) c;
              taille += 1;
            }
          } else {
            if (!isspace(c)) {
              word1[taille] = (char) c;
              taille += 1;
            }
          }
          c = fgetc(f);
        }
        if (c == '\n') {
          ligne += 1;
        }
      }
      word1[taille] = '\0';
      if (strlen(word1) <= limite) {
        strcpy(word, word1);
        word[limite] = '\0';
      } else {
        strncpy(word, word1, limite);
        word[limite] = '\0';
      }
      wrdx *cptr = hashtable_search(ht, word);
      if (cptr != NULL) {
        cptr->count += 1;
        if (cptr->f_idx != index) {
          cptr->nfiles += 1;
        }
      } else {
        char *s = malloc(strlen(word) + 1);
        if (s == NULL) {
          goto error_capacity;
        }
        strcpy(s, word);
        if (holdall_put(has, s) != 0) {
          free(s);
          goto error_capacity;
        }
        cptr = malloc(sizeof *cptr);
        if (cptr == NULL) {
          goto error_capacity;
        }
        cptr->f_idx = index;
        cptr->nfiles = 1;
        cptr->count = 1;
        if (holdall_put(hacptr, cptr) != 0) {
          free(cptr);
          fclose(f);
          goto error_capacity;
        }
        if (hashtable_add(ht, s, cptr) == NULL) {
          fclose(f);
          goto error_capacity;
        }
      }
    }
    if (f == stdin) {
      printf(
          ANSI_BLUE_ON_WHITE "--- ends reading for #%d FILE"
          ANSI_RESET_COLOR "\n",
          index);
      if (!feof(f)) {
        goto error_read;
      }
    } else {
      if (fclose(f) != 0) {
        goto error_read;
      }
    }
    index += 1;
  }
  if (optind != 0) {
    for (int i = optind; i < argc; i++) {
      if (strcmp(argv[i], "-") != 0) {
        printf("\t%s", argv[i]);
      }
    }
  } else {
    printf("\t\"\"");
  }
  printf("\n");

  if (reverse) {
    setlocale(LC_COLLATE, "");
    holdall_sort(has, (int (*)(const void *, const void *))reverse_strcoll);
  }
  if (lexic) {
    setlocale(LC_COLLATE, "");
    holdall_sort(has, (int (*)(const void *, const void *))strcoll);
  }
  if (holdall_apply_context(has,
      ht, (void *(*)(void *, void *))hashtable_search,
      (int (*)(void *, void *))scptr_display) != 0) {
    goto error_write;
  }
#if defined HASHTABLE_STATS && HASHTABLE_STATS != 0
  hashtable_fprint_stats(ht, stderr);
#endif
  goto dispose;
error_capacity:
  fprintf(stderr, "*** Error: Not enough memory\n");
  goto error;
error_option:
  fprintf(stderr, "*** Error: Option error occurs\n");
  goto error;
error_read:
  fprintf(stderr, "*** Error: A read error occurs\n");
  goto error;
error_write:
  fprintf(stderr, "*** Error: A write error occurs\n");
  goto error;
error_file:
  fprintf(stderr, "xwc: can't open for reading file\n");
  goto error;
error:
  r = EXIT_FAILURE;
  goto dispose;
dispose:
  hashtable_dispose(&ht);
  if (has != NULL) {
    holdall_apply(has, rfree);
  }
  holdall_dispose(&has);
  if (hacptr != NULL) {
    holdall_apply(hacptr, rfree);
  }
  holdall_dispose(&hacptr);
  return r;
}

size_t str_hashfun(const char *s) {
  size_t h = 0;
  for (const unsigned char *p = (const unsigned char *) s; *p != '\0'; ++p) {
    h = 37 * h + *p;
  }
  return h;
}

int scptr_display(const char *s, wrdx *cptr) {
  if (cptr->nfiles == 1) {
    printf("%s", s);
    for (int i = 0; i < (cptr->f_idx) - 1; i++) {
      printf("\t");
    }
    printf("\t\t");
    return printf("%ld\n", (cptr->count)) < 0;
  }
  return 0;
}

int reverse_strcoll(const char *a, const char *b) {
  return -strcoll(a, b);
}

int rfree(void *ptr) {
  free(ptr);
  return 0;
}
