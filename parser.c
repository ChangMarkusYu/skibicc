#include "parser.h"

#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "errors.h"
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

// TODO: Too basic. Make it pretty later.
void emit_error(token* actual, const char* expecte_tok) {
  char* actual_str = malloc(actual->size + 1);
  memcpy(actual, actual->loc, actual->size);
  actual_str[actual->size] = '\0';
  error("Unexpected token at [%zu, %zu]. Expected %s, got %s", actual->line_num,
        actual->col_num, expecte_tok, actual_str);
}

static void expect_token_internal(token* actual, token_type expected_type,
                                  const char* expecte_tok) {
  if (actual->token_type != expected_type) {
    emit_error(actual, expecte_tok);
  }
  size_t len = strlen(expecte_tok);
  if (len != actual->size) {
    emit_error(actual, expecte_tok);
  }
  if (strncmp(actual->loc, expecte_tok, len) != 0) {
    emit_error(actual, expecte_tok);
  }
}

inline token* peek_token(parser* parser) {
  return array_at(parser->tokens, parser->cur);
}

inline void consume_token(parser* parser) { parser->cur++; }

static void consume_keyword(parser* parser, const char* expecte_tok) {
  token* tok = peek_token(parser);
  expect_token_internal(tok, TK_KEYWRD, expecte_tok);
  consume_token(parser);
}

static void consume_punctuator(parser* parser, const char* expecte_tok) {
  token* tok = peek_token(parser);
  expect_token_internal(tok, TK_PUNCT, expecte_tok);
  consume_token(parser);
}

static void consume_literal(parser* parser) {
  token* tok = peek_token(parser);
  if (tok->token_type != TK_ICONST && tok->token_type != TK_FCONST &&
      tok->token_type != TK_STRLIT) {
    // TODO: This is not helpful. Write token type to string method and make
    // this pretty.
    error("Unexpected token type. Expected a constant or a literal.");
  }
  consume_token(parser);
}

static void consume_identifier(parser* parser) {
  token* tok = peek_token(parser);
  if (tok->token_type != TK_IDENT) {
    error("Unexpected token type. Expected an identifier.");
  }
  consume_token(parser);
}

void parse_expression(parser* parser) { consume_literal(parser); }

void parse_statement(parser* parser) {
  consume_keyword(parser, "return");

  parse_expression(parser);

  consume_punctuator(parser, ";");
}

void parse_function_definition(parser* parser) {
  consume_keyword(parser, "int");
  consume_identifier(parser);

  consume_punctuator(parser, "(");
  consume_keyword(parser, "void");
  consume_punctuator(parser, ")");

  consume_punctuator(parser, "{");
  parse_statement(parser);
  consume_punctuator(parser, "}");
}

void parse(array* tokens) {
  parser parser;
  parser.cur = 0;
  parser.tokens = tokens;
  parser.ast = NULL;

  parse_function_definition(&parser);
  return;
}
