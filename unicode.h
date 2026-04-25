#ifndef SKIBICC_UNICODE_H
#define SKIBICC_UNICODE_H

#include <stdint.h>

#include "array.h"

//! Given a unicode code point `c`, converts it into UTF-16 format and push the
//! resulting bytes to the end of the array `arr`. It is assumed that the type
//! of elements in `arr` is `uint16_t`.
void encode_utf16(uint32_t c, array* arr);

//! Given a unicode code point `c`, converts it into UTF-32 format and push the
//! resulting bytes to the end of the array `arr`. It is assumed that the type
//! of elements in `arr` is `uint32_t`.
void encode_utf32(uint32_t c, array* arr);

//! Given a UTF-8 string `s`, converts its first (unicode) character from UTF-8
//! encoding to unicode code point and stores the result in `out`. Returns `s`
//! after skipping the converted character. Note that UTF-8 is a variable length
//! encoding (1~4 bytes), so the returned pointer may be advanced by 1~4 bytes
//! after `s` depending on the character.
const char* decode_utf8(const char* s, uint32_t* out);

#endif  // SKIBICC_UNICODE_H
