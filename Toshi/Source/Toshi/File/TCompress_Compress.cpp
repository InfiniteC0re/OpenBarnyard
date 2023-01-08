#include "ToshiPCH.h"
#include "TCompress_Compress.h"
#include "BTECCompressor.h"

namespace Toshi
{
    int TCompress_Compress::usemaxoffset;

    int TCompress_Compress::Write(uint32_t length, char*& data, TFile* file)
    {
        // 0068a830
        TASSERT(length <= maxlength);

        int writtenSize = 0;
        uint32_t dataSize = length;

        length -= 1;

        if (length < BTECSizeFlag_BigSize)
        {
            // 6 bits value
            LOWBYTE(length) = length | BTECSizeFlag_NoOffset;
            writtenSize += file->Write(&length, 1);
        }
        else
        {
            // 14 bits value
            auto len = length;
            LOWBYTE(length) = HIGHBYTE(length) | (BTECSizeFlag_NoOffset | BTECSizeFlag_BigSize);
            BYTE1(length) = len;
            writtenSize += file->Write(&length, 2);
        }

        writtenSize += file->Write(data, dataSize);
        data += dataSize;

        return writtenSize;
    }

    int TCompress_Compress::WriteOffset(uint32_t length, int offset, char*& data, TFile* file)
    {
        // 0068a8c0
        TASSERT(length <= maxlength, "Length is greater than {0}", maxlength);
        TASSERT(offset <= usemaxoffset, "Offset is greater than {0}", usemaxoffset);

        int writtenSize = 0;
        uint32_t dataSize = length;

        length -= 1;
        offset -= 1;

        if (length < BTECSizeFlag_BigSize)
        {
            LOWBYTE(length) = length;
            writtenSize += file->Write(&length, 1);
        }
        else
        {
            auto len = length;
            LOWBYTE(length) = HIGHBYTE(length) | BTECSizeFlag_BigSize;
            BYTE1(length) = len;
            writtenSize += file->Write(&length, 2);
        }

        if (offset <= BTECOffsetFlag_BigOffset)
        {
            LOWBYTE(offset) = offset;
            writtenSize += file->Write(&offset, 1);
        }
        else
        {
            auto _offset = offset;
            LOWBYTE(offset) = HIGHBYTE(offset) | BTECOffsetFlag_BigOffset;
            BYTE1(offset) = _offset;
            writtenSize += file->Write(&offset, 2);
        }

        data += dataSize;

        return writtenSize;
    }

    size_t TCompress_Compress::Compress(TFile* file, char* buffer, uint32_t size, uint32_t unused, bool isBigEndian)
    {
        BTECCompressor compressor;

        usemaxoffset = 0x8000;
        compressor.Initialize(buffer, size, usemaxoffset, 3);

        char* bufferPos = buffer;
        char* bufferEnd = buffer + size;

        auto initialPos = file->Tell();
        file->Seek(TCompress::HEADER_SIZE_12, TFile::TSEEK_CUR);

        size_t compressedSize = 0;
        size_t chunkSize = 0;
        char* chunkStart = TNULL;

        while (bufferPos < bufferEnd)
        {
            size_t uncompressedLeft = TMath::Min(size - (bufferPos - buffer), maxlength);

            char* offset = TNULL;
            size_t dataSize = 0;
            bool hasOffset = compressor.FUN_0068af10(bufferPos, uncompressedLeft, offset, dataSize);

            if (hasOffset == false || dataSize < 3)
            {
                dataSize = 1;
                
                // Check if we have to start a new chunk
                if (chunkStart == TNULL) chunkStart = bufferPos;
                chunkSize += 1;

                if (maxlength <= chunkSize)
                {
                    // Write a chunk of data
                    compressedSize += TCompress_Compress::Write(chunkSize, chunkStart, file);
                    chunkStart = TNULL;
                    chunkSize = 0;
                }

                bufferPos += 1;
            }
            else
            {
                if (chunkStart != TNULL)
                {
                    // We found an offset but we have a not-written chunk of data so write it
                    compressedSize += TCompress_Compress::Write(chunkSize, chunkStart, file);
                    chunkStart = TNULL;
                    chunkSize = 0;
                }

                compressedSize += TCompress_Compress::WriteOffset(dataSize, bufferPos - offset, bufferPos, file);
            }

            compressor.FUN_0068ae40(dataSize);
        }

        if (chunkStart != TNULL)
        {
            // We are out of the while loop but still have some chunk of data that should be written
            compressedSize += TCompress_Compress::Write(chunkSize, chunkStart, file);
        }

        TCompress::Header btecHeader;
        btecHeader.Magic = TMAKEFOUR("BTEC");
        btecHeader.Version = TMAKEVERSION(1, 2);
        btecHeader.CompressedSize = compressedSize;
        btecHeader.Size = size;

        if (isBigEndian)
        {
            btecHeader.Magic = PARSEDWORD_BIG(btecHeader.Magic);
            btecHeader.Version = PARSEDWORD_BIG(btecHeader.Version);
            btecHeader.CompressedSize = PARSEDWORD_BIG(btecHeader.CompressedSize);
            btecHeader.Size = PARSEDWORD_BIG(btecHeader.Size);
        }

        file->Seek(initialPos, TFile::TSEEK_SET);
        file->Write(&btecHeader, TCompress::HEADER_SIZE_12);
        file->Seek(compressedSize, TFile::TSEEK_CUR);

        return compressedSize;
    }
}
