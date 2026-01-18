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
		TUINT32       uiNumTokens;
		TUINT8*       pTokenData;
		const TCHAR** ppStrings;
		TINT32        iNumTokens;
		const TCHAR** ppCustomTokens;
	};

	struct Token
	{
		TUINT        uiToken;
		TINT         iLineNumber;
		const TCHAR* pString;
	};

public:
	PLexerTRB();
	virtual ~PLexerTRB();

	TBOOL Load( const TCHAR* a_pchFileName, const TCHAR* a_pchDebugInfoFileName );
	void  Close();

	TUINT8 PeekToken( TINT a_iTokenOffset );
	TBOOL  GetNextToken( Token& a_rOutToken );
	TBOOL  GetPrevToken( Token& a_rOutToken );

	const TCHAR*  GetTokenName( TUINT a_uiToken ) const;
	const TCHAR** GetCustomTokens() const;

private:
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
