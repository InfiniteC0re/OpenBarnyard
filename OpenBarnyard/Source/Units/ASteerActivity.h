#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Changes the steer of the unit of the category to "steer". With "type"
// "whennetwork" only when the unit is on an interior network.
//-----------------------------------------------------------------------------
class ASteerActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( ASteerActivity, AInteractionActivity );

	using TYPE = TUINT8;
	enum TYPE_ : TYPE
	{
		TYPE_ALWAYS       = 0,
		TYPE_WHEN_NETWORK = 1, // "whennetwork"
	};

public:
	ASteerActivity();
	~ASteerActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c

private:
	TYPE           m_eType;       // 0x08, "type"
	Toshi::TClass* m_pSteerClass; // 0x0c, "steer"
};

TSTATICASSERT( sizeof( ASteerActivity ) == 0x10 );
