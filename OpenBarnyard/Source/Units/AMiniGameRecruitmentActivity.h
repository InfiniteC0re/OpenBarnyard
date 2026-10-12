#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Only holds the "minigamestate" name, the activity itself does nothing (its
// OnStart and Update are the base ones).
//-----------------------------------------------------------------------------
class AMiniGameRecruitmentActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( AMiniGameRecruitmentActivity, AInteractionActivity );

public:
	AMiniGameRecruitmentActivity();
	~AMiniGameRecruitmentActivity();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c

	const Toshi::TPString8& GetMiniGameState() const { return m_strMiniGameState; }

private:
	Toshi::TPString8 m_strMiniGameState; // 0x08, "minigamestate"
};

TSTATICASSERT( sizeof( AMiniGameRecruitmentActivity ) == 0xc );
