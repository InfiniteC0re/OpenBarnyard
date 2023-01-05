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

    void TCompress_Compress::Compress(TFile* file, char* bufferToCompress, uint32_t size, uint32_t unk3, uint8_t endian)
    {
        int pos = file->Tell();
        file->Seek(0x10, TFile::TSEEK_CUR);
        usemaxoffset = 0x8000;
        TTODO("Implement FUN_0068AC60, parameters: local_490, param_2, param_3, 0x8000, 3");

        char* end = bufferToCompress + unk3;

        while (bufferToCompress < end)
        {
            /*if (bufferToCompress + (size - bufferToCompress) < 0x4000)
            {

            }*/
        }
    }
}
