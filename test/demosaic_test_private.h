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
 * @file        demosaic_test_private.h
 * @date        2020-06-05
 * @author      Neil Abcouwer
 * @brief       Definition private macros for demosaic functions
 *
 * A user must provide a demosaic_conf_private.h to define the following macros.
 * This file is copied for unit testing.
 */

#ifndef DEMOSAIC_CONF_PRIVATE_H
#define DEMOSAIC_CONF_PRIVATE_H

#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

// private demosacic functions are preceded by DEMOSAIC_PRIVATE
// this can be defined as 0 when compiling unit tests to allow access
// if your infrastructure does something similar, replace it here
#ifndef DEMOSAIC_PRIVATE
#define DEMOSAIC_PRIVATE static
#endif

/* This library was written with the philosophy that assertions be used to
   check anomalous conditions. Demosaic functions assert if inputs
   indicate there is a logic error.
   See http://spinroot.com/p10/rule5.html.

   This file defines the DEMOSAIC_ASSERT macros used in demosaic.c as simple
   c asserts, for testing. THe google test suite for the repo uses
   "death tests" to test that they are called appropriately.

   It is the intent that user of the demosaic library will copy an
   appropriate demosaic_conf.h to include/demosaic, such that asserts
   are defined appropriately for the application.

   Possible asserts:
        cstd assers
        ROS_ASSERT
        BOOST_ASSERT
        (test) ? (void)(0)
               : send_asynchronous_safing_alert_to_other_process(), assert(0):

   Asserts could also be disabled, but this is is discouraged.
 */
#define DEMOSAIC_ASSERT(test) assert(test)
#define DEMOSAIC_ASSERT_1(test, arg1) assert(test)
#define DEMOSAIC_ASSERT_2(test, arg1, arg2) assert(test)
#define DEMOSAIC_ASSERT_DBL_1(test, arg1) assert(test)

#ifdef __cplusplus
}
#endif

#endif /* DEMOSAIC_CONF_PRIVATE_H */
