#include "ToshiPCH.h"
#include "TCompress.h"
#include "BTECCompressor.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi
{
    TINT TCompress::usemaxoffset;

    TINT TCompress::Write(TUINT32 length, TCHAR*& data, TFile* file)
    {
        // 0068a830
        TASSERT(length <= maxlength);

        TINT writtenSize = 0;
        TUINT32 dataSize = length;

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

    TINT TCompress::WriteOffset(TUINT32 length, TINT offset, TCHAR*& data, TFile* file)
    {
        // 0068a8c0
        TASSERT(length <= maxlength);
        TASSERT(offset <= usemaxoffset);

        TINT writtenSize = 0;
        TUINT32 dataSize = length;

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

    size_t TCompress::Compress(TFile* file, TCHAR* buffer, TUINT32 size, TUINT32 unused, TBOOL isBigEndian)
    {
        BTECCompressor compressor;

        usemaxoffset = 0x8000;
        compressor.Initialize(buffer, size, usemaxoffset, 3);

        TCHAR* bufferPos = buffer;
        TCHAR* bufferEnd = buffer + size;

        auto initialPos = file->Tell();
        file->Seek(TCompress::HEADER_SIZE_12, TSEEK_CUR);

        size_t compressedSize = 0;
        size_t chunkSize = 0;
        TCHAR* chunkStart = TNULL;

        while (bufferPos < bufferEnd)
        {
            size_t uncompressedLeft = TMath::Min<size_t>(size - (bufferPos - buffer), maxlength);

            TCHAR* offset = TNULL;
            size_t dataSize = 0;
            TBOOL hasOffset = compressor.FUN_0068af10(bufferPos, uncompressedLeft, offset, dataSize);

            if (hasOffset == TFALSE || dataSize < 3)
            {
                dataSize = 1;
                
                // Check if we have to start a new chunk
                if (chunkStart == TNULL) chunkStart = bufferPos;
                chunkSize += 1;

                if (maxlength <= chunkSize)
                {
                    // Write a chunk of data
                    compressedSize += TCompress::Write(chunkSize, chunkStart, file);
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
                    compressedSize += TCompress::Write(chunkSize, chunkStart, file);
                    chunkStart = TNULL;
                    chunkSize = 0;
                }

                compressedSize += TCompress::WriteOffset(dataSize, bufferPos - offset, bufferPos, file);
            }

            compressor.FUN_0068ae40(dataSize);
        }

        if (chunkStart != TNULL)
        {
            // We are out of the while loop but still have some chunk of data that should be written
            compressedSize += TCompress::Write(chunkSize, chunkStart, file);
        }

        TCompress::Header btecHeader;
        btecHeader.Magic = TFourCC("BTEC");
        btecHeader.Version = TVERSION(1, 2);
        btecHeader.CompressedSize = compressedSize;
        btecHeader.Size = size;

        if (isBigEndian)
        {
            btecHeader.Magic = PARSEDWORD_BIG(btecHeader.Magic);
            btecHeader.Version = PARSEDWORD_BIG(btecHeader.Version);
            btecHeader.CompressedSize = PARSEDWORD_BIG(btecHeader.CompressedSize);
            btecHeader.Size = PARSEDWORD_BIG(btecHeader.Size);
        }

        file->Seek(initialPos, TSEEK_SET);
        file->Write(&btecHeader, TCompress::HEADER_SIZE_12);
        file->Seek(compressedSize, TSEEK_CUR);

        return compressedSize;
    }
}
