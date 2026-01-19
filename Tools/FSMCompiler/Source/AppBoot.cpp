#include "pch.h"

#include <Toshi/Toshi.h>
#include <Toshi/TPString8.h>
#include <Plugins/PLexerTRB.h>
#include <ToshiTools/T2CommandLine.h>
#include <Toshi/TDebugConsole.h>
#include <Core/TMemoryInitialiser.h>

TOSHI_NAMESPACE_USING

static TMemoryInitialiser s_MemoryInitialiser;

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
					break;
				case TFileLexer::TOKEN_UINTEGER:
					pOutFile->CPrintf( "%u", oToken.uiValue );
					break;
				case TFileLexer::TOKEN_FLOAT:
					pOutFile->CPrintf( "%f", oToken.flValue );
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

	return 0;
}
