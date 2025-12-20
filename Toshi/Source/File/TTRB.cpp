#include "ToshiPCH.h"
#include "TTRB.h"
#include "TTSF.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

static constexpr TTRB::t_MemoryFuncAlloc s_cbDefAllocator = []( TTRB::AllocType alloctype, TUINT32 size, short unk1, TUINT32 unk2, void* userData ) {
	return TMalloc( size );
};

static constexpr TTRB::t_MemoryFuncDealloc s_cbDefDeallocator = []( TTRB::AllocType alloctype, void* ptr, short unk1, TUINT32 unk2, void* userData ) {
	TFree( ptr );
};

void* TTRB::s_pDefAllocatorUserData = TNULL;

// $Barnyard: FUNCTION 006ba9e0
TTRB::TTRB()
{
	m_pHeader = TNULL;
	m_SYMB    = TNULL;
	SetMemoryFunctions( s_cbDefAllocator, s_cbDefDeallocator, s_pDefAllocatorUserData );
}

// $Barnyard: FUNCTION 006baff0
TTRB::~TTRB()
{
	Close();
}

// $Barnyard: FUNCTION 006bb000
TTRB::ERROR TTRB::Load( const TCHAR* a_szFilePath, TUINT32 a_uiUnknown )
{
	TPROFILER_SCOPE();
	ERROR error = m_TTSFI.Open( a_szFilePath );

	if ( error == ERROR_OK )
	{
		if ( m_TTSFI.m_Magic == TFourCC( "TRBF" ) )
		{
			m_UNK = a_uiUnknown;

			if ( ProcessForm( m_TTSFI ) )
			{
				error = ERROR_OK;
			}
			else
			{
				error = ERROR_PARSE_ERROR;
			}
		}
		else
		{
			error = ERROR_NOT_TRBF;
		}
	}
	else
	{
		error = ERROR_NO_HEADER;
	}

	m_TTSFI.Close();
	return error;
}

// $Barnyard: FUNCTION 006baad0
TBOOL TTRB::ProcessForm( TTSFI& ttsf )
{
	TPROFILER_SCOPE();

	static constexpr TUINT32 MAX_RELC_NUM_BATCH = 512;
	RELCEntry                relcEntries[ MAX_RELC_NUM_BATCH ];

	TINT32 fileSize = ttsf.m_CurrentHunk.Size - 4;
	TINT32 leftSize = fileSize;
	ttsf.PushForm();

	do
	{
		TUINT32 sectionName = 0;
		TUINT32 sectionSize = 0;

		while ( TTRUE )
		{
			if ( fileSize < 1 )
			{
				ttsf.PopForm();
				return TTRUE;
			}

			uint8_t readResult = ttsf.ReadHunk();
			if ( readResult != ERROR_OK ) return TFALSE;

			sectionName = ttsf.m_CurrentHunk.Name;
			sectionSize = ttsf.m_CurrentHunk.Size;
			leftSize -= TAlignNumUp( sectionSize ) + sizeof( Toshi::TTSF::Hunk );

			if ( TFourCC( "HEAD" ) < sectionName ) break;

			if ( sectionName == TFourCC( "HEAD" ) )
			{
				TINT numsections = ( sectionSize - 4 ) / 0xC;

				m_pHeader                = static_cast<Header*>( m_MemAllocator( AllocType_Header, sizeof( Header ) + sizeof( SecInfo ) * numsections, 0, 0, m_MemUserData ) );
				m_pHeader->m_ui32Version = { 0 };

				ttsf.ReadRaw( &m_pHeader->m_i32SectionCount, sizeof( m_pHeader->m_i32SectionCount ) );

				TASSERT( m_pHeader->m_i32SectionCount == numsections, "HEAD section has wrong num of sections" );

				for ( TINT i = 0; i < m_pHeader->m_i32SectionCount; i++ )
				{
					SecInfo* pSect = GetSectionInfo( i );

					ttsf.ReadRaw( pSect, 0xC );
					pSect->m_Data = m_MemAllocator( AllocType_Data, pSect->m_Size, 0, 0, m_MemUserData );
					pSect->m_Unk1 = ( pSect->m_Unk1 == 0 ) ? 16 : pSect->m_Unk1;
					pSect->m_Unk2 = 0;
				}

				ttsf.SkipHunk();
			}
			else if ( sectionName == TFourCC( "SYMB" ) )
			{
				m_SYMB = static_cast<SYMB*>( m_MemAllocator( AllocType_Symbols, ttsf.m_CurrentHunk.Size, 0, 0, m_MemUserData ) );
				ttsf.ReadHunkData( m_SYMB );
			}
			else if ( sectionName == TFourCC( "SECC" ) )
			{
				Toshi::TCompress::ms_bIsBigEndian = ( CURRENT_ENDIANESS == Endianess_Big );

				for ( TINT i = 0; i < m_pHeader->m_i32SectionCount; i++ )
				{
					auto* secInfo = GetSectionInfo( i );

					if ( secInfo->m_Data != TNULL )
					{
						ttsf.ReadCompressed( secInfo->m_Data, secInfo->m_Size );
					}
				}

				ttsf.SkipHunk();
			}
			else if ( sectionName == TFourCC( "RELC" ) )
			{
				TUINT32 relocCount   = 0;
				TUINT32 curReloc     = 0;
				TUINT32 readedRelocs = 0;

				ttsf.ReadRaw( &relocCount, sizeof( relocCount ) );

				if ( relocCount < 1 )
				{
					relocCount = 0;
				}
				else
				{
					do
					{
						TUINT32 relocReadCount = relocCount - readedRelocs;

						// limit count of RELCs to read
						relocReadCount = TMath::Min( relocReadCount, MAX_RELC_NUM_BATCH );
						ttsf.Read( relcEntries, relocReadCount );
						curReloc = readedRelocs + relocReadCount;

						auto& header = *m_pHeader;
						for ( TUINT32 i = 0; i < relocReadCount; i++ )
						{
							auto& relcEntry = relcEntries[ i ];
							auto  hdrx1     = GetSectionInfo( relcEntry.HDRX1 );
							auto  hdrx2     = hdrx1;

							if ( m_pHeader->m_ui32Version.Value >= TVERSION( 1, 0 ) )
							{
								hdrx2 = GetSectionInfo( relcEntry.HDRX2 );
							}

							// this won't work in x64 because pointers in TRB files are always 4 bytes
							// need some workaround to support x64 again
							TSTATICASSERT( sizeof( void* ) == 4 );
							TUINTPTR* ptr = TREINTERPRETCAST( TUINTPTR*, TUINTPTR( hdrx1->m_Data ) + relcEntry.Offset );
							*ptr += TUINTPTR( hdrx2->m_Data );
						}

						readedRelocs += relocReadCount;
					} while ( curReloc < relocCount );
				}

				ttsf.SkipHunk();
			}
			else
			{
				// Unknown section
				ttsf.SkipHunk();
			}

			fileSize = leftSize;
		}

		if ( sectionName != TFourCC( "FORM" ) )
		{
			if ( sectionName == TFourCC( "SECT" ) )
			{
				for ( TINT i = 0; i < m_pHeader->m_i32SectionCount; i++ )
				{
					SecInfo* pSect = GetSectionInfo( i );
					ttsf.ReadRaw( pSect->m_Data, pSect->m_Size );
				}

				ttsf.SkipHunk();
			}
			else if ( sectionName == TFourCC( "HDRX" ) )
			{
				m_pHeader = TSTATICCAST( Header, m_MemAllocator( AllocType_Header, sectionSize, 0, 0, m_MemUserData ) );
				ttsf.ReadHunkData( m_pHeader );

				for ( TINT i = 0; i < m_pHeader->m_i32SectionCount; i++ )
				{
					SecInfo* pSect = GetSectionInfo( i );
					pSect->m_Unk1  = ( pSect->m_Unk1 == 0 ) ? 16 : pSect->m_Unk1;
					pSect->m_Data  = m_MemAllocator( AllocType_Data, pSect->m_Size, pSect->m_Unk1, pSect->m_Unk2, m_MemUserData );
				}
			}
			else
			{
				// Unknown section
				ttsf.SkipHunk();
			}

			fileSize = leftSize;
		}
	} while ( TTRUE );

	TFORM form;
	ttsf.ReadFORM( &form );

	TBOOL result = ProcessForm( ttsf );
	fileSize     = leftSize;

	return result;
}

// $Barnyard: FUNCTION 006bafa0
void* TTRB::GetSymbolAddress( const TCHAR* symbName )
{
	auto index = GetSymbolIndex( symbName );

	if ( m_SYMB != TNULL && index != -1 && index < m_SYMB->m_i32SymbCount )
	{
		auto entry = GetSymbol( index );
		return static_cast<TCHAR*>( GetSection( entry->HDRX ) ) + entry->DataOffset;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 006baeb0
TINT TTRB::GetSymbolIndex( const TCHAR* symbName )
{
	if ( m_SYMB != TNULL )
	{
		short hash = HashString( symbName );

		for ( TINT i = 0; i < m_SYMB->m_i32SymbCount; i++ )
		{
			auto symbol = GetSymbol( i );

			if ( symbol->NameHash == hash )
			{
				if ( Toshi::TStringManager::String8Compare( symbName, GetSymbolName( symbol ), -1 ) == 0 )
				{
					return i;
				}
			}
		}
	}

	return -1;
}

// $Barnyard: FUNCTION 006baa20
void TTRB::Close()
{
	if ( m_pHeader != TNULL )
	{
		for ( TINT i = 0; i < m_pHeader->m_i32SectionCount; i++ )
		{
			auto sec = GetSectionInfo( i );

			if ( sec->m_Data != TNULL )
			{
				m_MemDeallocator( AllocType_Data, sec->m_Data, 0, sec->m_Unk2, m_MemUserData );
			}
		}

		m_MemDeallocator( AllocType_Header, m_pHeader, 0, 0, m_MemUserData );
		m_pHeader = TNULL;
	}

	DeleteSymbolTable();
}

// $Barnyard: FUNCTION 006ba950
void TTRB::SetMemoryFunctions( t_MemoryFuncAlloc allocator, t_MemoryFuncDealloc deallocator, void* userdata )
{
	m_MemAllocator   = allocator;
	m_MemDeallocator = deallocator;
	m_MemUserData    = userdata;
}

// $Barnyard: FUNCTION 006ba8b0
void TTRB::DeleteSymbolTable()
{
	if ( m_SYMB != TNULL )
	{
		m_MemDeallocator( AllocType_Symbols, m_SYMB, 0, 0, m_MemUserData );
		m_SYMB = TNULL;
	}
}

TINT16 TTRB::HashString( const TCHAR* str )
{
	TINT16 hash = 0;
	TCHAR  character;

	while ( *str != '\0' )
	{
		character = *( str++ );
		hash      = (TINT16)character + hash * 0x1f;
	}

	return hash;
}

// $Barnyard: FUNCTION 006ba920
const TCHAR* TTRB::GetSymbolName( TTRBSymbol* symbol ) const
{
	if ( m_SYMB == TNULL )
	{
		return TNULL;
	}

	return reinterpret_cast<const TCHAR*>(
	    reinterpret_cast<uintptr_t>( m_SYMB ) +
	    GetSymbolTableSize( m_SYMB->m_i32SymbCount ) +
	    symbol->NameOffset
	);
}

const TCHAR* TTRB::GetSymbolName( TINT index ) const
{
	if ( m_SYMB == TNULL )
	{
		return TNULL;
	}

	return GetSymbolName( reinterpret_cast<TTRBSymbol*>( m_SYMB + 1 ) + index );
}

// $Barnyard: FUNCTION 006ba8f0
TTRB::TTRBSymbol* TTRB::GetSymbol( TINT index ) const
{
	if ( m_SYMB == TNULL )
	{
		return TNULL;
	}
	else if ( index < m_SYMB->m_i32SymbCount )
	{
		return reinterpret_cast<TTRBSymbol*>( m_SYMB + 1 ) + index;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 006baf60
TTRB::TTRBSymbol* TTRB::GetSymbol( const TCHAR* a_symbolName )
{
	TINT index = GetSymbolIndex( a_symbolName );
	if ( index == -1 )
	{
		return TNULL;
	}

	return GetSymbol( index );
}

TOSHI_NAMESPACE_END
