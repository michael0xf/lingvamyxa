#ifndef L2_TEST_STRUCT_TYPEDEF_OUTER_H
#define L2_TEST_STRUCT_TYPEDEF_OUTER_H

/* Declares one handle itself and reaches another through its own #include --
   the way mixa_pump.h reaches MixaBackend and mixa_process_win32.h reaches
   MixaProcess. The inner header includes this one back, under guards, so a
   header walk that follows #include lines has to terminate on a cycle. */

#include "l2src/tests/struct_typedef_inner.h"

typedef struct L2TestOuter L2TestOuter;

#endif
