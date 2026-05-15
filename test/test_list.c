#include <stdint.h>
#include <stdlib.h>

#include "../list.h"
#include "../unity/unity.h"

void setUp(void) {}

void tearDown(void) {}

void test_list_basic(void) {
  list* lst = list_init();
  list_node* cur = lst->head;
  for (uint64_t i = 0; i < 10000; ++i) {
    uint64_t* item = malloc(sizeof(uint64_t));
    *item = i;
    cur = list_insert(lst, cur, item);
  }

  TEST_ASSERT_EQUAL(10000, lst->size);
  size_t expected = 0;
  cur = lst->head;
  while (cur) {
    TEST_ASSERT_EQUAL(expected, *(uint64_t*)cur->data);
    cur = list_next(cur);
    ++expected;
  }

  list_destroy(lst);
}

void test_list_insert(void) {
  list* lst = list_init();
  list_node* cur = lst->head;
  list_node* mid;
  for (uint64_t i = 0; i < 100; ++i) {
    uint64_t* item = malloc(sizeof(uint64_t));
    *item = i;
    cur = list_insert(lst, cur, item);
    if (i == 50) {
      mid = cur;
    }
  }
  cur = mid;
  for (uint64_t i = 0; i < 50; ++i) {
    uint64_t* item = malloc(sizeof(uint64_t));
    *item = 999;
    cur = list_insert(lst, cur, item);
  }

  TEST_ASSERT_EQUAL(150, lst->size);
  cur = lst->head;
  for (uint64_t i = 0; i <= 50; ++i) {
    TEST_ASSERT_EQUAL(i, *(uint64_t*)cur->data);
    cur = list_next(cur);
  }
  for (uint64_t i = 0; i < 50; ++i) {
    TEST_ASSERT_EQUAL(999, *(uint64_t*)cur->data);
    cur = list_next(cur);
  }
  for (uint64_t i = 51; i < 100; ++i) {
    TEST_ASSERT_EQUAL(i, *(uint64_t*)cur->data);
    cur = list_next(cur);
  }
  list_destroy(lst);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_list_basic);
  RUN_TEST(test_list_insert);
  return UNITY_END();
}
