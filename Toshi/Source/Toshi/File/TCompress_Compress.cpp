#include "ToshiPCH.h"
#include "TCompress_Compress.h"

namespace Toshi
{
    int TCompress_Compress::Write(uint32_t length, char*& data, TFile* file)
    {
        // 0068a830
        TASSERT(length <= maxlength, "");

        int bytesToWrite = 0;
        int lengthWritten, dataWritten = 0;
        uint32_t dataSize = length;

        length -= 1;

        if (length <= BTECSizeFlag_BigSize)
        {
            length = (length & 0xFF) | (BTECSizeFlag_NoOffset);
            bytesToWrite = 1;
        }
        else
        {
            length = (length & 0xFFFF) | (BTECSizeFlag_NoOffset | BTECSizeFlag_BigSize);
            bytesToWrite = 2;
        }

        lengthWritten = file->Write(&length, bytesToWrite);
        dataWritten = file->Write(data, dataSize);

        data += dataSize;

        return lengthWritten + dataWritten;
    }

    int TCompress_Compress::WriteOffset(uint32_t length, int offset, char*& data, TFile* file)
    {
        // 0068a8c0
        TASSERT(length <= maxlength, "Length is greater than {0}", maxlength);
        TASSERT(offset <= usemaxoffset, "Offset is greater than {0}", usemaxoffset);

        int bytesToWrite = 0;
        int writtenSize = 0;
        uint32_t dataSize = length;

        length -= 1;
        offset -= 1;

        if (length <= BTECSizeFlag_BigSize)
        {
            length = length & 0xFF;
            bytesToWrite = 1;
        }
        else
        {
            length = (length & 0xFFFF) | BTECSizeFlag_BigSize;
            bytesToWrite = 2;
        }

        writtenSize += file->Write(&length, bytesToWrite);

        if (offset <= BTECOffsetFlag_BigOffset)
        {
            offset = offset & 0xFF;
        }
        else
        {
            offset = (offset & 0xFFFF) | BTECOffsetFlag_BigOffset;
        }

        writtenSize += file->Write(&offset, bytesToWrite);
        data += dataSize;

        return 0;
    }

    void TCompress_Compress::Compress(TFile* file, char* buffer, uint32_t size, uint32_t unused, bool isBigEndian)
    {
        usemaxoffset = 0x8000;
        TTODO("Implement FUN_0068AC60, parameters: local_490, param_2, param_3, 0x8000, 3");

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

            bool hasOffset = false;
            size_t offset = 0;
            size_t dataSize = 0;
            TTODO("hasOffset = FUN_0068af10(&s1,bufferPos,toUncompressSize,&offset,&dataSize)");

            if (!hasOffset || dataSize < 3)
            {
                dataSize = 1;
                
                // Check if we have to start a new chunk
                if (chunkStart == TNULL) chunkStart = bufferPos;
                chunkSize += 1;

                if (chunkSize >= maxlength)
                {
                    // Write a chunk of data
                    compressedSize += TCompress_Compress::Write(chunkSize, chunkStart, file);
                    chunkStart = TNULL;
                    chunkSize = 0;
                }
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

                compressedSize += TCompress_Compress::WriteOffset(dataSize, (size_t)bufferPos - offset, bufferPos, file);
            }

            bufferPos += 1;
            TTODO("FUN_0068ae40(&s1,dataSize)");
        }

        if (chunkStart != TNULL)
        {
            // We are out of the while loop but still have some chunk of data that should be written
            compressedSize += TCompress_Compress::Write(chunkSize, chunkStart, file);
            chunkStart = TNULL;
            chunkSize = 0;
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
    }
}
