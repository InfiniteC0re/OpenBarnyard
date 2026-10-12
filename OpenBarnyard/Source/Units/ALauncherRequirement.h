#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// The player's active projectile launcher is the milk squirt ("launcher" is
// "normal") or the aimed squirt ("aimed").
// NOTE: retail's Create never stores "launcher", so Evaluate never does anything.
// TClass 0x0078ad1c, vtable 0x0072a738, size 0x50.
//-----------------------------------------------------------------------------
class ALauncherRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( ALauncherRequirement, AInteractionRequirement );

public:
	// NOTE: inlined into CreateObject
	ALauncherRequirement() = default;
	~ALauncherRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	Toshi::TPString8 m_strLauncher; // 0x4c, "launcher" (never set)
};

TSTATICASSERT( sizeof( ALauncherRequirement ) == 0x50 );
