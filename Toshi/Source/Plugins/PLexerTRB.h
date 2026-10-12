#pragma once
#include "File/TTRB.h"
#include "File/TFileLexer.h"

#undef EOF

//-----------------------------------------------------------------------------
// Purpose: reader of precompiled TFileLexer data packed into TRB files
//-----------------------------------------------------------------------------
class PLexerTRB
{
public:
	struct TRBMainHeader
	{
		TUINT32       uiCount1;
		TUINT32       uiNumCustomIdents;
		TUINT8*       pTokens;
		TUINT32*      pTokenValues;
		TINT32        iNumFileTokens;
		const TCHAR** ppCustomIdents;
	};

	struct Token
	{
		TUINT uiTokenType;
		TINT  iLineNumber;

		union
		{
			TUINT32      uiValue;
			TINT32       iValue;
			TFLOAT       flValue;
			const TCHAR* pString;
		};

		Token() = default;

		// $Barnyard: FUNCTION 006b42f0
		Token( TUINT a_uiTokenType, TUINT32 a_uiValue, TINT a_iLineNumber )
		    : uiTokenType( a_uiTokenType ), iLineNumber( a_iLineNumber ), uiValue( a_uiValue ) {}

		// $Barnyard: FUNCTION 006b4310
		Token( TUINT a_uiTokenType, TINT32 a_iValue, TINT a_iLineNumber )
		    : uiTokenType( a_uiTokenType ), iLineNumber( a_iLineNumber ), iValue( a_iValue ) {}

		// $Barnyard: FUNCTION 006b4330
		Token( TUINT a_uiTokenType, TFLOAT a_flValue, TINT a_iLineNumber )
		    : uiTokenType( a_uiTokenType ), iLineNumber( a_iLineNumber ), flValue( a_flValue ) {}

		// $Barnyard: FUNCTION 006b4350
		Token( TUINT a_uiTokenType, const TCHAR* a_pString, TINT a_iLineNumber )
		    : uiTokenType( a_uiTokenType ), iLineNumber( a_iLineNumber ), pString( a_pString ) {}

		const TCHAR* GetIdent( const TCHAR** a_ppCustomTokens );

		// NOTE: unreferenced in retail, the names are invented
		// Returns TTRUE if the value of the token is a string: identifiers, strings,
		// comments, includes and custom identifiers
		// $Barnyard: FUNCTION 006b43a0
		TBOOL HasString() const
		{
			switch ( uiTokenType )
			{
				case Toshi::TFileLexer::TOKEN_IDENT:
				case Toshi::TFileLexer::TOKEN_STRING:
				case Toshi::TFileLexer::TOKEN_COMMENT:
				case Toshi::TFileLexer::TOKEN_INCLUDE:
					return TTRUE;
				default:
					return uiTokenType >= Toshi::TFileLexer::TOKEN_NUMOF;
			}
		}

		// Returns TTRUE if tokens of type a_iTokenType carry a value (TOKEN_IDENT..TOKEN_INCLUDE)
		// $Barnyard: FUNCTION 006b4400
		static TBOOL HasValue( TINT a_iTokenType )
		{
			return a_iTokenType >= Toshi::TFileLexer::TOKEN_IDENT && a_iTokenType <= Toshi::TFileLexer::TOKEN_INCLUDE;
		}
	};

public:
	PLexerTRB();
	virtual ~PLexerTRB();

	TBOOL Load( const TCHAR* a_pchFileName, const TCHAR* a_pchDebugInfoFileName = TNULL );
	void  Close();

	TUINT8 PeekToken( TINT a_iTokenOffset );
	TBOOL  PeekToken( Token& a_rOutToken, TINT a_iTokenOffset );
	TBOOL  GetNextToken( Token& a_rOutToken );
	TBOOL  GetPrevToken( Token& a_rOutToken );

	const TCHAR*  GetTokenName( TUINT a_uiToken ) const;
	const TCHAR** GetCustomIdents() const;

	// $Barnyard: FUNCTION 006b4670
	TUINT32 GetNumCustomIdents() const { return m_pMainHeader->uiNumCustomIdents; }

	void PrintError( Token* a_pToken, const TCHAR* a_pchError );

private:
	Toshi::TTRB     m_DataTRB;
	Toshi::TString8 m_DataFilePath;
	Toshi::TTRB*    m_pDebugDataTRB;
	TINT32*         m_pLineNums;
	TRBMainHeader*  m_pMainHeader;
	TINT            m_iTokenCounter;
	TINT            m_iStringCounter;
};
