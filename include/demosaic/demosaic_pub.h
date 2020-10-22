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
 * @file        demosaic_pub.h
 * @date        2020-05-19
 * @author      Neil Abcouwer
 * @brief       Function declarations for Demosaicing RGGB Bayer Images
 */

#ifndef DEMOSAIC_PUB_H
#define DEMOSAIC_PUB_H

#include <demosaic/demosaic_types_pub.h>

// ensure symbols are not mangled by C++
#ifdef __cplusplus
   extern "C" {
#endif

/** @brief Demosaic a row of a 16-bit bayer image into 16-bit rgb
 *         with malvar linear interpolation
 *
 *         Image dimensions must be positive, even, row must be within image.
 *
 *         Row number is used to index within the image, but also determine
 *         whether the row number is even (red-green) or odd (green-blue)
 *         and also whether the row is on the edge and functions that do
 *         not sample from beyond the edge must be used.
 *
 *         This row function is publicly accessible so that operations
 *         that work row by row (like JPEG) can demosaic one row at a time.
 *
 * @param bayer         An input 16-bit Bayer image
 * @param args          Dimensions and maximum value of image
 * @param row           The row to be demosaiced
 * @param output_row    Output row of 16-bit RGB pixels,
 *                      length must be equal to the width of the Bayer image.
 *
 */
void demosaic_malvar_row_rgb16(
        const U16 * const bayer,
        const demosaic_args * const args,
        const I32 row,
        demosaic_pix_rgb16 output_row[]);

/** @brief Demosaic a 16-bit bayer image into 16-bit rgb
 *         with malvar linear interpolation
 *
 *         Image dimensions must be positive, even
 *
 * @param bayer         An input 16-bit Bayer image
 * @param args          Dimensions and maximum value of image
 * @param output        Output image of 16-bit RGB pixels,
 *                      Dimensions must be equal to the Bayer image.
 */
void demosaic_malvar_rgb16(
        const U16 * const bayer,
        const demosaic_args * const args,
        demosaic_pix_rgb16 * output);


/** @brief Demosaic a row of a 8-bit bayer image into 8-bit rgb
 *         with malvar linear interpolation
 *
 *         Image dimensions must be positive, even, row must be within image
 *
 *         Row number is used to index within the image, but also determine
 *         whether the row number is even (red-green) or odd (green-blue)
 *         and also whether the row is on the edge and functions that do
 *         not sample from beyond the edge must be used.
 *
 *         This row function is publicly accessible so that operations
 *         that work row by row (like JPEG) can demosaic one row at a time.
 *
 * @param bayer         An input 8-bit Bayer image
 * @param args          Dimensions and maximum value of image
 * @param row           The row to be demosaic
 * @param output_row    Output row of 8-bit RGB pixels,
 *                      length must be equal to the width of the Bayer image.
 */
void demosaic_malvar_row_rgb8(
        const U8 * const bayer,
        const demosaic_args * const args,
        const I32 row,
        demosaic_pix_rgb8 output_row[]);

/** @brief Demosaic an 8-bit bayer image into 8-bit rgb
 *         with malvar linear interpolation
 *
 *         Image dimensions must be positive, even
 *
 * @param bayer         An input 8-bit Bayer image
 * @param args          Dimensions and maximum value of image
 * @param output        Output image of 8-bit RGB pixels,
 *                      Dimensions must be equal to the Bayer image.
 */
void demosaic_malvar_rgb8(
        const U8 * const bayer,
        const demosaic_args * const args,
        demosaic_pix_rgb8 * output);

/** @brief Demosaic a row of a 16-bit bayer image into 8-bit rgb
 *         with malvar linear interpolation
 *
 *         Image dimensions must be positive, even, row must be within image
 *
 *         Row number is used to index within the image, but also determine
 *         whether the row number is even (red-green) or odd (green-blue)
 *         and also whether the row is on the edge and functions that do
 *         not sample from beyond the edge must be used.
 *
 *         This row function is publicly accessible so that operations
 *         that work row by row (like JPEG) can demosaic one row at a time.
 *
 *
 * @param bayer         An input 16-bit Bayer image
 * @param args          Dimensions, maximum value of image, how to shift
 * @param row           The row to be demosaic
 * @param output_row    Output row of 8-bit RGB pixels,
 *                      length must be equal to the width of the Bayer image.
 */
void demosaic_malvar_row_rgb16to8(
        const U16 * const bayer,
        const demosaic_args * const args,
        const I32 row,
        demosaic_pix_rgb8 output_row[]);

/** @brief Demosaic an 16-bit bayer image into 8-bit rgb
 *         with malvar linear interpolation
 *
 *         Image dimensions must be positive, even
 *
 * @param bayer         An input 16-bit Bayer image
 * @param args          Dimensions and maximum value of image
 * @param output        Output image of 8-bit RGB pixels,
 *                      Dimensions must be equal to the Bayer image.
 */
void demosaic_malvar_rgb16to8(
        const U16 * const bayer,
        const demosaic_args * const args,
        demosaic_pix_rgb8 * output);

/** @brief Demosaic a row of a 16-bit bayer image into 16-bit mono
 *         with malvar linear interpolation
 *
 *         Image dimensions must be positive, even, row must be within image
 *
 *         Row number is used to index within the image, but also determine
 *         whether the row number is even (red-green) or odd (green-blue)
 *         and also whether the row is on the edge and functions that do
 *         not sample from beyond the edge must be used.
 *
 *         This row function is publicly accessible so that operations
 *         that work row by row (like JPEG) can demosaic one row at a time.
 *
 * @param bayer         An input Bayer image
 * @param args          Dimensions, maximum value of image, luma coefficients
 * @param row           The row to be demosaic
 * @param output_row    Output row of mono pixels,
 *                      length must be equal to the width of the Bayer image.
 */
void demosaic_malvar_row_mono16(
        const U16 * const bayer,
        const demosaic_args * const args,
        const I32 row,
        U16 output_row[]);

/** @brief Demosaic a 16-bit bayer image into 16-bit mono
 *         with malvar linear interpolation
 *
 *         Image dimensions must be positive, even, row must be within image
 *
 * @param bayer         An input Bayer image
 * @param args          Dimensions, maximum value of image, luma coefficients
 * @param output        Output image of mono pixels,
 *                      dimensions must be equal to the Bayer image.
 */
void demosaic_malvar_mono16(
        const U16 * const bayer,
        const demosaic_args * const args,
        U16 * output);

/** @brief Demosaic a row of a 8-bit bayer image into 8-bit mono
 *         with malvar linear interpolation
 *
 *         Image dimensions must be positive, even, row must be within image
 *
 *         Row number is used to index within the image, but also determine
 *         whether the row number is even (red-green) or odd (green-blue)
 *         and also whether the row is on the edge and functions that do
 *         not sample from beyond the edge must be used.
 *
 *         This row function is publicly accessible so that operations
 *         that work row by row (like JPEG) can demosaic one row at a time.
 *
 * @param bayer         An input Bayer image
 * @param args          Dimensions, maximum value of image, luma coefficients
 * @param row           The row to be demosaic
 * @param output_row    Output row of mono pixels,
 *                      length must be equal to the width of the Bayer image.
 */
void demosaic_malvar_row_mono8(
        const U8 * const bayer,
        const demosaic_args * const args,
        const I32 row,
        U8 output_row[]);

/** @brief Demosaic an 8-bit bayer image into 8-bit mono
 *         with malvar linear interpolation
 *
 *         Image dimensions must be positive, even, row must be within image
 *
 * @param bayer         An input Bayer image
 * @param args          Dimensions, maximum value of image, luma coefficients
 * @param output        Output image of mono pixels,
 *                      dimensions must be equal to the Bayer image.
 */
void demosaic_malvar_mono8(
        const U8 * const bayer,
        const demosaic_args * const args,
        U8 * output);

/** @brief Demosaic a row of a 16-bit bayer image into 8-bit mono
 *         with malvar linear interpolation
 *
 *         Image dimensions must be positive, even, row must be within image
 *
 *         Row number is used to index within the image, but also determine
 *         whether the row number is even (red-green) or odd (green-blue)
 *         and also whether the row is on the edge and functions that do
 *         not sample from beyond the edge must be used.
 *
 *         This row function is publicly accessible so that operations
 *         that work row by row (like JPEG) can demosaic one row at a time.
 *
 *
 * @param bayer         An input Bayer image
 * @param args          Dimensions, maximum value of image, luma coefs, shift
 * @param row           The row to be demosaic
 * @param output_row    Output row of mono pixels,
 *                      length must be equal to the width of the Bayer image.
 */
void demosaic_malvar_row_mono16to8(
        const U16 * const bayer,
        const demosaic_args * const args,
        const I32 row,
        U8 output_row[]);

/** @brief Demosaic a 16-bit bayer image into 8-bit mono
 *         with malvar linear interpolation
 *
 *         Image dimensions must be positive, even, row must be within image
 *
 * @param bayer         An input Bayer image
 * @param args          Dimensions, maximum value of image, luma coefficients
 * @param output        Output image of mono pixels,
 *                      dimensions must be equal to the Bayer image.
 */
void demosaic_malvar_mono16to8(
        const U16 * const bayer,
        const demosaic_args * const args,
        U8 * output);

#ifdef __cplusplus
   }
#endif

#endif // DEMOSAIC_PUB_H
