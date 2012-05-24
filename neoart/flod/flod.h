#ifndef FLOD_H
#define FLOD_H

#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define NO_ASSERT
#ifndef NO_ASSERT
#include <assert.h>
#else
#define assert(x) do {} while(0)
#endif

#include "../../flashlib/Common.h"
#include "../../flashlib/ByteArray.h"

#define PFUNC_QUIET
#ifndef PFUNC_QUIET
#define PFUNC() fprintf(stderr, "%s\n", __FUNCTION__)
#else
#define PFUNC() do { } while(0)
#endif

#define assert_dbg(exp) do { if (!(exp)) __asm__("int3"); } while(0)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#endif