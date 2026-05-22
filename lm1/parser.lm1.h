#ifndef LM_P0_H
#define LM_P0_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum LmP0NodeKind {
    LM_P0_NODE_STRUCTURE = 1,
    LM_P0_NODE_FRAME = 2,
    LM_P0_NODE_ATOM = 3,
    LM_P0_NODE_DISABLED = 4
} LmP0NodeKind;

typedef enum LmP0FrameFlags {
    LM_P0_FRAME_COLON = 1,
    LM_P0_FRAME_COMPACT = 2,
    LM_P0_FRAME_INLINE_BODY = 4
} LmP0FrameFlags;

typedef enum LmP0NodeFlags {
    LM_P0_NODE_INACTIVE = 1,
    LM_P0_NODE_MIX = 2
} LmP0NodeFlags;

typedef enum LmP0TrailerFlags {
    LM_P0_TRAILER_TAIL_CUTTER = 1
} LmP0TrailerFlags;

typedef struct LmP0Text {
    const char *data;
    size_t length;
} LmP0Text;

typedef struct LmP0Span {
    size_t line;
    size_t column;
    size_t offset;
    size_t length;
} LmP0Span;

typedef struct LmP0Diagnostic {
    int code;
    size_t line;
    size_t column;
    char message[256];
} LmP0Diagnostic;

typedef struct LmP0Node LmP0Node;
typedef struct LmP0Field LmP0Field;
typedef struct LmP0Trailer LmP0Trailer;

typedef struct LmP0Structure {
    LmP0Field *first_field;
    LmP0Field *last_field;
    size_t field_count;
    LmP0Trailer *trailer;
} LmP0Structure;

struct LmP0Trailer {
    LmP0Text spelling;
    unsigned flags;
    LmP0Structure body;
};

typedef struct LmP0Frame {
    LmP0Text head;
    unsigned flags;
    LmP0Structure body;
    LmP0Trailer *trailer;
} LmP0Frame;

struct LmP0Node {
    LmP0NodeKind kind;
    unsigned flags;
    LmP0Span span;
    union {
        LmP0Structure structure;
        LmP0Frame frame;
        LmP0Text atom;
    } as;
};

struct LmP0Field {
    LmP0Node *value;
    LmP0Field *next;
};

typedef struct LmP0Document LmP0Document;

int lm_p0_parse_string(const char *source, LmP0Document **out_document);
int lm_p0_parse_bytes(const char *source, size_t source_length, LmP0Document **out_document);
int lm_p0_parse_file(const char *path, LmP0Document **out_document);

void lm_p0_document_destroy(LmP0Document *document);

const LmP0Node *lm_p0_document_root(const LmP0Document *document);
const LmP0Diagnostic *lm_p0_document_diagnostic(const LmP0Document *document);

char *lm_p0_dump_alloc(const LmP0Document *document);
void lm_p0_free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* LM_P0_H */
