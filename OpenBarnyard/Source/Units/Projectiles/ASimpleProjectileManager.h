#pragma once
#include <Toshi/TTask.h>
#include <Toshi/TSingleton.h>
#include <Math/TVector4.h>

#include "FSM/AFunctionTable.h"
#include "GUI/AGUI2TargetIndicator.h"
#include "GUI/AGUI2StarRating.h"

class AProjectile;

// Owns the pools of projectiles (one per projectile type) and updates the
// projectiles in use. AProjectileLauncher fetches its projectiles from here.
// Can also track the oldest projectile for the distance HUD of the minigames.
//
// Singleton at 0x007cd368, vtable 0x0073b7b8.
class ASimpleProjectileManager
    : public Toshi::TTask
    , public Toshi::TSingleton<ASimpleProjectileManager>
{
public:
	TDECLARE_CLASS( ASimpleProjectileManager, Toshi::TTask );

	static constexpr TINT NUM_PROJECTILE_TYPES = 11;

	// The pools of the projectiles (see CreateProjectile, ALaunchAmmo::SetProjectileType)
	using TYPE = TINT;
	enum TYPE_ : TYPE
	{
		TYPE_SIM_MILK                    = 0, // ASimMilkProjectile
		TYPE_VEGGIE_DEFENDER_MILK        = 1, // AVeggieDefenderMilkProjectile
		TYPE_WITH_UNIT                   = 2, // AProjectileWithUnit
		TYPE_VEGGIE_THROW                = 3, // AVeggieThrowProjectile
		TYPE_CHASING_CHICKS_THROW        = 4, // AChasingChicksThrowProjectile
		TYPE_CHICKEN_COOP_DEFENDER_THROW = 5, // AChickenCoopDefenderThrowProjectile
		TYPE_UNK6                        = 6, // not created
		TYPE_MILK_PRECISION              = 7, // AMilkPrecisionProjectile
		TYPE_CHICKEN_COOP_DEFENDER_MILK  = 8, // AChickenCoopDefenderMilkProjectile
		TYPE_CHASING_CHICKS_MILK         = 9, // AChasingChicksMilkProjectile
	};

public:
	ASimpleProjectileManager();
	~ASimpleProjectileManager();

	//-----------------------------------------------------------------------------
	// Toshi::TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL OnCreate() OVERRIDE { return TTRUE; } // +0x18, RETURNS_TRUE (0046cde0)
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual AFunctionTable* GetFunctionTable(); // +0x38
	virtual void            DestroyAllPools();  // +0x3c

	// Creates the pools used by the player's launchers (25 projectiles of type 0 and 10 of type 2).
	TBOOL CreateDefaultPools();

	// (Re)creates the pool of a_iType projectiles with a_iSize projectiles.
	TBOOL CreatePool( TINT a_iSize, TINT a_iType );

	void DestroyPool( TINT a_iType );

	// Takes a free projectile of a_iType from its pool (resetting it).
	// If every projectile is in use, the first one is reused.
	AProjectile* GetProjectile( TINT a_iType );

	// Gives a_pProjectile back to its pool.
	void ReleaseProjectile( AProjectile* a_pProjectile );

	// Returns the projectile flying closest to a_rcPoint (within a_fMaxDistance) or TNULL.
	AProjectile* FindClosestProjectile( const Toshi::TVector4& a_rcPoint, TFLOAT a_fMaxDistance );

	// Starts tracking the projectiles (shows the distance HUD, a_iRecord is the high score record of the distance).
	void StartTracking( TINT a_iRecord );
	void StopTracking();

private:
	// Creates a new projectile of a_iType.
	AProjectile* CreateProjectile( TINT a_iType );

	void UpdateTracking( TFLOAT a_fDeltaTime, TBOOL a_bHasNewRecord );

public:
	static AFunctionTable ms_oFunctionTable;

private:
	// Time since the tracked record has last changed (0x007cd36c)
	inline static TFLOAT ms_fTrackingTimer;

private:
	AGUI2TargetIndicator m_oHUDElement1;                        // 0x20, distance of the tracked projectile
	AGUI2StarRating      m_oStarRating;                         // 0x270, rating of the distance
	AProjectile**        m_apPools[ NUM_PROJECTILE_TYPES ];     // 0x6ac
	TINT                 m_aiPoolSizes[ NUM_PROJECTILE_TYPES ]; // 0x6d8
	TINT                 m_aiNumUsed[ NUM_PROJECTILE_TYPES ];   // 0x704, every projectile at or after this index is free
	TUINT8               PADDING[ 0x2c ];                       // 0x730
	TFLOAT               m_fTrackedDistance;                    // 0x75c
	TFLOAT               m_fTrackedAge;                         // 0x760, age of the oldest tracked projectile
	TFLOAT               m_fRecordDistance;                     // 0x764, best score of m_iTrackedRecord
	Toshi::TVector4      m_vTrackedVelocity;                    // 0x768
	Toshi::TVector4      m_vTrackedPosition;                    // 0x778
	TBOOL                m_bTracking;                           // 0x788
	TBOOL                m_bNewRecord;                          // 0x789, m_fRecordDistance was beaten
	TINT                 m_iTrackedRecord;                      // 0x78c, high score record of the distance
};

TSTATICASSERT( sizeof( ASimpleProjectileManager ) == 0x790 );
