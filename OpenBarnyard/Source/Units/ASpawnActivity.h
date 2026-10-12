#pragma once
#include "AInteractionActivity.h"

#include <Math/TVector2.h>
#include <Math/TVector4.h>

class AUnitType;

//-----------------------------------------------------------------------------
// Spawns "count" units of the "unittype" (in an "area", at a "goal" or a
// "locator"), or unspawns / teleports units. "playerrespawngoal" sets the goal
// the player respawns at.
//-----------------------------------------------------------------------------
class ASpawnActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( ASpawnActivity, AInteractionActivity );

	using FLAGS = TUINT16;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE                    = 0,
		FLAGS_AREA                    = BITFLAG( 0 ),  // "area"
		FLAGS_RANDOM                  = BITFLAG( 1 ),  // "random"
		FLAGS_SET_ON_GROUND           = BITFLAG( 2 ),  // "setonground"
		FLAGS_GOAL                    = BITFLAG( 4 ),  // "goal"
		FLAGS_ADD_UNIT_TO_INTERACTION = BITFLAG( 5 ),  // "addunittointeraction"
		FLAGS_NAME                    = BITFLAG( 6 ),  // "name" (unless teleporting)
		FLAGS_STEERING_CLASS          = BITFLAG( 7 ),  // "steeringclass"
		FLAGS_RANDOM_Y                = BITFLAG( 8 ),  // "randomy"
		FLAGS_UNSPAWN                 = BITFLAG( 9 ),  // "unspawn"
		FLAGS_TELEPORT                = BITFLAG( 10 ), // "teleport"
		FLAGS_DISTANCE_TRIGGER        = BITFLAG( 11 ), // "distancetrigger"
		FLAGS_PLAYER_RESPAWN_GOAL     = BITFLAG( 12 ), // "playerrespawngoal"
		FLAGS_IGNORE_FRUSTUM          = BITFLAG( 13 ), // "ignorefrustrum"
		FLAGS_SPAWN_UP_TO             = BITFLAG( 14 ), // "spawnupto", only spawns up to "count" units of the type
		FLAGS_LOCATOR                 = BITFLAG( 15 ), // "locator"
	};

public:
	ASpawnActivity();
	~ASpawnActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c

private:
	// Whether the unit's bounding sphere is outside of the view frustum.
	TBOOL IsUnitOutOfView( AUnit* a_pUnit );

private:
	AUnitType*       m_pUnitType;            // 0x08, "unittype"
	TINT             m_iCount;               // 0x0c, "count"
	Toshi::TVector4  m_vecOffset;            // 0x10, "offset"
	Toshi::TPString8 m_strNetwork;           // 0x20, "network"
	Toshi::TPString8 m_strArea;              // 0x24, "area"
	Toshi::TPString8 m_strGoal;              // 0x28, "goal"
	Toshi::TPString8 m_strName;              // 0x2c, "name"
	Toshi::TPString8 m_strSteeringClass;     // 0x30, "steeringclass"
	Toshi::TVector2  m_vecRandomY;           // 0x34, "randomy", range of the random height offset
	FLAGS            m_uiFlags;              // 0x3c
	TFLOAT           m_fDistanceTriggerSq;   // 0x40, "distancetrigger" squared
	Toshi::TPString8 m_strPlayerRespawnGoal; // 0x44, "playerrespawngoal"
	Toshi::TPString8 m_strLocator;           // 0x48, "locator"
};

TSTATICASSERT( sizeof( ASpawnActivity ) == 0x4c );
