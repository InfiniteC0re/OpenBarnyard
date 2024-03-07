#include "ToshiPCH.h"
#include "TCompress.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi
{
	uintptr_t TCompress::Decompress(TFile* file, TCompress::Header* header, TCHAR* buffer, TUINT32 bufferSize)
	{
		TPROFILER_SCOPE();

		if (header->Magic != TMakeFour("BTEC"))
			return TCOMPRESS_ERROR_WRONG_MAGIC;
		if (header->Version != TVERSION(1, 2) && header->Version != TVERSION(1, 3))
			return TCOMPRESS_ERROR_WRONG_VERSION;
		if (header->Size > bufferSize)
			return TCOMPRESS_ERROR_WRONG_SIZE;

		TCHAR* pBufferPos = buffer;
		TUINT32 compressedLeft = header->CompressedSize;

		while (compressedLeft > 0)
		{
			TUINT32 chunkSize;
			TBOOL noOffset;
			TINT offset;

			compressedLeft -= GetCommand(file, noOffset, chunkSize, offset);

			if (!noOffset)
			{
				// The data is already unpacked so just copy it
				TCHAR* unpackedData = pBufferPos - offset;
				
				while (chunkSize > 0)
				{
					*(pBufferPos++) = *(unpackedData++);
					chunkSize--;
				}
			}
			else
			{
				// The data wasn't previously unpacked, read it from file
				size_t readedCount = file->Read(pBufferPos, chunkSize);
				pBufferPos += readedCount;
				compressedLeft -= readedCount;
			}
		}

		if (header->Version == TVERSION(1, 3))
		{
			pBufferPos = buffer;
			for (size_t i = 0; i < header->Size; i++, pBufferPos++)
			{
				*pBufferPos ^= header->XorValue;
			}
		}

		uintptr_t sizeDecompressed = pBufferPos - buffer;
		TASSERT(sizeDecompressed == header->Size, "Wrong decompressed size");

		return sizeDecompressed;
	}

	int8_t TCompress::GetHeader(TFile* file, TCompress::Header& btecHeader)
	{
		TUINT32 headerSize = TCompress::HEADER_SIZE_12;
		TUINT32 savedPos = file->Tell();
		size_t readedSize = file->Read(&btecHeader, headerSize);

		if (btecHeader.Version == TVERSION(1, 3))
		{
			readedSize += file->Read(&btecHeader.XorValue, sizeof(TCompress::Header::XorValue));
			headerSize += sizeof(TCompress::Header::XorValue);
		}

		if (readedSize != headerSize)
		{
			file->Seek(savedPos, TSEEK_SET);
			return TCOMPRESS_ERROR_WRONG_HEADERSIZE;
		}

		if (btecHeader.Magic != TMakeFour("BTEC"))
		{
			file->Seek(savedPos, TSEEK_SET);
			return TCOMPRESS_ERROR_WRONG_MAGIC;
		}

		return TCOMPRESS_ERROR_OK;
	}

	// With the help of Revel8n approach
	// Could use some code cleanup
	TINT TCompress::GetCommand(TFile* file, TBOOL& noOffset, TUINT32& size, TINT& offset)
	{
		TINT read_count = file->Read(&size, 1);
		TBOOL isBigSize = size & BTECSizeFlag_BigSize;
		noOffset = size & BTECSizeFlag_NoOffset;
		offset = -1;

		// by default size is 6 bits long
		size &= BTECSizeFlag_SizeMask;

		if (isBigSize)
		{
			// now the size is 14 bits long
			uint8_t secondByte;
			read_count += file->Read(&secondByte, 1);
			size = (size << 8) | secondByte;
		}

		if (!noOffset)
		{
			// by default offset is 7 bits long
			read_count += file->Read(&offset, 1);
			TBOOL bigOffset = offset & BTECOffsetFlag_BigOffset;
			offset &= BTECOffsetFlag_OffsetMask;

			if (bigOffset)
			{	
				// now the offset is 15 bits long
				uint8_t secondByte;
				read_count += file->Read(&secondByte, 1);
				offset = (offset << 8) | secondByte;
			}
		}

		size += 1;
		offset += 1;

		return read_count;
	}
}