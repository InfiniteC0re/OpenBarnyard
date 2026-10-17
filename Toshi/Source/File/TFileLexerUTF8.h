#pragma once
#include "TFileLexer.h"
#include "File/TFile.h"
#include "Toshi/TArray.h"
#include "Toshi/TEvent.h"
#include "Toshi/TPString8.h"
#include "Toshi/TString8.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: lexer of the TOSHI text formats (.ini/.gui PProperties files),
// reading 8-bit characters from a TFile. Supports // and /* */ comments
// (nestable), #ifdef/#ifndef/#else/#endif/#stop/#include directives at the
// start of a line, identifiers, "strings" with \n and \" escapes, decimal and
// 0x hex integers (u suffix: unsigned), floats (f suffix, exponents, 1.#INF).
//
// NOTE: this class isn't in the Barnyard PC binary. It's ported from
// Nicktoons Unite! PS2 (SLES_536.34, the primary source, field offsets in the
// comments are the PS2 ones), names and signatures from JPOG's
// TKernelInterface.dll. Nicktoons registers its TClass as "LexerT".
//-----------------------------------------------------------------------------
class TFileLexerUTF8 : public TObject
{
public:
	TDECLARE_CLASS( TFileLexerUTF8, TObject );

	using TokenType = TFileLexer::TokenType;

	//-----------------------------------------------------------------------------
	// A token. Identifiers, strings, comments and include file names hold a
	// pooled string, numbers hold their value.
	//-----------------------------------------------------------------------------
	class Token
	{
	public:
		// $JPOG: FUNCTION TKernelInterface.dll+00012230
		Token()
		    : m_eType( TFileLexer::TOKEN_EOF ), m_iLine( 0 )
		{}

		// $JPOG: FUNCTION TKernelInterface.dll+00012170
		Token( TINT a_iLine )
		    : m_eType( TFileLexer::TOKEN_EOF ), m_iLine( a_iLine )
		{}

		// $JPOG: FUNCTION TKernelInterface.dll+00012150
		Token( TINT a_iLine, TINT a_iValue )
		    : m_eType( TFileLexer::TOKEN_INTEGER ), m_iLine( a_iLine )
		{
			m_uValue.iValue = a_iValue;
		}

		// $JPOG: FUNCTION TKernelInterface.dll+00012110
		Token( TINT a_iLine, TUINT32 a_uiValue )
		    : m_eType( TFileLexer::TOKEN_UINTEGER ), m_iLine( a_iLine )
		{
			m_uValue.uiValue = a_uiValue;
		}

		// $JPOG: FUNCTION TKernelInterface.dll+00012130
		Token( TINT a_iLine, TFLOAT a_fValue )
		    : m_eType( TFileLexer::TOKEN_FLOAT ), m_iLine( a_iLine )
		{
			m_uValue.fValue = a_fValue;
		}

		// $JPOG: FUNCTION TKernelInterface.dll+000121f0
		Token( TokenType a_eType, TINT a_iLine )
		    : m_eType( a_eType ), m_iLine( a_iLine )
		{}

		// $JPOG: FUNCTION TKernelInterface.dll+00012190
		Token( TokenType a_eType, TINT a_iLine, const TPString8& a_rcString )
		    : m_eType( a_eType ), m_iLine( a_iLine )
		{
			m_uValue.pString = TNULL;
			GetStringRef()   = a_rcString;
		}

		// $JPOG: FUNCTION TKernelInterface.dll+00012210
		Token( const Token& a_rcOther )
		    : m_eType( TFileLexer::TOKEN_EOF )
		{
			assign( a_rcOther );
		}

		// $JPOG: FUNCTION TKernelInterface.dll+000120d0
		~Token()
		{
			if ( IsStringType( m_eType ) )
				GetStringRef().~TPString8();
		}

		// $JPOG: FUNCTION TKernelInterface.dll+000120b0
		Token& operator=( const Token& a_rcOther )
		{
			assign( a_rcOther );
			return *this;
		}

		void assign( const Token& a_rcOther );

		// Returns a description such as "Token[Line: 3, IDENT:name]"
		TString8 tostring() const;

		// $JPOG: FUNCTION TKernelInterface.dll+00012240
		TokenType GetType() const { return m_eType; }

		// $JPOG: FUNCTION TKernelInterface.dll+00012250
		TINT GetLine() const { return m_iLine; }

		// $JPOG: FUNCTION TKernelInterface.dll+00012260
		TFLOAT GetFloat() const { return m_uValue.fValue; }

		// $JPOG: FUNCTION TKernelInterface.dll+00012280
		TINT GetInteger() const { return m_uValue.iValue; }

		// $JPOG: FUNCTION TKernelInterface.dll+00012270
		TUINT32 GetUInteger() const { return m_uValue.uiValue; }

		// $JPOG: FUNCTION TKernelInterface.dll+00012290
		const TPString8& GetString() const { return *TREINTERPRETCAST( const TPString8*, &m_uValue.pString ); }

		// $JPOG: FUNCTION TKernelInterface.dll+000122a0
		TPString8& GetString() { return GetStringRef(); }

		// Types holding a pooled string: identifiers, strings, comments and include file names
		static TBOOL IsStringType( TokenType a_eType )
		{
			return a_eType == TFileLexer::TOKEN_IDENT || a_eType == TFileLexer::TOKEN_STRING ||
			    a_eType == TFileLexer::TOKEN_COMMENT || a_eType == TFileLexer::TOKEN_INCLUDE;
		}

	private:
		TPString8& GetStringRef() { return *TREINTERPRETCAST( TPString8*, &m_uValue.pString ); }

	private:
		TokenType m_eType; // 0x0
		TINT      m_iLine; // 0x4

		union
		{
			TINT            iValue;
			TUINT32         uiValue;
			TFLOAT          fValue;
			TPooledString8* pString; // a TPString8
		} m_uValue;                  // 0x8
	};

	//-----------------------------------------------------------------------------
	// Thrown with the parse error emitter
	//-----------------------------------------------------------------------------
	struct ParseError
	{
		TINT         m_iLine;   // 0x0
		const TCHAR* m_szError; // 0x4

		// $JPOG: FUNCTION TKernelInterface.dll+000122b0
		ParseError& operator=( const ParseError& a_rcOther )
		{
			m_iLine   = a_rcOther.m_iLine;
			m_szError = a_rcOther.m_szError;
			return *this;
		}
	};

	//-----------------------------------------------------------------------------
	// States of the nested #ifdef blocks. m_aStates[ 0 ] is the state of the file
	// itself (set by SetInputStream), the blocks start at index 1.
	// NOTE: the name is unknown. It's an older version of the TStack template
	// (the top index first, 0 means empty), its methods aren't inlined in Nicktoons.
	//-----------------------------------------------------------------------------
	struct PreprocessorStack
	{
		static constexpr TINT MAX_STATES = 32;

		// $NicktoonsPS2: FUNCTION 00395308
		void Clear() { m_iTop = 0; }

		// $NicktoonsPS2: FUNCTION 00395310
		TINT* Top() { return &m_aStates[ m_iTop ]; }

		// $NicktoonsPS2: FUNCTION 00395328
		TINT Size() const { return m_iTop; }

		// Returns the a_iIndex-th state from the top
		// $NicktoonsPS2: FUNCTION 00395330
		TINT* At( TINT a_iIndex ) { return ( m_iTop - a_iIndex < 0 ) ? &m_aStates[ 0 ] : &m_aStates[ m_iTop - a_iIndex ]; }

		// $NicktoonsPS2: FUNCTION 00395358
		void Push( const TINT& a_rcState )
		{
			m_iTop++;
			if ( m_iTop < MAX_STATES ) m_aStates[ m_iTop ] = a_rcState;
		}

		// $NicktoonsPS2: FUNCTION 00395388
		TBOOL IsEmpty() const { return m_iTop == 0; }

		// $NicktoonsPS2: FUNCTION 00395398
		TINT* Pop()
		{
			m_iTop--;
			if ( m_iTop < 0 ) m_iTop = 0;
			return &m_aStates[ m_iTop ];
		}

		TINT m_iTop;                  // 0x0
		TINT m_aStates[ MAX_STATES ]; // 0x4
	};

	using Emitter = TEmitter<TFileLexerUTF8, ParseError>;

	enum FLAGS_ : TUINT32
	{
		FLAGS_NONE              = 0,
		FLAGS_COMMENTS          = BITFLAG( 0 ), // return comments as TOKEN_COMMENT (never set in Nicktoons)
		FLAGS_ALLOWPREPROCESSOR = BITFLAG( 1 ), // every #ifdef block around the current position is enabled
		FLAGS_EOF               = BITFLAG( 2 ), // #stop was found
		FLAGS_LOWERCASE         = BITFLAG( 3 ), // identifiers are converted to lowercase
	};

public:
	TFileLexerUTF8();
	TFileLexerUTF8( TFile* a_pInputStream, TINT a_iTokenLookaheadSize );
	~TFileLexerUTF8();

	void  Define( const TCHAR* a_szDefinition );
	void  Undef( const TCHAR* a_szDefinition );
	TBOOL IfDef( const TCHAR* a_szDefinition );

	TBOOL Expect( TokenType a_eType );
	TBOOL Expect( TokenType a_eType, Token& a_rToken );

	Token GetNextToken();
	Token PeekNextToken( TINT a_iDistance );

	// $JPOG: FUNCTION TKernelInterface.dll+00011bf0
	Token GetLastToken() { return Token( m_oToken ); }

	void SetCharacterLookaheadSize( TINT a_iLookaheadSize );
	void SetInputStream( TFile* a_pInputStream );

	// NOTE: Nicktoons' lexer never reads this, it checks FLAGS_COMMENTS instead
	// $JPOG: FUNCTION TKernelInterface.dll+00012390
	void SetOutputComments( TBOOL a_bOutputComments ) { m_bOutputComments = a_bOutputComments; }

	// $JPOG: FUNCTION TKernelInterface.dll+00012380
	Emitter* GetParseErrorEmitter() { return &m_oEmitter; }

	// NOTE: not in JPOG, Nicktoons sets the bits directly (FLAGS_LOWERCASE in PPropertyReader::Load)
	TUINT32 GetFlags() const { return m_uiFlags; }
	void    SetFlags( TUINT32 a_uiFlags ) { m_uiFlags = a_uiFlags; }

	static const TCHAR* tostring( TokenType a_eType );

protected:
	TBOOL ComputePreprocessorAllow();
	void  DefinePlatformSymbols();

	Token get_next_token();
	void  skipWhiteSpace( TBOOL* a_pbInclude );
	void  fillLookAhead();

	// $JPOG: FUNCTION TKernelInterface.dll+000122d0
	void ThrowError( const TCHAR* a_szError )
	{
		ParseError oError;
		oError.m_iLine   = m_iLine;
		oError.m_szError = a_szError;
		m_oEmitter.Throw( &oError );
	}

	// $JPOG: FUNCTION TKernelInterface.dll+00012300
	void advance()
	{
		m_iCharLookaheadBack                       = ( m_iCharLookaheadBack + 1 ) & m_iCharLookaheadMask;
		m_piCharLookahead[ m_iCharLookaheadFront ] = m_pFile->GetCChar();
		m_iCharLookaheadFront                      = m_iCharLookaheadBack;
	}

	// $JPOG: FUNCTION TKernelInterface.dll+00012330
	void advance( TINT a_iDistance )
	{
		m_iCharLookaheadBack = ( m_iCharLookaheadBack + a_iDistance ) & m_iCharLookaheadMask;
		fillLookAhead();
	}

	// $JPOG: FUNCTION TKernelInterface.dll+00012350
	TINT peek() { return m_piCharLookahead[ m_iCharLookaheadBack ]; }

	// $JPOG: FUNCTION TKernelInterface.dll+00012360
	TINT peek( TINT a_iDistance ) { return m_piCharLookahead[ ( m_iCharLookaheadBack + a_iDistance ) & m_iCharLookaheadMask ]; }

	// Rounds up to the next power of two (the PS2 code uses plzcw)
	static TINT RoundUpToPowerOfTwo( TINT a_iValue )
	{
		TINT iResult = 1;
		while ( iResult < a_iValue ) iResult <<= 1;
		return iResult;
	}

private:
	TFile*            m_pFile;                   // 0x04
	TBOOL             m_bOutputComments;         // 0x08
	TINT              m_iStringMode;             // 0x0C NOTE: name unknown, never set in Nicktoons. Non-zero: no escapes in strings, 1: '\' allowed in identifiers
	TINT              m_iCharLookaheadSize;      // 0x10
	TINT              m_iCharLookaheadMask;      // 0x14
	TINT*             m_piCharLookahead;         // 0x18
	TINT              m_iCharLookaheadBack;      // 0x1C read position
	TINT              m_iCharLookaheadFront;     // 0x20 fill position
	TINT              m_iLine;                   // 0x24
	TINT              m_iTokenLookaheadSize;     // 0x28
	TINT              m_iTokenLookaheadMask;     // 0x2C
	Token             m_oToken;                  // 0x30 last returned token
	Token*            m_pLookaheadTokens;        // 0x3C
	TINT              m_iTokenLookaheadBuffered; // 0x40
	TINT              m_iTokenLookaheadFront;    // 0x44
	TINT              m_iTokenLookaheadBack;     // 0x48
	PreprocessorStack m_oPreprocessorStack;      // 0x4C
	TINT              m_aUnknown[ 2 ];           // 0xD0 never accessed
	TArray<TPString8> m_aDefines;                // 0xD8 sorted by the pooled string pointers
	TUINT32           m_uiFlags;                 // 0xE8 FLAGS_
	Emitter           m_oEmitter;                // 0xEC
};

TOSHI_NAMESPACE_END
