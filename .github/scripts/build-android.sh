#!/usr/bin/env bash
# ----------------------------------------------------------------------------------------------------------------------
# The MIT License (MIT)
#
# Copyright (c) 2018-2019 Ralph-Gordon Paul. All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
# documentation files (the "Software"), to deal in the Software without restriction, including without limitation the 
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit 
# persons to whom the Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the 
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE 
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR 
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
# OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
# ----------------------------------------------------------------------------------------------------------------------

set -e

source ~/.profile

#=======================================================================================================================
# settings

declare TOOLCHAIN_VERSION=clang
# please check the compiler version of your ndk before building f.e.:
# /opt/android-ndks/android-ndk-r19c/toolchains/llvm/prebuilt/darwin-x86_64/bin/clang++ --version
declare COMPILER_VERSION=8.0
declare COMPILER_LIBCXX=libc++
declare STL_TYPE=c++_static
declare ANDROID_NDK_PATH=$ANDROID_SDK_ROOT/ndk-bundle

declare ARCH=$1
declare API_LEVEL=$2
declare BUILD_TYPE=$3

echo "using NDK Path: ${ANDROID_NDK_PATH}"
export ANDROID_NDK_PATH=${ANDROID_NDK_PATH}

#=======================================================================================================================

function getAndroidNdkVersion()
{
    # get properties file that contains the ndk revision number
    local NDK_RELEASE_FILE=$ANDROID_NDK_PATH"/source.properties"
    if [ -f "${NDK_RELEASE_FILE}" ]; then
        local NDK_RN=`cat $NDK_RELEASE_FILE | grep 'Pkg.Revision' | sed -E 's/^.*[=] *([0-9]+[.][0-9]+)[.].*/\1/g'`
    else
        echo "ERROR: can not find android ndk version"
        exit 1
    fi

    # convert ndk revision number
    case "${NDK_RN#*'.'}" in
        "0")
            NDK_VERSION="r${NDK_RN%%'.'*}"
            ;;

        "1")
            NDK_VERSION="r${NDK_RN%%'.'*}b"
            ;;

        "2")
            NDK_VERSION="r${NDK_RN%%'.'*}c"
            ;;
        
        "3")
            NDK_VERSION="r${NDK_RN%%'.'*}d"
            ;;

        "4")
            NDK_VERSION="r${NDK_RN%%'.'*}e"
            ;;

        *)
            echo "Undefined or not supported Android NDK version: $NDK_RN"
            exit 1
    esac
}

#=======================================================================================================================

function getCompilerVersion()
{
    case "${NDK_VERSION}" in
        "r16b")
            COMPILER_VERSION=5.0
            ;;
        "r17c")
            COMPILER_VERSION=6.0
            ;;
        "r18b")
            COMPILER_VERSION=7.0
            ;;
        "r19c")
            COMPILER_VERSION=8.0
            ;;
        "r20")
            COMPILER_VERSION=8.0
            ;;
            
        *)
            echo "Unknown Compiler version for Android NDK ${NDK_VERSION}"
            exit 1
    esac
}

#=======================================================================================================================
# create conan package

function createConanPackage()
{
    local arch=$1
    local api_level=$2
    local build_type=$3

    conan create . ${CONAN_PACKAGE_NAME}/${LIBRARY_VERSION}@${CONAN_USER}/${CONAN_CHANNEL} -s os=Android \
        -s os.api_level=${api_level} -s compiler=${TOOLCHAIN_VERSION} -s compiler.version=${COMPILER_VERSION} \
        -s compiler.libcxx=${COMPILER_LIBCXX} -s build_type=${build_type} -o android_ndk=${NDK_VERSION} \
        -o android_stl_type=${STL_TYPE} -s arch=${arch} -o shared=False
}

#=======================================================================================================================
# create packages for all architectures and build types

getAndroidNdkVersion
getCompilerVersion

createConanPackage $ARCH $API_LEVEL $BUILD_TYPE
