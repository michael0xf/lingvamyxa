#include <stdint.h>

__thread uint_fast8_t *lmx_turn_running;
int lmx_msg_poll_abort(void) {
    return 1;
}
