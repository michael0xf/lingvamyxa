/* MUST COMPILE: reading is unaffected. */
#include "mixa_manager/tests/l1_gaps/vt.h"
int pos_read(void) { const VT *const *t = reg_tables(0); return t[0]->x; }
