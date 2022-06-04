#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include "vec.h"
#include "interner.h"
#include "source.h"
#include "error.h"
#include "lex.h"
#include "parse.h"
#include "interp.h"

int
shell(void)
{
  interner_t in = interner();
  interner_key_t prompt_key = intern(&in, "prompt");

  interp_state_t s = (interp_state_t)
    {
      .debug      = false,
      .show_exec  = false,
      .stack      = vec_(sizeof(stack_entry_t), 32)
    };

  push_cfuns(&s, &in);

  vec_t errs    = vec_null(sizeof(error_t)),
        toks    = vec_null(sizeof(token_t)),
        nodes   = vec_null(sizeof(node_t)),
        history = vec(sizeof(char *));

  while (true)
  {
    value_t *prompt = stack_get(&s, prompt_key);

    if (NULL != prompt)
      printf("%s", prompt->data.str.raw);
    else
    {
      char cwd[PATH_MAX];
      if (NULL != getcwd(cwd, sizeof(cwd)))
        printf("%s> ", cwd);
      else
        printf("no-cwd!> ");
    }

    char *line = NULL;
    size_t len = 0; // ???
    ssize_t line_len = getline(&line, &len, stdin);

    vec_push(&history, &line);

    source_t src = (source_t)
      {
        .origin = "<stdin>",
        .raw    = line,
        .len    = line_len
      };

    lex(&src, &in, &toks, &errs);
    parse(&toks, &src, &errs, &nodes);

    if (errs.len > 0)
    {
      for (size_t i = 0; i < errs.len; ++i)
        error_print((error_t *)vec_at(&errs, i), &src);

      goto SHELL_CLEANUP;
    }

    for (size_t i = 0; i < nodes.len; ++i)
      eval(&s, (node_t *)vec_at(&nodes, i));

    SHELL_CLEANUP:
      for (size_t i = 0; i < nodes.len; ++i)
        node_del((node_t *)vec_at(&nodes, i));
      for (size_t i = 0; i < toks.len; ++i)
        token_del((token_t *)vec_at(&toks, i));

      vec_clear(&toks);
      vec_clear(&nodes);
      vec_clear(&errs);
  }

  for (size_t i = 0; i < history.len; ++i)
    free(*(char **)vec_at(&history, i));

  return 0;
}

int 
main(int argc, char **argv) 
{
  // TODO: Temporary!

  if (1 == argc) return shell();

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

  /*
  printf("\nTokens:\n");
  for (size_t i = 0; i < toks.len; ++i)
  {
    token_t *tok = (token_t *)vec_at(&toks, i);
    token_print(tok);
  }
  */

  vec_t nodes = vec_null(sizeof(node_t));
  parse(&toks, &source, &errs, &nodes);
  
  /*
  printf("\nNodes:\n");
  for (size_t i = 0; i < nodes.len; ++i)
  {
    node_t *n = (node_t *)vec_at(&nodes, i);
    node_print(n);
  }
  */

  for (size_t i = 0; i < errs.len; ++i)
  {
    error_t *e = (error_t *)vec_at(&errs, i);
    error_print(e, &source);
    error_del(e);
  }

  interp(&nodes, &intern);

  for (size_t i = 0; i < nodes.len; ++i)
    node_del((node_t *)vec_at(&nodes, i));
  vec_del(&nodes);
  for (size_t i = 0; i < toks.len; ++i)
    token_del((token_t *)vec_at(&toks, i));
  vec_del(&toks);
  vec_del(&errs);
  interner_del(&intern);
  source_del(&source);

  return 0;
}
