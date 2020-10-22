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
 * @file        demosaic_osal_global_types.h
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
 * these types.  For the purposes of working in the NASA core FSW framework,
 * test/demosaic_osal_global_types.h is copied to
 * include/demosaic/demosaic_conf_global_types.h.
 * It includes common_types.h from osal, https://github.com/nasa/osal
 *
 */

#ifndef DEMOSAIC_CONF_GLOBAL_TYPES_H
#define DEMOSAIC_CONF_GLOBAL_TYPES_H

#include "common_types.h" // defines NULL and sized types

#define DEMOSAIC_COMPILE_ASSERT(test, msg) CompileTimeAssert(test, msg)

typedef int32 I32;
typedef uint8 U8;
typedef uint16 U16;
typedef double F64;

DEMOSAIC_COMPILE_ASSERT(sizeof(I32) == 4, I32BadSize);
DEMOSAIC_COMPILE_ASSERT(sizeof(U8)  == 1,  U8BadSize);
DEMOSAIC_COMPILE_ASSERT(sizeof(U16) == 2, U16BadSize);
DEMOSAIC_COMPILE_ASSERT(sizeof(F64) == 8, F64BadSize);

#define U8_MAX (0xFF)

#endif /* DEMOSAIC_CONF_GLOBAL_TYPES_H */
