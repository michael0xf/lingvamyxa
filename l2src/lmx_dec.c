#include "lmx_dec.h"
#include <stdio.h>
#include <string.h>

static int digits_ok(const LmxDecCtx *ctx) {
    return ctx != 0 && ctx->set.digits >= 1 && ctx->set.digits <= LMX_DEC_DIGITS;
}

static void op_ctx(const LmxDecCtx *owner, decContext *op) {
    *op = owner->set;
    op->status = 0;
}

static int finish_op(LmxDecCtx *owner, const decContext *op) {
    owner->set.status |= op->status;
    return (op->status & DEC_Errors) != 0U ? 1 : 0;
}

int lmx_dec_ctx_set_digits(LmxDecCtx *ctx, int digits) {
    if (ctx == 0) {
        return 1;
    }
    ctx->set.digits = digits;
    return 0;
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
    decContext op;
    if (out == 0 || text == 0 || !digits_ok(ctx)) {
        return 1;
    }
    op_ctx(ctx, &op);
    decNumberFromString(&out->n, text, &op);
    return finish_op(ctx, &op);
}

int lmx_dec_add(LmxDec *out, const LmxDec *a, const LmxDec *b, LmxDecCtx *ctx) {
    decContext op;
    if (out == 0 || a == 0 || b == 0 || !digits_ok(ctx)) {
        return 1;
    }
    op_ctx(ctx, &op);
    decNumberAdd(&out->n, &a->n, &b->n, &op);
    return finish_op(ctx, &op);
}

int lmx_dec_sub(LmxDec *out, const LmxDec *a, const LmxDec *b, LmxDecCtx *ctx) {
    decContext op;
    if (out == 0 || a == 0 || b == 0 || !digits_ok(ctx)) {
        return 1;
    }
    op_ctx(ctx, &op);
    decNumberSubtract(&out->n, &a->n, &b->n, &op);
    return finish_op(ctx, &op);
}

int lmx_dec_mul(LmxDec *out, const LmxDec *a, const LmxDec *b, LmxDecCtx *ctx) {
    decContext op;
    if (out == 0 || a == 0 || b == 0 || !digits_ok(ctx)) {
        return 1;
    }
    op_ctx(ctx, &op);
    decNumberMultiply(&out->n, &a->n, &b->n, &op);
    return finish_op(ctx, &op);
}

int lmx_dec_div(LmxDec *out, const LmxDec *a, const LmxDec *b, LmxDecCtx *ctx) {
    decContext op;
    if (out == 0 || a == 0 || b == 0 || !digits_ok(ctx)) {
        return 1;
    }
    op_ctx(ctx, &op);
    decNumberDivide(&out->n, &a->n, &b->n, &op);
    return finish_op(ctx, &op);
}

int lmx_dec_neg(LmxDec *out, const LmxDec *a, LmxDecCtx *ctx) {
    decContext op;
    if (out == 0 || a == 0 || !digits_ok(ctx)) {
        return 1;
    }
    op_ctx(ctx, &op);
    decNumberMinus(&out->n, &a->n, &op);
    return finish_op(ctx, &op);
}

int lmx_dec_pow(LmxDec *out, const LmxDec *a, const LmxDec *b, LmxDecCtx *ctx) {
    decContext op;
    if (out == 0 || a == 0 || b == 0 || !digits_ok(ctx)) {
        return 1;
    }
    op_ctx(ctx, &op);
    decNumberPower(&out->n, &a->n, &b->n, &op);
    return finish_op(ctx, &op);
}

int lmx_dec_compare(const LmxDec *a, const LmxDec *b, LmxDecCtx *ctx, int *rel) {
    decContext op;
    decNumber cmp;
    if (a == 0 || b == 0 || rel == 0 || !digits_ok(ctx)) {
        return 1;
    }
    op_ctx(ctx, &op);
    decNumberCompare(&cmp, &a->n, &b->n, &op);
    if (finish_op(ctx, &op) != 0 || decNumberIsNaN(&cmp)) {
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
    decContext op;
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
    op_ctx(ctx, &op);
    op.round = round;
    decNumberFromString(&exp.n, exptext, &op);
    decNumberQuantize(&out->n, &a->n, &exp.n, &op);
    return finish_op(ctx, &op);
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
