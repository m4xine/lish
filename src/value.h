#ifndef LISH_VALUE_H
#define LISH_VALUE_H

#include <stdint.h>
#include "vec.h"
#include "string.h"

typedef uint8_t value_kind_t;
enum
{
  VAL_UNIT,
  VAL_STR,
  VAL_BOOL,
  VAL_CFUN,
};

typedef struct value_t value_t;
struct value_t 
{
  value_kind_t kind;
  union
  {
    // VAL_STR
    string_t str;

    // VAL_BOOL
    bool bool_;

    // VAL_CFUN
    value_t (*cfun)(vec_t const *);
  } data;
};

#define UNIT ((value_t) \
  {                     \
    .kind = VAL_UNIT    \
  })                    \

#define BOOL(b) ((value_t) \
  {                        \
    .kind = VAL_BOOL       \
    .data.bool_ = (b)      \
  })

#endif 