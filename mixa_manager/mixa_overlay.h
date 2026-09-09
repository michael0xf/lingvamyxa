#ifndef MIXA_OVERLAY_H
#define MIXA_OVERLAY_H

#include <stddef.h>

/* Pixel format: 4-byte RGBA, one byte per channel, NOT premultiplied. */

typedef unsigned char MixaU8;

typedef struct MixaOverlayRect {
    size_t width;        /* pixels = cols * cell_width */
    size_t height;       /* pixels = rows * cell_height */
    size_t cell_width;
    size_t cell_height;
    size_t cols;
    size_t rows;
    MixaU8 *pixels; /* RGBA, 4 bytes/pixel, NOT premultiplied */
} MixaOverlayRect;

/* Classic 16-color VGA-style palette: index 0..15 -> RGB. */
static const unsigned char MIXA_PALETTE[16][3] = {
    {0, 0, 0},
    {170, 0, 0},
    {0, 170, 0},
    {170, 85, 0},
    {0, 0, 170},
    {170, 0, 170},
    {0, 170, 170},
    {170, 170, 170},
    {85, 85, 85},
    {255, 85, 85},
    {85, 255, 85},
    {255, 255, 85},
    {85, 85, 255},
    {255, 85, 255},
    {85, 255, 255},
    {255, 255, 255}
};

/* Resolve palette index to RGB. Index is taken modulo 16. */
static void mixa_palette_rgb(unsigned index, MixaU8 *out_r, MixaU8 *out_g, MixaU8 *out_b)
{
    unsigned i = index & 15U;
    if (out_r) {
        *out_r = MIXA_PALETTE[i][0];
    }
    if (out_g) {
        *out_g = MIXA_PALETTE[i][1];
    }
    if (out_b) {
        *out_b = MIXA_PALETTE[i][2];
    }
}

#endif