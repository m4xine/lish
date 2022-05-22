#ifndef LISH_UTIL_H
#define LISH_UTIL_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define ARRAY_LEN(x) (sizeof(x) / sizeof(*x))

#define IS_ALPHA(x) ((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z'))
#define IS_NUM(x) (x >= '0' && x <= '9')
#define IS_WHITESPACE(x) (x == ' ' || x == '\t' || x == '\n' || x == '\r')

uint8_t 
char_size(char const *p)
{
  bool m[] =
    { 
      (*p & 0b10000000) == 0b00000000,
      (*p & 0b11100000) == 0b11000000,
      (*p & 0b11110000) == 0b11100000,
      (*p & 0b11111000) == 0b11110000,
    };

  uint8_t sz = 0;
  for (size_t i = 0; i < ARRAY_LEN(m); ++i) 
    if (m[i]) sz = i;

  return sz + 1;
}

#define VEC_DEFAULT_CAP 64

typedef struct 
{
  size_t len, tysz, cap;
  void *raw;
} vec_t; 

vec_t
vec_(size_t tysz, size_t cap)
{
  return (vec_t)
    {
      .len = 0,
      .tysz = tysz,
      .cap = cap,
      .raw = malloc(tysz * cap)
    };
}

vec_t
vec(size_t tysz)
{
  return vec_(tysz, VEC_DEFAULT_CAP);
}

vec_t
vec_null(size_t tysz)
{
  return (vec_t)
    {
      .len = 0,
      .tysz = tysz,
      .cap = 0,
      .raw = NULL
    };
} 

void
vec_init(vec_t *v)
{
  if (0 == v->cap) v->cap = VEC_DEFAULT_CAP;
  if (NULL == v->raw) v->raw = malloc(v->tysz * v->cap);
}

void
vec_fit(vec_t *v, size_t n)
{
  if (v->cap < n) v->cap = n;
  if (NULL != v->raw)
    v->raw = realloc(v->raw, v->tysz * n);
  else vec_init(v);
}

void
vec_grow(vec_t *v, size_t n)
{
  vec_init(v);

  while (v->cap < v->len + n) v->cap *= 2;
  v->raw = realloc(v->raw, v->tysz * v->cap);
}

void
vec_push_(vec_t *v, void const *es, size_t n)
{
  vec_grow(v, n);
  memcpy((char *)v->raw + (v->tysz * v->len), es, n * v->tysz);
  v->len += n;
}

void
vec_push(vec_t *v, void const *e)
{
  vec_push_(v, e, 1);
}

void *
vec_at(vec_t *v, size_t i)
{
  return (char *)v->raw + (i * v->tysz); 
}

void
vec_del(vec_t *v) 
{
  if (NULL != v->raw) free(v->raw);
  memset(v, 0, sizeof(vec_t));
} 

#define STRING_DEFAULT_CAP 32

typedef struct
{
  size_t len, cap;
  char *raw;
} string_t;

string_t
string_(size_t cap)
{
  char *raw = (char *)malloc(cap);
  *raw = 0;

  return (string_t)
    {
      .len = 0,
      .cap = cap,
      .raw = raw,
    };
}

string_t 
string(void)
{
  return string_(STRING_DEFAULT_CAP);
}

string_t
string_null(void)
{
  return (string_t)
    {
      .len = 0,
      .cap = 0,
      .raw = NULL
    };
}

void 
string_init(string_t *s)
{
  if (0 == s->cap) s->cap = VEC_DEFAULT_CAP;
  if (NULL == s->raw) 
  {
    s->raw = malloc(s->cap);
    *s->raw = 0;
  }
}

void
string_grow(string_t *s, size_t n)
{
  string_init(s);

  while (s->cap < s->len + n) s->cap *= 2;
  s->raw = realloc(s->raw, s->cap);
}

void
string_push(string_t *s, char c)
{
  string_grow(s, 1);
  *(s->raw + s->len++) = c;
  *(s->raw + s->len) = 0;
}

void
string_append_(string_t *s, char const *p, size_t n)
{
  string_grow(s, n);
  memcpy(s->raw + s->len, p, n);
  s->len += n;
  *(s->raw + s->len) = 0; 
}

void
string_del(string_t *s)
{
  if (NULL != s->raw) free(s->raw);
  memset(s, 0, sizeof(string_t));
}

#endif
