#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

char* read_file(char* p) {
  FILE* f = fopen(p, "r");
  if (!f) {
    fprintf(stderr, "readfile(): failed to open file: %s.\n", p);
    return NULL;
  }

  char* out_buf;
  size_t out_size;
  FILE* out_stream = open_memstream(&out_buf, &out_size);
  if (!out_stream) {
    fprintf(stderr, "readfile(): failed to create output stream.\n");
    fclose(f);
    return NULL;
  }

  while (true) {
    char buf[4096];
    int res = fread(buf, 1, sizeof(buf), f);
    if (res == 0) {
      break;
    }
    fwrite(buf, 1, res, out_stream);
  }
  fflush(out_stream);
  fclose(out_stream);
  fclose(f);
  return out_buf;
}

// Returns true if `c` matches [a-zA-Z_]. Otherwise returns false.
static bool is_word_char(char c) { return isalnum(c) || c == '_'; }

uint64_t lex_identifier(const char* s) {
  if (!isalpha(s[0]) && s[0] != '_') {
    return 0;
  }

  const char* start = s;
  ++s;
  while (true) {
    char c = *s;
    if (!is_word_char(c)) {
      break;
    }
    ++s;
  }
  return s - start;
}

// TODO: Add support for oct, hex numbers. Add support for suffixes.
uint64_t lex_constant(const char* s) {
  const char* start = s;
  while (isdigit(*s)) {
    ++s;
  }
  if (is_word_char(*s)) {
    return 0;
  }
  return s - start;
}
