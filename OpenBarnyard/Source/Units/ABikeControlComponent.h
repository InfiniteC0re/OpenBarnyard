#pragma once
#include "AControlComponent.h"

#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// Purpose: Control component of the unit riding a bike (ABikeManager makes it
// the player unit). Turns the input into the ABikeSteer input (SetDesiredVelocity),
// rings the horn, starts the tricks and gets the rider off the bike.
// Also picks up the items the bike rides over (AutoPickup).
// TClass 0x0078a024, vtable 0x00728698, size 0x44.
//-----------------------------------------------------------------------------
class ABikeControlComponent
    : public AControlComponent
{
public:
	TDECLARE_CLASS( ABikeControlComponent, AControlComponent );

public:
	ABikeControlComponent();
	~ABikeControlComponent();

	//-----------------------------------------------------------------------------
	// AUnitComponent
	//-----------------------------------------------------------------------------
	virtual void UpdateAI( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x2c

	//-----------------------------------------------------------------------------
	// AControlComponent
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessCommand( AInputCommand a_eCommand, const Toshi::TInputInterface::InputEvent* a_pEvent ) OVERRIDE; // +0x3c
	virtual void  OnDisable() OVERRIDE;                                                                                    // +0x40

#ifdef BARNYARD_COMMUNITY_PATCH
	// Dev: -devbike rides the bike without input (steering, throttle)
	static void SetDevInput( TFLOAT a_fSteering, TFLOAT a_fThrottle )
	{
		ms_bDevInput    = TTRUE;
		ms_fDevSteer    = a_fSteering;
		ms_fDevThrottle = a_fThrottle;
	}
#endif

private:
#ifdef BARNYARD_COMMUNITY_PATCH
	inline static TBOOL  ms_bDevInput    = TFALSE;
	inline static TFLOAT ms_fDevSteer    = 0.0f;
	inline static TFLOAT ms_fDevThrottle = 0.0f;
#endif

	// Picks up the items around the bike
	void UpdatePickUps( TFLOAT a_fDeltaTime );

private:
	Toshi::TVector4 m_vecInput;     // 0x1c, see ABikeSteer::SetDesiredVelocity
	TBOOL           m_bHorn;        // 0x2c
	TBOOL           m_bUnk2D;       // 0x2d
	TBOOL           m_bTrickLeft;   // 0x2e
	TBOOL           m_bTrickRight;  // 0x2f
	TBOOL           m_bTrick;       // 0x30
	TINT            m_iUnk34;       // 0x34
	TBOOL           m_bBoost;       // 0x38
	TINT            m_iUnk3C;       // 0x3c
	TFLOAT          m_fPickUpTimer; // 0x40
};

TSTATICASSERT( sizeof( ABikeControlComponent ) == 0x44 );
