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

declare ABSOLUTE_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Install conan (windows)
if [[ "$GITHUB_OS_NAME" == "windows" ]]; then
    choco install python3;
    choco install conan;

# Install conan (linux)
elif [[ "$GITHUB_OS_NAME" == "linux" ]]; then
    pip3 install setuptools wheel --user
    pip3 install conan --user
    source ~/.profile

# Install conan (macos)
elif [[ "$GITHUB_OS_NAME" == "macos" ]]; then
    pip3 install conan;
fi

# Add conan repository and apply conan config
conan remote add ${CONAN_REPOSITORY_NAME} ${CONAN_REPOSITORY}
conan config install ${ABSOLUTE_DIR}/../conan/config.zip

# login to conan
conan user -p "${BINTRAY_KEY}" -r ${CONAN_REPOSITORY_NAME} ${BINTRAY_USER}
