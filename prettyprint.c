#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"

//! Prints `depth` number of tab characters.
static void print_tabs(size_t depth) {
  for (size_t i = 0; i < depth; ++i) {
    printf("\t");
  }
}

//! Prints a variable `ast` node. `depth` is the depth of the node within the
//! whole AST.
static void print_variable(ast_node* ast, size_t depth) {
  token* tok = ast->node.variable->tok;
  char* str = strndup(tok->loc, tok->size);
  print_tabs(depth);
  printf("(Variable: %s)\n", str);
  free(str);
}

//! Prints a constant `ast` node. `depth` is the depth of the node within the
//! whole AST.
static void print_constant(ast_node* ast, size_t depth) {
  token* tok = ast->node.consant->tok;
  char* str = strndup(tok->loc, tok->size);
  print_tabs(depth);
  printf("(Identifier: %s)\n", str);
  free(str);
}

// Forward declaration.
static void prettyprint_internal(ast_node*, size_t);

//! Prints an expression `ast` node. `depth` is the depth of the node within the
//! whole AST.
static void print_expression(ast_node* ast, size_t depth) {
  print_tabs(depth);
  printf("(Expression, op: ");
  ast_expression* expression = ast->node.expression;
  // Print the operator
  token* op_tok = expression->op->tok;
  char* str = strndup(op_tok->loc, op_tok->size);
  printf("%s,\n", str);
  free(str);

  print_tabs(depth);
  printf(" lhs: \n");
  prettyprint_internal(expression->lhs, depth + 1);
  if (expression->rhs) {
    print_tabs(depth);
    printf(" rhs: \n");
    prettyprint_internal(expression->rhs, depth + 1);
  }

  print_tabs(depth);
  printf(")\n");
}

//! Helper method that implements the actual meat of the prettyprint function.
//! Prints the `ast` node. `depth` is the node's depth within the entire AST.
static void prettyprint_internal(ast_node* ast, size_t depth) {
  switch (ast->node_type) {
    case AST_UNKNOWN:
      print_tabs(depth);
      printf("(Unknown)\n");
      break;
    case AST_EXPR:
      print_expression(ast, depth);
      break;
    case AST_VAR:
      print_variable(ast, depth);
      break;
    case AST_CONST:
      print_constant(ast, depth);
      break;
  }
}

void prettyprint(ast_node* ast) { prettyprint_internal(ast, /*depth=*/0); }
