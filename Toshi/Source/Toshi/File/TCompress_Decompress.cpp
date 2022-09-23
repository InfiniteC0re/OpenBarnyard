#include "ToshiPCH.h"
#include "TCompress_Decompress.h"

Toshi::TCompress_ERROR Toshi::TCompress_Decompress::Decompress(TFile* file, Header* header, void* buffer, uint32_t& size)
{
	if (header->Magic != TMAKEFOUR("BTEC")) return TCompress_ERROR::ERROR_WRONG_MAGIC;
	if (header->Version != TMAKEVERSION(1, 2)) return TCompress_ERROR::ERROR_WRONG_VERSION;
	if (header->CompressedSize > size) return TCompress_ERROR::ERROR_WRONG_SIZE;

	int remaining = header->UncompressedSize;
	int read_count = 0;
	int read_dst, offset;
	char* buffer2 = (char*)buffer;

	while (remaining != 0)
	{
		read_count = GetReadCount(file, read_dst, size, offset);
		remaining -= read_count;

		if (read_dst)
		{
			char* buf = buffer2 - offset;
			if (size > 0)
			{
				if (size < offset)
				{
					for (size_t i = 0; i < size; i++)
					{
						*buffer2++ = *buf++;
					}
				}
				else
				{
					int read = file->Read(buffer2, size);
					buffer2 += read;
					remaining -= read;
				}
			}
		}
	}

	//TASSERT()

	return TCompress_ERROR::ERROR_OK;
}

Toshi::TCompress_ERROR Toshi::TCompress_Decompress::ReadHeader(TFile* file, Header& btecHeader)
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
int Toshi::TCompress_Decompress::GetReadCount(TFile* file, int& read_dst, uint32_t& size, int& offset)
{
	int read_count = 0;
	read_count += file->Read(&size, 1);

	read_dst = size >> 7 & 1; // revel8n: size & 0x80 == 0

	if (size & 0x40)
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

	if (read_dst)
	{
		read_count += file->Read(&offset, 1);

		if (offset & 0x80)
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
