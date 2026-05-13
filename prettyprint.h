#ifndef SKIBICC_PRETTYPRINT_H
#define SKIBICC_PRETTYPRINT_H

#include "ir.h"
#include "parser.h"

//! Prints the `ast` in a human readable format to stdout.
void prettyprint_ast(ast_node* ast);

void prettyprint_ir(ir_node* ir);

#endif
