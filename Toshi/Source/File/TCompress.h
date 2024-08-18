#pragma once
#include "Toshi/TVersion.h"
#include "File/TFile.h"

TOSHI_NAMESPACE_START

enum TCOMPRESS_ERROR
{
	TCOMPRESS_ERROR_OK               = 0,
	TCOMPRESS_ERROR_WRONG_MAGIC      = -1,
	TCOMPRESS_ERROR_WRONG_VERSION    = -2,
	TCOMPRESS_ERROR_WRONG_SIZE       = -3,
	TCOMPRESS_ERROR_WRONG_HEADERSIZE = -4
};

enum BTECSizeFlag
{
	BTECSizeFlag_BigSize  = BITFLAG( 6 ),
	BTECSizeFlag_NoOffset = BITFLAG( 7 ),
	BTECSizeFlag_SizeMask = BTECSizeFlag_BigSize - 1,
};

enum BTECOffsetFlag
{
	BTECOffsetFlag_BigOffset  = BITFLAG( 7 ),
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
		TUINT32  Magic;
		TVersion Version;
		TUINT32  CompressedSize;
		TUINT32  Size;
		TUINT32  XorValue;
	};

	// Size of header that is common for any BTEC version
	static constexpr TUINT32 HEADER_SIZE_12 = sizeof( Header ) - sizeof( Header::XorValue );

	// Size of header that is common for BTEC 1.3
	static constexpr TUINT32 HEADER_SIZE_13 = sizeof( Header );

public:
	//-----------------------------------------------------------------------------
	// Compressor
	//-----------------------------------------------------------------------------

	static constexpr TINT maxlength = 0x4000;
	static TINT           usemaxoffset;

public:
	static size_t Compress( TFile* file, TCHAR* data, TUINT32 size, TUINT32 unused, TBOOL isBigEndian );

private:
	static TINT WriteOffset( TUINT32 length, TINT offset, TCHAR*& data, TFile* file );
	static TINT Write( TUINT32 length, TCHAR*& data, TFile* file );

public:
	//-----------------------------------------------------------------------------
	// Decompressor
	//-----------------------------------------------------------------------------

	static uintptr_t Decompress( TFile* file, TCompress::Header* header, TCHAR* buffer, TUINT32 bufferSize );
	static int8_t    GetHeader( TFile* file, TCompress::Header& btecHeader );
	static TINT      GetCommand( TFile* file, TBOOL& hasOffset, TUINT32& size, TINT& offset );
};

TOSHI_NAMESPACE_END
