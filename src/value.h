#ifndef LISH_VALUE_H
#define LISH_VALUE_H 

#include <stdbool.h>
#include "value_decls.h"
#include "cfun_decls.h"

enum
{
  VAL_UNIT,
  VAL_BOOL,
  VAL_STR,
  VAL_CFUN
};

struct value_t
{
  value_kind_t kind;
  union
  {
    bool bool_;

    // VAL_STR
    string_t str;

    // VAL_CFUN
    cfun_t cfun;
  } data;
};

#endif 