#include "ToshiPCH.h"
#include "TFileLexerUTF8.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS_NORUNTIME( TFileLexerUTF8 );

TFileLexerUTF8::TFileLexerUTF8()
    : m_oEmitter( this )
{
	m_pFile                   = 0;
	m_bUnk1                   = TFALSE;
	m_iCharLookaheadSize      = 0;
	m_iUnk3                   = 0;
	m_piCharLookahead         = TNULL;
	m_iUnk4                   = 0;
	m_iUnk5                   = 0;
	m_iUnk6                   = 0;
	m_iTokenLookaheadSize     = 1;
	m_iTokenLookaheadMask     = 1;
	m_LookaheadTokens         = LookaheadTokens::Allocate()->GetTokens();
	m_iTokenLookaheadBuffered = 0;
	m_iTokenLookaheadFront    = 0;
	m_iTokenLookaheadBack     = 0;
	m_iSomeNum                = 0;
	m_bAllowPreprocessor      = TTRUE;
	m_bUnk7                   = TFALSE;

#if defined( TOSHI_SKU_WINDOWS )
	Define( "TOSHI_SKU_WINDOWS" );
#elif defined( TOSHI_SKU_PS2 )
	Define( "TOSHI_SKU_PS2" );
#elif defined( TOSHI_SKU_GC )
	Define( "TOSHI_SKU_GC" );
#elif defined( TOSHI_SKU_WII )
	Define( "TOSHI_SKU_WII" );
#else
#  error Wrong define?
#endif
}

TFileLexerUTF8::TFileLexerUTF8( TFile* a_pInputStream, TINT a_iTokenLookaheadSize )
    : m_oEmitter( this )
{
	m_pFile                   = 0;
	m_bUnk1                   = TFALSE;
	m_iCharLookaheadSize      = 0;
	m_iUnk3                   = 0;
	m_piCharLookahead         = TNULL;
	m_iUnk4                   = 0;
	m_iUnk5                   = 0;
	m_iUnk6                   = 0;
	m_iTokenLookaheadSize     = 1;
	m_iTokenLookaheadMask     = 1;
	m_LookaheadTokens         = LookaheadTokens::Allocate( a_iTokenLookaheadSize )->GetTokens();
	m_iTokenLookaheadBuffered = 0;
	m_iTokenLookaheadFront    = 0;
	m_iTokenLookaheadBack     = 0;
	m_iSomeNum                = 0;
	m_bAllowPreprocessor      = TTRUE;
	m_bUnk7                   = TFALSE;

	m_iTokenLookaheadSize = 1 << ( ( ( a_iTokenLookaheadSize * 2 + -1 ) >> 0x17 ) + 0x81U & 0x1f );
	m_iTokenLookaheadMask = ( m_iTokenLookaheadSize - 1 < 2 ) ? 1 : m_iTokenLookaheadSize - 1;

	TASSERT( a_pInputStream != TNULL )
	SetInputStream( a_pInputStream );

#if defined( TOSHI_SKU_WINDOWS )
	Define( "TOSHI_SKU_WINDOWS" );
#elif defined( TOSHI_SKU_PS2 )
	Define( "TOSHI_SKU_PS2" );
#elif defined( TOSHI_SKU_GC )
	Define( "TOSHI_SKU_GC" );
#elif defined( TOSHI_SKU_WII )
	Define( "TOSHI_SKU_WII" );
#else
#  error Wrong define?
#endif
}

TFileLexerUTF8::~TFileLexerUTF8()
{
	TIMPLEMENT();

	LookaheadTokens::Free( m_LookaheadTokens );
}

TBOOL TFileLexerUTF8::ComputePreprocessorAllow()
{
	for ( TINT i = 0; i < m_iSomeNum; i++ )
	{
		bool bFlag;

		if ( m_iSomeNum - i < 0 )
			bFlag = m_bFlags[ 0 ];
		else
			bFlag = m_bFlags[ m_iSomeNum - i ];

		if ( !bFlag )
			return TFALSE;
	}

	return TTRUE;
}

void TFileLexerUTF8::SetCharacterLookaheadSize( TINT a_iLookaheadSize )
{
	TASSERT( m_pFile != TNULL );
	TASSERT( m_piCharLookahead == TNULL );

	TINT iIntCount       = 1 << ( ( a_iLookaheadSize * 2 - 1 ) >> 0x17 ) + 0x81 & 0x1f;
	m_iCharLookaheadSize = iIntCount;
	m_iUnk3              = iIntCount;
	m_iUnk4              = 0;
	m_iUnk5              = 0;
	m_piCharLookahead    = new TINT[ iIntCount ];

	for ( TINT i = 0; i < iIntCount; i++ )
		m_piCharLookahead[ i ] = m_pFile->GetCChar();
}

void TFileLexerUTF8::SetInputStream( TFile* a_pInputStream )
{
	m_pFile              = a_pInputStream;
	m_iUnk6              = 1;
	m_iSomeNum           = 0;
	m_bFlags[ 0 ]        = TTRUE;
	m_bAllowPreprocessor = ComputePreprocessorAllow();
	SetCharacterLookaheadSize( 3 );
}

void TFileLexerUTF8::Define( const TCHAR* a_szDefinition )
{
	bool bAlreadyDefined = TFALSE;

	for ( TINT i = 0; i < m_Defines.Size(); i++ )
	{
		if ( m_Defines[ i ] == a_szDefinition )
		{
			bAlreadyDefined = TTRUE;
			break;
		}
	}

	if ( !bAlreadyDefined )
	{
		m_Defines.Push( a_szDefinition );
	}
}

void TFileLexerUTF8::Token::assign( const Token& token )
{
	TIMPLEMENT();
}

TOSHI_NAMESPACE_END
