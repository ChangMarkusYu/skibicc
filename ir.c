#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "array.h"
#include "errors.h"
#include "parser.h"

typedef struct ir_val {
  bool is_constant;
  union {
    const char* var_name;
    ast_node* constant;
  } val;
} ir_val;

typedef enum ir_instruction_type {
  INST_RETURN,
  INST_ARITH,
} ir_instruction_type;

typedef struct ir_instruction {
  ir_instruction_type instruction_type;
  ast_operator* op;
  // Populated if unary or binary `op`.
  ir_val* lhs;
  // Populated if binary `op`.
  ir_val* rhs;
  // Destination operand. NULL if not applicable to the instruction type.
  ir_val* dst;
} ir_instruction;

typedef struct ir_func_def {
  const char* name;
  array* instructions;
} ir_func_def;

struct ir_node;
typedef struct ir_node {
  ir_func_def* function_definition;
  struct ir_node* next;
} ir_node;

const uint64_t MAX_NAME_SIZE = 32;

static uint64_t counter = 0;

static const char* generate_name(void) {
  char* name = malloc(MAX_NAME_SIZE);
  if (!name) {
    error("generate_name(): malloc() failed");
  }
  ++counter;
  // Names are formatted like 1_, 2_, 3_,...
  snprintf(name, MAX_NAME_SIZE, "%" PRIu64 "_", counter);
  return name;
}

static ir_val* create_ir_val_var(void) {
  ir_val* val = calloc(/*__nmemb=*/1, sizeof(ir_val));
  if (!val) {
    error("FATAL: create_ir_val_var(): calloc() failed");
  }
  val->is_constant = false;
  val->val.var_name = generate_name();
  return val;
}

static ir_val* create_ir_val_constant(ast_node* node) {
  ir_val* val = calloc(/*__nmemb=*/1, sizeof(ir_val));
  if (!val) {
    error("FATAL: create_ir_val_constant(): calloc() failed");
  }
  val->is_constant = true;
  val->val.constant = node;
  return val;
}

ir_val* emit_ir_instruction(ast_node* node, array* instructions) {
  if (node->node_type == AST_VAR) {
    return create_ir_val_var();
  }
  if (node->node_type == AST_CONST) {
    return create_ir_val_constant(node);
  }

  if (node->node_type == AST_EXPR) {
    // Only unary ops for now. Only recurse on lhs.
    ir_val* lhs = emit_ir_instruction(node->node.expression->lhs, instructions);
    ir_instruction* inst = array_push_back(instructions);
    inst->instruction_type = INST_ARITH;
    inst->op = node->node.expression->op;
    inst->lhs = lhs;
    inst->dst = create_ir_val_var();
    return inst->dst;
  }

  if (node->node_type == AST_RETSTMNT) {
    emit_ir_instruction(node->node.statement->expression, instructions);
    ir_instruction* inst = array_push_back(instructions);
    inst->instruction_type = INST_RETURN;
    return NULL;
  }

  error("Unimplemented");
  return NULL;
}

static ir_node* create_ir_node(void) {
  ir_node* node = calloc(/*__nmemb=*/1, sizeof(ir_node));
  if (!node) {
    error("FATAL: create_ir_node(): calloc failed.");
  }
  node->function_definition = calloc(/*__nmemb=*/1, sizeof(ir_func_def));
  return node;
}

ir_node* emit_ir(ast_node* ast) {
  ir_node* ir = create_ir_node();
  ir->function_definition->name = "main";
  emit_ir_instruction(ast, ir->function_definition->instructions);
  return ir;
}
