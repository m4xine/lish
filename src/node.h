#ifndef LISH_NODE_H
#define LISH_NODE_H

#include <stdint.h>
#include <stdio.h>
#include "vec.h"
#include "string.h"

typedef uint8_t node_kind_t;
enum 
{
  NODE_NAME,
  NODE_STR,
  NODE_KW,
  NODE_LIST,
  NODE_IF,
  NODE_DEFUN,
  NODE_DEVAL,
};


char const *
node_kind_str(node_kind_t k)
{
  switch (k)
  {
    case NODE_NAME:   return "Name";
    case NODE_STR:    return "String";
    case NODE_KW:     return "Keyword";
    case NODE_LIST:   return "List";
    case NODE_IF:     return "If";
    case NODE_DEFUN:  return "Defun";
    case NODE_DEVAL:  return "Deval";
  }
}

typedef struct node_t node_t;
struct node_t
{
  node_kind_t kind;
  char const *begin, *end;
  union
  {
    // NODE_NAME
    interner_key_t key;
  
    // NODE_STR
    string_t str;

    // NODE_KW
    kw_t kw;

    // NODE_LIST
    vec_t list;

    // NODE_IF
    struct 
    {
      node_t  *cond,
              *true_branch,
              *opt_false_branch;
    } if_;

    // NODE_DEFUN
    struct 
    { 
      interner_key_t name;
      vec_t params;
      node_t *body;
    } defun;

    // NODE_DEVAL
    struct 
    { 
      interner_key_t name;
      node_t *value;
    } deval;
  } data;
};

void
node_print_(node_t const *n, int depth)
{ 
  printf("%*s", depth, "");
  switch (n->kind)
  {
    case NODE_NAME: 
      printf(
        "Name: %.*s (%zu)\n", 
        (int)(n->end - n->begin), 
        n->begin,
        n->data.key
      ); 
      break;
    case NODE_STR:
      printf("String: %s\n", n->data.str.raw); 
      break;
    case NODE_KW:
      printf("Keyword: %s\n", kw_str(n->data.key));
      break;
    case NODE_LIST: 
      printf("List\n"); 
      vec_t children = n->data.list;
      for (size_t i = 0; i < children.len; ++i)
        node_print_((node_t const *)vec_at(&children, i), depth + 2);
      break;
    case NODE_IF:
      printf("If:\n");
      node_print_(n->data.if_.true_branch, depth + 2);
      if (NULL != n->data.if_.opt_false_branch)
        node_print_(n->data.if_.opt_false_branch, depth + 2);
      break;
    case NODE_DEFUN:
      printf("Defun %zu (", n->data.defun.name);
      for (size_t i = 0; i < n->data.defun.params.len; ++i)
        printf("%zu,", *(interner_key_t const *)vec_at((vec_t *)&n->data.defun.params, i));
      printf(")\n");
      node_print_(n->data.defun.body, depth + 2);
      break;
    case NODE_DEVAL:
      printf("Deval %zu:\n", n->data.deval.name);
      node_print_(n->data.deval.value, depth + 2);
      break;
  }
}

void
node_print(node_t const *n)
{
  node_print_(n, 0);
}

void
node_del(node_t *n)
{
  switch (n->kind)
  {
    case NODE_LIST:
      vec_del(&n->data.list);
      break;
    case NODE_STR:
      string_del(&n->data.str);
      break;
  }
  memset(n, 0, sizeof(node_t));
}

#endif 