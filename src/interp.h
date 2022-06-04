#ifndef LISH_INTERP_H
#define LISH_INTERP_H

#include <stdbool.h>
#include "vec.h"
#include "node.h"
#include "value.h"
#include "cfun.h"

typedef struct
{
  interner_key_t  key;
  value_t         value;
} stack_entry_t;

typedef struct
{
  bool  debug,
        show_exec;
  vec_t stack;
} interp_state_t;

void
stack_push(interp_state_t *s, 
           interner_key_t  key, 
           value_t         value)
{
  vec_push(&s->stack, &(stack_entry_t)
    {
      .key   = key,
      .value = value
    });

  if (s->debug)
  {
    printf("DEBUG: Pushed %zu\n", key);
    // TODO: Print value
  }

}

value_t *
stack_get(interp_state_t *s, interner_key_t key)
{
  for (size_t i = 0; i < s->stack.len; ++i)
  {
    stack_entry_t *e = 
      (stack_entry_t *)vec_at(&s->stack, s->stack.len - (i + 1));

    if (e->key == key) 
    {
      if (s->debug) printf("DEBUG: Got %zu\n", key);
      return &e->value;
    }
  }

  return NULL;
}

typedef uint8_t eval_result_kind_t;
enum
{
  EVAL_OK,
  EVAL_ERR
};

typedef struct
{
  eval_result_kind_t kind;
  value_t opt_value;
} eval_result_t;

#define UNIT ((eval_result_t) \
  {                           \
    .kind = EVAL_OK,          \
    .opt_value = (value_t)    \
      {                       \
        .kind = VAL_UNIT      \
      }                       \
  })

#define OK(v) ((eval_result_t)  \
  {                             \
    .kind       = EVAL_OK,      \
    .opt_value  = v             \
  })

#define ERR ((eval_result_t)  \
  {                           \
    .kind = EVAL_ERR          \
  })

#define PROPAGATE(r)             \
  {                              \
    eval_result_t res = *(r);    \
    switch (res.kind)            \
    {                            \
      case EVAL_ERR: return ERR; \
    }                            \
  }

eval_result_t
invoke(interp_state_t *s, value_t const *f, vec_t const *args)
{
  switch (f->kind)
  {
    case VAL_STR:
      string_t cmd = string_clone(&f->data.str);
      string_push(&cmd, ' ');
      for (size_t i = 0; i < args->len; ++i)
      {
        // TODO: Dont assume args type
        string_t const *s = &((value_t const *)vec_at((vec_t *)args, i))->data.str;
        string_append_(&cmd, s->raw, s->len);
      }

      if(s->show_exec) puts(cmd.raw);

      system(cmd.raw);
      string_del(&cmd);
      return UNIT;
    case VAL_CFUN:
      return OK(f->data.cfun(args));
  }

  return UNIT;
}

eval_result_t
eval(interp_state_t *s, node_t const *n)
{
  switch (n->kind)
  {
    case NODE_NAME:
      { 
        value_t *val = stack_get(s, n->data.key);
        if (NULL == val)
        {
          printf("TODO: Name didnt exist!\n");
          return ERR;
        } 
        else return OK(*val /* TEMPORARY */);
      } 
    case NODE_STR:
      {
        value_t val = (value_t)
          {
            .kind     = VAL_STR,
            .data.str = string_clone(&n->data.str)
          };
        return OK(val);
      }
    case NODE_LIST:
      {
        if (0 == n->data.list.len) return UNIT;

        eval_result_t head_res = eval(s, (node_t const *)vec_at((vec_t *)&n->data.list, 0));
        PROPAGATE(&head_res);

        vec_t args = vec_null(sizeof(value_t));
        vec_fit(&args, n->data.list.len - 1);
        for(size_t i = 1; i < n->data.list.len; ++i)
        {
          eval_result_t arg_res = 
            eval(s, (node_t const *)vec_at((vec_t *)&n->data.list, i));
          PROPAGATE(&arg_res);
          vec_push(&args, &arg_res.opt_value);
        }

        return invoke(s, &head_res.opt_value, &args);
      }
    case NODE_IF:
      {
        eval_result_t cond_res = eval(s, n->data.if_.cond);
        PROPAGATE(&cond_res);

        switch (cond_res.opt_value.kind)
        {
          case VAL_BOOL: 
            if (cond_res.opt_value.data.bool_)
            {
              eval_result_t true_branch_res = 
                eval(s, n->data.if_.true_branch);
              PROPAGATE(&true_branch_res);
              return OK(true_branch_res.opt_value);
            }
            else if (NULL != n->data.if_.opt_false_branch)
            {
              eval_result_t false_branch_res = 
                eval(s, n->data.if_.opt_false_branch);
              PROPAGATE(&false_branch_res);
              return OK(false_branch_res.opt_value);
            }
            return UNIT;
          default: return ERR; /* TEMPORARY */
        }
      }  
    case NODE_DEVAL:
      {
        eval_result_t res = eval(s, n->data.deval.value);
        PROPAGATE(&res);
        stack_push(s, n->data.deval.name, res.opt_value);
        return UNIT;
      }
  }

  return UNIT;
}

void
push_cfuns(interp_state_t *s, interner_t *in)
{
  for (size_t i = 0; i < ARRAY_LEN(CFUNS); ++i)
    stack_push(
      s,
      intern(in, CFUNS[i].name), 
      (value_t)
        {
          .kind = VAL_CFUN,
          .data.cfun = CFUNS[i].f
        }
    );
}

int
interp(vec_t const *nodes, interner_t *in)
{
  interp_state_t s = (interp_state_t)
    {
      .debug      = false,
      .show_exec  = true,
      .stack      = vec_(sizeof(stack_entry_t), 256)
    };

  push_cfuns(&s, in);

  for (size_t i = 0; i < nodes->len; ++i)
  {
    node_t const *n = (node_t const *)vec_at((vec_t *)nodes, i);
    eval_result_t res = eval(&s, n);
    switch (res.kind)
    {
      case EVAL_OK:   continue;
      case EVAL_ERR:  return 1;
    }
  }

  return 0;
}

#undef EVAL_PROP
#undef ERR
#undef UNIT

#endif