#include "util.h"
#include "source.h"
#include "error.h"
#include "lex.h"

int 
main(int argc, char **argv) 
{
  // TODO: Temporary!

  if (2 != argc) 
  {
    fprintf(stderr, "Expected path.\n");
    return 1;
  }

  source_t source;

  if (1 == source_open(argv[1], &source))
  {
    fprintf(stderr, "Couldnt open file: %s\n", argv[1]);
    return 1;
  }

  vec_t toks = vec_null(sizeof(token_t));
  vec_t errs = vec_null(sizeof(error_t));
  lex(&source, &toks, &errs);

  for (size_t i = 0; i < errs.len; ++i)
  {
    error_t *e = (error_t *)vec_at(&errs, i);
    error_print(e, &source);
    error_del(e);
  }

  for (size_t i = 0; i < toks.len; ++i)
  {
    token_t *tok = (token_t *)vec_at(&toks, i);
    token_print(tok);
    token_del(tok);
  }

  vec_del(&toks);
  vec_del(&errs);
  source_del(&source);

  return 0;
}
