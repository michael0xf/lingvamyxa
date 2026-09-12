/* Isolated proof: generated unit_own_array_char_index L1 against current Message. */
#include "l2src/lmx.h"
#include "l2src/lmx_message.h"
#include "l2src/lmx_array_owned.lm1.h"
#include "l2src/lmx_branch_owned.lm1.h"
#include <stdio.h>
#include <string.h>

int l2_program_entry(LmxMsg *process_message);

void lmx_msg_test_on_admit(LmxMsgAddr dest, const LmxMsgCopy *fresh)
{
    (void)dest;
    (void)fresh;
}

int main(void)
{
    LmxMsgRuntime *rt;
    LmxMsgAddr a = 0;
    LmxMsg *ma;
    Lmx *unit;
    LmxArrayDesc *buf;
    LmxArrayDesc *dead;
    LmxOwnedRange *rg;
    char *cells;
    void *buf_back;
    void *dead_back;
    int rc;

    rt = lmx_msg_runtime_new();
    if (rt == 0 || lmx_msg_create(rt, 0, 1, 0, 0, &a) != LMX_MSG_OK) {
        fprintf(stderr, "generated char_index create\n");
        return 1;
    }
    ma = lmx_msg_find(rt, a);
    if (ma == 0) {
        fprintf(stderr, "generated char_index find\n");
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    rc = l2_program_entry(ma);
    if (rc != 127) {
        fprintf(stderr, "generated char_index entry rc=%d\n", rc);
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    unit = lmx_msg_graph(ma);
    buf = (unit == 0) ? 0 : (LmxArrayDesc *)lmx_branch_child_known(unit, 0U);
    if (unit == 0 || buf == 0 || buf->data == 0 || buf->len != 3U) {
        fprintf(stderr, "generated char_index graph field\n");
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    rg = lmx_owned_ranges_find(ma->ranges, buf);
    if (rg == 0 || rg->kind != LMX_KIND_ARRAY || rg->type != LMX_TYPE_ARRAY_OF_CHAR) {
        fprintf(stderr, "generated char_index descriptor type\n");
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    cells = (char *)buf->data;
    buf_back = buf->data;
    if ((unsigned char)cells[0] != 65U || cells[1] != 0 || (unsigned char)cells[2] != 62U) {
        fprintf(stderr, "generated CHAR writes %d/%d/%d\n", (int)cells[0], (int)cells[1], (int)cells[2]);
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    dead = lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_INT, 2U, &ma->blocks, &ma->ranges);
    if (dead == 0 || dead->data == 0) {
        fprintf(stderr, "generated char neighbour new\n");
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    dead_back = dead->data;
    if (lmx_msg_end_turn(rt, a, 1) != LMX_MSG_OK) {
        fprintf(stderr, "generated char_index end_turn\n");
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    if (lmx_owned_ranges_find(ma->ranges, buf) == 0
        || lmx_owned_ranges_find(ma->ranges, buf_back) == 0
        || (unsigned char)cells[0] != 65U || cells[1] != 0 || (unsigned char)cells[2] != 62U) {
        fprintf(stderr, "generated rooted 65/62 dropped\n");
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    if (lmx_owned_ranges_find(ma->ranges, dead) != 0
        || lmx_owned_ranges_find(ma->ranges, dead_back) != 0) {
        fprintf(stderr, "generated char neighbour immortal\n");
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    lmx_msg_set_graph(ma, 0);
    if (lmx_msg_end_turn(rt, a, 1) != LMX_MSG_OK
        || lmx_owned_ranges_find(ma->ranges, buf) != 0
        || lmx_owned_ranges_find(ma->ranges, buf_back) != 0) {
        fprintf(stderr, "generated char unroot left field\n");
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    fprintf(stderr, "generated char_index: writes 65/0/62; end_turn keeps; neighbour dies; unroot reclaims\n");
    lmx_msg_runtime_delete(rt);
    return 0;
}
