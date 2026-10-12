#pragma once
#include <Toshi/T2SList.h>
#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// Purpose: A pool of MAX_DEBRIS flying instances (dynamic instances of
// AInstanceManager knocked away and tumbling until they hit the ground).
// Joy Ride owns one; it reserves MAX_DEBRIS dynamic instances of AInstanceManager.
// NOTE: the name is invented. The update of a piece (00539810) isn't referenced
// by the retail binary.
//-----------------------------------------------------------------------------
class AInstanceDebrisManager
{
public:
	static constexpr TINT MAX_DEBRIS = 50;

	struct Debris
	    : public Toshi::T2SList<Debris>::Node
	{
		// $Barnyard: FUNCTION 00539800
		Debris()
		    : pUnk08( TNULL )
		{}

		// Moves and rotates the instance, sweeps it against the collision world and stops it when it hits something.
		// NOTE: in retail this is the third slot of an unreferenced vtable (0x00723c5c, the other two slots
		// return TFALSE), so nothing calls it. Always returns TFALSE.
		TBOOL Update( TFLOAT a_fDeltaTime );

		TUINT8*         pInstance;  // 0x04, AInstanceManager::DynamicInstanceEntry (transform at +0xc, locator list at +0x4), not initialised
		void*           pUnk08;     // 0x08
		Toshi::TVector4 vVelocity;  // 0x0c
		TFLOAT          fRotSpeedX; // 0x1c
		TFLOAT          fRotSpeedY; // 0x20, also used as the fall speed
		TFLOAT          fRotSpeedZ; // 0x24
		TFLOAT          fUnk28;     // 0x28
	};

	// NOTE: the original has a separate constructor/destructor for this part (00539ab0/00539790)
	struct DebrisList
	{
		DebrisList();
		~DebrisList();

		TINT                   iNumUsed;              // 0x000
		Debris                 aDebris[ MAX_DEBRIS ]; // 0x004
		Toshi::T2SList<Debris> llFree;                // 0x89c
		Toshi::T2SList<Debris> llUsed;                // 0x8a0
	};

public:
	AInstanceDebrisManager();
	~AInstanceDebrisManager();

private:
	DebrisList m_oDebris; // 0x000
	TFLOAT     m_fUnk8a4; // 0x8a4, 1.0f
};

TSTATICASSERT( sizeof( AInstanceDebrisManager::Debris ) == 0x2c );
TSTATICASSERT( sizeof( AInstanceDebrisManager ) == 0x8a8 );
