#include "lmx_dec.h"
#include <stdio.h>
#include <string.h>

static int digits_ok(const LmxDecCtx *ctx) {
    return ctx != 0 && ctx->set.digits >= 1 && ctx->set.digits <= LMX_DEC_DIGITS;
}

unsigned lmx_dec_ctx_new_errors(unsigned before, unsigned after) {
    return (after & ~before) & DEC_Errors;
}

static int finish(LmxDecCtx *ctx, unsigned before) {
    return lmx_dec_ctx_new_errors(before, ctx->set.status) != 0U ? 1 : 0;
}

int lmx_dec_digits(void) {
    return LMX_DEC_DIGITS;
}

int lmx_dec_ctx_init(LmxDecCtx *ctx) {
    if (ctx == 0) {
        return 1;
    }
    if (decContextTestEndian(1) != 0) {
        return 1;
    }
    decContextDefault(&ctx->set, DEC_INIT_BASE);
    ctx->set.traps = 0;
    ctx->set.digits = LMX_DEC_DIGITS;
    ctx->set.round = DEC_ROUND_HALF_EVEN;
    ctx->set.status = 0;
    return 0;
}

unsigned lmx_dec_ctx_status(const LmxDecCtx *ctx) {
    if (ctx == 0) {
        return 0U;
    }
    return ctx->set.status;
}

void lmx_dec_ctx_clear(LmxDecCtx *ctx) {
    if (ctx != 0) {
        ctx->set.status = 0;
    }
}

int lmx_dec_from_text(LmxDec *out, const char *text, LmxDecCtx *ctx) {
    unsigned before;
    if (out == 0 || text == 0 || !digits_ok(ctx)) {
        return 1;
    }
    before = ctx->set.status;
    decNumberFromString(&out->n, text, &ctx->set);
    return finish(ctx, before);
}

int lmx_dec_add(LmxDec *out, const LmxDec *a, const LmxDec *b, LmxDecCtx *ctx) {
    unsigned before;
    if (out == 0 || a == 0 || b == 0 || !digits_ok(ctx)) {
        return 1;
    }
    before = ctx->set.status;
    decNumberAdd(&out->n, &a->n, &b->n, &ctx->set);
    return finish(ctx, before);
}

int lmx_dec_sub(LmxDec *out, const LmxDec *a, const LmxDec *b, LmxDecCtx *ctx) {
    unsigned before;
    if (out == 0 || a == 0 || b == 0 || !digits_ok(ctx)) {
        return 1;
    }
    before = ctx->set.status;
    decNumberSubtract(&out->n, &a->n, &b->n, &ctx->set);
    return finish(ctx, before);
}

int lmx_dec_mul(LmxDec *out, const LmxDec *a, const LmxDec *b, LmxDecCtx *ctx) {
    unsigned before;
    if (out == 0 || a == 0 || b == 0 || !digits_ok(ctx)) {
        return 1;
    }
    before = ctx->set.status;
    decNumberMultiply(&out->n, &a->n, &b->n, &ctx->set);
    return finish(ctx, before);
}

int lmx_dec_div(LmxDec *out, const LmxDec *a, const LmxDec *b, LmxDecCtx *ctx) {
    unsigned before;
    if (out == 0 || a == 0 || b == 0 || !digits_ok(ctx)) {
        return 1;
    }
    before = ctx->set.status;
    decNumberDivide(&out->n, &a->n, &b->n, &ctx->set);
    return finish(ctx, before);
}

int lmx_dec_neg(LmxDec *out, const LmxDec *a, LmxDecCtx *ctx) {
    unsigned before;
    if (out == 0 || a == 0 || !digits_ok(ctx)) {
        return 1;
    }
    before = ctx->set.status;
    decNumberMinus(&out->n, &a->n, &ctx->set);
    return finish(ctx, before);
}

int lmx_dec_pow(LmxDec *out, const LmxDec *a, const LmxDec *b, LmxDecCtx *ctx) {
    unsigned before;
    if (out == 0 || a == 0 || b == 0 || !digits_ok(ctx)) {
        return 1;
    }
    before = ctx->set.status;
    decNumberPower(&out->n, &a->n, &b->n, &ctx->set);
    return finish(ctx, before);
}

int lmx_dec_compare(const LmxDec *a, const LmxDec *b, LmxDecCtx *ctx, int *rel) {
    unsigned before;
    decNumber cmp;
    if (a == 0 || b == 0 || rel == 0 || !digits_ok(ctx)) {
        return 1;
    }
    before = ctx->set.status;
    decNumberCompare(&cmp, &a->n, &b->n, &ctx->set);
    if (finish(ctx, before) != 0 || decNumberIsNaN(&cmp)) {
        return 1;
    }
    if (decNumberIsZero(&cmp)) {
        *rel = 0;
    } else if (decNumberIsNegative(&cmp)) {
        *rel = -1;
    } else {
        *rel = 1;
    }
    return 0;
}

int lmx_dec_quantize_places(LmxDec *out, const LmxDec *a, int places, enum rounding round, LmxDecCtx *ctx) {
    unsigned before;
    decContext tmp;
    LmxDec exp;
    char exptext[24];
    if (out == 0 || a == 0 || !digits_ok(ctx) || places < 0 || places > LMX_DEC_DIGITS || round >= DEC_ROUND_MAX) {
        return 1;
    }
    if (places == 0) {
        memcpy(exptext, "1", 2);
    } else if (sprintf(exptext, "1E-%d", places) < 0) {
        return 1;
    }
    tmp = ctx->set;
    tmp.round = round;
    before = ctx->set.status;
    decNumberFromString(&exp.n, exptext, &tmp);
    decNumberQuantize(&out->n, &a->n, &exp.n, &tmp);
    ctx->set.status = tmp.status;
    return finish(ctx, before);
}

int lmx_dec_to_text(const LmxDec *a, char *buf, size_t n) {
    char tmp[LMX_DEC_TEXT_MIN];
    size_t k;
    if (a == 0 || buf == 0) {
        return 1;
    }
    decNumberToString(&a->n, tmp);
    k = strlen(tmp);
    if (k + 1U > n) {
        return 1;
    }
    memcpy(buf, tmp, k + 1U);
    return 0;
}
