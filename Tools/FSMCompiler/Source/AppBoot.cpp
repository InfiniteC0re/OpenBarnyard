#include "pch.h"

#include <Toshi/Toshi.h>
#include <Toshi/TPString8.h>
#include <Toshi/TArray.h>
#include <Toshi/T2String8.h>
#include <Plugins/PLexerTRB.h>
#include <Plugins/PTRB.h>
#include <ToshiTools/T2CommandLine.h>
#include <Toshi/TDebugConsole.h>
#include <Core/TMemoryInitialiser.h>

TOSHI_NAMESPACE_USING

static TMemoryInitialiser s_MemoryInitialiser;

// Keywords are stored in every compiled fsm file and their order matters
// since the token stream refers to them by index
static const TCHAR* s_apszKeywords[] = {
	"state", "machine", "if", "transition", "action", "local", "static", "default",
	"exitstart", "exitend", "common", "goto", "call", "named", "direct", "stay",
	"myunit", "t_machine", "t_state", "bool", "int", "float", "string", "unit",
	"true", "false", "null", "assert", "print", "setexception", "checkexception", "clearexception"
};

static constexpr TINT s_iNumKeywords = TINT( TARRAYSIZE( s_apszKeywords ) );

struct LexedValue
{
	TBOOL   bIsString;
	TUINT32 uiValue;
	TINT    iPoolIndex;
};

static TINT FindKeyword( const TCHAR* a_pszIdent )
{
	for ( TINT i = 0; i < s_iNumKeywords; i++ )
		if ( T2String8::Compare( a_pszIdent, s_apszKeywords[ i ] ) == 0 )
			return i;

	return -1;
}

static TINT FindOrAddPoolString( TArray<TString8>& a_rStringPool, const TCHAR* a_pszString )
{
	for ( TINT i = 0; i < a_rStringPool.Size(); i++ )
		if ( T2String8::Compare( a_rStringPool[ i ].GetString(), a_pszString ) == 0 )
			return i;

	a_rStringPool.Push( a_pszString );
	return a_rStringPool.Size() - 1;
}

static TBOOL LexFSMFile( const TCHAR* a_pszText, TArray<TUINT8>& a_rTokens, TArray<TINT32>& a_rLineNumbers, TArray<LexedValue>& a_rValues, TArray<TString8>& a_rStringPool )
{
	const TCHAR* pszCursor   = a_pszText;
	TINT         iLineNumber = 1;

	while ( *pszCursor != '\0' )
	{
		TCHAR cChar = *pszCursor;

		// Count lines and skip any whitespaces
		if ( cChar == '\n' )
		{
			iLineNumber += 1;
			pszCursor += 1;
			continue;
		}

		if ( cChar == ' ' || cChar == '\t' || cChar == '\r' )
		{
			pszCursor += 1;
			continue;
		}

		// Skip comments since the original files never store them
		if ( cChar == '/' && pszCursor[ 1 ] == '/' )
		{
			while ( *pszCursor != '\0' && *pszCursor != '\n' ) pszCursor += 1;
			continue;
		}

		TUINT8     uiToken   = TFileLexer::TOKEN_UNK;
		TBOOL      bHasValue = TFALSE;
		LexedValue oValue    = {};

		if ( cChar == '"' )
		{
			TCHAR szBuffer[ 2048 ];
			TINT  iLength = 0;

			pszCursor += 1;
			while ( *pszCursor != '"' )
			{
				if ( *pszCursor == '\0' || iLength >= TARRAYSIZE( szBuffer ) - 1 )
				{
					TERROR( "Unterminated or too long string at line %d!\n", iLineNumber );
					return TFALSE;
				}

				if ( *pszCursor == '\\' && pszCursor[ 1 ] == 'n' )
				{
					szBuffer[ iLength++ ] = '\n';
					pszCursor += 2;
					continue;
				}

				szBuffer[ iLength++ ] = *pszCursor;
				pszCursor += 1;
			}

			pszCursor += 1;
			szBuffer[ iLength ] = '\0';

			uiToken           = TFileLexer::TOKEN_STRING;
			oValue.bIsString  = TTRUE;
			oValue.iPoolIndex = FindOrAddPoolString( a_rStringPool, szBuffer );
			bHasValue         = TTRUE;
		}
		else if ( ( cChar >= '0' && cChar <= '9' ) || ( cChar == '-' && pszCursor[ 1 ] >= '0' && pszCursor[ 1 ] <= '9' ) )
		{
			// The original compiler folds the minus sign into the number literal
			const TCHAR* pszStart = pszCursor;
			TBOOL        bIsFloat = TFALSE;

			if ( cChar == '-' ) pszCursor += 1;

			while ( ( *pszCursor >= '0' && *pszCursor <= '9' ) || ( *pszCursor == '.' && !bIsFloat ) )
			{
				if ( *pszCursor == '.' ) bIsFloat = TTRUE;
				pszCursor += 1;
			}

			TString8 strNumber;
			strNumber.Copy( pszStart, TINT( pszCursor - pszStart ) );

			if ( bIsFloat )
			{
				TFLOAT flValue = T2String8::StringToFloat( strNumber.GetString() );
				oValue.uiValue = *TREINTERPRETCAST( TUINT32*, &flValue );
				uiToken        = TFileLexer::TOKEN_FLOAT;
			}
			else
			{
				oValue.uiValue = TUINT32( T2String8::StringToInt( strNumber.GetString() ) );
				uiToken        = TFileLexer::TOKEN_INTEGER;
			}

			bHasValue = TTRUE;
		}
		else if ( cChar == '_' || ( cChar >= 'a' && cChar <= 'z' ) || ( cChar >= 'A' && cChar <= 'Z' ) )
		{
			const TCHAR* pszStart = pszCursor;

			while ( *pszCursor == '_' || ( *pszCursor >= 'a' && *pszCursor <= 'z' ) || ( *pszCursor >= 'A' && *pszCursor <= 'Z' ) || ( *pszCursor >= '0' && *pszCursor <= '9' ) )
				pszCursor += 1;

			TString8 strIdent;
			strIdent.Copy( pszStart, TINT( pszCursor - pszStart ) );
			strIdent.MakeLower();

			TINT iKeyword = FindKeyword( strIdent.GetString() );

			if ( iKeyword != -1 )
			{
				uiToken = TFileLexer::TOKEN_NUMOF + iKeyword;
			}
			else
			{
				uiToken           = TFileLexer::TOKEN_IDENT;
				oValue.bIsString  = TTRUE;
				oValue.iPoolIndex = FindOrAddPoolString( a_rStringPool, strIdent.GetString() );
				bHasValue         = TTRUE;
			}
		}
		else
		{
			switch ( cChar )
			{
				case ';': uiToken = TFileLexer::TOKEN_SEMI; break;
				case ':': uiToken = TFileLexer::TOKEN_COLON; break;
				case ',': uiToken = TFileLexer::TOKEN_COMMA; break;
				case '.': uiToken = TFileLexer::TOKEN_DOT; break;
				case '$': uiToken = TFileLexer::TOKEN_DOLLAR; break;
				case '[': uiToken = TFileLexer::TOKEN_OPENSQR; break;
				case ']': uiToken = TFileLexer::TOKEN_CLOSESQR; break;
				case '{': uiToken = TFileLexer::TOKEN_OPENBRACE; break;
				case '}': uiToken = TFileLexer::TOKEN_CLOSEBRACE; break;
				case '(': uiToken = TFileLexer::TOKEN_OPENPAREN; break;
				case ')': uiToken = TFileLexer::TOKEN_CLOSEPAREN; break;
				case '<': uiToken = TFileLexer::TOKEN_LESSTHAN; break;
				case '>': uiToken = TFileLexer::TOKEN_GREATERTHAN; break;
				case '=': uiToken = TFileLexer::TOKEN_EQUAL; break;
				case '!': uiToken = TFileLexer::TOKEN_EXCLAMATION; break;
				case '-': uiToken = TFileLexer::TOKEN_HYPHEN; break;
				default:
					TERROR( "Unexpected character '%c' at line %d!\n", cChar, iLineNumber );
					return TFALSE;
			}

			pszCursor += 1;
		}

		a_rTokens.Push( uiToken );
		a_rLineNumbers.Push( iLineNumber );

		if ( bHasValue )
			a_rValues.Push( oValue );
	}

	// The original files always end with an explicit eof token
	a_rTokens.Push( TFileLexer::TOKEN_EOF );
	a_rLineNumbers.Push( iLineNumber );

	return TTRUE;
}

int main( int argc, char** argv )
{
	TUtil::TOSHIParams toshiParams;
	toshiParams.szCommandLine = GetCommandLineA();

	TUtil::ToshiCreate( toshiParams );
	TUtil::SetTPStringPool( new TPString8Pool( 1024, 0, GetGlobalAllocator(), TNULL ) );

	TDebugConsole oDebugConsole;

	T2CommandLine commandLine( GetCommandLineA() );
	auto          strInput     = commandLine.GetParameterValue( "-input" );
	auto          strOutPath   = commandLine.GetParameterValue( "-o", "." );
	auto          bIsBtec      = commandLine.HasParameter( "-btec" );
	auto          bIsDecompile = commandLine.HasParameter( "-d" );
	auto          bIsCompile   = commandLine.HasParameter( "-c" );

	// Fix slashes for the paths
	FixPathSlashes( strInput );
	FixPathSlashes( strOutPath );

	if ( bIsDecompile )
	{
		auto strDebugInfo = commandLine.GetParameterValue( "-debug-info" );

		// Make output directory from the input file if none is specified
		if ( !strOutPath )
		{
			strOutPath.Copy( strInput, strInput.FindReverse( '\\', -1 ) + 1 );
			FixPathSlashes( strOutPath );
		}

		// Read the file
		PLexerTRB oLexer;
		TBOOL     bLoaded = oLexer.Load( strInput, strDebugInfo ? strDebugInfo.GetString() : NULL );
		if ( !bLoaded )
		{
			TERROR( "Couldn't load the input file!\n" );
			return 0;
		}

		// Create output file
		TFile* pOutFile = TFile::Create( strOutPath + "\\decompiled.fsm", TFILEMODE_CREATENEW );
		if ( !pOutFile ) return 0;

		// Parse the compiled fsm file
		PLexerTRB::Token oToken;
		TINT             iPrevLine = 0;
		TINT             iNumTabs  = 0;
		while ( oLexer.GetNextToken( oToken ) )
		{
			// Add new lines
			for ( TINT i = 0; i < TMath::Min( oToken.iLineNumber - iPrevLine, 2 ); i++ )
			{
				pOutFile->CPrintf( "\n" );

				TINT iOffsetNumTabs = oToken.uiTokenType == TFileLexer::TOKEN_CLOSEBRACE ? -1 : 0;
				for ( TINT k = 0; k < iNumTabs + iOffsetNumTabs; k++ ) pOutFile->CPrintf( "\t" );
			}
			iPrevLine = oToken.iLineNumber;

			// Decompile the actual token
			switch ( oToken.uiTokenType )
			{
				case TFileLexer::TOKEN_SEMI:
					pOutFile->CPrintf( ";" );
					break;
				case TFileLexer::TOKEN_COLON:
					pOutFile->CPrintf( ":" );
					break;
				case TFileLexer::TOKEN_COMMA:
					pOutFile->CPrintf( ", " );
					break;
				case TFileLexer::TOKEN_DOT:
					pOutFile->CPrintf( "." );
					break;
				case TFileLexer::TOKEN_UNK:
					TASSERT( TFALSE );
					break;
				case TFileLexer::TOKEN_DOLLAR:
					pOutFile->CPrintf( "$" );
					break;
				case TFileLexer::TOKEN_OPENSQR:
					pOutFile->CPrintf( "[" );
					break;
				case TFileLexer::TOKEN_CLOSESQR:
					pOutFile->CPrintf( "]" );
					break;
				case TFileLexer::TOKEN_OPENBRACE:
					pOutFile->CPrintf( "{" );
					iNumTabs += 1;
					break;
				case TFileLexer::TOKEN_CLOSEBRACE:
					pOutFile->CPrintf( "}" );
					iNumTabs -= 1;
					break;
				case TFileLexer::TOKEN_OPENPAREN:
					pOutFile->CPrintf( "(" );
					break;
				case TFileLexer::TOKEN_CLOSEPAREN:
					pOutFile->CPrintf( ")" );
					break;
				case TFileLexer::TOKEN_LESSTHAN:
					pOutFile->CPrintf( " < " );
					break;
				case TFileLexer::TOKEN_GREATERTHAN:
					pOutFile->CPrintf( " > " );
					break;
				case TFileLexer::TOKEN_EQUAL:
				{
					if ( oLexer.PeekToken( -2 ) == TFileLexer::TOKEN_IDENT && oLexer.PeekToken( 0 ) != TFileLexer::TOKEN_EQUAL ) pOutFile->CPrintf( " = " );
					else pOutFile->CPrintf( "=" );
					break;
				}
				case TFileLexer::TOKEN_EXCLAMATION:
					pOutFile->CPrintf( "!" );
					break;
				case TFileLexer::TOKEN_HYPHEN:
					pOutFile->CPrintf( " - " );
					break;
				case TFileLexer::TOKEN_IDENT:
					pOutFile->CPrintf( "%s", oToken.pString );

					// Keep a space between this and the next token so they don't merge into a single identifier
					if ( oLexer.PeekToken( 0 ) >= TFileLexer::TOKEN_IDENT ) pOutFile->CPrintf( " " );
					break;
				case TFileLexer::TOKEN_STRING:
				{
					pOutFile->CPrintf( "\"" );

					TString8 strString = oToken.pString;
					for ( TINT i = 0; i < strString.Length(); i++ )
						if ( oToken.pString[ i ] != '\n' )
							pOutFile->CPrintf( "%c", oToken.pString[ i ] );
						else
							pOutFile->CPrintf( "\\n" );

					pOutFile->CPrintf( "\"" );
					break;
				}
				case TFileLexer::TOKEN_INTEGER:
					pOutFile->CPrintf( "%d", oToken.iValue );
					if ( oLexer.PeekToken( 0 ) >= TFileLexer::TOKEN_IDENT ) pOutFile->CPrintf( " " );
					break;
				case TFileLexer::TOKEN_UINTEGER:
					pOutFile->CPrintf( "%u", oToken.uiValue );
					if ( oLexer.PeekToken( 0 ) >= TFileLexer::TOKEN_IDENT ) pOutFile->CPrintf( " " );
					break;
				case TFileLexer::TOKEN_FLOAT:
					pOutFile->CPrintf( "%f", oToken.flValue );
					if ( oLexer.PeekToken( 0 ) >= TFileLexer::TOKEN_IDENT ) pOutFile->CPrintf( " " );
					break;
				case TFileLexer::TOKEN_COMMENT:
					pOutFile->CPrintf( "// ");
					break;
				case TFileLexer::TOKEN_INCLUDE:
					pOutFile->CPrintf( "#include " );
					break;
				case TFileLexer::TOKEN_EOF:
					break;
				default:
					pOutFile->CPrintf( "%s ", oLexer.GetTokenName( oToken.uiTokenType ) );
					break;
			}
		}

		pOutFile->Destroy();
	}
	else if ( bIsCompile )
	{
		// Make output directory from the input file if none is specified
		if ( !strOutPath )
		{
			strOutPath.Copy( strInput, strInput.FindReverse( '\\', -1 ) + 1 );
			FixPathSlashes( strOutPath );
		}

		// Read the file
		TFile* pTextFile = TFile::Create( strInput );
		if ( !pTextFile )
		{
			TERROR( "Couldn't load the input file!\n" );
			return 0;
		}

		auto   uiFileSize  = pTextFile->GetSize();
		TCHAR* pFileBuffer = new TCHAR[ uiFileSize + 1 ];
		pTextFile->Read( pFileBuffer, uiFileSize );
		pFileBuffer[ uiFileSize ] = '\0';
		pTextFile->Destroy();

		// Split the text into tokens
		TArray<TUINT8>     aTokens;
		TArray<TINT32>     aLineNumbers;
		TArray<LexedValue> aValues;
		TArray<TString8>   aStringPool;

		TBOOL bLexed = LexFSMFile( pFileBuffer, aTokens, aLineNumbers, aValues, aStringPool );
		delete[] pFileBuffer;

		if ( !bLexed )
			return 0;

		TINFO( "Parsed %d tokens, %d values, %d unique strings\n", aTokens.Size(), aValues.Size(), aStringPool.Size() );

		// Build the main file
		PTRB outFile;
		auto pStack = outFile.GetSections()->CreateStream();

		auto pMainHeader               = pStack->Alloc<PLexerTRB::TRBMainHeader>();
		pMainHeader->uiCount1          = TFileLexer::TOKEN_NUMOF;
		pMainHeader->uiNumCustomIdents = s_iNumKeywords;
		pMainHeader->iNumFileTokens    = aTokens.Size();

		auto pTokens = pStack->Alloc<TUINT8>( &pMainHeader->pTokens, aTokens.Size() );
		for ( TINT i = 0; i < aTokens.Size(); i++ )
			pTokens.get()[ i ] = aTokens[ i ];

		auto pCustomIdents = pStack->Alloc<TCHAR*>( (TCHAR***)&pMainHeader->ppCustomIdents, s_iNumKeywords );
		for ( TINT i = 0; i < s_iNumKeywords; i++ )
		{
			auto pKeyword = pStack->Alloc<TCHAR>( &pCustomIdents.get()[ i ], T2String8::Length( s_apszKeywords[ i ] ) + 1 );
			T2String8::Copy( pKeyword.get(), s_apszKeywords[ i ] );
		}

		auto pValues = pStack->Alloc<TUINT32>( &pMainHeader->pTokenValues, aValues.Size() );

		TArray<PTRBSections::MemoryStream::Ptr<TCHAR>> aPoolPtrs;
		for ( TINT i = 0; i < aStringPool.Size(); i++ )
		{
			auto pString = pStack->Alloc<TCHAR>( aStringPool[ i ].Length() + 1 );
			T2String8::Copy( pString.get(), aStringPool[ i ].GetString() );
			aPoolPtrs.Push( pString );
		}

		for ( TINT i = 0; i < aValues.Size(); i++ )
		{
			if ( aValues[ i ].bIsString )
				pStack->WritePointer( TREINTERPRETCAST( TCHAR**, pValues.get() + i ), aPoolPtrs[ aValues[ i ].iPoolIndex ] );
			else
				pValues.get()[ i ] = aValues[ i ].uiValue;
		}

		outFile.GetSymbols()->Add( pStack, "Main", pMainHeader.get() );

		TString8 strMainFilePath = strOutPath + "\\compiled.trb";
		outFile.WriteToFile( strMainFilePath.GetString(), bIsBtec );

		// Build the debug info file with line numbers
		PTRB outDebugFile;
		auto pDebugStack = outDebugFile.GetSections()->CreateStream();

		auto pLineNums = pDebugStack->Alloc<TINT32>( aLineNumbers.Size() );
		for ( TINT i = 0; i < aLineNumbers.Size(); i++ )
			pLineNums.get()[ i ] = aLineNumbers[ i ];

		outDebugFile.GetSymbols()->Add( pDebugStack, "LineNums", pLineNums.get() );

		TString8 strDebugFilePath = strOutPath + "\\compiledD.trb";
		outDebugFile.WriteToFile( strDebugFilePath.GetString(), bIsBtec );

		TINFO( "Saved compiled files to %s\n", strOutPath.GetString() );
	}

	return 0;
}
