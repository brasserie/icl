/**
 * MIT License
 * Copyright (c) 2019 Anthony Rabine
 */

#define ZIP_CPP_INCLUDED
#include "miniz.h"
#include "Zip.h"
#include <fstream>
#include <cstring>
#include <functional>
#include <memory>

/*****************************************************************************/
Zip::Zip()
    : mIsValid(false)
    , mNumberOfFiles(0U)
{
    std::memset(&mZipArchive, 0, sizeof(mZipArchive));
}
/*****************************************************************************/
Zip::~Zip()
{
    Close();
}

/*****************************************************************************/
bool Zip::Open(const std::string &zip, bool isFile)
{
    mz_bool status;
    mIsValid = false;

    mNumberOfFiles = 0U;

    std::memset(&mZipArchive, 0, sizeof(mZipArchive));

    if (isFile)
    {
        // Physical file on disk
        status = mz_zip_reader_init_file(&mZipArchive, zip.c_str(), 0);
    }
    else
    {
        // Zipped memory
        status = mz_zip_reader_init_mem(&mZipArchive, zip.c_str(), zip.size(), 0);
    }

    if (status)
    {
        // Get and print information about each file in the archive.
        for (std::uint32_t i = 0; i < mz_zip_reader_get_num_files(&mZipArchive); i++)
        {
            mz_zip_archive_file_stat file_stat;
            if (mz_zip_reader_file_stat(&mZipArchive, i, &file_stat))
            {
                mNumberOfFiles++;
                //printf("Filename: \"%s\", Comment: \"%s\", Uncompressed size: %u, Compressed size: %u\n", file_stat.m_filename, file_stat.m_comment, (std::uint32_t)file_stat.m_uncomp_size, (std::uint32_t)file_stat.m_comp_size);
            }
        }
    }

    if (mNumberOfFiles == mz_zip_reader_get_num_files(&mZipArchive))
    {
        mIsValid = true;
    }

    return mIsValid;
}
/*****************************************************************************/
void Zip::Close()
{
    if (mIsValid)
    {
        mz_zip_reader_end(&mZipArchive);
    }
}

class Callback
{
public:
    static mz_bool DeflateCallback(const void *pBuf, int len, void *pUser)
    {
        std::memcpy(output + offset, pBuf, len);
        offset += len;
        (void) len;
        (void) pUser;
        return MZ_TRUE;
    }

    static char *output;
    static int offset;
};

char * Callback::output;
int Callback::offset;

/*****************************************************************************/
int Zip::CompressBuffer(const char *input, size_t input_size, char *output)
{
    int finalsize = -1;
    tdefl_compressor Comp;


    Callback::offset = 0U;
    Callback::output = output;

    if (tdefl_init(&Comp, &Callback::DeflateCallback, nullptr, 0) == TDEFL_STATUS_OKAY)
    {
        if(tdefl_compress_buffer(&Comp, input, input_size, TDEFL_FINISH) == TDEFL_STATUS_DONE)
        {
            finalsize = Callback::offset;
        }
    }


    return finalsize;
}
/*****************************************************************************/
bool Zip::GetFile(const std::string &fileName, std::string &contents)
{
    bool ret = false;
    if (mIsValid)
    {
        size_t size;
        char *p = reinterpret_cast<char *>(mz_zip_reader_extract_file_to_heap(&mZipArchive, fileName.c_str(), &size, 0));

        if (p != nullptr)
        {
            contents.assign(p, size);
            free(p);
            ret = true;
        }
    }
    return ret;
}

//=============================================================================
// End of file Zip.cpp
//=============================================================================
