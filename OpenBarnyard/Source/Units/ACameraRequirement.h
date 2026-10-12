#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// "type" "inview": the units are in the view of the current camera (closer
// than "viewdistance" along the view direction). Any other type only sets the
// weight (1 if inverted, 0 otherwise).
// TClass 0x0078aa1c, vtable 0x0072a20c, size 0x54.
//-----------------------------------------------------------------------------
class ACameraRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( ACameraRequirement, AInteractionRequirement );

	enum TYPE : TINT
	{
		TYPE_INVIEW = 0, // "inview"
		TYPE_NONE   = 1,
	};

public:
	// NOTE: inlined into CreateObject
	ACameraRequirement()
	    : m_eType( TYPE_NONE )
	    , m_fViewDistance( 150.0f )
	{}

	~ACameraRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	// Allows the units that are in the view of the current camera
	void EvaluateInView( AInteractionParticipants* a_pParticipants );

private:
	TYPE   m_eType;         // 0x4c, "type"
	TFLOAT m_fViewDistance; // 0x50, "viewdistance"
};

TSTATICASSERT( sizeof( ACameraRequirement ) == 0x54 );
