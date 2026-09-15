/* Focused harness for l2_fmt_l1_string / l2_tok_text dest+cap.
 * Links against l2trans.c with main renamed. No new L2 semantics. */
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "l1src/p0.lm1.h"

int l2_fmt_l1_string(char *dest, size_t cap, const char *data, size_t n);
int l2_tok_text(const LmP0Text *t, char *dest, size_t cap);

static int g_fail;

static void fail(const char *msg)
{
    fprintf(stderr, "fmt_buf fail: %s\n", msg);
    g_fail = 1;
}

static int oob(const char *arena, size_t cap, size_t arena_n)
{
    size_t i;
    for (i = cap; i < arena_n; i++) {
        if ((unsigned char)arena[i] != 0xAAu) {
            return 1;
        }
    }
    return 0;
}

static void expect_fmt_ok(size_t cap, const char *data, size_t n, const char *want, size_t wantn, const char *what)
{
    char arena[64];
    int rc;
    memset(arena, 0xAA, sizeof arena);
    rc = l2_fmt_l1_string(arena, cap, data, n);
    if (rc != 0) {
        fail(what);
        fprintf(stderr, "  rejected exact-fit\n");
        return;
    }
    if (memcmp(arena, want, wantn) != 0) {
        fail(what);
        fprintf(stderr, "  bytes mismatch\n");
        return;
    }
    if (oob(arena, cap, sizeof arena)) {
        fail(what);
        fprintf(stderr, "  wrote past cap\n");
    }
}

static void expect_fmt_overflow(size_t cap, const char *data, size_t n, const char *what)
{
    char arena[64];
    int rc;
    memset(arena, 0xAA, sizeof arena);
    rc = l2_fmt_l1_string(arena, cap, data, n);
    if (rc == 0) {
        fail(what);
        fprintf(stderr, "  overflow accepted\n");
        return;
    }
    if (oob(arena, cap, sizeof arena)) {
        fail(what);
        fprintf(stderr, "  overflow wrote past cap\n");
    }
}

static void tok_from(LmP0Text *t, const char *data, size_t n)
{
    t->data = data;
    t->length = n;
}

static void expect_tok_ok(size_t cap, const char *data, size_t n, const char *want, size_t wantn, const char *what)
{
    char arena[64];
    LmP0Text t;
    int rc;
    tok_from(&t, data, n);
    memset(arena, 0xAA, sizeof arena);
    rc = l2_tok_text(&t, arena, cap);
    if (rc != 0) {
        fail(what);
        fprintf(stderr, "  tok rejected exact-fit\n");
        return;
    }
    if (memcmp(arena, want, wantn) != 0) {
        fail(what);
        fprintf(stderr, "  tok bytes mismatch\n");
        return;
    }
    if (oob(arena, cap, sizeof arena)) {
        fail(what);
        fprintf(stderr, "  tok wrote past cap\n");
    }
}

static void expect_tok_overflow(size_t cap, const char *data, size_t n, int untouched, const char *what)
{
    char arena[64];
    LmP0Text t;
    int rc;
    tok_from(&t, data, n);
    memset(arena, 0xAA, sizeof arena);
    rc = l2_tok_text(&t, arena, cap);
    if (rc == 0) {
        fail(what);
        fprintf(stderr, "  tok overflow accepted\n");
        return;
    }
    if (oob(arena, cap, sizeof arena)) {
        fail(what);
        fprintf(stderr, "  tok overflow wrote past cap\n");
    }
    if (untouched && (unsigned char)arena[0] != 0xAAu) {
        fail(what);
        fprintf(stderr, "  tok overflow wrote dest\n");
    }
}

int main(void)
{
    static const char qbody[] = { 34, 0 };
    static const char bsbody[] = { 92, 0 };
    static const char nlbody[] = { 10, 0 };
    static const char crbody[] = { 13, 0 };
    static const char qmark[] = { 63, 0 };
    static const char nulbody[] = { 0 };
    static const char hibody[] = { (char)255, 0 };
    static const char empty_out[] = { 34, 34, 0 };
    static const char a_out[] = { 34, 97, 34, 0 };
    static const char q_out[] = { 34, 92, 34, 34, 0 };
    static const char bs_out[] = { 34, 92, 92, 34, 0 };
    static const char nl_out[] = { 34, 92, 110, 34, 0 };
    static const char cr_out[] = { 34, 92, 114, 34, 0 };
    static const char qm_out[] = { 34, 92, 63, 34, 0 };
    static const char nul_out[] = { 34, 92, 48, 48, 48, 34, 0 };
    static const char hi_out[] = { 34, 92, 51, 55, 55, 34, 0 };
    static const char dq[] = { 34, 120, 34, 0 };
    static const char sq[] = { 39, 120, 39, 0 };
    static const char trip_dq[] = { 34, 34, 34, 97, 98, 34, 34, 34, 0 };
    static const char trip_sq[] = { 39, 39, 39, 97, 98, 39, 39, 39, 0 };
    static const char trip_empty_dq[] = { 34, 34, 34, 34, 34, 34, 0 };
    static const char trip_empty_sq[] = { 39, 39, 39, 39, 39, 39, 0 };
    static const char trip_nul_dq[] = { 34, 34, 34, 0, 34, 34, 34, 0 };
    static const char trip_hi_dq[] = { 34, 34, 34, (char)255, 34, 34, 34, 0 };
    static const char ab_out[] = { 34, 97, 98, 34, 0 };
    char tiny[2];
    LmP0Text t;

    g_fail = 0;

    if (l2_fmt_l1_string(0, 8, "", 0) == 0) {
        fail("fmt dest null accepted");
    }
    memset(tiny, 0xAA, sizeof tiny);
    if (l2_fmt_l1_string(tiny, 2, "", 0) == 0) {
        fail("fmt cap<3 empty accepted");
    }
    if ((unsigned char)tiny[0] != 0xAAu || (unsigned char)tiny[1] != 0xAAu) {
        fail("fmt cap<3 wrote dest");
    }

    expect_fmt_ok(3, "", 0, empty_out, 3, "fmt empty exact");
    expect_fmt_overflow(2, "", 0, "fmt empty overflow");

    expect_fmt_ok(4, "a", 1, a_out, 4, "fmt a exact");
    expect_fmt_overflow(3, "a", 1, "fmt a overflow");

    expect_fmt_ok(5, qbody, 1, q_out, 5, "fmt quote exact");
    expect_fmt_overflow(4, qbody, 1, "fmt quote overflow");

    expect_fmt_ok(5, bsbody, 1, bs_out, 5, "fmt backslash exact");
    expect_fmt_overflow(4, bsbody, 1, "fmt backslash overflow");

    expect_fmt_ok(5, nlbody, 1, nl_out, 5, "fmt nl exact");
    expect_fmt_overflow(4, nlbody, 1, "fmt nl overflow");

    expect_fmt_ok(5, crbody, 1, cr_out, 5, "fmt cr exact");
    expect_fmt_overflow(4, crbody, 1, "fmt cr overflow");

    expect_fmt_ok(5, qmark, 1, qm_out, 5, "fmt question exact");
    expect_fmt_overflow(4, qmark, 1, "fmt question overflow");

    expect_fmt_ok(7, nulbody, 1, nul_out, 7, "fmt NUL exact");
    expect_fmt_overflow(6, nulbody, 1, "fmt NUL overflow");

    expect_fmt_ok(7, hibody, 1, hi_out, 7, "fmt high exact");
    expect_fmt_overflow(6, hibody, 1, "fmt high overflow");

    tok_from(&t, "x", 1);
    if (l2_tok_text(0, tiny, 8) == 0) {
        fail("tok null text accepted");
    }
    if (l2_tok_text(&t, 0, 8) == 0) {
        fail("tok dest null accepted");
    }
    if (l2_tok_text(&t, tiny, 0) == 0) {
        fail("tok cap 0 accepted");
    }

    expect_tok_ok(2, "x", 1, "x", 2, "tok raw exact");
    expect_tok_overflow(1, "x", 1, 1, "tok raw overflow");

    expect_tok_ok(4, dq, 3, dq, 4, "tok double-quote atom exact");
    expect_tok_overflow(3, dq, 3, 1, "tok double-quote atom overflow");

    expect_tok_ok(4, sq, 3, sq, 4, "tok single-quote atom exact");
    expect_tok_overflow(3, sq, 3, 1, "tok single-quote atom overflow");

    expect_tok_ok(5, trip_dq, 8, ab_out, 5, "tok triple double exact");
    expect_tok_overflow(4, trip_dq, 8, 0, "tok triple double overflow");

    expect_tok_ok(5, trip_sq, 8, ab_out, 5, "tok triple single exact");
    expect_tok_overflow(4, trip_sq, 8, 0, "tok triple single overflow");

    expect_tok_ok(3, trip_empty_dq, 6, empty_out, 3, "tok triple empty double exact");
    expect_tok_overflow(2, trip_empty_dq, 6, 0, "tok triple empty double overflow");

    expect_tok_ok(3, trip_empty_sq, 6, empty_out, 3, "tok triple empty single exact");
    expect_tok_overflow(2, trip_empty_sq, 6, 0, "tok triple empty single overflow");

    expect_tok_ok(7, trip_nul_dq, 7, nul_out, 7, "tok triple NUL exact");
    expect_tok_overflow(6, trip_nul_dq, 7, 0, "tok triple NUL overflow");

    expect_tok_ok(7, trip_hi_dq, 7, hi_out, 7, "tok triple high exact");
    expect_tok_overflow(6, trip_hi_dq, 7, 0, "tok triple high overflow");

    if (g_fail != 0) {
        return 1;
    }
    fprintf(stdout, "fmt_buf ok\n");
    return 0;
}
