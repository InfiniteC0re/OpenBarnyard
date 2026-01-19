#pragma once
#include "AFSM.h"
#include "AMiscFunctions.h"

#include <Toshi/TTask.h>
#include <Toshi/TSingleton.h>

class AFSMManager
    : public Toshi::TTask
    , public Toshi::TSingleton<AFSMManager>
{
public:
	static constexpr TINT MAX_FILES = 1;

public:
	AFSMManager();
	~AFSMManager();

	//-----------------------------------------------------------------------------
	// Toshi::TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL OnCreate() OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	AFSM* CreateStateMachine( TINT a_iFSMFileIndex, const Toshi::TPString8& a_strName );
	AFSM* FindStateMachine( TINT a_iFSMFileIndex, const Toshi::TPString8& a_strName );
	void  DestroyStateMachine( AFSM* a_pFSM );

private:
	void ParseFSMs( TINT a_iFSMFileIndex, const TCHAR* a_pchFileName );

private:
	Toshi::TDList<AFSM> m_aMachineLists[ MAX_FILES ]; // 0x20
	AMiscFunctions*     m_pMiscFunctions;             // 0x28
};
