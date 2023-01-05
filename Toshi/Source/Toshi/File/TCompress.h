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
	
    };
}