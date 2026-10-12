#pragma once
#include <Toshi/TPString8.h>
#include <Math/TVector4.h>

class AUnit;

//-----------------------------------------------------------------------------
// Purpose: throws coins (the "buck_1", "buck_5" and "buck_20" units) out of a
// position: when the player spills money (AInventoryActivity "spillmoney") and
// when a crate with coins breaks. The coins jump away from the player, spin,
// and fade out after a while unless they're picked up.
// NOTE: the original name is unknown, these are free __stdcall functions
// (0066e950-0066f2a0) placed after ASubtitleManager.
//-----------------------------------------------------------------------------
class ACoinSpawner
{
public:
	// Throws a coin of type a_rcType from a_rcPosition
	static void SpawnCoin( const Toshi::TPString8& a_rcType, const Toshi::TVector4& a_rcPosition );

	// Throws a_iNumBucks "buck_1" coins from a_rcPosition
	static void SpawnMoney( TINT a_iNumBucks, const Toshi::TVector4& a_rcPosition );

private:
	// Returns a random direction around the direction of the player, mostly along -Y
	static void GetRandomThrowDirection( Toshi::TVector4& a_rOutDirection );

	// Moves a_pCoin back out of the collision it was spawned in
	static void PushOutOfCollision( AUnit* a_pCoin );
};
