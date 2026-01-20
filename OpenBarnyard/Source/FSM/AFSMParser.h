#pragma once
#include "AFSMManager.h"
#include "AFSMInstruction.h"

#include <Toshi/TDList.h>
#include <Toshi/TPString8.h>
#include <Plugins/PLexerTRB.h>

struct AFSMParserS1 : public Toshi::TDList<AFSMParserS1>::TNode
{
	Toshi::TPString8 m_strName;
};

class AFSMParser
{
public:
	struct PPropertiesClassRefs
	{
		PPropertiesClassRefs();
		~PPropertiesClassRefs();

		void Setup();

		const Toshi::TClass* apPropertiesClasses[ 8 ];
	};

	enum OPERATOR
	{
		OPERATOR_EQUAL,
		OPERATOR_NOT_EQUAL,
		OPERATOR_LESS_OR_EQUAL,
		OPERATOR_LESS,
		OPERATOR_GREATER_OR_EQUAL,
		OPERATOR_GREATER,
		OPERATOR_EQUAL_SET
	};

	struct OperatorDescription
	{
		TUINT8 eToken1;
		TUINT8 eToken2;
	};

	// NOTE: the structure contains only 6 elements, no clue why it doesn't have space for OPERATOR_EQUAL_SET
	inline static constexpr OperatorDescription OPERATORS[ 6 ] = {
		{ Toshi::TFileLexer::TOKEN_EQUAL, Toshi::TFileLexer::TOKEN_EQUAL },
		{ Toshi::TFileLexer::TOKEN_EXCLAMATION, Toshi::TFileLexer::TOKEN_EQUAL },
		{ Toshi::TFileLexer::TOKEN_LESSTHAN, Toshi::TFileLexer::TOKEN_EQUAL },
		{ Toshi::TFileLexer::TOKEN_LESSTHAN, 0 },
		{ Toshi::TFileLexer::TOKEN_GREATERTHAN, Toshi::TFileLexer::TOKEN_EQUAL },
		{ Toshi::TFileLexer::TOKEN_GREATERTHAN, 0 }
	};

public:
	AFSMParser();
	virtual ~AFSMParser();

	TBOOL LoadFSMFile( AFSMManager* a_pFSMManager, Toshi::TString8* a_pFSMFilePath, Toshi::TString8* a_pFSMDebugFilePath, TINT a_iFSMFileIndex );

	Toshi::TPString8 GetIdent();
	void             ParseStateMachine( AFSMManager* a_pFSMManager, TINT a_iFSMFileIndex, AFSM* a_pFSM );

	void SkipTokenSafe( TUINT8 a_uiExpect );
	void SkipCustomTokenSafe( TUINT8 a_uiExpect );
	void ResetError() { m_bErrorOccured = TFALSE; }
	void PrintError( const TCHAR* a_pchErrorText );

private:
	void ParseStateMachineImpl();
	void ResolveTransitionsImpl();

	AFSMInstruction* ParseVariable( TBOOL a_bAllowAssignment );
	AFSMInstruction* ParseValue( TBOOL a_bAllowGlobalWithoutAssignment, TINT a_iVarIdx );

	// Returns detected TClass type of the variable string type specified in a_rName and then changes it value to be variable name
	const Toshi::TClass* ParseVariableType( Toshi::TPString8& a_rName );

	OPERATOR GetOperator( TBOOL a_bSkip );

private:
	PPropertiesClassRefs*          m_pPPropertiesClassRefs; // 0x04
	AFSM*                          m_pFSM;                  // 0x08
	void*                          m_Unk;                   // 0x0C
	AFSMManager*                   m_pFSMManager;           // 0x10
	TINT                           a_iFSMFileIndex;         // 0x14
	Toshi::TDList<AFSMParserS1>    m_llList1;               // 0x18
	Toshi::TDList<AFSMParserS1>    m_llList2;               // 0x20
	PLexerTRB*                     m_pFSMLexer;             // 0x28
	TBOOL                          m_bErrorOccured;         // 0x2C
};
