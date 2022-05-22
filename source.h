#ifndef LISH_SOURCE_H
#define LISH_SOURCE_H

#include <stdio.h>
#include <stdlib.h>

typedef struct
{
  char const *origin;
  char *raw;
  size_t len;
} source_t;

int
source_open(char const *path, source_t *out)
{
  // I just copied this code from StackOverflow,
  // I've got no idea how to read a file properly.
  // Wow, I can write a compiler in C but I cant
  // even open a file, I'm going to cry for a bit.

  int64_t sz = 0;
  char *buf = NULL;
  FILE *f = fopen(path, "r");
  
  if (NULL != f && 0 == fseek(f, 0, SEEK_END)) 
  {
      sz = ftell(f);
      if (-1 == sz) return 1;

      buf = malloc(sz + 1);

      if (0 != fseek(f, 0, SEEK_SET)) return 1;
      
      sz = fread(buf, sizeof(char), sz, f);
      if (0 != ferror(f)) return 1; 
      else buf[sz] = 0;
  }
  else return 1;
  
  fclose(f);

  *out = (source_t)
    { 
      .origin = path,
      .raw = buf,
      .len = sz
    };
  
  return 0;
}

void
source_del(source_t *s)
{
  free(s->raw);
  memset(s, 0, sizeof(source_t));
}

typedef struct
{
  size_t line, col;
} loc_t;

int
source_loc(source_t const *s, char const *p, loc_t *out)
{
  char const *off = s->raw;
  *out = (loc_t)
    { 
      .line = 0,
      .col = 0,
    };
  
  while (off < p)
  {
    if (0 == *off) 
      // p points to memory outside of the source string
      return 1;
    else if ('\n' == *off)
    {
      ++out->line;
      out->col = 0;
    }
    else ++out->col;

    off += char_size(off);
  }

  return 0;
}

#endif 
