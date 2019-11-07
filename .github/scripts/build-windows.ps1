#!/usr/bin/env powershell
# ----------------------------------------------------------------------------------------------------------------------
# The MIT License (MIT)
#
# Copyright (c) 2019 Ralph-Gordon Paul. All rights reserved.
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

param(
    [string]$ARCH,
    [string]$BUILD_TYPE
)

#=======================================================================================================================
# create conan package

function createConanPackage($arch, $build_type)
{
    $runtime = "MT"

    if ($build_type.equals("Debug"))
    {
        $runtime = "MTd"
    }

    conan create . ${env:CONAN_PACKAGE_NAME}/${env:LIBRARY_VERSION}@${env:CONAN_USER}/${env:CONAN_CHANNEL}  `
        -s os=Windows -s compiler="Visual Studio" -s compiler.runtime=$runtime -s arch=${arch}  `
        -s build_type=${build_type} -o shared=False
}

#=======================================================================================================================
# create packages for all architectures and build types

createConanPackage $ARCH $BUILD_TYPE
