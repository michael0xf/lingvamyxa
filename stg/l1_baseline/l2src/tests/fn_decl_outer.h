#ifndef L2_TEST_FN_DECL_OUTER_H
#define L2_TEST_FN_DECL_OUTER_H

/* Declares one function itself and reaches another only through its own
   #include -- the way mixa_pump.h reaches mixa_event_fifo_init. The inner header
   includes this one back, under guards, so the walk must stop on the cycle. */

#include "l2src/tests/fn_decl_inner.h"

int l2_test_outer_fn(int x);

#endif
