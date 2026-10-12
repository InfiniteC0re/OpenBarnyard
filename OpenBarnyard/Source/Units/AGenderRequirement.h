#pragma once
#include "AInteractionRequirement.h"
#include "Player/APlayerProfile.h"

//-----------------------------------------------------------------------------
// The player takes part and is of the gender "gender" ("male" or "female").
// TClass 0x0078abfc, vtable 0x0072a51c, size 0x50.
//-----------------------------------------------------------------------------
class AGenderRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( AGenderRequirement, AInteractionRequirement );

public:
	// NOTE: inlined into CreateObject
	AGenderRequirement()
	    : m_eGender( APlayerProfile::GENDER_NONE )
	{}

	~AGenderRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	APlayerProfile::GENDER m_eGender; // 0x4c, "gender"
};

TSTATICASSERT( sizeof( AGenderRequirement ) == 0x50 );
