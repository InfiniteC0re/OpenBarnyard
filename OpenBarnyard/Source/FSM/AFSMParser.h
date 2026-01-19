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
	struct PPropertiesClassRefs
	{
		PPropertiesClassRefs();
		~PPropertiesClassRefs();

		void Setup();

		Toshi::TClass* apPropertiesClasses[ 8 ];
	};

public:
	AFSMParser();
	virtual ~AFSMParser();

	TBOOL LoadFSMFile( AFSMManager* a_pFSMManager, Toshi::TString8* a_pFSMFilePath, Toshi::TString8* a_pFSMDebugFilePath, TINT a_iFSMFileIndex );

	Toshi::TPString8 GetIdent();
	void             ParseStateMachine( AFSMManager* a_pFSMManager, TINT a_iFSMFileIndex, AFSM* a_pFSM );

	void SkipTokenSafe( Toshi::TFileLexer::TOKEN a_eExpect );
	void SkipCustomTokenSafe( TUINT8 a_uiExpect );
	void ResetError() { m_bErrorOccured = TFALSE; }
	void SetError() { m_bErrorOccured = TTRUE; }

private:
	void ParseStateMachineImpl();
	void ResolveTransitionsImpl();

private:
	PPropertiesClassRefs*          m_pPPropertiesClassRefs; // 0x04
	AFSM*                          m_pFSM;                  // 0x08
	void*                          m_Unk;                   // 0x0C
	AFSMManager*                   m_pFSMManager;           // 0x10
	TINT                           a_iFSMFileIndex;         // 0x14
	Toshi::TQList<AFSMInstruction> m_llList1;               // 0x18
	Toshi::TQList<AFSMInstruction> m_llList2;               // 0x20
	PLexerTRB*                     m_pFSMLexer;             // 0x28
	TBOOL                          m_bErrorOccured;         // 0x2C
};
