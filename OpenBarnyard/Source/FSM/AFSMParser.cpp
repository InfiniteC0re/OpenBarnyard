#include "pch.h"
#include "AFSMParser.h"
#include "CVar/AVarType.h"

#include <Plugins/PPropertyParser/PPropertyValue.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 005fde70
AFSMParser::PPropertiesClassRefs::PPropertiesClassRefs()
{
	Setup();
}

AFSMParser::PPropertiesClassRefs::~PPropertiesClassRefs()
{
}

// $Barnyard: FUNCTION 005fddb0
void AFSMParser::PPropertiesClassRefs::Setup()
{
	T2_FOREACH_ARRAY( apPropertiesClasses, it )
	{
		apPropertiesClasses[ it ] = PPropertyValue::TYPE_UNDEF;
	}
}

//-----------------------------------------------------------------------------
// AFSMParser
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00600930
AFSMParser::AFSMParser()
{
	m_pPPropertiesClassRefs = new PPropertiesClassRefs();
	m_pFSMLexer             = new PLexerTRB();
}

// $Barnyard: FUNCTION 00602580
// $Barnyard: FUNCTION 00601e60
AFSMParser::~AFSMParser()
{
	delete m_pPPropertiesClassRefs;
	delete m_pFSMLexer;

	// $Barnyard: FUNCTION 00601e30
	m_llList1.DeleteAll();
	m_llList2.DeleteAll();
}

// $Barnyard: FUNCTION 00602a50
TBOOL AFSMParser::LoadFSMFile( AFSMManager* a_pFSMManager, TString8* a_pFSMFilePath, TString8* a_pFSMDebugFilePath, TINT a_iFSMFileIndex )
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

// $Barnyard: FUNCTION 006029d0
void AFSMParser::ParseStateMachine( AFSMManager* a_pFSMManager, TINT a_iFSMFileIndex, AFSM* a_pFSM )
{
	a_iFSMFileIndex = a_iFSMFileIndex;
	m_pFSMManager   = a_pFSMManager;
	m_pFSM          = a_pFSM;

	// Delete data from the lists
	m_llList1.DeleteAll();
	m_llList2.DeleteAll();

	// Setup everything for parsing arguments
	m_pPPropertiesClassRefs->Setup();
	a_pFSM->ResetArguments();
	
	// Reset error state
	AFSMParser::ResetError();
	m_Unk = 0;

	// Do the parse thing!
	ParseStateMachineImpl();
	if ( !m_bErrorOccured ) ResolveTransitionsImpl();

	// Delete data from the lists again
	m_llList1.DeleteAll();
	m_llList2.DeleteAll();
}

// $Barnyard: FUNCTION 006000a0
void AFSMParser::SkipTokenSafe( TFileLexer::TOKEN a_eExpect )
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

// $Barnyard: FUNCTION 00600810
void AFSMParser::SkipCustomTokenSafe( TUINT8 a_uiExpect )
{
	TUINT8 uiToken = m_pFSMLexer->PeekToken( 0 );

	// If we don't expect to see this token, raise error
	if ( ( uiToken != TFileLexer::TOKEN_IDENT && uiToken < TFileLexer::TOKEN_NUMOF ) || uiToken != a_uiExpect )
	{
		TString8 strErrorText;
		strErrorText.Format( "Expected %s", m_pFSMLexer->GetTokenName( a_uiExpect ) );

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

void AFSMParser::ParseStateMachineImpl()
{
	TIMPLEMENT();
	SkipCustomTokenSafe( 26 ); // machine
	TPString8 strMachineName = GetIdent();

	TUINT8 uiToken = m_pFSMLexer->PeekToken( 0 );
	if ( uiToken == TFileLexer::TOKEN_OPENPAREN )
	{

	}
}

void AFSMParser::ResolveTransitionsImpl()
{
}

VARTYPE_DECLARE( bool );
VARTYPE_DECLARE( int );
VARTYPE_DECLARE( float );
VARTYPE_DECLARE( string );
VARTYPE_DECLARE( unit );

// $Barnyard: FUNCTION 006016c0
const TClass* AFSMParser::ParseVariableType( TPString8& a_rName )
{
	const TClass* pVarClass = PPropertyValue::TYPE_UNDEF;

	if ( VARTYPE( bool ) == a_rName ) pVarClass = PPropertyValue::TYPE_BOOL;
	else if ( VARTYPE( int ) == a_rName ) pVarClass = PPropertyValue::TYPE_INT;
	else if ( VARTYPE( float ) == a_rName ) pVarClass = PPropertyValue::TYPE_FLOAT;
	else if ( VARTYPE( string ) == a_rName ) pVarClass = PPropertyValue::TYPE_TPSTRING8;
	else if ( VARTYPE( unit ) == a_rName ) TASSERT( TFALSE && "Not implemented!" )
	else
	{
		// Try looking for TClass with such name
		const TClass* pClass = TClass::Find( a_rName, TNULL );
		if ( pClass != TNULL ) pVarClass = pClass;
	}

	TASSERT( pVarClass != PPropertyValue::TYPE_UNDEF );
	if ( pVarClass != PPropertyValue::TYPE_UNDEF )
	{
		// Read variable name into the passed argument (this is awful)
		a_rName = GetIdent();
	}

	return pVarClass;
}
