#ifndef LISH_STRING_H
#define LISH_STRING_H

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

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
  if (0 == s->cap) s->cap = STRING_DEFAULT_CAP;
  if (NULL == s->raw) 
  {
    s->raw = malloc(s->cap + 1);
    *s->raw = 0;
  }
}

void
string_grow(string_t *s, size_t n)
{
  string_init(s);

  while (s->cap < s->len + n) s->cap *= 2;
  s->raw = realloc(s->raw, s->cap + 1);
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

string_t
string_from_(char const *s, size_t n)
{
  string_t str = string_null();
  string_append_(&str, s, n);
  return str;
}

string_t 
string_clone(string_t const *s)
{
  char *buf = malloc(s->cap + 1);
  memcpy(buf, s->raw, s->len + 1);
  return (string_t)
    {
      .len = s->len,
      .cap = s->cap,
      .raw = buf,
    };
}

void
string_del(string_t *s)
{
  if (NULL != s->raw) free(s->raw);
  memset(s, 0, sizeof(string_t));
}

#endif