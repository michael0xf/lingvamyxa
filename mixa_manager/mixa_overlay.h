#ifndef MIXA_OVERLAY_H
#define MIXA_OVERLAY_H

#include <stddef.h>

/* Shared pixel byte type + classic 16-color VGA-style palette.
 * OverlayRect raster unit removed (TASK17): upper layer is a MixaTextRect.
 * This header remains so mixa_backend.h can keep #include for MixaU8. */

typedef unsigned char MixaU8;

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