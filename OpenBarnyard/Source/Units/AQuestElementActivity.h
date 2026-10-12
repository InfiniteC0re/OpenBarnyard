#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Completes the element "elementnum" (1-based) of the "questinstancenum"-th
// assigned quest called "quest".
//-----------------------------------------------------------------------------
class AQuestElementActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( AQuestElementActivity, AInteractionActivity );

public:
	AQuestElementActivity();
	~AQuestElementActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c

private:
	Toshi::TPString8 m_strQuest;       // 0x08, "quest"
	TINT             m_iElement;       // 0x0c, "elementnum", 1-based
	TINT             m_iQuestInstance; // 0x10, "questinstancenum", 1-based
	TBOOL            m_bCompleted;     // 0x14, "completed", unused
};

TSTATICASSERT( sizeof( AQuestElementActivity ) == 0x18 );
