#ifndef LISH_LEX_H
#define LISH_LEX_H

#include "util.h"
#include "source.h"
#include "error.h"

#define LEX_TOKS_DEFAULT_CAP 1024

typedef struct 
{
  char const *p;
  source_t const *src;
  vec_t *toks, *errs;
} lex_state_t;

typedef uint8_t token_kind_t;
enum
{
  TOK_LPAREN,
  TOK_RPAREN,
  TOK_NAME,
  TOK_STR,
};

typedef struct
{
  token_kind_t kind;
  union 
  {
    // TOK_NAME
    struct { char const *begin, *end; } span;
    
    // TOK_STR
    string_t str;
  } data;
} token_t;

void
token_print(token_t const *t) 
{
  switch (t->kind)
  {
    case TOK_LPAREN: printf("LParen"); break;
    case TOK_RPAREN: printf("RParen"); break;
    case TOK_NAME: 
      printf(
        "Name: %.*s", 
        (int)(t->data.span.end - t->data.span.begin), 
        t->data.span.begin
      ); 
      break;
    case TOK_STR: 
      printf("String: %s", t->data.str.raw); 
      break;
  }
  putchar('\n');
}

void
token_del(token_t *t)
{
  switch (t->kind)
  {
    case TOK_STR: string_del(&t->data.str); break;
    default: break;
  }
  memset(t, 0, sizeof(token_t));
}

typedef uint8_t lexlet_result_t;
enum
{
  LEX_NONE,
  LEX_ERR,
  LEX_OK,
};

typedef lexlet_result_t (*lexlet_t)(lex_state_t *);

#define LEXLET(name, st) lexlet_result_t name(lex_state_t *st)

LEXLET(lex_whitespace, s)
{
  while (IS_WHITESPACE(*s->p)) ++s->p;

  // Always fall through.
  return LEX_NONE;
}

LEXLET(lex_paren, s)
{
  token_kind_t kind;

  switch (*s->p) 
  {
    case '(': kind = TOK_LPAREN; break;
    case ')': kind = TOK_RPAREN; break;
    default: return LEX_NONE;
  }

  vec_push(
    s->toks, 
    &(token_t) { .kind = kind }
  );

  ++s->p;

  return LEX_OK;
}

LEXLET(lex_name, s)
{
  if (IS_ALPHA(*s->p))
  {
    char const *begin = s->p;

    while (IS_ALPHA(*s->p) || IS_NUM(*s->p)) ++s->p;
  
    token_t tok = (token_t)
      {
        .kind = TOK_NAME
      };

    tok.data.span.begin = begin;
    tok.data.span.end = s->p;

    vec_push(s->toks, &tok);

    return LEX_OK;
  }
  
  return LEX_NONE;
}

LEXLET(lex_str, s)
{
  if ('\'' == *s->p)
  {
    char const *begin = s->p;

    string_t str = string();

    ++s->p;
    while (0 != *s->p && '\'' != *s->p)
    {
      uint8_t sz = char_size(s->p);
      string_append_(&str, s->p, sz);
      s->p += sz;
    }

    if (*s->p != '\'')
    {
      error_t err = error(ERR_FATAL, begin, "Expected closing `'`");
      vec_push(s->errs, &err);
      return LEX_ERR;
    }
    else ++s->p;

    vec_push(
      s->toks, 
      &(token_t)
        {
          .kind = TOK_STR,
          .data.str = str
        }
    );

    return LEX_OK;
  }

  return LEX_NONE;
}

lexlet_t const LEXLETS[] =
  {
    lex_whitespace,
    lex_paren,
    lex_name,
    lex_str
  };

int
lex(source_t const *src, vec_t *out, vec_t *errs)
{
  vec_fit(out, LEX_TOKS_DEFAULT_CAP);

  lex_state_t s = (lex_state_t)
    {
      .p = src->raw,
      .src = src,
      .toks = out,
      .errs = errs
    };

  LEX_REITER:
    {
      for (size_t i = 0; i < ARRAY_LEN(LEXLETS); ++i)
        switch (LEXLETS[i](&s))
        {
          case LEX_OK: goto LEX_REITER;
          case LEX_ERR: return 1;
          case LEX_NONE: continue;
        }
      
      if (0 == *s.p) return 0; 

      {
        error_t err = error(
          ERR_FATAL, 
          s.p, 
          "Unexpected character `%.*s`", 
          char_size(s.p), 
          s.p
        );
        vec_push(errs, &err);
      }
    }
  
  return 0;
}

#endif
