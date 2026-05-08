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
  return parser->cur < parser->tokens->size;
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
static ast_node* parse_unary_expression(parser*);
static ast_node* parse_expression(parser*);

static void parse_type_name(parser* parser) {
  // TODO: Implement this.
}

static ast_node* parse_cast_expression(parser* parser) {
  token* tok = peek_token(parser);
  if (is_punctuator_token(tok, "(")) {
    consume_token(parser);
    parse_type_name(parser);
    consume_punctuator(parser, ")");
    return parse_cast_expression(parser);
  }
  return parse_unary_expression(parser);
}

static ast_node* create_ast_expression(void) {
  ast_node* node = calloc(sizeof(ast_node), /*__size=*/1);
  if (!node) {
    error("FATAL: create_ast_expression(): calloc() failed.");
  }
  node->node_type = AST_EXPR;
  ast_expression* expression = calloc(sizeof(ast_expression), /*__size=*/1);
  if (!expression) {
    error("FATAL: create_ast_expression(): calloc() failed.");
  }
  node->node.expression = expression;
  return node;
}

static ast_node* create_ast_constant(token* tok) {
  ast_node* node = calloc(sizeof(ast_node), /*__size=*/1);
  if (!node) {
    error("FATAL: create_ast_constant(): calloc() failed.");
  }
  node->node_type = AST_CONST;
  ast_constant* constant = calloc(sizeof(ast_constant), /*__size=*/1);
  if (!constant) {
    error("FATAL: create_ast_constant(): calloc() failed.");
  }
  constant->tok = tok;
  node->node.consant = constant;
  return node;
}

static ast_node* create_ast_variable(token* tok) {
  ast_node* node = calloc(sizeof(ast_node), /*__size=*/1);
  if (!node) {
    error("FATAL: create_ast_variable(): calloc() failed.");
  }
  node->node_type = AST_VAR;
  ast_variable* variable = calloc(sizeof(ast_variable), /*__size=*/1);
  if (!variable) {
    error("FATAL: create_ast_variable(): calloc() failed.");
  }
  variable->tok = tok;
  node->node.variable = variable;
  return node;
}

static ast_node* parse_primary_expression(parser* parser) {
  token* tok = peek_token(parser);
  token_type tok_type = tok->token_type;
  // Constant or string literal.
  if (tok_type == TK_ICONST || tok_type == TK_FCONST || tok_type == TK_STRLIT) {
    consume_token(parser);
    return create_ast_constant(tok);
  }
  // Identifier.
  if (tok_type == TK_IDENT) {
    consume_token(parser);
    return create_ast_variable(tok);
  }

  // ( expression )
  if (is_punctuator_token(tok, "(")) {
    // TODO: Fix this
    consume_token(parser);
    ast_node* node = parse_expression(parser);
    consume_punctuator(parser, ")");
    return node;
  }
  // TODO: implement generic selection parsing.
  error_tok(tok);
  return NULL;
}

static ast_operator* create_ast_operator(token* tok,
                                         ast_operator_type op_type) {
  ast_operator* op = calloc(sizeof(ast_operator), /*__size=*/1);
  if (!op) {
    error("FATAL: create_ast_operator(): malloc() failed.");
  }
  op->tok = tok;
  op->op_type = op_type;
  return op;
}

static ast_node* parse_postfix_operator(parser* parser) {
  token* tok = peek_token(parser);
  if (tok->token_type != TK_PUNCT) {
    return NULL;
  }

  ast_operator* op = NULL;
  if (is_token_string_match(tok, "++")) {
    op = create_ast_operator(tok, OP_POSTINC);
  } else if (is_token_string_match(tok, "--")) {
    op = create_ast_operator(tok, OP_POSTDEC);
  }

  if (op) {
    consume_token(parser);
    ast_node* node = create_ast_expression();
    node->node.expression->op = op;
    return node;
  }
  return NULL;
}

static ast_node* parse_postfix_expression(parser* parser) {
  // TODO: handle compound struct literal ( type-name ) { initializer-list }

  ast_node* node = parse_primary_expression(parser);
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
    ast_node* new_node = parse_postfix_operator(parser);
    if (new_node) {
      new_node->node.expression->lhs = node;
      node = new_node;
      continue;
    }
    // No matching token - no more postfix expression to parse.
    break;
  }
  return node;
}

static ast_node* parse_prefix_operator(parser* parser) {
  token* tok = peek_token(parser);
  if (tok->token_type != TK_PUNCT) {
    return NULL;
  }

  ast_operator* op = NULL;
  if (is_token_string_match(tok, "++")) {
    op = create_ast_operator(tok, OP_PREINC);
  } else if (is_token_string_match(tok, "--")) {
    op = create_ast_operator(tok, OP_PREDEC);
  }

  if (op) {
    consume_token(parser);
    ast_node* res = create_ast_expression();
    res->node.expression->op = op;
    return res;
  }
  return NULL;
}

static ast_node* parse_unary_operator(parser* parser) {
  token* tok = peek_token(parser);
  if (tok->token_type != TK_PUNCT) {
    return NULL;
  }

  ast_operator* op = NULL;
  if (is_token_string_match(tok, "*")) {
    op = create_ast_operator(tok, OP_DEREF);
  } else if (is_token_string_match(tok, "&")) {
    op = create_ast_operator(tok, OP_ADDROF);
  } else if (is_token_string_match(tok, "+")) {
    op = create_ast_operator(tok, OP_POS);
  } else if (is_token_string_match(tok, "-")) {
    op = create_ast_operator(tok, OP_NEG);
  } else if (is_token_string_match(tok, "!")) {
    op = create_ast_operator(tok, OP_NOT);
  } else if (is_token_string_match(tok, "~")) {
    op = create_ast_operator(tok, OP_BITNOT);
  }

  if (op) {
    consume_token(parser);
    ast_node* res = create_ast_expression();
    res->node.expression->op = op;
    return res;
  }
  return NULL;
}

static ast_node* parse_unary_expression(parser* parser) {
  ast_node* node = parse_prefix_operator(parser);
  if (node) {
    ast_node* lhs = parse_unary_expression(parser);
    node->node.expression->lhs = lhs;
    return node;
  }

  node = parse_unary_operator(parser);
  if (node) {
    ast_node* lhs = parse_cast_expression(parser);
    node->node.expression->lhs = lhs;
    return node;
  }

  token* tok = peek_token(parser);
  if (is_keyword_token(tok, "sizeof")) {
    consume_token(parser);
    if (is_punctuator_token(tok, "(")) {
      consume_token(parser);
      parse_type_name(parser);
      consume_punctuator(parser, ")");
      return NULL;
    }
    return parse_unary_expression(parser);
  }
  // TODO: Handle _Alignof

  node = parse_postfix_expression(parser);
  return node;
}

static ast_node* parse_expression(parser* parser) {
  return parse_unary_expression(parser);
}

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
