#include "ToshiPCH.h"
#include "TFileLexerUTF8.h"

#include <ctype.h>
#include <stdio.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS( TFileLexerUTF8 );

TSTATICASSERT( sizeof( TPString8 ) == sizeof( TPooledString8* ) );

// Word buffer shared by get_next_token and skipWhiteSpace
// NOTE: the size is estimated from the gap to the next variable in the PS2 bss
static TCHAR s_szWordBuffer[ 2000 ];

//-----------------------------------------------------------------------------
// Character classes of the PS2 C library (newlib), the character is cast to
// a signed char first, so only ASCII characters can match
//-----------------------------------------------------------------------------
static TBOOL IsAlpha( TINT a_iChar )
{
	TINT8 iChar = TINT8( a_iChar );
	return iChar >= 0 && isalpha( iChar );
}

static TBOOL IsAlNum( TINT a_iChar )
{
	TINT8 iChar = TINT8( a_iChar );
	return iChar >= 0 && isalnum( iChar );
}

static TBOOL IsDigit( TINT a_iChar )
{
	TINT8 iChar = TINT8( a_iChar );
	return iChar >= 0 && isdigit( iChar );
}

static TBOOL IsXDigit( TINT a_iChar )
{
	TINT8 iChar = TINT8( a_iChar );
	return iChar >= 0 && isxdigit( iChar );
}

static TBOOL IsSpace( TINT a_iChar )
{
	TINT8 iChar = TINT8( a_iChar );
	return iChar >= 0 && isspace( iChar );
}

static TCHAR ToLower( TINT a_iChar )
{
	return ( IsAlpha( a_iChar ) && isupper( a_iChar ) ) ? TCHAR( a_iChar + 32 ) : TCHAR( a_iChar );
}

// $NicktoonsPS2: FUNCTION 00391e10
// $JPOG: FUNCTION TKernelInterface.dll+0000f640
TFileLexerUTF8::TFileLexerUTF8()
    : m_aDefines( 10, 0 )
    , m_oEmitter( this )
{
	m_pFile                     = TNULL;
	m_bOutputComments           = TFALSE;
	m_iStringMode               = 0;
	m_iCharLookaheadSize        = 0;
	m_iCharLookaheadMask        = 0;
	m_piCharLookahead           = TNULL;
	m_iCharLookaheadBack        = 0;
	m_iCharLookaheadFront       = 0;
	m_iLine                     = 0;
	m_iTokenLookaheadSize       = 1;
	m_iTokenLookaheadMask       = 1;
	m_pLookaheadTokens          = new Token[ 1 ];
	m_iTokenLookaheadBuffered   = 0;
	m_iTokenLookaheadFront      = 0;
	m_iTokenLookaheadBack       = 0;
	m_oPreprocessorStack.m_iTop = 0;
	m_uiFlags                   = FLAGS_NONE;
	m_uiFlags |= FLAGS_ALLOWPREPROCESSOR;

	DefinePlatformSymbols();
}

// $NicktoonsPS2: FUNCTION 00391f40
// $JPOG: FUNCTION TKernelInterface.dll+0000f6f0
TFileLexerUTF8::TFileLexerUTF8( TFile* a_pInputStream, TINT a_iTokenLookaheadSize )
    : m_aDefines( 10, 0 )
    , m_oEmitter( this )
{
	m_pFile                     = a_pInputStream;
	m_bOutputComments           = TFALSE;
	m_iStringMode               = 0;
	m_iCharLookaheadSize        = 0;
	m_iCharLookaheadMask        = 0;
	m_piCharLookahead           = TNULL;
	m_iCharLookaheadBack        = 0;
	m_iCharLookaheadFront       = 0;
	m_iLine                     = 0;
	m_iTokenLookaheadSize       = 0;
	m_iTokenLookaheadMask       = 0;
	m_iTokenLookaheadBuffered   = 0;
	m_iTokenLookaheadFront      = 0;
	m_iTokenLookaheadBack       = 0;
	m_oPreprocessorStack.m_iTop = 0;
	m_uiFlags                   = FLAGS_NONE;

	m_iTokenLookaheadSize = RoundUpToPowerOfTwo( a_iTokenLookaheadSize );
	m_iTokenLookaheadMask = m_iTokenLookaheadSize - 1;
	m_uiFlags |= FLAGS_ALLOWPREPROCESSOR;
	m_pLookaheadTokens = new Token[ m_iTokenLookaheadSize ];

	SetInputStream( m_pFile );
	DefinePlatformSymbols();
}

// $NicktoonsPS2: FUNCTION 003920a8
// $JPOG: FUNCTION TKernelInterface.dll+0000f810
TFileLexerUTF8::~TFileLexerUTF8()
{
	if ( m_piCharLookahead )
		delete[] m_piCharLookahead;

	if ( m_pLookaheadTokens )
		delete[] m_pLookaheadTokens;

	// NOTE: Nicktoons' TEmitter destructor disconnects the listeners, ours doesn't
	m_oEmitter.Destroy();

	// NOTE: TArray doesn't destroy its elements
	for ( TINT i = m_aDefines.Size() - 1; i >= 0; i-- )
		m_aDefines[ i ].~TPString8();

	m_aDefines.FreeData();
}

// Defines the symbols of the platform the lexer runs on
// NOTE: the name is unknown. Nicktoons defines TOSHI_SKU_PS2, TOSHI_REGION_AU and USER_USER_NAME
// $NicktoonsPS2: FUNCTION 00391dc0
void TFileLexerUTF8::DefinePlatformSymbols()
{
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

// $NicktoonsPS2: FUNCTION 003922a8
// $JPOG: FUNCTION TKernelInterface.dll+0000f8d0
void TFileLexerUTF8::SetCharacterLookaheadSize( TINT a_iLookaheadSize )
{
	TASSERT( m_pFile != TNULL );
	TASSERT( m_piCharLookahead == TNULL );

	m_iCharLookaheadBack  = 0;
	m_iCharLookaheadFront = 0;
	m_iCharLookaheadSize  = RoundUpToPowerOfTwo( a_iLookaheadSize );
	m_iCharLookaheadMask  = m_iCharLookaheadSize - 1;
	m_piCharLookahead     = new TINT[ m_iCharLookaheadSize ];

	for ( TINT i = 0; i < m_iCharLookaheadSize; i++ )
		m_piCharLookahead[ i ] = m_pFile->GetCChar();
}

// $NicktoonsPS2: FUNCTION 00392350
// $JPOG: FUNCTION TKernelInterface.dll+0000f890
void TFileLexerUTF8::SetInputStream( TFile* a_pInputStream )
{
	m_iLine = 1;
	m_pFile = a_pInputStream;

	m_oPreprocessorStack.Clear();
	*m_oPreprocessorStack.Top() = TTRUE;

	if ( ComputePreprocessorAllow() )
		m_uiFlags |= FLAGS_ALLOWPREPROCESSOR;
	else
		m_uiFlags &= ~FLAGS_ALLOWPREPROCESSOR;

	SetCharacterLookaheadSize( 3 );
}

// Returns TTRUE if every #ifdef block around the current position is enabled
// $NicktoonsPS2: FUNCTION 003923d8
// $JPOG: FUNCTION TKernelInterface.dll+000106c0
TBOOL TFileLexerUTF8::ComputePreprocessorAllow()
{
	for ( TINT i = 0; i < m_oPreprocessorStack.Size(); i++ )
	{
		if ( *m_oPreprocessorStack.At( i ) == 0 )
			return TFALSE;
	}

	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 00392440
// $JPOG: FUNCTION TKernelInterface.dll+0000fb60
TBOOL TFileLexerUTF8::Expect( TokenType a_eType )
{
	Token oToken = GetNextToken();
	return oToken.GetType() == a_eType;
}

// $NicktoonsPS2: FUNCTION 00392538
// $JPOG: FUNCTION TKernelInterface.dll+0000fbf0
TBOOL TFileLexerUTF8::Expect( TokenType a_eType, Token& a_rToken )
{
	Token oToken = GetNextToken();
	a_rToken     = oToken;

	return a_rToken.GetType() == a_eType;
}

// $NicktoonsPS2: FUNCTION 003925f0
// $JPOG: FUNCTION TKernelInterface.dll+0000fc60
TFileLexerUTF8::Token TFileLexerUTF8::PeekNextToken( TINT a_iDistance )
{
	while ( a_iDistance >= m_iTokenLookaheadBuffered )
	{
		TINT iBack                  = m_iTokenLookaheadBack;
		m_pLookaheadTokens[ iBack ] = get_next_token();
		m_iTokenLookaheadBack       = ( m_iTokenLookaheadBack + 1 ) & m_iTokenLookaheadMask;
		m_iTokenLookaheadBuffered++;
	}

	m_oToken = m_pLookaheadTokens[ ( m_iTokenLookaheadFront + a_iDistance ) & m_iTokenLookaheadMask ];
	return Token( m_oToken );
}

// $NicktoonsPS2: FUNCTION 00392750
// $JPOG: FUNCTION TKernelInterface.dll+0000fd50
TFileLexerUTF8::Token TFileLexerUTF8::GetNextToken()
{
	if ( m_iTokenLookaheadBuffered > 0 )
	{
		m_oToken               = m_pLookaheadTokens[ m_iTokenLookaheadFront ];
		m_iTokenLookaheadFront = ( m_iTokenLookaheadFront + 1 ) & m_iTokenLookaheadMask;
		m_iTokenLookaheadBuffered--;
	}
	else
	{
		m_oToken = get_next_token();
	}

	return Token( m_oToken );
}

// $NicktoonsPS2: FUNCTION 00392868
// $JPOG: FUNCTION TKernelInterface.dll+00010710
TFileLexerUTF8::Token TFileLexerUTF8::get_next_token()
{
	if ( m_uiFlags & FLAGS_EOF )
		return Token( TFileLexer::TOKEN_EOF, m_iLine );

	TBOOL bInclude = TFALSE;
	skipWhiteSpace( &bInclude );

	TINT iChar = peek();

	if ( iChar == -1 )
		return Token( TFileLexer::TOKEN_EOF, m_iLine );

	if ( bInclude && iChar != '"' )
	{
		ThrowError( "Have #include without quoted file name" );
		return Token( TFileLexer::TOKEN_EOF, 0 );
	}

	if ( m_uiFlags & FLAGS_COMMENTS )
	{
		TINT iLength = 0;

		if ( peek( 0 ) == '/' )
		{
			if ( peek( 1 ) == '/' )
			{
				// Line comment
				advance( 2 );

				iChar = peek();
				while ( iChar != '\n' && iChar != '\r' )
				{
					s_szWordBuffer[ iLength++ ] = TCHAR( iChar );
					advance();
					iChar = peek();
				}

				s_szWordBuffer[ iLength ] = '\0';
				return Token( TFileLexer::TOKEN_COMMENT, m_iLine, TPString8( s_szWordBuffer ) );
			}

			if ( peek( 1 ) == '*' )
			{
				// Block comment, can be nested. The closing */ isn't stored, nested /* are
				TINT iDepth = 1;
				advance( 2 );

				while ( TTRUE )
				{
					// NOTE: checks the previous character, so an end of file gets stored first
					if ( iChar == -1 )
					{
						ThrowError( "Unterminated block comment /* ... */" );
						return Token( TFileLexer::TOKEN_EOF, 0 );
					}

					iChar = peek( 0 );

					if ( iChar == '*' && peek( 1 ) == '/' )
					{
						iDepth--;
						advance( 2 );
					}
					else if ( iChar == '/' && peek( 1 ) == '*' )
					{
						iDepth++;
						s_szWordBuffer[ iLength++ ] = '/';
						s_szWordBuffer[ iLength++ ] = TCHAR( peek( 1 ) );
						advance( 2 );
					}
					else
					{
						if ( iChar == '\n' )
							m_iLine++;

						s_szWordBuffer[ iLength++ ] = TCHAR( iChar );
						advance();
					}

					if ( iDepth == 0 )
					{
						s_szWordBuffer[ iLength ] = '\0';
						return Token( TFileLexer::TOKEN_COMMENT, m_iLine, TPString8( s_szWordBuffer ) );
					}
				}
			}
		}
	}

	if ( IsAlpha( iChar ) || iChar == '_' )
	{
		// Identifier
		TBOOL bLowerCase = ( m_uiFlags & FLAGS_LOWERCASE ) != 0;
		TINT  iLength    = 1;

		s_szWordBuffer[ 0 ] = bLowerCase ? ToLower( iChar ) : TCHAR( iChar );
		advance();

		while ( TTRUE )
		{
			iChar = peek();

			if ( !IsAlNum( iChar ) && iChar != '_' && ( iChar != '\\' || m_iStringMode != 1 ) )
				break;

			s_szWordBuffer[ iLength++ ] = bLowerCase ? ToLower( iChar ) : TCHAR( iChar );
			advance();
		}

		s_szWordBuffer[ iLength ] = '\0';
		return Token( TFileLexer::TOKEN_IDENT, m_iLine, TPString8( s_szWordBuffer ) );
	}

	TBOOL bHasDigits;

	if ( IsDigit( iChar ) || iChar == '-' )
	{
		bHasDigits = iChar != '-';
	}
	else if ( iChar == '.' && IsDigit( peek( 1 ) ) )
	{
		bHasDigits = TFALSE;
	}
	else if ( iChar == '"' )
	{
		// String or the file name of an #include
		TINT iLength = 0;
		advance();
		iChar = peek();

		if ( iChar != '"' )
		{
			TINT iPrevChar;

			do
			{
				if ( iChar == '\\' && m_iStringMode == 0 )
				{
					advance();
					iChar = peek();

					if ( iChar == '\n' )
						m_iLine++;
					else if ( iChar == 'n' )
						iChar = '\n';

					s_szWordBuffer[ iLength++ ] = TCHAR( iChar );
					advance();
					iPrevChar = 0;
				}
				else
				{
					if ( iChar == '\n' )
						m_iLine++;

					s_szWordBuffer[ iLength++ ] = TCHAR( iChar );
					advance();
					iPrevChar = TCHAR( iChar );
				}

				iChar = peek();
			} while ( iChar != '"' || ( iPrevChar == '\\' && m_iStringMode == 0 ) );
		}

		advance();
		s_szWordBuffer[ iLength ] = '\0';

		return Token( bInclude ? TFileLexer::TOKEN_INCLUDE : TFileLexer::TOKEN_STRING, m_iLine, TPString8( s_szWordBuffer ) );
	}
	else
	{
		TokenType eType;

		switch ( iChar )
		{
			case '!': eType = TFileLexer::TOKEN_EXCLAMATION; break;
			case '#':
				ThrowError( "Found '#' in wrong place.  '#ifdef' statements must be at the start of a new line." );
				return Token( TFileLexer::TOKEN_EOF, 0 );
			case '$': eType = TFileLexer::TOKEN_DOLLAR; break;
			case '(': eType = TFileLexer::TOKEN_OPENPAREN; break;
			case ')': eType = TFileLexer::TOKEN_CLOSEPAREN; break;
			case '*': eType = TFileLexer::TOKEN_ASTERISK; break;
			case ',': eType = TFileLexer::TOKEN_COMMA; break;
			case '.': eType = TFileLexer::TOKEN_DOT; break;
			case ':': eType = TFileLexer::TOKEN_COLON; break;
			case ';': eType = TFileLexer::TOKEN_SEMI; break;
			case '<': eType = TFileLexer::TOKEN_LESSTHAN; break;
			case '=': eType = TFileLexer::TOKEN_EQUAL; break;
			case '>': eType = TFileLexer::TOKEN_GREATERTHAN; break;
			case '[': eType = TFileLexer::TOKEN_OPENSQR; break;
			case ']': eType = TFileLexer::TOKEN_CLOSESQR; break;
			case '{': eType = TFileLexer::TOKEN_OPENBRACE; break;
			case '}': eType = TFileLexer::TOKEN_CLOSEBRACE; break;
			default:
			{
				TCHAR szError[ 128 ];
				T2String8::Format( szError, sizeof( szError ), "Invalid character 0x%04x '%c'", iChar, iChar );
				ThrowError( szError );
				return Token( TFileLexer::TOKEN_EOF, 0 );
			}
		}

		advance();
		return Token( eType, m_iLine );
	}

	// Number (or a lone hyphen)
	TINT  iLength   = 0;
	TBOOL bFloat    = TFALSE;
	TBOOL bHex      = TFALSE;
	TBOOL bUnsigned = TFALSE;

	if ( iChar == '.' )
	{
		s_szWordBuffer[ 0 ] = '0';
		bFloat              = TTRUE;
		iLength             = 1;
	}

	s_szWordBuffer[ iLength++ ] = TCHAR( iChar );
	advance();
	iChar = peek();

	if ( iChar == 'x' || iChar == 'X' )
	{
		// The leading 0 isn't kept
		iLength = 0;
		bHex    = TTRUE;

		if ( IsXDigit( peek( 1 ) ) )
		{
			bHasDigits = TTRUE;

			while ( TTRUE )
			{
				advance();
				iChar = peek();

				if ( !IsXDigit( iChar ) )
					break;

				s_szWordBuffer[ iLength++ ] = TCHAR( iChar );
			}

			if ( iChar == 'u' )
			{
				bUnsigned = TTRUE;
				advance();
			}
		}
	}
	else
	{
		while ( TTRUE )
		{
			iChar = peek();

			if ( IsDigit( iChar ) )
			{
				bHasDigits = TTRUE;
			}
			else if ( iChar == '.' )
			{
				if ( bFloat )
				{
					ThrowError( "Floating point number has multiple decimal places" );
					return Token( TFileLexer::TOKEN_EOF, 0 );
				}

				bFloat = TTRUE;
			}
			else
			{
				break;
			}

			s_szWordBuffer[ iLength++ ] = TCHAR( iChar );
			advance();
		}

		if ( iChar == 'f' && bHasDigits )
		{
			bFloat = TTRUE;
			advance();
		}
		else if ( iChar == 'u' && bHasDigits )
		{
			bUnsigned = TTRUE;
			advance();
		}
		else if ( ( iChar == 'e' || iChar == 'E' ) && bHasDigits )
		{
			s_szWordBuffer[ iLength++ ] = TCHAR( iChar );
			advance();
			iChar = peek();

			if ( iChar != '+' && iChar != '-' )
			{
				ThrowError( "Exponential number not of form e+ or e-" );
				return Token( TFileLexer::TOKEN_EOF, 0 );
			}

			do
			{
				s_szWordBuffer[ iLength++ ] = TCHAR( iChar );
				advance();
				iChar = peek();
			} while ( IsDigit( iChar ) );

			bFloat = TTRUE;

			if ( iChar == 'f' )
				advance();
		}
		else if ( iChar == '#' )
		{
			// 1.#INF, 1.#QNAN...
			s_szWordBuffer[ iLength++ ] = '#';
			advance();

			while ( IsAlpha( peek() ) )
			{
				s_szWordBuffer[ iLength++ ] = TCHAR( peek() );
				advance();
			}

			bFloat = TTRUE;
		}
	}

	s_szWordBuffer[ iLength ] = '\0';

	if ( !bHasDigits )
		return Token( TFileLexer::TOKEN_HYPHEN, m_iLine );

	// NOTE: the values are zeroed here, the original leaves them uninitialised if sscanf fails
	if ( bUnsigned )
	{
		TUINT32 uiValue = 0;
		sscanf( s_szWordBuffer, bHex ? "%x" : "%u", &uiValue );

		return Token( m_iLine, uiValue );
	}

	if ( bHex )
	{
		TINT iValue = 0;
		sscanf( s_szWordBuffer, "%x", &iValue );

		return Token( m_iLine, iValue );
	}

	if ( bFloat )
	{
		TFLOAT fValue = 0.0f;
		sscanf( s_szWordBuffer, "%f", &fValue );

		return Token( m_iLine, fValue );
	}

	// Decimal integer
	TINT         iValue  = 0;
	TBOOL        bNegate = s_szWordBuffer[ 0 ] == '-';
	const TCHAR* pcChar  = &s_szWordBuffer[ bNegate ? 1 : 0 ];

	while ( *pcChar != '\0' )
		iValue = iValue * 10 + *( pcChar++ ) - '0';

	return Token( m_iLine, bNegate ? -iValue : iValue );
}

// $NicktoonsPS2: FUNCTION 00393d00
// $JPOG: FUNCTION TKernelInterface.dll+0000f950
void TFileLexerUTF8::Define( const TCHAR* a_szDefinition )
{
	TPString8 strDefinition( a_szDefinition );

	// Binary search by the pooled string pointers
	TINT  iLow   = 0;
	TINT  iHigh  = m_aDefines.Size() - 1;
	TBOOL bFound = TFALSE;

	while ( iHigh >= iLow )
	{
		TINT iMiddle  = ( iLow + iHigh ) / 2;
		TINT iCompare = -m_aDefines[ iMiddle ].Compare( strDefinition );

		if ( iCompare < 0 )
		{
			iHigh = iMiddle - 1;
		}
		else if ( iCompare > 0 )
		{
			iLow = iMiddle + 1;
		}
		else
		{
			bFound = TTRUE;
			break;
		}
	}

	if ( bFound )
		return;

	// Insert at iLow
	if ( m_aDefines.SizeAllocated() < m_aDefines.Size() + 1 )
	{
		TINT iGrowSize = ( m_aDefines.GetGrowSize() > 1 ) ? m_aDefines.GetGrowSize() : 1;
		m_aDefines.Resize( m_aDefines.SizeAllocated() + iGrowSize );
	}

	TINT iSize = m_aDefines.Size();
	m_aDefines.SetSize( iSize + 1 );
	new ( &m_aDefines[ iSize ] ) TPString8();

	for ( TINT i = iSize - 1; i >= iLow; i-- )
		m_aDefines[ i + 1 ] = m_aDefines[ i ];

	m_aDefines[ iLow ] = strDefinition;
}

// $JPOG: FUNCTION TKernelInterface.dll+0000fa00
void TFileLexerUTF8::Undef( const TCHAR* a_szDefinition )
{
	TPString8 strDefinition( a_szDefinition );

	for ( TINT i = 0; i < m_aDefines.Size(); i++ )
	{
		if ( m_aDefines[ i ] == strDefinition )
		{
			// Move the following ones down
			TINT iLast = m_aDefines.Size() - 1;

			for ( TINT k = i; k < iLast; k++ )
				m_aDefines[ k ] = m_aDefines[ k + 1 ];

			m_aDefines[ iLast ].~TPString8();
			m_aDefines.SetSize( iLast );
			return;
		}
	}
}

// $NicktoonsPS2: FUNCTION 00394198
// $JPOG: FUNCTION TKernelInterface.dll+0000fac0
TBOOL TFileLexerUTF8::IfDef( const TCHAR* a_szDefinition )
{
	for ( TINT i = 0; i < m_aDefines.Size(); i++ )
	{
		if ( TStringManager::String8Compare( a_szDefinition, m_aDefines[ i ].GetString(), -1 ) == 0 )
			return TTRUE;
	}

	return TFALSE;
}

// Skips spaces, comments and preprocessor directives. a_pbInclude is set if an #include was found,
// get_next_token then reads its file name
// $NicktoonsPS2: FUNCTION 003942a8
// $JPOG: FUNCTION TKernelInterface.dll+0000fea0
void TFileLexerUTF8::skipWhiteSpace( TBOOL* a_pbInclude )
{
	*a_pbInclude = TFALSE;

	while ( TTRUE )
	{
		// Spaces, also skips the UTF-16 byte order mark
		TBOOL bNewLine = TFALSE;
		TINT  iChar    = peek();

		while ( IsSpace( iChar ) || iChar == 0xFEFF )
		{
			advance();

			if ( iChar == '\n' )
			{
				bNewLine = TTRUE;
				m_iLine++;
			}
			else
			{
				bNewLine = TFALSE;
			}

			iChar = peek();
		}

		TBOOL bDirective;

		if ( !( m_uiFlags & FLAGS_ALLOWPREPROCESSOR ) )
		{
			// Inside a disabled #ifdef block: skip everything up to the next '#' at the start of a line
			iChar = peek();

			while ( iChar != '#' || !bNewLine )
			{
#ifdef BARNYARD_COMMUNITY_PATCH
				// NOTE: the original loops forever when a disabled block isn't closed before the end of the file
				if ( iChar == -1 )
					return;
#endif

				advance();

				if ( iChar == '\n' )
				{
					bNewLine = TTRUE;
					m_iLine++;
				}
				else
				{
					bNewLine = TFALSE;
				}

				iChar = peek();
			}

			bDirective = TTRUE;
		}
		else
		{
			// NOTE: a directive on the first line of a file isn't found, it has no new line before it
			bDirective = bNewLine;
		}

		if ( bDirective && peek() == '#' )
		{
			// Preprocessor directive
			advance();

			TINT iLength = 0;
			iChar        = peek();

			while ( !IsSpace( iChar ) && iChar != -1 )
			{
				s_szWordBuffer[ iLength++ ] = TCHAR( iChar );
				advance();
				iChar = peek();
			}

			s_szWordBuffer[ iLength ] = '\0';

			enum DIRECTIVE
			{
				DIRECTIVE_IFDEF,
				DIRECTIVE_IFNDEF,
				DIRECTIVE_ELSE,
				DIRECTIVE_ENDIF,
				DIRECTIVE_STOP,
				DIRECTIVE_INCLUDE,
			};

			DIRECTIVE eDirective;

			if ( TStringManager::String8Compare( s_szWordBuffer, "ifdef", -1 ) == 0 )
				eDirective = DIRECTIVE_IFDEF;
			else if ( TStringManager::String8Compare( s_szWordBuffer, "ifndef", -1 ) == 0 )
				eDirective = DIRECTIVE_IFNDEF;
			else if ( TStringManager::String8Compare( s_szWordBuffer, "else", -1 ) == 0 )
				eDirective = DIRECTIVE_ELSE;
			else if ( TStringManager::String8Compare( s_szWordBuffer, "endif", -1 ) == 0 )
				eDirective = DIRECTIVE_ENDIF;
			else if ( TStringManager::String8Compare( s_szWordBuffer, "stop", -1 ) == 0 )
				eDirective = DIRECTIVE_STOP;
			else if ( TStringManager::String8Compare( s_szWordBuffer, "include", -1 ) == 0 )
				eDirective = DIRECTIVE_INCLUDE;
			else
			{
				TString8 strError;
				strError.Format( "Unknown preprocessor command '#%s'", s_szWordBuffer );
				ThrowError( strError.GetString() );
				continue;
			}

			switch ( eDirective )
			{
				case DIRECTIVE_IFDEF:
				case DIRECTIVE_IFNDEF:
				{
					if ( peek() == ' ' )
					{
						do advance();
						while ( peek() == ' ' );
					}

					iLength = 0;
					iChar   = peek();

					while ( !IsSpace( iChar ) && iChar != -1 )
					{
						s_szWordBuffer[ iLength++ ] = TCHAR( iChar );
						advance();
						iChar = peek();
					}

					s_szWordBuffer[ iLength ] = '\0';

					if ( iChar == -1 )
					{
						ThrowError( "Unexpected EOF while processing preprocessor directive" );
						continue;
					}

					TBOOL bDefined = IfDef( s_szWordBuffer );
					TINT  iState   = ( eDirective == DIRECTIVE_IFNDEF ) ? !bDefined : bDefined;
					m_oPreprocessorStack.Push( iState );
					break;
				}
				case DIRECTIVE_ELSE:
					if ( m_oPreprocessorStack.IsEmpty() )
					{
						ThrowError( "Unexpected #else directive" );
						continue;
					}

					*m_oPreprocessorStack.Top() = *m_oPreprocessorStack.Top() ^ 1;
					break;
				case DIRECTIVE_ENDIF:
					if ( m_oPreprocessorStack.IsEmpty() )
					{
						ThrowError( "Unexpected #endif directive" );
						continue;
					}

					m_oPreprocessorStack.Pop();
					break;
				case DIRECTIVE_STOP:
					if ( m_uiFlags & FLAGS_ALLOWPREPROCESSOR )
					{
						// Ends the file here
						for ( TINT i = 0; i < m_iTokenLookaheadSize; i++ )
							m_pLookaheadTokens[ i ] = Token( TFileLexer::TOKEN_EOF, m_iLine );

						m_piCharLookahead[ m_iCharLookaheadBack ] = -1;
						m_uiFlags |= FLAGS_EOF;
						return;
					}

					break;
				case DIRECTIVE_INCLUDE:
					if ( peek() == ' ' )
					{
						do advance();
						while ( peek() == ' ' );
					}

					*a_pbInclude = TTRUE;
					return;
			}

			if ( eDirective != DIRECTIVE_STOP )
			{
				if ( ComputePreprocessorAllow() )
					m_uiFlags |= FLAGS_ALLOWPREPROCESSOR;
				else
					m_uiFlags &= ~FLAGS_ALLOWPREPROCESSOR;
			}

			// Skip the rest of the line
			iChar = peek();

			while ( iChar != '\n' && iChar != -1 )
			{
				advance();
				iChar = peek();
			}

			continue;
		}

		// Comments
		if ( m_uiFlags & FLAGS_COMMENTS )
			return;

		if ( peek( 0 ) != '/' )
			return;

		if ( peek( 1 ) == '/' )
		{
			// Line comment, the new line is left for the next loop
			advance( 1 );

			do
			{
				advance();
				iChar = peek( 0 );
			} while ( iChar != -1 && iChar != '\n' );

			continue;
		}

		if ( peek( 1 ) != '*' )
			return;

		// Block comment, can be nested
		TINT iDepth = 1;
		advance( 2 );

		while ( TTRUE )
		{
			iChar = peek( 0 );

			if ( iChar == -1 )
			{
				ThrowError( "Unterminated block comment /* ... */" );
				return;
			}

			if ( iChar == '*' && peek( 1 ) == '/' )
			{
				iDepth--;
				advance( 2 );
			}
			else if ( iChar == '/' && peek( 1 ) == '*' )
			{
				iDepth++;
				advance( 2 );
			}
			else
			{
				if ( peek( 0 ) == '\n' )
					m_iLine++;

				advance();
			}

			if ( iDepth == 0 )
				break;
		}
	}
}

// $NicktoonsPS2: FUNCTION 00394de8
// $JPOG: FUNCTION TKernelInterface.dll+00010680
void TFileLexerUTF8::fillLookAhead()
{
	while ( m_iCharLookaheadFront != m_iCharLookaheadBack )
	{
		m_piCharLookahead[ m_iCharLookaheadFront ] = m_pFile->GetCChar();
		m_iCharLookaheadFront                      = ( m_iCharLookaheadFront + 1 ) & m_iCharLookaheadMask;
	}
}

// Returns the name of a single character token, "?????" for the others
// $NicktoonsPS2: FUNCTION 00394e60
// $JPOG: FUNCTION TKernelInterface.dll+0000fe00
const TCHAR* TFileLexerUTF8::tostring( TokenType a_eType )
{
	switch ( a_eType )
	{
		case TFileLexer::TOKEN_EOF: return "EOF";
		case TFileLexer::TOKEN_SEMI: return "SEMI";
		case TFileLexer::TOKEN_COLON: return "COLON";
		case TFileLexer::TOKEN_COMMA: return "COMMA";
		case TFileLexer::TOKEN_DOT: return "DOT";
		case TFileLexer::TOKEN_DOLLAR: return "DOLLAR";
		case TFileLexer::TOKEN_OPENSQR: return "OPENSQR";
		case TFileLexer::TOKEN_CLOSESQR: return "CLOSESQR";
		case TFileLexer::TOKEN_OPENBRACE: return "OPENBRACE";
		case TFileLexer::TOKEN_CLOSEBRACE: return "CLOSEBRACE";
		case TFileLexer::TOKEN_OPENPAREN: return "OPENPAREN";
		case TFileLexer::TOKEN_CLOSEPAREN: return "CLOSEPAREN";
		case TFileLexer::TOKEN_LESSTHAN: return "LESSTHAN";
		case TFileLexer::TOKEN_GREATERTHAN: return "GREATERTHAN";
		case TFileLexer::TOKEN_EQUAL: return "EQUAL";
		case TFileLexer::TOKEN_EXCLAMATION: return "EXCLAMATION";
		case TFileLexer::TOKEN_HYPHEN: return "HYPHEN";
		default: return "?????";
	}
}

// $NicktoonsPS2: FUNCTION 00394f60
// $JPOG: FUNCTION TKernelInterface.dll+00011700
TString8 TFileLexerUTF8::Token::tostring() const
{
	TString8 strToken;

	switch ( m_eType )
	{
		case TFileLexer::TOKEN_IDENT:
			strToken = TString8( "IDENT:" ) + GetString().GetString();
			break;
		case TFileLexer::TOKEN_STRING:
			strToken = TString8( "STRING:" ) + GetString().GetString();
			break;
		case TFileLexer::TOKEN_INCLUDE:
			strToken = TString8( "INCLUDE:" ) + GetString().GetString();
			break;
		case TFileLexer::TOKEN_INTEGER:
		{
			TString8 strPrefix = strToken;
			strToken.Format( "%sINTEGER:%d", strPrefix.GetString(), m_uValue.iValue );
			break;
		}
		case TFileLexer::TOKEN_UINTEGER:
		{
			TString8 strPrefix = strToken;
			strToken.Format( "%sUINTEGER:%d", strPrefix.GetString(), m_uValue.uiValue );
			break;
		}
		case TFileLexer::TOKEN_FLOAT:
		{
			TString8 strPrefix = strToken;
			strToken.Format( "%sFLOAT:%f", strPrefix.GetString(), m_uValue.fValue );
			break;
		}
		default:
			strToken = TFileLexerUTF8::tostring( m_eType );
			break;
	}

	TString8 strResult;
	strResult.Format( "Token[Line: %d, %s]", m_iLine, strToken.GetString() );

	return strResult;
}

// $NicktoonsPS2: FUNCTION 00395130
// $JPOG: FUNCTION TKernelInterface.dll+000118a0
void TFileLexerUTF8::Token::assign( const Token& a_rcOther )
{
	if ( IsStringType( m_eType ) )
	{
		// Release the string if the new value isn't one
		if ( !IsStringType( a_rcOther.m_eType ) )
			GetStringRef() = TSTATICCAST( TPooledString8, TNULL );
	}
	else if ( IsStringType( a_rcOther.m_eType ) )
	{
		m_uValue.pString = TNULL;
	}

	m_eType = a_rcOther.m_eType;
	m_iLine = a_rcOther.m_iLine;

	switch ( m_eType )
	{
		case TFileLexer::TOKEN_IDENT:
		case TFileLexer::TOKEN_STRING:
		case TFileLexer::TOKEN_COMMENT:
		case TFileLexer::TOKEN_INCLUDE:
			GetStringRef() = a_rcOther.GetString();
			break;
		case TFileLexer::TOKEN_INTEGER:
		case TFileLexer::TOKEN_UINTEGER:
			m_uValue.iValue = a_rcOther.m_uValue.iValue;
			break;
		case TFileLexer::TOKEN_FLOAT:
			m_uValue.fValue = a_rcOther.m_uValue.fValue;
			break;
	}
}

TOSHI_NAMESPACE_END
