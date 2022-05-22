#ifndef LISH_INTERNER_H
#define LISH_INTERNER_H 

#define INTERNER_DEFAULT_CAP 16

typedef size_t interner_key_t;

typedef struct
{
  vec_t strings;
} interner_t;

interner_t
interner_(size_t n)
{
  return (interner_t)
    { 
      .strings = vec_new_(sizeof(string_t), INTERNER_DEFAULT_CAP),
    };
}

interner_t
interner(void)
{
  return interner_(INTERNER_DEFAULT_CAP);
}

interner_key_t
intern(interner_t *inter)
{
   
}

#endif