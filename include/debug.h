#ifndef DEBUG_H
#define DEBUG_H

#define empty_body do {} while(0)

//#define NO_BREAKPOINTS
#ifdef NO_BREAKPOINTS
#  define breakpoint() empty_body
#else
#  if ((defined(__X86)) || (defined(__X86_64)) || (defined(__X86__)) || (defined(__X86_64__)))
#    define breakpoint() __asm__("int3")
#  else
#    warning "untested"
#    include <signal.h>
#    include <unistd.h>
#    define breakpoint() kill(getpid(), SIGTRAP)
#  endif
#endif

//#define NO_ASSERT
#ifdef NO_ASSERT
#  define assert_dbg(exp) empty_body
#  define assert_lt(exp) empty_body
#  define assert_lte(exp) empty_body
#  define assert(x) empty_body
#else
#  include <stdio.h>
#  define assert_dbg(exp) do { if (!(exp)) breakpoint(); } while(0)
#  define assert_op(val, op, max) do { if(!((val) op (max))) { \
    fprintf(stderr, "[%s:%d] %s: assert failed: %s < %s\n", \
    __FILE__, __LINE__, __FUNCTION__, # val, # max); \
    breakpoint();}} while(0)

#  define assert_lt (val, max) assert_op(val, <, max)
#  define assert_lte(val, max) assert_op(val, <= ,max)
#endif

#endif