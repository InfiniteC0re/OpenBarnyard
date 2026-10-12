#pragma once
#include "AInteractionActivity.h"

class AUnitType;

//-----------------------------------------------------------------------------
// Turns the units of the category to face a "target". With "block" it waits
// until they face it.
//-----------------------------------------------------------------------------
class AFacingActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( AFacingActivity, AInteractionActivity );

	using FLAGS = TUINT16;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE                 = 0,
		FLAGS_BLOCK                = BITFLAG( 0 ),  // "block"
		FLAGS_SEARCH               = BITFLAG( 1 ),  // "search", the closest unit of the type "targettype"
		FLAGS_NETWORK              = BITFLAG( 2 ),  // "network", the area "area" of the interior network "network"
		FLAGS_INTERACTION          = BITFLAG( 3 ),  // "interaction", the units of the category "targetcategory"
		FLAGS_GOAL                 = BITFLAG( 4 ),  // "goal", the goal "goaltype" of the default interior network
		FLAGS_GOAL_FACING          = BITFLAG( 5 ),  // "goalfacing", the direction of the goal "goaltype"
		FLAGS_PRIMARY              = BITFLAG( 6 ),  // "primary"
		FLAGS_SECONDARY            = BITFLAG( 7 ),  // "secondary"
		FLAGS_AREA                 = BITFLAG( 8 ),  // "area", the current area of the unit
		FLAGS_MILKWEED             = BITFLAG( 9 ),  // "milkweed"
		FLAGS_FACE_CLOSEST_PADLOCK = BITFLAG( 10 ), // "faceclosestpadlock"
	};

public:
	AFacingActivity();
	~AFacingActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE;  // +0x0c
	virtual RESULT OnStart( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x18
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE;  // +0x1c

private:
	// Averages the positions of the units of a category, returns TFALSE if there are none.
	static TBOOL GetCategoryCenter( Toshi::TVector4& a_rOutPosition, AInteractionSequence* a_pSequence, AInteractionSequence::CATEGORY a_eCategory );

	// Returns the closest unit of the type within a_fMaxRadius, the player has to be within it too.
	static AUnit* FindClosestUnit( const Toshi::TVector4& a_rcPosition, AUnitType* a_pType, TFLOAT a_fMaxRadius );

private:
	AUnitType*                     m_pTargetType;     // 0x08, "targettype"
	FLAGS                          m_uiFlags;         // 0x0c
	Toshi::TPString8               m_strNetwork;      // 0x10, "network"
	Toshi::TPString8               m_strArea;         // 0x14, "area"
	Toshi::TPString8               m_strGoalType;     // 0x18, "goaltype"
	TFLOAT                         m_fMaxRadius;      // 0x1c, "maxradius"
	AInteractionSequence::CATEGORY m_eTargetCategory; // 0x20, "targetcategory"
};

TSTATICASSERT( sizeof( AFacingActivity ) == 0x24 );
