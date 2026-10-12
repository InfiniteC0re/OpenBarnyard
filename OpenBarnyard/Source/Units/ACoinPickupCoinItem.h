#pragma once
#include "AAnimatedUnit.h"

#include <Math/TVector4.h>

class ATrackBoneAniEffect;

//-----------------------------------------------------------------------------
// A coin of the coin pickups (unit type "precisioncoin"), spawned by
// ACoinTrailActivity and ACoinPickupMicroGame. It spins in place; once it's
// collected it either flies to its target (the player) and fades out, or, with
// m_bUseTrackEffect, an ATrackBoneAniEffect moves it to the target's spine.
//
// TClass 0x00783abc (parent AAnimatedUnit, size 0xc8), vtables 0x00723868, 0x00721b7c.
//-----------------------------------------------------------------------------
class ACoinPickupCoinItem
    : public AAnimatedUnit
{
public:
	TDECLARE_CLASS( ACoinPickupCoinItem, AAnimatedUnit );

public:
	ACoinPickupCoinItem();
	~ACoinPickupCoinItem();

	//-----------------------------------------------------------------------------
	// AUnit
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( AUnitTransform* a_pTransform ) OVERRIDE; // +0x58
	virtual void  OnDestroy() OVERRIDE;                            // +0x60
	virtual void  UpdateAI( TFLOAT a_fDeltaTime ) OVERRIDE;        // +0x68

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// TTRUE: the coin is collected and flies to the target.
	// TFALSE: puts the coin back at m_vStartPosition.
	void SetCollected( TBOOL a_bCollected );

	// The unit collecting the coin
	void SetTarget( AUnit* a_pTarget );

	// Destroys the coin unit
	void Destroy();

	// NOTE: folded setter of a field at +0xa4 (00502a20)
	void SetValue( TINT a_iValue ) { m_iValue = a_iValue; }

	void SetStartPosition( const Toshi::TVector4& a_rcPosition ) { m_vStartPosition = a_rcPosition; }

	TBOOL IsCollected() const { return m_bCollected; }
	TBOOL IsCollectDone() const { return m_bCollectDone; }

	// Not handed to the inventory yet (ACoinPickupMicroGame clears it once the coin reached the player)
	TBOOL IsPendingCollect() const { return m_bPendingCollect; }
	void  SetPendingCollect( TBOOL a_bValue ) { m_bPendingCollect = a_bValue; }

	// The value of the coin (ACoinPickupMicroGame adds it to its score)
	TINT GetValue() const { return m_iValue; }

	void SetFlashing( TBOOL a_bFlashing ) { m_bFlashing = a_bFlashing; }
	void SetUseTrackEffect( TBOOL a_bUseTrackEffect ) { m_bUseTrackEffect = a_bUseTrackEffect; }

private:
	AUnit*               m_pTarget;         // 0x98
	TBOOL                m_bCollected;      // 0x9c
	TBOOL                m_bCollectDone;    // 0x9d, the track effect is over (also stops the spinning)
	TBOOL                m_bPendingCollect; // 0x9e
	TFLOAT               m_fOpacity;        // 0xa0
	TINT                 m_iValue;          // 0xa4, ACoinTrailActivity sets it to 1
	TBOOL                m_bFlashing;       // 0xa8, the opacity pulses
	TBOOL                m_bUseTrackEffect; // 0xa9
	TFLOAT               m_fTime;           // 0xac
	ATrackBoneAniEffect* m_pTrackEffect;    // 0xb0
	Toshi::TVector4      m_vStartPosition;  // 0xb4
	TINT                 m_iUnkC4;          // 0xc4
};

TSTATICASSERT( sizeof( ACoinPickupCoinItem ) == 0xc8 );
