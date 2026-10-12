#pragma once
#include "AInteractionRequirement.h"
#include "Helpers/ARegrowthManager.h"

//-----------------------------------------------------------------------------
// An ARegrowthManager object of one of the types "type" ("light", "milkcan",
// "heavy", ...) is in front of the unit within "maxradius" ("facingthresh" is
// the minimum dot product of the unit's direction and the direction to it).
// TClass 0x0078aa4c, vtable 0x0072a25c, size 0x5c.
//-----------------------------------------------------------------------------
class ACloseInstanceRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( ACloseInstanceRequirement, AInteractionRequirement );

public:
	// NOTE: inlined into CreateObject
	ACloseInstanceRequirement()
	    : m_fMaxRadius( 0.0f )
	    , m_fFacingThreshold( 0.0f )
	{}

	~ACloseInstanceRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	ARegrowthManager::TypeMask m_oTypes;           // 0x4c, "type"
	TFLOAT                     m_fMaxRadius;       // 0x54, "maxradius"
	TFLOAT                     m_fFacingThreshold; // 0x58, "facingthresh"
};

TSTATICASSERT( sizeof( ACloseInstanceRequirement ) == 0x5c );
