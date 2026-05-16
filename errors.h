//!@file
//!@brief Header file for various error printing functions.

#ifndef SKIBICC_ERRORS_H
#define SKIBICC_ERRORS_H

#include "lexer.h"

//! Reports an error and exit.
void error(char* fmt, ...);

//! Reports an unexpected token error and exit.
void error_tok(token* tok);

//! Reports an unexpected token error, followed by a message `fmt` and exit.
void error_tok_fmt(token* tok, char* fmt, ...);

//! Delegates to `malloc`, but if `malloc` returns `NULL`, exits the program
//! with an error message.
void* malloc_safe(size_t size);

//! Delegates to `calloc`, but if `calloc` returns `NULL`, exits the program
//! with an error message.
void* calloc_safe(size_t nelem, size_t elsize);

#endif  // SKIBICC_ERRORS_H
