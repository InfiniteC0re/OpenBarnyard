#include "pch.h"
#include "AFSMParser.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

static TClass* s_pInvalidClass = TNULL;

// $Barnyard: FUNCTION 005fde70
AFSMParser::ClassRefs::ClassRefs()
{
	T2_FOREACH_ARRAY( apClasses, it )
		apClasses[ it ] = s_pInvalidClass;
}

AFSMParser::ClassRefs::~ClassRefs()
{
}

//-----------------------------------------------------------------------------
// AFSMParser
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00600930
AFSMParser::AFSMParser()
{
	m_pClassRefs = new ClassRefs();
	m_pFSMLexer  = new PLexerTRB();
}

// $Barnyard: FUNCTION 00602580
// $Barnyard: FUNCTION 00601e60
AFSMParser::~AFSMParser()
{
	delete m_pClassRefs;
	delete m_pFSMLexer;
	
	// $Barnyard: FUNCTION 00601e30
	m_llList1.DeleteAll();
	m_llList2.DeleteAll();
}

// $Barnyard: FUNCTION 00602a50
TBOOL AFSMParser::Read( AFSMManager* a_pFSMManager, TString8* a_pFSMFilePath, TString8* a_pFSMDebugFilePath, TINT a_iFSMFileIndex )
{
	TVALIDPTR( a_pFSMFilePath );

	if ( m_pFSMLexer->Load( *a_pFSMFilePath, a_pFSMDebugFilePath ? *a_pFSMDebugFilePath : TNULL ) )
	{
		// Parse all tokens
		TUINT8 uiToken = m_pFSMLexer->PeekToken( 0 );

		while ( uiToken != TFileLexer::TOKEN_EOF )
		{
			ResetError();

			// Read beginning of the state machine declaration (I guess it would be better to call it a layer?)
			TPString8 strFSMName = GetIdent();
			SkipTokenSafe( TFileLexer::TOKEN_OPENBRACE );

			AFSM* pFSM = a_pFSMManager->CreateStateMachine( a_iFSMFileIndex, strFSMName );
			if ( !pFSM )
			{
				TASSERT( TFALSE );
				SetError();
				break;
			}

			ParseStateMachine( a_pFSMManager, a_iFSMFileIndex, pFSM );
			SkipTokenSafe( TFileLexer::TOKEN_CLOSEBRACE );

			if ( m_bErrorOccured )
			{
				a_pFSMManager->DestroyStateMachine( pFSM );
				break;
			}

			// Get next token
			uiToken = m_pFSMLexer->PeekToken( 0 );
		}

		m_pFSMLexer->Close();
	}

	return m_bErrorOccured == TFALSE;
}

// $Barnyard: FUNCTION 006009f0
TPString8 AFSMParser::GetIdent()
{
	TUINT8 uiToken = m_pFSMLexer->PeekToken( 0 );

	// Make sure the token is string or a custom token
	if ( uiToken != TFileLexer::TOKEN_IDENT && uiToken < TFileLexer::TOKEN_NUMOF )
	{
		TString8 strErrorText;
		strErrorText.Format( "Expected a string indentifier" );

		PLexerTRB::Token oPrevToken;
		m_pFSMLexer->GetPrevToken( oPrevToken );
		m_pFSMLexer->PrintError( &oPrevToken, strErrorText );
		SetError();

		return TPString8();
	}

	PLexerTRB::Token oToken;
	m_pFSMLexer->GetNextToken( oToken );

	return TPString8( oToken.GetIdent( m_pFSMLexer->GetCustomIdents() ) );
}

void AFSMParser::ParseStateMachine( AFSMManager* a_pFSMManager, TINT a_iFSMFileIndex, AFSM* a_pFSM )
{
	TIMPLEMENT();
}

// $Barnyard: FUNCTION 006000a0
void AFSMParser::SkipTokenSafe( Toshi::TFileLexer::TOKEN a_eExpect )
{
	TUINT8 uiToken = m_pFSMLexer->PeekToken( 0 );
	
	// If we don't expect to see this token, raise error
	if ( uiToken != a_eExpect )
	{
		TString8 strErrorText;
		strErrorText.Format( "Expected %s", m_pFSMLexer->GetTokenName( a_eExpect ) );

		PLexerTRB::Token oPrevToken;
		m_pFSMLexer->GetPrevToken( oPrevToken );
		m_pFSMLexer->PrintError( &oPrevToken, strErrorText );

		SetError();
		return;
	}

	// No errors, go to the text token
	PLexerTRB::Token oToken;
	m_pFSMLexer->GetNextToken( oToken );
}
