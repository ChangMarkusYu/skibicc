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
static void parse_unary_expression(parser*, ast_expression*);
static void parse_expression(parser*, ast_expression*);

static void parse_type_name(parser* parser) {
  // TODO: Implement this.
}

static void parse_cast_expression(parser* parser, ast_expression* expression) {
  token* tok = peek_token(parser);
  if (is_punctuator_token(tok, "(")) {
    consume_token(parser);
    parse_type_name(parser);
    consume_punctuator(parser, ")");
  }
  parse_unary_expression(parser, expression);
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

static ast_node* parse_primary_expression(parser* parser,
                                          ast_expression* expression) {
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
    consume_token(parser);
    parse_expression(parser, expression);
    consume_punctuator(parser, ")");
    return NULL;
  }
  // TODO: implement generic selection parsing.
  error_tok(tok);
  return NULL;
}

static ast_operator* create_ast_operator(token* tok,
                                         ast_operator_type op_type) {
  ast_operator* op = malloc(sizeof(ast_operator));
  if (!op) {
    error("FATAL: create_ast_operator(): malloc() failed.");
  }
  op->tok = tok;
  op->op_type = op_type;
  return op;
}

static ast_expression* parse_postfix_operator(parser* parser,
                                              ast_expression* expression) {
  token* tok = peek_token(parser);
  if (tok->token_type != TK_PUNCT) {
    return NULL;
  }

  if (is_token_string_match(tok, "++")) {
    expression->op = create_ast_operator(tok, OP_POSTINC);
  } else if (is_token_string_match(tok, "--")) {
    expression->op = create_ast_operator(tok, OP_POSTDEC);
  }

  if (expression->op) {
    consume_token(parser);
    ast_node* lhs = create_ast_expression();
    expression->lhs = lhs;
    return lhs->node.expression;
  }
  return NULL;
}

static void parse_postfix_expression(parser* parser,
                                     ast_expression* expression) {
  // TODO: handle compound struct literal ( type-name ) { initializer-list }

  // TODO: Handle this.
  parse_primary_expression(parser, expression);

  ast_expression* cur = expression;
  while (has_token(parser)) {
    token* tok = peek_token(parser);
    // postfix-expression [ expression ]
    if (is_punctuator_token(tok, "[")) {
      consume_token(parser);
      parse_expression(parser, expression);
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
      parse_expression(parser, expression);
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
    ast_expression* res_exp = parse_postfix_operator(parser, cur);
    if (res_exp) {
      cur = res_exp;
      continue;
    }
    // No matching token - no more postfix expression to parse.
    return;
  }
}

static ast_expression* parse_prefix_operator(parser* parser,
                                             ast_expression* expression) {
  token* tok = peek_token(parser);
  if (tok->token_type != TK_PUNCT) {
    return NULL;
  }

  if (is_token_string_match(tok, "++")) {
    expression->op = create_ast_operator(tok, OP_PREINC);
  } else if (is_token_string_match(tok, "--")) {
    expression->op = create_ast_operator(tok, OP_PREDEC);
  }

  if (expression->op) {
    consume_token(parser);
    ast_node* lhs = create_ast_expression();
    expression->lhs = lhs;
    return lhs->node.expression;
  }
  return NULL;
}

static ast_expression* parse_unary_operator(parser* parser,
                                            ast_expression* expression) {
  token* tok = peek_token(parser);
  if (tok->token_type != TK_PUNCT) {
    return NULL;
  }

  if (is_token_string_match(tok, "*")) {
    expression->op = create_ast_operator(tok, OP_DEREF);
  } else if (is_token_string_match(tok, "&")) {
    expression->op = create_ast_operator(tok, OP_ADDROF);
  } else if (is_token_string_match(tok, "+")) {
    expression->op = create_ast_operator(tok, OP_POS);
  } else if (is_token_string_match(tok, "-")) {
    expression->op = create_ast_operator(tok, OP_NEG);
  } else if (is_token_string_match(tok, "!")) {
    expression->op = create_ast_operator(tok, OP_NOT);
  } else if (is_token_string_match(tok, "~")) {
    expression->op = create_ast_operator(tok, OP_BITNOT);
  }

  if (expression->op) {
    consume_token(parser);
    ast_node* lhs = create_ast_expression();
    expression->lhs = lhs;
    return lhs->node.expression;
  }
  return NULL;
}

static void parse_unary_expression(parser* parser, ast_expression* expression) {
  ast_expression* res_exp = parse_prefix_operator(parser, expression);
  if (res_exp) {
    parse_unary_expression(parser, res_exp);
    return;
  }

  res_exp = parse_unary_operator(parser, expression);
  if (res_exp) {
    parse_cast_expression(parser, res_exp);
    return;
  }

  token* tok = peek_token(parser);
  if (is_keyword_token(tok, "sizeof")) {
    consume_token(parser);
    if (is_punctuator_token(tok, "(")) {
      consume_token(parser);
      parse_type_name(parser);
      consume_punctuator(parser, ")");
      return;
    }
    parse_unary_expression(parser, expression);
    return;
  }
  // TODO: Handle _Alignof

  parse_postfix_expression(parser, expression);
}

static void parse_expression(parser* parser, ast_expression* expression) {
  parse_unary_expression(parser, expression);
}

static void parse_statement(parser* parser) {
  consume_keyword(parser, "return");

  ast_node ast;
  memset(&ast, 0, sizeof(ast_node));
  parse_expression(parser, ast.node.expression);

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
