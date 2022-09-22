#include "ToshiPCH.h"
#include "TCompress_Decompress.h"

Toshi::TCompress_ERROR Toshi::TCompress_Decompress::Decompress(TFile* file, Header* header, void* buffer, uint32_t size)
{
	if (header->Magic != TMAKEFOUR("BTEC")) return TCompress_ERROR::ERROR_WRONG_MAGIC;
	if (header->Version != TMAKEVERSION(1, 2)) return TCompress_ERROR::ERROR_WRONG_VERSION;
	if (header->CompressedSize > size) return TCompress_ERROR::ERROR_WRONG_SIZE;

	int remaining = header->UncompressedSize;
	void* buff = buffer;
	int read_count;

	while (remaining != 0)
	{
		TTODO("ReadCountFunc");
		//read_count = 
		remaining -= read_count;
		if (buffer)
		{

		}
	}

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

int Toshi::TCompress_Decompress::GetReadCount()
{
	return 0;
}
