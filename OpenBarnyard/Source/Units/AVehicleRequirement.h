#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// The player rides a bike ("bikeflag" TRUE) or doesn't ("bikeflag" FALSE).
// TClass 0x0078b19c, vtable 0x0072af0c, size 0x50.
//-----------------------------------------------------------------------------
class AVehicleRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( AVehicleRequirement, AInteractionRequirement );

	using VEHICLE = TINT;
	enum VEHICLE_ : VEHICLE
	{
		VEHICLE_NONE = 0, // "bikeflag" FALSE
		VEHICLE_BIKE = 1, // "bikeflag" TRUE
	};

public:
	// NOTE: inlined into CreateObject, m_eVehicle isn't initialised
	AVehicleRequirement() = default;
	~AVehicleRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	VEHICLE m_eVehicle; // 0x4c, "bikeflag"
};

TSTATICASSERT( sizeof( AVehicleRequirement ) == 0x50 );
