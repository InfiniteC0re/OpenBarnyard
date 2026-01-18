#include "ToshiPCH.h"
#include "PLexerTRB.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 006b4420
PLexerTRB::PLexerTRB()
    : m_pDebugDataTRB( TNULL )
    , m_pLineNums( TNULL )
    , m_pMainHeader( TNULL )
    , m_iTokenCounter( 0 )
    , m_iStringCounter( -1 )
{
}

// $Barnyard: FUNCTION 006b49a0
// $Barnyard: FUNCTION 006b48b0
PLexerTRB::~PLexerTRB()
{
	Close();
}

// $Barnyard: FUNCTION 006b48e0
TBOOL PLexerTRB::Load( const TCHAR* a_pchFileName, const TCHAR* a_pchDebugInfoFileName )
{
	Close();

	// Load main file
	TTRB::ERROR eTRBError = m_DataTRB.Load( a_pchFileName );
	if ( eTRBError != TTRB::ERROR_OK ) return TFALSE;

	m_pMainHeader = m_DataTRB.CastSymbol<TRBMainHeader>( "Main" );
	m_DataTRB.DeleteSymbolTable();
	TASSERT( m_pMainHeader != TNULL );

	// Load debug info if it presents
	if ( a_pchDebugInfoFileName )
	{
		m_pDebugDataTRB = new TTRB();
		m_pDebugDataTRB->Load( a_pchDebugInfoFileName );
		TVALIDPTR( m_pDebugDataTRB );

		m_pLineNums = m_pDebugDataTRB->CastSymbol<TINT32>( "LineNums" );
		m_pDebugDataTRB->DeleteSymbolTable();
	}

	// Reset state
	m_iTokenCounter  = 0;
	m_iStringCounter = -1;
	m_DataFilePath   = a_pchFileName;
	return TTRUE;
}

// $Barnyard: FUNCTION 006b4850
void PLexerTRB::Close()
{
	m_DataTRB.Close();
	m_pMainHeader = TNULL;
	m_DataFilePath.FreeBuffer();

	if ( m_pDebugDataTRB )
	{
		m_pDebugDataTRB->Close();
		delete m_pDebugDataTRB;
		m_pDebugDataTRB = TNULL;
	}

	m_pLineNums = TNULL;
}

// $Barnyard: FUNCTION 006b4470
TUINT8 PLexerTRB::PeekToken( TINT a_iTokenOffset )
{
	TVALIDPTR( m_pMainHeader );

	TINT iTokenIdx = m_iTokenCounter + a_iTokenOffset;
	if ( iTokenIdx >= 0 && iTokenIdx < m_pMainHeader->iNumTokens )
		return m_pMainHeader->pTokenData[ iTokenIdx ];

	return 0;
}

// $Barnyard: FUNCTION 006b4550
TBOOL PLexerTRB::GetNextToken( Token& a_rOutToken )
{
	TVALIDPTR( m_pMainHeader );

	a_rOutToken.iLineNumber = 0;
	a_rOutToken.pString     = TNULL;

	// Check the file is not over
	if ( m_pMainHeader->iNumTokens <= m_iTokenCounter )
	{
		a_rOutToken.uiToken = TFileLexer::TOKEN_EOF;
		return TFALSE;
	}

	TUINT uiToken       = m_pMainHeader->pTokenData[ m_iTokenCounter ];
	a_rOutToken.uiToken = uiToken;

	// Read associated string
	if ( uiToken >= TFileLexer::TOKEN_IDENT && uiToken < TFileLexer::TOKEN_NUMOF )
	{
		m_iStringCounter += 1;
		a_rOutToken.pString = m_pMainHeader->ppStrings[ m_iStringCounter ];
	}

	// Try obtaining line number
	if ( m_pLineNums )
		a_rOutToken.iLineNumber = m_pLineNums[ m_iTokenCounter ];

	m_iTokenCounter += 1;
	return TTRUE;
}

// $Barnyard: FUNCTION 006b45e0
TBOOL PLexerTRB::GetPrevToken( Token& a_rOutToken )
{
	TVALIDPTR( m_pMainHeader );

	a_rOutToken.iLineNumber = 0;
	a_rOutToken.pString     = TNULL;

	const TINT iCurrTokenIdx = m_iTokenCounter - 1;
	const TINT iPrevTokenIdx = m_iTokenCounter - 1;

	// Make sure it's not the end of the file
	if ( iPrevTokenIdx < 0 || iPrevTokenIdx >= m_pMainHeader->iNumTokens )
	{
		a_rOutToken.uiToken = TFileLexer::TOKEN_EOF;
		return TFALSE;
	}

	TUINT uiPrevToken   = m_pMainHeader->pTokenData[ iPrevTokenIdx ];
	a_rOutToken.uiToken = uiPrevToken;

	// Get associated string
	TINT iStringIdx = m_iStringCounter;
	if ( uiPrevToken >= TFileLexer::TOKEN_IDENT && uiPrevToken < TFileLexer::TOKEN_NUMOF )
	{
		// [1/19/2026 InfiniteC0re]
		// This was supposed to help getting the previous string, but it's broken lmao
		// Should use pTokenData[ iCurrTokenIdx ] instead of just iCurrTokenIdx
		if ( iCurrTokenIdx >= TFileLexer::TOKEN_IDENT && iCurrTokenIdx < TFileLexer::TOKEN_NUMOF )
			iStringIdx -= 1;

		a_rOutToken.pString = m_pMainHeader->ppStrings[ iStringIdx ];
	}

	// Try obtaining line number
	if ( m_pLineNums )
		a_rOutToken.iLineNumber = m_pLineNums[ iPrevTokenIdx ];

	// NOTE: return value looks like a mistake
	return TFALSE;
}

// $Barnyard: FUNCTION 006b4690
const TCHAR* PLexerTRB::GetTokenName( TUINT a_uiToken ) const
{
	TVALIDPTR( m_pMainHeader );

	// If one of defines tokens, get it from the file
	if ( a_uiToken >= TFileLexer::TOKEN_NUMOF && m_pMainHeader->uiNumTokens + TFileLexer::TOKEN_NUMOF > a_uiToken )
		return GetCustomTokens()[ a_uiToken - TFileLexer::TOKEN_NUMOF ];

	// Fallback to default tokens
	TASSERT( a_uiToken < TFileLexer::TOKEN_NUMOF );
	return TFileLexer::GetTokenName( TCAST( TFileLexer::TOKEN, a_uiToken ) );
}

// $Barnyard: FUNCTION 006b4680
const TCHAR** PLexerTRB::GetCustomTokens() const
{
	TVALIDPTR( m_pMainHeader );
	return m_pMainHeader->ppCustomTokens;
}

// $Barnyard: FUNCTION 006b47f0
void PLexerTRB::PrintError( Token* a_pToken, const TCHAR* a_pchError )
{
	TVALIDPTR( a_pToken );

	TString8 strMessage;
	strMessage.Format( "%s(%d) : error : %s\n", m_DataFilePath.GetString(), a_pchError, a_pToken->iLineNumber );

	TERROR( strMessage );
}
