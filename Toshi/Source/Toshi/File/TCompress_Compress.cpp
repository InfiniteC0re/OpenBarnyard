#include "ToshiPCH.h"
#include "TCompress_Compress.h"

int Toshi::TCompress_Compress::FUN_0068a830(int length, char* data, TFile* file)
{
    TASSERT(length <= maxlength, "");

    int bytesToWrite = 0;
    int lengthWritten, dataWritten = 0;
    int oldLength = length;

    if (length <= BTECSizeFlag_BigSize)
    {
        length &= 0xFFFFF00 | length - 1 & 0xFF | BTECSizeFlag_NoOffset;
        bytesToWrite = 1;
    }
    else
    {
        length -= 1 | 0xC0;
        length &= 0xFFFF0000 | length;
        bytesToWrite = 2;
    }

    lengthWritten = file->Write(&length, bytesToWrite);
    dataWritten = file->Write(data, oldLength);

    data += oldLength;

    return lengthWritten + dataWritten;
}

void Toshi::TCompress_Compress::Compress(TFile* file, char* bufferToCompress, uint32_t size, uint32_t unk3, uint8_t endian)
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
