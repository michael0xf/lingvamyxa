/* mixa_file.h - portable console-file seam (FILE_SEAM.txt section 3.5).
 *
 * First slice only: open/close, append, read, size, and absolute seek for
 * bounded offset reads. No list/stat/mkdir/remove/rename. Paths are UTF-8.
 * Handles are opaque and owner-held. Short I/O is reported, never silent.
 * Platform conversion and OS primitives live behind the concrete unit.
 */
#ifndef MIXA_FILE_H
#define MIXA_FILE_H

#include <stddef.h>

typedef struct MixaFile MixaFile;

/* Open modes. */
#define MIXA_FILE_MODE_READ   1
#define MIXA_FILE_MODE_APPEND 2

/* Explicit status: 0 success; nonzero is a failure class (never silent empty). */
#define MIXA_FILE_OK          0
#define MIXA_FILE_ERR_ARG     1
#define MIXA_FILE_ERR_MISSING 2
#define MIXA_FILE_ERR_IO      3
#define MIXA_FILE_ERR_NOMEM   4

/* Open by UTF-8 path. On success *out holds an owned handle; caller closes.
 * READ on a missing path -> MIXA_FILE_ERR_MISSING (not an empty file).
 * APPEND creates the file when absent. */
int mixa_file_open(MixaFile **out, const char *path_utf8, int mode);

/* Close and release. Null-safe. After return the pointer must not be used. */
void mixa_file_close(MixaFile *f);

/* Append bytes. *wrote receives the count actually written (may be short).
 * Returns MIXA_FILE_OK when the call completed; short write is not an error. */
int mixa_file_append(MixaFile *f, const void *buf, size_t n, size_t *wrote);

/* Read at the current position into buf. *got may be short or 0 at EOF.
 * Returns MIXA_FILE_OK when the call completed; short/EOF is not an error. */
int mixa_file_read(MixaFile *f, void *buf, size_t n, size_t *got);

/* Absolute seek from start of file (for bounded offset reads). */
int mixa_file_seek(MixaFile *f, size_t abs_off);

/* Current size in bytes. */
int mixa_file_size(MixaFile *f, size_t *out);

#endif
