#include "pch.h"
#include "AFSMParser.h"
#include "CVar/AVarType.h"
#include "Units/AUnit.h"

#include <Plugins/PPropertyParser/PPropertyValue.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

VARTYPE_DECLARE( bool );
VARTYPE_DECLARE( int );
VARTYPE_DECLARE( float );
VARTYPE_DECLARE( string );
VARTYPE_DECLARE( unit );

// $Barnyard: FUNCTION 00600930
AFSMParser::AFSMParser()
    : m_pFSM( TNULL )
    , m_pState( TNULL )
    , m_pFSMManager( TNULL )
    , m_iFSMFileIndex( 0 )
    , m_bErrorOccured( TFALSE )
{
	m_pLocalTypes = new AFSMLocalTypes();
	m_pFSMLexer   = new PLexerTRB();
}

// $Barnyard: FUNCTION 00602580
// $Barnyard: FUNCTION 00601e60
AFSMParser::~AFSMParser()
{
	delete m_pLocalTypes;
	delete m_pFSMLexer;

	// $Barnyard: FUNCTION 00601e30
	m_llStates.DeleteAll();
	m_llTransitions.DeleteAll();
}

// $Barnyard: FUNCTION 00602a50
TBOOL AFSMParser::LoadFSMFile( AFSMManager* a_pFSMManager, TString8* a_pFSMFilePath, TString8* a_pFSMDebugFilePath, TINT a_iFSMFileIndex )
{
	TVALIDPTR( a_pFSMFilePath );

	if ( m_pFSMLexer->Load( a_pFSMFilePath->GetString(), a_pFSMDebugFilePath ? a_pFSMDebugFilePath->GetString() : TNULL ) == TTRUE )
	{
		if ( m_pFSMLexer->PeekToken( 0 ) != TFileLexer::TOKEN_EOF )
		{
			while ( TTRUE )
			{
				ResetError();

				TPString8 strFSMName = GetIdent();
				SkipTokenSafe( TFileLexer::TOKEN_OPENBRACE );

				AFSM* pFSM = a_pFSMManager->CreateStateMachine( a_iFSMFileIndex, strFSMName );

				if ( !pFSM )
				{
					m_bErrorOccured = TTRUE;
					break;
				}

				ParseStateMachine( a_pFSMManager, a_iFSMFileIndex, pFSM );
				SkipTokenSafe( TFileLexer::TOKEN_CLOSEBRACE );

				if ( m_bErrorOccured )
				{
					// NOTE: the machines after the one that failed aren't loaded
					TERROR( "AFSMParser: failed to parse FSM '%s', the remaining FSMs aren't loaded\n", strFSMName.GetString() );
					a_pFSMManager->DestroyStateMachine( pFSM );
					break;
				}

				if ( m_pFSMLexer->PeekToken( 0 ) == TFileLexer::TOKEN_EOF )
					break;
			}
		}

		m_pFSMLexer->Close();
	}

	return m_bErrorOccured == TFALSE;
}

// $Barnyard: FUNCTION 006029d0
void AFSMParser::ParseStateMachine( AFSMManager* a_pFSMManager, TINT a_iFSMFileIndex, AFSM* a_pFSM )
{
	m_iFSMFileIndex = a_iFSMFileIndex;
	m_pFSMManager   = a_pFSMManager;
	m_pFSM          = a_pFSM;

	m_llStates.DeleteAll();
	m_llTransitions.DeleteAll();

	m_pLocalTypes->Reset();
	ResetError();
	m_pFSM->ResetArguments();
	m_pState = TNULL;

	ParseMachine();

	if ( !m_bErrorOccured )
		ResolveTransitions();

	m_llStates.DeleteAll();
	m_llTransitions.DeleteAll();
}

// $Barnyard: FUNCTION 006009f0
TPString8 AFSMParser::GetIdent()
{
	if ( !IsIdentOrKeyword() )
	{
		TString8 strErrorText;
		strErrorText.Format( "Expected a string indentifier" );

		PrintError( strErrorText );
		return TPString8();
	}

	PLexerTRB::Token oToken;
	m_pFSMLexer->GetNextToken( oToken );

	return TPString8( oToken.GetIdent( m_pFSMLexer->GetCustomIdents() ) );
}

TBOOL AFSMParser::IsKeyword( TUINT8 a_uiKeyword )
{
	return IsIdentOrKeyword() && m_pFSMLexer->PeekToken( 0 ) == a_uiKeyword;
}

// $Barnyard: FUNCTION 00600050
TBOOL AFSMParser::SkipKeyword( TUINT8 a_uiKeyword )
{
	if ( !IsKeyword( a_uiKeyword ) )
		return TFALSE;

	PLexerTRB::Token oToken;
	m_pFSMLexer->GetNextToken( oToken );
	return TTRUE;
}

// $Barnyard: FUNCTION 005fffd0
TBOOL AFSMParser::IsIdentOrKeyword()
{
	TUINT8 uiToken = m_pFSMLexer->PeekToken( 0 );
	return uiToken == TFileLexer::TOKEN_IDENT || uiToken >= TFileLexer::TOKEN_NUMOF;
}

// "class::function"
// $Barnyard: FUNCTION 005fff60
TBOOL AFSMParser::IsAction()
{
	return m_pFSMLexer->PeekToken( 0 ) == TFileLexer::TOKEN_IDENT && m_pFSMLexer->PeekToken( 1 ) == TFileLexer::TOKEN_COLON &&
	    m_pFSMLexer->PeekToken( 2 ) == TFileLexer::TOKEN_COLON && m_pFSMLexer->PeekToken( 3 ) == TFileLexer::TOKEN_IDENT;
}

// "value->class"
// $Barnyard: FUNCTION 005fff10
TBOOL AFSMParser::IsTargetAction()
{
	return m_pFSMLexer->PeekToken( 0 ) == TFileLexer::TOKEN_IDENT && m_pFSMLexer->PeekToken( 1 ) == TFileLexer::TOKEN_HYPHEN &&
	    m_pFSMLexer->PeekToken( 2 ) == TFileLexer::TOKEN_GREATERTHAN && m_pFSMLexer->PeekToken( 3 ) == TFileLexer::TOKEN_IDENT;
}

// $Barnyard: FUNCTION 006000a0
void AFSMParser::SkipTokenSafe( TUINT8 a_uiExpect )
{
	if ( m_pFSMLexer->PeekToken( 0 ) != a_uiExpect )
	{
		TString8 strErrorText;
		strErrorText.Format( "Expected %s", m_pFSMLexer->GetTokenName( a_uiExpect ) );

		PrintError( strErrorText );
		return;
	}

	PLexerTRB::Token oToken;
	m_pFSMLexer->GetNextToken( oToken );
}

// $Barnyard: FUNCTION 00600810
void AFSMParser::SkipCustomTokenSafe( TUINT8 a_uiExpect )
{
	if ( !IsKeyword( a_uiExpect ) )
	{
		TString8 strErrorText;
		strErrorText.Format( "Expected %s", m_pFSMLexer->GetTokenName( a_uiExpect ) );

		PrintError( strErrorText );
		return;
	}

	PLexerTRB::Token oToken;
	m_pFSMLexer->GetNextToken( oToken );
}

// $Barnyard: FUNCTION 00600020
void AFSMParser::PrintError( const TCHAR* a_pchErrorText )
{
	PLexerTRB::Token oPrevToken;
	m_pFSMLexer->GetPrevToken( oPrevToken );
	m_pFSMLexer->PrintError( &oPrevToken, a_pchErrorText );

	m_bErrorOccured = TTRUE;
}

// $Barnyard: FUNCTION 005ffff0
void AFSMParser::PrintWarning( const TCHAR* a_pchWarningText )
{
	PLexerTRB::Token oPrevToken;
	m_pFSMLexer->GetPrevToken( oPrevToken );

	// NOTE: the message is compiled out in retail
	TWARN( "AFSMParser: (%d) : warning : %s\n", oPrevToken.iLineNumber, a_pchWarningText );
}

// $Barnyard: FUNCTION 006002d0
AFSMParser::OPERATOR AFSMParser::GetOperator( TBOOL a_bSkip )
{
	TINT i = 0;

	for ( ; i < AFSMInstructionComparison::OPERATOR_NUMOF; i++ )
	{
		if ( OPERATORS[ i ].eToken1 == m_pFSMLexer->PeekToken( 0 ) &&
		     ( OPERATORS[ i ].eToken2 == 0 || OPERATORS[ i ].eToken2 == m_pFSMLexer->PeekToken( 1 ) ) )
		{
			if ( a_bSkip )
			{
				SkipTokenSafe( OPERATORS[ i ].eToken1 );

				if ( OPERATORS[ i ].eToken2 != 0 )
					SkipTokenSafe( OPERATORS[ i ].eToken2 );
			}

			break;
		}
	}

	return TCAST( OPERATOR, i );
}

// $Barnyard: FUNCTION 00602880
void AFSMParser::ParseMachine()
{
	SkipCustomTokenSafe( KEYWORD_MACHINE );
	TPString8 strMachineName = GetIdent();

	// Arguments of the machine are its first variables
	if ( m_pFSMLexer->PeekToken( 0 ) == TFileLexer::TOKEN_OPENPAREN )
	{
		PLexerTRB::Token oToken;
		m_pFSMLexer->GetNextToken( oToken );

		while ( TTRUE )
		{
			ParseVariable( TFALSE );
			m_pFSM->AddArgument();

			if ( m_pFSMLexer->PeekToken( 0 ) == TFileLexer::TOKEN_CLOSEPAREN )
				break;

			SkipTokenSafe( TFileLexer::TOKEN_COMMA );
		}

		m_pFSMLexer->GetNextToken( oToken );
	}

	SkipTokenSafe( TFileLexer::TOKEN_OPENBRACE );

	while ( m_pFSMLexer->PeekToken( 0 ) != TFileLexer::TOKEN_CLOSEBRACE )
	{
		ParseState();

		if ( m_bErrorOccured )
			return;
	}

	PLexerTRB::Token oToken;
	m_pFSMLexer->GetNextToken( oToken );

	if ( !m_pFSM->HasStates() )
	{
		PrintError( "Machine Contains No States" );
	}
	else if ( ( m_pFSM->GetExitEndState() && !m_pFSM->GetExitStartState() ) || ( m_pFSM->GetExitStartState() && !m_pFSM->GetExitEndState() ) )
	{
		PrintError( "Invalid Exit States - Missing Start or End Exit State" );
	}
}

// $Barnyard: FUNCTION 006025a0
void AFSMParser::ParseState()
{
	m_pLocalTypes->Reset();

	if ( IsKeyword( KEYWORD_STATIC ) )
	{
		PLexerTRB::Token oToken;
		m_pFSMLexer->GetNextToken( oToken );

		ParseVariable( TFALSE );

		if ( m_pFSMLexer->PeekToken( 0 ) == TFileLexer::TOKEN_SEMI )
			PrintError( "Unknown symbol ';' found - do not end static declarations with a semicolon!" );

		return;
	}

	NameLink* pLink   = new NameLink();
	TBOOL     bCommon = TFALSE;

	m_pState     = new AFSMState();
	pLink->pData = m_pState;
	m_llStates.InsertTail( pLink );

	if ( IsKeyword( KEYWORD_COMMON ) )
	{
		PLexerTRB::Token oToken;
		m_pFSMLexer->GetNextToken( oToken );

		m_pFSM->SetCommonState( m_pState );
		bCommon = TTRUE;
	}
	else
	{
		TBOOL bDefault   = TFALSE;
		TBOOL bExitStart = TFALSE;
		TBOOL bExitEnd   = TFALSE;

		// The modifiers can come in any order
		while ( TTRUE )
		{
			PLexerTRB::Token oToken;

			if ( IsKeyword( KEYWORD_DEFAULT ) )
			{
				m_pFSMLexer->GetNextToken( oToken );
				bDefault = TTRUE;
			}
			else if ( IsKeyword( KEYWORD_EXITSTART ) )
			{
				m_pFSMLexer->GetNextToken( oToken );
				bExitStart = TTRUE;
			}
			else if ( IsKeyword( KEYWORD_EXITEND ) )
			{
				m_pFSMLexer->GetNextToken( oToken );
				bExitEnd = TTRUE;
			}
			else
			{
				break;
			}
		}

		m_pFSM->AddState( m_pState, bDefault, bExitStart, bExitEnd );
		SkipCustomTokenSafe( KEYWORD_STATE );

		TPString8 strName = GetIdent();
		pLink->strName    = strName;
		m_pState->SetName( strName );
	}

	SkipTokenSafe( TFileLexer::TOKEN_OPENBRACE );

	while ( m_pFSMLexer->PeekToken( 0 ) != TFileLexer::TOKEN_CLOSEBRACE )
	{
		// The common state can't have transitions
		AFSMInstruction* pInstruction = ParseInstruction( TFALSE, !bCommon, TTRUE );

		if ( m_bErrorOccured )
			return;

		if ( pInstruction )
			m_pState->AddInstruction( pInstruction );
	}

	PLexerTRB::Token oToken;
	m_pFSMLexer->GetNextToken( oToken );
}

// $Barnyard: FUNCTION 006006b0
void AFSMParser::ResolveTransitions()
{
	T2_FOREACH( m_llTransitions, itTransition )
	{
		TBOOL bResolved = TFALSE;

		T2_FOREACH( m_llStates, itState )
		{
			if ( TStringManager::String8CompareNoCase( itState->strName.GetString(), itTransition->strName.GetString(), -1 ) == 0 )
			{
				TSTATICCAST( AFSMInstructionTransition, itTransition->pData )->SetTargetState( TSTATICCAST( AFSMState, itState->pData ) );
				bResolved = TTRUE;
				break;
			}
		}

		if ( !bResolved )
		{
			TString8 strError;
			strError.Format( "Unresolved Transition: %s", itTransition->strName.GetString() );

			PrintError( strError );
		}
	}
}

// $Barnyard: FUNCTION 00601ee0
AFSMInstruction* AFSMParser::ParseInstruction( TBOOL a_bValue, TBOOL a_bAllowTransition, TBOOL a_bAllowTarget )
{
	AFSMInstruction* pInstruction = TNULL;
	TBOOL            bBracket     = TFALSE;

	if ( m_pFSMLexer->PeekToken( 0 ) == TFileLexer::TOKEN_OPENPAREN )
	{
		PLexerTRB::Token oToken;
		m_pFSMLexer->GetNextToken( oToken );
		bBracket = TTRUE;

		if ( !a_bValue )
		{
			PrintError( "Found start of bracketd expression, but values not allowed here" );
			return TNULL;
		}
	}

	TBOOL bValidate = TTRUE;

	if ( IsKeyword( KEYWORD_SETEXCEPTION ) )
	{
		PLexerTRB::Token oToken;
		m_pFSMLexer->GetNextToken( oToken );
		pInstruction = ParseSetException();
	}
	else if ( SkipKeyword( KEYWORD_CHECKEXCEPTION ) )
	{
		pInstruction = ParseCheckException();
	}
	else if ( SkipKeyword( KEYWORD_CLEAREXCEPTION ) )
	{
		pInstruction = ParseClearException();
	}
	else if ( SkipKeyword( KEYWORD_ASSERT ) )
	{
		pInstruction = ParseAssert();
	}
	else if ( SkipKeyword( KEYWORD_PRINT ) )
	{
		// NOTE: nothing is parsed for print
		bValidate = TFALSE;
	}
	else if ( a_bAllowTarget && IsTargetAction() )
	{
		pInstruction = ParseTargetAction();
	}
	else if ( SkipKeyword( KEYWORD_ACTION ) || IsAction() )
	{
		pInstruction = ParseAction();
	}
	else if ( SkipKeyword( KEYWORD_TRANSITION ) )
	{
		if ( !a_bAllowTransition )
			PrintWarning( "'transition' may not be handled correctly in this context" );

		pInstruction = ParseTransition();
	}
	else if ( SkipKeyword( KEYWORD_IF ) )
	{
		pInstruction = ParseIfBlock();
	}
	else if ( SkipKeyword( KEYWORD_MYUNIT ) )
	{
		if ( !a_bValue )
		{
			PrintError( "Cannot have 'myunit' keyword by itself, expected instruction" );
			bValidate = TFALSE;
		}
		else
		{
			pInstruction = new AFSMInstructionValueCurrentUnit();
		}
	}
	else if ( SkipKeyword( KEYWORD_T_MACHINE ) )
	{
		if ( !a_bValue )
		{
			PrintError( "Cannot have 't_machine' keyword by itself, expected instruction" );
			bValidate = TFALSE;
		}
		else
		{
			pInstruction = new AFSMInstructionValueMachineTime();
		}
	}
	else if ( SkipKeyword( KEYWORD_T_STATE ) )
	{
		if ( !a_bValue )
		{
			PrintError( "Cannot have 't_state' keyword by itself, expected instruction" );
			bValidate = TFALSE;
		}
		else
		{
			pInstruction = new AFSMInstructionValueStateTime();
		}
	}
	else if ( SkipKeyword( KEYWORD_LOCAL ) )
	{
		pInstruction = ParseLocalDeclaration();
	}
	else if ( SkipKeyword( KEYWORD_STATIC ) )
	{
		PrintError( "Static declaration not allowed in states" );
		bValidate = TFALSE;
	}
	else if ( SkipKeyword( KEYWORD_STAY ) )
	{
		pInstruction = new AFSMInstructionStay();
	}
	else if ( IsIdentOrKeyword() )
	{
		PLexerTRB::Token oToken;
		m_pFSMLexer->GetNextToken( oToken );

		if ( oToken.uiTokenType == KEYWORD_TRUE || oToken.uiTokenType == KEYWORD_FALSE || oToken.uiTokenType == KEYWORD_NULL )
		{
			if ( !a_bValue )
			{
				if ( oToken.uiTokenType == KEYWORD_TRUE )
					PrintError( "Value not allowed here (got 'true')" );
				else if ( oToken.uiTokenType == KEYWORD_FALSE )
					PrintError( "Value not allowed here (got 'false')" );
				else
					PrintError( "Value not allowed here (got 'null')" );

				bValidate = TFALSE;
			}
			else
			{
				PPropertyValue oValue;

				if ( oToken.uiTokenType == KEYWORD_TRUE )
					oValue.SetBoolean( TTRUE );
				else if ( oToken.uiTokenType == KEYWORD_FALSE )
					oValue.SetBoolean( TFALSE );
				else
					oValue.SetTObject( TNULL );

				pInstruction = new AFSMInstructionValueConstant( oValue );
			}
		}
		else
		{
			// A local or a variable of the machine
			const TCHAR* pchIdent = oToken.GetIdent( m_pFSMLexer->GetCustomIdents() );
			TPString8    strName( pchIdent );

			TINT iLocalIdx = m_pState->FindLocalIndex( strName );

			if ( iLocalIdx == -1 )
			{
				TINT iVarIdx = m_pFSM->FindVariableNameIndex( strName );

				if ( iVarIdx == -1 )
				{
					TString8 strError;

					if ( m_pFSMLexer->PeekToken( 0 ) == TFileLexer::TOKEN_EQUAL )
						strError.Format( "Have form of 'local = ', but token '%s' is not a valid local", pchIdent );
					else
						strError.Format( "Unknown instruction '%s' found", pchIdent );

					PrintError( strError );
				}
				else
				{
					pInstruction = ParseGlobal( a_bValue, iVarIdx );
				}
			}
			else
			{
				pInstruction = ParseLocal( a_bValue, iLocalIdx );
			}
		}
	}
	else if ( a_bValue )
	{
		pInstruction = ParseLiteral( TTRUE );
	}
	else
	{
		// $Barnyard: FUNCTION 005fffb0
		if ( m_pFSMLexer->PeekToken( 0 ) == TFileLexer::TOKEN_SEMI )
			PrintError( "Unknown symbol ';' found - do not end statements with a semicolon!" );
		else
			PrintError( "Unknown symbol found, expecting instruction" );

		bValidate = TFALSE;
	}

	if ( bValidate && pInstruction )
	{
		if ( a_bValue )
			pInstruction = ParseComparison( pInstruction, TTRUE );

		// NOTE: retail doesn't check the comparison could be parsed
		if ( pInstruction && !pInstruction->Validate( m_pFSM, m_pLocalTypes ) )
		{
			PLexerTRB::Token oPrevToken;
			m_pFSMLexer->GetPrevToken( oPrevToken );
			m_pFSMLexer->PrintError( &oPrevToken, "Instruction was not valid" );
			m_bErrorOccured = TTRUE;

			delete pInstruction;
			pInstruction = TNULL;
		}
	}

	if ( bBracket )
		SkipTokenSafe( TFileLexer::TOKEN_CLOSEPAREN );

	return pInstruction;
}

// $Barnyard: FUNCTION 00600350
AFSMInstruction* AFSMParser::ParseComparison( AFSMInstruction* a_pLeft, TBOOL a_bOptional )
{
	OPERATOR eOperator = GetOperator( TTRUE );

	if ( eOperator == AFSMInstructionComparison::OPERATOR_NONE )
	{
		if ( !a_bOptional )
			PrintError( "Expected a comparison" );

		return a_pLeft;
	}

	AFSMInstruction* pRight = ParseInstruction( TTRUE, TTRUE, TTRUE );

	if ( m_bErrorOccured || !pRight )
	{
		PrintError( "Transition RHS of comparison must be a valid expression" );

		if ( a_pLeft )
			delete a_pLeft;

		return TNULL;
	}

	AFSMInstructionComparison* pComparison = new AFSMInstructionComparison();
	pComparison->SetLeft( a_pLeft );
	pComparison->SetRight( pRight );
	pComparison->SetOperator( eOperator );

	return pComparison;
}

// $Barnyard: FUNCTION 00600e60
AFSMInstruction* AFSMParser::ParseAction()
{
	AFSMInstructionAction* pAction = new AFSMInstructionAction();

	TPString8 strClassName = GetIdent();
	SkipTokenSafe( TFileLexer::TOKEN_COLON );
	SkipTokenSafe( TFileLexer::TOKEN_COLON );
	TPString8 strFunctionName = GetIdent();

	if ( !pAction->GetFunction().Find( strClassName, strFunctionName ) )
	{
		TString8 strError;
		strError.Format( "Unresolved function %s::%s", strClassName.GetString(), strFunctionName.GetString() );
		PrintError( strError );

		// NOTE: retail doesn't delete the action here
		return TNULL;
	}

	SkipTokenSafe( TFileLexer::TOKEN_OPENPAREN );

	AFSMInstruction* apArguments[ 8 ];
	TINT             iNumArguments = 0;

	if ( m_pFSMLexer->PeekToken( 0 ) != TFileLexer::TOKEN_CLOSEPAREN )
	{
		while ( TTRUE )
		{
			AFSMInstruction* pArgument = ParseInstruction( TTRUE, TTRUE, TTRUE );

			if ( m_bErrorOccured || !pArgument )
			{
				PrintError( "Invalid arguments in function" );

				for ( TINT i = 0; i < iNumArguments; i++ )
				{
					if ( apArguments[ i ] )
						delete apArguments[ i ];

					apArguments[ i ] = TNULL;
				}

				if ( pArgument )
					delete pArgument;

				delete pAction;
				return TNULL;
			}

			apArguments[ iNumArguments++ ] = pArgument;

			if ( m_pFSMLexer->PeekToken( 0 ) != TFileLexer::TOKEN_COMMA )
				break;

			PLexerTRB::Token oToken;
			m_pFSMLexer->GetNextToken( oToken );
		}

		SkipTokenSafe( TFileLexer::TOKEN_CLOSEPAREN );
	}
	else
	{
		PLexerTRB::Token oToken;
		m_pFSMLexer->GetNextToken( oToken );
	}

	if ( m_bErrorOccured )
	{
		PrintError( "Invalid arguments in function" );

		for ( TINT i = 0; i < iNumArguments; i++ )
		{
			if ( apArguments[ i ] )
				delete apArguments[ i ];

			apArguments[ i ] = TNULL;
		}

		delete pAction;
		return TNULL;
	}

	if ( !pAction->SetArguments( apArguments, iNumArguments ) )
	{
		TString8 strError;
		strError.Format( "Failed to initialise arguments to function %s::%s", strClassName.GetString(), strFunctionName.GetString() );
		PrintError( strError );

		return TNULL;
	}

	return pAction;
}

// $Barnyard: FUNCTION 00601c80
AFSMInstruction* AFSMParser::ParseTargetAction()
{
	AFSMInstruction* pTarget = ParseInstruction( TTRUE, TFALSE, TFALSE );

	if ( !pTarget )
	{
		PrintError( "Invalid target for function call" );
		return TNULL;
	}

	SkipTokenSafe( TFileLexer::TOKEN_HYPHEN );
	SkipTokenSafe( TFileLexer::TOKEN_GREATERTHAN );

	AFSMInstructionAction* pAction = TSTATICCAST( AFSMInstructionAction, ParseAction() );

	if ( !pAction )
	{
		delete pTarget;
		return TNULL;
	}

	pAction->SetTarget( pTarget );
	return pAction;
}

// $Barnyard: FUNCTION 00600170
AFSMInstruction* AFSMParser::ParseIfBlock()
{
	AFSMInstructionIfBlock* pIfBlock = new AFSMInstructionIfBlock();

	if ( m_pFSMLexer->PeekToken( 0 ) == TFileLexer::TOKEN_OPENBRACE )
	{
		PLexerTRB::Token oToken;
		m_pFSMLexer->GetNextToken( oToken );
	}
	else
	{
		AFSMInstruction* pCondition = ParseInstruction( TTRUE, TTRUE, TTRUE );

		if ( m_bErrorOccured || !pCondition )
		{
			PrintError( "If-block comparison must have a valid expression" );
			delete pIfBlock;
			return TNULL;
		}

		pIfBlock->SetCondition( pCondition );
		SkipTokenSafe( TFileLexer::TOKEN_OPENBRACE );
	}

	if ( !m_bErrorOccured )
	{
		while ( m_pFSMLexer->PeekToken( 0 ) != TFileLexer::TOKEN_CLOSEBRACE )
		{
			AFSMInstruction* pInstruction = ParseInstruction( TFALSE, TTRUE, TTRUE );

			if ( pInstruction )
				pIfBlock->AddInstruction( pInstruction );

			if ( m_bErrorOccured )
				break;
		}

		if ( !m_bErrorOccured )
		{
			PLexerTRB::Token oToken;
			m_pFSMLexer->GetNextToken( oToken );

			if ( !m_bErrorOccured )
				return pIfBlock;
		}
	}

	delete pIfBlock;
	return TNULL;
}

// $Barnyard: FUNCTION 00601260
AFSMInstruction* AFSMParser::ParseTransition()
{
	AFSMInstructionTransition* pTransition = new AFSMInstructionTransition();
	PLexerTRB::Token           oToken;

	if ( m_pFSMLexer->PeekToken( 0 ) == TFileLexer::TOKEN_OPENBRACE )
	{
		m_pFSMLexer->GetNextToken( oToken );
	}
	else
	{
		AFSMInstruction* pCondition = ParseInstruction( TTRUE, TTRUE, TTRUE );

		if ( m_bErrorOccured || !pCondition )
		{
			PrintError( "Transition comparison must have a valid expression" );
			delete pTransition;
			return TNULL;
		}

		pTransition->SetCondition( pCondition );
		SkipTokenSafe( TFileLexer::TOKEN_OPENBRACE );
	}

	TBOOL bCall = TFALSE;

	// The instructions of the transition, then "[direct] goto" or "[direct] call"
	while ( !m_bErrorOccured )
	{
		TBOOL bDirect = TFALSE;

		if ( IsKeyword( KEYWORD_DIRECT ) )
		{
			m_pFSMLexer->GetNextToken( oToken );
			pTransition->SetDirect( TTRUE );
			bDirect = TTRUE;
		}

		if ( IsKeyword( KEYWORD_CALL ) )
		{
			bCall = TTRUE;
			break;
		}

		if ( IsKeyword( KEYWORD_GOTO ) )
		{
			m_pFSMLexer->GetNextToken( oToken );
			break;
		}

		if ( bDirect )
		{
			// NOTE: retail doesn't delete the transition here
			PrintError( "Expected 'goto' or 'call' keywords" );
			return TNULL;
		}

		AFSMInstruction* pInstruction = ParseInstruction( TFALSE, TTRUE, TTRUE );

		if ( m_bErrorOccured )
		{
			delete pTransition;
			return TNULL;
		}

		if ( pInstruction )
			pTransition->AddInstruction( pInstruction );
	}

	if ( bCall )
	{
		m_pFSMLexer->GetNextToken( oToken );

		if ( IsKeyword( KEYWORD_NAMED ) )
		{
			// The name of the machine is a value
			m_pFSMLexer->GetNextToken( oToken );
			AFSMInstruction* pMachineName = ParseInstruction( TTRUE, TTRUE, TTRUE );

			if ( m_bErrorOccured || !pMachineName )
			{
				if ( pMachineName )
					delete pMachineName;

				delete pTransition;
				return TNULL;
			}

			pTransition->SetMachineName( m_iFSMFileIndex, pMachineName );
		}
		else
		{
			TPString8 strMachineName = GetIdent();
			AFSM*     pMachine       = m_pFSMManager->FindOrCreateStateMachine( m_iFSMFileIndex, strMachineName );

			if ( !pMachine )
			{
				TString8 strError;
				strError.Format( "Unresolved machine %s called", strMachineName.GetString() );
				PrintError( strError );

				delete pTransition;
				return TNULL;
			}

			pTransition->SetCallMachine( pMachine );
		}

		// Arguments of the called machine
		if ( m_pFSMLexer->PeekToken( 0 ) == TFileLexer::TOKEN_OPENPAREN )
		{
			m_pFSMLexer->GetNextToken( oToken );

			while ( TTRUE )
			{
				AFSMInstruction* pArgument = ParseInstruction( TTRUE, TFALSE, TTRUE );

				if ( m_bErrorOccured )
				{
					delete pTransition;
					return TNULL;
				}

				if ( pArgument )
					pTransition->AddArgument( pArgument );

				if ( m_pFSMLexer->PeekToken( 0 ) == TFileLexer::TOKEN_CLOSEPAREN )
				{
					m_pFSMLexer->GetNextToken( oToken );
					break;
				}

				SkipTokenSafe( TFileLexer::TOKEN_COMMA );
			}
		}
	}

	if ( m_bErrorOccured )
	{
		delete pTransition;
		return TNULL;
	}

	// The state to go to (or to return to from the called machine) is resolved once the machine is parsed
	NameLink* pLink = new NameLink();
	pLink->pData    = pTransition;
	pLink->strName  = GetIdent();
	SkipTokenSafe( TFileLexer::TOKEN_CLOSEBRACE );
	m_llTransitions.InsertTail( pLink );

	return pTransition;
}

// $Barnyard: FUNCTION 006024b0
AFSMInstruction* AFSMParser::ParseSetException()
{
	PPropertyValue oIndex;
	SkipTokenSafe( TFileLexer::TOKEN_OPENPAREN );
	ParseLiteralValue( &oIndex );
	SkipTokenSafe( TFileLexer::TOKEN_COMMA );

	AFSMInstruction* pCondition = ParseInstruction( TFALSE, TFALSE, TTRUE );
	AFSMInstruction* pException = new AFSMInstructionSetException( oIndex.GetInteger(), pCondition );

	SkipTokenSafe( TFileLexer::TOKEN_CLOSEPAREN );
	return pException;
}

// $Barnyard: FUNCTION 00601b20
AFSMInstruction* AFSMParser::ParseCheckException()
{
	PPropertyValue oIndex;
	SkipTokenSafe( TFileLexer::TOKEN_OPENPAREN );
	ParseLiteralValue( &oIndex );

	AFSMInstruction* pException = new AFSMInstructionCheckException( oIndex.GetInteger() );

	SkipTokenSafe( TFileLexer::TOKEN_CLOSEPAREN );
	return pException;
}

// $Barnyard: FUNCTION 00601bd0
AFSMInstruction* AFSMParser::ParseClearException()
{
	PPropertyValue oIndex;
	SkipTokenSafe( TFileLexer::TOKEN_OPENPAREN );
	ParseLiteralValue( &oIndex );

	AFSMInstruction* pException = new AFSMInstructionClearException( oIndex.GetInteger() );

	SkipTokenSafe( TFileLexer::TOKEN_CLOSEPAREN );
	return pException;
}

// The asserts are skipped
// $Barnyard: FUNCTION 006005f0
AFSMInstruction* AFSMParser::ParseAssert()
{
	SkipTokenSafe( TFileLexer::TOKEN_OPENPAREN );

	TINT iDepth = 1;

	do
	{
		if ( m_bErrorOccured )
			break;

		PLexerTRB::Token oToken;
		TUINT8           uiToken = m_pFSMLexer->PeekToken( 0 );

		if ( uiToken == TFileLexer::TOKEN_OPENPAREN )
		{
			m_pFSMLexer->GetNextToken( oToken );
			iDepth++;
		}
		else if ( uiToken == TFileLexer::TOKEN_CLOSEPAREN )
		{
			m_pFSMLexer->GetNextToken( oToken );
			iDepth--;
		}
		else if ( uiToken != TFileLexer::TOKEN_EOF )
		{
			m_pFSMLexer->GetNextToken( oToken );
		}
		else
		{
			m_pFSMLexer->GetNextToken( oToken );
			PrintError( "Error parsing assert statement" );
		}
	} while ( iDepth > 0 );

	return TNULL;
}

// $Barnyard: FUNCTION 00600d80
AFSMInstruction* AFSMParser::ParseLiteral( TBOOL a_bAllowString )
{
	PPropertyValue oValue;
	ParseLiteralValue( &oValue );

	if ( m_bErrorOccured )
		return TNULL;

	if ( !a_bAllowString && oValue.GetType() == PPropertyValue::TYPE_TPSTRING8 )
	{
		PrintError( "Unknown string, expected value" );
		return TNULL;
	}

	return new AFSMInstructionValueConstant( oValue );
}

// $Barnyard: FUNCTION 00600af0
void AFSMParser::ParseLiteralValue( PPropertyValue* a_pValue )
{
	PLexerTRB::Token oToken;
	m_pFSMLexer->GetNextToken( oToken );

	if ( oToken.uiTokenType == TFileLexer::TOKEN_IDENT || oToken.uiTokenType >= TFileLexer::TOKEN_NUMOF )
	{
		TString8 strError;
		strError.Format( "Expected value, got '%s'", oToken.GetIdent( m_pFSMLexer->GetCustomIdents() ) );
		PrintError( strError );
		return;
	}

	// NOTE: retail assigns through the PPropertyValue::operator= overloads (006b17d0, 006b1730, 006b1750, 006b17b0)
	switch ( oToken.uiTokenType )
	{
		case TFileLexer::TOKEN_STRING:
			a_pValue->SetTPString8( TPString8( oToken.pString ) );
			break;
		case TFileLexer::TOKEN_INTEGER:
			a_pValue->SetInteger( oToken.iValue );
			break;
		case TFileLexer::TOKEN_UINTEGER:
			a_pValue->SetUINT32( oToken.uiValue );
			break;
		case TFileLexer::TOKEN_FLOAT:
			a_pValue->SetFloat( oToken.flValue );
			break;
		default:
		{
			// NOTE: retail gets the identifier of the token, which isn't valid for symbols
			TString8 strError;
			strError.Format( "Expected value, got '%s'", m_pFSMLexer->GetTokenName( oToken.uiTokenType ) );
			PrintError( strError );
			break;
		}
	}
}

// $Barnyard: FUNCTION 006019e0
AFSMInstruction* AFSMParser::ParseVariable( TBOOL a_bAllowAssignment )
{
	// This reads variable type, but ParseVariableType changes it to be variable name!!!
	TPString8     strVarName = GetIdent();
	const TClass* pTypeClass = ParseVariableType( strVarName );

	// Find or create variable
	TINT iVarIdx = m_pFSM->FindVariableNameIndex( strVarName );
	if ( iVarIdx == -1 ) iVarIdx = m_pFSM->AddVariableName( strVarName );

	m_pFSM->SetVariableType( iVarIdx, pTypeClass );

	if ( GetOperator( TFALSE ) != AFSMInstructionComparison::OPERATOR_NONE )
	{
		PrintError( "Cannot have global declaration in comparison expression" );
		return TNULL;
	}

	if ( m_pFSMLexer->PeekToken( 0 ) != TFileLexer::TOKEN_EQUAL )
		return TNULL;

	if ( !a_bAllowAssignment )
	{
		PrintError( "Assignment not allowed in this context" );
		return TNULL;
	}

	return ParseGlobal( TFALSE, iVarIdx );
}

// $Barnyard: FUNCTION 00600480
AFSMInstruction* AFSMParser::ParseGlobal( TBOOL a_bAllowGlobalWithoutAssignment, TINT a_iVarIdx )
{
	if ( GetOperator( TFALSE ) == AFSMInstructionComparison::OPERATOR_NONE && m_pFSMLexer->PeekToken( 0 ) == TFileLexer::TOKEN_EQUAL )
	{
		PLexerTRB::Token oToken;
		m_pFSMLexer->GetNextToken( oToken );

		AFSMInstruction* pValue = ParseInstruction( TTRUE, TTRUE, TTRUE );

		if ( m_bErrorOccured || !pValue )
			return TNULL;

		AFSMInstructionAssignment* pAssignment = new AFSMInstructionAssignment();
		pAssignment->SetGlobalIndex( a_iVarIdx );
		pAssignment->SetValue( pValue );
		return pAssignment;
	}

	if ( !a_bAllowGlobalWithoutAssignment )
	{
		PrintError( "Global without assignment found" );
		return TNULL;
	}

	return new AFSMInstructionValueGlobal( m_pFSM, a_iVarIdx );
}

// $Barnyard: FUNCTION 00601d10
AFSMInstruction* AFSMParser::ParseLocalDeclaration()
{
	TPString8     strName   = GetIdent();
	const TClass* pType     = ParseVariableType( strName );
	TINT          iLocalIdx = m_pState->FindLocalIndex( strName );

	if ( iLocalIdx == -1 )
		iLocalIdx = m_pState->AddLocal( strName );

	m_pLocalTypes->SetType( iLocalIdx, pType );

	if ( GetOperator( TFALSE ) != AFSMInstructionComparison::OPERATOR_NONE )
	{
		PrintError( "Cannot have local declaration in comparison expression" );
		return TNULL;
	}

	if ( m_pFSMLexer->PeekToken( 0 ) != TFileLexer::TOKEN_EQUAL )
		return TNULL;

	return ParseLocal( TFALSE, iLocalIdx );
}

// $Barnyard: FUNCTION 006017d0
AFSMInstruction* AFSMParser::ParseLocal( TBOOL a_bAllowLocalWithoutAssignment, TINT a_iLocalIdx )
{
	if ( a_iLocalIdx == -1 )
	{
		TPString8 strName = GetIdent();
		a_iLocalIdx       = m_pState->FindLocalIndex( strName );

		if ( a_iLocalIdx == -1 )
		{
			TString8 strError;
			strError.Format( "Expression in form of 'local = ', but local name '%s' unknown", strName.GetString() );
			PrintError( strError );
			return TNULL;
		}
	}

	if ( GetOperator( TFALSE ) == AFSMInstructionComparison::OPERATOR_NONE && m_pFSMLexer->PeekToken( 0 ) == TFileLexer::TOKEN_EQUAL )
	{
		PLexerTRB::Token oToken;
		m_pFSMLexer->GetNextToken( oToken );

		AFSMInstruction* pValue = ParseInstruction( TTRUE, TTRUE, TTRUE );

		if ( m_bErrorOccured || !pValue )
			return TNULL;

		AFSMInstructionAssignment* pAssignment = new AFSMInstructionAssignment();
		pAssignment->SetLocalIndex( a_iLocalIdx );
		pAssignment->SetValue( pValue );
		return pAssignment;
	}

	if ( !a_bAllowLocalWithoutAssignment )
	{
		PrintError( "Local without assignment found" );
		return TNULL;
	}

	return new AFSMInstructionValueLocal( a_iLocalIdx );
}

// $Barnyard: FUNCTION 006016c0
const TClass* AFSMParser::ParseVariableType( TPString8& a_rName )
{
	const TClass* pVarClass = PPropertyValue::TYPE_UNDEF;

	if ( VARTYPE( bool ) == a_rName ) pVarClass = PPropertyValue::TYPE_BOOL;
	else if ( VARTYPE( int ) == a_rName ) pVarClass = PPropertyValue::TYPE_INT;
	else if ( VARTYPE( float ) == a_rName ) pVarClass = PPropertyValue::TYPE_FLOAT;
	else if ( VARTYPE( string ) == a_rName ) pVarClass = PPropertyValue::TYPE_TPSTRING8;
	else if ( VARTYPE( unit ) == a_rName ) pVarClass = &TGetClass( AUnit );
	else
	{
		// Try looking for TClass with such name
		const TClass* pClass = TClass::Find( a_rName.GetString(), TNULL );
		if ( pClass != TNULL ) pVarClass = pClass;
	}

	if ( pVarClass != PPropertyValue::TYPE_UNDEF )
	{
		// Read variable name into the passed argument (this is awful)
		a_rName = GetIdent();
	}

	return pVarClass;
}
