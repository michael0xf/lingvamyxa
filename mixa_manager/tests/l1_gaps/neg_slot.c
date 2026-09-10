/* MUST NOT COMPILE: writing a table slot through the returned type. */
#include "mixa_manager/tests/l1_gaps/vt.h"
static VT one;
void neg_slot(void) { const VT *const *t = reg_tables(0); t[0] = &one; }
