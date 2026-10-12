#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// The AAnimalAttributes attribute "attribute" ("happiness", "command", ...) of
// the unit is inside "range" ([ min, max ]). Players are always allowed.
// TClass 0x0078aadc, vtable 0x0072a31c, size 0x58.
//-----------------------------------------------------------------------------
class ADriveRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( ADriveRequirement, AInteractionRequirement );

public:
	// NOTE: inlined into CreateObject
	ADriveRequirement()
	    : m_eAttribute( -1 )
	    , m_fMin( 0.0f )
	    , m_fMax( 1.0f )
	{
		m_eCategory = AInteractionSequence::CATEGORY_PRIMARY;
	}

	~ADriveRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	TINT   m_eAttribute; // 0x4c, "attribute", AAnimalAttributes::ATTRIBUTE, -1 if none
	TFLOAT m_fMin;       // 0x50, "range"
	TFLOAT m_fMax;       // 0x54, "range"
};

TSTATICASSERT( sizeof( ADriveRequirement ) == 0x58 );
