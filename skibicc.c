#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"

static struct option opts[] = {
    {"lex", no_argument, NULL, 1},
    {"parse", no_argument, NULL, 2},
    {"codegen", no_argument, NULL, 3},
    {0, 0, 0, 0},
};

typedef enum compiler_option {
  LEX,
  PARSE,
  CODEGEN,
  DEFAULT,
} compiler_option;

int main(int argc, char* argv[]) {
  int c;
  int opt_index;
  compiler_option opt = DEFAULT;
  while (1) {
    c = getopt_long_only(argc, argv, "", opts, &opt_index);
    if (c == -1) {
      break;
    }

    switch (c) {
      case 1:
        opt = LEX;
        break;
      case 2:
        opt = PARSE;
        break;
      case 3:
        opt = CODEGEN;
        break;
      default:
        return 1;
    }
  }

  if (optind >= argc) {
    fprintf(stderr, "Input file path not specified\n");
    return 1;
  }
  printf("Got argument: %s.\n",
         opt == DEFAULT ? "<none>" : opts[opt_index].name);
  char* path = argv[optind];
  printf("Got input file path: %s\n", path);
  char* res = read_file(path);
  printf("Got file: %s", res);
  return 0;
}
