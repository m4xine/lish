#ifndef LISH_PARSE_H
#define LISH_PARSE_H

#include <stdbool.h>
#include "vec.h"
#include "string.h"
#include "source.h"
#include "error.h"
#include "token.h"
#include "node.h"

typedef uint8_t parselet_result_kind_t; 
enum
{
  PARSE_OK,
  PARSE_ERR,
  PARSE_NONE
};

typedef struct
{
  parselet_result_kind_t kind; 
  node_t opt_node; 
} parselet_result_t; 

parselet_result_t 
parselet_result_bind(parselet_result_t x, 
                     parselet_result_t (*f)(node_t))
{
  // Oh I miss my >>= so much.
  switch (x.kind)
  {
    case PARSE_OK: return f(x.opt_node);
    default: return x;
  }
}

void
parselet_result_print(parselet_result_t const *res)
{
  switch (res->kind)
  {
    case PARSE_OK:
      printf("Ok:\n");
      node_print_(&res->opt_node, 2);
      break;
    case PARSE_ERR:
      printf("Err\n");
      break;
    case PARSE_NONE:
      printf("None\n");
      break;
  }
}

#define OK(n) ((parselet_result_t) \
  {                                \
    .kind = PARSE_OK,              \
    .opt_node = n                  \
  })

#define ERR ((parselet_result_t) \
  {                              \
    .kind = PARSE_ERR            \
  })

#define NONE ((parselet_result_t) \
  {                               \
    .kind = PARSE_NONE            \
  })

typedef struct
{
  size_t       p;
  vec_t const *toks;
  vec_t       *errs, 
              *out;
} parse_state_t; 

#define CUR(s) ((token_t const *)vec_at((vec_t *)(s)->toks, (s)->p))
#define END(s) ((s)->p >= (s)->toks->len)
#define IS_KIND(s,k) ((k) == CUR(s)->kind)
#define GET_CHILD(n, i) ((node_t const *)vec_at((vec_t *)&(n)->data.list, i))
#define HAS_CHILD(n, i) (NODE_LIST == (n)->kind && i < (n)->data.list.len)
#define HAS_CHILD_KIND(n, k, i) (HAS_CHILD(n, i) && (k) == GET_CHILD(n, i)->kind)  
#define EXPECT_CHILD(s, n, i)     \
  {                               \
    if (!HAS_CHILD(n, i))         \
    {                             \
      error_t err = error(        \
        ERR_FATAL,                \
        n->begin,                 \
        "Expected child at %zu",  \
        i + 1                     \
      );                          \
      vec_push((s)->errs, &err);  \
      return ERR;                 \
    }                             \
  }
#define EXPECT_CHILD_KIND(s, n, k, i) \
  {                                   \
    if (!HAS_CHILD_KIND(n, k, i))     \
    {                                 \
      error_t err = error(            \
        ERR_FATAL,                    \
        n->begin,                     \
        "Expected %s at %zu",         \
        node_kind_str(k),             \
        i + 1                         \
      );                              \
      vec_push((s)->errs, &err);      \
      return ERR;                     \
    }                                 \
  }
#define EXPECT_NO_EXCESS(s, n, i)         \
  {                                       \
    if (HAS_CHILD(n, i + 1))              \
    {                                     \
      error_t err = error(                \
        ERR_FATAL,                        \
        n->begin,                         \
        "Unexpected excess child at %zu", \
        i + 2                             \
      );                                  \
      vec_push((s)->errs, &err);          \
      return ERR;                         \
    }                                     \
  }
#define MATCH_CHILD(n, i, f)                                    \
  (NODE_LIST == (n)->kind                                       \
    && i < (n)->data.list.len                                   \
    && f((node_t const *)vec_at((vec_t *)&(n)->data.list, i)))  
#define MATCH_KW_PRED(kw_) LAMBDA(bool, (node_t const *n) \
  {                                                       \
    return NODE_KW == n->kind && (kw_) == n->data.kw;     \
  })                                                      \

typedef parselet_result_t(*p1_parselet_t)(parse_state_t *);
typedef parselet_result_t(*p2_parselet_t)(parse_state_t *, node_t const *);

parselet_result_t
p1_parse_once(parse_state_t *s);

parselet_result_t
p2_parse(parse_state_t *s, node_t const *);

#define P1(name, s) parselet_result_t name(parse_state_t *s)
#define P2(name, s, n) parselet_result_t name(parse_state_t *s, node_t const *n)

P1(p1_name, s)
{
  if (IS_KIND(s, TOK_NAME))
  {
    token_t const *tok = CUR(s);
    ++s->p;

    node_t n = (node_t)
      {
        .kind = NODE_NAME,
        .begin = tok->begin,
        .end = tok->end,
        .data.key = tok->data.key
      };
    return OK(n);
  }

  return NONE;
}

P1(p1_str, s)
{
  if (IS_KIND(s, TOK_STR))
  {
    token_t const *tok = CUR(s);
    ++s->p;

    node_t n = (node_t)
      {
        .kind = NODE_STR,
        .begin = tok->begin,
        .end = tok->end,
        .data.str = string_clone(&tok->data.str)
      };
    return OK(n);
  }

  return NONE;
}

P1(p1_kw, s)
{
  if (IS_KIND(s, TOK_KW))
  {
    token_t const *tok = CUR(s);
    ++s->p;

    node_t n = (node_t)
      {
        .kind = NODE_KW,
        .begin = tok->begin,
        .end = tok->end,
        .data.kw = tok->data.kw
      };
    return OK(n);
  }

  return NONE;
}

P1(p1_list, s)
{
  if (IS_KIND(s, TOK_LPAREN))
  {
    token_t const *begin_tok = CUR(s);
    ++s->p;

    vec_t children = vec_null(sizeof(node_t));
    while (!END(s) && !IS_KIND(s, TOK_RPAREN))
    {
      token_t const *before_plet = CUR(s);
      parselet_result_t res = p1_parse_once(s);

      switch (res.kind)
      {
        case PARSE_OK:
          vec_push(&children, &res.opt_node);
          continue;
        case PARSE_ERR: return res;
        case PARSE_NONE:
          {
            error_t err = error(
              ERR_FATAL,
              begin_tok->begin,
              "Expected list child"
            );

            vec_push(s->errs, &err);
            return ERR;
          }
      }
    }

    token_t const *end_tok = NULL;
    if (END(s))
    {
      error_t err = error(
        ERR_FATAL,
        begin_tok->begin,
        "Expected matching closing parenthesis"
      );
      
      vec_push(s->errs, &err);
      return ERR;
    }
    else
    {
      end_tok = CUR(s);
      ++s->p;
    }

    node_t n = (node_t)
      {
        .kind = NODE_LIST,
        .begin = begin_tok->begin,
        .end = end_tok->end,
        .data.list = children
      };
    return OK(n);
  }

  return NONE;
}

p1_parselet_t const P1_PARSELETS[] = 
  {
    p1_name,
    p1_str,
    p1_kw,
    p1_list
  };

parselet_result_t
p1_parse_once(parse_state_t *s)
{
  for (size_t i = 0; i < ARRAY_LEN(P1_PARSELETS); ++i)
  {
    parselet_result_t p = P1_PARSELETS[i](s);

    switch (p.kind)
    {
      case PARSE_OK: 
      case PARSE_ERR: return p;
      case PARSE_NONE: continue;
    }
  }

  return NONE;
}

P2(p2_if, s, n)
{
  if (MATCH_CHILD(n, 0, MATCH_KW_PRED(KW_IF)))
  {
    EXPECT_CHILD(s, n, 1);

    node_t *true_branch = CLONE(GET_CHILD(n, 1)),
           *false_branch = NULL;

    if (HAS_CHILD(n, 2))
    {
      false_branch = CLONE(GET_CHILD(n, 2));
      EXPECT_NO_EXCESS(s, n, 2);
    }

    node_t ret = (node_t)
      {
        .kind = NODE_IF,
        .begin = n->begin,
        .end = n->end,
        .data.if_.true_branch = true_branch,
        .data.if_.opt_false_branch = false_branch,
      };
    
    return OK(ret);
  }

  return NONE;
}

P2(p2_defun, s, n)
{
  if (MATCH_CHILD(n, 0, MATCH_KW_PRED(KW_DEFUN)))
  {
    EXPECT_CHILD_KIND(s, n, NODE_NAME, 1);
    EXPECT_CHILD_KIND(s, n, NODE_LIST, 2);
    EXPECT_CHILD(s, n, 3);
    EXPECT_NO_EXCESS(s, n, 3);

    node_t const *n2 = GET_CHILD(n, 2);
    vec_t const *n2_children = &n2->data.list;
    vec_t params = vec_(sizeof(interner_key_t), n2_children->cap);
    for (size_t i = 0; i < n2_children->len; ++i)
    {
      EXPECT_CHILD_KIND(s, n2, NODE_NAME, i);
      vec_push(&params, &GET_CHILD(n2, i)->data.key);
    }

    node_t ret = (node_t)
      {
        .kind = NODE_DEFUN,
        .begin = n->begin,
        .end = n->end,
        .data.defun.name = GET_CHILD(n, 1)->data.key,
        .data.defun.params = params,
        .data.defun.body = CLONE(GET_CHILD(n, 3))
      };
    return OK(ret);
  }

  return NONE;
}

P2(p2_deval, s, n)
{
  if (MATCH_CHILD(n, 0, MATCH_KW_PRED(KW_DEVAL)))
  {
    EXPECT_CHILD_KIND(s, n, NODE_NAME, 1);
    EXPECT_CHILD(s, n, 2);
    EXPECT_NO_EXCESS(s, n, 2);

    node_t ret = (node_t)
      {
        .kind = NODE_DEVAL,
        .begin = n->begin,
        .end = n->end,
        .data.deval.name = GET_CHILD(n, 1)->data.key,
        .data.deval.value = CLONE(GET_CHILD(n, 2))
      };
    return OK(ret);
  }

  return NONE;
}

P2(p2_list, s, n)
{
  if (NODE_LIST == n->kind)
  {
    vec_t children = vec_(sizeof(node_t), n->data.list.cap);
    for (size_t i = 0; i < n->data.list.len; ++i)
    {
      node_t const *child = (node_t const *)vec_at((vec_t *)&n->data.list, i);
      parselet_result_t res = p2_parse(s, child);
      switch (res.kind)
      {
        case PARSE_OK: 
          vec_push(&children, &res.opt_node);
          break;
        default: return res;
      }
    }

    node_t ret = (node_t)
      {
        .kind = NODE_LIST,
        .begin = n->begin,
        .end = n->end,
        .data.list = children
      };
    return OK(ret);
  }

  return NONE;
}

p2_parselet_t const P2_PARSELETS[] =
  {
    p2_if,
    p2_defun,
    p2_deval,
    p2_list
  };

parselet_result_t
p2_parse(parse_state_t *s, node_t const *n)
{
  for (size_t i = 0; i < ARRAY_LEN(P2_PARSELETS); ++i)
  {
    parselet_result_t p = P2_PARSELETS[i](s, n);

    switch (p.kind)
    {
      case PARSE_NONE: continue;
      default: return p;
    }
  }

  return OK(*n);
}

int
parse(vec_t    const *toks,
      source_t const *src,
      vec_t          *errs,
      vec_t          *out)
{
  parse_state_t s = (parse_state_t)
    {
      .p = 0,
      .toks = toks,
      .errs = errs
    };

  PARSE_REITER:
    {
      if (END(&s)) goto PARSE_SUCCEED;

      parselet_result_t p = parselet_result_bind(
        p1_parse_once(&s),
        LAMBDA(parselet_result_t, (node_t n)
          {
            return p2_parse(&s, &n);
          })
      );

      switch (p.kind)
      {
        case PARSE_OK: 
          vec_push(out, &p.opt_node);
          goto PARSE_REITER;
        case PARSE_ERR: return 1;
        case PARSE_NONE: goto PARSE_FAILED;
      }
    }

  PARSE_SUCCEED:
    {
      return 0;
    }

  PARSE_FAILED:
    {
      token_t const *tok = CUR(&s);
      error_t err = error(
        ERR_FATAL,
        tok->begin,
        "Unexpected %s `%.*s`",
        token_kind_str(tok->kind),
        (int)(tok->end - tok->begin),
        tok->begin
      );

      vec_push(s.errs, &err);
      return 1;
    }
}

#undef P2
#undef P1
#undef OK
#undef ERR
#undef NONE
#undef MATCH_KW_PRED
#undef MATCH_CHILD
#undef EXPECT_NO_EXCESS
#undef EXPECT_CHILD_KIND
#undef HAS_CHILD_KIND
#undef GET_CHILD
#undef IS_KIND
#undef CUR
#undef END

#endif
