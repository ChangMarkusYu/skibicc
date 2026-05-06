#include "parser.h"

#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "errors.h"
#include "lexer.h"

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

static token* peek_token(parser* parser) {
  return array_at(parser->tokens, parser->cur);
}

static inline void consume_token(parser* parser) { parser->cur++; }

static inline bool has_token(parser* parser) {
  return parser->cur >= parser->tokens->size;
}

static void check_token(token* actual, token_type expected_type,
                        const char* expecte_tok) {
  if (actual->token_type != expected_type ||
      !is_token_string_match(actual, expecte_tok)) {
    error_tok_fmt(actual, "Expected a \"%s\".", expecte_tok);
  }
}

static void consume_keyword(parser* parser, const char* expecte_tok) {
  token* tok = peek_token(parser);
  check_token(tok, TK_KEYWRD, expecte_tok);
  consume_token(parser);
}

static void consume_punctuator(parser* parser, const char* expecte_tok) {
  token* tok = peek_token(parser);
  check_token(tok, TK_PUNCT, expecte_tok);
  consume_token(parser);
}

static void consume_any_literal(parser* parser) {
  token* tok = peek_token(parser);
  if (tok->token_type != TK_ICONST && tok->token_type != TK_FCONST &&
      tok->token_type != TK_STRLIT) {
    error_tok_fmt(tok, "Expected a constant or a literal.");
  }
  consume_token(parser);
}

static void consume_any_identifier(parser* parser) {
  token* tok = peek_token(parser);
  if (tok->token_type != TK_IDENT) {
    error_tok_fmt(tok, "Expected an identifier.");
  }
  consume_token(parser);
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

static void parse_primary_expression(parser* parser) {
  token* tok = peek_token(parser);
  token_type tok_type = tok->token_type;
  // identifier, constant or string literal.
  if (tok_type == TK_ICONST || tok_type == TK_FCONST || tok_type == TK_STRLIT ||
      tok_type == TK_IDENT) {
    consume_token(parser);
    return;
  }

  // ( expression )
  if (is_punctuator_token(tok, "(")) {
    consume_token(parser);
    parse_expression(parser);
    consume_punctuator(parser, ")");
    return;
  }
  // TODO: implement generic selection parsing.

  error_tok(tok);
}

static void parse_postfix_expression(parser* parser) {
  // TODO: handle compound struct literal ( type-name ) { initializer-list }

  parse_primary_expression(parser);
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
