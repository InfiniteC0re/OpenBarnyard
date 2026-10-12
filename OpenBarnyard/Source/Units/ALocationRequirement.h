#pragma once
#include "AInteractionRequirement.h"

#include <Math/TVector3.h>
#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// The units are between "minradius" and "maxradius" of a place: the area
// "location" of the interior network "network" (moved by "offset") or the
// point "position". Otherwise "world" ("barn" or anything else for the sim)
// picks the game state that has to be on the stack.
// TClass 0x0078ad4c, vtable 0x0072a780, size 0x70.
//-----------------------------------------------------------------------------
class ALocationRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( ALocationRequirement, AInteractionRequirement );

public:
	// NOTE: inlined into CreateObject, m_vecPosition isn't initialised
	ALocationRequirement()
	    : m_fMinRadiusSq( 0.0f )
	    , m_fMaxRadiusSq( 0.0f )
	{}

	~ALocationRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	void SetPosition( const Toshi::TVector4& a_rcPosition );

private:
	Toshi::TPString8 m_strNetwork;  // 0x4c, "network"
	Toshi::TPString8 m_strLocation; // 0x50, "location" (name of the area) or the name of the game state ("world")
	TINT             m_iUnk54;      // 0x54, unused
	// NOTE: a TVector4 that isn't 16-byte aligned in retail
	Toshi::TVector3 m_vecPosition;  // 0x58, "position" or "offset"
	TFLOAT          m_fPositionW;   // 0x64
	TFLOAT          m_fMinRadiusSq; // 0x68, "minradius"
	TFLOAT          m_fMaxRadiusSq; // 0x6c, "maxradius"
};

TSTATICASSERT( sizeof( ALocationRequirement ) == 0x70 );
