#ifndef _OS_LIBC_H_
#define _OS_LIBC_H_

#include "ultratypes.h"

// Old deprecated functions from strings.h, replaced by memcpy/memset.
extern "C" void bcopy(const void *, void *, size_t);
extern "C" void bzero(void *, size_t);

#endif /* !_OS_LIBC_H_ */
