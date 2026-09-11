#include "lmx_dec.h"
#include <string.h>

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
    if (out == 0 || text == 0 || ctx == 0) {
        return 1;
    }
    decNumberFromString(&out->n, text, &ctx->set);
    if ((ctx->set.status & DEC_Conversion_syntax) != 0) {
        return 1;
    }
    return 0;
}

int lmx_dec_add(LmxDec *out, const LmxDec *a, const LmxDec *b, LmxDecCtx *ctx) {
    if (out == 0 || a == 0 || b == 0 || ctx == 0) {
        return 1;
    }
    decNumberAdd(&out->n, &a->n, &b->n, &ctx->set);
    return 0;
}

int lmx_dec_sub(LmxDec *out, const LmxDec *a, const LmxDec *b, LmxDecCtx *ctx) {
    if (out == 0 || a == 0 || b == 0 || ctx == 0) {
        return 1;
    }
    decNumberSubtract(&out->n, &a->n, &b->n, &ctx->set);
    return 0;
}

int lmx_dec_mul(LmxDec *out, const LmxDec *a, const LmxDec *b, LmxDecCtx *ctx) {
    if (out == 0 || a == 0 || b == 0 || ctx == 0) {
        return 1;
    }
    decNumberMultiply(&out->n, &a->n, &b->n, &ctx->set);
    return 0;
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
