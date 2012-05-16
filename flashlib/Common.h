#ifndef COMMON_H
#define COMMON_H

#include <math.h>
#include <stddef.h>

#define false (0)
#define true (1)
#define null NULL

typedef float Number;

#include <stdio.h>
#define DEBUGP(format, ...) fprintf(stderr, format, __VA_ARGS__)
#define INT3 __asm__("int3")

#endif