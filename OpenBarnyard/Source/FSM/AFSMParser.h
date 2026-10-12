#pragma once
#include "AFSMManager.h"
#include "AFSMInstruction.h"

#include <Toshi/TDList.h>
#include <Toshi/TPString8.h>
#include <Plugins/PLexerTRB.h>

//-----------------------------------------------------------------------------
// Compiles the token stream of a compiled FSM file (see Tools/FSMCompiler) into
// AFSM machines made of AFSMInstruction trees (vtable 0x007361cc).
//
// Syntax of a machine:
//   name { machine name( type arg, ... ) { [static type var [= value]] [default] [exitstart] [exitend] state name { instructions } common { transitions } } }
//-----------------------------------------------------------------------------
class AFSMParser
{
public:
	// Keywords of the compiled files, their tokens follow the ones of TFileLexer
	enum KEYWORD : TUINT8
	{
		KEYWORD_STATE = Toshi::TFileLexer::TOKEN_NUMOF,
		KEYWORD_MACHINE,
		KEYWORD_IF,
		KEYWORD_TRANSITION,
		KEYWORD_ACTION,
		KEYWORD_LOCAL,
		KEYWORD_STATIC,
		KEYWORD_DEFAULT,
		KEYWORD_EXITSTART,
		KEYWORD_EXITEND,
		KEYWORD_COMMON,
		KEYWORD_GOTO,
		KEYWORD_CALL,
		KEYWORD_NAMED,
		KEYWORD_DIRECT,
		KEYWORD_STAY,
		KEYWORD_MYUNIT,
		KEYWORD_T_MACHINE,
		KEYWORD_T_STATE,
		KEYWORD_BOOL,
		KEYWORD_INT,
		KEYWORD_FLOAT,
		KEYWORD_STRING,
		KEYWORD_UNIT,
		KEYWORD_TRUE,
		KEYWORD_FALSE,
		KEYWORD_NULL,
		KEYWORD_ASSERT,
		KEYWORD_PRINT,
		KEYWORD_SETEXCEPTION,
		KEYWORD_CHECKEXCEPTION,
		KEYWORD_CLEAREXCEPTION,
	};

	using OPERATOR = AFSMInstructionComparison::OPERATOR;

	struct OperatorDescription
	{
		TUINT eToken1;
		TUINT eToken2;
	};

	// DAT_007360cc
	inline static constexpr OperatorDescription OPERATORS[ AFSMInstructionComparison::OPERATOR_NUMOF ] = {
		{ Toshi::TFileLexer::TOKEN_EQUAL, Toshi::TFileLexer::TOKEN_EQUAL },
		{ Toshi::TFileLexer::TOKEN_EXCLAMATION, Toshi::TFileLexer::TOKEN_EQUAL },
		{ Toshi::TFileLexer::TOKEN_LESSTHAN, Toshi::TFileLexer::TOKEN_EQUAL },
		{ Toshi::TFileLexer::TOKEN_LESSTHAN, 0 },
		{ Toshi::TFileLexer::TOKEN_GREATERTHAN, Toshi::TFileLexer::TOKEN_EQUAL },
		{ Toshi::TFileLexer::TOKEN_GREATERTHAN, 0 }
	};

	// A name that is resolved once the machine is parsed: a state or the state a transition goes to
	// NOTE: the name is made up
	struct NameLink
	    : public Toshi::TDList<NameLink>::TNode
	{
		NameLink()
		    : pData( TNULL )
		{}

		// $Barnyard: FUNCTION 00600d10
		~NameLink() = default;

		Toshi::TPString8 strName; // 0x08
		void*            pData;   // 0x0C, AFSMState or AFSMInstructionTransition
	};

public:
	AFSMParser();
	virtual ~AFSMParser();

	TBOOL LoadFSMFile( AFSMManager* a_pFSMManager, Toshi::TString8* a_pFSMFilePath, Toshi::TString8* a_pFSMDebugFilePath, TINT a_iFSMFileIndex );

	void ParseStateMachine( AFSMManager* a_pFSMManager, TINT a_iFSMFileIndex, AFSM* a_pFSM );

private:
	Toshi::TPString8 GetIdent();

	TBOOL IsKeyword( TUINT8 a_uiKeyword );
	TBOOL SkipKeyword( TUINT8 a_uiKeyword );
	TBOOL IsIdentOrKeyword();
	TBOOL IsAction();
	TBOOL IsTargetAction();

	void SkipTokenSafe( TUINT8 a_uiExpect );
	void SkipCustomTokenSafe( TUINT8 a_uiExpect );
	void ResetError() { m_bErrorOccured = TFALSE; }
	void PrintError( const TCHAR* a_pchErrorText );
	void PrintWarning( const TCHAR* a_pchWarningText );

	OPERATOR GetOperator( TBOOL a_bSkip );

	void ParseMachine();
	void ParseState();
	void ResolveTransitions();

	// Parses an instruction. a_bValue allows values (and comparisons of them), a_bAllowTransition
	// allows transitions and a_bAllowTarget allows calling functions on a value ("value->class::function()").
	AFSMInstruction* ParseInstruction( TBOOL a_bValue, TBOOL a_bAllowTransition, TBOOL a_bAllowTarget );

	AFSMInstruction* ParseComparison( AFSMInstruction* a_pLeft, TBOOL a_bOptional );
	AFSMInstruction* ParseAction();
	AFSMInstruction* ParseTargetAction();
	AFSMInstruction* ParseIfBlock();
	AFSMInstruction* ParseTransition();
	AFSMInstruction* ParseSetException();
	AFSMInstruction* ParseCheckException();
	AFSMInstruction* ParseClearException();
	AFSMInstruction* ParseAssert();
	AFSMInstruction* ParseLiteral( TBOOL a_bAllowString );
	void             ParseLiteralValue( PPropertyValue* a_pValue );

	// Variable of the machine ("type name [= value]")
	AFSMInstruction* ParseVariable( TBOOL a_bAllowAssignment );
	AFSMInstruction* ParseGlobal( TBOOL a_bAllowGlobalWithoutAssignment, TINT a_iVarIdx );

	// Local of the state ("local [type] name = value")
	AFSMInstruction* ParseLocalDeclaration();
	AFSMInstruction* ParseLocal( TBOOL a_bAllowLocalWithoutAssignment, TINT a_iLocalIdx );

	// Returns detected TClass type of the variable string type specified in a_rName and then changes it value to be variable name
	const Toshi::TClass* ParseVariableType( Toshi::TPString8& a_rName );

private:
	AFSMLocalTypes*         m_pLocalTypes;   // 0x04
	AFSM*                   m_pFSM;          // 0x08
	AFSMState*              m_pState;        // 0x0C
	AFSMManager*            m_pFSMManager;   // 0x10
	TINT                    m_iFSMFileIndex; // 0x14
	Toshi::TDList<NameLink> m_llStates;      // 0x18
	Toshi::TDList<NameLink> m_llTransitions; // 0x20
	PLexerTRB*              m_pFSMLexer;     // 0x28
	TBOOL                   m_bErrorOccured; // 0x2C
};
