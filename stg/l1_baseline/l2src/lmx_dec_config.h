#ifndef LMX_DEC_CONFIG_H
#define LMX_DEC_CONFIG_H

/* Shared storage/config for the vendored decNumber backend.
 * Must be identical in every translation unit that includes decNumber.h
 * or compiles decNumber.c / decContext.c. */
#ifndef DECNUMDIGITS
#define DECNUMDIGITS 34
#endif

#endif
