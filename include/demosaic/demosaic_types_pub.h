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
 * @file        demosaic_types_pub.h
 * @date        2020-05-19
 * @author      Neil Abcouwer
 * @brief       Types for Demosaicing Bayer Images
 */

#ifndef DEMOSAIC_TYPES_PUB_H
#define DEMOSAIC_TYPES_PUB_H

// defines, or includes something that defines, F64, I32, U16, U8
#include <demosaic/demosaic_conf_global_types.h>

/// luma coefficients for converting RGB to monochrome
/// should all be >= 0 and add up to 1
typedef struct {
    F64 red;
    F64 green;
    F64 blue;
} demosaic_luma_coefs;

/// arguments for the demosaicing operation
typedef struct {
    I32 n_rows; /// number of rows in the bayer image
    I32 n_cols; /// number of columns in the bayer image
    /** the maximum permissible value of an input pixel,
        i.e. 0x0FFF for 12-bit pixels
        no pixels in the bayer input should be greater than this value
        and demosaicing will not create any pixels greater than this value,
        or any values greater than max_val >> rshift if shifting */
    U16 max_val;
    /// if demosaicing from 16-bit input to 8-bit output,
    /// the amount to right shift, i.e. 4 for 12-bit to 8-bit
    I32 rshift;
    /// if demosacing to mono, the coefficients to use
    demosaic_luma_coefs coefs;
} demosaic_args;

/// a 3x16bit rgb pixel
typedef struct {
    U16 red;
    U16 green;
    U16 blue;
} demosaic_pix_rgb16;

/// a 3x8bit rgb pixel
typedef struct {
    U8 red;
    U8 green;
    U8 blue;
} demosaic_pix_rgb8;

#endif // DEMOSAIC_TYPES_PUB_H
