#include "list.h"

#include <stdlib.h>

#include "errors.h"

list* list_init(void) {
  list* lst = malloc(sizeof(list));
  if (!lst) {
    error("FATAL: list_init(): malloc() failed");
  }
  lst->size = 0;
  lst->head = NULL;
  return lst;
}

list_node* list_insert(list* lst, list_node* node, void* data) {
  list_node* new_node = malloc(sizeof(list_node));
  if (!new_node) {
    error("FATAL: list_insert(): malloc() failed");
  }

  new_node->data = data;
  new_node->next = NULL;
  if (lst->size == 0) {
    lst->head = new_node;
  } else {
    new_node->next = node->next;
    node->next = new_node;
  }
  ++lst->size;
  return new_node;
}

list_node* list_next(list_node* node) { return node->next; }

void list_destroy(list* lst) {
  list_node* head = lst->head;
  while (head) {
    list_node* cur = head;
    head = head->next;
    free(cur->data);
    free(cur);
  }
  free(lst);
}
