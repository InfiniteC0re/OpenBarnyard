#include "ToshiPCH.h"
#include "TTSF.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $Barnyard FUNCTION 006d8ca0
TUINT32 TTSFI::Open( TFile* a_pFile )
{
	m_pFile         = a_pFile;
	m_FileInfoCount = 0;
	m_UNKFLAG       = TFALSE;

	if ( m_pFile == TNULL )
	{
		return TTRB::ERROR_NO_FILE;
	}

	m_UNKFLAG = TTRUE;

	m_pFile->Read( &m_Header, sizeof( TTSFI::Header ) );

	if ( m_Header.Magic == IDMAGICL )
	{
		m_Endianess = Endianess_Little;
	}
	else
	{
		if ( m_Header.Magic != IDMAGICB )
		{
			return TTRB::ERROR_NOT_TRB;
		}

		m_Endianess = Endianess_Big;
	}

	m_ReadPos += m_pFile->Read( &m_Magic, 4 );

	if ( m_Endianess == Endianess_Big )
	{
		m_Header.FileSize = PARSEDWORD_BIG( m_Header.FileSize );
		m_Magic           = PARSEDWORD_BIG( m_Magic );
	}

	m_CurrentHunk.Name = m_Header.Magic;
	m_CurrentHunk.Size = m_Header.FileSize;
	PushForm();

	return TTRB::ERROR_OK;
}

// $Barnyard FUNCTION 006d8e70
TUINT32 TTSFI::Open( const TCHAR* a_szFilePath )
{
	TFile* pFile    = TFile::Create( a_szFilePath );
	auto   uiResult = Open( pFile );
	m_UNKFLAG       = TFALSE;
	return uiResult;
}

// $Barnyard FUNCTION 006d8be0
TUINT32 TTSFI::PushForm()
{
	if ( m_CurrentHunk.Name != TFourCC( "FORM" ) &&
	     m_CurrentHunk.Name != TFourCC( "TSFL" ) &&
	     m_CurrentHunk.Name != TFourCC( "TSFB" ) )
	{
		return TTRB::ERROR_WRONG_MAGIC;
	}

	m_FileInfo[ m_FileInfoCount ].FileStartOffset = m_pFile->Tell() - 4;
	m_FileInfo[ m_FileInfoCount ].FileSize        = m_CurrentHunk.Size;
	m_FileInfoCount++;

	return TTRB::ERROR_OK;
}

// $Barnyard FUNCTION 006d8c30
TUINT32 TTSFI::PopForm()
{
	if ( m_FileInfoCount < 1 ) return TTRB::ERROR_NO_FILEINFO_ON_STACK;

	m_FileInfoCount--;
	auto& fileInfo = m_FileInfo[ m_FileInfoCount ];

	m_CurrentHunk.Size = fileInfo.FileSize;
	m_ReadPos          = m_pFile->Tell() - fileInfo.FileStartOffset;

	TUINT32 alignedPos = TAlignNumUp( m_CurrentHunk.Size );
	m_pFile->Seek( alignedPos - m_ReadPos, TSEEK_CUR );
	m_ReadPos = alignedPos;

	return TTRB::ERROR_OK;
}

// $Barnyard FUNCTION 006d8a60
TUINT8 TTSFI::ReadHunk()
{
	m_pFile->Read( &m_CurrentHunk, sizeof( Hunk ) );

	if ( m_Endianess != Endianess_Little )
	{
		m_CurrentHunk.Name = PARSEDWORD_BIG( m_CurrentHunk.Name );
		m_CurrentHunk.Size = PARSEDWORD_BIG( m_CurrentHunk.Size );
	}

	m_ReadPos = 0;

	return TTRB::ERROR_OK;
}

// $Barnyard FUNCTION 006d8ba0
TUINT8 TTSFI::SkipHunk()
{
	TUINT32 alignedSize = TAlignNumUp( m_CurrentHunk.Size );
	m_pFile->Seek( alignedSize - m_ReadPos, TSEEK_CUR );
	m_ReadPos = alignedSize;

	return TTRB::ERROR_OK;
}

// $Barnyard FUNCTION 006d8ae0
TUINT8 TTSFI::ReadFORM( TFORM* section )
{
	if ( m_CurrentHunk.Name != TFourCC( "FORM" ) )
	{
		return TTRB::ERROR_WRONG_MAGIC;
	}

	m_ReadPos += m_pFile->Read( section, sizeof( TFORM ) );

	return TTRB::ERROR_OK;
}

// $Barnyard FUNCTION 006d8e00
TUINT8 TTSFI::ReadHunkData( void* dst )
{
	if ( m_CurrentHunk.Name == TFourCC( "FORM" ) )
	{
		return TTRB::ERROR_FORM_MAGIC;
	}

	TASSERT( m_ReadPos == 0, "m_ReadPos should be zero" );
	m_ReadPos += m_pFile->Read( dst, m_CurrentHunk.Size );
	m_ReadPos += TTSFI::ReadAlignmentPad();

	return TTRB::ERROR_OK;
}

// $Barnyard FUNCTION 006d8dc0
void TTSFI::Close( TBOOL free )
{
	PopForm();

	if ( m_pFile != TNULL && free )
	{
		m_pFile->Destroy();
	}

	m_pFile         = TNULL;
	m_FileInfoCount = 0;
}

// $Barnyard FUNCTION 006d8b40
void TTSFI::ReadCompressed( void* buffer, TUINT32 size )
{
	TCompress::Header header;

	TUINT32 headerStart = m_pFile->Tell();
	int8_t  error       = TCompress::GetHeader( m_pFile, header );

	if ( error == TCOMPRESS_ERROR_OK )
	{
		TUINT32 headerSize = m_pFile->Tell() - headerStart;
		TCompress::Decompress( m_pFile, &header, (TCHAR*)buffer, size );
		m_ReadPos += header.CompressedSize + headerSize;
	}
}

// $Barnyard FUNCTION 006d8a20
TTSFI::TTSFI()
{
	m_pFile         = TNULL;
	m_FileInfoCount = 0;
	m_UNKFLAG       = TFALSE;
	m_ReadPos       = 0;
	TUtil::MemClear( m_FileInfo, sizeof( m_FileInfo ) );
	m_Magic            = 0;
	m_CurrentHunk.Name = 0;
	m_CurrentHunk.Size = 0;
}

TTSFO::ERROR TTSFO::Create( const TCHAR* filepath, const TCHAR* magic, Endianess endianess )
{
	m_pFile = TFile::Create( filepath, TFILEMODE_CREATENEW | TFILEMODE_WRITE );

	if ( m_pFile != TNULL )
	{
		TTSF::Hunk hunk{ TFourCC( "TSFL" ), 0 };
		TUINT32    magicValue = TFourCC( magic );

		if ( endianess == Endianess_Big )
		{
			hunk.Name = TFourCC( "TSFB" );
			hunk.Size = PARSEDWORD_BIG( hunk.Size );
		}

		Write( hunk );
		Write( magicValue );

		m_Endianess                    = endianess;
		m_Positions[ m_PositionCount ] = 0;
		m_PositionCount += 1;

		return TTRB::ERROR_OK;
	}
	else
	{
		return TTSFO::ERROR_FILE;
	}
}

void TTSFO::Close()
{
	if ( m_pFile != TNULL )
	{
		EndForm();
		m_pFile->Destroy();
		m_pFile = TNULL;
	}
}

TSIZE TTSFO::BeginForm( const TCHAR* name )
{
	TASSERT( m_pFile != TNULL, "TTSFO is not created" );

	TTSF::Hunk hunk{ TFourCC( "FORM" ), 0 };
	auto       nameValue = TFourCC( name );

	if ( m_Endianess == Endianess_Big )
	{
		hunk.Name = PARSEDWORD_BIG( hunk.Name );
		nameValue = PARSEDWORD_BIG( nameValue );
	}

	m_Positions[ m_PositionCount++ ] = m_pFile->Tell();

	auto written1 = Write( hunk );
	auto written2 = Write( nameValue );

	return written1 + written2;
}

TSIZE TTSFO::EndForm()
{
	TASSERT( m_pFile != TNULL, "TTSFO is not created" );

	if ( m_PositionCount > 0 )
	{
		auto formPosition = m_Positions[ --m_PositionCount ];

		auto oldPos   = m_pFile->Tell();
		auto formSize = ( oldPos - formPosition ) - 8;

		if ( m_Endianess == Endianess_Big )
		{
			formSize = PARSEDWORD_BIG( formSize );
		}

		m_pFile->Seek( formPosition + 4, TSEEK_SET );
		Write( formSize );
		m_pFile->Seek( oldPos, TSEEK_SET );

		return formSize;
	}

	return 0;
}

TBOOL TTSFO::OpenHunk( HunkMark* hunkMark, const TCHAR* hunkName )
{
	TASSERT( hunkMark != TNULL, "HunkMark is TNULL" );
	hunkMark->Name = TFourCC( hunkName );
	hunkMark->Pos  = m_pFile->Tell();

	WriteHunk( hunkMark->Name, TNULL, 0 );
	return TTRUE;
}

TBOOL TTSFO::CloseHunk( HunkMark* hunkMark )
{
	TASSERT( hunkMark != TNULL, "HunkMark is TNULL" );
	auto oldPos = m_pFile->Tell();

	m_pFile->Seek( hunkMark->Pos, TSEEK_SET );
	WriteHunk( hunkMark->Name, TNULL, ( oldPos - hunkMark->Pos ) - sizeof( TTSF::Hunk ) );
	m_pFile->Seek( oldPos, TSEEK_SET );
	TTSFO::WriteAlignmentPad();

	return TTRUE;
}

TUINT32 TTSFO::WriteHunk( TUINT32 hunkName, void* buffer, TSIZE bufferSize )
{
	TTSF::Hunk hunk{ hunkName, bufferSize };

	// Convert endianess
	if ( m_Endianess == Endianess_Big )
	{
		hunk.Name = PARSEDWORD_BIG( hunk.Name );
		hunk.Size = PARSEDWORD_BIG( hunk.Size );
	}

	// Write hunk
	TSIZE writtenSize = Write( hunk );

	// Write buffer
	if ( buffer != TNULL && bufferSize > 0 )
	{
		writtenSize += m_pFile->Write( buffer, bufferSize );
	}

	// Return num of written bytes
	return writtenSize + TTSFO::WriteAlignmentPad();
}

TUINT32 TTSFI::ReadAlignmentPad()
{
	TASSERT( m_pFile != TNULL, "File is TNULL" );

	static TCHAR s_AlignBuffer[ 4 ];
	TUINT8       alignValue = 4 - ( m_pFile->Tell() & 3 );

	if ( alignValue != 4 )
	{
		return m_pFile->Read( s_AlignBuffer, alignValue );
	}

	return 0;
}

void TTSFI::ReadRaw( void* dst, TUINT32 size )
{
	m_ReadPos += m_pFile->Read( dst, size );
}

TOSHI_NAMESPACE_END
