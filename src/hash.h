#ifndef LISH_HASH_H
#define LISH_HASH_H

#include <stdint.h>

// TODO: Detect bus size and change seed/prime accordingly.
#define FNV1A_SEED 14695981039346656037
#define FNV1A_PRIME 1099511628211

uint64_t 
fnv1a_(uint8_t b) 
{   
  return (b ^ FNV1A_SEED) * FNV1A_PRIME; 
}

uint64_t 
fnv1a(void const *p, size_t n) 
{      
  uint8_t const *p = (uint8_t const *)p;   
  while (n-- > 0)     
    hash = fnv1a_(*p++, hash);   
  return hash; 
}

#endif 