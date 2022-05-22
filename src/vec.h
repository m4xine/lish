#ifndef LISH_VEC_H
#define LISH_VEC_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

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

#endif