#pragma once
#include "ASimAnimModelHelper.h"

#include <Math/TMatrix44.h>

class AAnimatableObject;

//-----------------------------------------------------------------------------
// Purpose: Spins the ceiling fans of the barn (every object whose type name
// contains "fan"). The fans are split by height, only the set of the floor
// the player is on (ABarnExploreMinigameState::ms_iFloor) is animated.
// Created by ABarnExploreMinigameState::OnInsertion at night.
// NOTE: the original name is unknown. Size 0x270, vtable 0x00723edc.
//-----------------------------------------------------------------------------
class ABarnFanHelper
    : public ASimAnimModelHelper
{
public:
	static constexpr TINT MAX_NUM_FANS = 4;

	struct Fan
	{
		Fan();

		void Create( AAnimatableObject* a_pObject, const Toshi::TMatrix44& a_rcTransform );
		void Update( TFLOAT a_fDeltaTime, TBOOL a_bCheckFloor );

		AAnimatableObject* pObject;   // 0x00
		TFLOAT             fAngle;    // 0x04
		TFLOAT             fSpeed;    // 0x08
		Toshi::TMatrix44   matOrigin; // 0x0C
	};

public:
	// $Barnyard: FUNCTION 0053f930
	ABarnFanHelper()
	    : m_iNumLowerFans( 0 )
	    , m_iNumUpperFans( 0 )
	    , m_bCheckFloor( TTRUE )
	{}

	//-----------------------------------------------------------------------------
	// ASimAnimModelHelper
	//-----------------------------------------------------------------------------
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnDestroy() OVERRIDE {} // DOES_NOTHING

	//-----------------------------------------------------------------------------
	// Own Methods
	//-----------------------------------------------------------------------------

	// Adds the fan a_pObject placed at a_rcTransform, to the lower or the upper set depending on its height
	void AddFan( AAnimatableObject* a_pObject, const Toshi::TMatrix44& a_rcTransform );

private:
	TINT  m_iNumLowerFans;              // 0x004
	Fan   m_aLowerFans[ MAX_NUM_FANS ]; // 0x008, y <= 0
	TINT  m_iNumUpperFans;              // 0x138
	Fan   m_aUpperFans[ MAX_NUM_FANS ]; // 0x13C, y > 0
	TBOOL m_bCheckFloor;                // 0x26C, skips the fans on the other side of y = 0 from the camera
};

TSTATICASSERT( sizeof( ABarnFanHelper ) == 0x270 );
