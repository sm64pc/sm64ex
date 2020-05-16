#ifndef _OS_LIBC_H_
#define _OS_LIBC_H_

#include "ultratypes.h"

// Old deprecated functions from strings.h, replaced by memcpy/memset.
#ifdef NO_BZERO
#include <string.h>
#define bzero(buf, size) memset(buf, 0, size)
#define bcopy(src, dst, size) memcpy(dst, src, size)
#else
extern void bcopy(const void *, void *, size_t);
extern void bzero(void *, size_t);
#endif

#endif /* !_OS_LIBC_H_ */
