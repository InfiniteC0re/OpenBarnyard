#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// The replay of the minigame "minigame" is unlocked.
// TClass 0x0078aecc, vtable 0x0072aa3c, size 0x50.
//-----------------------------------------------------------------------------
class AReplayRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( AReplayRequirement, AInteractionRequirement );

public:
	// NOTE: inlined into CreateObject
	AReplayRequirement() = default;
	~AReplayRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	Toshi::TPString8 m_strMiniGame; // 0x4c, "minigame"
};

TSTATICASSERT( sizeof( AReplayRequirement ) == 0x50 );
