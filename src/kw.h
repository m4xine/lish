#ifndef LISH_KW_H
#define LISH_KW_H 

typedef uint8_t kw_t;
enum
{
  KW_IF,
  KW_DEFUN,
  KW_DEVAL
};

char const *KEYWORDS[] =
  {
    [KW_IF]    = "if",
    [KW_DEFUN] = "defun",
    [KW_DEVAL] = "deval"
  };

char const *
kw_str(kw_t kw)
{
  switch (kw)
  {
    case KW_IF:    return "if";
    case KW_DEFUN: return "defun";
    case KW_DEVAL: return "deval";
  }

  return NULL;
}

#endif  