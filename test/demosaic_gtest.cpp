#include <stdio.h>

#define STATIC

#include "gtest/gtest.h"
#include <time.h>

#include <demosaic/demosaic_pub.h>

extern "C"
{
#include "imageio.h"
#include "basic.h"

extern void demosaic_malvar_row_rgb16_unoptimized(
        const U16 * const bayer,
        const demosaic_args *args,
        const int row,
        demosaic_pix_rgb16 output_row[]);

extern void demosaic_malvar_row_rgb8_unoptimized(
        const U8 * const bayer,
        const demosaic_args *args,
        const int row,
        demosaic_pix_rgb8 output_row[]);

extern void demosaic_malvar_row_rgb16to8_unoptimized(
        const U16 * const bayer,
        const demosaic_args *args,
        const int row,
        demosaic_pix_rgb8 output_row[]);

extern void demosaic_malvar_row_mono16_unoptimized(
        const U16 * const bayer,
        const demosaic_args *args,
        const int row,
        U16 output_row[]);

extern void demosaic_malvar_row_mono8_unoptimized(
        const U8 * const bayer,
        const demosaic_args *args,
        const int row,
        U8 output_row[]);

extern void demosaic_malvar_row_mono16to8_unoptimized(
        const U16 * const bayer,
        const demosaic_args *args,
        const int row,
        U8 output_row[]);
} // extern C

#define ABS(x) ( (x < 0) ? -(x) : (x) )

double time_rgb_unoptimized = 0;
double time_rgb_optimized = 0;
double time_mono_unoptimized = 0;
double time_mono_optimized = 0;

// whether to print image buffers
bool print_images = true;

int performance_iters = 5;

demosaic_pix_rgb16 * image_truth16;
demosaic_pix_rgb8 * image_truth8;
U16 * bayer16;
U8 *  bayer8;
demosaic_pix_rgb16 * image_out_rgb16;
demosaic_pix_rgb8 *  image_out_rgb8;
demosaic_pix_rgb8 *  image_out_rgb8from16;
U16* image_out_mono16;
U8*  image_out_mono8;
U8*  image_out_mono8from16;
demosaic_pix_rgb16 * image_out_rgb16_unopt;
demosaic_pix_rgb8 *  image_out_rgb8_unopt;
demosaic_pix_rgb8 *  image_out_rgb8from16_unopt;
U16* image_out_mono16_unopt;
U8*  image_out_mono8_unopt;
U8*  image_out_mono8from16_unopt;



void alloc_global_bufs(int n_rows, int n_cols)
{
    image_truth16 = (demosaic_pix_rgb16*) malloc(
            n_rows * n_cols * sizeof(demosaic_pix_rgb16));
    ASSERT_TRUE(image_truth16 != NULL);
    image_truth8 = (demosaic_pix_rgb8*) malloc(
            n_rows * n_cols * sizeof(demosaic_pix_rgb8));
    ASSERT_TRUE(image_truth8 != NULL);

    bayer16 = (U16 *) malloc(n_rows * n_cols * sizeof(U16));
    ASSERT_TRUE(bayer16 != NULL);
    bayer8 = (U8 *) malloc(n_rows * n_cols * sizeof(U8));
    ASSERT_TRUE(bayer8 != NULL);


    image_out_rgb16 = (demosaic_pix_rgb16*) malloc(
            n_rows * n_cols * sizeof(demosaic_pix_rgb16));
    ASSERT_TRUE(image_out_rgb16 != NULL);
    image_out_rgb8 = (demosaic_pix_rgb8*) malloc(
            n_rows * n_cols * sizeof(demosaic_pix_rgb8));
    ASSERT_TRUE(image_out_rgb8 != NULL);
    image_out_rgb8from16 = (demosaic_pix_rgb8*) malloc(
            n_rows * n_cols * sizeof(demosaic_pix_rgb8));
    ASSERT_TRUE(image_out_rgb8from16 != NULL);

    image_out_mono16 = (U16*) malloc(n_rows * n_cols * sizeof(U16));
    ASSERT_TRUE(image_out_mono16 != NULL);
    image_out_mono8 = (U8*) malloc(n_rows * n_cols * sizeof(U8));
    ASSERT_TRUE(image_out_mono8 != NULL);
    image_out_mono8from16 = (U8*) malloc(n_rows * n_cols * sizeof(U8));
    ASSERT_TRUE(image_out_mono8from16 != NULL);

    image_out_rgb16_unopt = (demosaic_pix_rgb16*) malloc(
            n_rows * n_cols * sizeof(demosaic_pix_rgb16));
    ASSERT_TRUE(image_out_rgb16_unopt != NULL);
    image_out_rgb8_unopt = (demosaic_pix_rgb8*) malloc(
            n_rows * n_cols * sizeof(demosaic_pix_rgb8));
    ASSERT_TRUE(image_out_rgb8_unopt != NULL);
    image_out_rgb8from16_unopt = (demosaic_pix_rgb8*) malloc(
            n_rows * n_cols * sizeof(demosaic_pix_rgb8));
    ASSERT_TRUE(image_out_rgb8from16_unopt != NULL);

    image_out_mono16_unopt = (U16*) malloc(n_rows * n_cols * sizeof(U16));
    ASSERT_TRUE(image_out_mono16_unopt != NULL);
    image_out_mono8_unopt = (U8*) malloc(n_rows * n_cols * sizeof(U8));
    ASSERT_TRUE(image_out_mono8_unopt != NULL);
    image_out_mono8from16_unopt = (U8*) malloc(n_rows * n_cols * sizeof(U8));
    ASSERT_TRUE(image_out_mono8from16_unopt != NULL);
}

void free_global_bufs(void)
{
    free(image_truth16);
    free(image_truth8);
    free(bayer16);
    free(bayer8);
    free(image_out_rgb16);
    free(image_out_rgb8);
    free(image_out_rgb8from16);
    free(image_out_mono16);
    free(image_out_mono8);
    free(image_out_mono8from16);
    free(image_out_rgb16_unopt);
    free(image_out_rgb8_unopt);
    free(image_out_rgb8from16_unopt);
    free(image_out_mono16_unopt);
    free(image_out_mono8_unopt);
    free(image_out_mono8from16_unopt);
}

void make_single_color_input(
        U16 red, U16 green, U16 blue,
        demosaic_args * args)
{
    printf("make single color input: r: %u g: %u b: %u shift: %d\n",
            red, green, blue, args->rshift);
    int n_rows = args->n_rows;
    int n_cols = args->n_cols;
    int rshift = args->rshift;
    for (int row = 0; row < n_rows; row++) {
        for (int col = 0; col < n_cols; col++) {
            image_truth16[(row*n_cols)+col].red = red;
            image_truth16[(row*n_cols)+col].green = green;
            image_truth16[(row*n_cols)+col].blue = blue;
            image_truth8[(row*n_cols)+col].red = red >> rshift;
            image_truth8[(row*n_cols)+col].green = green >> rshift;
            image_truth8[(row*n_cols)+col].blue = blue >> rshift;
            if(row % 2 == 0 && col % 2 == 0) {
                bayer16[(row*n_cols)+col] = red;
                bayer8[(row*n_cols)+col] = red >> rshift;
            } else if(row % 2 == 1 && col % 2 == 1) {
                bayer16[(row*n_cols)+col] = blue;
                bayer8[(row*n_cols)+col] = blue >> rshift;
            } else {
                bayer16[(row*n_cols)+col] = green;
                bayer8[(row*n_cols)+col] = green >> rshift;
            }
        }
    }
}

void make_random_input(
        demosaic_args * args)
{
    U16 red;
    U16 green;
    U16 blue;
    U16 max_val = args->max_val;
    int n_rows = args->n_rows;
    int n_cols = args->n_cols;
    int rshift = args->rshift;
    for (int row = 0; row < n_rows; row++) {
        for (int col = 0; col < n_cols; col++) {
            red = rand() % max_val;
            green = rand() % max_val;
            blue = rand() % max_val;
            image_truth16[(row*n_cols)+col].red = red;
            image_truth16[(row*n_cols)+col].green = green;
            image_truth16[(row*n_cols)+col].blue = blue;
            image_truth8[(row*n_cols)+col].red = red;
            image_truth8[(row*n_cols)+col].green = green;
            image_truth8[(row*n_cols)+col].blue = blue;
            if(row % 2 == 0 && col % 2 == 0) {
                bayer16[(row*n_cols)+col] = red;
                bayer8[(row*n_cols)+col] = red >> rshift;
            } else if(row % 2 == 1 && col % 2 == 1) {
                bayer16[(row*n_cols)+col] = blue;
                bayer8[(row*n_cols)+col] = blue >> rshift;
            } else {
                bayer16[(row*n_cols)+col] = green;
                bayer8[(row*n_cols)+col] = green >> rshift;
            }
        }
    }
}

void do_demosaicing(
        demosaic_args * args)
{
    int n_rows = args->n_rows;
    int n_cols = args->n_cols;
    demosaic_args args8 = *args;
    args8.max_val = 0xFF;

    clock_t t;

    t = clock();
    demosaic_malvar_rgb16(bayer16, args, image_out_rgb16);
    t = clock() - t;
    time_rgb_optimized += ((double)t)/CLOCKS_PER_SEC;

    demosaic_malvar_rgb8(bayer8, &args8, image_out_rgb8);

    demosaic_malvar_rgb16to8(bayer16, args, image_out_rgb8from16);

    t = clock();
    demosaic_malvar_mono16(bayer16, args, image_out_mono16);
    t = clock() - t;
    time_mono_optimized += ((double) t) / CLOCKS_PER_SEC;

    demosaic_malvar_mono8(bayer8, &args8, image_out_mono8);

    demosaic_malvar_mono16to8(bayer16, args, image_out_mono8from16);

    t = clock();
    for (int row = 0; row < args->n_rows; row++) {
        demosaic_malvar_row_rgb16_unoptimized(bayer16, args, row,
                &image_out_rgb16_unopt[row * args->n_cols]);
    }
    t = clock() - t;
    time_rgb_unoptimized += ((double)t)/CLOCKS_PER_SEC;

    for (int row = 0; row < args->n_rows; row++) {
        demosaic_malvar_row_rgb8_unoptimized(bayer8, &args8, row,
                &image_out_rgb8_unopt[row * args->n_cols]);
    }

    for (int row = 0; row < args->n_rows; row++) {
        demosaic_malvar_row_rgb16to8_unoptimized(bayer16, args, row,
                &image_out_rgb8from16_unopt[row * args->n_cols]);
    }


    t = clock();
    for (int row = 0; row < args->n_rows; row++) {
        demosaic_malvar_row_mono16_unoptimized(bayer16, args, row,
                &image_out_mono16_unopt[row * args->n_cols]);
    }
    t = clock() - t;
    time_mono_unoptimized += ((double) t) / CLOCKS_PER_SEC;

    for (int row = 0; row < args->n_rows; row++) {
        demosaic_malvar_row_mono8_unoptimized(bayer8, &args8, row,
                &image_out_mono8_unopt[row * args->n_cols]);
    }

    for (int row = 0; row < args->n_rows; row++) {
        demosaic_malvar_row_mono16to8_unoptimized(bayer16, args, row,
                &image_out_mono8from16_unopt[row * args->n_cols]);
    }


    if(print_images) {
        printf("input image:\n");
        for(int row = 0; row < 4; row++) {
            for(int col = 0; col < 4; col++) {
                printf("%04u ", bayer16[row*n_cols+col]);
            }
            printf("... ");
            for(int col = n_cols-4; col < n_cols; col++) {
                printf("%04u ", bayer16[row*n_cols+col]);
            }
            printf("\n");
        }
        printf("  :    :    :    :  :::   :    :    :    :  \n");
        for(int row = n_rows-4; row < n_rows; row++) {
            for(int col = 0; col < 4; col++) {
                printf("%04u ", bayer16[row*n_cols+col]);
            }
            printf("... ");
            for(int col = n_cols-4; col < n_cols; col++) {
                printf("%04u ", bayer16[row*n_cols+col]);
            }
            printf("\n");
        }


        printf("\nrgb image out:\n");
        for(int row = 0; row < 4; row++) {
            for(int col = 0; col < 4; col++) {
                printf("(%04u %04u %04u) ",
                        image_out_rgb16[row*n_cols+col].red,
                        image_out_rgb16[row*n_cols+col].green,
                        image_out_rgb16[row*n_cols+col].blue);
            }
            printf("... ");
            for(int col = n_cols-4; col < n_cols; col++) {
                printf("(%04u %04u %04u) ",
                        image_out_rgb16[row*n_cols+col].red,
                        image_out_rgb16[row*n_cols+col].green,
                        image_out_rgb16[row*n_cols+col].blue);
            }
            printf("\n");
        }
        printf("        :                :                :               :         "
                ":::"
                "         :               :                :                :       \n");
        for(int row = n_rows-4; row < n_rows; row++) {
            for(int col = 0; col < 4; col++) {
                printf("(%04u %04u %04u) ",
                        image_out_rgb16[row*n_cols+col].red,
                        image_out_rgb16[row*n_cols+col].green,
                        image_out_rgb16[row*n_cols+col].blue);
            }
            printf("... ");
            for(int col = n_cols-4; col < n_cols; col++) {
                printf("(%04u %04u %04u) ",
                        image_out_rgb16[row*n_cols+col].red,
                        image_out_rgb16[row*n_cols+col].green,
                        image_out_rgb16[row*n_cols+col].blue);
            }
            printf("\n");
        }

        printf("\nrgb 8 image out:\n");
        for(int row = 0; row < 4; row++) {
            for(int col = 0; col < 4; col++) {
                printf("(%04u %04u %04u) ",
                        image_out_rgb8[row*n_cols+col].red,
                        image_out_rgb8[row*n_cols+col].green,
                        image_out_rgb8[row*n_cols+col].blue);
            }
            printf("... ");
            for(int col = n_cols-4; col < n_cols; col++) {
                printf("(%04u %04u %04u) ",
                        image_out_rgb8[row*n_cols+col].red,
                        image_out_rgb8[row*n_cols+col].green,
                        image_out_rgb8[row*n_cols+col].blue);
            }
            printf("\n");
        }
        printf("        :                :                :               :         "
                ":::"
                "         :               :                :                :       \n");
        for(int row = n_rows-4; row < n_rows; row++) {
            for(int col = 0; col < 4; col++) {
                printf("(%04u %04u %04u) ",
                        image_out_rgb8[row*n_cols+col].red,
                        image_out_rgb8[row*n_cols+col].green,
                        image_out_rgb8[row*n_cols+col].blue);
            }
            printf("... ");
            for(int col = n_cols-4; col < n_cols; col++) {
                printf("(%04u %04u %04u) ",
                        image_out_rgb8[row*n_cols+col].red,
                        image_out_rgb8[row*n_cols+col].green,
                        image_out_rgb8[row*n_cols+col].blue);
            }
            printf("\n");
        }

        printf("\nrgb 8from16 image out:\n");
        for(int row = 0; row < 4; row++) {
            for(int col = 0; col < 4; col++) {
                printf("(%04u %04u %04u) ",
                        image_out_rgb8from16[row*n_cols+col].red,
                        image_out_rgb8from16[row*n_cols+col].green,
                        image_out_rgb8from16[row*n_cols+col].blue);
            }
            printf("... ");
            for(int col = n_cols-4; col < n_cols; col++) {
                printf("(%04u %04u %04u) ",
                        image_out_rgb8from16[row*n_cols+col].red,
                        image_out_rgb8from16[row*n_cols+col].green,
                        image_out_rgb8from16[row*n_cols+col].blue);
            }
            printf("\n");
        }
        printf("        :                :                :               :         "
                ":::"
                "         :               :                :                :       \n");
        for(int row = n_rows-4; row < n_rows; row++) {
            for(int col = 0; col < 4; col++) {
                printf("(%04u %04u %04u) ",
                        image_out_rgb8from16[row*n_cols+col].red,
                        image_out_rgb8from16[row*n_cols+col].green,
                        image_out_rgb8from16[row*n_cols+col].blue);
            }
            printf("... ");
            for(int col = n_cols-4; col < n_cols; col++) {
                printf("(%04u %04u %04u) ",
                        image_out_rgb8from16[row*n_cols+col].red,
                        image_out_rgb8from16[row*n_cols+col].green,
                        image_out_rgb8from16[row*n_cols+col].blue);
            }
            printf("\n");
        }



        printf("\nmono 16 image out:\n");
        for(int row = 0; row < 4; row++) {
            for(int col = 0; col < 4; col++) {
                printf("%04u ", image_out_mono16[row*n_cols+col]);
            }
            printf("... ");
            for(int col = n_cols-4; col < n_cols; col++) {
                printf("%04u ", image_out_mono16[row*n_cols+col]);
            }
            printf("\n");
        }
        printf("  :    :    :    :  :::   :    :    :    :  \n");
        for(int row = n_rows-4; row < n_rows; row++) {
            for(int col = 0; col < 4; col++) {
                printf("%04u ", image_out_mono16[row*n_cols+col]);
            }
            printf("... ");
            for(int col = n_cols-4; col < n_cols; col++) {
                printf("%04u ", image_out_mono16[row*n_cols+col]);
            }
            printf("\n");
        }

        printf("\nmono 8 image out:\n");
        for(int row = 0; row < 4; row++) {
            for(int col = 0; col < 4; col++) {
                printf("%03u ", (unsigned int)image_out_mono8[row*n_cols+col]);
            }
            printf("... ");
            for(int col = n_cols-4; col < n_cols; col++) {
                printf("%03u ", (unsigned int)image_out_mono8[row*n_cols+col]);
            }
            printf("\n");
        }
        printf(" :   :   :   :  :::   :   :   :   :  \n");
        for(int row = n_rows-4; row < n_rows; row++) {
            for(int col = 0; col < 4; col++) {
                printf("%03u ", (unsigned int)image_out_mono8[row*n_cols+col]);
            }
            printf("... ");
            for(int col = n_cols-4; col < n_cols; col++) {
                printf("%03u ", (unsigned int)image_out_mono8[row*n_cols+col]);
            }
            printf("\n");
        }

        printf("\nmono 8 image out unopt:\n");
        for(int row = 0; row < 4; row++) {
            for(int col = 0; col < 4; col++) {
                printf("%03u ", (unsigned int)image_out_mono8_unopt[row*n_cols+col]);
            }
            printf("... ");
            for(int col = n_cols-4; col < n_cols; col++) {
                printf("%03u ", (unsigned int)image_out_mono8_unopt[row*n_cols+col]);
            }
            printf("\n");
        }
        printf(" :   :   :   :  :::   :   :   :   :  \n");
        for(int row = n_rows-4; row < n_rows; row++) {
            for(int col = 0; col < 4; col++) {
                printf("%03u ", (unsigned int)image_out_mono8_unopt[row*n_cols+col]);
            }
            printf("... ");
            for(int col = n_cols-4; col < n_cols; col++) {
                printf("%03u ", (unsigned int)image_out_mono8_unopt[row*n_cols+col]);
            }
            printf("\n");
        }


        printf("\nmono 16 to 8 image out:\n");
        for(int row = 0; row < 4; row++) {
            for(int col = 0; col < 4; col++) {
                printf("%03u ", (unsigned int)image_out_mono8from16[row*n_cols+col]);
            }
            printf("... ");
            for(int col = n_cols-4; col < n_cols; col++) {
                printf("%03u ", (unsigned int)image_out_mono8from16[row*n_cols+col]);
            }
            printf("\n");
        }
        printf(" :   :   :   :  :::   :   :   :   :  \n");
        for(int row = n_rows-4; row < n_rows; row++) {
            for(int col = 0; col < 4; col++) {
                printf("%03u ", (unsigned int)image_out_mono8from16[row*n_cols+col]);
            }
            printf("... ");
            for(int col = n_cols-4; col < n_cols; col++) {
                printf("%03u ", (unsigned int)image_out_mono8from16[row*n_cols+col]);
            }
            printf("\n");
        }
    }
}

// test optimized and unoptimized functions returned same values,
// and that values are less thatn error bound
void check_demosaicing_error(
        demosaic_args * args,
        double rms_error_bound) {

    int n_rows = args->n_rows;
    int n_cols = args->n_cols;
    int rshift = args->rshift;

    int error = 0;
    double sum_error = 0;
    double sum_sq_error = 0;
    double mean_error = 0;
    double rms_error = 0;
    int samples = 0;
    int max_error = -1;
    int max_error_row = -1;
    int max_error_col = -1;
    int max_error_channel = -1;

    sum_error = 0;
    sum_sq_error = 0;
    max_error = -1;
    max_error_row = -1;
    max_error_col = -1;
    max_error_channel = -1;
    // don't test edges
    for (int row = 2; row < n_rows - 2; row++) {
        for (int col = 2; col < n_cols - 2; col++) {
            error = (image_out_rgb16[row * n_cols + col].red
                     - image_truth16[row * n_cols + col].red);
            sum_error += error;
            sum_sq_error += error * error;
            if(ABS(error) > max_error) {
                max_error = ABS(error);
                max_error_row = row;
                max_error_col = col;
                max_error_channel = 0;
            }
            error = (image_out_rgb16[row * n_cols + col].green
                     - image_truth16[row * n_cols + col].green);
            sum_error += error;
            sum_sq_error += error * error;
            if(ABS(error) > max_error) {
                max_error = ABS(error);
                max_error_row = row;
                max_error_col = col;
                max_error_channel = 1;
            }
            error = (image_out_rgb16[row * n_cols + col].blue
                     - image_truth16[row * n_cols + col].blue);
            sum_error += error;
            sum_sq_error += error * error;
            if(ABS(error) > max_error) {
                max_error = ABS(error);
                max_error_row = row;
                max_error_col = col;
                max_error_channel = 2;
            }
            samples += 3;
        }
    }
    mean_error = sum_error / samples;
    rms_error = sqrt( sum_sq_error / samples);
    printf("rgb 16 rms err = %f, mean_error = %f, rms_error_bound = %f. "
            "max error of %d at row %d, col %d, ch %d.\n",
            rms_error, mean_error, rms_error_bound,
            max_error, max_error_row, max_error_col, max_error_channel);
    EXPECT_LE(rms_error, rms_error_bound);

    sum_error = 0;
    sum_sq_error = 0;
    max_error = -1;
    max_error_row = -1;
    max_error_col = -1;
    max_error_channel = -1;
    // don't test edges
    for (int row = 2; row < n_rows - 2; row++) {
        for (int col = 2; col < n_cols - 2; col++) {
            error = (image_out_rgb8[row * n_cols + col].red
                     - image_truth8[row * n_cols + col].red);
            sum_error += error;
            sum_sq_error += error * error;
            if(ABS(error) > max_error) {
                max_error = ABS(error);
                max_error_row = row;
                max_error_col = col;
                max_error_channel = 0;
            }
            error = (image_out_rgb8[row * n_cols + col].green
                     - image_truth8[row * n_cols + col].green);
            sum_error += error;
            sum_sq_error += error * error;
            if(ABS(error) > max_error) {
                max_error = ABS(error);
                max_error_row = row;
                max_error_col = col;
                max_error_channel = 1;
            }
            error = (image_out_rgb8[row * n_cols + col].blue
                     - image_truth8[row * n_cols + col].blue);
            sum_error += error;
            sum_sq_error += error * error;
            if(ABS(error) > max_error) {
                max_error = ABS(error);
                max_error_row = row;
                max_error_col = col;
                max_error_channel = 2;
            }
            samples += 3;
        }
    }
    mean_error = sum_error / samples;
    rms_error = sqrt( sum_sq_error / samples);
    printf("rgb 8 rms err = %f, mean_error = %f, rms_error_bound = %f. "
            "max error of %d at row %d, col %d, ch %d.\n",
            rms_error, mean_error, rms_error_bound/16.0,
            max_error, max_error_row, max_error_col, max_error_channel);
    EXPECT_LE(rms_error, rms_error_bound/16.0);

    sum_error = 0;
    sum_sq_error = 0;
    max_error = -1;
    max_error_row = -1;
    max_error_col = -1;
    max_error_channel = -1;
    // don't test edges
    for (int row = 2; row < n_rows - 2; row++) {
        for (int col = 2; col < n_cols - 2; col++) {
            error = image_out_rgb8from16[row * n_cols + col].red
                     - image_truth8[row * n_cols + col].red;
            sum_error += error;
            sum_sq_error += error * error;
            if(ABS(error) > max_error) {
                max_error = ABS(error);
                max_error_row = row;
                max_error_col = col;
                max_error_channel = 0;
            }
            error = image_out_rgb8from16[row * n_cols + col].green
                     - image_truth8[row * n_cols + col].green;
            sum_error += error;
            sum_sq_error += error * error;
            if(ABS(error) > max_error) {
                max_error = ABS(error);
                max_error_row = row;
                max_error_col = col;
                max_error_channel = 1;
            }
            error = image_out_rgb8from16[row * n_cols + col].blue
                     - image_truth8[row * n_cols + col].blue;
            sum_error += error;
            sum_sq_error += error * error;
            if(ABS(error) > max_error) {
                max_error = ABS(error);
                max_error_row = row;
                max_error_col = col;
                max_error_channel = 2;
            }
            samples += 3;
        }
    }
    mean_error = sum_error / samples;
    rms_error = sqrt( sum_sq_error / samples);
    printf("rgb 8 from 16 rms err = %f, mean_error = %f, "
            "rms_error_bound = %f. "
            "max error of %d at row %d, col %d, ch %d.\n",
            rms_error, mean_error, rms_error_bound/16.0,
            max_error, max_error_row, max_error_col, max_error_channel);
    EXPECT_LE(rms_error, rms_error_bound/16.0);


    sum_error = 0;
    sum_sq_error = 0;
    max_error = -1;
    max_error_row = -1;
    max_error_col = -1;
    max_error_channel = -1;
    for (int row = 2; row < n_rows-2; row++) {
        for (int col = 2; col < n_cols-2; col++) {
            error = (image_out_mono16[row*n_cols+ col] -
                     (image_truth16[row * n_cols + col].red * args->coefs.red
                      + image_truth16[row * n_cols + col].green * args->coefs.green
                      + image_truth16[row * n_cols + col].blue * args->coefs.blue));
            sum_error += error;
            sum_sq_error += error*error;
            if(ABS(error) > max_error) {
                max_error = ABS(error);
                max_error_row = row;
                max_error_col = col;
            }
            samples++;
        }
    }
    mean_error = sum_error / samples;
    rms_error = sqrt( sum_sq_error / samples);
    printf("mono 16 rms err = %f, mean_error = %f, rms_error_bound = %f. "
            "max error of %d at row %d, col %d.\n",
            rms_error, mean_error, rms_error_bound,
            max_error, max_error_row, max_error_col);
    EXPECT_LE(rms_error, rms_error_bound);

    sum_error = 0;
    sum_sq_error = 0;
    max_error = -1;
    max_error_row = -1;
    max_error_col = -1;
    max_error_channel = -1;
    for (int row = 2; row < n_rows-2; row++) {
        for (int col = 2; col < n_cols-2; col++) {
            error = (image_out_mono8[row*n_cols+ col] -
                     (image_truth8[row * n_cols + col].red * args->coefs.red
                      + image_truth8[row * n_cols + col].green * args->coefs.green
                      + image_truth8[row * n_cols + col].blue * args->coefs.blue));
            sum_error += error;
            sum_sq_error += error*error;
            if(ABS(error) > max_error) {
                max_error = ABS(error);
                max_error_row = row;
                max_error_col = col;
            }
            samples++;
        }
    }
    mean_error = sum_error / samples;
    rms_error = sqrt( sum_sq_error / samples);
    printf("mono 8 rms err = %f, mean_error = %f, rms_error_bound = %f. "
            "max error of %d at row %d, col %d.\n",
            rms_error, mean_error, rms_error_bound/16.0,
            max_error, max_error_row, max_error_col);
    EXPECT_LE(rms_error, rms_error_bound/16.0);

    sum_error = 0;
    sum_sq_error = 0;
    max_error = -1;
    max_error_row = -1;
    max_error_col = -1;
    max_error_channel = -1;
    for (int row = 2; row < n_rows-2; row++) {
        for (int col = 2; col < n_cols-2; col++) {
            error = (image_out_mono8from16[row*n_cols+ col] -
                     (image_truth8[row * n_cols + col].red * args->coefs.red
                      + image_truth8[row * n_cols + col].green * args->coefs.green
                      + image_truth8[row * n_cols + col].blue * args->coefs.blue));
            sum_error += error;
            sum_sq_error += error*error;
            if(ABS(error) > max_error) {
                max_error = ABS(error);
                max_error_row = row;
                max_error_col = col;
            }
            samples++;
        }
    }
    mean_error = sum_error / samples;
    rms_error = sqrt( sum_sq_error / samples);
    printf("mono 8 from 16, rms err = %f, mean_error = %f, "
            "rms_error_bound = %f. "
            "max error of %d at row %d, col %d.\n",
            rms_error, mean_error, rms_error_bound/16.0,
            max_error, max_error_row, max_error_col);
    EXPECT_LE(rms_error, rms_error_bound/16.0);




    // check optimized and unoptimized return the same values
    for (int row = 0; row < n_rows; row++) {
        for (int col = 0; col < n_cols; col++) {
            if ((image_out_rgb16_unopt[row * n_cols + col].red !=
                 image_out_rgb16[row * n_cols + col].red)
                ||
                (image_out_rgb16_unopt[row * n_cols + col].green !=
                 image_out_rgb16[row * n_cols + col].green)
                ||
                (image_out_rgb16_unopt[row * n_cols + col].blue !=
                 image_out_rgb16[row * n_cols + col].blue)
                ||
                (image_out_rgb8_unopt[row * n_cols + col].red !=
                 image_out_rgb8[row * n_cols + col].red)
                ||
                (image_out_rgb8_unopt[row * n_cols + col].green !=
                 image_out_rgb8[row * n_cols + col].green)
                ||
                (image_out_rgb8_unopt[row * n_cols + col].blue !=
                 image_out_rgb8[row * n_cols + col].blue)
                ||
                (image_out_rgb8from16_unopt[row * n_cols + col].red !=
                 image_out_rgb8from16[row * n_cols + col].red)
                ||
                (image_out_rgb8from16_unopt[row * n_cols + col].green !=
                 image_out_rgb8from16[row * n_cols + col].green)
                ||
                (image_out_rgb8from16_unopt[row * n_cols + col].blue !=
                 image_out_rgb8from16[row * n_cols + col].blue)
                ||
                (image_out_mono16_unopt[row * n_cols + col] !=
                 image_out_mono16[row * n_cols + col])
                ||
                (image_out_mono8_unopt[row * n_cols + col] !=
                 image_out_mono8[row * n_cols + col])
                ||
                (image_out_mono8from16_unopt[row * n_cols + col] !=
                 image_out_mono8from16[row * n_cols + col])) {
                printf("bad val at row %d col %d\n", row, col);
            }



            ASSERT_EQ(image_out_rgb16_unopt[row * n_cols + col].red,
                    image_out_rgb16[row * n_cols + col].red);
            ASSERT_EQ(image_out_rgb16_unopt[row * n_cols + col].green,
                    image_out_rgb16[row * n_cols + col].green);
            ASSERT_EQ(image_out_rgb16_unopt[row * n_cols + col].blue,
                    image_out_rgb16[row * n_cols + col].blue);
            ASSERT_EQ(image_out_rgb8_unopt[row * n_cols + col].red,
                    image_out_rgb8[row * n_cols + col].red);
            ASSERT_EQ(image_out_rgb8_unopt[row * n_cols + col].green,
                    image_out_rgb8[row * n_cols + col].green);
            ASSERT_EQ(image_out_rgb8_unopt[row * n_cols + col].blue,
                    image_out_rgb8[row * n_cols + col].blue);
            ASSERT_EQ(image_out_rgb8from16_unopt[row * n_cols + col].red,
                    image_out_rgb8from16[row * n_cols + col].red);
            ASSERT_EQ(image_out_rgb8from16_unopt[row * n_cols + col].green,
                    image_out_rgb8from16[row * n_cols + col].green);
            ASSERT_EQ(image_out_rgb8from16_unopt[row * n_cols + col].blue,
                    image_out_rgb8from16[row * n_cols + col].blue);
            ASSERT_EQ(image_out_mono16_unopt[row * n_cols + col],
                    image_out_mono16[row * n_cols + col]);
            ASSERT_EQ(image_out_mono8_unopt[row * n_cols + col],
                    image_out_mono8[row * n_cols + col]);
            ASSERT_EQ(image_out_mono8from16_unopt[row * n_cols + col],
                    image_out_mono8from16[row * n_cols + col]);
        }
    }

}

void test_demosaicing(
        demosaic_args * args,
        double rms_error_bound) {

    do_demosaicing(args);

    check_demosaicing_error(args, rms_error_bound);
}

void test_demosaicing_single_color(
        U16 red, U16 green, U16 blue,
        demosaic_args * args)
{
    printf("single color image r=%u, g=%u, b=%u\n", red, green, blue);

    make_single_color_input(red, green, blue, args);

    test_demosaicing(args, 1.0);


}

void test_demosaicing_random_image(
        demosaic_args * args)
{
//    int n_rows = args->n_rows;
//    int n_cols = args->n_cols;

    make_random_input(args);

    test_demosaicing(args, args->max_val/2.0);
}

TEST(DemosaicTest, SingleColors) {
    int n_rows = 480;
    int n_cols = 480;
    int n_pix = n_rows*n_cols;
    unsigned int max_val = 0x0FFF;

    alloc_global_bufs(n_rows, n_cols);

    demosaic_args args;
    args.n_rows = n_rows;
    args.n_cols = n_cols;
    args.max_val = max_val;
    args.rshift = 4;
    args.coefs = {0.299, 0.587, 0.114};    // ccir 601 formula

    printf("\nall black image\n");
    test_demosaicing_single_color(0,0,0, &args);

    printf("\nall white image\n");
    test_demosaicing_single_color(max_val,max_val,max_val, &args);

    printf("\nall grey image\n");
    test_demosaicing_single_color(max_val/2,max_val/2,max_val/2, &args);


    printf("\nred image\n");
    test_demosaicing_single_color(max_val,0,0, &args);

    printf("\ngreen image\n");
    test_demosaicing_single_color(0,max_val,0, &args);

    printf("\nblue image\n");
    test_demosaicing_single_color(0,0,max_val, &args);

    printf("\nyellow image\n");
    test_demosaicing_single_color(max_val,max_val,0, &args);

    printf("\ncyan image\n");
    test_demosaicing_single_color(0,max_val,max_val, &args);

    printf("\nmagenta image\n");
    test_demosaicing_single_color(max_val,0,max_val, &args);

    free_global_bufs();
}

TEST(DemosaicTest, Random) {
    int n_rows = 480;
    int n_cols = 480;
    int n_pix = n_rows*n_cols;
    unsigned int max_val = 0x0FFF;

    alloc_global_bufs(n_rows, n_cols);

    demosaic_args args;
    args.n_rows = n_rows;
    args.n_cols = n_cols;
    args.max_val = max_val;
    args.rshift = 4;
    args.coefs = {0.299, 0.587, 0.114};     // ccir 601 formula

    test_demosaicing_random_image(&args);

    free_global_bufs();

}

TEST(DemosaicTest, Performance) {
    int n_rows = 960;
    int n_cols = 960;
    int n_pix = n_rows*n_cols;
    unsigned int max_val = 0x0FFF;

    alloc_global_bufs(n_rows, n_cols);

    demosaic_args args;
    args.n_rows = n_rows;
    args.n_cols = n_cols;
    args.max_val = max_val;
    args.rshift = 4;
    args.coefs = {0.299, 0.587, 0.114}; // ccir 601 formula

    time_rgb_unoptimized = 0;
    time_rgb_optimized = 0;
    time_mono_unoptimized = 0;
    time_mono_optimized = 0;
    for (int i = 0; i < performance_iters; i++) {
        test_demosaicing_random_image(&args);
    }
    time_rgb_unoptimized = time_rgb_unoptimized/performance_iters;
    time_rgb_optimized = time_rgb_optimized/performance_iters;
    time_mono_unoptimized = time_mono_unoptimized/performance_iters;
    time_mono_optimized = time_mono_optimized/performance_iters;

    printf("average times: rgb unopt: %f s, rgb opt: %f s, %f x speedup. "
            "mono unopt: %f s, mono opt: %f s, %f x speedup.\n",
            time_rgb_unoptimized, time_rgb_optimized,
            time_rgb_unoptimized/time_rgb_optimized,
            time_mono_unoptimized, time_mono_optimized,
            time_mono_unoptimized/ time_mono_optimized);

    free_global_bufs();
}


TEST(DemosaicTest, Image) {

    int n_cols = 0;
    int n_rows = 0;
    uint32_t* frog_image = NULL;


    frog_image = (uint32_t*) ReadImage(&n_cols, &n_rows,
            "../test/frog.bmp", IMAGEIO_U8 | IMAGEIO_RGBA);

    ASSERT_TRUE(frog_image != NULL);

    printf("width = %d, height = %d\n", n_cols, n_rows);

    alloc_global_bufs(n_rows, n_cols);

    demosaic_pix_rgb8 * image_mosaic = (demosaic_pix_rgb8*) malloc(
            n_rows * n_cols * sizeof(demosaic_pix_rgb8));

    // make inputs

    for (int row = 0; row < n_rows; row++) {
        for (int col = 0; col < n_cols; col++) {
            U8* u8p = (U8*)(&frog_image[row*n_cols+col]);
            U8 red = u8p[0];
            U8 green = u8p[1];
            U8 blue = u8p[2];
            image_truth16[(row * n_cols) + col].red = (U16)red << 4;
            image_truth16[(row * n_cols) + col].green = (U16)green << 4;
            image_truth16[(row * n_cols) + col].blue = (U16)blue << 4;
            image_truth8[(row * n_cols) + col].red = red;
            image_truth8[(row * n_cols) + col].green = green;
            image_truth8[(row * n_cols) + col].blue = blue;
            image_mosaic[(row * n_cols) + col].red = 0;
            image_mosaic[(row * n_cols) + col].green = 0;
            image_mosaic[(row * n_cols) + col].blue = 0;
            if (row % 2 == 0 && col % 2 == 0) {
                bayer16[(row * n_cols) + col] = (U16)red << 4;
                bayer8[(row * n_cols) + col] = red;
                image_mosaic[(row * n_cols) + col].red = red;
            } else if (row % 2 == 1 && col % 2 == 1) {
                bayer16[(row * n_cols) + col] = (U16)blue << 4;
                bayer8[(row * n_cols) + col] = blue;
                image_mosaic[(row * n_cols) + col].blue = blue;
            } else {
                bayer16[(row * n_cols) + col] = (U16)green << 4;
                bayer8[(row * n_cols) + col] = green;
                image_mosaic[(row * n_cols) + col].green = green;
            }
        }
    }

    int ret;
    ret = WriteImage(image_truth8,n_cols,n_rows, "frog_check.bmp",
            IMAGEIO_U8 | IMAGEIO_RGB, 85);
    ASSERT_EQ(ret, 1);
    ret = WriteImage(image_mosaic,n_cols,n_rows, "frog_mosaic_rgb.bmp",
            IMAGEIO_U8 | IMAGEIO_RGB, 85);
    ASSERT_EQ(ret, 1);
    ret = WriteImage(bayer8,n_cols,n_rows, "frog_mosaic_gray.bmp",
            IMAGEIO_U8 | IMAGEIO_GRAYSCALE, 85);
    ASSERT_EQ(ret, 1);

    demosaic_args args;
    args.n_rows = n_rows;
    args.n_cols = n_cols;
    args.max_val = 0xFFF;
    args.rshift = 4;
    args.coefs = {0.299, 0.587, 0.114}; // ccir 601 formula


    do_demosaicing(&args);

    // frog image has some test patterns meant to generate artifacts
    check_demosaicing_error(&args, 1000);

    ret = WriteImage(image_out_rgb8,n_cols,n_rows, "frog_demosaic_rgb.bmp",
            IMAGEIO_U8 | IMAGEIO_RGB, 85);
    ASSERT_EQ(ret, 1);
    ret = WriteImage(image_out_mono8,n_cols,n_rows, "frog_demosaic_gray.bmp",
            IMAGEIO_U8 | IMAGEIO_GRAYSCALE, 85);
    ASSERT_EQ(ret, 1);


    free(frog_image);
    free(image_mosaic);

    free_global_bufs();

}

TEST(DemosaicTest, Misc) {
    // adding two maxed uints to a larger signed int should not overflow
    U16 buf[] = {65535,65534};

    I32 ans = buf[0] + buf[1];
    EXPECT_EQ(ans, 131069);


}

TEST(DemosaicDeathTest, Asserts) {

    if (getenv("DEMOSAIC_DISABLE_DEATH_TESTS")) {
        printf("disabling death tests... they don't play well with valgrind.\n");
        return;
    } else {
        printf("death tests enabled.\n");
    }

    int n_rows = 480;
    int n_cols = 480;
    int n_pix = n_rows*n_cols;
    unsigned int max_val = 0x0FFF;

    alloc_global_bufs(n_rows, n_cols);
    free_global_bufs();

    demosaic_pix_rgb16 image_truth16_loc = {0,0,0};
    demosaic_pix_rgb8 image_truth8_loc = {0,0,0};
    U16 bayer16_loc = 0;
    U8  bayer8_loc = 0;
    demosaic_pix_rgb16 image_out_rgb16_loc = {0,0,0};
    demosaic_pix_rgb8  image_out_rgb8_loc = {0,0,0};
    demosaic_pix_rgb8  image_out_rgb8from16_loc = {0,0,0};
    U16 image_out_mono16_loc = 0;
    U8  image_out_mono8_loc = 0;
    U8  image_out_mono8from16_loc = 0;
    demosaic_pix_rgb16 image_out_rgb16_unopt_loc = {0,0,0};
    demosaic_pix_rgb8  image_out_rgb8_unopt_loc = {0,0,0};
    demosaic_pix_rgb8  image_out_rgb8from16_unopt_loc = {0,0,0};
    U16 image_out_mono16_unopt_loc = 0;
    U8  image_out_mono8_unopt_loc = 0;
    U8  image_out_mono8from16_unopt_loc = 0;

    image_truth16 = &image_truth16_loc;
    image_truth8 = &image_truth8_loc;
    bayer16 = &bayer16_loc;
    bayer8 = &bayer8_loc;
    image_out_rgb16 = &image_out_rgb16_loc;
    image_out_rgb8 = &image_out_rgb8_loc;
    image_out_rgb8from16 = &image_out_rgb8from16_loc;
    image_out_mono16 = &image_out_mono16_loc;
    image_out_mono8 = &image_out_mono8_loc;
    image_out_mono8from16 = &image_out_mono8from16_loc;
    image_out_rgb16_unopt = &image_out_rgb16_unopt_loc;
    image_out_rgb8_unopt = &image_out_rgb8_unopt_loc;
    image_out_rgb8from16_unopt = &image_out_rgb8from16_unopt_loc;
    image_out_mono16_unopt = &image_out_mono16_unopt_loc;
    image_out_mono8_unopt = &image_out_mono8_unopt_loc;
    image_out_mono8from16_unopt = &image_out_mono8from16_unopt_loc;


    demosaic_args args;
    args.n_rows = n_rows;
    args.n_cols = n_cols;
    args.max_val = max_val;
    args.rshift = 4;
    args.coefs = {0.299, 0.587, 0.114};     // ccir 601 formula
//    make_random_input(&args);
    demosaic_args bad_args;
    int row = 0;

    ASSERT_DEATH(
            demosaic_malvar_row_rgb16(NULL, &args, row,
                    &image_out_rgb16[row * args.n_cols]),
            "bayer");

    ASSERT_DEATH(
            demosaic_malvar_row_rgb16(bayer16, NULL, row,
                    &image_out_rgb16[row * args.n_cols]),
            "args");

    ASSERT_DEATH(
            demosaic_malvar_row_rgb16(bayer16, &args, row,
                    NULL),
            "output_row");

    row = -1;
    ASSERT_DEATH(
            demosaic_malvar_row_rgb16(bayer16, &args, row,
                    &image_out_rgb16[row * args.n_cols]),
            "row");

    row = n_rows;
    ASSERT_DEATH(
            demosaic_malvar_row_rgb16(bayer16, &args, row,
                    &image_out_rgb16[row * args.n_cols]),
            "row");

    row = 0;


    bad_args = args;
    bad_args.n_rows = 0;
    ASSERT_DEATH(
            demosaic_malvar_row_rgb16(bayer16, &bad_args, row,
                    &image_out_rgb16[row * args.n_cols]),
            "n_rows");

    bad_args = args;
    bad_args.n_rows = 45;
    ASSERT_DEATH(
            demosaic_malvar_row_rgb16(bayer16, &bad_args, row,
                    &image_out_rgb16[row * args.n_cols]),
            "n_rows");

    bad_args = args;
    bad_args.n_cols = -1;
    ASSERT_DEATH(
            demosaic_malvar_row_rgb16(bayer16, &bad_args, row,
                    &image_out_rgb16[row * args.n_cols]),
            "n_cols");

    bad_args = args;
    bad_args.n_cols = 43;
    ASSERT_DEATH(
            demosaic_malvar_row_rgb16(bayer16, &bad_args, row,
                    &image_out_rgb16[row * args.n_cols]),
            "n_cols");



//    demosaic_malvar_row_rgb16to8(bayer16, &args, row,
//                    &image_out_rgb8[row * args.n_cols]);

    ASSERT_DEATH(
            demosaic_malvar_row_rgb16to8(NULL, &args, row,
                    &image_out_rgb8[row * args.n_cols]),
            "bayer");

    ASSERT_DEATH(
            demosaic_malvar_row_rgb16to8(bayer16, NULL, row,
                    &image_out_rgb8[row * args.n_cols]),
            "args");

    ASSERT_DEATH(
            demosaic_malvar_row_rgb16to8(bayer16, &args, row,
                    NULL),
            "output_row");

    row = -1;
    ASSERT_DEATH(
            demosaic_malvar_row_rgb16to8(bayer16, &args, row,
                    &image_out_rgb8[row * args.n_cols]),
            "row");

    row = n_rows;
    ASSERT_DEATH(
            demosaic_malvar_row_rgb16to8(bayer16, &args, row,
                    &image_out_rgb8[row * args.n_cols]),
            "row");

    row = 0;


    bad_args = args;
    bad_args.n_rows = 0;
    ASSERT_DEATH(
            demosaic_malvar_row_rgb16to8(bayer16, &bad_args, row,
                    &image_out_rgb8[row * args.n_cols]),
            "n_rows");

    bad_args = args;
    bad_args.n_rows = 45;
    ASSERT_DEATH(
            demosaic_malvar_row_rgb16to8(bayer16, &bad_args, row,
                    &image_out_rgb8[row * args.n_cols]),
            "n_rows");

    bad_args = args;
    bad_args.n_cols = -1;
    ASSERT_DEATH(
            demosaic_malvar_row_rgb16to8(bayer16, &bad_args, row,
                    &image_out_rgb8[row * args.n_cols]),
            "n_cols");

    bad_args = args;
    bad_args.n_cols = 43;
    ASSERT_DEATH(
            demosaic_malvar_row_rgb16to8(bayer16, &bad_args, row,
                    &image_out_rgb8[row * args.n_cols]),
            "n_cols");

    printf("death tests complete.\n");



//    free_global_bufs();




}
