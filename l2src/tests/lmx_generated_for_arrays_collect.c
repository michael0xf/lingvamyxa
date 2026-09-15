/* Isolated proof: generated for-host INT/CHAR arrays against current Message. */
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
    Lmx *host;
    LmxArrayDesc *ibuf;
    LmxArrayDesc *cbuf;
    LmxArrayDesc *dead;
    LmxOwnedRange *rg;
    int *icells;
    char *ccells;
    void *iback;
    void *cback;
    void *dead_back;
    int rc;

    rt = lmx_msg_runtime_new();
    if (rt == 0 || lmx_msg_create(rt, 0, 1, 0, 0, &a) != LMX_MSG_OK) {
        fprintf(stderr, "generated for_arrays create\n");
        return 1;
    }
    ma = lmx_msg_find(rt, a);
    if (ma == 0) {
        fprintf(stderr, "generated for_arrays find\n");
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    rc = l2_program_entry(ma);
    if (rc != 145) {
        fprintf(stderr, "generated for_arrays entry rc=%d\n", rc);
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    unit = lmx_msg_graph(ma);
    host = (unit == 0) ? 0 : lmx_branch_struct_known(unit, 1U);
    ibuf = (host == 0) ? 0 : (LmxArrayDesc *)lmx_branch_child_known(host, 1U);
    cbuf = (host == 0) ? 0 : (LmxArrayDesc *)lmx_branch_child_known(host, 2U);
    if (unit == 0 || host == 0 || host == unit
        || ibuf == 0 || cbuf == 0 || ibuf->data == 0 || cbuf->data == 0
        || ibuf->len != 3U || cbuf->len != 4U) {
        fprintf(stderr, "generated for_arrays host graph\n");
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    rg = lmx_owned_ranges_find(ma->ranges, ibuf);
    if (rg == 0 || rg->kind != LMX_KIND_ARRAY || rg->type != LMX_TYPE_ARRAY_OF_INT) {
        fprintf(stderr, "generated for_arrays INT type\n");
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    rg = lmx_owned_ranges_find(ma->ranges, cbuf);
    if (rg == 0 || rg->kind != LMX_KIND_ARRAY || rg->type != LMX_TYPE_ARRAY_OF_CHAR) {
        fprintf(stderr, "generated for_arrays CHAR type\n");
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    icells = (int *)ibuf->data;
    ccells = (char *)cbuf->data;
    iback = ibuf->data;
    cback = cbuf->data;
    if (icells[0] != 10 || icells[1] != 0 || icells[2] != 1
        || (unsigned char)ccells[0] != 65U || ccells[1] != 0 || ccells[2] != 0
        || (unsigned char)ccells[3] != 62U) {
        fprintf(stderr, "generated for_arrays writes INT %d/%d/%d CHAR %d/%d/%d/%d\n",
            icells[0], icells[1], icells[2],
            (int)ccells[0], (int)ccells[1], (int)ccells[2], (int)ccells[3]);
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    dead = lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_INT, 2U, &ma->blocks, &ma->ranges);
    if (dead == 0 || dead->data == 0) {
        fprintf(stderr, "generated for_arrays neighbour\n");
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    dead_back = dead->data;
    if (lmx_msg_end_turn(rt, a, 1) != LMX_MSG_OK) {
        fprintf(stderr, "generated for_arrays end_turn\n");
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    if (lmx_owned_ranges_find(ma->ranges, ibuf) == 0
        || lmx_owned_ranges_find(ma->ranges, iback) == 0
        || lmx_owned_ranges_find(ma->ranges, cbuf) == 0
        || lmx_owned_ranges_find(ma->ranges, cback) == 0
        || icells[0] != 10 || icells[2] != 1
        || (unsigned char)ccells[0] != 65U || (unsigned char)ccells[3] != 62U) {
        fprintf(stderr, "generated for-host arrays dropped\n");
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    if (lmx_owned_ranges_find(ma->ranges, dead) != 0
        || lmx_owned_ranges_find(ma->ranges, dead_back) != 0) {
        fprintf(stderr, "generated for_arrays neighbour immortal\n");
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    lmx_msg_set_graph(ma, 0);
    if (lmx_msg_end_turn(rt, a, 1) != LMX_MSG_OK
        || lmx_owned_ranges_find(ma->ranges, ibuf) != 0
        || lmx_owned_ranges_find(ma->ranges, cbuf) != 0) {
        fprintf(stderr, "generated for_arrays unroot left fields\n");
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    fprintf(stderr, "generated for_arrays: host INT10/0/1 CHAR65/0/0/62 live; neighbour dies; unroot reclaims\n");
    lmx_msg_runtime_delete(rt);
    return 0;
}
