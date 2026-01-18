#pragma once
#include "TFileLexer.h"
#include "File/TFile.h"
#include "Toshi/TArray.h"

TOSHI_NAMESPACE_START

class TFileLexerUTF8 : public TObject
{
public:
	TDECLARE_CLASS( TFileLexerUTF8, TObject );

public:
	struct Token
	{
	public:
		void assign( const Token& token );

	public:
		TFileLexer::TOKEN m_type    = TFileLexer::TOKEN_EOF;
		TINT              m_unknown = 0;
		TString8          m_value;
	};

	struct LookaheadTokens
	{
	public:
		Token* GetTokens()
		{
			return TREINTERPRETCAST( Token*, this + 1 );
		}

		static LookaheadTokens* Allocate( TINT a_iCount = 1 )
		{
			return TREINTERPRETCAST(
			    LookaheadTokens*,
			    TMalloc( sizeof( LookaheadTokens ) + sizeof( Token ) * a_iCount )
			);
		}

		static void Free( Token* a_pHeadToken )
		{
			TFree( FromToken( a_pHeadToken ) );
		}

		static LookaheadTokens* FromToken( Token* a_pHeadToken )
		{
			return TREINTERPRETCAST(
			    LookaheadTokens*,
			    TREINTERPRETCAST( uintptr_t, a_pHeadToken ) - sizeof( LookaheadTokens )
			);
		}

	public:
		TINT m_iCount;
	};

public:
	TFileLexerUTF8();
	TFileLexerUTF8( TFile* a_pInputStream, TINT a_iTokenLookaheadSize );
	~TFileLexerUTF8();

	TBOOL ComputePreprocessorAllow();
	void  SetCharacterLookaheadSize( TINT a_iLookaheadSize );
	void  SetInputStream( TFile* a_pInputStream );
	void  Define( const TCHAR* a_szDefinition );

private:
	TFile*           m_pFile;                   // 0x04
	TINT             m_bUnk1;                   // 0x08
	TINT             m_iCharLookaheadSize;      // 0x0C
	TINT             m_iUnk3;                   // 0x10
	TINT*            m_piCharLookahead;         // 0x14
	TINT             m_iUnk4;                   // 0x18
	TINT             m_iUnk5;                   // 0x1C
	TINT             m_iUnk6;                   // 0x20
	TINT             m_iTokenLookaheadSize;     // 0x24
	TINT             m_iTokenLookaheadMask;     // 0x28
	Token            m_Token;                   // 0x2C
	Token*           m_LookaheadTokens;         // 0x38
	TINT             m_iTokenLookaheadBuffered; // 0x3C
	TINT             m_iTokenLookaheadFront;    // 0x40
	TINT             m_iTokenLookaheadBack;     // 0x44
	TINT             m_iSomeNum;                // 0x48
	TBOOL            m_bFlags[ 4 ];             // 0x4C
	TBOOL            m_bAllowPreprocessor;      // 0x6C
	TBOOL            m_bUnk7;                   // 0x6D
	TArray<TString8> m_Defines;                 // 0x70
	TGenericEmitter  m_oEmitter;                // 0x80
};

TOSHI_NAMESPACE_END
