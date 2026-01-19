#pragma once
#include "AFSMManager.h"

#include <Toshi/TQList.h>
#include <Toshi/TPString8.h>
#include <Plugins/PLexerTRB.h>

struct AFSMInstruction : public Toshi::TQList<AFSMInstruction>::TNode
{
	Toshi::TPString8 m_strName;
};

class AFSMParser
{
public:
	struct ClassRefs
	{
		ClassRefs();
		~ClassRefs();

		Toshi::TClass* apClasses[ 8 ];
	};

public:
	AFSMParser();
	virtual ~AFSMParser();

	TBOOL Read( AFSMManager* a_pFSMManager, Toshi::TString8* a_pFSMFilePath, Toshi::TString8* a_pFSMDebugFilePath, TINT a_iFSMFileIndex );

	Toshi::TPString8 GetIdent();
	void             ParseStateMachine( AFSMManager* a_pFSMManager, TINT a_iFSMFileIndex, AFSM* a_pFSM );

	void SkipTokenSafe( Toshi::TFileLexer::TOKEN a_eExpect );
	void ResetError() { m_bErrorOccured = TFALSE; }
	void SetError() { m_bErrorOccured = TTRUE; }

private:
	ClassRefs*                     m_pClassRefs;    // 0x04
	// ...
	Toshi::TQList<AFSMInstruction> m_llList1;       // 0x18
	Toshi::TQList<AFSMInstruction> m_llList2;       // 0x20
	PLexerTRB*                     m_pFSMLexer;     // 0x28
	TBOOL                          m_bErrorOccured; // 0x2C
};
