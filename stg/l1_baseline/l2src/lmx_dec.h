#ifndef LMX_DEC_H
#define LMX_DEC_H

#include "lmx_dec_config.h"
#include "decNumber.h"
#include <stddef.h>
#include <stdint.h>

/* Narrow owner-local decimal backend over vendored C decNumber.
 * Not an L1 rewrite. Context is supplied by the caller; no process-global
 * decimal state. DECNUMDIGITS is 34 (IEEE decimal128 coefficient length).
 * Default exponent range is decNumber BASE (±999999999), not IEEE
 * decimal128's ±6143. Rounding is half-even. Traps are off; status lives
 * on the caller context. */

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
void lmx_dec_ctx_clear(LmxDecCtx *ctx);
int lmx_dec_from_text(LmxDec *out, const char *text, LmxDecCtx *ctx);
int lmx_dec_add(LmxDec *out, const LmxDec *a, const LmxDec *b, LmxDecCtx *ctx);
int lmx_dec_sub(LmxDec *out, const LmxDec *a, const LmxDec *b, LmxDecCtx *ctx);
int lmx_dec_mul(LmxDec *out, const LmxDec *a, const LmxDec *b, LmxDecCtx *ctx);
int lmx_dec_to_text(const LmxDec *a, char *buf, size_t n);

#endif
