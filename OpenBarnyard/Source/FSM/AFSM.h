#pragma once
#include "AFSMState.h"

#include <Toshi/TDList.h>
#include <Toshi/TPString8.h>

//-----------------------------------------------------------------------------
// Compiled FSM script machine: its states and its variables (the arguments of
// the machine followed by its statics).
//-----------------------------------------------------------------------------
class AFSM
    : public Toshi::TDList<AFSM>::TNode
{
public:
	static constexpr TINT MAX_NUM_VARIABLES = 16;

	// NOTE: kept for compatibility with older code
	static constexpr TINT MAX_NUM_ARGS = MAX_NUM_VARIABLES;

public:
	AFSM( const Toshi::TPString8& a_strName );
	~AFSM();

	const Toshi::TPString8& GetName() const { return m_strName; }

	// The default state is inserted at the head of the list, the machine starts in it
	void AddState( AFSMState* a_pState, TBOOL a_bDefault, TBOOL a_bExitStart, TBOOL a_bExitEnd );

	// The common state isn't entered, its transitions are checked before the ones of the current state
	void SetCommonState( AFSMState* a_pState );

	// Lets the instructions release their cached data
	void Reset();

	Toshi::TDList<AFSMState>& GetStates() { return m_llStates; }
	TBOOL                     HasStates() { return !m_llStates.IsEmpty(); }
	AFSMState*                GetStartState() { return m_llStates.IsEmpty() ? TNULL : m_llStates.Head(); }
	AFSMState*                GetExitStartState() const { return m_pExitStartState; }
	AFSMState*                GetExitEndState() const { return m_pExitEndState; }
	AFSMState*                GetCommonState() const { return m_pCommonState; }

	void AddArgument() { m_iNumArguments++; }
	TINT GetNumArguments() const { return m_iNumArguments; }

	void ResetArguments();

	TINT AddVariableName( const Toshi::TPString8& a_strName );
	TINT FindVariableNameIndex( const Toshi::TPString8& a_strName ) const;

	const Toshi::TClass* GetVariableType( TINT a_iIndex ) const;

	// Returns TTRUE if the type wasn't previously set or is the same, otherwise TFALSE
	TBOOL SetVariableType( TINT a_iIndex, const Toshi::TClass* a_pTypeClass );

private:
	Toshi::TPString8         m_strName;                        // 0x08
	Toshi::TDList<AFSMState> m_llStates;                       // 0x0C
	AFSMState*               m_pExitStartState;                // 0x14
	AFSMState*               m_pExitEndState;                  // 0x18
	AFSMState*               m_pCommonState;                   // 0x1C
	TINT                     m_iNumArguments;                  // 0x20
	const Toshi::TClass*     m_aVarTypes[ MAX_NUM_VARIABLES ]; // 0x24
	TINT                     m_iNumVarNames;                   // 0x64
	Toshi::TPString8         m_aVarNames[ MAX_NUM_VARIABLES ]; // 0x68
};
