/*
 -----------------------------------------------------------------------------------------------------------------------
 The MIT License (MIT)

 Copyright (c) 2018 Ralph-Gordon Paul. All rights reserved.

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 -----------------------------------------------------------------------------------------------------------------------
*/

/*
 * The used image (golden-retriever-3058383_1920.jpg) for testing is licensed under CC0 and can be found on Pixabay:
 * https://pixabay.com/de/golden-retiver-hund-happy-dog-3058383/
 */

// define the module name (prints at testing)
#define BOOST_TEST_MODULE "RGPZip"

#include <rgpaul/RGPZip.hpp>
using rgpaul::Zip;

#include <string>

// include this last
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(RGPZip)

BOOST_AUTO_TEST_CASE(constructor)
{
    BOOST_CHECK_NO_THROW(rgpaul::Zip {});
}

BOOST_AUTO_TEST_CASE(zip)
{
    rgpaul::Zip zipper {};

    BOOST_CHECK(zipper.createZipFile("Test.zip"));

    BOOST_CHECK(zipper.addFileToZip("golden-retriever-3058383_1920.jpg", "Dog.jpg"));

    BOOST_CHECK(zipper.closeZipFile());
}

BOOST_AUTO_TEST_CASE(unzip)
{
    rgpaul::Zip zipper {};

    BOOST_CHECK(zipper.openUnzipFile("Test.zip"));

    BOOST_CHECK(zipper.unzipFiles("test_output/files", true));

    // TODO: Check if the extracted image is equal to the original

    BOOST_CHECK(zipper.closeUnzipFile());
}

BOOST_AUTO_TEST_SUITE_END()
