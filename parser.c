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
  error("[%zu, %zu]: unexpected token. Expected %s, got %s", actual->line_num,
        actual->col_num, expecte_tok, actual_str);
}

static bool is_token_string_match(token* tok, const char* expected) {
  size_t len = strlen(expected);
  if (len != tok->size) {
    return false;
  }
  if (strncmp(tok->loc, expected, len) != 0) {
    return false;
  }
  return true;
}

static void expect_token_internal(token* actual, token_type expected_type,
                                  const char* expecte_tok) {
  if (actual->token_type != expected_type) {
    emit_error(actual, expecte_tok);
  }
  if (is_token_string_match(actual, expecte_tok)) {
    emit_error(actual, expecte_tok);
  }
}

static token* peek_token(parser* parser) {
  return array_at(parser->tokens, parser->cur);
}

static inline void consume_token(parser* parser) { parser->cur++; }

static inline bool has_token(parser* parser) {
  return parser->cur >= parser->tokens->size;
}

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

static void consume_any_literal(parser* parser) {
  token* tok = peek_token(parser);
  if (tok->token_type != TK_ICONST && tok->token_type != TK_FCONST &&
      tok->token_type != TK_STRLIT) {
    // TODO: This is not helpful. Write token type to string method and make
    // this pretty.
    error(
        "[%zu, %zu]: unexpected token type. Expected a constant or a literal.",
        tok->line_num, tok->col_num);
  }
  consume_token(parser);
}

static void consume_any_identifier(parser* parser) {
  token* tok = peek_token(parser);
  if (tok->token_type != TK_IDENT) {
    error("[%zu, %zu]: unexpected token type. Expected an identifier.",
          tok->line_num, tok->col_num);
  }
  consume_token(parser);
}

static void consume_primary_expression(parser* parser) {
  // TODO: implement this.
}

static bool is_punctuator_token(token* tok, const char* expected) {
  if (tok->token_type != TK_PUNCT) {
    return false;
  }
  return is_token_string_match(tok, expected);
}

static bool is_prefix_operator(token* tok) {
  return is_punctuator_token(tok, "++") || is_punctuator_token(tok, "--");
}

static bool is_postfix_operator(token* tok) {
  return is_punctuator_token(tok, "++") || is_punctuator_token(tok, "--");
}

static bool is_unary_operator(token* tok) {
  return is_punctuator_token(tok, "&") || is_punctuator_token(tok, "*") ||
         is_punctuator_token(tok, "+") || is_punctuator_token(tok, "-") ||
         is_punctuator_token(tok, "~") || is_punctuator_token(tok, "!");
}

static bool is_keyword_token(token* tok, const char* expected) {
  if (tok->token_type != TK_KEYWRD) {
    return false;
  }
  return is_token_string_match(tok, expected);
}

// Forward declarations.
static void parse_unary_expression(parser*);
static void parse_expression(parser*);

static void parse_type_name(parser* parser) {
  // TODO: Implement this.
}

static void parse_cast_expression(parser* parser) {
  token* tok = peek_token(parser);
  if (is_punctuator_token(tok, "(")) {
    consume_token(parser);
    parse_type_name(parser);
    consume_punctuator(parser, ")");
  }
  parse_unary_expression(parser);
}

static void parse_postfix_expression(parser* parser) {
  // TODO: handle compound struct literal ( type-name ) { initializer-list }

  consume_primary_expression(parser);
  while (has_token(parser)) {
    token* tok = peek_token(parser);
    // postfix-expression [ expression ]
    if (is_punctuator_token(tok, "[")) {
      consume_token(parser);
      parse_expression(parser);
      consume_punctuator(parser, "]");
      continue;
    }

    // postfix-expression ( argument-expression-list_opt )
    if (is_punctuator_token(tok, "(")) {
      consume_token(parser);
      tok = peek_token(parser);
      if (is_punctuator_token(tok, ")")) {
        consume_token(parser);
        continue;
      }
      parse_expression(parser);
      consume_punctuator(parser, ")");
      continue;
    }

    // postfix-expression . identifier
    // postfix-expression -> identifier
    if (is_punctuator_token(tok, ".") || is_punctuator_token(tok, "->")) {
      consume_token(parser);
      consume_any_identifier(parser);
      continue;
    }

    // postfix-expression ++
    // postfix-expression --
    if (is_postfix_operator(tok)) {
      consume_token(parser);
      continue;
    }
    // No matching token - no more postfix expression to parse.
    return;
  }
}

static void parse_unary_expression(parser* parser) {
  token* tok = peek_token(parser);
  if (is_prefix_operator(tok)) {
    consume_token(parser);
    parse_unary_expression(parser);
    return;
  }

  if (is_unary_operator(tok)) {
    consume_token(parser);
    parse_cast_expression(parser);
    return;
  }

  if (is_keyword_token(tok, "sizeof")) {
    consume_token(parser);
    if (is_punctuator_token(tok, "(")) {
      consume_token(parser);
      parse_type_name(parser);
      consume_punctuator(parser, ")");
      return;
    }
    parse_unary_expression(parser);
    return;
  }
  // TODO: Handle _Alignof

  parse_postfix_expression(parser);
}

static void parse_expression(parser* parser) { consume_any_literal(parser); }

static void parse_statement(parser* parser) {
  consume_keyword(parser, "return");

  parse_expression(parser);

  consume_punctuator(parser, ";");
}

static void parse_function_definition(parser* parser) {
  consume_keyword(parser, "int");
  consume_any_identifier(parser);

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
