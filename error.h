#ifndef LISH_ERROR_H
#define LISH_ERROR_H

#include <stdio.h>
#include <stdarg.h>
#include "source.h"

#define error_level_t uint8_t

enum 
{
  ERR_FATAL,
  ERR_WARN,
  ERR_NOTE
};

typedef struct
{
  error_level_t level;
  char const *pos;
  char *msg;
} error_t;

error_t
error(error_level_t lvl, char const *pos, char const *fmt, ...)
{
  va_list args1, args2;
  va_start(args1, fmt);
  va_copy(args2, args1);
  size_t sz = vsnprintf(NULL, 0, fmt, args1);
  va_end(args1);
  char *msg = (char *)malloc(sz + 1);
  vsprintf(msg, fmt, args2);
  va_end(args2);

  return (error_t)
    {
      .level = lvl,
      .pos = pos,
      .msg = msg
    };
}

void
error_print(error_t const *e, source_t const *src)
{
  switch (e->level)
  {
    case ERR_FATAL: fprintf(stderr, "Fatal"); break;
    case ERR_WARN: fprintf(stderr, "Warn"); break;
    case ERR_NOTE: fprintf(stderr, "Note"); break;
  }

  loc_t loc;
  if (1 == source_loc(src, e->pos, &loc))
    fprintf(stderr, "(%s:<invalid>)", src->origin);
  else
    fprintf(stderr, "(%s:%zu:%zu)", src->origin, loc.line, loc.col);

  fprintf(stderr, ": %s\n", e->msg);
}

void
error_del(error_t *e)
{
  free(e->msg);
  memset(e, 0, sizeof(error_t));
}

#endif
