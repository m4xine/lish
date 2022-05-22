#ifndef LISH_TOKEN_H
#define LISH_TOKEN_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "string.h"
#include "interner.h"

typedef uint8_t token_kind_t;
enum
{
  TOK_LPAREN,
  TOK_RPAREN,
  TOK_NAME,
  TOK_STR,
};

char const *
token_kind_str(token_kind_t kind)
{
  switch (kind)
  {
    case TOK_LPAREN: return "(";
    case TOK_RPAREN: return ")";
    case TOK_NAME: return "Name";
    case TOK_STR: return "String";
  }
}

typedef struct
{
  token_kind_t kind;
  char const *begin, *end;
  union 
  {
    // TOK_NAME
    interner_key_t key;

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
        "Name: %.*s (%zu)", 
        (int)(t->end - t->begin), 
        t->begin,
        t->data.key
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

#endif