#ifndef LMX_DEC_H
#define LMX_DEC_H

#include "lmx_dec_config.h"
#include "decNumber.h"
#include <stddef.h>
#include <stdint.h>

/* Narrow owner-local decimal backend over vendored C decNumber.
 * Storage digits=34. Default ctx: BASE emax/emin (±999999999), HALF_EVEN,
 * traps off. That is not IEEE decimal128 encoding or exponent range.
 * ctx.set.digits must stay in 1..LMX_DEC_DIGITS or ops fail (no silent
 * overwrite of LmxDec storage).
 *
 * Return 0 = this call added no DEC_Errors bits. Status is sticky on ctx
 * (diagnostic history). Callers that need a clean slate use ctx_clear.
 * Inexact/Rounded/Clamped are information, not return-1. */

#define LMX_DEC_DIGITS DECNUMDIGITS
#define LMX_DEC_TEXT_MIN (DECNUMDIGITS + 14)

typedef struct LmxDecCtx {
    decContext set;
} LmxDecCtx;

typedef struct LmxDec {
    decNumber n;
} LmxDec;

int lmx_dec_digits(void);
int lmx_dec_ctx_init(LmxDecCtx *ctx);
unsigned lmx_dec_ctx_status(const LmxDecCtx *ctx);
unsigned lmx_dec_ctx_new_errors(unsigned before, unsigned after);
void lmx_dec_ctx_clear(LmxDecCtx *ctx);
int lmx_dec_from_text(LmxDec *out, const char *text, LmxDecCtx *ctx);
int lmx_dec_add(LmxDec *out, const LmxDec *a, const LmxDec *b, LmxDecCtx *ctx);
int lmx_dec_sub(LmxDec *out, const LmxDec *a, const LmxDec *b, LmxDecCtx *ctx);
int lmx_dec_mul(LmxDec *out, const LmxDec *a, const LmxDec *b, LmxDecCtx *ctx);
int lmx_dec_div(LmxDec *out, const LmxDec *a, const LmxDec *b, LmxDecCtx *ctx);
int lmx_dec_neg(LmxDec *out, const LmxDec *a, LmxDecCtx *ctx);
int lmx_dec_pow(LmxDec *out, const LmxDec *a, const LmxDec *b, LmxDecCtx *ctx);
int lmx_dec_compare(const LmxDec *a, const LmxDec *b, LmxDecCtx *ctx, int *rel);
int lmx_dec_quantize_places(LmxDec *out, const LmxDec *a, int places, enum rounding round, LmxDecCtx *ctx);
int lmx_dec_to_text(const LmxDec *a, char *buf, size_t n);

#endif
