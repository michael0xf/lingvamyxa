#ifndef MIXA_POINTER_H
#define MIXA_POINTER_H

#include <stddef.h>
#include <stdint.h>

/* Mouse pointer: one half-cell position + one octant arrow (UI_MODEL 6).
 * Octant is the last NONZERO half-cell displacement; zero leaves it unchanged.
 * No latch, smoothing, hysteresis, accumulation, floats, or trig.
 */

#define MIXA_POINTER_OK  0
#define MIXA_POINTER_ERR 1

/* Clockwise from up. Initial before any move: DOWN_RIGHT. */
#define MIXA_POINTER_UP          0
#define MIXA_POINTER_UP_RIGHT    1
#define MIXA_POINTER_RIGHT       2
#define MIXA_POINTER_DOWN_RIGHT  3
#define MIXA_POINTER_DOWN        4
#define MIXA_POINTER_DOWN_LEFT   5
#define MIXA_POINTER_LEFT        6
#define MIXA_POINTER_UP_LEFT     7

#define MIXA_POINTER_CP_UP         0x2B06U
#define MIXA_POINTER_CP_UP_RIGHT   0x2B08U
#define MIXA_POINTER_CP_RIGHT      0x2B95U
#define MIXA_POINTER_CP_DOWN_RIGHT 0x2B0AU
#define MIXA_POINTER_CP_DOWN       0x2B07U
#define MIXA_POINTER_CP_DOWN_LEFT  0x2B0BU
#define MIXA_POINTER_CP_LEFT       0x2B05U
#define MIXA_POINTER_CP_UP_LEFT    0x2B09U

typedef struct MixaPointer {
    int is_open;
    int octant;
    size_t row; /* half-cell */
    size_t col; /* half-cell */
} MixaPointer;

int mixa_pointer_init(MixaPointer *p);
void mixa_pointer_release(MixaPointer *p);

/* Unicode scalar for the current octant (ordinary codepoint). */
unsigned mixa_pointer_codepoint(const MixaPointer *p);
unsigned mixa_pointer_codepoint_for_octant(int octant);

/* Classify a displacement. Zero dx and dy returns prev_octant unchanged. */
int mixa_pointer_octant_from_delta(int32_t dx, int32_t dy, int prev_octant);

/* Apply a half-cell displacement: update octant if nonzero, then position.
 * Position updates are checked against size_t range.
 */
int mixa_pointer_nudge(MixaPointer *p, int32_t dx, int32_t dy);

/* Move to an absolute half-cell; displacement = to - current. */
int mixa_pointer_move_to(MixaPointer *p, size_t row, size_t col);

#endif