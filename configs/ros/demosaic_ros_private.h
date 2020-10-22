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
 * @file        demosaic_ros_private.h
 * @date        2020-06-09
 * @author      Neil Abcouwer
 * @brief       Definition private macros for demosaic functions
 *
 * A user must provide a demosaic_conf_private.h to define the following macros.
 * This file is copied for unit testing.
 *
 * TODO actually test this in a ros framework...
 */

#ifndef DEMOSAIC_CONF_PRIVATE_H
#define DEMOSAIC_CONF_PRIVATE_H

#include <ros/types.h>
#include <ros/assert.h>

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

   This file defines the DEMOSAIC_ASSERT macros used in demosaic.c as
   ROS_ASSERTS.
 */
#define DEMOSAIC_ASSERT(test) \
    ROS_ASSERT_MSG(test, \
    "ASSERT in file %s, line %d.", \
    __FILE__, __LINE__)
#define DEMOSAIC_ASSERT_1(test, arg1) \
    ROS_ASSERT_MSG((test), \
    "ASSERT in file %s, line %d, arg1 = %d.", \
    __FILE__, __LINE__, (int32_t)(arg1))
#define DEMOSAIC_ASSERT_2(test, arg1, arg2) \
    ROS_ASSERT_MSG((test), \
    "ASSERT in file %s, line %d, arg1 = %d, arg2 = %d.", \
    __FILE__, __LINE__, (int32_t)(arg1), (int32_t)(arg2))
#define DEMOSAIC_ASSERT_DBL_1(test, arg1) \
    ROS_ASSERT_MSG((test), \
    "ASSERT in file %s, line %d, arg1 = %f.", \
    __FILE__, __LINE__, (double)(arg1))

#ifdef __cplusplus
}
#endif

#endif /* DEMOSAIC_CONF_PRIVATE_H */
