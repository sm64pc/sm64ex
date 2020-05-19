#ifndef STDLIB_H
#define STDLIB_H

#ifndef TARGET_WEB

typedef struct lldiv_t
{
    long long quot;
    long long rem;
} lldiv_t;

typedef struct ldiv_t
{
    long quot;
    long rem;
} ldiv_t;

lldiv_t lldiv(long long num, long long denom);
ldiv_t ldiv(long num, long denom);

#else

#include <stdlib.h>

#endif

#endif
