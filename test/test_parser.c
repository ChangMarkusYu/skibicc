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
}

int main(void) {
  UNITY_BEGIN();
  return UNITY_END();
}
