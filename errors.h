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

#endif  // SKIBICC_ERRORS_H
