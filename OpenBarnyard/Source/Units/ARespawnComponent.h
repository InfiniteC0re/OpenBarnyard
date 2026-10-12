#pragma once
#include "AUnitComponent.h"
#include "AUnitStaticData.h"

#include <Toshi/TPString8.h>
#include <Math/TVector4.h>

class ARespawnComponentData;

//-----------------------------------------------------------------------------
// Moves its unit back to the closest of the interior goals "<goal>01".."<goal>NN"
// (closest to the player) when the unit and that goal are both out of the
// player's sight. Checked every 3 seconds, followed by a 5 second cooldown
// after a respawn (or when the unit already stands at the goal).
// TClass version 1.0, size 0x20, vtable 0x00728bdc.
//-----------------------------------------------------------------------------
class ARespawnComponent
    : public AUnitComponent
{
public:
	TDECLARE_CLASS( ARespawnComponent, AUnitComponent );

	using FLAGS = TUINT32;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE     = 0,
		FLAGS_COOLDOWN = BITFLAG( 1 ), // counts m_fTimer down to 0
		FLAGS_CHECKING = BITFLAG( 2 ), // counts m_fTimer up and checks every 3 seconds
	};

public:
	ARespawnComponent();
	~ARespawnComponent();

	virtual AUnitComponentManager* GetManager() OVERRIDE;                    // +0x10
	virtual void                   UpdateAI( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x2c

	TBOOL Create( AUnit* a_pOwner, ARespawnComponentData* a_pData );

private:
	// Whether a_rcPosition is closer to the camera than a_fMinDistance, or closer than a_fMaxDistance and in front of the camera
	TBOOL IsSeenByCamera( const Toshi::TVector4& a_rcPosition, TFLOAT a_fMinDistance, TFLOAT a_fMaxDistance );

	// Finds the goal closest to a_rcPosition (XZ distance)
	TBOOL FindClosestGoal( const Toshi::TVector4& a_rcPosition, Toshi::TVector4& a_rOutPosition, Toshi::TVector4& a_rOutFacing );

private:
	FLAGS            m_eFlags;    // 0x10
	Toshi::TPString8 m_strGoal;   // 0x14, goal name prefix
	TFLOAT           m_fTimer;    // 0x18
	TINT             m_iNumGoals; // 0x1c, number of goals whose name contains m_strGoal
};

TSTATICASSERT( sizeof( ARespawnComponent ) == 0x20 );

//-----------------------------------------------------------------------------
// Static data that gives every unit of the type an ARespawnComponent.
// TClass version 1.0, size 0x10, vtable 0x00728bbc.
//-----------------------------------------------------------------------------
class ARespawnComponentData
    : public AUnitStaticData
{
public:
	TDECLARE_CLASS( ARespawnComponentData, AUnitStaticData );

	friend class ARespawnComponent;

public:
	ARespawnComponentData();
	~ARespawnComponentData();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual void  OnUnitCreate( AUnit* a_pUnit ) OVERRIDE;              // +0x10

private:
	Toshi::TPString8 m_strGoal; // 0x0c
};

TSTATICASSERT( sizeof( ARespawnComponentData ) == 0x10 );
