// Les lignes 31 et 59 sont faites pour le cas ou l'option -h (help) est
// fournie, affiche l'aide et quitter immédiatement
// sans essayer d'ouvrir l'entrée standard.

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>

#include "opt.h"

int options(int argc, char *argv[], bool *b, size_t *i,bool *p, bool *l, bool *R, bool *S, int *op) {
  errno = 0;
  opterr = 0;
  int opt;
  static struct option long_options[] = {
    {"help", no_argument, 0, 'h'},
    {"max-length", required_argument, 0, 'i'},
    {"lexicographic", no_argument, 0, 'l'},
    {"reverse", no_argument, 0, 'R'},
    {"no_sort", no_argument, 0, 'S'},
    {0, 0, 0, 0}
  };
  while ((opt
        = getopt_long(argc, argv, "hbi:plRS", long_options, NULL)) != -1) {
    switch (opt) {
      case 'h':
        *op = argc + 1;
        show_help(argv);
        return 0;
        break;
      case 'b':
        *b = true;
        break;
      case 'i':
        *i = strtoul(optarg, NULL, 10);
        if (errno != 0) {
          fprintf(stderr, "Overflow error.\n");
          return -1;
        }
        break;
      case 'l':
        *l = true;
        break;
      case 'p':
        *p = true;
        break;
      case 'S':
        *l = false;
        *R = false;
        *S = true;
        break;
      case 'R':
        *R = true;
        break;
      default:
        *op = argc + 1;
        fprintf(stderr, "%s: Unrecognized option '%s'.\n", argv[0],
            argv[optind - 1]);
        fprintf(stderr, "Try './xwc --help' for more information.\n");
        return -1;
    }
  }
  *op = optind;
  return 0;
}

void show_help(char *argv[]) {
  printf("Usage: %s [OPTIONS] [FILE(S)]\n", argv[0]);
  printf("Options:\n");
  printf("  -h         Display this help and exit\n");
  printf("  -a         Option non available\n");
  printf("  -i VALUE   Set maximum word length (0 for unlimited)\n");
  printf("  -l         Sort output lexicographically\n");
  printf("  -R         Reverse sort order\n");
  printf("  (no args)  Read words from standard input\n");
  printf("Files:\n");
  printf("  FILE(S)    Read words from the specified file(s)\n");
  printf("  -          Read words from standard input\n");
}
