#ifndef LISH_CFUN_H
#define LISH_CFUN_H

#include <unistd.h>
#include "cfun_decls.h"
#include "value.h"

#define CFUN(n, a) value_t n(vec_t const *a)

CFUN(cfun_exists, args)
{
  string_t path = ((value_t const *)vec_at((vec_t *)args, 0))->data.str;
  bool res = access(path.raw, F_OK) != -1;

  return (value_t) 
    {
      .kind       = VAL_BOOL,
      .data.bool_ = res
    };
}

struct named_cfun_t
{
  char const *name;
  cfun_t f;
};

#define NAMED_CFUN(n, f_) ((named_cfun_t) \
  {                                       \
    .name = n,                            \
    .f = f_                               \
  })

named_cfun_t const CFUNS[] =
  {
    NAMED_CFUN("exists", cfun_exists)
  };

#endif 