#ifndef LISH_PARSE_H
#define LISH_PARSE_H

#include "util.h"
#include "source.h"
#include "error.h"
#include "lex.h"

typedef uint8_t node_kind_t;
enum 
{
  NODE_LIST,
  NODE_NAME,
  NODE_STR,
};

typedef struct
{
  node_kind_t kind;
  union
  {
    // NODE_LIST
    vec_t list;

    // NODE_NAME
    struct { char const *begin, *end; } span;
  
    // NODE_STR
    string_t str;
  } data;
} node_t;

void
node_print_(node_t const *n, int depth)
{ 
  printf("%*s", depth, "");
  switch (n->kind)
  {
    case NODE_LIST: 
      printf("List\n"); 
      vec_t children = n->data.list;
      for (size_t i = 0; i < children.len; ++i)
        node_print_((node_t const *)vec_at(&children, i), depth + 2);
      break;
    case NODE_NAME: 
      printf(
        "Name: %.*s\n", 
        (int)(n->data.span.end - n->data.span.begin), 
        n->data.span.begin
      ); 
      break;
    case NODE_STR:
      printf("String: %s\n", n->data.str.raw); 
      break;
  }
}

void
node_print(node_t const *n)
{
  node_print_(n, 0);
}

void
node_del(node_t *n)
{
  switch (n->kind)
  {
    case NODE_LIST:
      vec_del(&n->data.list);
      break;
    case NODE_STR:
      string_del(&n->data.str);
      break;
  }
  memset(n, 0, sizeof(node_t));
}

typedef uint8_t parselet_result_kind_t;
enum
{
  PARSE_OK,
  PARSE_ERR,
  PARSE_NONE,
};

typedef struct
{
  parselet_result_kind_t kind;
  
  // Optional node; Has value if kind == PARSE_OK
  node_t opt_node;
} parselet_result_t;

#define OK(n) (parselet_result_t) \
  {                               \
    .kind = PARSE_OK,             \
    .opt_node = n                 \
  }                       

#define ERR (parselet_result_t) \
  {                             \
    .kind = PARSE_ERR           \
  }

#define NONE (parselet_result_t) \
  {                              \
    .kind = PARSE_NONE           \
  }

typedef struct 
{
  vec_t const *toks;
  vec_t *nodes, *errs;
  size_t p;
} parse_state_t;

parselet_result_t
parse_once(parse_state_t *);

#define CUR(s) ((token_t const *)vec_at((vec_t *)(s)->toks, (s)->p))
#define EXPECT(s, k) (k == CUR(s)->kind)
#define END(s) ((s)->p >= (s)->toks->len)

typedef parselet_result_t(*parselet_t)(parse_state_t *);
#define PARSELET(name, ps) parselet_result_t name(parse_state_t *ps)

PARSELET(parselet_list, s)
{
  if (EXPECT(s, TOK_LPAREN))
  {
    token_t const *begin_tok = CUR(s);
    ++s->p; 

    vec_t children = vec_null(sizeof(node_t));
    while (false == END(s) && TOK_RPAREN != CUR(s)->kind)
    {
      token_t const *before_plet = CUR(s);
      parselet_result_t n = parse_once(s);
      switch (n.kind)
      {
        case PARSE_OK: 
          vec_push(&children, &n.opt_node);
          continue;
        case PARSE_ERR: return ERR;
        case PARSE_NONE: 
          
          return ERR;    
      }
    }

    if (true == END(s))
    {
      error_t err = error(
        ERR_FATAL,
        begin_tok->begin,
        "Expected matching closing parenthesis"
      );
      vec_push(s->errs, &err);
      return ERR;
    }
    // Skip, it's the closing right parenthesis.
    else ++s->p;

    node_t n = (node_t) 
      { 
        .kind = NODE_LIST,
        .data.list = children
      };
    return OK(n);
  }

  return NONE;
}

PARSELET(parselet_name, s)
{
  if (EXPECT(s, TOK_NAME))
  {
    token_t const *tok = CUR(s);
    ++s->p;
   
    node_t n = (node_t)
      {
        .kind = NODE_NAME,
        .data.span.begin = tok->begin,
        .data.span.end = tok->end
      };
    return OK(n);
  }

  return NONE;
}

PARSELET(parselet_str, s)
{
  if (EXPECT(s, TOK_STR))
  {
    token_t const *tok = CUR(s);
    ++s->p;
    
    node_t n = (node_t)
      { 
        .kind = NODE_STR,
        .data.str = string_clone(&tok->data.str)
      };
    return OK(n);
  }

  return NONE;
}

#undef PARSELET
#undef EXPECT

parselet_t const PARSELETS[] =
  {
    parselet_list,
    parselet_name,
    parselet_str,
  };

parselet_result_t
parse_once(parse_state_t *s)
{
  for (size_t i = 0; i < ARRAY_LEN(PARSELETS); ++i)
  {
    parselet_result_t res = PARSELETS[i](s);

    switch (res.kind)
    {
      case PARSE_NONE: continue;
      default: return res; 
    }
  }

  return NONE;
}

#undef NONE
#undef ERR
#undef OK

int
parse(vec_t const *toks, source_t const *src, vec_t *errs, vec_t *out)
{
  parse_state_t s = (parse_state_t)
    {
      .toks = toks,
      .nodes = out,
      .errs = errs,
      .p = 0
    };

  PARSE_REITER:
    {
      if (END(&s)) return 0; 

      parselet_result_t res = parse_once(&s);
      switch (res.kind)
      {
        case PARSE_OK: 
          vec_push(out, &res.opt_node);
          goto PARSE_REITER;
        case PARSE_ERR: return 1;
        case PARSE_NONE: break;
      }

      {
        error_t err = error(
          ERR_FATAL,
          CUR(&s)->begin,
          "Unexpected %s",
          token_kind_str(CUR(&s)->kind)
        );
        vec_push(errs, &err);
      }
    }

  return 0;
}

#undef CUR
#undef END

#endif
