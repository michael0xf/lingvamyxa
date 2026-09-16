#ifndef MIXA_PROCESS_MARKER_H
#define MIXA_PROCESS_MARKER_H

#include <stddef.h>
#include <stdint.h>

#define MIXA_PROCESS_MARKER_OK 0
#define MIXA_PROCESS_MARKER_NO_MATCH 1
#define MIXA_PROCESS_MARKER_MALFORMED 2

typedef struct {
    int32_t exit_code;
    size_t cwd_offset;
    size_t cwd_length;
} MixaProcessMarkerResult;

int mixa_process_marker_parse(const char *expected_marker, size_t marker_len, 
                             const char *payload, size_t payload_len, 
                             MixaProcessMarkerResult *result);

#endif
