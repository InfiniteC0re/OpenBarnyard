#pragma once

namespace Toshi
{
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
        struct Header
        {
            uint32_t Magic;
            TVersion Version;
            uint32_t CompressedSize;
            uint32_t Size;
            uint32_t XorValue;
        };

    public:
        // Size of header that is common for any BTEC version
        static constexpr uint32_t HEADER_SIZE_12 = sizeof(Header) - sizeof(Header::XorValue);

        // Size of header that is common for BTEC 1.3
        static constexpr uint32_t HEADER_SIZE_13 = sizeof(Header);
    };
}