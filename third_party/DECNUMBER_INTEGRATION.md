# decNumber native dependency

Source: the repository's `decNumber.zip`, SHA256
`790EB93B629958444C386DFC9F58EF69518D46CECE9911126287F725671C9AAA`.
Extracted unchanged into `decNumber/`; upstream ICU package version 3.68 is in
`decNumber/decNumber-icu-368/`. Keep `ICU-license.html` and source notices with
redistribution. Bundled patches/build scripts were not applied or executed.

The owner explicitly permits this third-party implementation to remain C.
The core and calculator should share it, not duplicate it or rewrite it in L1.
Core linkage and calculator integration are pending their respective agent tasks;
extraction alone is not integration acceptance.

For decNumber arithmetic, compile `decNumber.c` and `decContext.c`. Do not compile
`decBasic.c`/`decCommon.c` independently: other upstream units include them.
Use consistent `DECNUMDIGITS`/storage declarations across the wrapper and library
translation units; select context precision, rounding, status and traps explicitly.
Check upstream endianness configuration for each target. Context/state belongs
to the invoking owner, not shared mutable global state.

Decimal text must go directly to decimal conversion; no `strtod`/`double` bridge.
The ClearShell literal bridge through `Double.parseDouble` is an owner-confirmed
bug. Retain other calculator rules unless explicitly changed. Binary-only math
functions require a separate contract, not an undocumented decimal fallback.

Initial extraction smoke test (2026-09-11, local GCC, C99, `-O2`,
`-DDECNUMDIGITS=34`, upstream `example1.c` + the two sources above):

```
0.1 + 0.2 => 0.3
9007199254740993 + 1 => 9007199254740994
1.000000000000000000000000000000001
  + 0.000000000000000000000000000000001
  => 1.000000000000000000000000000000002
```

These checks verify direct decimal conversion/addition, not full calculator,
Message concurrency, all-target portability or self-hosting acceptance.
