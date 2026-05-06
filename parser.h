#ifndef SKIBICC_PARSER_H

#include "array.h"
#include "lexer.h"

typedef enum ast_node_type {
  AN_UNKNOWN,
  AN_EXPR,
  AN_LITERAL,
  AN_STMNT,
  AN_FUNCDEF,
  AN_RETURN,
  AN_SEMICOL,
} ast_node_type;

typedef struct ast_node ast_node;

typedef struct ast_expression {
  token* tok;
} ast_expression;

typedef struct ast_statement {
  ast_node* expression;
} ast_statement;

struct ast_node {
  ast_node_type node_type;
  union {
    ast_expression* expression;
    ast_statement* statement;
  } node;
};

typedef struct parser {
  array* tokens;
  size_t cur;
  ast_node* ast;
} parser;

void parse(array* tokens);

#endif  // SKIBICC_PARSER_H
