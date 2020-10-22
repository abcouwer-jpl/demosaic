/***********************************************************************
 * Copyright 2020 by the California Institute of Technology
 * ALL RIGHTS RESERVED. United States Government Sponsorship acknowledged.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @file        demosaic.c
 * @date        2020-05-19
 * @author      Neil Abcouwer
 * @brief       Functions for Malvar Demosaicing RGGB Bayer Images
 *
 * These functions support linear demosaicing from RGGB format images
 * stored in 16-bit or 8-bit unsigned integer arrays
 * to 16-bit or 8-bit RGB or monochrmome/panchromatic pixels.
 *
 * Based on:
 * H. S. Malvar, Li-wei He and R. Cutler, "High-quality linear interpolation
 * for demosaicing of Bayer-patterned color images," 2004 IEEE International
 * Conference on Acoustics, Speech, and Signal Processing, Montreal, Que.,
 * 2004, pp. iii-485, doi: 10.1109/ICASSP.2004.1326587.
 *
 * Also acknowledge a great analysis in:
 * Pascal Getreuer, Malvar-He-Cutler Linear Image Demosaicking,
 * Image Processing On Line, 1 (2011), pp. 83�89.
 * https://doi.org/10.5201/ipol.2011.g_mhcd
 *
 */

#include <demosaic/demosaic_types_pub.h>
#include <demosaic/demosaic_pub.h>
#include <demosaic/demosaic_conf_private.h>

// macros for faster access

// get pixel value in image buffer with NO safety checking
#define GET_PIX(bayer, n_cols, row, col) \
    ((bayer)[(n_cols)*(row)+(col)])

// constrain between limits, inclusive
#define DM_LIMIT(x, min, max) \
    ( ( (x) >= (max) ) ? (max) : ( ( (x) <= (min) ) ? (min) : (x) ) )

// private helper functions

// get pixel. if out of bounds, get closest pixel of same bayer color
DEMOSAIC_PRIVATE U16 get_pixel16_safe(const U16 * const bayer,
        const I32 n_rows, const I32 n_cols, I32 row, I32 col)
{
    DEMOSAIC_ASSERT(bayer != NULL);

    if (row < 0) {
        row = (-row) % 2;
    }
    if (row >= n_rows) {
        row = n_rows - 2 + (row % 2);
    }
    if (col < 0) {
        col = (-col) % 2;
    }
    if (col >= n_cols) {
        col = n_cols - 2 + (col % 2);
    }

    return bayer[n_cols*row + col];
}

DEMOSAIC_PRIVATE U8 get_pixel8_safe(const U8 * const bayer,
        const I32 n_rows, const I32 n_cols, I32 row, I32 col)
{
    DEMOSAIC_ASSERT(bayer != NULL);

    if (row < 0) {
        row = (-row) % 2;
    }
    if (row >= n_rows) {
        row = n_rows - 2 + (row % 2);
    }
    if (col < 0) {
        col = (-col) % 2;
    }
    if (col >= n_cols) {
        col = n_cols - 2 + (col % 2);
    }

    return bayer[n_cols*row + col];
}

// get rgb values from linear interpolation of bayer pixels

// interpolate green value from a bayer pixel that is not green
// See Malvar paper, Fig 2, G at R locations
//       -1
//       +2
// -1 +2 +4 +2 -1
//       +2
//       -1
DEMOSAIC_PRIVATE U16 get_green16(
        const U16 * const bayer,
        const demosaic_args * const args,
        const I32 row, const I32 col)
{
    DEMOSAIC_ASSERT(args != NULL);

    I32 n_rows = args->n_rows;
    I32 n_cols = args->n_cols;
    I32 max_val = (I32)args->max_val;
    I32 val = 0;

    val =   ( get_pixel16_safe(bayer, n_rows, n_cols, row - 2, col + 0) * -1
            + get_pixel16_safe(bayer, n_rows, n_cols, row - 1, col + 0) * +2
            + get_pixel16_safe(bayer, n_rows, n_cols, row + 0, col - 2) * -1
            + get_pixel16_safe(bayer, n_rows, n_cols, row + 0, col - 1) * +2
            + get_pixel16_safe(bayer, n_rows, n_cols, row + 0, col + 0) * +4
            + get_pixel16_safe(bayer, n_rows, n_cols, row + 0, col + 1) * +2
            + get_pixel16_safe(bayer, n_rows, n_cols, row + 0, col + 2) * -1
            + get_pixel16_safe(bayer, n_rows, n_cols, row + 1, col + 0) * +2
            + get_pixel16_safe(bayer, n_rows, n_cols, row + 2, col + 0) * -1)
          / 8;

    return (U16) DM_LIMIT(val, 0, max_val);
}

DEMOSAIC_PRIVATE U8 get_green8(
        const U8 * const bayer,
        const demosaic_args * const args,
        const I32 row, const I32 col)
{
    DEMOSAIC_ASSERT(args != NULL);

    I32 n_rows = args->n_rows;
    I32 n_cols = args->n_cols;
    I32 max_val = (I32)args->max_val;
    I32 val = 0;

    val =   ( get_pixel8_safe(bayer, n_rows, n_cols, row - 2, col + 0) * -1
            + get_pixel8_safe(bayer, n_rows, n_cols, row - 1, col + 0) * +2
            + get_pixel8_safe(bayer, n_rows, n_cols, row + 0, col - 2) * -1
            + get_pixel8_safe(bayer, n_rows, n_cols, row + 0, col - 1) * +2
            + get_pixel8_safe(bayer, n_rows, n_cols, row + 0, col + 0) * +4
            + get_pixel8_safe(bayer, n_rows, n_cols, row + 0, col + 1) * +2
            + get_pixel8_safe(bayer, n_rows, n_cols, row + 0, col + 2) * -1
            + get_pixel8_safe(bayer, n_rows, n_cols, row + 1, col + 0) * +2
            + get_pixel8_safe(bayer, n_rows, n_cols, row + 2, col + 0) * -1)
          / 8;

    return (U8) DM_LIMIT(val, 0, max_val);
}

// interpolate red or blue value from a green bayer pixel value,
// using reds or blues from that row
// interpolate green value from a bayer pixel that is not green
// See Malvar paper, Fig 2, R at green in R row, B column
// and B at green in B row, R column
// These two kernels are identical, and so use the same function
//       +1
//    -2     -2
// -2 +8 +10 +8 -2
//    -2     -2
//       +1
DEMOSAIC_PRIVATE U16 get_red_blue_from_row16(
        const U16 * const bayer,
        const demosaic_args * const args,
        const I32 row, const I32 col)
{
    DEMOSAIC_ASSERT(args != NULL);

    I32 n_rows = args->n_rows;
    I32 n_cols = args->n_cols;
    I32 max_val = (I32)args->max_val;
    I32 val = 0;

    val =   ( get_pixel16_safe(bayer, n_rows, n_cols, row - 2, col + 0) * +1
            + get_pixel16_safe(bayer, n_rows, n_cols, row - 1, col - 1) * -2
            + get_pixel16_safe(bayer, n_rows, n_cols, row - 1, col + 1) * -2
            + get_pixel16_safe(bayer, n_rows, n_cols, row + 0, col - 2) * -2
            + get_pixel16_safe(bayer, n_rows, n_cols, row + 0, col - 1) * +8
            + get_pixel16_safe(bayer, n_rows, n_cols, row + 0, col + 0) * +10
            + get_pixel16_safe(bayer, n_rows, n_cols, row + 0, col + 1) * +8
            + get_pixel16_safe(bayer, n_rows, n_cols, row + 0, col + 2) * -2
            + get_pixel16_safe(bayer, n_rows, n_cols, row + 1, col - 1) * -2
            + get_pixel16_safe(bayer, n_rows, n_cols, row + 1, col + 1) * -2
            + get_pixel16_safe(bayer, n_rows, n_cols, row + 2, col + 0) * +1)
          / 16;

    return (U16) DM_LIMIT(val, 0, max_val);
}

DEMOSAIC_PRIVATE U8 get_red_blue_from_row8(
        const U8 * const bayer,
        const demosaic_args * const args,
        const I32 row, const I32 col)
{
    DEMOSAIC_ASSERT(args != NULL);

    I32 n_rows = args->n_rows;
    I32 n_cols = args->n_cols;
    I32 max_val = (I32)args->max_val;
    I32 val = 0;

    val =   ( get_pixel8_safe(bayer, n_rows, n_cols, row - 2, col + 0) * +1
            + get_pixel8_safe(bayer, n_rows, n_cols, row - 1, col - 1) * -2
            + get_pixel8_safe(bayer, n_rows, n_cols, row - 1, col + 1) * -2
            + get_pixel8_safe(bayer, n_rows, n_cols, row + 0, col - 2) * -2
            + get_pixel8_safe(bayer, n_rows, n_cols, row + 0, col - 1) * +8
            + get_pixel8_safe(bayer, n_rows, n_cols, row + 0, col + 0) * +10
            + get_pixel8_safe(bayer, n_rows, n_cols, row + 0, col + 1) * +8
            + get_pixel8_safe(bayer, n_rows, n_cols, row + 0, col + 2) * -2
            + get_pixel8_safe(bayer, n_rows, n_cols, row + 1, col - 1) * -2
            + get_pixel8_safe(bayer, n_rows, n_cols, row + 1, col + 1) * -2
            + get_pixel8_safe(bayer, n_rows, n_cols, row + 2, col + 0) * +1)
          / 16;

    return (U16) DM_LIMIT(val, 0, max_val);
}

// interpolate red or blue value from a green bayer pixel,
// using reds or blues from that column
// See Malvar paper, Fig 2, R at green in B row, R column
// and B at green in R row, G column
// These two kernels are identical, and so use the same function
//       -2
//    -2 +8 -2
// +1   +10    +1
//    -2 +8 -2
//       -2
DEMOSAIC_PRIVATE U16 get_red_blue_from_column16(
        const U16 * const bayer,
        const demosaic_args * const args,
        const I32 row, const I32 col)
{
    DEMOSAIC_ASSERT(args != NULL);

    I32 n_rows = args->n_rows;
    I32 n_cols = args->n_cols;
    I32 max_val = (I32)args->max_val;
    I32 val = 0;

    val =   ( get_pixel16_safe(bayer, n_rows, n_cols, row - 2, col + 0) * -2
            + get_pixel16_safe(bayer, n_rows, n_cols, row - 1, col - 1) * -2
            + get_pixel16_safe(bayer, n_rows, n_cols, row - 1, col + 0) * +8
            + get_pixel16_safe(bayer, n_rows, n_cols, row - 1, col + 1) * -2
            + get_pixel16_safe(bayer, n_rows, n_cols, row + 0, col - 2) * +1
            + get_pixel16_safe(bayer, n_rows, n_cols, row + 0, col + 0) * +10
            + get_pixel16_safe(bayer, n_rows, n_cols, row + 0, col + 2) * +1
            + get_pixel16_safe(bayer, n_rows, n_cols, row + 1, col - 1) * -2
            + get_pixel16_safe(bayer, n_rows, n_cols, row + 1, col + 0) * +8
            + get_pixel16_safe(bayer, n_rows, n_cols, row + 1, col + 1) * -2
            + get_pixel16_safe(bayer, n_rows, n_cols, row + 2, col + 0) * -2)
          / 16;

    return (U16) DM_LIMIT(val, 0, max_val);
}

DEMOSAIC_PRIVATE U8 get_red_blue_from_column8(
        const U8 * const bayer,
        const demosaic_args * const args,
        const I32 row, const I32 col)
{
    DEMOSAIC_ASSERT(args != NULL);

    I32 n_rows = args->n_rows;
    I32 n_cols = args->n_cols;
    I32 max_val = (I32)args->max_val;
    I32 val = 0;

    val =   ( get_pixel8_safe(bayer, n_rows, n_cols, row - 2, col + 0) * -2
            + get_pixel8_safe(bayer, n_rows, n_cols, row - 1, col - 1) * -2
            + get_pixel8_safe(bayer, n_rows, n_cols, row - 1, col + 0) * +8
            + get_pixel8_safe(bayer, n_rows, n_cols, row - 1, col + 1) * -2
            + get_pixel8_safe(bayer, n_rows, n_cols, row + 0, col - 2) * +1
            + get_pixel8_safe(bayer, n_rows, n_cols, row + 0, col + 0) * +10
            + get_pixel8_safe(bayer, n_rows, n_cols, row + 0, col + 2) * +1
            + get_pixel8_safe(bayer, n_rows, n_cols, row + 1, col - 1) * -2
            + get_pixel8_safe(bayer, n_rows, n_cols, row + 1, col + 0) * +8
            + get_pixel8_safe(bayer, n_rows, n_cols, row + 1, col + 1) * -2
            + get_pixel8_safe(bayer, n_rows, n_cols, row + 2, col + 0) * -2)
          / 16;

    return (U16) DM_LIMIT(val, 0, max_val);
}

// get red or blue value from a blue or red bayer pixel, respectively
// using reds or blues from corners
// See Malvar paper, Fig 2, R at blue in B row, B column
// and B at red in R row, R column
// These two kernels are identical, and so use the same function
//       -3
//    +4    +4
// -3   +12    -3
//    +4    +4
//       -3
DEMOSAIC_PRIVATE U16 get_red_blue_from_opposite16(
        const U16 * const bayer,
        const demosaic_args * const args,
        const I32 row, const I32 col)
{
    DEMOSAIC_ASSERT(args != NULL);

    I32 n_rows = args->n_rows;
    I32 n_cols = args->n_cols;
    I32 max_val = (I32)args->max_val;
    I32 val = 0;

    val =   ( get_pixel16_safe(bayer, n_rows, n_cols, row - 2, col + 0) * -3
            + get_pixel16_safe(bayer, n_rows, n_cols, row - 1, col - 1) * +4
            + get_pixel16_safe(bayer, n_rows, n_cols, row - 1, col + 1) * +4
            + get_pixel16_safe(bayer, n_rows, n_cols, row + 0, col - 2) * -3
            + get_pixel16_safe(bayer, n_rows, n_cols, row + 0, col + 0) * +12
            + get_pixel16_safe(bayer, n_rows, n_cols, row + 0, col + 2) * -3
            + get_pixel16_safe(bayer, n_rows, n_cols, row + 1, col - 1) * +4
            + get_pixel16_safe(bayer, n_rows, n_cols, row + 1, col + 1) * +4
            + get_pixel16_safe(bayer, n_rows, n_cols, row + 2, col + 0) * -3)
          / 16;

    return (U16) DM_LIMIT(val, 0, max_val);
}

DEMOSAIC_PRIVATE U8 get_red_blue_from_opposite8(
        const U8 * const bayer,
        const demosaic_args * const args,
        const I32 row, const I32 col)
{
    DEMOSAIC_ASSERT(args != NULL);

    I32 n_rows = args->n_rows;
    I32 n_cols = args->n_cols;
    I32 max_val = (I32)args->max_val;
    I32 val = 0;

    val =   ( get_pixel8_safe(bayer, n_rows, n_cols, row - 2, col + 0) * -3
            + get_pixel8_safe(bayer, n_rows, n_cols, row - 1, col - 1) * +4
            + get_pixel8_safe(bayer, n_rows, n_cols, row - 1, col + 1) * +4
            + get_pixel8_safe(bayer, n_rows, n_cols, row + 0, col - 2) * -3
            + get_pixel8_safe(bayer, n_rows, n_cols, row + 0, col + 0) * +12
            + get_pixel8_safe(bayer, n_rows, n_cols, row + 0, col + 2) * -3
            + get_pixel8_safe(bayer, n_rows, n_cols, row + 1, col - 1) * +4
            + get_pixel8_safe(bayer, n_rows, n_cols, row + 1, col + 1) * +4
            + get_pixel8_safe(bayer, n_rows, n_cols, row + 2, col + 0) * -3)
          / 16;

    return (U16) DM_LIMIT(val, 0, max_val);
}

// get rgb pixels

// get 16-bit rgb pixel from 16-bit bayer at red
DEMOSAIC_PRIVATE void get_rgb16_at_red16(
        const U16 * const bayer,
        const demosaic_args * const args,
        const I32 row, const I32 col,
        demosaic_pix_rgb16 * output_pixel)
{
    DEMOSAIC_ASSERT(output_pixel != NULL);
    DEMOSAIC_ASSERT(args != NULL);

    output_pixel->red =
            get_pixel16_safe(bayer, args->n_rows, args->n_cols, row, col);
    output_pixel->green = get_green16(bayer, args, row, col);
    // estimate blue at red pixel
    output_pixel->blue = get_red_blue_from_opposite16(bayer, args, row, col);
}

// get 8-bit rgb pixel from 8-bit bayer at red
DEMOSAIC_PRIVATE void get_rgb8_at_red8(
        const U8 * const bayer,
        const demosaic_args * const args,
        const I32 row, const I32 col,
        demosaic_pix_rgb8 * output_pixel)
{
    DEMOSAIC_ASSERT(output_pixel != NULL);
    DEMOSAIC_ASSERT(args != NULL);

    output_pixel->red =
            get_pixel8_safe(bayer, args->n_rows, args->n_cols, row, col);
    output_pixel->green = get_green8(bayer, args, row, col);
    // estimate blue at red pixel
    output_pixel->blue = get_red_blue_from_opposite8(bayer, args, row, col);
}

// get 8-bit rgb pixel from 16-bit bayer at red
DEMOSAIC_PRIVATE void get_rgb8_at_red16(
        const U16 * const bayer,
        const demosaic_args * const args,
        const I32 row, const I32 col,
        demosaic_pix_rgb8 * output_pixel)
{
    DEMOSAIC_ASSERT(output_pixel != NULL);
    DEMOSAIC_ASSERT(args != NULL);

    output_pixel->red =
            get_pixel16_safe(bayer, args->n_rows, args->n_cols, row, col)
            >> args->rshift;
    output_pixel->green =
            get_green16(bayer, args, row, col)
            >> args->rshift;
    // estimate blue at red pixel
    output_pixel->blue =
            get_red_blue_from_opposite16(bayer, args, row, col)
            >> args->rshift;
}

// get 16-bit rgb pixel from 16-bit bayer at green in red-green row
DEMOSAIC_PRIVATE void get_rgb16_at_green_rg16(
        const U16 * const bayer,
        const demosaic_args * const args,
        const I32 row, const I32 col,
        demosaic_pix_rgb16 * output_pixel)
{
    DEMOSAIC_ASSERT(output_pixel != NULL);
    DEMOSAIC_ASSERT(args != NULL);

    // estimate red at green pixel in a red row
    output_pixel->red = get_red_blue_from_row16(bayer, args, row, col);
    output_pixel->green =
            get_pixel16_safe(bayer, args->n_rows, args->n_cols, row, col);
    // estimate blue at green pixel in a blue column
    output_pixel->blue = get_red_blue_from_column16(bayer, args, row, col);
}

// get 8-bit rgb pixel from 8-bit bayer at green in red-green row
DEMOSAIC_PRIVATE void get_rgb8_at_green_rg8(
        const U8 * const bayer,
        const demosaic_args * const args,
        const I32 row, const I32 col,
        demosaic_pix_rgb8 * output_pixel)
{
    DEMOSAIC_ASSERT(output_pixel != NULL);
    DEMOSAIC_ASSERT(args != NULL);

    // estimate red at green pixel in a red row
    output_pixel->red = get_red_blue_from_row8(bayer, args, row, col);
    output_pixel->green =
            get_pixel8_safe(bayer, args->n_rows, args->n_cols, row, col);
    // estimate blue at green pixel in a blue column
    output_pixel->blue = get_red_blue_from_column8(bayer, args, row, col);
}

// get 8-bit rgb pixel from 16-bit bayer at green in red-green row
DEMOSAIC_PRIVATE void get_rgb8_at_green_rg16(
        const U16 * const bayer,
        const demosaic_args * const args,
        const I32 row, const I32 col,
        demosaic_pix_rgb8 * output_pixel)
{
    DEMOSAIC_ASSERT(output_pixel != NULL);
    DEMOSAIC_ASSERT(args != NULL);

    // estimate red at green pixel in a red row
    output_pixel->red = get_red_blue_from_row16(bayer, args, row, col)
            >> args->rshift;
    output_pixel->green =
            get_pixel16_safe(bayer, args->n_rows, args->n_cols, row, col)
            >> args->rshift;
    // estimate blue at green pixel in a blue column
    output_pixel->blue = get_red_blue_from_column16(bayer, args, row, col)
            >> args->rshift;
}

// get 16-bit rgb pixel from 16-bit bayer at green in green-blue row
DEMOSAIC_PRIVATE void get_rgb16_at_green_gb16(
        const U16 * const bayer,
        const demosaic_args * const args,
        const I32 row, const I32 col,
        demosaic_pix_rgb16 * output_pixel)
{
    DEMOSAIC_ASSERT(output_pixel != NULL);
    DEMOSAIC_ASSERT(args != NULL);

    // estimate red at green pixel in red column
    output_pixel->red = get_red_blue_from_column16(bayer, args, row, col);
    output_pixel->green =
            get_pixel16_safe(bayer, args->n_rows, args->n_cols, row, col);
    // estimate blue at green pixel in blue row
    output_pixel->blue = get_red_blue_from_row16(bayer, args, row, col);
}

// get 8-bit rgb pixel from 8-bit bayer at green in green-blue row
DEMOSAIC_PRIVATE void get_rgb8_at_green_gb8(
        const U8 * const bayer,
        const demosaic_args * const args,
        const I32 row, const I32 col,
        demosaic_pix_rgb8 * output_pixel)
{
    DEMOSAIC_ASSERT(output_pixel != NULL);
    DEMOSAIC_ASSERT(args != NULL);

    output_pixel->red = get_red_blue_from_column8(bayer, args, row, col);
    output_pixel->green =
            get_pixel8_safe(bayer, args->n_rows, args->n_cols, row, col);
    output_pixel->blue = get_red_blue_from_row8(bayer, args, row, col);
}

// get 8-bit rgb pixel from 16-bit bayer at green in green-blue row
DEMOSAIC_PRIVATE void get_rgb8_at_green_gb16(
        const U16 * const bayer,
        const demosaic_args * const args,
        const I32 row, const I32 col,
        demosaic_pix_rgb8 * output_pixel)
{
    DEMOSAIC_ASSERT(output_pixel != NULL);
    DEMOSAIC_ASSERT(args != NULL);

    output_pixel->red = get_red_blue_from_column16(bayer, args, row, col)
            >> args->rshift;
    output_pixel->green =
            get_pixel16_safe(bayer, args->n_rows, args->n_cols, row, col)
            >> args->rshift;
    output_pixel->blue = get_red_blue_from_row16(bayer, args, row, col)
            >> args->rshift;
}


// get 16-bit rgb pixel from 16-bit bayer at blue
DEMOSAIC_PRIVATE void get_rgb16_at_blue16(
        const U16 * const bayer,
        const demosaic_args * const args,
        const I32 row, const I32 col,
        demosaic_pix_rgb16 * output_pixel)
{
    DEMOSAIC_ASSERT(output_pixel != NULL);
    DEMOSAIC_ASSERT(args != NULL);

    output_pixel->red = get_red_blue_from_opposite16(bayer, args, row, col);
    output_pixel->green = get_green16(bayer, args, row, col);
    output_pixel->blue =
            get_pixel16_safe(bayer, args->n_rows, args->n_cols, row, col);
}

// get 8-bit rgb pixel from 8-bit bayer at blue
DEMOSAIC_PRIVATE void get_rgb8_at_blue8(
        const U8 * const bayer,
        const demosaic_args * const args,
        const I32 row, const I32 col,
        demosaic_pix_rgb8 * output_pixel)
{
    DEMOSAIC_ASSERT(output_pixel != NULL);
    DEMOSAIC_ASSERT(args != NULL);

    output_pixel->red = get_red_blue_from_opposite8(bayer, args, row, col);
    output_pixel->green = get_green8(bayer, args, row, col);
    output_pixel->blue =
            get_pixel8_safe(bayer, args->n_rows, args->n_cols, row, col);
}

// get 8-bit rgb pixel from 16-bit bayer at blue
DEMOSAIC_PRIVATE void get_rgb8_at_blue16(
        const U16 * const bayer,
        const demosaic_args * const args,
        const I32 row, const I32 col,
        demosaic_pix_rgb8 * output_pixel)
{
    DEMOSAIC_ASSERT(output_pixel != NULL);
    DEMOSAIC_ASSERT(args != NULL);

    output_pixel->red = get_red_blue_from_opposite16(bayer, args, row, col)
            >> args->rshift;
    output_pixel->green = get_green16(bayer, args, row, col) >> args->rshift;
    output_pixel->blue =
            get_pixel16_safe(bayer, args->n_rows, args->n_cols, row, col)
            >> args->rshift;
}

// assert dimensions are properly sized
DEMOSAIC_PRIVATE void demosaic_malvar_assert_proper_dimensions(
        const demosaic_args * const args)
{
    // assert at least 2x2 image
    DEMOSAIC_ASSERT_1(args->n_cols >= 2, args->n_cols);
    DEMOSAIC_ASSERT_1(args->n_rows >= 2, args->n_rows);

    // assert even number of rows and columns
    DEMOSAIC_ASSERT_1(args->n_cols % 2 == 0, args->n_cols);
    DEMOSAIC_ASSERT_1(args->n_rows % 2 == 0, args->n_rows);
}

// demosaic 16 bit bayer to 16 bit rgb, without optimizations
DEMOSAIC_PRIVATE void demosaic_malvar_row_rgb16_unoptimized(
        const U16 * const bayer,
        const demosaic_args * const args,
        const I32 row,
        demosaic_pix_rgb16 output_row[])
{
    // assert pointers not null
    DEMOSAIC_ASSERT(bayer != NULL);
    DEMOSAIC_ASSERT(args != NULL);
    DEMOSAIC_ASSERT(output_row != NULL);

    // assert even rows and columns, at least 2x2
    demosaic_malvar_assert_proper_dimensions(args);

    // assert row is in image
    DEMOSAIC_ASSERT_2(0 <= row && row < args->n_rows, row, args->n_rows);

    const I32 n_cols = args->n_cols;
    I32 col = 0;

    if ((row % 2) == 0) { // red-green row
        while (col < n_cols) {
            get_rgb16_at_red16(bayer, args, row, col, &(output_row[col]));
            ++col;
            get_rgb16_at_green_rg16(bayer, args, row, col, &(output_row[col]));
            ++col;
        }
    } else { // green-blue row
        while (col < n_cols) {
            get_rgb16_at_green_gb16(bayer, args, row, col, &(output_row[col]));
            ++col;
            get_rgb16_at_blue16(bayer, args, row, col, &(output_row[col]));
            ++col;
        }
    }
}

// demosaic 8 bit bayer to 8 bit rgb, without optimizations
DEMOSAIC_PRIVATE void demosaic_malvar_row_rgb8_unoptimized(
        const U8 * const bayer,
        const demosaic_args * const args,
        const I32 row,
        demosaic_pix_rgb8 output_row[])
{
    // assert pointers not null
    DEMOSAIC_ASSERT(bayer != NULL);
    DEMOSAIC_ASSERT(args != NULL);
    DEMOSAIC_ASSERT(output_row != NULL);

    // assert even rows and columns, at least 2x2
    demosaic_malvar_assert_proper_dimensions(args);

    // assert row is in image
    DEMOSAIC_ASSERT_2(0 <= row && row < args->n_rows, row, args->n_rows);

    // assert max val is 255 or less
    DEMOSAIC_ASSERT_1(args->max_val <= U8_MAX, args->max_val);

    const I32 n_cols = args->n_cols;
    I32 col = 0;

    if ((row % 2) == 0) { // red-green row
        while (col < n_cols) {
            get_rgb8_at_red8(bayer, args, row, col, &(output_row[col]));
            ++col;
            get_rgb8_at_green_rg8(bayer, args, row, col, &(output_row[col]));
            ++col;
        }
    } else { // green-blue row
        while (col < n_cols) {
            get_rgb8_at_green_gb8(bayer, args, row, col, &(output_row[col]));
            ++col;
            get_rgb8_at_blue8(bayer, args, row, col, &(output_row[col]));
            ++col;
        }
    }
}

// demosaic 8 bit bayer to 8 bit rgb, without optimizations
DEMOSAIC_PRIVATE void demosaic_malvar_row_rgb16to8_unoptimized(
        const U16 * const bayer,
        const demosaic_args * const args,
        const I32 row,
        demosaic_pix_rgb8 output_row[])
{
    // assert pointers not null
    DEMOSAIC_ASSERT(bayer != NULL);
    DEMOSAIC_ASSERT(args != NULL);
    DEMOSAIC_ASSERT(output_row != NULL);

    // assert even rows and columns, at least 2x2
    demosaic_malvar_assert_proper_dimensions(args);

    // assert row is in image
    DEMOSAIC_ASSERT_2(0 <= row && row < args->n_rows, row, args->n_rows);

    // assert shift is not negative
    DEMOSAIC_ASSERT_1(args->rshift >= 0, args->rshift);

    // assert max val can be shifted to 8 bits
    DEMOSAIC_ASSERT_2((args->max_val >> args->rshift) <= U8_MAX,
            args->max_val, args->rshift);

    const I32 n_cols = args->n_cols;
    I32 col = 0;

    if ((row % 2) == 0) { // red-green row
        while (col < n_cols) {
            get_rgb8_at_red16(bayer, args, row, col, &(output_row[col]));
            ++col;
            get_rgb8_at_green_rg16(bayer, args, row, col, &(output_row[col]));
            ++col;
        }
    } else { // green-blue row
        while (col < n_cols) {
            get_rgb8_at_green_gb16(bayer, args, row, col, &(output_row[col]));
            ++col;
            get_rgb8_at_blue16(bayer, args, row, col, &(output_row[col]));
            ++col;
        }
    }
}

// Demosaic 16 bit bayer row to 16 bit rgb row
// If the row is at the top or bottom, use the unoptimized function,
// which ensures that sampling past the image edges does not occur.
// Otherwise, use safe functions at left and right edges, and unsafe macros
// in image interior, where edge testing is known to be unnecessary.
// This vastly improves performance.
void demosaic_malvar_row_rgb16(
        const U16 * const bayer,
        const demosaic_args * const args,
        const I32 row,
        demosaic_pix_rgb16 output_row[])
{
    DEMOSAIC_ASSERT(args != NULL);

    // if this row is one of the two top or bottom rows, use safe functions
    if (row < 2 || row >= args->n_rows - 2 ) {
        demosaic_malvar_row_rgb16_unoptimized(bayer, args, row, output_row);
        return;
    }
    // else middle row, can be optimized

    // assert pointers not null
    DEMOSAIC_ASSERT(output_row != NULL);
    DEMOSAIC_ASSERT(bayer != NULL);

    // assert even rows and columns, at least 2x2
    demosaic_malvar_assert_proper_dimensions(args);

    // assert row is in image
    DEMOSAIC_ASSERT_2(0 <= row && row < args->n_rows, row, args->n_rows);

    const I32 ncol = args->n_cols;
    I32 red = 0;
    I32 green = 0;
    I32 blue = 0;
    const U16 max_val = args->max_val;

    if ((row % 2) == 0) { // red-green row
        // at left edge, use safe helpers
        get_rgb16_at_red16(bayer, args, row, 0, &(output_row[0]));
        get_rgb16_at_green_rg16(bayer, args, row, 1, &(output_row[1]));

        // In middle of the image, use macros. This prevents function call
        // overhead and edge-checking, which greatly improves performance.
        I32 col = 2;
        while (col < ncol - 2) {
            // comments on the right indicate sampling kernels. See Malvar paper.

            // red pixel
            output_row[col].red = GET_PIX(bayer,ncol,row,col);

            green = (GET_PIX(bayer, ncol, row-2, col+0) * -1 + //      -1
                     GET_PIX(bayer, ncol, row-1, col+0) * +2 + //       2
                     GET_PIX(bayer, ncol, row+0, col-2) * -1 + // -1 2  4 2 -1
                     GET_PIX(bayer, ncol, row+0, col-1) * +2 + //       2
                     GET_PIX(bayer, ncol, row+0, col+0) * +4 + //      -1
                     GET_PIX(bayer, ncol, row+0, col+1) * +2 +
                     GET_PIX(bayer, ncol, row+0, col+2) * -1 +
                     GET_PIX(bayer, ncol, row+1, col+0) * +2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -1)/8;
            output_row[col].green = DM_LIMIT(green, 0, max_val);

            blue =  (GET_PIX(bayer, ncol, row-2, col+0) * -3 + //      -3
                     GET_PIX(bayer, ncol, row-1, col-1) * +4 + //    4    4
                     GET_PIX(bayer, ncol, row-1, col+1) * +4 + // -3   12   -3
                     GET_PIX(bayer, ncol, row+0, col-2) * -3 + //    4    4
                     GET_PIX(bayer, ncol, row+0, col+0) *+12 + //      -3
                     GET_PIX(bayer, ncol, row+0, col+2) * -3 +
                     GET_PIX(bayer, ncol, row+1, col-1) * +4 +
                     GET_PIX(bayer, ncol, row+1, col+1) * +4 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -3)/16;
            output_row[col].blue = DM_LIMIT(blue, 0, max_val);

            ++col;

            // green pixel
            red =   (GET_PIX(bayer, ncol, row-2, col+0) * +1 + //        1
                     GET_PIX(bayer, ncol, row-1, col-1) * -2 + //    -2    -2
                     GET_PIX(bayer, ncol, row-1, col+1) * -2 + // -2  8 10  8 -2
                     GET_PIX(bayer, ncol, row+0, col-2) * -2 + //    -2    -2
                     GET_PIX(bayer, ncol, row+0, col-1) * +8 + //        1
                     GET_PIX(bayer, ncol, row+0, col+0) *+10 +
                     GET_PIX(bayer, ncol, row+0, col+1) * +8 +
                     GET_PIX(bayer, ncol, row+0, col+2) * -2 +
                     GET_PIX(bayer, ncol, row+1, col-1) * -2 +
                     GET_PIX(bayer, ncol, row+1, col+1) * -2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * +1)/16;
            output_row[col].red = DM_LIMIT(red, 0, max_val);

            output_row[col].green = GET_PIX(bayer,ncol,row,col);

            blue =  (GET_PIX(bayer, ncol, row-2, col+0) * -2 + //      -2
                     GET_PIX(bayer, ncol, row-1, col-1) * -2 + //   -2  8 -2
                     GET_PIX(bayer, ncol, row-1, col+0) * +8 + // 1    10    1
                     GET_PIX(bayer, ncol, row-1, col+1) * -2 + //   -2  8 -2
                     GET_PIX(bayer, ncol, row+0, col-2) * +1 + //      -2
                     GET_PIX(bayer, ncol, row+0, col+0) *+10 +
                     GET_PIX(bayer, ncol, row+0, col+2) * +1 +
                     GET_PIX(bayer, ncol, row+1, col-1) * -2 +
                     GET_PIX(bayer, ncol, row+1, col+0) * +8 +
                     GET_PIX(bayer, ncol, row+1, col+1) * -2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -2)/16;
            output_row[col].blue = DM_LIMIT(blue, 0, max_val);
            ++col;
        }
        // at right edge, use safe helpers
        get_rgb16_at_red16(bayer, args, row, ncol-2, &(output_row[ncol-2]));
        get_rgb16_at_green_rg16(bayer, args, row, ncol-1, &(output_row[ncol-1]));
    } else { // green-blue row
        // at left edge, use safe helpers
        get_rgb16_at_green_gb16(bayer, args, row, 0, &(output_row[0]));
        get_rgb16_at_blue16(bayer, args, row, 1, &(output_row[1]));

        // In middle of the image, use macros. This prevents function call
        // overhead and edge-checking, which greatly improves performance.
        I32 col = 2;
        while (col < ncol - 2) {
            // green pixel
            red =   (GET_PIX(bayer, ncol, row-2, col+0) * -2 + //      -2
                     GET_PIX(bayer, ncol, row-1, col-1) * -2 + //   -2  8 -2
                     GET_PIX(bayer, ncol, row-1, col+0) * +8 + // 1    10    1
                     GET_PIX(bayer, ncol, row-1, col+1) * -2 + //   -2  8 -2
                     GET_PIX(bayer, ncol, row+0, col-2) * +1 + //      -2
                     GET_PIX(bayer, ncol, row+0, col+0) *+10 +
                     GET_PIX(bayer, ncol, row+0, col+2) * +1 +
                     GET_PIX(bayer, ncol, row+1, col-1) * -2 +
                     GET_PIX(bayer, ncol, row+1, col+0) * +8 +
                     GET_PIX(bayer, ncol, row+1, col+1) * -2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -2)/16;
            output_row[col].red = DM_LIMIT(red, 0, max_val);

            output_row[col].green = GET_PIX(bayer,ncol,row,col);

            blue =  (GET_PIX(bayer, ncol, row-2, col+0) * +1 + //        1
                     GET_PIX(bayer, ncol, row-1, col-1) * -2 + //    -2    -2
                     GET_PIX(bayer, ncol, row-1, col+1) * -2 + // -2  8 10  8 -2
                     GET_PIX(bayer, ncol, row+0, col-2) * -2 + //    -2    -2
                     GET_PIX(bayer, ncol, row+0, col-1) * +8 + //        1
                     GET_PIX(bayer, ncol, row+0, col+0) *+10 +
                     GET_PIX(bayer, ncol, row+0, col+1) * +8 +
                     GET_PIX(bayer, ncol, row+0, col+2) * -2 +
                     GET_PIX(bayer, ncol, row+1, col-1) * -2 +
                     GET_PIX(bayer, ncol, row+1, col+1) * -2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * +1)/16;
            output_row[col].blue = DM_LIMIT(blue, 0, max_val);

            ++col;

            // blue pixel
            red =   (GET_PIX(bayer, ncol, row-2, col+0) * -3 + //      -3
                     GET_PIX(bayer, ncol, row-1, col-1) * +4 + //    4  0 4
                     GET_PIX(bayer, ncol, row-1, col+1) * +4 + // -3   12   -3
                     GET_PIX(bayer, ncol, row+0, col-2) * -3 + //    4  0 4
                     GET_PIX(bayer, ncol, row+0, col+0) *+12 + //      -3
                     GET_PIX(bayer, ncol, row+0, col+2) * -3 +
                     GET_PIX(bayer, ncol, row+1, col-1) * +4 +
                     GET_PIX(bayer, ncol, row+1, col+1) * +4 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -3)/16;
            output_row[col].red = DM_LIMIT(red, 0, max_val);

            green = (GET_PIX(bayer, ncol, row-2, col+0) * -1 + //      -1
                     GET_PIX(bayer, ncol, row-1, col+0) * +2 + //       2
                     GET_PIX(bayer, ncol, row+0, col-2) * -1 + // -1 2  4 2 -1
                     GET_PIX(bayer, ncol, row+0, col-1) * +2 + //       2
                     GET_PIX(bayer, ncol, row+0, col+0) * +4 + //      -1
                     GET_PIX(bayer, ncol, row+0, col+1) * +2 +
                     GET_PIX(bayer, ncol, row+0, col+2) * -1 +
                     GET_PIX(bayer, ncol, row+1, col+0) * +2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -1)/8;
            output_row[col].green = DM_LIMIT(green, 0, max_val);

            output_row[col].blue = GET_PIX(bayer,ncol,row,col);

            ++col;
        }
        // at right edge, use safe helpers
        get_rgb16_at_green_gb16(bayer, args, row, ncol-2, &(output_row[ncol-2]));
        get_rgb16_at_blue16(bayer, args, row, ncol-1, &(output_row[ncol-1]));
    }
}

// demosaic 16 bit bayer row to 16 bit rgb row
void demosaic_malvar_rgb16(
        const U16 * const bayer,
        const demosaic_args * const args,
        demosaic_pix_rgb16 * output)
{
    DEMOSAIC_ASSERT(args != NULL);

    // assert even rows and columns, at least 2x2
    demosaic_malvar_assert_proper_dimensions(args);

    for (I32 row = 0; row < args->n_rows; row++) {
        demosaic_malvar_row_rgb16(bayer, args, row,
                &output[row * args->n_cols]);
    }
}

// demosaic 8 bit bayer to 8 bit rgb
// If the row is at the top or bottom, use the unoptimized function,
// which ensures that sampling past the image edges does not occur.
// Otherwise, use safe functions at left and right edges, and unsafe macros
// in image interior, where edge testing is known to be unnecessary.
// This vastly improves performance.
void demosaic_malvar_row_rgb8(
        const U8 * const bayer,
        const demosaic_args * const args,
        const I32 row,
        demosaic_pix_rgb8 output_row[])
{
    DEMOSAIC_ASSERT(args != NULL);

    // if this row is one of the two top or bottom rows, use safe functions
    if (row < 2 || row >= args->n_rows - 2 ) {
        demosaic_malvar_row_rgb8_unoptimized(bayer, args, row, output_row);
        return;
    }
    // else middle row, can be optimized

    // assert pointers not null
    DEMOSAIC_ASSERT(output_row != NULL);
    DEMOSAIC_ASSERT(bayer != NULL);

    // assert even rows and columns, at least 2x2
    demosaic_malvar_assert_proper_dimensions(args);

    // assert row is in image
    DEMOSAIC_ASSERT_2(0 <= row && row < args->n_rows, row, args->n_rows);

    // assert max val is 255 or less
    DEMOSAIC_ASSERT_1(args->max_val <= U8_MAX, args->max_val);

    const I32 ncol = args->n_cols;
    I32 red = 0;
    I32 green = 0;
    I32 blue = 0;
    const U16 max_val = args->max_val;

    if ((row % 2) == 0) { // red-green row
        // at left edge, use safe helpers
        get_rgb8_at_red8(bayer, args, row, 0, &(output_row[0]));
        get_rgb8_at_green_rg8(bayer, args, row, 1, &(output_row[1]));

        // In middle of the image, use macros. This prevents function call
        // overhead and edge-checking, which greatly improves performance.
        I32 col = 2;
        while (col < ncol - 2) {
            // comments on the right indicate sampling kernels. See Malvar paper.

            // red pixel
            output_row[col].red = GET_PIX(bayer,ncol,row,col);
            green = (GET_PIX(bayer, ncol, row-2, col+0) * -1 + //      -1
                     GET_PIX(bayer, ncol, row-1, col+0) * +2 + //       2
                     GET_PIX(bayer, ncol, row+0, col-2) * -1 + // -1 2  4 2 -1
                     GET_PIX(bayer, ncol, row+0, col-1) * +2 + //       2
                     GET_PIX(bayer, ncol, row+0, col+0) * +4 + //      -1
                     GET_PIX(bayer, ncol, row+0, col+1) * +2 +
                     GET_PIX(bayer, ncol, row+0, col+2) * -1 +
                     GET_PIX(bayer, ncol, row+1, col+0) * +2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -1)/8;
            output_row[col].green = DM_LIMIT(green, 0, max_val);

            blue =  (GET_PIX(bayer, ncol, row-2, col+0) * -3 + //      -3
                     GET_PIX(bayer, ncol, row-1, col-1) * +4 + //    4    4
                     GET_PIX(bayer, ncol, row-1, col+1) * +4 + // -3   12   -3
                     GET_PIX(bayer, ncol, row+0, col-2) * -3 + //    4    4
                     GET_PIX(bayer, ncol, row+0, col+0) *+12 + //      -3
                     GET_PIX(bayer, ncol, row+0, col+2) * -3 +
                     GET_PIX(bayer, ncol, row+1, col-1) * +4 +
                     GET_PIX(bayer, ncol, row+1, col+1) * +4 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -3)/16;
            output_row[col].blue = DM_LIMIT(blue, 0, max_val);

            ++col;

            // green pixel
            red =   (GET_PIX(bayer, ncol, row-2, col+0) * +1 + //        1
                     GET_PIX(bayer, ncol, row-1, col-1) * -2 + //    -2    -2
                     GET_PIX(bayer, ncol, row-1, col+1) * -2 + // -2  8 10  8 -2
                     GET_PIX(bayer, ncol, row+0, col-2) * -2 + //    -2    -2
                     GET_PIX(bayer, ncol, row+0, col-1) * +8 + //        1
                     GET_PIX(bayer, ncol, row+0, col+0) *+10 +
                     GET_PIX(bayer, ncol, row+0, col+1) * +8 +
                     GET_PIX(bayer, ncol, row+0, col+2) * -2 +
                     GET_PIX(bayer, ncol, row+1, col-1) * -2 +
                     GET_PIX(bayer, ncol, row+1, col+1) * -2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * +1)/16;
            output_row[col].red = DM_LIMIT(red, 0, max_val);

            output_row[col].green = GET_PIX(bayer,ncol,row,col);

            blue =  (GET_PIX(bayer, ncol, row-2, col+0) * -2 + //      -2
                     GET_PIX(bayer, ncol, row-1, col-1) * -2 + //   -2  8 -2
                     GET_PIX(bayer, ncol, row-1, col+0) * +8 + // 1    10    1
                     GET_PIX(bayer, ncol, row-1, col+1) * -2 + //   -2  8 -2
                     GET_PIX(bayer, ncol, row+0, col-2) * +1 + //      -2
                     GET_PIX(bayer, ncol, row+0, col+0) *+10 +
                     GET_PIX(bayer, ncol, row+0, col+2) * +1 +
                     GET_PIX(bayer, ncol, row+1, col-1) * -2 +
                     GET_PIX(bayer, ncol, row+1, col+0) * +8 +
                     GET_PIX(bayer, ncol, row+1, col+1) * -2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -2)/16;
            output_row[col].blue = DM_LIMIT(blue, 0, max_val);

            ++col;
        }
        // at right edge, use safe helpers
        get_rgb8_at_red8(bayer, args, row, ncol-2, &(output_row[ncol-2]));
        get_rgb8_at_green_rg8(bayer, args, row, ncol-1, &(output_row[ncol-1]));
    } else { // green-blue row
        // at left edge, use safe helpers
        get_rgb8_at_green_gb8(bayer, args, row, 0, &(output_row[0]));
        get_rgb8_at_blue8(bayer, args, row, 1, &(output_row[1]));

        // In middle of the image, use macros. This prevents function call
        // overhead and edge-checking, which greatly improves performance.
        I32 col = 2;
        while (col < ncol - 2) {
            // green pixel
            red =   (GET_PIX(bayer, ncol, row-2, col+0) * -2 + //      -2
                     GET_PIX(bayer, ncol, row-1, col-1) * -2 + //   -2  8 -2
                     GET_PIX(bayer, ncol, row-1, col+0) * +8 + // 1    10    1
                     GET_PIX(bayer, ncol, row-1, col+1) * -2 + //   -2  8 -2
                     GET_PIX(bayer, ncol, row+0, col-2) * +1 + //      -2
                     GET_PIX(bayer, ncol, row+0, col+0) *+10 +
                     GET_PIX(bayer, ncol, row+0, col+2) * +1 +
                     GET_PIX(bayer, ncol, row+1, col-1) * -2 +
                     GET_PIX(bayer, ncol, row+1, col+0) * +8 +
                     GET_PIX(bayer, ncol, row+1, col+1) * -2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -2)/16;
            output_row[col].red = DM_LIMIT(red, 0, max_val);

            output_row[col].green = GET_PIX(bayer,ncol,row,col);

            blue =  (GET_PIX(bayer, ncol, row-2, col+0) * +1 + //        1
                     GET_PIX(bayer, ncol, row-1, col-1) * -2 + //    -2    -2
                     GET_PIX(bayer, ncol, row-1, col+1) * -2 + // -2  8 10  8 -2
                     GET_PIX(bayer, ncol, row+0, col-2) * -2 + //    -2    -2
                     GET_PIX(bayer, ncol, row+0, col-1) * +8 + //        1
                     GET_PIX(bayer, ncol, row+0, col+0) *+10 +
                     GET_PIX(bayer, ncol, row+0, col+1) * +8 +
                     GET_PIX(bayer, ncol, row+0, col+2) * -2 +
                     GET_PIX(bayer, ncol, row+1, col-1) * -2 +
                     GET_PIX(bayer, ncol, row+1, col+1) * -2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * +1)/16;
            output_row[col].blue = DM_LIMIT(blue, 0, max_val);

            ++col;

            // blue pixel
            red =   (GET_PIX(bayer, ncol, row-2, col+0) * -3 + //      -3
                     GET_PIX(bayer, ncol, row-1, col-1) * +4 + //    4    4
                     GET_PIX(bayer, ncol, row-1, col+1) * +4 + // -3   12   -3
                     GET_PIX(bayer, ncol, row+0, col-2) * -3 + //    4    4
                     GET_PIX(bayer, ncol, row+0, col+0) *+12 + //      -3
                     GET_PIX(bayer, ncol, row+0, col+2) * -3 +
                     GET_PIX(bayer, ncol, row+1, col-1) * +4 +
                     GET_PIX(bayer, ncol, row+1, col+1) * +4 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -3)/16;
            output_row[col].red = DM_LIMIT(red, 0, max_val);

            green = (GET_PIX(bayer, ncol, row-2, col+0) * -1 + //      -1
                     GET_PIX(bayer, ncol, row-1, col+0) * +2 + //       2
                     GET_PIX(bayer, ncol, row+0, col-2) * -1 + // -1 2  4 2 -1
                     GET_PIX(bayer, ncol, row+0, col-1) * +2 + //       2
                     GET_PIX(bayer, ncol, row+0, col+0) * +4 + //      -1
                     GET_PIX(bayer, ncol, row+0, col+1) * +2 +
                     GET_PIX(bayer, ncol, row+0, col+2) * -1 +
                     GET_PIX(bayer, ncol, row+1, col+0) * +2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -1)/8;
            output_row[col].green = DM_LIMIT(green, 0, max_val);

            output_row[col].blue = GET_PIX(bayer,ncol,row,col);
            ++col;
        }
        // at right edge, use safe helpers
        get_rgb8_at_green_gb8(bayer, args, row, ncol-2, &(output_row[ncol-2]));
        get_rgb8_at_blue8(bayer, args, row, ncol-1, &(output_row[ncol-1]));
    }
}

// demosaic 8 bit bayer row to 8 bit rgb
void demosaic_malvar_rgb8(
        const U8 * const bayer,
        const demosaic_args * const args,
        demosaic_pix_rgb8 * output)
{
    DEMOSAIC_ASSERT(args != NULL);

    // assert even rows and columns, at least 2x2
    demosaic_malvar_assert_proper_dimensions(args);

    // assert max val is 255 or less
    DEMOSAIC_ASSERT_1(args->max_val <= U8_MAX, args->max_val);

    for (I32 row = 0; row < args->n_rows; row++) {
        demosaic_malvar_row_rgb8(bayer, args, row,
                &output[row * args->n_cols]);
    }
}


// demosaic 16 bit bayer to 8 bit rgb
// If the row is at the top or bottom, use the unoptimized function,
// which ensures that sampling past the image edges does not occur.
// Otherwise, use safe functions at left and right edges, and unsafe macros
// in image interior, where edge testing is known to be unnecessary.
// This vastly improves performance.
void demosaic_malvar_row_rgb16to8(
        const U16 * const bayer,
        const demosaic_args * const args,
        const I32 row,
        demosaic_pix_rgb8 output_row[])
{
    DEMOSAIC_ASSERT(args != NULL);

    // if this row is one of the two top or bottom rows, use safe functions
    if (row < 2 || row >= args->n_rows - 2 ) {
        demosaic_malvar_row_rgb16to8_unoptimized(bayer, args, row, output_row);
        return;
    }
    // else middle row, can be optimized

    // assert pointers not null
    DEMOSAIC_ASSERT(output_row != NULL);
    DEMOSAIC_ASSERT(bayer != NULL);

    // assert even rows and columns, at least 2x2
    demosaic_malvar_assert_proper_dimensions(args);

    // assert row is in image
    DEMOSAIC_ASSERT_2(0 <= row && row < args->n_rows, row, args->n_rows);

    // assert shift is not negative
    DEMOSAIC_ASSERT_1(args->rshift >= 0, args->rshift);

    // assert max val can be shifted to 8 bits
    DEMOSAIC_ASSERT_2((args->max_val >> args->rshift) <= U8_MAX,
            args->max_val, args->rshift);

    const I32 ncol = args->n_cols;
    I32 red = 0;
    I32 green = 0;
    I32 blue = 0;
    const U16 max_val = args->max_val;
    const I32 rshift = args->rshift;
    const U8 max_val_rshift = max_val >> rshift;

    if ((row % 2) == 0) { // red-green row
        // at left edge, use safe helpers
        get_rgb8_at_red16(bayer, args, row, 0, &(output_row[0]));
        get_rgb8_at_green_rg16(bayer, args, row, 1, &(output_row[1]));

        // In middle of the image, use macros. This prevents function call
        // overhead and edge-checking, which greatly improves performance.
        I32 col = 2;
        while (col < ncol - 2) {
            // comments on the right indicate sampling kernels. See Malvar paper.

            // red pixel
            output_row[col].red = GET_PIX(bayer,ncol,row,col) >> rshift;

            green = (GET_PIX(bayer, ncol, row-2, col+0) * -1 + //      -1
                     GET_PIX(bayer, ncol, row-1, col+0) * +2 + //       2
                     GET_PIX(bayer, ncol, row+0, col-2) * -1 + // -1 2  4 2 -1
                     GET_PIX(bayer, ncol, row+0, col-1) * +2 + //       2
                     GET_PIX(bayer, ncol, row+0, col+0) * +4 + //      -1
                     GET_PIX(bayer, ncol, row+0, col+1) * +2 +
                     GET_PIX(bayer, ncol, row+0, col+2) * -1 +
                     GET_PIX(bayer, ncol, row+1, col+0) * +2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -1)/8;
            output_row[col].green = (green >= max_val) ? max_val_rshift :
                                    (green <= 0) ? 0 :
                                            (green >> rshift);

            blue =  (GET_PIX(bayer, ncol, row-2, col+0) * -3 + //      -3
                     GET_PIX(bayer, ncol, row-1, col-1) * +4 + //    4    4
                     GET_PIX(bayer, ncol, row-1, col+1) * +4 + // -3   12   -3
                     GET_PIX(bayer, ncol, row+0, col-2) * -3 + //    4    4
                     GET_PIX(bayer, ncol, row+0, col+0) *+12 + //      -3
                     GET_PIX(bayer, ncol, row+0, col+2) * -3 +
                     GET_PIX(bayer, ncol, row+1, col-1) * +4 +
                     GET_PIX(bayer, ncol, row+1, col+1) * +4 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -3)/16;
            output_row[col].blue = (blue >= max_val) ? max_val_rshift :
                                    (blue <= 0) ? 0 :
                                            (blue >> rshift);
            ++col;

            // green pixel
            red =   (GET_PIX(bayer, ncol, row-2, col+0) * +1 + //        1
                     GET_PIX(bayer, ncol, row-1, col-1) * -2 + //    -2    -2
                     GET_PIX(bayer, ncol, row-1, col+1) * -2 + // -2  8 10  8 -2
                     GET_PIX(bayer, ncol, row+0, col-2) * -2 + //    -2    -2
                     GET_PIX(bayer, ncol, row+0, col-1) * +8 + //        1
                     GET_PIX(bayer, ncol, row+0, col+0) *+10 +
                     GET_PIX(bayer, ncol, row+0, col+1) * +8 +
                     GET_PIX(bayer, ncol, row+0, col+2) * -2 +
                     GET_PIX(bayer, ncol, row+1, col-1) * -2 +
                     GET_PIX(bayer, ncol, row+1, col+1) * -2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * +1)/16;
            output_row[col].red = (red >= max_val) ? max_val_rshift :
                                    (red <= 0) ? 0 :
                                            (red >> rshift);

            output_row[col].green = GET_PIX(bayer,ncol,row,col) >> rshift;

            blue =  (GET_PIX(bayer, ncol, row-2, col+0) * -2 + //      -2
                     GET_PIX(bayer, ncol, row-1, col-1) * -2 + //   -2  8 -2
                     GET_PIX(bayer, ncol, row-1, col+0) * +8 + // 1    10    1
                     GET_PIX(bayer, ncol, row-1, col+1) * -2 + //   -2  8 -2
                     GET_PIX(bayer, ncol, row+0, col-2) * +1 + //      -2
                     GET_PIX(bayer, ncol, row+0, col+0) *+10 +
                     GET_PIX(bayer, ncol, row+0, col+2) * +1 +
                     GET_PIX(bayer, ncol, row+1, col-1) * -2 +
                     GET_PIX(bayer, ncol, row+1, col+0) * +8 +
                     GET_PIX(bayer, ncol, row+1, col+1) * -2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -2)/16;
            output_row[col].blue = (blue >= max_val) ? max_val_rshift :
                                    (blue <= 0) ? 0 :
                                            (blue >> rshift);
            ++col;
        }
        // at right edge, use safe helpers
        get_rgb8_at_red16(bayer, args, row, ncol-2, &(output_row[ncol-2]));
        get_rgb8_at_green_rg16(bayer, args, row, ncol-1, &(output_row[ncol-1]));
    } else { // green-blue row
        // at left edge, use safe helpers
        get_rgb8_at_green_gb16(bayer, args, row, 0, &(output_row[0]));
        get_rgb8_at_blue16(bayer, args, row, 1, &(output_row[1]));

        // In middle of the image, use macros. This prevents function call
        // overhead and edge-checking, which greatly improves performance.
        I32 col = 2;
        while (col < ncol - 2) {
            // green pixel
            red =   (GET_PIX(bayer, ncol, row-2, col+0) * -2 + //      -2
                     GET_PIX(bayer, ncol, row-1, col-1) * -2 + //   -2  8 -2
                     GET_PIX(bayer, ncol, row-1, col+0) * +8 + // 1    10    1
                     GET_PIX(bayer, ncol, row-1, col+1) * -2 + //   -2  8 -2
                     GET_PIX(bayer, ncol, row+0, col-2) * +1 + //      -2
                     GET_PIX(bayer, ncol, row+0, col+0) *+10 +
                     GET_PIX(bayer, ncol, row+0, col+2) * +1 +
                     GET_PIX(bayer, ncol, row+1, col-1) * -2 +
                     GET_PIX(bayer, ncol, row+1, col+0) * +8 +
                     GET_PIX(bayer, ncol, row+1, col+1) * -2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -2)/16;
            output_row[col].red = (red >= max_val) ? max_val_rshift:
                                    (red <= 0) ? 0 :
                                            (red >> rshift);

            output_row[col].green = GET_PIX(bayer,ncol,row,col) >> rshift;

            blue =  (GET_PIX(bayer, ncol, row-2, col+0) * +1 + //        1
                     GET_PIX(bayer, ncol, row-1, col-1) * -2 + //    -2    -2
                     GET_PIX(bayer, ncol, row-1, col+1) * -2 + // -2  8 10  8 -2
                     GET_PIX(bayer, ncol, row+0, col-2) * -2 + //    -2    -2
                     GET_PIX(bayer, ncol, row+0, col-1) * +8 + //        1
                     GET_PIX(bayer, ncol, row+0, col+0) *+10 +
                     GET_PIX(bayer, ncol, row+0, col+1) * +8 +
                     GET_PIX(bayer, ncol, row+0, col+2) * -2 +
                     GET_PIX(bayer, ncol, row+1, col-1) * -2 +
                     GET_PIX(bayer, ncol, row+1, col+1) * -2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * +1)/16;
            output_row[col].blue = (blue >= max_val) ? max_val_rshift :
                                    (blue <= 0) ? 0 :
                                            (blue >> rshift);
            ++col;

            // blue pixel
            red =   (GET_PIX(bayer, ncol, row-2, col+0) * -3 + //      -3
                     GET_PIX(bayer, ncol, row-1, col-1) * +4 + //    4    4
                     GET_PIX(bayer, ncol, row-1, col+1) * +4 + // -3   12   -3
                     GET_PIX(bayer, ncol, row+0, col-2) * -3 + //    4    4
                     GET_PIX(bayer, ncol, row+0, col+0) *+12 + //      -3
                     GET_PIX(bayer, ncol, row+0, col+2) * -3 +
                     GET_PIX(bayer, ncol, row+1, col-1) * +4 +
                     GET_PIX(bayer, ncol, row+1, col+1) * +4 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -3)/16;
            output_row[col].red = (red >= max_val) ? max_val_rshift :
                                    (red <= 0) ? 0 :
                                            (red >> rshift);

            green = (GET_PIX(bayer, ncol, row-2, col+0) * -1 + //      -1
                     GET_PIX(bayer, ncol, row-1, col+0) * +2 + //       2
                     GET_PIX(bayer, ncol, row+0, col-2) * -1 + // -1 2  4 2 -1
                     GET_PIX(bayer, ncol, row+0, col-1) * +2 + //       2
                     GET_PIX(bayer, ncol, row+0, col+0) * +4 + //      -1
                     GET_PIX(bayer, ncol, row+0, col+1) * +2 +
                     GET_PIX(bayer, ncol, row+0, col+2) * -1 +
                     GET_PIX(bayer, ncol, row+1, col+0) * +2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -1)/8;
            output_row[col].green = (green >= max_val) ? max_val_rshift :
                                    (green <= 0) ? 0 :
                                            (green >> rshift);

            output_row[col].blue = GET_PIX(bayer,ncol,row,col) >> rshift;

            ++col;
        }
        // at right edge, use safe helpers
        get_rgb8_at_green_gb16(bayer, args, row, ncol-2, &(output_row[ncol-2]));
        get_rgb8_at_blue16(bayer, args, row, ncol-1, &(output_row[ncol-1]));
    }
}

// demosaic 16 bit bayer to 8 bit rgb
void demosaic_malvar_rgb16to8(
        const U16 * const bayer,
        const demosaic_args * const args,
        demosaic_pix_rgb8 * output)
{
    DEMOSAIC_ASSERT(args != NULL);

    // assert even rows and columns, at least 2x2
    demosaic_malvar_assert_proper_dimensions(args);

    for (I32 row = 0; row < args->n_rows; row++) {
        demosaic_malvar_row_rgb16to8(bayer, args, row,
                &output[row * args->n_cols]);
    }
}

// demosaic 16 bit bayer to 16 bit mono, without optimizations
DEMOSAIC_PRIVATE void demosaic_malvar_row_mono16_unoptimized(
        const U16 * const bayer,
        const demosaic_args * const args,
        const I32 row,
        U16 output_row[])
{
    // assert pointers not null
    DEMOSAIC_ASSERT(bayer != NULL);
    DEMOSAIC_ASSERT(args != NULL);
    DEMOSAIC_ASSERT(output_row != NULL);

    // assert even rows and columns, at least 2x2
    demosaic_malvar_assert_proper_dimensions(args);

    // assert row is in image
    DEMOSAIC_ASSERT_2(0 <= row && row < args->n_rows, row, args->n_rows);

    // assert coefficients are in [0,1]
    DEMOSAIC_ASSERT_DBL_1(0 <= args->coefs.red && args->coefs.red <= 1,
            args->coefs.red);
    DEMOSAIC_ASSERT_DBL_1(0 <= args->coefs.green && args->coefs.green <= 1,
            args->coefs.green);
    DEMOSAIC_ASSERT_DBL_1(0 <= args->coefs.blue && args->coefs.blue <= 1,
            args->coefs.blue);

    // normalize coefficients
    demosaic_luma_coefs coefs_normed;
    F64 coef_sum = args->coefs.red + args->coefs.green + args->coefs.blue
            + 0.000001;
    coefs_normed.red = args->coefs.red / coef_sum;
    coefs_normed.green = args->coefs.green / coef_sum;
    coefs_normed.blue = args->coefs.blue / coef_sum;
    coef_sum = coefs_normed.red + coefs_normed.green + coefs_normed.blue;
    DEMOSAIC_ASSERT_DBL_1(coef_sum < 1.0, coef_sum);

    const I32 ncol = args->n_cols;
    I32 col = 0;
    demosaic_pix_rgb16 rgb = {0,0,0};

    if ((row % 2) == 0) { // red-green row
        while (col < ncol) {
            get_rgb16_at_red16(bayer, args, row, col, &rgb);
            output_row[col] = coefs_normed.red * rgb.red
                              + coefs_normed.green * rgb.green
                              + coefs_normed.blue * rgb.blue
                              + 0.5;
            ++col;
            get_rgb16_at_green_rg16(bayer, args, row, col, &rgb);
            output_row[col] = coefs_normed.red * rgb.red
                              + coefs_normed.green * rgb.green
                              + coefs_normed.blue * rgb.blue
                              + 0.5;
            ++col;
        }
    } else { // green-blue row
        while (col < ncol) {
            get_rgb16_at_green_gb16(bayer, args, row, col, &rgb);
            output_row[col] = coefs_normed.red * rgb.red
                              + coefs_normed.green * rgb.green
                              + coefs_normed.blue * rgb.blue
                              + 0.5;
            ++col;
            get_rgb16_at_blue16(bayer, args, row, col, &rgb);
            output_row[col] = coefs_normed.red * rgb.red
                              + coefs_normed.green * rgb.green
                              + coefs_normed.blue * rgb.blue
                              + 0.5;
            ++col;
        }
    }
}

// demosaic 8 bit bayer to 8 bit mono, without optimizations
DEMOSAIC_PRIVATE void demosaic_malvar_row_mono8_unoptimized(
        const U8 * const bayer,
        const demosaic_args * const args,
        const I32 row,
        U8 output_row[])
{
    // assert pointers not null
    DEMOSAIC_ASSERT(bayer != NULL);
    DEMOSAIC_ASSERT(args != NULL);
    DEMOSAIC_ASSERT(output_row != NULL);

    // assert even rows and columns, at least 2x2
    demosaic_malvar_assert_proper_dimensions(args);

    // assert row is in image
    DEMOSAIC_ASSERT_2(0 <= row && row < args->n_rows, row, args->n_rows);

    // assert max val is 255 or less
    DEMOSAIC_ASSERT_1(args->max_val <= U8_MAX, args->max_val);

    // assert coefficients are in [0,1]
    DEMOSAIC_ASSERT_DBL_1(0 <= args->coefs.red && args->coefs.red <= 1,
            args->coefs.red);
    DEMOSAIC_ASSERT_DBL_1(0 <= args->coefs.green && args->coefs.green <= 1,
            args->coefs.green);
    DEMOSAIC_ASSERT_DBL_1(0 <= args->coefs.blue && args->coefs.blue <= 1,
            args->coefs.blue);

    // normalize coefficients
    demosaic_luma_coefs coefs_normed;
    F64 coef_sum = args->coefs.red + args->coefs.green + args->coefs.blue
            + 0.000001;
    coefs_normed.red = args->coefs.red / coef_sum;
    coefs_normed.green = args->coefs.green / coef_sum;
    coefs_normed.blue = args->coefs.blue / coef_sum;
    coef_sum = coefs_normed.red + coefs_normed.green + coefs_normed.blue;
    DEMOSAIC_ASSERT_DBL_1(coef_sum < 1.0, coef_sum);

    const I32 ncol = args->n_cols;
    I32 col = 0;
    demosaic_pix_rgb8 rgb = {0,0,0};

    if ((row % 2) == 0) { // red-green row
        while (col < ncol) {
            get_rgb8_at_red8(bayer, args, row, col, &rgb);
            output_row[col] = coefs_normed.red * rgb.red
                              + coefs_normed.green * rgb.green
                              + coefs_normed.blue * rgb.blue
                              + 0.5;
            ++col;
            get_rgb8_at_green_rg8(bayer, args, row, col, &rgb);
            output_row[col] = coefs_normed.red * rgb.red
                              + coefs_normed.green * rgb.green
                              + coefs_normed.blue * rgb.blue
                              + 0.5;
            ++col;
        }
    } else { // green-blue row
        while (col < ncol) {
            get_rgb8_at_green_gb8(bayer, args, row, col, &rgb);
            output_row[col] = coefs_normed.red * rgb.red
                              + coefs_normed.green * rgb.green
                              + coefs_normed.blue * rgb.blue
                              + 0.5;
            ++col;
            get_rgb8_at_blue8(bayer, args, row, col, &rgb);
            output_row[col] = coefs_normed.red * rgb.red
                              + coefs_normed.green * rgb.green
                              + coefs_normed.blue * rgb.blue
                              + 0.5;
            ++col;
        }
    }
}

// demosaic 16 bit bayer to 8 bit mono, without optimizations
DEMOSAIC_PRIVATE void demosaic_malvar_row_mono16to8_unoptimized(
        const U16 * const bayer,
        const demosaic_args * const args,
        const I32 row,
        U8 output_row[])
{
    // assert pointers not null
    DEMOSAIC_ASSERT(bayer != NULL);
    DEMOSAIC_ASSERT(args != NULL);
    DEMOSAIC_ASSERT(output_row != NULL);

    // assert even rows and columns, at least 2x2
    demosaic_malvar_assert_proper_dimensions(args);

    // assert even rows and columns, at least 2x2
    demosaic_malvar_assert_proper_dimensions(args);

    // assert row is in image
    DEMOSAIC_ASSERT_2(0 <= row && row < args->n_rows, row, args->n_rows);

    // assert coefficients are in [0,1]
    DEMOSAIC_ASSERT_DBL_1(0 <= args->coefs.red && args->coefs.red <= 1,
            args->coefs.red);
    DEMOSAIC_ASSERT_DBL_1(0 <= args->coefs.green && args->coefs.green <= 1,
            args->coefs.green);
    DEMOSAIC_ASSERT_DBL_1(0 <= args->coefs.blue && args->coefs.blue <= 1,
            args->coefs.blue);

    // assert shift is not negative
    DEMOSAIC_ASSERT_1(args->rshift >= 0, args->rshift);

    // assert max val can be shifted to 8 bits
    DEMOSAIC_ASSERT_2((args->max_val >> args->rshift) <= U8_MAX,
            args->max_val, args->rshift);

    // normalize coefficients
    demosaic_luma_coefs coefs_normed;
    F64 coef_sum = args->coefs.red + args->coefs.green + args->coefs.blue
            + 0.000001;
    coefs_normed.red = args->coefs.red / coef_sum;
    coefs_normed.green = args->coefs.green / coef_sum;
    coefs_normed.blue = args->coefs.blue / coef_sum;
    coef_sum = coefs_normed.red + coefs_normed.green + coefs_normed.blue;
    DEMOSAIC_ASSERT_DBL_1(coef_sum < 1.0, coef_sum);

    const I32 ncol = args->n_cols;
    I32 col = 0;
    demosaic_pix_rgb8 rgb = {0,0,0};

    if ((row % 2) == 0) { // red-green row
        while (col < ncol) {
            get_rgb8_at_red16(bayer, args, row, col, &rgb);
            output_row[col] = coefs_normed.red * rgb.red
                              + coefs_normed.green * rgb.green
                              + coefs_normed.blue * rgb.blue
                              + 0.5;
            ++col;
            get_rgb8_at_green_rg16(bayer, args, row, col, &rgb);
            output_row[col] = coefs_normed.red * rgb.red
                              + coefs_normed.green * rgb.green
                              + coefs_normed.blue * rgb.blue
                              + 0.5;
            ++col;
        }
    } else { // green-blue row
        while (col < ncol) {
            get_rgb8_at_green_gb16(bayer, args, row, col, &rgb);
            output_row[col] = coefs_normed.red * rgb.red
                              + coefs_normed.green * rgb.green
                              + coefs_normed.blue * rgb.blue
                              + 0.5;
            ++col;
            get_rgb8_at_blue16(bayer, args, row, col, &rgb);
            output_row[col] = coefs_normed.red * rgb.red
                              + coefs_normed.green * rgb.green
                              + coefs_normed.blue * rgb.blue
                              + 0.5;
            ++col;
        }
    }
}

// demosaic 16 bit bayer to 16 bit monchromatic
// If the row is at the top or bottom, use the unoptimized function,
// which ensures that sampling past the image edges does not occur.
// Otherwise, use safe functions at left and right edges, and unsafe macros
// in image interior, where edge testing is known to be unnecessary.
// This vastly improves performance.
void demosaic_malvar_row_mono16(
        const U16 * const bayer,
        const demosaic_args * const args,
        const I32 row,
        U16 output_row[])
{
    DEMOSAIC_ASSERT(args != NULL);

    // if this row is one of the two top or bottom rows, use safe functions
    if (row < 2 || row >= args->n_rows - 2 ) {
        demosaic_malvar_row_mono16_unoptimized(bayer, args, row, output_row);
        return;
    }
    // else this is a middle row, can be optimized

    // assert pointers not null
    DEMOSAIC_ASSERT(output_row != NULL);
    DEMOSAIC_ASSERT(bayer != NULL);

    // assert even rows and columns, at least 2x2
    demosaic_malvar_assert_proper_dimensions(args);

    // assert row is in image
    DEMOSAIC_ASSERT_2(0 <= row && row < args->n_rows, row, args->n_rows);

    // assert coefficients are in [0,1]
    DEMOSAIC_ASSERT_DBL_1(0 <= args->coefs.red && args->coefs.red <= 1,
            args->coefs.red);
    DEMOSAIC_ASSERT_DBL_1(0 <= args->coefs.green && args->coefs.green <= 1,
            args->coefs.green);
    DEMOSAIC_ASSERT_DBL_1(0 <= args->coefs.blue && args->coefs.blue <= 1,
            args->coefs.blue);

    // normalize coefficients
    demosaic_luma_coefs coefs_normed;
    F64 coef_sum = args->coefs.red + args->coefs.green + args->coefs.blue
            + 0.000001;
    coefs_normed.red = args->coefs.red / coef_sum;
    coefs_normed.green = args->coefs.green / coef_sum;
    coefs_normed.blue = args->coefs.blue / coef_sum;
    coef_sum = coefs_normed.red + coefs_normed.green + coefs_normed.blue;
    DEMOSAIC_ASSERT_DBL_1(coef_sum < 1.0, coef_sum);

    const I32 ncol = args->n_cols;
    I32 red = 0;
    I32 green = 0;
    I32 blue = 0;
    const U16 max_val = args->max_val;
    demosaic_pix_rgb16 rgb = {0,0,0};

    if ((row % 2) == 0) { // red-green row
        // at left edge, use safe helpers
        get_rgb16_at_red16(bayer, args, row, 0, &rgb);
        output_row[0] = coefs_normed.red * rgb.red
                        + coefs_normed.green * rgb.green
                        + coefs_normed.blue * rgb.blue
                        + 0.5;
        get_rgb16_at_green_rg16(bayer, args, row, 1, &rgb);
        output_row[1] = coefs_normed.red * rgb.red
                        + coefs_normed.green * rgb.green
                        + coefs_normed.blue * rgb.blue
                        + 0.5;

        // In middle of the image, use macros. This prevents function call
        // overhead and edge-checking, which greatly improves performance.
        I32 col = 2;
        while (col < ncol - 2) {
            // comments on the right indicate sampling kernels. See Malvar paper.

            // red pixel
            red = GET_PIX(bayer,ncol,row,col);

            green = (GET_PIX(bayer, ncol, row-2, col+0) * -1 + //      -1
                     GET_PIX(bayer, ncol, row-1, col+0) * +2 + //       2
                     GET_PIX(bayer, ncol, row+0, col-2) * -1 + // -1 2  4 2 -1
                     GET_PIX(bayer, ncol, row+0, col-1) * +2 + //       2
                     GET_PIX(bayer, ncol, row+0, col+0) * +4 + //      -1
                     GET_PIX(bayer, ncol, row+0, col+1) * +2 +
                     GET_PIX(bayer, ncol, row+0, col+2) * -1 +
                     GET_PIX(bayer, ncol, row+1, col+0) * +2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -1)/8;
            green = DM_LIMIT(green, 0, max_val);

            blue =  (GET_PIX(bayer, ncol, row-2, col+0) * -3 + //      -3
                     GET_PIX(bayer, ncol, row-1, col-1) * +4 + //    4    4
                     GET_PIX(bayer, ncol, row-1, col+1) * +4 + // -3   12   -3
                     GET_PIX(bayer, ncol, row+0, col-2) * -3 + //    4    4
                     GET_PIX(bayer, ncol, row+0, col+0) *+12 + //      -3
                     GET_PIX(bayer, ncol, row+0, col+2) * -3 +
                     GET_PIX(bayer, ncol, row+1, col-1) * +4 +
                     GET_PIX(bayer, ncol, row+1, col+1) * +4 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -3)/16;
            blue = DM_LIMIT(blue, 0, max_val);

            output_row[col] = coefs_normed.red * red
                            + coefs_normed.green * green
                            + coefs_normed.blue * blue
                            + 0.5;
            ++col;

            // green pixel
            red =   (GET_PIX(bayer, ncol, row-2, col+0) * +1 + //        1
                     GET_PIX(bayer, ncol, row-1, col-1) * -2 + //    -2    -2
                     GET_PIX(bayer, ncol, row-1, col+1) * -2 + // -2  8 10  8 -2
                     GET_PIX(bayer, ncol, row+0, col-2) * -2 + //    -2    -2
                     GET_PIX(bayer, ncol, row+0, col-1) * +8 + //        1
                     GET_PIX(bayer, ncol, row+0, col+0) *+10 +
                     GET_PIX(bayer, ncol, row+0, col+1) * +8 +
                     GET_PIX(bayer, ncol, row+0, col+2) * -2 +
                     GET_PIX(bayer, ncol, row+1, col-1) * -2 +
                     GET_PIX(bayer, ncol, row+1, col+1) * -2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * +1)/16;
            red = DM_LIMIT(red, 0, max_val);

            green = GET_PIX(bayer,ncol,row,col);

            blue =  (GET_PIX(bayer, ncol, row-2, col+0) * -2 + //      -2
                     GET_PIX(bayer, ncol, row-1, col-1) * -2 + //   -2  8 -2
                     GET_PIX(bayer, ncol, row-1, col+0) * +8 + // 1    10    1
                     GET_PIX(bayer, ncol, row-1, col+1) * -2 + //   -2  8 -2
                     GET_PIX(bayer, ncol, row+0, col-2) * +1 + //      -2
                     GET_PIX(bayer, ncol, row+0, col+0) *+10 +
                     GET_PIX(bayer, ncol, row+0, col+2) * +1 +
                     GET_PIX(bayer, ncol, row+1, col-1) * -2 +
                     GET_PIX(bayer, ncol, row+1, col+0) * +8 +
                     GET_PIX(bayer, ncol, row+1, col+1) * -2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -2)/16;
            blue = DM_LIMIT(blue, 0, max_val);

            output_row[col] = coefs_normed.red * red
                            + coefs_normed.green * green
                            + coefs_normed.blue * blue
                            + 0.5;
            ++col;
        }
        // at right edge, use safe helpers
        get_rgb16_at_red16(bayer, args, row, ncol-2, &rgb);
        output_row[ncol - 2] = coefs_normed.red * rgb.red
                               + coefs_normed.green * rgb.green
                               + coefs_normed.blue * rgb.blue
                               + 0.5;
        get_rgb16_at_green_rg16(bayer, args, row, ncol-1, &rgb);
        output_row[ncol - 1] = coefs_normed.red * rgb.red
                               + coefs_normed.green * rgb.green
                               + coefs_normed.blue * rgb.blue
                               + 0.5;
    } else { // green-blue row
        // at left edge, use safe helpers
        get_rgb16_at_green_gb16(bayer, args, row, 0, &rgb);
        output_row[0] = coefs_normed.red * rgb.red
                        + coefs_normed.green * rgb.green
                        + coefs_normed.blue * rgb.blue
                        + 0.5;
        get_rgb16_at_blue16(bayer, args, row, 1, &rgb);
        output_row[1] = coefs_normed.red * rgb.red
                        + coefs_normed.green * rgb.green
                        + coefs_normed.blue * rgb.blue
                        + 0.5;
        // In middle of the image, use macros. This prevents function call
        // overhead and edge-checking, which greatly improves performance.
        I32 col = 2;
        while (col < ncol - 2) {
            // green pixel
            red =   (GET_PIX(bayer, ncol, row-2, col+0) * -2 + //      -2
                     GET_PIX(bayer, ncol, row-1, col-1) * -2 + //   -2  8 -2
                     GET_PIX(bayer, ncol, row-1, col+0) * +8 + // 1    10    1
                     GET_PIX(bayer, ncol, row-1, col+1) * -2 + //   -2  8 -2
                     GET_PIX(bayer, ncol, row+0, col-2) * +1 + //      -2
                     GET_PIX(bayer, ncol, row+0, col+0) *+10 +
                     GET_PIX(bayer, ncol, row+0, col+2) * +1 +
                     GET_PIX(bayer, ncol, row+1, col-1) * -2 +
                     GET_PIX(bayer, ncol, row+1, col+0) * +8 +
                     GET_PIX(bayer, ncol, row+1, col+1) * -2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -2)/16;
            red = DM_LIMIT(red, 0, max_val);

            green = GET_PIX(bayer,ncol,row,col);

            blue =  (GET_PIX(bayer, ncol, row-2, col+0) * +1 + //        1
                     GET_PIX(bayer, ncol, row-1, col-1) * -2 + //    -2    -2
                     GET_PIX(bayer, ncol, row-1, col+1) * -2 + // -2  8 10  8 -2
                     GET_PIX(bayer, ncol, row+0, col-2) * -2 + //    -2    -2
                     GET_PIX(bayer, ncol, row+0, col-1) * +8 + //        1
                     GET_PIX(bayer, ncol, row+0, col+0) *+10 +
                     GET_PIX(bayer, ncol, row+0, col+1) * +8 +
                     GET_PIX(bayer, ncol, row+0, col+2) * -2 +
                     GET_PIX(bayer, ncol, row+1, col-1) * -2 +
                     GET_PIX(bayer, ncol, row+1, col+1) * -2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * +1)/16;
            blue = DM_LIMIT(blue, 0, max_val);

            output_row[col] = coefs_normed.red * red
                            + coefs_normed.green * green
                            + coefs_normed.blue * blue
                            + 0.5;
            ++col;

            // blue pixel
            red =   (GET_PIX(bayer, ncol, row-2, col+0) * -3 + //      -3
                     GET_PIX(bayer, ncol, row-1, col-1) * +4 + //    4    4
                     GET_PIX(bayer, ncol, row-1, col+1) * +4 + // -3   12   -3
                     GET_PIX(bayer, ncol, row+0, col-2) * -3 + //    4    4
                     GET_PIX(bayer, ncol, row+0, col+0) *+12 + //      -3
                     GET_PIX(bayer, ncol, row+0, col+2) * -3 +
                     GET_PIX(bayer, ncol, row+1, col-1) * +4 +
                     GET_PIX(bayer, ncol, row+1, col+1) * +4 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -3)/16;
            red = DM_LIMIT(red, 0, max_val);

            green = (GET_PIX(bayer, ncol, row-2, col+0) * -1 + //      -1
                     GET_PIX(bayer, ncol, row-1, col+0) * +2 + //       2
                     GET_PIX(bayer, ncol, row+0, col-2) * -1 + // -1 2  4 2 -1
                     GET_PIX(bayer, ncol, row+0, col-1) * +2 + //       2
                     GET_PIX(bayer, ncol, row+0, col+0) * +4 + //      -1
                     GET_PIX(bayer, ncol, row+0, col+1) * +2 +
                     GET_PIX(bayer, ncol, row+0, col+2) * -1 +
                     GET_PIX(bayer, ncol, row+1, col+0) * +2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -1)/8;
            green = DM_LIMIT(green, 0, max_val);

            blue = GET_PIX(bayer,ncol,row,col);
            output_row[col] = coefs_normed.red * red
                            + coefs_normed.green * green
                            + coefs_normed.blue * blue
                            + 0.5;
            ++col;
        }
        // at right edge, use safe helpers
        get_rgb16_at_green_gb16(bayer, args, row, ncol - 2, &rgb);
        output_row[ncol - 2] = coefs_normed.red * rgb.red
                               + coefs_normed.green * rgb.green
                               + coefs_normed.blue * rgb.blue
                               + 0.5;
        get_rgb16_at_blue16(bayer, args, row, ncol - 1, &rgb);
        output_row[ncol - 1] = coefs_normed.red * rgb.red
                               + coefs_normed.green * rgb.green
                               + coefs_normed.blue * rgb.blue
                               + 0.5;
    }
}

// demosaic 16 bit bayer row to 16 bit mono
void demosaic_malvar_mono16(
        const U16 * const bayer,
        const demosaic_args * const args,
        U16 * output)
{
    DEMOSAIC_ASSERT(args != NULL);

    // assert even rows and columns, at least 2x2
    demosaic_malvar_assert_proper_dimensions(args);

    for (I32 row = 0; row < args->n_rows; row++) {
        demosaic_malvar_row_mono16(bayer, args, row,
                &output[row * args->n_cols]);
    }
}

// demosaic 8 bit bayer to 8 bit monchromatic
// If the row is at the top or bottom, use the unoptimized function,
// which ensures that sampling past the image edges does not occur.
// Otherwise, use safe functions at left and right edges, and unsafe macros
// in image interior, where edge testing is known to be unnecessary.
// This vastly improves performance.
void demosaic_malvar_row_mono8(
        const U8 * const bayer,
        const demosaic_args * const args,
        const I32 row,
        U8 output_row[])
{
    DEMOSAIC_ASSERT(args != NULL);

    // if this row is one of the two top or bottom rows, use safe functions
    if (row < 2 || row >= args->n_rows - 2 ) {
        demosaic_malvar_row_mono8_unoptimized(bayer, args, row, output_row);
        return;
    }
    // else this is a middle row, can be optimized

    // assert pointers not null
    DEMOSAIC_ASSERT(output_row != NULL);
    DEMOSAIC_ASSERT(bayer != NULL);

    // assert even rows and columns, at least 2x2
    demosaic_malvar_assert_proper_dimensions(args);

    // assert row is in image
    DEMOSAIC_ASSERT_2(0 <= row && row < args->n_rows, row, args->n_rows);

    // assert max val is 255 or less
    DEMOSAIC_ASSERT_1(args->max_val <= U8_MAX, args->max_val);

    // assert coefficients are in [0,1]
    DEMOSAIC_ASSERT_DBL_1(0 <= args->coefs.red && args->coefs.red <= 1,
            args->coefs.red);
    DEMOSAIC_ASSERT_DBL_1(0 <= args->coefs.green && args->coefs.green <= 1,
            args->coefs.green);
    DEMOSAIC_ASSERT_DBL_1(0 <= args->coefs.blue && args->coefs.blue <= 1,
            args->coefs.blue);

    // normalize coefficients
    demosaic_luma_coefs coefs_normed;
    F64 coef_sum = args->coefs.red + args->coefs.green + args->coefs.blue
            + 0.000001;
    coefs_normed.red = args->coefs.red / coef_sum;
    coefs_normed.green = args->coefs.green / coef_sum;
    coefs_normed.blue = args->coefs.blue / coef_sum;
    coef_sum = coefs_normed.red + coefs_normed.green + coefs_normed.blue;
    DEMOSAIC_ASSERT_DBL_1(coef_sum < 1.0, coef_sum);

    const I32 ncol = args->n_cols;
    I32 red = 0;
    I32 green = 0;
    I32 blue = 0;
    const U16 max_val = args->max_val;
    demosaic_pix_rgb8 rgb = {0,0,0};

    if ((row % 2) == 0) { // red-green row
        // at left edge, use safe helpers
        get_rgb8_at_red8(bayer, args, row, 0, &rgb);
        output_row[0] = coefs_normed.red * rgb.red
                        + coefs_normed.green * rgb.green
                        + coefs_normed.blue * rgb.blue
                        + 0.5;
        get_rgb8_at_green_rg8(bayer, args, row, 1, &rgb);
        output_row[1] = coefs_normed.red * rgb.red
                        + coefs_normed.green * rgb.green
                        + coefs_normed.blue * rgb.blue
                        + 0.5;

        // In middle of the image, use macros. This prevents function call
        // overhead and edge-checking, which greatly improves performance.
        I32 col = 2;
        while (col < ncol - 2) {
            // comments on the right indicate sampling kernels. See Malvar paper.

            // red pixel
            red = GET_PIX(bayer,ncol,row,col);

            green = (GET_PIX(bayer, ncol, row-2, col+0) * -1 + //      -1
                     GET_PIX(bayer, ncol, row-1, col+0) * +2 + //       2
                     GET_PIX(bayer, ncol, row+0, col-2) * -1 + // -1 2  4 2 -1
                     GET_PIX(bayer, ncol, row+0, col-1) * +2 + //       2
                     GET_PIX(bayer, ncol, row+0, col+0) * +4 + //      -1
                     GET_PIX(bayer, ncol, row+0, col+1) * +2 +
                     GET_PIX(bayer, ncol, row+0, col+2) * -1 +
                     GET_PIX(bayer, ncol, row+1, col+0) * +2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -1)/8;
            green = DM_LIMIT(green, 0, max_val);

            blue =  (GET_PIX(bayer, ncol, row-2, col+0) * -3 + //      -3
                     GET_PIX(bayer, ncol, row-1, col-1) * +4 + //    4    4
                     GET_PIX(bayer, ncol, row-1, col+1) * +4 + // -3   12   -3
                     GET_PIX(bayer, ncol, row+0, col-2) * -3 + //    4    4
                     GET_PIX(bayer, ncol, row+0, col+0) *+12 + //      -3
                     GET_PIX(bayer, ncol, row+0, col+2) * -3 +
                     GET_PIX(bayer, ncol, row+1, col-1) * +4 +
                     GET_PIX(bayer, ncol, row+1, col+1) * +4 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -3)/16;
            blue = DM_LIMIT(blue, 0, max_val);

            output_row[col] = coefs_normed.red * red
                            + coefs_normed.green * green
                            + coefs_normed.blue * blue
                            + 0.5;
            ++col;

            // green pixel
            red =   (GET_PIX(bayer, ncol, row-2, col+0) * +1 + //        1
                     GET_PIX(bayer, ncol, row-1, col-1) * -2 + //    -2    -2
                     GET_PIX(bayer, ncol, row-1, col+1) * -2 + // -2  8 10  8 -2
                     GET_PIX(bayer, ncol, row+0, col-2) * -2 + //    -2    -2
                     GET_PIX(bayer, ncol, row+0, col-1) * +8 + //        1
                     GET_PIX(bayer, ncol, row+0, col+0) *+10 +
                     GET_PIX(bayer, ncol, row+0, col+1) * +8 +
                     GET_PIX(bayer, ncol, row+0, col+2) * -2 +
                     GET_PIX(bayer, ncol, row+1, col-1) * -2 +
                     GET_PIX(bayer, ncol, row+1, col+1) * -2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * +1)/16;
            red = DM_LIMIT(red, 0, max_val);

            green = GET_PIX(bayer, ncol, row, col);

            blue =  (GET_PIX(bayer, ncol, row-2, col+0) * -2 + //      -2
                     GET_PIX(bayer, ncol, row-1, col-1) * -2 + //   -2  8 -2
                     GET_PIX(bayer, ncol, row-1, col+0) * +8 + // 1    10    1
                     GET_PIX(bayer, ncol, row-1, col+1) * -2 + //   -2  8 -2
                     GET_PIX(bayer, ncol, row+0, col-2) * +1 + //      -2
                     GET_PIX(bayer, ncol, row+0, col+0) *+10 +
                     GET_PIX(bayer, ncol, row+0, col+2) * +1 +
                     GET_PIX(bayer, ncol, row+1, col-1) * -2 +
                     GET_PIX(bayer, ncol, row+1, col+0) * +8 +
                     GET_PIX(bayer, ncol, row+1, col+1) * -2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -2)/16;
            blue = DM_LIMIT(blue, 0, max_val);

            output_row[col] = coefs_normed.red * red
                            + coefs_normed.green * green
                            + coefs_normed.blue * blue
                            + 0.5;
            ++col;
        }
        // at right edge, use safe helpers
        get_rgb8_at_red8(bayer, args, row, ncol-2, &rgb);
        output_row[ncol - 2] = coefs_normed.red * rgb.red
                               + coefs_normed.green * rgb.green
                               + coefs_normed.blue * rgb.blue
                               + 0.5;
        get_rgb8_at_green_rg8(bayer, args, row, ncol-1, &rgb);
        output_row[ncol - 1] = coefs_normed.red * rgb.red
                               + coefs_normed.green * rgb.green
                               + coefs_normed.blue * rgb.blue
                               + 0.5;
    } else { // green-blue row
        // at left edge, use safe helpers
        get_rgb8_at_green_gb8(bayer, args, row, 0, &rgb);
        output_row[0] = coefs_normed.red * rgb.red
                        + coefs_normed.green * rgb.green
                        + coefs_normed.blue * rgb.blue
                        + 0.5;
        get_rgb8_at_blue8(bayer, args, row, 1, &rgb);
        output_row[1] = coefs_normed.red * rgb.red
                        + coefs_normed.green * rgb.green
                        + coefs_normed.blue * rgb.blue
                        + 0.5;

        // In middle of the image, use macros. This prevents function call
        // overhead and edge-checking, which greatly improves performance.
        I32 col = 2;
        while (col < ncol - 2) {
            // green pixel
            red =   (GET_PIX(bayer, ncol, row-2, col+0) * -2 + //      -2
                     GET_PIX(bayer, ncol, row-1, col-1) * -2 + //   -2  8 -2
                     GET_PIX(bayer, ncol, row-1, col+0) * +8 + // 1    10    1
                     GET_PIX(bayer, ncol, row-1, col+1) * -2 + //   -2  8 -2
                     GET_PIX(bayer, ncol, row+0, col-2) * +1 + //      -2
                     GET_PIX(bayer, ncol, row+0, col+0) *+10 +
                     GET_PIX(bayer, ncol, row+0, col+2) * +1 +
                     GET_PIX(bayer, ncol, row+1, col-1) * -2 +
                     GET_PIX(bayer, ncol, row+1, col+0) * +8 +
                     GET_PIX(bayer, ncol, row+1, col+1) * -2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -2)/16;
            red = DM_LIMIT(red, 0, max_val);

            green = GET_PIX(bayer,ncol,row,col);

            blue =  (GET_PIX(bayer, ncol, row-2, col+0) * +1 + //        1
                     GET_PIX(bayer, ncol, row-1, col-1) * -2 + //    -2    -2
                     GET_PIX(bayer, ncol, row-1, col+1) * -2 + // -2  8 10  8 -2
                     GET_PIX(bayer, ncol, row+0, col-2) * -2 + //    -2    -2
                     GET_PIX(bayer, ncol, row+0, col-1) * +8 + //        1
                     GET_PIX(bayer, ncol, row+0, col+0) *+10 +
                     GET_PIX(bayer, ncol, row+0, col+1) * +8 +
                     GET_PIX(bayer, ncol, row+0, col+2) * -2 +
                     GET_PIX(bayer, ncol, row+1, col-1) * -2 +
                     GET_PIX(bayer, ncol, row+1, col+1) * -2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * +1)/16;
            blue = DM_LIMIT(blue, 0, max_val);

            output_row[col] = coefs_normed.red * red
                            + coefs_normed.green * green
                            + coefs_normed.blue * blue
                            + 0.5;
            ++col;

            // blue pixel
            red =   (GET_PIX(bayer, ncol, row-2, col+0) * -3 + //      -3
                     GET_PIX(bayer, ncol, row-1, col-1) * +4 + //    4    4
                     GET_PIX(bayer, ncol, row-1, col+1) * +4 + // -3   12   -3
                     GET_PIX(bayer, ncol, row+0, col-2) * -3 + //    4    4
                     GET_PIX(bayer, ncol, row+0, col+0) *+12 + //      -3
                     GET_PIX(bayer, ncol, row+0, col+2) * -3 +
                     GET_PIX(bayer, ncol, row+1, col-1) * +4 +
                     GET_PIX(bayer, ncol, row+1, col+1) * +4 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -3)/16;
            red = DM_LIMIT(red, 0, max_val);

            green = (GET_PIX(bayer, ncol, row-2, col+0) * -1 + //      -1
                     GET_PIX(bayer, ncol, row-1, col+0) * +2 + //       2
                     GET_PIX(bayer, ncol, row+0, col-2) * -1 + // -1 2  4 2 -1
                     GET_PIX(bayer, ncol, row+0, col-1) * +2 + //       2
                     GET_PIX(bayer, ncol, row+0, col+0) * +4 + //      -1
                     GET_PIX(bayer, ncol, row+0, col+1) * +2 +
                     GET_PIX(bayer, ncol, row+0, col+2) * -1 +
                     GET_PIX(bayer, ncol, row+1, col+0) * +2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -1)/8;
            green = DM_LIMIT(green, 0, max_val);

            blue = GET_PIX(bayer,ncol,row,col);

            output_row[col] = coefs_normed.red * red
                            + coefs_normed.green * green
                            + coefs_normed.blue * blue
                            + 0.5;
            ++col;
        }
        // at right edge, use safe helpers
        get_rgb8_at_green_gb8(bayer, args, row, ncol - 2, &rgb);
        output_row[ncol - 2] = coefs_normed.red * rgb.red
                               + coefs_normed.green * rgb.green
                               + coefs_normed.blue * rgb.blue
                               + 0.5;
        get_rgb8_at_blue8(bayer, args, row, ncol - 1, &rgb);
        output_row[ncol - 1] = coefs_normed.red * rgb.red
                               + coefs_normed.green * rgb.green
                               + coefs_normed.blue * rgb.blue
                               + 0.5;
    }
}

void demosaic_malvar_mono8(
        const U8 * const bayer,
        const demosaic_args * const args,
        U8 * output)
{
    DEMOSAIC_ASSERT(args != NULL);

    // assert even rows and columns, at least 2x2
    demosaic_malvar_assert_proper_dimensions(args);

    // assert max val is 255 or less
    DEMOSAIC_ASSERT_1(args->max_val <= U8_MAX, args->max_val);

    for (I32 row = 0; row < args->n_rows; row++) {
        demosaic_malvar_row_mono8(bayer, args, row,
                &output[row * args->n_cols]);
    }
}

// demosaic 16 bit bayer to 8 bit monchromatic
// If the row is at the top or bottom, use the unoptimized function,
// which ensures that sampling past the image edges does not occur.
// Otherwise, use safe functions at left and right edges, and unsafe macros
// in image interior, where edge testing is known to be unnecessary.
// This vastly improves performance.
void demosaic_malvar_row_mono16to8(
        const U16 * const bayer,
        const demosaic_args * const args,
        const I32 row,
        U8 output_row[])
{
    DEMOSAIC_ASSERT(args != NULL);

    // if this row is one of the two top or bottom rows, use safe functions
    if (row < 2 || row >= args->n_rows - 2 ) {
        demosaic_malvar_row_mono16to8_unoptimized(
                bayer, args, row, output_row);
        return;
    }
    // else this is a middle row, can be optimized

    // assert pointers not null
    DEMOSAIC_ASSERT(output_row != NULL);
    DEMOSAIC_ASSERT(bayer != NULL);

    // assert even rows and columns, at least 2x2
    demosaic_malvar_assert_proper_dimensions(args);

    // assert row is in image
    DEMOSAIC_ASSERT_2(0 <= row && row < args->n_rows, row, args->n_rows);

    // assert coefficients are in [0,1]
    DEMOSAIC_ASSERT_DBL_1(0 <= args->coefs.red && args->coefs.red <= 1,
            args->coefs.red);
    DEMOSAIC_ASSERT_DBL_1(0 <= args->coefs.green && args->coefs.green <= 1,
            args->coefs.green);
    DEMOSAIC_ASSERT_DBL_1(0 <= args->coefs.blue && args->coefs.blue <= 1,
            args->coefs.blue);

    // assert shift is not negative
    DEMOSAIC_ASSERT_1(args->rshift >= 0, args->rshift);

    // assert max val can be shifted to 8 bits
    DEMOSAIC_ASSERT_2((args->max_val >> args->rshift) <= U8_MAX,
            args->max_val, args->rshift);

    // normalize coefficients
    demosaic_luma_coefs coefs_normed;
    F64 coef_sum = args->coefs.red + args->coefs.green + args->coefs.blue
            + 0.000001;
    coefs_normed.red = args->coefs.red / coef_sum;
    coefs_normed.green = args->coefs.green / coef_sum;
    coefs_normed.blue = args->coefs.blue / coef_sum;
    coef_sum = coefs_normed.red + coefs_normed.green + coefs_normed.blue;
    DEMOSAIC_ASSERT_DBL_1(coef_sum < 1.0, coef_sum);

    const I32 ncol = args->n_cols;
    I32 red = 0;
    I32 green = 0;
    I32 blue = 0;
    const U16 max_val = args->max_val;
    demosaic_pix_rgb8 rgb = {0,0,0};
    const I32 rshift = args->rshift;
    const U8 max_val_rshift = max_val >> rshift;


    if ((row % 2) == 0) { // red-green row
        // at left edge, use safe helpers
        get_rgb8_at_red16(bayer, args, row, 0, &rgb);
        output_row[0] = coefs_normed.red * rgb.red
                        + coefs_normed.green * rgb.green
                        + coefs_normed.blue * rgb.blue
                        + 0.5;
        get_rgb8_at_green_rg16(bayer, args, row, 1, &rgb);
        output_row[1] = coefs_normed.red * rgb.red
                        + coefs_normed.green * rgb.green
                        + coefs_normed.blue * rgb.blue
                        + 0.5;

        // In middle of the image, use macros. This prevents function call
        // overhead and edge-checking, which greatly improves performance.
        I32 col = 2;
        while (col < ncol - 2) {
            // comments on the right indicate sampling kernels. See Malvar paper.

            // red pixel
            red = GET_PIX(bayer,ncol,row,col) >> rshift;

            green = (GET_PIX(bayer, ncol, row-2, col+0) * -1 + //      -1
                     GET_PIX(bayer, ncol, row-1, col+0) * +2 + //       2
                     GET_PIX(bayer, ncol, row+0, col-2) * -1 + // -1 2  4 2 -1
                     GET_PIX(bayer, ncol, row+0, col-1) * +2 + //       2
                     GET_PIX(bayer, ncol, row+0, col+0) * +4 + //      -1
                     GET_PIX(bayer, ncol, row+0, col+1) * +2 +
                     GET_PIX(bayer, ncol, row+0, col+2) * -1 +
                     GET_PIX(bayer, ncol, row+1, col+0) * +2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -1)/8;
            green = (green >= max_val) ? max_val_rshift :
                    (green <= 0) ? 0 :
                                   (green >> rshift);

            blue =  (GET_PIX(bayer, ncol, row-2, col+0) * -3 + //      -3
                     GET_PIX(bayer, ncol, row-1, col-1) * +4 + //    4    4
                     GET_PIX(bayer, ncol, row-1, col+1) * +4 + // -3   12   -3
                     GET_PIX(bayer, ncol, row+0, col-2) * -3 + //    4    4
                     GET_PIX(bayer, ncol, row+0, col+0) *+12 + //      -3
                     GET_PIX(bayer, ncol, row+0, col+2) * -3 +
                     GET_PIX(bayer, ncol, row+1, col-1) * +4 +
                     GET_PIX(bayer, ncol, row+1, col+1) * +4 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -3)/16;
            blue = (blue >= max_val) ? max_val_rshift :
                   (blue <= 0) ? 0 :
                                 (blue >> rshift);

            output_row[col] = coefs_normed.red * red
                            + coefs_normed.green * green
                            + coefs_normed.blue * blue
                            + 0.5;
            ++col;

            // green pixel
            red =   (GET_PIX(bayer, ncol, row-2, col+0) * +1 + //        1
                     GET_PIX(bayer, ncol, row-1, col-1) * -2 + //    -2    -2
                     GET_PIX(bayer, ncol, row-1, col+1) * -2 + // -2  8 10  8 -2
                     GET_PIX(bayer, ncol, row+0, col-2) * -2 + //    -2    -2
                     GET_PIX(bayer, ncol, row+0, col-1) * +8 + //        1
                     GET_PIX(bayer, ncol, row+0, col+0) *+10 +
                     GET_PIX(bayer, ncol, row+0, col+1) * +8 +
                     GET_PIX(bayer, ncol, row+0, col+2) * -2 +
                     GET_PIX(bayer, ncol, row+1, col-1) * -2 +
                     GET_PIX(bayer, ncol, row+1, col+1) * -2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * +1)/16;
            red = (red >= max_val) ? max_val_rshift :
                  (red <= 0) ? 0 :
                               (red >> rshift);

            green = GET_PIX(bayer,ncol,row,col) >> rshift;

            blue =  (GET_PIX(bayer, ncol, row-2, col+0) * -2 + //      -2
                     GET_PIX(bayer, ncol, row-1, col-1) * -2 + //   -2  8 -2
                     GET_PIX(bayer, ncol, row-1, col+0) * +8 + // 1    10    1
                     GET_PIX(bayer, ncol, row-1, col+1) * -2 + //   -2  8 -2
                     GET_PIX(bayer, ncol, row+0, col-2) * +1 + //      -2
                     GET_PIX(bayer, ncol, row+0, col+0) *+10 +
                     GET_PIX(bayer, ncol, row+0, col+2) * +1 +
                     GET_PIX(bayer, ncol, row+1, col-1) * -2 +
                     GET_PIX(bayer, ncol, row+1, col+0) * +8 +
                     GET_PIX(bayer, ncol, row+1, col+1) * -2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -2)/16;
            blue = (blue >= max_val) ? max_val_rshift :
                   (blue <= 0) ? 0 :
                                 (blue >> rshift);

            output_row[col] = coefs_normed.red * red
                            + coefs_normed.green * green
                            + coefs_normed.blue * blue
                            + 0.5;
            ++col;
        }
        // at right edge, use safe helpers
        get_rgb8_at_red16(bayer, args, row, ncol-2, &rgb);
        output_row[ncol - 2] = coefs_normed.red * rgb.red
                               + coefs_normed.green * rgb.green
                               + coefs_normed.blue * rgb.blue
                               + 0.5;
        get_rgb8_at_green_rg16(bayer, args, row, ncol-1, &rgb);
        output_row[ncol - 1] = coefs_normed.red * rgb.red
                               + coefs_normed.green * rgb.green
                               + coefs_normed.blue * rgb.blue
                               + 0.5;
    } else { // green-blue row
        // at left edge, use safe helpers
        get_rgb8_at_green_gb16(bayer, args, row, 0, &rgb);
        output_row[0] = coefs_normed.red * rgb.red
                        + coefs_normed.green * rgb.green
                        + coefs_normed.blue * rgb.blue
                        + 0.5;
        get_rgb8_at_blue16(bayer, args, row, 1, &rgb);
        output_row[1] = coefs_normed.red * rgb.red
                        + coefs_normed.green * rgb.green
                        + coefs_normed.blue * rgb.blue
                        + 0.5;

        // In middle of the image, use macros. This prevents function call
        // overhead and edge-checking, which greatly improves performance.
        I32 col = 2;
        while (col < ncol - 2) {
            // green pixel
            red =   (GET_PIX(bayer, ncol, row-2, col+0) * -2 + //      -2
                     GET_PIX(bayer, ncol, row-1, col-1) * -2 + //   -2  8 -2
                     GET_PIX(bayer, ncol, row-1, col+0) * +8 + // 1    10    1
                     GET_PIX(bayer, ncol, row-1, col+1) * -2 + //   -2  8 -2
                     GET_PIX(bayer, ncol, row+0, col-2) * +1 + //      -2
                     GET_PIX(bayer, ncol, row+0, col+0) *+10 +
                     GET_PIX(bayer, ncol, row+0, col+2) * +1 +
                     GET_PIX(bayer, ncol, row+1, col-1) * -2 +
                     GET_PIX(bayer, ncol, row+1, col+0) * +8 +
                     GET_PIX(bayer, ncol, row+1, col+1) * -2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -2)/16;
            red = (red >= max_val) ? max_val_rshift :
                  (red <= 0) ? 0 :
                               (red >> rshift);

            green = GET_PIX(bayer, ncol, row, col) >> rshift;

            blue =  (GET_PIX(bayer, ncol, row-2, col+0) * +1 + //        1
                     GET_PIX(bayer, ncol, row-1, col-1) * -2 + //    -2    -2
                     GET_PIX(bayer, ncol, row-1, col+1) * -2 + // -2  8 10  8 -2
                     GET_PIX(bayer, ncol, row+0, col-2) * -2 + //    -2    -2
                     GET_PIX(bayer, ncol, row+0, col-1) * +8 + //        1
                     GET_PIX(bayer, ncol, row+0, col+0) *+10 +
                     GET_PIX(bayer, ncol, row+0, col+1) * +8 +
                     GET_PIX(bayer, ncol, row+0, col+2) * -2 +
                     GET_PIX(bayer, ncol, row+1, col-1) * -2 +
                     GET_PIX(bayer, ncol, row+1, col+1) * -2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * +1)/16;
            blue = (blue >= max_val) ? max_val_rshift :
                   (blue <= 0) ? 0 :
                                 (blue >> rshift);

            output_row[col] = coefs_normed.red * red
                            + coefs_normed.green * green
                            + coefs_normed.blue * blue
                            + 0.5;
            ++col;

            // blue pixel
            red =   (GET_PIX(bayer, ncol, row-2, col+0) * -3 + //      -3
                     GET_PIX(bayer, ncol, row-1, col-1) * +4 + //    4    4
                     GET_PIX(bayer, ncol, row-1, col+1) * +4 + // -3   12   -3
                     GET_PIX(bayer, ncol, row+0, col-2) * -3 + //    4    4
                     GET_PIX(bayer, ncol, row+0, col+0) *+12 + //      -3
                     GET_PIX(bayer, ncol, row+0, col+2) * -3 +
                     GET_PIX(bayer, ncol, row+1, col-1) * +4 +
                     GET_PIX(bayer, ncol, row+1, col+1) * +4 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -3)/16;
            red = (red >= max_val) ? max_val_rshift :
                  (red <= 0) ? 0 :
                               (red >> rshift);

            green = (GET_PIX(bayer, ncol, row-2, col+0) * -1 + //      -1
                     GET_PIX(bayer, ncol, row-1, col+0) * +2 + //       2
                     GET_PIX(bayer, ncol, row+0, col-2) * -1 + // -1 2  4 2 -1
                     GET_PIX(bayer, ncol, row+0, col-1) * +2 + //       2
                     GET_PIX(bayer, ncol, row+0, col+0) * +4 + //      -1
                     GET_PIX(bayer, ncol, row+0, col+1) * +2 +
                     GET_PIX(bayer, ncol, row+0, col+2) * -1 +
                     GET_PIX(bayer, ncol, row+1, col+0) * +2 +
                     GET_PIX(bayer, ncol, row+2, col+0) * -1)/8;
            green = (green >= max_val) ? max_val_rshift :
                    (green <= 0) ? 0 :
                                   (green >> rshift);

            blue = GET_PIX(bayer, ncol, row, col) >> rshift;

            output_row[col] = coefs_normed.red * red
                            + coefs_normed.green * green
                            + coefs_normed.blue * blue
                            + 0.5;
            ++col;
        }
        // at right edge, use safe helpers
        get_rgb8_at_green_gb16(bayer, args, row, ncol - 2, &rgb);
        output_row[ncol - 2] = coefs_normed.red * rgb.red
                               + coefs_normed.green * rgb.green
                               + coefs_normed.blue * rgb.blue
                               + 0.5;
        get_rgb8_at_blue16(bayer, args, row, ncol - 1, &rgb);
        output_row[ncol - 1] = coefs_normed.red * rgb.red
                               + coefs_normed.green * rgb.green
                               + coefs_normed.blue * rgb.blue
                               + 0.5;
    }
}

// demosaic 16 bit bayer row to 8 bit mono
void demosaic_malvar_mono16to8(
        const U16 * const bayer,
        const demosaic_args * const args,
        U8 * output)
{
    DEMOSAIC_ASSERT(args != NULL);

    // assert even rows and columns, at least 2x2
    demosaic_malvar_assert_proper_dimensions(args);

    for (I32 row = 0; row < args->n_rows; row++) {
        demosaic_malvar_row_mono16to8(bayer, args, row,
                &output[row * args->n_cols]);
    }
}

