#pragma once

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: token types of the TOSHI text lexers (TFileLexerUTF8) and of the
// precompiled token streams read by PLexerTRB (compiled FSM scripts).
//-----------------------------------------------------------------------------
class TFileLexer
{
public:
	enum TOKEN
	{
		TOKEN_EOF,         // 0
		TOKEN_SEMI,        // 1  ;
		TOKEN_COLON,       // 2  :
		TOKEN_COMMA,       // 3  ,
		TOKEN_DOT,         // 4  .
		TOKEN_ASTERISK,    // 5  * (no name in the tostring tables)
		TOKEN_DOLLAR,      // 6  $
		TOKEN_OPENSQR,     // 7  [
		TOKEN_CLOSESQR,    // 8  ]
		TOKEN_OPENBRACE,   // 9  {
		TOKEN_CLOSEBRACE,  // 10 }
		TOKEN_OPENPAREN,   // 11 (
		TOKEN_CLOSEPAREN,  // 12 )
		TOKEN_LESSTHAN,    // 13 <
		TOKEN_GREATERTHAN, // 14 >
		TOKEN_EQUAL,       // 15 =
		TOKEN_EXCLAMATION, // 16 !
		TOKEN_HYPHEN,      // 17 - (not followed by a number)
		TOKEN_IDENT,       // 18
		TOKEN_STRING,      // 19
		TOKEN_INTEGER,     // 20
		TOKEN_UINTEGER,    // 21
		TOKEN_FLOAT,       // 22
		TOKEN_COMMENT,     // 23
		TOKEN_INCLUDE,     // 24 the quoted file name of an #include
		TOKEN_NUMOF,

		// NOTE: old name of TOKEN_ASTERISK, used by the FSM tools as "unknown token"
		TOKEN_UNK = TOKEN_ASTERISK
	};

	// Name used by JPOG's exports (Toshi::TFileLexer::TokenType)
	using TokenType = TOKEN;

	static const TCHAR* GetTokenName( TOKEN a_eToken );
};

TOSHI_NAMESPACE_END
