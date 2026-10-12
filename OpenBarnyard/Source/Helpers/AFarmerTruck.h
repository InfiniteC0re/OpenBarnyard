#pragma once
#include "Units/AUnitProxy.h"

#include <Toshi/TRefCounted.h>
#include <Math/TVector4.h>

class ACollisionObjectMesh;

//-----------------------------------------------------------------------------
// Purpose: the farmer's truck (actor_ss_truck_ntrailer) parked at the farm
// (simtruckgoal of the park2 network) during the day, and the state of the sim
// while the farmer arrives or leaves (the farmer_arive/farmer_depart, SSNBO and
// SSNBC sequences of ASimState::UpdateDailyEvents).
// NOTE: the retail name isn't known. It's a plain 0x20 byte structure that
// ASimState allocates (DAT_00783a84), its functions are free functions.
//-----------------------------------------------------------------------------
class AFarmerTruck
{
public:
	static constexpr TINT NUM_HIDDEN_CHARACTERS = 16;

public:
	// Creates the truck, it's parked if a_bParked or hidden otherwise
	static TBOOL Create( TBOOL a_bParked );

	// Despawns and frees the truck
	static void Destroy();

	// Hides the characters (off_00773f18) and the player during a sequence and moves the player to farmersafepos,
	// or shows them again and moves the player back where it was
	static void SetSequenceMode( TBOOL a_bSequence );

	// Moves the player to farmersafepos (only if it's within 4 units of simtruckgoal if a_bOnlyIfOnTruckGoal)
	static void MovePlayerToSafePos( TBOOL a_bOnlyIfOnTruckGoal );

	// Places the truck at simtruckgoal and shows it
	static void Park();

	// Hides the truck and disables its collision
	static void Hide();

	// Shows the truck
	static void Show();

	static TBOOL IsCreated() { return ms_pInstance != TNULL; }

private:
	// Loads the farmer sequences, spawns the truck and parks it
	static TBOOL Spawn();

	// Destroys the truck and its collision
	static void Despawn();

	static AUnit* GetTruck() { return ms_pInstance->m_pTruck ? ms_pInstance->m_pTruck->GetUnit() : TNULL; }

private:
	inline static AFarmerTruck* ms_pInstance = TNULL; // DAT_00783a84

	static const Toshi::TPString8* ms_apHiddenCharacters[ NUM_HIDDEN_CHARACTERS ]; // off_00773f18

private:
	Toshi::TRef<AUnitProxy> m_pTruck;               // 0x00
	ACollisionObjectMesh*   m_pCollision;           // 0x04
	TUINT32                 m_uiVisibleCharacters;  // 0x08, characters that were visible before SetSequenceMode( TTRUE )
	TBOOL                   m_bSequenceMode;        // 0x0c
	TUINT8                  m_uiUnk0D;              // 0x0d, 2 if the truck was hidden when it was created
	TBOOL                   m_bUnk0E;               // 0x0e
	Toshi::TVector4         m_vSavedPlayerPosition; // 0x10
};

TSTATICASSERT( sizeof( AFarmerTruck ) == 0x20 );
