#ifndef LISH_HELPERS_H
#define LISH_HELPERS_H

#define ARRAY_LEN(x) (sizeof(x) / sizeof(*x))
#define LAMBDA(ret_type, _body) ({ ret_type _ _body _; })
#define CLONE(x) (memcpy(malloc(sizeof(*x)), (void *)(x), sizeof(*x)))

#endif