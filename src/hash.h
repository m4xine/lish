#ifndef LISH_HASH_H
#define LISH_HASH_H

#include <stdint.h>

// TODO: Detect bus size and change seed/prime accordingly.
// http://isthe.com/chongo/tech/comp/fnv/
#define FNV1A_SEED 14695981039346656037ULL
#define FNV1A_PRIME 1099511628211ULL

uint64_t 
fnv1a_(uint8_t b, uint64_t hash) 
{   
  return (b ^ hash) * FNV1A_PRIME; 
}

uint64_t 
fnv1a(uint8_t const *p, size_t n) 
{      
  uint64_t hash = FNV1A_SEED;  
  while (n-- > 0)     
    hash = fnv1a_(*p++, hash);   
  return hash; 
}

#endif 