#include "ToshiPCH.h"
#include "TCompress_Decompress.h"

uintptr_t Toshi::TCompress_Decompress::Decompress(TFile* file, Header* header, void* buffer, uint32_t& size)
{
	if (header->Magic != TMAKEFOUR("BTEC")) return TCompress_ERROR::ERROR_WRONG_MAGIC;
	if (header->Version != TMAKEVERSION(1, 2)) return TCompress_ERROR::ERROR_WRONG_VERSION;
	if (header->CompressedSize > size) return TCompress_ERROR::ERROR_WRONG_SIZE;

	char* pBufferPos = static_cast<char*>(buffer);
	uint32_t sizedecompressed = header->UncompressedSize;

	while (sizedecompressed != 0)
	{
		int offset;
		uint32_t read_dst;
		uint32_t read_count;

		read_count = GetReadCount(file, read_dst, size, offset);
		sizedecompressed -= read_count;

		if (read_dst == 0)
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
			sizedecompressed -= readedCount;
		}
	}

	return static_cast<uintptr_t>(pBufferPos - buffer);
}

uint8_t Toshi::TCompress_Decompress::ReadHeader(TFile* file, Header& btecHeader)
{
	int pos = file->Tell();
	int read = file->Read(&btecHeader, sizeof(Header));

	if (read != sizeof(Header))
	{
		file->Seek(pos, TFile::TSEEK_SET);
		return TCompress_ERROR::ERROR_WRONG_HEADERSIZE;
	}

	if (btecHeader.Magic != TMAKEFOUR("BTEC"))
	{
		file->Seek(pos, TFile::TSEEK_SET);
		return TCompress_ERROR::ERROR_WRONG_MAGIC;
	}

	return TCompress_ERROR::ERROR_OK;
}

// With the help of Revel8n approach
// Could use some code cleanup
int Toshi::TCompress_Decompress::GetReadCount(TFile* file, uint32_t& read_dst, uint32_t& size, int& offset)
{
	int read_count = 0;
	read_count += file->Read(&size, 1);

	read_dst = size >> 7 & 1; // revel8n: size & 0x80 == 0

	if ((size & 0x40) == 0)
	{
		size &= 0x3F;
	}
	else
	{
		uint8_t byte;
		read_count += file->Read(&byte, 1);
		size = ((size & 0x3F) << 8) + byte;
	}

	size++;
	offset = 0;

	if (read_dst == 0)
	{
		read_count += file->Read(&offset, 1);

		if ((offset & 0x80) == 0)
		{
			offset & 0x7F;
		}
		else
		{
			uint8_t byte;
			read_count += file->Read(&byte, 1);
			offset = ((offset & 0x7F) << 8) + byte;
		}
		offset++;
	}

	return read_count;
}
