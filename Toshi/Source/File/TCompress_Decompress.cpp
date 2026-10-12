#include "ToshiPCH.h"
#include "TCompress.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// The compressed data is read through a small buffer on the stack of Decompress.
// NOTE: names unknown, retail keeps the state in globals (007d3530..007d353c)
//-----------------------------------------------------------------------------
static constexpr TSIZE s_uiReadBufferSize = 1024;

static TBYTE*  s_pReadPos;    // 0x007d3530
static TBYTE*  s_pReadBuffer; // 0x007d3534
static TBYTE*  s_pReadEnd;    // 0x007d3538
static TUINT32 s_uiFileLeft;  // 0x007d353c, compressed bytes not yet read into the buffer

// Copies the rest of the buffer to a_pDst, refills the buffer from the file and copies more of it
// to get a_uiSize bytes. Returns the number of copied bytes.
// $Barnyard: FUNCTION 006dc280
static TSIZE RefillAndRead( TFile* a_pFile, TBYTE* a_pDst, TSIZE a_uiSize )
{
	TBYTE* pDst      = a_pDst;
	TINT   iBuffered = TINT( s_pReadEnd - s_pReadPos );

	if ( iBuffered > 0 )
	{
		TUtil::MemCopy( pDst, s_pReadPos, iBuffered );
		pDst += iBuffered;
	}

	TINT iToRead = TMath::Min( TINT( s_uiFileLeft ), TINT( s_uiReadBufferSize ) );
	TINT iRead   = TINT( a_pFile->Read( s_pReadBuffer, iToRead ) );

	s_pReadEnd = s_pReadBuffer + iRead;
	s_uiFileLeft -= iToRead;

	TINT iCopy = TMath::Min( iRead, TINT( a_uiSize - iBuffered ) );
	TUtil::MemCopy( pDst, s_pReadBuffer, iCopy );
	s_pReadPos = s_pReadBuffer + iCopy;

	return iBuffered + iCopy;
}

// Reads a_uiSize bytes to a_rpDst and moves it forward. Returns the number of read bytes.
// $Barnyard: FUNCTION 006dc4a0
static TSIZE ReadBuffered( TFile* a_pFile, TBYTE*& a_rpDst, TSIZE a_uiSize )
{
	if ( TINT( a_uiSize ) <= s_pReadEnd - s_pReadPos )
	{
		TUtil::MemCopy( a_rpDst, s_pReadPos, a_uiSize );
		a_rpDst += a_uiSize;
		s_pReadPos += a_uiSize;
		return a_uiSize;
	}

#ifdef BARNYARD_COMMUNITY_PATCH
	// Retail refills the buffer only once, so a raw chunk longer than the buffer (the compressor writes up to
	// maxlength bytes) was read only partially and the rest of it was then parsed as commands
	TSIZE uiTotal = 0;

	while ( uiTotal < a_uiSize )
	{
		TSIZE uiRead = RefillAndRead( a_pFile, a_rpDst, a_uiSize - uiTotal );
		a_rpDst += uiRead;
		uiTotal += uiRead;

		if ( uiRead == 0 )
			break;
	}

	return uiTotal;
#else
	TSIZE uiRead = RefillAndRead( a_pFile, a_rpDst, a_uiSize );
	a_rpDst += uiRead;

	return uiRead;
#endif
}

// $Barnyard: FUNCTION 006dc4f0
uintptr_t TCompress::Decompress( TFile* file, TCompress::Header* header, TBYTE* buffer, TUINT32 bufferSize )
{
	TPROFILER_SCOPE();

	TBYTE aReadBuffer[ s_uiReadBufferSize ];
	s_pReadPos    = aReadBuffer;
	s_pReadBuffer = aReadBuffer;
	s_pReadEnd    = aReadBuffer;
	s_uiFileLeft  = header->CompressedSize;

	if ( header->Magic != TFourCC( "BTEC" ) )
		return TCOMPRESS_ERROR_WRONG_MAGIC;
	// NOTE: retail only accepts version 1.2 (1.3 with the XOR comes from other TOSHI versions)
	if ( header->Version != TVERSION( 1, 2 ) && header->Version != TVERSION( 1, 3 ) )
		return TCOMPRESS_ERROR_WRONG_VERSION;
	if ( header->Size > bufferSize )
		return TCOMPRESS_ERROR_WRONG_SIZE;

	TBYTE*  pBufferPos     = buffer;
	TUINT32 compressedLeft = header->CompressedSize;

	while ( compressedLeft > 0 )
	{
		TUINT32 chunkSize;
		TBOOL   noOffset;
		TINT    offset;

		compressedLeft -= GetCommand( file, noOffset, chunkSize, offset );

		if ( !noOffset )
		{
			// The data is already unpacked so just copy it
			TBYTE* unpackedData = pBufferPos - offset;

			while ( chunkSize > 0 )
			{
				*( pBufferPos++ ) = *( unpackedData++ );
				chunkSize--;
			}
		}
		else
		{
			// The data wasn't previously unpacked, read it from file
			compressedLeft -= ReadBuffered( file, pBufferPos, chunkSize );
		}
	}

	if ( header->Version == TVERSION( 1, 3 ) )
	{
		pBufferPos = buffer;
		for ( TSIZE i = 0; i < header->Size; i++, pBufferPos++ )
		{
			*pBufferPos ^= header->XorValue;
		}
	}

	uintptr_t sizeDecompressed = pBufferPos - buffer;
	TASSERT( sizeDecompressed == header->Size, "Wrong decompressed size" );

	return sizeDecompressed;
}

// $Barnyard: FUNCTION 006dc220
int8_t TCompress::GetHeader( TFile* file, TCompress::Header& btecHeader )
{
	TUINT32 headerSize = TCompress::HEADER_SIZE_12;
	TUINT32 savedPos   = file->Tell();
	TSIZE   readedSize = file->Read( &btecHeader, headerSize );

	if ( ms_bIsBigEndian )
	{
		btecHeader.Size           = CONVERTENDIANESS( Endianess_Big, btecHeader.Size );
		btecHeader.CompressedSize = CONVERTENDIANESS( Endianess_Big, btecHeader.CompressedSize );
		btecHeader.Magic          = CONVERTENDIANESS( Endianess_Big, btecHeader.Magic );
		btecHeader.Version        = CONVERTENDIANESS( Endianess_Big, btecHeader.Version.Value );
	}

	if ( btecHeader.Version == TVERSION( 1, 3 ) )
	{
		readedSize += file->Read( &btecHeader.XorValue, sizeof( TCompress::Header::XorValue ) );
		headerSize += sizeof( TCompress::Header::XorValue );

		if ( ms_bIsBigEndian )
		{
			btecHeader.XorValue = CONVERTENDIANESS( Endianess_Big, btecHeader.XorValue );
		}
	}

	if ( readedSize != headerSize )
	{
		file->Seek( savedPos, TSEEK_SET );
		return TCOMPRESS_ERROR_WRONG_HEADERSIZE;
	}

	if ( btecHeader.Magic != TFourCC( "BTEC" ) )
	{
		file->Seek( savedPos, TSEEK_SET );
		return TCOMPRESS_ERROR_WRONG_MAGIC;
	}

	return TCOMPRESS_ERROR_OK;
}

// With the help of Revel8n approach
// Could use some code cleanup
// $Barnyard: FUNCTION 006dc340
TINT TCompress::GetCommand( TFile* file, TBOOL& noOffset, TUINT32& size, TINT& offset )
{
	// NOTE: retail inlines ReadBuffered for every byte
	TBYTE  uiByte;
	TBYTE* pByte = &uiByte;

	TINT read_count = TINT( ReadBuffered( file, pByte, 1 ) );
	size            = uiByte;
	TBOOL isBigSize = size & BTECSizeFlag_BigSize;
	noOffset        = size & BTECSizeFlag_NoOffset;
	offset          = -1;

	// by default size is 6 bits long
	size &= BTECSizeFlag_SizeMask;

	if ( isBigSize )
	{
		// now the size is 14 bits long
		pByte = &uiByte;
		read_count += TINT( ReadBuffered( file, pByte, 1 ) );
		size = ( size << 8 ) | uiByte;
	}

	if ( !noOffset )
	{
		// by default offset is 7 bits long
		pByte = &uiByte;
		read_count += TINT( ReadBuffered( file, pByte, 1 ) );
		offset          = uiByte;
		TBOOL bigOffset = offset & BTECOffsetFlag_BigOffset;
		offset &= BTECOffsetFlag_OffsetMask;

		if ( bigOffset )
		{
			// now the offset is 15 bits long
			pByte = &uiByte;
			read_count += TINT( ReadBuffered( file, pByte, 1 ) );
			offset = ( offset << 8 ) | uiByte;
		}
	}

	size += 1;
	offset += 1;

	return read_count;
}

TOSHI_NAMESPACE_END
