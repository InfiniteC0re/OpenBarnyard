#include "ToshiPCH.h"
#include "TFileLexer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

const TCHAR* TFileLexer::GetTokenName( TOKEN a_eToken )
{
	switch ( a_eToken )
	{
		case TOKEN_EOF: return "EOF";
		case TOKEN_SEMI: return "SEMI";
		case TOKEN_COLON: return "COLON";
		case TOKEN_COMMA: return "COMMA";
		case TOKEN_DOT: return "DOT";
		case TOKEN_DOLLAR: return "DOLLAR";
		case TOKEN_OPENSQR: return "OPENSQR";
		case TOKEN_CLOSESQR: return "CLOSESQR";
		case TOKEN_OPENBRACE: return "OPENBRACE";
		case TOKEN_CLOSEBRACE: return "CLOSEBRACE";
		case TOKEN_OPENPAREN: return "OPENPAREN";
		case TOKEN_CLOSEPAREN: return "CLOSEPAREN";
		case TOKEN_LESSTHAN: return "LESSTHAN";
		case TOKEN_GREATERTHAN: return "GREATERTHAN";
		case TOKEN_EQUAL: return "EQUAL";
		case TOKEN_EXCLAMATION: return "EXCLAMATION";
		case TOKEN_HYPHEN: return "HYPHEN";
		case TOKEN_IDENT: return "IDENT";
		case TOKEN_STRING: return "STRING";
		case TOKEN_INTEGER: return "INTEGER";
		case TOKEN_UINTEGER: return "UINTEGER";
		case TOKEN_FLOAT: return "FLOAT";
		case TOKEN_COMMENT: return "COMMENT";
		case TOKEN_INCLUDE: return "INCLUDE";
		default:
		{
			TASSERT( TFALSE );
			return "?????";
		}
	}
}

TOSHI_NAMESPACE_END
