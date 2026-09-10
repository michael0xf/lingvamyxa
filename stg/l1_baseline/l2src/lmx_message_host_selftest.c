/* Concurrent producers for LMX_MSG_HOST_INGRESS_V0. Owner drains. */
#include "l2src/lmx_message.h"
#include "l2src/lmx_message_host.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <pthread.h>
#endif

#define NPROD 4
#define NPOST 40

static LmxMsgRuntime *g_rt;
static LmxMsgAddr g_dest;
static volatile int g_fail;

typedef struct ProdArg {
    unsigned id;
} ProdArg;

static unsigned pack(unsigned prod, unsigned seq) {
    return (prod << 16) | seq;
}

#if defined(_WIN32)
static DWORD WINAPI producer(void *arg)
#else
static void *producer(void *arg)
#endif
{
    ProdArg *p = (ProdArg *)arg;
    LmxMsgEnv env;
    unsigned seq;
    int st;
    unsigned payload;
    memset(&env, 0, sizeof(env));
    env.kind = LMX_MSG_KIND_BYTES;
    env.n = sizeof(payload);
    env.bytes = (const uchar *)&payload;
    for (seq = 0; seq < NPOST; seq++) {
        payload = pack(p->id, seq);
        env.id = 0;
        st = lmx_msg_host_post(g_rt, g_dest, &env);
        if (st != LMX_MSG_STAGED && st != LMX_MSG_STOPPED) {
            g_fail = 1;
        }
        payload = 0xFFFFFFFFu;
    }
#if defined(_WIN32)
    return 0;
#else
    return 0;
#endif
}

int main(void) {
    LmxMsgAddr parent = 0;
    LmxMsgAddr child = 0;
    LmxMsgEnv got;
    uchar init[1];
    int st;
    int n;
    unsigned last[NPROD];
    unsigned seen[NPROD];
    unsigned total = 0;
    int i;
    ProdArg args[NPROD];
#if defined(_WIN32)
    HANDLE th[NPROD];
#else
    pthread_t th[NPROD];
#endif

    init[0] = 1;
    memset(&got, 0, sizeof(got));
    memset(last, 0xff, sizeof(last));
    memset(seen, 0, sizeof(seen));
    g_rt = lmx_msg_runtime_new();
    if (g_rt == 0) {
        return 1;
    }
    st = lmx_msg_create(g_rt, 0, 1, init, 1, &parent);
    if (st != LMX_MSG_OK) {
        return 1;
    }
    st = lmx_msg_create(g_rt, parent, 2, init, 1, &child);
    st = lmx_msg_end_turn(g_rt, parent, 1);
    g_dest = child;
    for (i = 0; i < NPROD; i++) {
        args[i].id = (unsigned)i;
#if defined(_WIN32)
        th[i] = CreateThread(0, 0, producer, &args[i], 0, 0);
        if (th[i] == 0) {
            return 1;
        }
#else
        if (pthread_create(&th[i], 0, producer, &args[i]) != 0) {
            return 1;
        }
#endif
    }
    n = 0;
    while (total < (unsigned)(NPROD * NPOST) && n < 10000) {
        lmx_msg_host_wait(g_rt, 20);
        lmx_msg_host_drain(g_rt);
        while (lmx_msg_recv(g_rt, child, &got) == LMX_MSG_OK) {
            unsigned v;
            unsigned prod;
            unsigned seq;
            if (got.n != sizeof(unsigned) || got.bytes == 0) {
                g_fail = 1;
                lmx_msg_env_release(&got);
                break;
            }
            memcpy(&v, got.bytes, sizeof(v));
            prod = v >> 16;
            seq = v & 0xFFFFu;
            if (prod >= NPROD) {
                g_fail = 1;
            } else {
                if (last[prod] != 0xFFFFFFFFu && seq <= last[prod]) {
                    fprintf(stderr, "producer %u fifo broke seq %u after %u\n", prod, seq, last[prod]);
                    g_fail = 1;
                }
                last[prod] = seq;
                seen[prod] += 1;
            }
            total += 1;
            lmx_msg_env_release(&got);
        }
        n += 1;
    }
    for (i = 0; i < NPROD; i++) {
#if defined(_WIN32)
        WaitForSingleObject(th[i], INFINITE);
        CloseHandle(th[i]);
#else
        pthread_join(th[i], 0);
#endif
    }
    lmx_msg_host_drain(g_rt);
    while (lmx_msg_recv(g_rt, child, &got) == LMX_MSG_OK) {
        total += 1;
        lmx_msg_env_release(&got);
    }
    if (total != (unsigned)(NPROD * NPOST)) {
        fprintf(stderr, "host concurrent total %u want %u\n", total, (unsigned)(NPROD * NPOST));
        g_fail = 1;
    }
    st = lmx_msg_runtime_shutdown(g_rt);
    if (st != LMX_MSG_OK) {
        g_fail = 1;
    }
    memset(&got, 0, sizeof(got));
    got.kind = LMX_MSG_KIND_NUMBER;
    st = lmx_msg_host_post(g_rt, child, &got);
    if (st != LMX_MSG_STOPPED) {
        fprintf(stderr, "post after shutdown %d\n", st);
        g_fail = 1;
    }
    lmx_msg_runtime_delete(g_rt);
    if (g_fail != 0) {
        return 1;
    }
    printf("lmx_message_host ok total=%u\n", total);
    return 0;
}
