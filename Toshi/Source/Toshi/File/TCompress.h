#pragma once
namespace Toshi
{
	enum TCompress_ERROR
	{
		ERROR_OK = 0,
		ERROR_WRONG_MAGIC = -1,
		ERROR_WRONG_VERSION = -2,
		ERROR_WRONG_SIZE = -3,
		ERROR_WRONG_HEADERSIZE = -4
	};
}