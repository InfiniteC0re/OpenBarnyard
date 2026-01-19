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
		TUINT        uiTokenType;
		TINT         iLineNumber;

		union
		{
			TUINT32      uiValue;
			TINT32       iValue;
			TFLOAT       flValue;
			const TCHAR* pString;
		};

		const TCHAR* GetIdent( const TCHAR** a_ppCustomTokens );
	};

public:
	PLexerTRB();
	virtual ~PLexerTRB();

	TBOOL Load( const TCHAR* a_pchFileName, const TCHAR* a_pchDebugInfoFileName = TNULL );
	void  Close();

	TUINT8 PeekToken( TINT a_iTokenOffset );
	TBOOL  GetNextToken( Token& a_rOutToken );
	TBOOL  GetPrevToken( Token& a_rOutToken );

	const TCHAR*  GetTokenName( TUINT a_uiToken ) const;
	const TCHAR** GetCustomIdents() const;

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
