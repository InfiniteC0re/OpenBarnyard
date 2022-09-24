#include "ToshiPCH.h"
#include "TCompress_Decompress.h"

uintptr_t Toshi::TCompress_Decompress::Decompress(TFile* file, Header* header, void* buffer, uint32_t& size)
{
	if (header->Magic != TMAKEFOUR("BTEC"))                                             return TCOMPRESS_ERROR_WRONG_MAGIC;
	if (header->Version != TMAKEVERSION(1, 2) && header->Version != TMAKEVERSION(1, 3)) return TCOMPRESS_ERROR_WRONG_VERSION;
	if (header->CompressedSize > size)                                                  return TCOMPRESS_ERROR_WRONG_SIZE;

	char* pBufferPos = static_cast<char*>(buffer);
	uint32_t sizeLeft = header->UncompressedSize;

	while (sizeLeft != 0)
	{
		int offset;
		bool hasOffset;

		sizeLeft -= GetReadCount(file, hasOffset, size, offset);

		if (hasOffset)
		{
			char* buf = pBufferPos - offset;
			int leftSize = size;

			while (leftSize != 0)
			{
				*(pBufferPos++) = *(buf++);
				leftSize--;
			}
		}
		else
		{
			size_t readedCount = file->Read(pBufferPos, size);
			pBufferPos += readedCount;
			sizeLeft -= readedCount;
		}
	}

	uint32_t sizeDecompressed = pBufferPos - static_cast<char*>(buffer);
	pBufferPos = static_cast<char*>(buffer);

	if (header->Version == TMAKEVERSION(1, 3))
	{
		for (size_t i = 0; i < header->UncompressedSize; i++)
		{
			*pBufferPos ^= header->XorValue;
		}
	}

	TASSERT(sizeDecompressed == header->CompressedSize, "TCompress_Decompress::Decompress: sizeDecompressed != header.CompressedSize");
	
	return static_cast<uintptr_t>(pBufferPos - static_cast<char*>(buffer));
}

int8_t Toshi::TCompress_Decompress::ReadHeader(TFile* file, Header& btecHeader, uint16_t& headerSize)
{
	int savedPos = file->Tell();
	size_t readedSize = file->Read(&btecHeader, sizeof(Header));

	int minusSize = 0;
	if (btecHeader.Version <= TMAKEVERSION(1, 2))
	{
		minusSize -= sizeof(btecHeader.XorValue);
	}

	file->Seek(minusSize, TFile::TSEEK_CUR);
	readedSize += minusSize;
	headerSize = file->Tell() - savedPos;

	if (readedSize != headerSize)
	{
		file->Seek(savedPos, TFile::TSEEK_SET);
		return TCOMPRESS_ERROR_WRONG_HEADERSIZE;
	}

	if (btecHeader.Magic != TMAKEFOUR("BTEC"))
	{
		file->Seek(savedPos, TFile::TSEEK_SET);
		return TCOMPRESS_ERROR_WRONG_MAGIC;
	}

	return TCOMPRESS_ERROR_OK;
}

// With the help of Revel8n approach
// Could use some code cleanup
int Toshi::TCompress_Decompress::GetReadCount(TFile* file, bool& hasOffset, uint32_t& size, int& offset)
{
	int read_count = file->Read(&size, 1);
	hasOffset = (size & BTECSizeFlag_NoOffset) == 0;
	offset = 0;

	if (size & BTECSizeFlag_BigSize)
	{
		// 14 bits value
		uint8_t addValue;
		read_count += file->Read(&addValue, sizeof(addValue));
		size = ((size & BTECSizeFlag_SizeMask) << (sizeof(addValue) * 8)) | addValue;
	}
	else
	{
		// 6 bits value
		size &= BTECSizeFlag_SizeMask;
	}

	size++;

	if (hasOffset)
	{
		read_count += file->Read(&offset, 1);

		if (offset & BTECOffsetFlag_BigOffset)
		{
			// 15 bits value
			uint8_t addValue;
			read_count += file->Read(&addValue, sizeof(addValue));
			offset = ((offset & BTECOffsetFlag_OffsetMask) << (sizeof(addValue) * 8)) | addValue;
		}
		else
		{
			// 7 bits value
			offset &= BTECOffsetFlag_OffsetMask;
		}

		offset++;
	}

	return read_count;
}
