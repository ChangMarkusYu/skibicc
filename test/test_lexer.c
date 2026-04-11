#include "../lexer.h"
#include "../unity/unity.h"
#include "unity_internals.h"

void setUp(void) {}

void tearDown(void) {}

void test_lex_identifier(void) {
  TEST_ASSERT_EQUAL(4, lex_identifier("main"));
  TEST_ASSERT_EQUAL(4, lex_identifier("m123"));
  TEST_ASSERT_EQUAL(4, lex_identifier("m12n"));

  TEST_ASSERT_EQUAL(9, lex_identifier("foobarbaz"));
  TEST_ASSERT_EQUAL(9, lex_identifier("FooBarbaZ"));
  TEST_ASSERT_EQUAL(9, lex_identifier("fo4bAr7Az"));
  TEST_ASSERT_EQUAL(11, lex_identifier("fo_4bAr7_Az"));
  TEST_ASSERT_EQUAL(12, lex_identifier("_fo_4bAr7_Az"));
  TEST_ASSERT_EQUAL(15, lex_identifier("__fo_4bAr7_Az__"));
  TEST_ASSERT_EQUAL(15, lex_identifier("____fo_4bAr7_Az"));
  TEST_ASSERT_EQUAL(18, lex_identifier("____fo_4bAr7_Az___"));
  TEST_ASSERT_EQUAL(5, lex_identifier("_1234"));
  TEST_ASSERT_EQUAL(6, lex_identifier("_1234_"));
  TEST_ASSERT_EQUAL(6, lex_identifier("__1234"));
  TEST_ASSERT_EQUAL(8, lex_identifier("__1234__"));
  TEST_ASSERT_EQUAL(7, lex_identifier("___1234"));
  TEST_ASSERT_EQUAL(10, lex_identifier("___1234___"));

  TEST_ASSERT_EQUAL(6, lex_identifier("foobar;thisdoesnotcount"));
  TEST_ASSERT_EQUAL(6, lex_identifier("foobar thisdoesnotcount"));
  TEST_ASSERT_EQUAL(6, lex_identifier("foobar{thisdoesnotcount}"));
  TEST_ASSERT_EQUAL(6, lex_identifier("foobar/thisdoesnotcount"));

  TEST_ASSERT_EQUAL(0, lex_identifier(";thisdoesnotcount"));
  TEST_ASSERT_EQUAL(0, lex_identifier(" thisdoesnotcount}"));
  TEST_ASSERT_EQUAL(0, lex_identifier("{thisdoesnotcount}"));
  TEST_ASSERT_EQUAL(0, lex_identifier("/thisdoesnotcount"));

  TEST_ASSERT_EQUAL(0, lex_identifier("123456"));
  TEST_ASSERT_EQUAL(0, lex_identifier("0xab12c"));
  TEST_ASSERT_EQUAL(0, lex_identifier("01234"));
  TEST_ASSERT_EQUAL(0, lex_identifier("123foobar"));
}

void test_lex_constant(void) {
  TEST_ASSERT_EQUAL(3, lex_constant("234"));
  TEST_ASSERT_EQUAL(3, lex_constant("234;"));
  TEST_ASSERT_EQUAL(3, lex_constant("234)"));
  TEST_ASSERT_EQUAL(3, lex_constant("234/123"));
  TEST_ASSERT_EQUAL(3, lex_constant("234+456"));
  TEST_ASSERT_EQUAL(3, lex_constant("234-456"));
  TEST_ASSERT_EQUAL(3, lex_constant("234*456"));
  TEST_ASSERT_EQUAL(3, lex_constant("234,456"));

  TEST_ASSERT_EQUAL(5, lex_constant("01234"));
  TEST_ASSERT_EQUAL(7, lex_constant("0123400"));
  TEST_ASSERT_EQUAL(10, lex_constant("0123456789"));
  TEST_ASSERT_EQUAL(16, lex_constant("3745912748323957"));

  TEST_ASSERT_EQUAL(0, lex_constant(";123"));
  TEST_ASSERT_EQUAL(0, lex_constant("foobar123"));
  TEST_ASSERT_EQUAL(0, lex_constant("123foobar"));
  TEST_ASSERT_EQUAL(0, lex_constant("__123"));
  TEST_ASSERT_EQUAL(0, lex_constant("__123__"));
  TEST_ASSERT_EQUAL(0, lex_constant("thisdoes123notcount"));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_lex_identifier);
  RUN_TEST(test_lex_constant);
  return UNITY_END();
}
