/* Concurrent producers + in-flight shutdown for LMX_MSG_HOST_INGRESS_V0. */
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

typedef struct ProdArg {
    unsigned id;
    unsigned posted_ok;
    unsigned posted_stopped;
    unsigned unexpected;
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
    for (seq = 0; seq < 20000U; seq++) {
        payload = pack(p->id, seq);
        env.id = 0;
        st = lmx_msg_host_post(g_rt, g_dest, &env);
        payload = 0xFFFFFFFFu;
        if (st == LMX_MSG_STAGED) {
            p->posted_ok += 1;
        } else if (st == LMX_MSG_STOPPED) {
            p->posted_stopped += 1;
            break;
        } else {
            p->unexpected += 1;
            break;
        }
    }
#if defined(_WIN32)
    return 0;
#else
    return 0;
#endif
}

#if defined(_WIN32)
static DWORD WINAPI off_owner(void *arg)
#else
static void *off_owner(void *arg)
#endif
{
    int *fail = (int *)arg;
    LmxMsgAddr a = 0;
    LmxMsgEnv env;
    unsigned seg = 0;
    uchar b = 1;
    memset(&env, 0, sizeof(env));
    if (lmx_msg_create(g_rt, 0, 99, &b, 1, &a) != LMX_MSG_INVALID) {
        *fail += 1;
    }
    if (lmx_msg_send(g_rt, 1, 1, &env) != LMX_MSG_INVALID) {
        *fail += 1;
    }
    if (lmx_msg_pump(g_rt) != LMX_MSG_INVALID) {
        *fail += 1;
    }
    if (lmx_msg_recv(g_rt, 1, &env) != LMX_MSG_INVALID) {
        *fail += 1;
    }
    if (lmx_msg_host_drain(g_rt) != LMX_MSG_INVALID) {
        *fail += 1;
    }
    if (lmx_msg_path_n(g_rt, 1) != -1) {
        *fail += 1;
    }
    if (lmx_msg_path_seg(g_rt, 1, 0, &seg) != LMX_MSG_INVALID) {
        *fail += 1;
    }
    if (lmx_msg_init_copy(g_rt, 1, &env) != LMX_MSG_INVALID) {
        *fail += 1;
    }
    if (lmx_msg_host_wait(g_rt, 0) != LMX_MSG_INVALID) {
        *fail += 1;
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
    int owner_fail = 0;
    int off_fail = 0;
    unsigned last[NPROD];
    unsigned seen[NPROD];
    unsigned total = 0;
    unsigned staged = 0;
    unsigned stopped = 0;
    int i;
    ProdArg args[NPROD];
    FILE *ev;
#if defined(_WIN32)
    HANDLE th[NPROD];
    HANDLE off;
#else
    pthread_t th[NPROD];
    pthread_t off;
#endif

    init[0] = 1;
    memset(&got, 0, sizeof(got));
    memset(last, 0xff, sizeof(last));
    memset(seen, 0, sizeof(seen));
    memset(args, 0, sizeof(args));
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

#if defined(_WIN32)
    off = CreateThread(0, 0, off_owner, &off_fail, 0, 0);
    if (off == 0) {
        return 1;
    }
    WaitForSingleObject(off, INFINITE);
    CloseHandle(off);
#else
    if (pthread_create(&off, 0, off_owner, &off_fail) != 0) {
        return 1;
    }
    pthread_join(off, 0);
#endif
    if (off_fail != 0) {
        fprintf(stderr, "off-owner rejections failed %d\n", off_fail);
        owner_fail = 1;
    }

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

    /* Drain while producers still run, then shutdown with callbacks in flight. */
#if defined(_WIN32)
    Sleep(5);
#else
    { struct timespec ts; ts.tv_sec = 0; ts.tv_nsec = 5000000L; nanosleep(&ts, 0); }
#endif
    st = lmx_msg_host_wait(g_rt, 20);
    if (st != LMX_MSG_OK && st != LMX_MSG_EMPTY) {
        fprintf(stderr, "wait status %d\n", st);
        owner_fail = 1;
    }
    lmx_msg_host_drain(g_rt);
    st = lmx_msg_runtime_shutdown(g_rt);
    if (st != LMX_MSG_OK) {
        owner_fail = 1;
    }
    for (i = 0; i < NPROD; i++) {
#if defined(_WIN32)
        WaitForSingleObject(th[i], INFINITE);
        CloseHandle(th[i]);
#else
        pthread_join(th[i], 0);
#endif
        staged += args[i].posted_ok;
        stopped += args[i].posted_stopped;
        if (args[i].unexpected != 0) {
            fprintf(stderr, "producer %u unexpected %u\n", args[i].id, args[i].unexpected);
            owner_fail = 1;
        }
    }
    lmx_msg_host_drain(g_rt);
    while (lmx_msg_recv(g_rt, child, &got) == LMX_MSG_OK) {
        unsigned v;
        unsigned prod;
        unsigned seq;
        if (got.n != sizeof(unsigned) || got.bytes == 0) {
            owner_fail = 1;
            lmx_msg_env_release(&got);
            break;
        }
        memcpy(&v, got.bytes, sizeof(v));
        prod = v >> 16;
        seq = v & 0xFFFFu;
        if (prod >= NPROD) {
            owner_fail = 1;
        } else if (last[prod] != 0xFFFFFFFFu && seq <= last[prod]) {
            fprintf(stderr, "producer %u fifo broke seq %u after %u\n", prod, seq, last[prod]);
            owner_fail = 1;
        } else {
            last[prod] = seq;
            seen[prod] += 1;
        }
        total += 1;
        lmx_msg_env_release(&got);
    }
    memset(&got, 0, sizeof(got));
    got.kind = LMX_MSG_KIND_NUMBER;
    st = lmx_msg_host_post(g_rt, child, &got);
    if (st != LMX_MSG_STOPPED) {
        fprintf(stderr, "late post after shutdown+join %d\n", st);
        owner_fail = 1;
    }
    /* rt is still valid here; delete only after the join barrier above. */
    lmx_msg_runtime_delete(g_rt);
    ev = fopen("build/l1trans/logs/gen2/lmx_message_host_selftest.evidence.txt", "w");
    if (ev != 0) {
        fprintf(ev, "host_selftest win32=%d posix_written_untested_here=1\n",
#if defined(_WIN32)
            1
#else
            0
#endif
        );
        fprintf(ev, "recv_total=%u staged=%u stopped=%u off_fail=%d owner_fail=%d\n",
            total, staged, stopped, off_fail, owner_fail);
        for (i = 0; i < NPROD; i++) {
            fprintf(ev, "prod %u ok=%u stopped=%u unexpected=%u seen=%u last=%u\n",
                args[i].id, args[i].posted_ok, args[i].posted_stopped,
                args[i].unexpected, seen[i], last[i]);
        }
        fclose(ev);
    }
    printf("lmx_message_host ok recv=%u staged=%u stopped=%u off_fail=%d\n",
        total, staged, stopped, off_fail);
    if (owner_fail != 0 || off_fail != 0) {
        return 1;
    }
    if (total == 0 || staged == 0) {
        fprintf(stderr, "no staged/recv work\n");
        return 1;
    }
    if (stopped == 0) {
        fprintf(stderr, "shutdown did not overlap in-flight posts\n");
        return 1;
    }
    if (total > staged) {
        fprintf(stderr, "recv %u > staged %u\n", total, staged);
        return 1;
    }
    return 0;
}
