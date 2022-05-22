#ifndef LISH_CHAR_H
#define LISH_CHAR_H

#include <stdbool.h>
#include "helpers.h"

#define IS_ALPHA(x) ((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z'))
#define IS_NUM(x) (x >= '0' && x <= '9')
#define IS_WHITESPACE(x) (x == ' ' || x == '\t' || x == '\n' || x == '\r')

uint8_t 
char_size(char const *p)
{
  bool m[] =
    { 
      (*p & 0b10000000) == 0b00000000,
      (*p & 0b11100000) == 0b11000000,
      (*p & 0b11110000) == 0b11100000,
      (*p & 0b11111000) == 0b11110000,
    };

  uint8_t sz = 0;
  for (size_t i = 0; i < ARRAY_LEN(m); ++i) 
    if (m[i]) sz = i;

  return sz + 1;
}

#endif 