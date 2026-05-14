#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "errors.h"
#include "hashmap.h"
#include "ir.h"

typedef enum asm_operand_type {
  ASM_OPND_IMM,
  ASM_OPND_REG,
  ASM_OPND_STACK,
} asm_operand_type;

typedef struct asm_operand {
  asm_operand_type operand_type;
  union {
    int64_t immediate;
    const char* reg;
    int64_t offset;
  } operand;
} asm_operad;

typedef enum asm_instruction_type {
  ASM_MOV,
  ASM_NEG,
  ASM_NOT,
  ASM_ALLOCSTACK,
  ASM_RETURN,
} asm_instruction_type;

typedef struct asm_instruction {
  asm_instruction_type instruction_type;
  asm_operad* lhs;
  asm_operad* rhs;
  asm_operad* dst;
} asm_instruction;

typedef struct asm_func_def {
  const char* name;
  array* instructions;
} asm_func_def;

struct asm_node;
typedef struct asm_node {
  asm_func_def* func_def;
  struct asm_node* next;
} asm_node;

typedef struct stack_allocator {
  hashmap var_to_offset;
  int64_t offset;
} stack_allocator;

static void stack_allocator_init(stack_allocator* alloc) {
  hashmap_init(&alloc->var_to_offset);
  alloc->offset = 0;
}

static asm_operad* stack_allocator_get(stack_allocator* alloc, ir_val* val,
                                       int64_t offset) {
  asm_operad* opnd = calloc(/*__nmemb=*/1, sizeof(asm_operad));
  if (!opnd) {
    error("FATAL: stack_allocator_get(): calloc() failed.");
  }
  opnd->operand_type = ASM_OPND_STACK;
  // TODO: strlen is expensive here. Write a string class.
  size_t name_len = strlen(val->val.var_name);
  hashmap_entry* entry =
      hashmap_get(&alloc->var_to_offset, val->val.var_name, name_len);
  if (entry) {
    opnd->operand.offset = *((int64_t*)(entry->data));
    return opnd;
  }
  alloc->offset -= offset;
  opnd->operand.offset = alloc->offset;

  hashmap_entry new_entry;
  new_entry.key = (char*)val->val.var_name;
  new_entry.key_size = name_len;
  // TODO: put this malloc/calloc + check into a dedicated function.
  int64_t* data = malloc(sizeof(int64_t));
  if (!data) {
    error("FATAL: stack_allocator_get(): malloc() failed.");
  }
  *data = alloc->offset;
  new_entry.data = data;
  new_entry.data_size = sizeof(int64_t);
  hashmap_insert(&alloc->var_to_offset, &new_entry);
  return opnd;
}

static void stack_allocator_destroy(stack_allocator* alloc) {
  hashmap_destroy(&alloc->var_to_offset);
}

static asm_func_def* lower_func_def(ir_func_def* ir_func_def) {
  asm_func_def* asm_func_def = NULL;
  stack_allocator alloc;
  stack_allocator_init(&alloc);

  stack_allocator_destroy(&alloc);
  return asm_func_def;
}

asm_node* lower_ir(ir_node* ir) { return NULL; }
