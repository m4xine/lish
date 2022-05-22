#include "vec.h"
#include "interner.h"
#include "source.h"
#include "error.h"
#include "lex.h"
#include "parse.h"

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

  interner_t intern = interner();
  vec_t toks = vec_null(sizeof(token_t));
  vec_t errs = vec_null(sizeof(error_t));
  lex(&source, &intern, &toks, &errs);

  for (size_t i = 0; i < toks.len; ++i)
  {
    token_t *tok = (token_t *)vec_at(&toks, i);
    token_print(tok);
  }

  vec_t nodes = vec_null(sizeof(node_t));
  parse(&toks, &source, &errs, &nodes);
  
  for (size_t i = 0; i < nodes.len; ++i)
  {
    node_t *n = (node_t *)vec_at(&nodes, i);
    node_print(n);
    node_del(n);
  }

  for (size_t i = 0; i < errs.len; ++i)
  {
    error_t *e = (error_t *)vec_at(&errs, i);
    error_print(e, &source);
    error_del(e);
  }

  vec_del(&nodes);
  for (size_t i = 0; i < toks.len; ++i)
    token_del((token_t *)vec_at(&toks, i));
  vec_del(&toks);
  vec_del(&errs);
  interner_del(&intern);
  source_del(&source);

  return 0;
}
