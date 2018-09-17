#  ---------------------------------------------------------------------------------------------------------------------
#  The MIT License (MIT)
#
#  Copyright (c) 2018 Ralph-Gordon Paul. All rights reserved.
#
#  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
#  documentation files (the "Software"), to deal in the Software without restriction, including without limitation the 
#  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to 
#  permit persons to whom the Software is furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the 
#  Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE 
#  WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR 
#  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
#  OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#  ---------------------------------------------------------------------------------------------------------------------

cmake_minimum_required (VERSION 3.9)
project (RGPZip)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)

# for Visual Studio Code
include(CMakeToolsHelpers OPTIONAL)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/lib)

# add local cmake modules to module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")

# specify windows specific settings
if (WIN32)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        set(CMAKE_CXX_FLAGS_RELEASE "/MT")
        set(CMAKE_CXX_FLAGS_DEBUG "/MTd")
    endif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
endif (WIN32)

# ----------------------------------------------------------------------------------------------------------------------
# Dependencies
# ----------------------------------------------------------------------------------------------------------------------

# zlibc Compressing File-I/O Library - https://www.zlib.net
#if (ANDROID)
#    find_package(ZLIB REQUIRED PATHS "${CMAKE_CURRENT_LIST_DIR}/deps/lib/${CMAKE_ANDROID_ARCH_ABI}" NO_DEFAULT_PATH)
#else()
    find_package(ZLIB REQUIRED)
#endif(ANDROID)

# Boost - http://www.boost.org
set(Boost_USE_STATIC_LIBS ON )
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost 1.67.0 REQUIRED COMPONENTS system filesystem)

# ----------------------------------------------------------------------------------------------------------------------
# Library
# ----------------------------------------------------------------------------------------------------------------------

file(GLOB MINIZIP_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/zlib/contrib/minizip/unzip.c
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/zlib/contrib/minizip/zip.c
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/zlib/contrib/minizip/ioapi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/zlib/contrib/minizip/mztools.c)

file(GLOB RGPZIP_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RGPZip.cpp)

if (WIN32)
    file(GLOB MINIZIP_PLATFORM_SRC
            ${CMAKE_CURRENT_SOURCE_DIR}/deps/zlib/contrib/minizip/iowin32.c)
endif (WIN32)

add_library(rgpzip STATIC
    # minizip sources
    ${MINIZIP_SRC}
    # platform specific minizip sources
    ${MINIZIP_PLATFORM_SRC}
    # rgpzip sources
    ${RGPZIP_SRC})

target_include_directories(rgpzip
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/deps/zlib/contrib/minizip
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(rgpzip
    ${ZLIB_LIBRARIES}
    ${Boost_LIBRARIES})

# set version info
set_target_properties(rgpzip PROPERTIES
    VERSION 1.0.1
    SOVERSION 0
    PUBLIC_HEADER "include/rgpaul/RGPZip.hpp")

# ----------------------------------------------------------------------------------------------------------------------
# Installation
# ----------------------------------------------------------------------------------------------------------------------

install(TARGETS rgpzip
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include/rgpaul
    RUNTIME DESTINATION bin)

# ----------------------------------------------------------------------------------------------------------------------
# Unit Tests
# ----------------------------------------------------------------------------------------------------------------------

# Boost Unit Tests
find_package(Boost 1.67.0 COMPONENTS unit_test_framework)

# only enable unit tests if the unit test framework was found
if(Boost_UNIT_TEST_FRAMEWORK_FOUND AND NOT IOS AND NOT ANDROID)

    # enable cmake testing
    enable_testing()

    # all tests are in the test folder
    file(GLOB TEST_SRC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} test/*.cpp)

    # copy test data
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test/golden-retriever-3058383_1920.jpg DESTINATION .)

    # process every test file
    foreach(testSrc ${TEST_SRC})

        # get the filename without the extension
        get_filename_component(testname ${testSrc} NAME_WE)

        # add test as executable
        add_executable(${testname} ${testSrc})

        # link jwtpp, boost and crypto++
        target_link_libraries(${testname}
            rgpzip
            ${ZLIB_LIBRARIES}
            ${Boost_LIBRARIES})

        target_include_directories(${testname}
                PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/deps/zlib/contrib/minizip
                PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
                PRIVATE ${Boost_INCLUDE_DIRS})

        # save the executable in the tests folder
        set_target_properties(${testname} PROPERTIES RUNTIME_OUTPUT_DIRECTORY tests)

        # add to cmake tests
        add_test(NAME ${testname} COMMAND tests/${testname})
    endforeach(testSrc)
endif()
