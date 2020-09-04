#ifndef _ULTRA64_TYPES_H_
#define _ULTRA64_TYPES_H_

#ifndef NULL
#define NULL    (void *)0
#endif

#define TRUE 1
#define FALSE 0

typedef signed char            s8;
typedef unsigned char          u8;
typedef signed short int       s16;
typedef unsigned short int     u16;
typedef signed int             s32;
typedef unsigned int           u32;
typedef signed long long int   s64;
typedef unsigned long long int u64;

typedef volatile u8   vu8;
typedef volatile u16 vu16;
typedef volatile u32 vu32;
typedef volatile u64 vu64;
typedef volatile s8   vs8;
typedef volatile s16 vs16;
typedef volatile s32 vs32;
typedef volatile s64 vs64;

typedef float  f32;
typedef double f64;

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#if defined(__MINGW32__) 
#include <_mingw.h>
#if !defined(__MINGW64_VERSION_MAJOR)
typedef long ssize_t;
#else
typedef ptrdiff_t ssize_t;
#endif
#endif

#endif // _ULTRA64_TYPES_H_
