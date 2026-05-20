#include "codegen.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "errors.h"
#include "hashmap.h"
#include "ir.h"
#include "list.h"

const char* asm_register_to_string(asm_register reg) {
  switch (reg) {
    case EAX:
      return "eax";
    case R10D:
      return "r10d";
  }
  error("Unimplemented");
}

const char* asm_instruction_type_to_string(asm_instruction_type inst_type) {
  switch (inst_type) {
    case ASM_MOV:
      return "movl";
    case ASM_NEG:
      return "negl";
    case ASM_NOT:
      return "notl";
    case ASM_ALLOCSTACK:
      return "subq";
    case ASM_RETURN:
      return "ret";
  }
  error("Unimplemented");
}

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
  inst->src->operand_type = ASM_OPND_IMM;
  inst->src->operand.immediate = alloc->offset;
  list_push_front(instructions, inst);
}

static asm_operand* create_register(asm_register reg) {
  asm_operand* opnd = calloc_safe(/*nelem=*/1, sizeof(asm_operand));
  opnd->operand_type = ASM_OPND_REG;
  opnd->operand.reg = reg;
  return opnd;
}

static void insert_mov(asm_operand* src, asm_operand* dst,
                       list* asm_instructions) {
  asm_instruction* inst = calloc_safe(/*nelem=*/1, sizeof(asm_instruction));
  inst->instruction_type = ASM_MOV;
  inst->src = src;
  if (src->operand_type == ASM_OPND_STACK &&
      dst->operand_type == ASM_OPND_STACK) {
    // Both are stack addreses. Not allowed.
    // Need to rewrite from:
    // mov <stack1>, <stack2>
    // to:
    // mov <stack1>, %r10d
    // mov %r10d, <stack2>
    inst->dst = create_register(R10D);
    list_push_back(asm_instructions, inst);

    // %r10d should be the src of the next mov:
    // mov %r10d, <stack2>
    inst->src = create_register(R10D);
  }
  inst->dst = dst;
}

static void insert_ret(list* asm_instructions) {
  asm_instruction* inst = calloc_safe(/*nelem=*/1, sizeof(asm_instruction));
  inst->instruction_type = ASM_RETURN;
  list_push_back(asm_instructions, inst);
}

static void lower_ir_return(ir_instruction* ir_instruction,
                            stack_allocator* alloc, list* asm_instructions) {
  // mov(val, reg(ax))
  ir_val* ir_lhs = ir_instruction->lhs;
  asm_operand* src = NULL;
  if (!ir_lhs->is_constant) {
    src = stack_allocator_get(alloc, ir_lhs, /*offset=*/-4);
  } else {
    src = calloc_safe(/*nelem=*/1, sizeof(asm_operand));
    // TODO: this is extremely dumb and probably wrong. Fix it.
    src->operand.immediate =
        ir_lhs->val.constant->node.consant->tok->constant.int_val;
  }
  asm_operand* dst = create_register(EAX);
  insert_mov(src, dst, asm_instructions);

  // ret
  insert_ret(asm_instructions);
}

static void lower_ir_instruction(ir_instruction* ir_instruction,
                                 stack_allocator* alloc,
                                 list* asm_instructions) {
  if (ir_instruction->instruction_type == IR_RETURN) {
    lower_ir_return(ir_instruction, alloc, asm_instructions);
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
    lower_ir_instruction(ir_inst, &alloc, instructions);
  }

  insert_allocate_stack_instruction(&alloc, instructions);
  stack_allocator_destroy(&alloc);
  return func_def;
}

asm_node* lower_ir(ir_node* ir) {
  asm_node* node = calloc_safe(/*nelem=*/1, sizeof(asm_node));
  node->func_def = lower_ir_func_def(ir->function_definition);
  return node;
}
