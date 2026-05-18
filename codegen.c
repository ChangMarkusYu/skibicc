#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "errors.h"
#include "hashmap.h"
#include "ir.h"
#include "list.h"

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
} asm_operand;

typedef enum asm_instruction_type {
  ASM_MOV,
  ASM_NEG,
  ASM_NOT,
  ASM_ALLOCSTACK,
  ASM_RETURN,
} asm_instruction_type;

typedef struct asm_instruction {
  asm_instruction_type instruction_type;
  asm_operand* lhs;
  asm_operand* rhs;
  asm_operand* dst;
} asm_instruction;

typedef struct asm_func_def {
  const char* name;
  list* instructions;
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

static inline bool stack_allocator_has_offset(stack_allocator* alloc) {
  return alloc->offset < 0;
}

static asm_operand* stack_allocator_get(stack_allocator* alloc, ir_val* val,
                                        int64_t offset) {
  asm_operand* opnd = calloc_safe(/*nelem=*/1, sizeof(asm_operand));
  opnd->operand_type = ASM_OPND_STACK;
  // TODO: strlen is expensive here. Should be precomputed by `var_name`.
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
  int64_t* data = malloc_safe(sizeof(int64_t));
  *data = alloc->offset;
  new_entry.data = data;
  new_entry.data_size = sizeof(int64_t);
  hashmap_insert(&alloc->var_to_offset, &new_entry);
  return opnd;
}

static void stack_allocator_destroy(stack_allocator* alloc) {
  hashmap_destroy(&alloc->var_to_offset);
}

static void insert_allocate_stack_instruction(stack_allocator* alloc,
                                              list* instructions) {
  if (!stack_allocator_has_offset(alloc)) {
    return;
  }

  asm_instruction* inst = calloc_safe(/*nelem=*/1, sizeof(asm_instruction));
  inst->instruction_type = ASM_ALLOCSTACK;
  inst->lhs->operand_type = ASM_OPND_IMM;
  inst->lhs->operand.immediate = alloc->offset;
  list_push_front(instructions, inst);
}

static void lower_ir_instruction(ir_instruction* ir_instruction,
                                 list* asm_instructions) {
  if (ir_instruction->instruction_type == IR_RETURN) {
    // mov(val, reg(ax))
    asm_instruction* inst = calloc_safe(/*nelem=*/1, sizeof(asm_instruction));
    inst->instruction_type = ASM_MOV;
    inst->lhs = calloc_safe(/*nelem=*/1, sizeof(asm_operand));
  }
}

static asm_func_def* lower_ir_func_def(ir_func_def* ir_func_def) {
  asm_func_def* func_def = calloc_safe(/*nelem=*/1, sizeof(asm_func_def));
  func_def->name = ir_func_def->name;
  func_def->instructions = list_init();
  list* instructions = func_def->instructions;

  stack_allocator alloc;
  stack_allocator_init(&alloc);

  for (size_t i = 0; i < instructions->size; ++i) {
    ir_instruction* ir_inst = array_at(ir_func_def->instructions, i);
    lower_ir_instruction(ir_inst, instructions);
  }

  insert_allocate_stack_instruction(&alloc, instructions);
  stack_allocator_destroy(&alloc);
  return func_def;
}

asm_node* lower_ir(ir_node* ir) { return NULL; }
