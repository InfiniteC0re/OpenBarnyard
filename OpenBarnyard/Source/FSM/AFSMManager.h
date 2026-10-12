#pragma once
#include "AFSM.h"
#include "AMiscFunctions.h"

#include <Toshi/TTask.h>
#include <Toshi/TSingleton.h>

//-----------------------------------------------------------------------------
// Loads the compiled FSM scripts (Data/Units/allfsms.trb) and keeps the machines.
// Created by ARootTask, the TClass is at 0x0079afb0 (vtable 0x00735ff8).
//-----------------------------------------------------------------------------
class AFSMManager
    : public Toshi::TTask
    , public Toshi::TSingleton<AFSMManager>
{
public:
	TDECLARE_CLASS( AFSMManager, Toshi::TTask );

	static constexpr TINT MAX_FILES = 1;

public:
	AFSMManager();
	~AFSMManager();

	//-----------------------------------------------------------------------------
	// Toshi::TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL OnCreate() OVERRIDE;
	virtual void  OnDestroy() OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Creates the machine, returns TNULL if it already exists and has been parsed
	AFSM* CreateStateMachine( TINT a_iFSMFileIndex, const Toshi::TPString8& a_strName );

	// Returns the machine, creates an empty one if it doesn't exist yet (called machines can be defined later)
	AFSM* FindOrCreateStateMachine( TINT a_iFSMFileIndex, const Toshi::TPString8& a_strName );

	AFSM* FindStateMachine( TINT a_iFSMFileIndex, const Toshi::TPString8& a_strName );
	void  DestroyStateMachine( AFSM* a_pFSM );

	// Lets the instructions of all machines release their cached data
	void ResetStateMachines();

private:
	void ParseFSMs( TINT a_iFSMFileIndex, const TCHAR* a_pchFileName );

private:
	Toshi::TDList<AFSM> m_aMachineLists[ MAX_FILES ]; // 0x20
	AMiscFunctions*     m_pMiscFunctions;             // 0x28
};
