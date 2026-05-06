//!@file
//!@brief Source file for various error printing functions.

#include "errors.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

void error(char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");
  exit(1);
}

// TODO: Too basic. Should include filename and print the whole line.
//! Prints a error message formatted like the following to stderr:
//! [line_num:col_num]: unexpected token: tok.
static void error_tok_header(token* tok) {
  char* tok_str = strndup(tok->loc, tok->size);
  if (!tok_str) {
    error("FATAL: error_tok(): strndup() failed");
  }
  tok_str[tok->size] = '\0';
  fprintf(stderr, "[%zu, %zu]: unexpected token: %s.", tok->line_num,
          tok->col_num, tok_str);
  free(tok_str);
}

void error_tok(token* tok) {
  error_tok_header(tok);
  exit(1);
}

void error_tok_fmt(token* tok, char* fmt, ...) {
  error_tok_header(tok);
  fprintf(stderr, " ");
  va_list args;
  va_start(args, fmt);
  error(fmt, args);
}
