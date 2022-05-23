#ifndef LISH_INTERP_H
#define LISH_INTERP_H

#include <stdbool.h>
#include "vec.h"
#include "interner.h"
#include "value.h"
#include "cfun.h"

typedef struct
{
  interner_key_t key;
  value_t value;
} var_t;

typedef struct
{
  interner_t const *intern;
  vec_t stack; 
} interp_state_t;

value_t *
get_var(interp_state_t *s, interner_key_t key)
{
  for (size_t i = 0; i < s->stack.len; ++i)
  {
    var_t *v = (var_t *)vec_at(&s->stack, s->stack.len - (i + 1));

    if (v->key == key) return &v->value;
  }

  // TODO: Error

  return NULL;
}

value_t
invoke(interp_state_t *s, value_t const *f, vec_t const *args)
{
  switch (f->kind)
  {
    case VAL_CFUN:
      return f->data.cfun(args);
  }

  return UNIT;
}

value_t 
eval(interp_state_t *s, bool top_level, node_t const *n)
{
  // TODO: pop difference off stack if not top_level

  switch (n->kind)
  {
    case NODE_LIST:
      if (0 == n->data.list.len) return UNIT;
      
      node_t const *head = 
        (node_t const *)vec_at((vec_t *)&n->data.list, 0);
      value_t f = eval(s, false, head);
      vec_t args = vec_null(sizeof(value_t));
      for (size_t i = 1; i < n->data.list.len; ++i)
      {
        value_t val = eval(
          s, 
          false, 
          (node_t const *)vec_at((vec_t *)&n->data.list, i)
        );
        vec_push(&args, &val);
      }

      value_t ret = invoke(s, &f, &args);
      // TODO: clean up
      return ret;
    case NODE_NAME: 
      {
        value_t *val = get_var(s, n->data.key);
        return *val;
      }
    case NODE_STR: 
      return (value_t)
        {
          .kind = VAL_STR,
          .data.str = string_clone(&n->data.str)
        };
  }

  return UNIT;
}

#undef UNIT

int
interp(vec_t const *nodes, interner_t *in)
{
  interp_state_t s = (interp_state_t)
    {
      .intern = in,
      .stack = vec(sizeof(var_t))
    };

  for (size_t i = 0; i < ARRAY_LEN(CFUNS); ++i)
    vec_push(
      &s.stack, 
      &(var_t)
        {
          .key = intern(in, CFUNS[i].name),
          .value = (value_t)
            {
              .kind = VAL_CFUN,
              .data.cfun = CFUNS[i].f
            }
        }
    );

  for (size_t i = 0; i < nodes->len; ++i)
    eval(&s, true, (node_t const *)vec_at((vec_t *)nodes, i));

  return 0;
}

#endif  