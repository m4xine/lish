#ifndef LISH_CFUN_H
#define LISH_CFUN_H

#include "vec.h"
#include "value.h"

#define CFUN(name, args) value_t name(vec_t const *args)

CFUN(cfun_echo, args)
{
  // TODO: Check args length/value kind

  printf("%s\n", ((value_t const *)vec_at((vec_t *)args, 0))->data.str.raw);

  return UNIT;
}

#undef CFUN

typedef struct
{
  char const *name;
  value_t (*f)(vec_t const *);
} named_cfun_t;

named_cfun_t const CFUNS[] = 
  {
    (named_cfun_t) { .name = "echo", .f = cfun_echo }
  };

#endif