#include "../lexer.h"
#include "../parser.h"
#include "../unity/unity.h"
#include "array.h"

void setUp(void) {}

void tearDown(void) {}

void test_parser_basic(void) {
  char text[] = "int main(void) { return 2; }";
  array tokens = lex(text);
  parse(&tokens);
  destroy_tokens(&tokens);
}

void test_parser_unary_expression(void) {
  char text[] = "++ + - &foo++--";
  array tokens = lex(text);
  parser parser;
  parser.cur = 0;
  parser.tokens = &tokens;
  parser.ast = NULL;
  ast_node* ast = parse_expression(&parser);
  destroy_tokens(&tokens);

  TEST_ASSERT_EQUAL(AST_EXPR, ast->node_type);
  TEST_ASSERT_EQUAL(OP_PREINC, ast->node.expression->op->op_type);

  ast = ast->node.expression->lhs;
  TEST_ASSERT_EQUAL(AST_EXPR, ast->node_type);
  TEST_ASSERT_EQUAL(OP_POS, ast->node.expression->op->op_type);

  ast = ast->node.expression->lhs;
  TEST_ASSERT_EQUAL(AST_EXPR, ast->node_type);
  TEST_ASSERT_EQUAL(OP_NEG, ast->node.expression->op->op_type);

  ast = ast->node.expression->lhs;
  TEST_ASSERT_EQUAL(AST_EXPR, ast->node_type);
  TEST_ASSERT_EQUAL(OP_ADDROF, ast->node.expression->op->op_type);

  ast = ast->node.expression->lhs;
  TEST_ASSERT_EQUAL(AST_EXPR, ast->node_type);
  TEST_ASSERT_EQUAL(OP_POSTDEC, ast->node.expression->op->op_type);

  ast = ast->node.expression->lhs;
  TEST_ASSERT_EQUAL(AST_EXPR, ast->node_type);
  TEST_ASSERT_EQUAL(OP_POSTINC, ast->node.expression->op->op_type);

  ast = ast->node.expression->lhs;
  TEST_ASSERT_EQUAL(AST_VAR, ast->node_type);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_parser_basic);
  RUN_TEST(test_parser_unary_expression);
  return UNITY_END();
}
