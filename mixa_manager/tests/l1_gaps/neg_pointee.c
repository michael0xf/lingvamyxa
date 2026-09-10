/* MUST NOT COMPILE: writing through a table pointer. */
#include "mixa_manager/tests/l1_gaps/vt.h"
void neg_pointee(void) { const VT *const *t = reg_tables(0); t[0]->x = 5; }
