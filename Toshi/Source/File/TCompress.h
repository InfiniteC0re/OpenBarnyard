#pragma once
#include <File/TFile.h>

namespace Toshi {

	enum TCOMPRESS_ERROR
	{
		TCOMPRESS_ERROR_OK = 0,
		TCOMPRESS_ERROR_WRONG_MAGIC = -1,
		TCOMPRESS_ERROR_WRONG_VERSION = -2,
		TCOMPRESS_ERROR_WRONG_SIZE = -3,
		TCOMPRESS_ERROR_WRONG_HEADERSIZE = -4
	};

    enum BTECSizeFlag
    {
        BTECSizeFlag_BigSize = BITFIELD(6),
        BTECSizeFlag_NoOffset = BITFIELD(7),
        BTECSizeFlag_SizeMask = BTECSizeFlag_BigSize - 1,
    };

    enum BTECOffsetFlag
    {
        BTECOffsetFlag_BigOffset = BITFIELD(7),
        BTECOffsetFlag_OffsetMask = BTECOffsetFlag_BigOffset - 1,
    };

    class TCompress
    {
    public:
		//-----------------------------------------------------------------------------
		// General Information
		//-----------------------------------------------------------------------------

        struct Header
        {
            uint32_t Magic;
            TVersion Version;
            uint32_t CompressedSize;
            uint32_t Size;
            uint32_t XorValue;
        };

        // Size of header that is common for any BTEC version
        static constexpr uint32_t HEADER_SIZE_12 = sizeof(Header) - sizeof(Header::XorValue);

        // Size of header that is common for BTEC 1.3
        static constexpr uint32_t HEADER_SIZE_13 = sizeof(Header);

	public:
		//-----------------------------------------------------------------------------
		// Compressor
		//-----------------------------------------------------------------------------

		static constexpr int maxlength = 0x4000;
		static int usemaxoffset;

	public:
		static size_t Compress(TFile* file, char* data, uint32_t size, uint32_t unused, TBOOL isBigEndian);

	private:
		static int WriteOffset(uint32_t length, int offset, char*& data, TFile* file);
		static int Write(uint32_t length, char*& data, TFile* file);

    public:
		//-----------------------------------------------------------------------------
		// Decompressor
		//-----------------------------------------------------------------------------

		static uintptr_t Decompress(TFile* file, TCompress::Header* header, char* buffer, uint32_t bufferSize);
		static int8_t GetHeader(TFile* file, TCompress::Header& btecHeader);
		static int GetCommand(TFile* file, TBOOL& hasOffset, uint32_t& size, int& offset);
    };
}