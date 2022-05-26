#ifndef LISH_LEX_H
#define LISH_LEX_H

#include "char.h"
#include "vec.h"
#include "string.h"
#include "source.h"
#include "error.h"
#include "interner.h"
#include "token.h"

#define LEX_TOKS_DEFAULT_CAP 1024

typedef struct 
{
  char const *p;
  source_t const *src;
  interner_t *intern;
  vec_t *toks, *errs;
} lex_state_t;

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

LEXLET(lex_comment, s)
{
  if ('#' == *s->p)
  {
    while (0 != *s->p && '\n' != *s->p) ++s->p;
    return LEX_OK;
  }

  return LEX_NONE;
}

LEXLET(lex_paren, s)
{
  token_kind_t kind;
  char const *begin = s->p;

  switch (*s->p) 
  {
    case '(': kind = TOK_LPAREN; break;
    case ')': kind = TOK_RPAREN; break;
    default: return LEX_NONE;
  }

  ++s->p;

  vec_push(
    s->toks, 
    &(token_t) 
      { 
        .kind = kind,
        .begin = begin,
        .end = s->p 
      }
  );

  return LEX_OK;
}

LEXLET(lex_kw_name, s)
{
  if (IS_ALPHA(*s->p))
  {
    char const *begin = s->p;

    while (IS_ALPHA(*s->p) 
      || IS_NUM(*s->p) 
      || '-' == *s->p 
      || '_' == *s->p) 
      ++s->p;
  
    for (size_t i = 0; i < ARRAY_LEN(KEYWORDS); ++i)
      if (0 == strncmp(begin, KEYWORDS[i], (size_t)(s->p - begin)))
      {
        token_t tok = (token_t)
          {
            .kind = TOK_KW,
            .begin = begin,
            .end = s->p,
            .data.kw = i
          };
        
        vec_push(s->toks, &tok);

        return LEX_OK;
      }

    interner_key_t key = intern_(s->intern, begin, (size_t)(s->p - begin));

    token_t tok = (token_t)
      {
        .kind = TOK_NAME,
        .begin = begin,
        .end = s->p,
        .data.key = key
      };

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
          .begin = begin,
          .end = s->p,
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
    lex_comment,
    lex_paren,
    lex_kw_name,
    lex_str
  };

int
lex(source_t const *src, interner_t *intern, vec_t *out, vec_t *errs)
{
  vec_fit(out, LEX_TOKS_DEFAULT_CAP);

  lex_state_t s = (lex_state_t)
    {
      .p = src->raw,
      .src = src,
      .intern = intern,
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
