#include <stdbool.h>
#include <stdlib.h>

#include "array.h"
#include "errors.h"
#include "parser.h"

typedef struct ir_val {
  bool is_constant;
  ast_node* val;
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
  // Always populated.
  ir_val* dst;
} ir_instruction;

typedef struct ir_node {
  array* instructions;
} ir_node;

static ir_val* create_ir_val(ast_node* node) {
  ir_val* val = calloc(/*__nmemb=*/1, sizeof(ir_val));
  if (!val) {
    error("FATAL: create_ir_val(): calloc() failed");
  }
  val->is_constant = false;
  if (node->node_type == AST_CONST) {
    val->is_constant = true;
  }
  val->val = node;
  return val;
}

static ir_node* create_ir_node(void) {
  ir_node* node = calloc(/*__nmemb=*/1, sizeof(ir_node));
  if (!node) {
    error("FATAL: create_ir_node(): calloc failed.");
  }
  array_init(node->instructions, sizeof(ir_instruction));
  return node;
}

const char* emit_ir_instruction(ast_node* node, array* instructions) {
  if (node->node_type == AST_EXPR) {
    // Only unary ops for now. Only recurse on lhs.
    const char* lhs =
        emit_ir_instruction(node->node.expression->lhs, instructions);
  }
  if (node->node_type == AST_RETSTMNT) {
  }
}

ir_node* emit_ir(ast_node* ast) {
  ir_node* ir = create_ir_node();
  emit_ir_instruction(ast, ir->instructions);
  return ir;
}
