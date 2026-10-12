#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TSingleton.h>
#include <Toshi/T2ObjectPool.h>

#include "ASteer.h"

//-----------------------------------------------------------------------------
// Purpose: Allocates every unit steer. Steers smaller than 0x120 bytes come from
// a pool of fixed 0x130 byte slots, bigger ones from a pool sized for the
// biggest ASteer subclass.
//-----------------------------------------------------------------------------
class ASteerPool
    : public Toshi::TObject
    , public Toshi::TSingleton<ASteerPool>
{
public:
	TDECLARE_CLASS( ASteerPool, Toshi::TObject );

	static constexpr TINT MAX_NUM_STEERS       = 320;
	static constexpr TINT STEER_SLOT_SIZE      = 0x130;
	static constexpr TINT STEER_SLOT_ALIGNMENT = 16;
	static constexpr TINT MAX_NUM_LARGE_STEERS = 40;
	static constexpr TINT LARGE_STEER_SIZE     = 0x120; // steers of this size and bigger use the large pool

public:
	ASteerPool();
	~ASteerPool();

	// Allocates a steer of the specified ASteer subclass
	ASteer* CreateSteer( Toshi::TClass* a_pClass );

	// Destroys a steer created by CreateSteer
	void DestroySteer( ASteer* a_pSteer );

private:
	Toshi::T2DynamicObjectPool<ASteer>        m_oSteerPool;      // 0x04
	Toshi::T2DerivedDynamicObjectPool<ASteer> m_oLargeSteerPool; // 0x1c
};

TSTATICASSERT( sizeof( ASteerPool ) == 0x34 );
