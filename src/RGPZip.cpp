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

#include <rgpaul/RGPZip.hpp>

#include <sys/stat.h>
#include <ctime>
#include <fstream>

#include <unzip.h>
#include <zconf.h>
#include <zip.h>
#include <zlib.h>
#include <boost/filesystem.hpp>

// ---------------------------------------------------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------------------------------------------------

rgpaul::Zip::Zip() {}

rgpaul::Zip::~Zip()
{
    // close any open files
    closeZipFile();
    closeUnzipFile();
}

// ---------------------------------------------------------------------------------------------------------------------
// Accessors
// ---------------------------------------------------------------------------------------------------------------------

std::shared_ptr<std::vector<std::string>> rgpaul::Zip::unzippedFiles() const { return _unzippedFiles; }

// ---------------------------------------------------------------------------------------------------------------------
// ZIP Functions
// ---------------------------------------------------------------------------------------------------------------------

bool rgpaul::Zip::createZipFile(const std::string &filepath, bool append, const std::string &password)
{
    _password = password;
    int appendData = append ? APPEND_STATUS_ADDINZIP : APPEND_STATUS_CREATE;

    _zipFile = zipOpen(filepath.data(), appendData);

    return _zipFile != nullptr;
}

bool rgpaul::Zip::addFileToZip(const std::string &filePath, const std::string &newName)
{
    struct stat fileStat = {0};
    int status = ::stat(filePath.c_str(), &fileStat);

    std::time_t timestamp = fileStat.st_mtime;

    std::chrono::system_clock::time_point tp;  // epoche (1.1.1970)
    std::chrono::system_clock::time_point modifiedDate = tp + std::chrono::seconds(timestamp);

    std::ifstream fs(filePath, std::ios::binary | std::ios::ate);

    if (!fs.is_open())
    {
        return false;
    }

    long long fileSize = fs.tellg();
    fs.seekg(0);

    // TODO: Improve this chunk of code. We read the whole file into memory.
    std::shared_ptr<std::vector<uint8_t>> data = std::make_shared<std::vector<uint8_t>>(fileSize);
    fs.read(reinterpret_cast<char *>(&(*data)[0]), fileSize);
    fs.close();

    return addDataToZip(data, newName, modifiedDate);
}

bool rgpaul::Zip::addDataToZip(std::shared_ptr<std::vector<uint8_t>> data, const std::string &newName,
                               std::chrono::system_clock::time_point created)
{
    if (data == nullptr || newName.empty() || _zipFile == nullptr)
    {
        return false;
    }

    std::time_t timestamp = std::chrono::system_clock::to_time_t(created);
    std::tm local_tm = *localtime(&timestamp);
    zip_fileinfo zipInfo = {{0, 0, 0, 0, 0, 0}, 0, 0, 0};

    zipInfo.tmz_date.tm_sec = static_cast<uInt>(local_tm.tm_sec);
    zipInfo.tmz_date.tm_min = static_cast<uInt>(local_tm.tm_min);
    zipInfo.tmz_date.tm_hour = static_cast<uInt>(local_tm.tm_hour);
    zipInfo.tmz_date.tm_mday = static_cast<uInt>(local_tm.tm_mday);
    zipInfo.tmz_date.tm_mon = static_cast<uInt>(local_tm.tm_mon);
    zipInfo.tmz_date.tm_year = static_cast<uInt>(local_tm.tm_year + 1900);

    if (_password.empty())
    {
        if (zipOpenNewFileInZip(_zipFile,              // file
                                newName.c_str(),       // filename
                                &zipInfo,              // zipfi
                                nullptr,               // extrafield_local
                                0,                     // size_extrafield_local
                                nullptr,               // extrafield_global
                                0,                     // size_extrafield_global
                                nullptr,               // comment
                                Z_DEFLATED,            // method
                                Z_DEFAULT_COMPRESSION  // level
                                ) != Z_OK)
        {
            return false;
        }
    }
    else
    {
        uLong crc = crc32(0L, nullptr, 0L);
        crc = crc32(crc, reinterpret_cast<const Bytef *>(&(*data)[0]), data->size());
        if (zipOpenNewFileInZip3(_zipFile,               // file
                                 newName.c_str(),        // filename
                                 &zipInfo,               // zipfi
                                 nullptr, 0,             // extrafield_local
                                 nullptr, 0,             // extrafield_global
                                 nullptr,                // comment
                                 Z_DEFLATED,             // method
                                 Z_DEFAULT_COMPRESSION,  // level
                                 0,                      // raw
                                 15,                     // windowBits
                                 8,                      // memLevel
                                 Z_DEFAULT_STRATEGY,     // strategy
                                 _password.c_str(),      // password
                                 crc                     // crcForCrypting
                                 ) != Z_OK)
        {
            return false;
        }
    }

    if (zipWriteInFileInZip(_zipFile, &(*data)[0], data->size()) != Z_OK)
    {
        return false;
    }

    return zipCloseFileInZip(_zipFile) == Z_OK;
}

bool rgpaul::Zip::closeZipFile()
{
    _password.clear();

    if (_zipFile == nullptr)
    {
        return false;
    }

    if (zipClose(_zipFile, nullptr) != Z_OK)
    {
        _zipFile = nullptr;
        return false;
    }

    _zipFile = nullptr;
    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
// UNZIP Functions
// ---------------------------------------------------------------------------------------------------------------------

bool rgpaul::Zip::openUnzipFile(const std::string &filePath, const std::string &password)
{
    _password = password;

    _unzippedFiles = std::make_shared<std::vector<std::string>>();

    _unzipFile = unzOpen(filePath.c_str());

    return _unzipFile != nullptr;
}

bool rgpaul::Zip::unzipFiles(std::string targetPath, bool overwrite)
{
    if (targetPath.empty() || _unzipFile == nullptr)
    {
        return false;
    }

    // targetPath needs a trailing /
    if (targetPath.back() != '/')
    {
        targetPath.append("/");
    }

    bool success = true;

    char buffer[4096] = {0};

    const char *password = {nullptr};

    if (!_password.empty())
    {
        password = _password.c_str();
    }

    if (unzGoToFirstFile(_unzipFile) != UNZ_OK)
    {
        return false;
    }

    do  // we iterate over all files
    {
        // open current file
        if (password != nullptr)
        {
            // we have a password - so we need to open with it
            if (unzOpenCurrentFilePassword(_unzipFile, password) != UNZ_OK)
            {
                success = false;
                break;
            }
        }
        else
        {
            // we don't have a password - just open
            if (unzOpenCurrentFile(_unzipFile) != UNZ_OK)
            {
                success = false;
                break;
            }
        }

        // reading file info
        unz_file_info fileInfo;
        if (unzGetCurrentFileInfo(_unzipFile, &fileInfo, nullptr, 0, nullptr, 0, nullptr, 0) != UNZ_OK)
        {
            success = false;
            unzCloseCurrentFile(_unzipFile);
            break;
        }

        // get filename from the fileInfo
        char *cFileName = new char[fileInfo.size_filename + 1];
        unzGetCurrentFileInfo(_unzipFile, &fileInfo, cFileName, fileInfo.size_filename + 1, nullptr, 0, nullptr, 0);
        cFileName[fileInfo.size_filename] = '\0';

        // check filename for paths
        std::string fileName = cFileName;

        delete[] cFileName;

        // replace all '\' with '/'
        unsigned long index;
        while ((index = fileName.find_first_of('\\')) != std::string::npos)
        {
            fileName.replace(index, 1, "/");
        }
        std::string fullPath = targetPath + fileName;

        // create directory / parent directories
        index = fullPath.find_last_of('/');
        if (index != std::string::npos)
        {
            boost::filesystem::path dir(fullPath.substr(0, index + 1));
            boost::filesystem::create_directories(dir);
        }

        std::shared_ptr<std::ofstream> fs;
        int read = 0;
        while ((read = unzReadCurrentFile(_unzipFile, buffer, 4096)) > 0)
        {
            if (!fs || !fs->is_open())
            {
                if (boost::filesystem::exists(fullPath) && !overwrite)
                {
                    break;
                }

                fs = std::make_shared<std::ofstream>(fullPath, std::ios::binary | std::ios::trunc);

                if (!fs->is_open())
                {
                    fs.reset();
                    break;
                }
            }

            fs->write(buffer, read);
        }

        // check if we got an error
        if (read < 0)
        {
            success = false;
        }

        if (fs)
        {
            fs->close();  // flush the data to the file
            // we list all unzipped files
            _unzippedFiles->push_back(fullPath);
            fs.reset();
        }

        // we are done with the file -> close the file
        if (unzCloseCurrentFile(_unzipFile) != UNZ_OK)
        {
            success = false;
            break;
        }

    }  // goto next file
    while (unzGoToNextFile(_unzipFile) == UNZ_OK && success);

    return success;
}

bool rgpaul::Zip::closeUnzipFile()
{
    _password.clear();

    if (_unzipFile == nullptr)
    {
        return false;
    }

    if (unzClose(_unzipFile) != UNZ_OK)
    {
        _unzipFile = nullptr;
        return false;
    }

    _unzipFile = nullptr;
    return true;
}
