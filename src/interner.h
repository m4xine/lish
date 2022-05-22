#ifndef LISH_INTERNER_H
#define LISH_INTERNER_H 

#include <string.h>
#include "string.h"
#include "vec.h"
#include "hash.h"

#define INTERNER_DEFAULT_CAP 16

typedef size_t interner_key_t;

typedef struct
{
  vec_t hashes;
  vec_t strings;
} interner_t;

interner_t
interner_(size_t n)
{
  return (interner_t)
    { 
      .hashes = vec_(sizeof(uint64_t), n),
      .strings = vec_(sizeof(string_t), n)
    };
}

interner_t
interner(void)
{
  return interner_(INTERNER_DEFAULT_CAP);
}

interner_key_t
intern(interner_t *in, char const *s)
{
  size_t sz = strlen(s);
  uint64_t hash = fnv1a(s, sz);

  for (size_t i = 0; i < in->hashes.len; ++i)
    if (hash == *(uint64_t *)vec_at(&in->hashes, i))
      return i;

  interner_key_t key = (interner_key_t)in->hashes.len;
  vec_push(&in->hashes, &hash);
  
  string_t str = string_from_(s, sz);
  vec_push(&in->strings, &str);

  return key;
}

void 
interner_del(interner_t *in)
{
  for (size_t i = 0; i < in->strings.len; ++i)
    string_del((string_t *)vec_at(&in->strings, i));
  vec_del(&in->hashes);
  vec_del(&in->strings);
  memset(in, 0, sizeof(interner_t));
}

#endif