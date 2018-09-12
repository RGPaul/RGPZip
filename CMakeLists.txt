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

cmake_minimum_required (VERSION 3.10)
project (RGPZip)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)

# for Visual Studio Code
include(CMakeToolsHelpers OPTIONAL)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/lib)

# ----------------------------------------------------------------------------------------------------------------------
# Dependencies
# ----------------------------------------------------------------------------------------------------------------------

# zlibc Compressing File-I/O Library - https://www.zlib.net
find_package(zlib REQUIRED)

# Boost - http://www.boost.org
find_package(Boost 1.64.0 REQUIRED COMPONENTS system filesystem)

# ----------------------------------------------------------------------------------------------------------------------

add_library(rgpzip STATIC
    # minizip sources
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/zlib/contrib/minizip/unzip.c
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/zlib/contrib/minizip/zip.c
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/zlib/contrib/minizip/ioapi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/zlib/contrib/minizip/mztools.c
    # rgpzip sources
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RGPZip.cpp)

target_include_directories(rgpzip
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/deps/zlib/contrib/minizip
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(rgpzip
    ${ZLIB_LIBRARIES}
    ${Boost_LIBRARIES})

# set version info
set_target_properties(rgpzip PROPERTIES
    VERSION 0.1
    SOVERSION 0
    PUBLIC_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/src/RGPZip.hpp")

# installation
install(TARGETS rgpzip
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include/rgp
    RUNTIME DESTINATION bin)
