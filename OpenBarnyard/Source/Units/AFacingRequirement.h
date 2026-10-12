#pragma once
#include "AInteractionRequirement.h"

#include <Math/TVector4.h>

class AUnit;
class AUnitType;

//-----------------------------------------------------------------------------
// A unit faces something (dot product of its direction and the direction to it
// above "threshold"). "target":
//  "goal":        the player faces the interior goal "goal" ("facingtype"
//                 "targetpos": its position, "targetfacing": its direction)
//  "search":      the units face the player ("targettype" must be "player")
//  "interaction": the units of "category" and "targetcategory" face each other
// TClass 0x0078ab3c, vtable 0x0072a394, size 0x64.
//-----------------------------------------------------------------------------
class AFacingRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( AFacingRequirement, AInteractionRequirement );

	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE          = 0,
		FLAGS_SEARCH        = BITFLAG( 0 ), // "target" "search"
		FLAGS_INTERACTION   = BITFLAG( 2 ), // "target" "interaction"
		FLAGS_TARGET_PLAYER = BITFLAG( 4 ), // "targettype" is "player"
		FLAGS_TARGET_POS    = BITFLAG( 5 ), // "facingtype" "targetpos"
		FLAGS_GOAL          = BITFLAG( 6 ), // "target" "goal"
		FLAGS_TARGET_FACING = BITFLAG( 7 ), // "facingtype" "targetfacing"
	};

public:
	// NOTE: inlined into CreateObject
	AFacingRequirement()
	    : m_uiFlags( FLAGS_NONE )
	    , m_fThreshold( 0.8f )
	{}

	~AFacingRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	// Whether the unit faces the (XZ) direction a_rcDirection
	TBOOL IsFacingDirection( AUnit* a_pUnit, const Toshi::TVector4& a_rcDirection );

	// Whether a_pUnit faces a_pTarget
	TBOOL IsFacingUnit( AUnit* a_pUnit, AUnit* a_pTarget );

private:
	AUnitType*                     m_pTargetType;     // 0x4c, "targettype"
	FLAGS                          m_uiFlags;         // 0x50
	TFLOAT                         m_fThreshold;      // 0x54, "threshold"
	TFLOAT                         m_fMaxRadius;      // 0x58, "maxradius", unused
	AInteractionSequence::CATEGORY m_eTargetCategory; // 0x5c, "targetcategory", also the index of the unit
	Toshi::TPString8               m_strGoal;         // 0x60, "goal"
};

TSTATICASSERT( sizeof( AFacingRequirement ) == 0x64 );
