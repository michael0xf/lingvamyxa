/* UAF kit, committed at l2src/tools/q_alloc.c: a poisoning quarantine allocator.
 * USED BY l2src/tools/uaf_run.sh, which rebuilds the executor selftest from a
 * finished run_port_message evidence directory with this allocator linked in --
 * read that script's header for the build and the clean-run criteria.
 * HISTORY: assembled 2026-09-14 for the residual exec-selftest crash; used for S5's
 * green on 2026-09-15 (165 clean runs under load shapes); used on 2026-09-16 on
 * S6-2's heap corruption (0xC0000374 at "live-cascade start"), where it ran 2 of 2
 * clean -- which located nothing, and is exactly why the limits below matter.
 * LIMITS, to read before trusting a clean run:
 *   - freed memory stays MAPPED and poisoned, so a stale READ sees 0xDD rather
 *     than faulting; only a stale WRITE or a double free is reported directly;
 *   - there is NO trailing canary, so an overflow past a block's end is NOT
 *     detected at all;
 *   - every malloc/calloc/realloc/free is serialised behind this file's own lock,
 *     which changes thread timing and can HIDE A RACE entirely.
 * So a clean run rules out stale writes and double frees -- not overflows, and not
 * concurrent mutation. A defect that crashes under the real allocator and vanishes
 * here points at one of those two, not at "no defect".
 * Build every object of the executor selftest with
 *   -Dmalloc=q_malloc -Dcalloc=q_calloc -Drealloc=q_realloc -Dfree=q_free
 * (lmx_msg_history_owned keeps its own malloc/free mapping) and link this file
 * without those -D flags. q_free poisons the payload with 0xDD and keeps the block
 * in a FIFO quarantine (Q_KEEP blocks, Q_CAP bytes) before the real free; the
 * header records the free-site stack. The poison is verified on eviction and in
 * an atexit sweep: a stale READ of a quarantined block reads 0xDD (and usually
 * faults soon after); a stale WRITE prints "STALE WRITE" with the free-site
 * frames as RVAs (resolve: addr2line -f -p -e exe 0x140000000+rva). A pointer
 * without the magic (allocated before the -D build) goes to the real free. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#if defined(_WIN32)
#include <windows.h>
#endif

#define Q_MAGIC 0x51A11C0CULL
#define Q_FRAMES 12
#define Q_KEEP 8192
#define Q_CAP ((size_t)256 * 1024 * 1024)
#define Q_POISON 0xDD

typedef struct QHdr {
    uint64_t magic;
    size_t size;
    int freed;
    unsigned short nframes;
    void *frames[Q_FRAMES];
} QHdr;

static QHdr *q_ring[Q_KEEP];
static size_t q_head;
static size_t q_count;
static size_t q_bytes;
static int q_armed;
#if defined(_WIN32)
static CRITICAL_SECTION q_cs;
static LONG q_cs_init;
#endif

static void q_lock(void) {
#if defined(_WIN32)
    if (InterlockedCompareExchange(&q_cs_init, 1, 0) == 0) {
        InitializeCriticalSection(&q_cs);
        InterlockedExchange(&q_cs_init, 2);
    }
    while (q_cs_init != 2) {
        SwitchToThread();
    }
    EnterCriticalSection(&q_cs);
#endif
}

static void q_unlock(void) {
#if defined(_WIN32)
    LeaveCriticalSection(&q_cs);
#endif
}

static void q_print_frames(QHdr *h) {
#if defined(_WIN32)
    uintptr_t base = (uintptr_t)GetModuleHandleA(0);
    unsigned short k;
    for (k = 0; k < h->nframes; k++) {
        fprintf(stderr, " 0x%llx", (unsigned long long)((uintptr_t)h->frames[k] - base));
    }
#else
    (void)h;
#endif
    fprintf(stderr, "\n");
    fflush(stderr);
}

static int q_poison_intact(QHdr *h) {
    unsigned char *p = (unsigned char *)(h + 1);
    size_t i;
    for (i = 0; i < h->size; i++) {
        if (p[i] != Q_POISON) {
            fprintf(stderr, "STALE WRITE at payload+%llu of a %llu-byte block (byte 0x%02x); free site RVAs:",
                (unsigned long long)i, (unsigned long long)h->size, p[i]);
            q_print_frames(h);
            return 0;
        }
    }
    return 1;
}

static void q_evict_one_locked(void) {
    size_t tail = (q_head + Q_KEEP - q_count) % Q_KEEP;
    QHdr *h = q_ring[tail];
    q_ring[tail] = 0;
    q_count -= 1;
    if (h != 0) {
        q_bytes -= h->size;
        (void)q_poison_intact(h);
        h->magic = 0;
        free(h);
    }
}

static void q_sweep(void) {
    size_t i;
    size_t bad = 0;
    q_lock();
    for (i = 0; i < Q_KEEP; i++) {
        if (q_ring[i] != 0 && q_poison_intact(q_ring[i]) == 0) {
            bad += 1;
        }
    }
    q_unlock();
    fprintf(stderr, "q_alloc sweep: %llu quarantined blocks, %llu stale writes\n",
        (unsigned long long)q_count, (unsigned long long)bad);
    fflush(stderr);
}

void *q_malloc(size_t n) {
    QHdr *h;
    if (n > SIZE_MAX - sizeof(QHdr)) {
        return 0;
    }
    h = (QHdr *)malloc(sizeof(QHdr) + n);
    if (h == 0) {
        return 0;
    }
    h->magic = Q_MAGIC;
    h->size = n;
    h->freed = 0;
    h->nframes = 0;
    if (q_armed == 0) {
        q_armed = 1;
        atexit(q_sweep);
    }
    return (void *)(h + 1);
}

void *q_calloc(size_t count, size_t n) {
    void *p;
    if (n != 0 && count > SIZE_MAX / n) {
        return 0;
    }
    p = q_malloc(count * n);
    if (p != 0) {
        memset(p, 0, count * n);
    }
    return p;
}

void q_free(void *p) {
    QHdr *h;
    if (p == 0) {
        return;
    }
    h = ((QHdr *)p) - 1;
    if (h->magic != Q_MAGIC) {
        free(p);
        return;
    }
    if (h->freed != 0) {
        fprintf(stderr, "DOUBLE FREE of a %llu-byte block; first free site RVAs:", (unsigned long long)h->size);
        q_print_frames(h);
        return;
    }
    h->freed = 1;
#if defined(_WIN32)
    h->nframes = RtlCaptureStackBackTrace(1, Q_FRAMES, h->frames, 0);
#endif
    memset(p, Q_POISON, h->size);
    q_lock();
    while (q_count > 0 && (q_count == Q_KEEP || q_bytes + h->size > Q_CAP)) {
        q_evict_one_locked();
    }
    q_ring[q_head] = h;
    q_head = (q_head + 1) % Q_KEEP;
    q_count += 1;
    q_bytes += h->size;
    q_unlock();
}

void *q_realloc(void *p, size_t n) {
    QHdr *h;
    void *np;
    if (p == 0) {
        return q_malloc(n);
    }
    h = ((QHdr *)p) - 1;
    if (h->magic != Q_MAGIC) {
        return realloc(p, n);
    }
    np = q_malloc(n);
    if (np == 0) {
        return 0;
    }
    memcpy(np, p, h->size < n ? h->size : n);
    q_free(p);
    return np;
}
