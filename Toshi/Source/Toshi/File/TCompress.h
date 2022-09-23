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
}