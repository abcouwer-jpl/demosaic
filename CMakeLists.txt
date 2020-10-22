cmake_minimum_required(VERSION 2.8.2)

set(CMAKE_VERBOSE_MAKEFILE ON)

project(demosaic)

set(VERSION "1.0")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-long-long -pedantic -Werror -Wno-unused-local-typedefs")

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include)

if (CMAKE_BUILD_TYPE STREQUAL "Coverage")
  # build the unit tests
  set(CMAKE_BUILD_TYPE "Test")
  set(CMAKE_MODULE_PATH ${CMAKE_HOME_DIRECTORY}/test)
  add_custom_target(ctest COMMAND ${CMAKE_CTEST_COMMAND})
  include(CodeCoverage)
  APPEND_COVERAGE_COMPILER_FLAGS()
  set(COVERAGE_EXCLUDES 
    '/usr/*' 
    '${CMAKE_CURRENT_SOURCE_DIR}/test/*'
    '${CMAKE_CURRENT_SOURCE_DIR}/build/*')
  MESSAGE( STATUS "COVERAGE_EXCLUDES: " ${COVERAGE_EXCLUDES} )
  setup_target_for_coverage(
    NAME coverage 
    EXECUTABLE ctest
    DEPENDENCIES ctest)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "CfsTest")
  configure_file(
      ${CMAKE_SOURCE_DIR}/configs/cfs/demosaic_cfs_global_types.h 
      ${CMAKE_SOURCE_DIR}/include/demosaic/demosaic_conf_global_types.h)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/test/cfs)
  set(CMAKE_BUILD_TYPE "Test")
else()
    configure_file(
      ${CMAKE_SOURCE_DIR}/test/demosaic_test_global_types.h 
      ${CMAKE_SOURCE_DIR}/include/demosaic/demosaic_conf_global_types.h)
endif()


if (CMAKE_BUILD_TYPE STREQUAL "Test")

  # Copy testing headers
  
  configure_file(
      ${CMAKE_SOURCE_DIR}/test/demosaic_test_private.h 
      ${CMAKE_SOURCE_DIR}/include/demosaic/demosaic_conf_private.h)

  set(CMAKE_BUILD_TYPE "Debug")
  add_definitions(-DDEMOSAIC_PRIVATE=)
  set(ENV{CTEST_OUTPUT_ON_FAILURE} TRUE)

  include(CTest)
  enable_testing()
    
  #============================================================================
  # Download imageio
  #============================================================================
  
  # ExternalProject_Add not working for me - it seems like the files are not cloned until the end
  if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/imageio/basic.h)
  else()
    message(STATUS "imageio not found, cloning...")
    
    execute_process(
                COMMAND             git clone https://github.com/nabcouwer/imageio_pg.git imageio
                WORKING_DIRECTORY   ${CMAKE_BINARY_DIR}
                RESULT_VARIABLE     git_result
                OUTPUT_VARIABLE     git_output)
        if(NOT git_result EQUAL "0")
            set(${CLONE_RESULT} FALSE CACHE INTERNAL "" FORCE)
            if(NOT PARGS_QUIET)
                message(WARNING "imageio clone error")  #ToDo: maybe FATAL_ERROR?
            endif()
            return()
        endif()     
    execute_process(
                COMMAND             rm -rf imageio/.git
                WORKING_DIRECTORY   ${CMAKE_BINARY_DIR})
  endif()   

  set(IMAGEIO_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/imageio/basic.c
    ${CMAKE_CURRENT_BINARY_DIR}/imageio/imageio.c
  )
    
  include_directories(${CMAKE_CURRENT_BINARY_DIR}/imageio)
  link_directories(${CMAKE_CURRENT_BINARY_DIR}/imageio)
  
 
    
  #============================================================================
  # Download and unpack googletest at configure time
  #============================================================================
  
  configure_file(test/GoogleTestCMakeLists.txt.in googletest-download/CMakeLists.txt)
  
  execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
  if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
  endif()
  execute_process(COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
  if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
  endif()
  
  # Prevent overriding the parent project's compiler/linker
  # settings on Windows
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  
  # Add googletest directly to our build. This defines
  # the gtest and gtest_main targets.
  add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                   ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                   EXCLUDE_FROM_ALL)
  
  # The gtest/gtest_main targets carry header search path
  # dependencies automatically when using CMake 2.8.11 or
  # later. Otherwise we have to add them here ourselves.
  if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include")
  endif()

  
  #==========================================================================
  # gtest binaries
  #==========================================================================
  
  add_executable(demosaic_gtest 
    include/demosaic/demosaic_conf_global_types.h 
    include/demosaic/demosaic_types_pub.h 
    include/demosaic/demosaic_pub.h 
    include/demosaic/demosaic_conf_private.h 
    src/demosaic.c 
    test/demosaic_gtest.cpp
    ${IMAGEIO_SRCS})
  target_link_libraries(demosaic_gtest gtest_main )
  add_test(NAME demosaic_gtest_test COMMAND demosaic_gtest)
  
endif()

