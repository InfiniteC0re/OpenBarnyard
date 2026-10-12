#pragma once
#include "AQuestElement.h"

#include <Toshi/TString16.h>

//-----------------------------------------------------------------------------
// Quest element that never completes by itself ("dummy"), it is completed by
// AQuestElementActivity (the interactions of the quest) or AQuestActivity.
//-----------------------------------------------------------------------------
class ADummyQuestElement : public AQuestElement
{
public:
	TDECLARE_CLASS( ADummyQuestElement, AQuestElement );

public:
	ADummyQuestElement();
	~ADummyQuestElement();

	//-----------------------------------------------------------------------------
	// AQuestElement
	//-----------------------------------------------------------------------------
	virtual TBOOL    Create( const PBProperties* a_pProperties, TINT* a_pDataOffset ) OVERRIDE; // +0x0c
	virtual TBOOL    InitData( AInteractionActivityData* a_pData ) OVERRIDE;                    // +0x10
	virtual TBOOL    GetDescription( const TWCHAR** a_ppOutText ) OVERRIDE;                     // +0x18
	virtual PROGRESS CheckProgress( AInteractionActivityData* a_pData ) OVERRIDE;               // +0x28

private:
	Toshi::TString16 m_strDescription; // 0x5c
};

TSTATICASSERT( sizeof( ADummyQuestElement ) == 0x68 );
