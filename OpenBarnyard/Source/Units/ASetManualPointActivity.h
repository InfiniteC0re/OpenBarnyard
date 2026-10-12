#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Stores the position and direction of the first unit of the sequence in the
// "manualpoint" AWorldPositionActivityMessage.
//-----------------------------------------------------------------------------
class ASetManualPointActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( ASetManualPointActivity, AInteractionActivity );

	using POSITIONTYPE = TINT;
	enum POSITIONTYPE_ : POSITIONTYPE
	{
		POSITIONTYPE_UNIT       = 0, // "unit"
		POSITIONTYPE_GOALOBJECT = 1, // "goalobject"
	};

public:
	ASetManualPointActivity();
	~ASetManualPointActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c

private:
	POSITIONTYPE m_ePositionType; // 0x08, "positiontype"
};

TSTATICASSERT( sizeof( ASetManualPointActivity ) == 0xc );
