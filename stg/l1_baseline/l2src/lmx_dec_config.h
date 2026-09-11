#ifndef LMX_DEC_CONFIG_H
#define LMX_DEC_CONFIG_H

/* Fixed LmxDec ABI: coefficient storage is 34 digits in every TU.
 * A different preexisting DECNUMDIGITS is a compile error, not a resize. */
#if defined(DECNUMDIGITS)
#if DECNUMDIGITS != 34
#error lmx_dec ABI requires DECNUMDIGITS 34
#endif
#else
#define DECNUMDIGITS 34
#endif

#endif
