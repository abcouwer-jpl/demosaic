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
 * @file        demosaic_test_global_types.h
 * @date        2020-06-05
 * @author      Neil Abcouwer
 * @brief       Definition of global types for testing demosaic
 *
 * demosaic_conf_types_pub.h defines public types used by public demosaic functions.
 * and follows the common guideline, as expressed in MISRA
 * directive 4.6, "typedefs that indicate size and signedness should be used
 * in place of the basic numerical types".
 *
 * demosaic_types_pub.h includes demosaic_conf_global_types.h, which must define
 * these types.  For the purposes of platform-independent unit testing,
 * test/demosaic_test_global_types.h is copied to
 * include/demosaic/demosaic_conf_global_types.h.
 *
 * Users must define a configuration dependent header for their purposes.
 *
 * Sources of sized types:
 *      cstd: stdint.h
 *      NASA core fsw: osal -> common_types.h
 *      VXWorks: inttypes.h
 *
 * Demosaic also needs a NULL, and U8_MAX.
 */

#ifndef DEMOSAIC_CONF_GLOBAL_TYPES_H
#define DEMOSAIC_CONF_GLOBAL_TYPES_H

#include <stdint.h>

/*============================================================================*/
/** Compile time assert that can be anywhere, not just within a function
 *
 * Compile time assertions from:
 * http://unixjunkie.blogspot.com/
 * by Greg Miller, a software engineer at Google
 *
 * Useage:
 * GBL_COMPILE_ASSERT( assertion, message )
 *
 * The assertion is a logical operation, such as (sizeof(a) + sizeof(b) < 8192)
 * and the message is a legal C identifier that represents the assertion you are
 * making, e.g., value_of_DP_INX_must_be_one.
 * If the assertion fails, compilation will fail and you'll get a message like:
 *
 * error: size of array 'GBL_COMPILE_ASSERT_71__value_of_DP_INX_must_be_one' is negative
 *
 * where 71 is the line number of the assertion that failed.
 */
#define DEMOSAIC_COMPILE_ASSERT_INNER_(line, msg) DEMOSAIC_COMPILE_ASSERT_ ## line ## __ ## msg
#define DEMOSAIC_COMPILE_ASSERT_SYMBOL_(line, msg) DEMOSAIC_COMPILE_ASSERT_INNER_(line, msg)
#define DEMOSAIC_COMPILE_ASSERT(test, msg) \
  typedef U8 DEMOSAIC_COMPILE_ASSERT_SYMBOL_(__LINE__, msg) [ ((test) ? 1 : -1) ]



#ifndef NULL
#define NULL  (0)
#endif

typedef int32_t I32;
typedef uint8_t U8;
typedef uint16_t U16;
typedef double  F64;

DEMOSAIC_COMPILE_ASSERT(sizeof(I32) == 4, I32BadSize);
DEMOSAIC_COMPILE_ASSERT(sizeof(U8)  == 1,  U8BadSize);
DEMOSAIC_COMPILE_ASSERT(sizeof(U16) == 2, U16BadSize);
DEMOSAIC_COMPILE_ASSERT(sizeof(F64) == 8, F64BadSize);

#define U8_MAX (0xFF)

#endif /* DEMOSAIC_CONF_GLOBAL_TYPES_H */
