#ifndef _MACROS_H_
#define _MACROS_H_

#include "platform_info.h"

#ifndef __sgi
#define GLOBAL_ASM(...)
#endif

#if !defined(__sgi) && (!defined(NON_MATCHING) || !defined(AVOID_UB))
// asm-process isn't supported outside of IDO, and undefined behavior causes
// crashes.
#error Matching build is only possible on IDO; please build with NON_MATCHING=1.
#endif

#define ARRAY_COUNT(arr) (s32)(sizeof(arr) / sizeof(arr[0]))

#define GLUE(a, b) a ## b
#define GLUE2(a, b) GLUE(a, b)

// Avoid compiler warnings for unused variables
#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

// Static assertions
#ifdef __GNUC__
#define STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#else
#define STATIC_ASSERT(cond, msg) typedef char GLUE2(static_assertion_failed, __LINE__)[(cond) ? 1 : -1]
#endif

// Align to 8-byte boundary for DMA requirements
#ifdef __GNUC__
#define ALIGNED8 __attribute__((aligned(8)))
#else
#define ALIGNED8
#endif

// Align to 16-byte boundary for audio lib requirements
#ifdef __GNUC__
#define ALIGNED16 __attribute__((aligned(16)))
#else
#define ALIGNED16
#endif

// no conversion for pc port other than cast
#define VIRTUAL_TO_PHYSICAL(addr)   ((uintptr_t)(addr))
#define PHYSICAL_TO_VIRTUAL(addr)   ((uintptr_t)(addr))
#define VIRTUAL_TO_PHYSICAL2(addr)  ((void *)(addr))

// Byteswap macros
#define BSWAP16(x) (((x) & 0xFF) << 8 | (((x) >> 8) & 0xFF))
#define BSWAP32(x) \
    ( (((x) >> 24) & 0x000000FF) | (((x) >>  8) & 0x0000FF00) | \
      (((x) <<  8) & 0x00FF0000) | (((x) << 24) & 0xFF000000) )

// Convenience macros for endian conversions
#if IS_BIG_ENDIAN
# define BE_TO_HOST16(x) (x)
# define BE_TO_HOST32(x) (x)
# define LE_TO_HOST16(x) BSWAP16(x)
# define LE_TO_HOST32(x) BSWAP32(x)
#else
# define BE_TO_HOST16(x) BSWAP16(x)
# define BE_TO_HOST32(x) BSWAP32(x)
# define LE_TO_HOST16(x) (x)
# define LE_TO_HOST32(x) (x)
#endif

#endif
