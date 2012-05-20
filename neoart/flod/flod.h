#ifndef FLOD_H
#define FLOD_H

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include "../../flashlib/Common.h"
#include "../../flashlib/ByteArray.h"
#include "../../flashlib/Encoding.h"
#include "../../flashlib/EventDispatcher.h"
#include "../../flashlib/SampleDataEvent.h"

#define PFUNC() fprintf(stderr, "%s\n", __FUNCTION__)
#define assert_dbg(exp) do { if (!(exp)) __asm__("int3"); } while(0)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#endif