#pragma once
#include "AAnimatedUnit.h"

#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// A target of the Sharp Squirter minigame (unit type "precisiontarget", a tin
// can). APrecisionSquirt creates 50 of them and places the ones of the current
// round at the "target" goals. They spin in place; when the milk hits one it
// flies off spinning and fades out.
// TClass 007092b0 (parent AAnimatedUnit, size 0xbc), vtables 00721e50, 00721b7c.
//-----------------------------------------------------------------------------
class APrecisionTargetItem
    : public AAnimatedUnit
{
public:
	TDECLARE_CLASS( APrecisionTargetItem, AAnimatedUnit );

public:
	APrecisionTargetItem();
	~APrecisionTargetItem();

	//-----------------------------------------------------------------------------
	// AUnit
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( AUnitTransform* a_pTransform ) OVERRIDE; // +0x58
	virtual void  UpdateAI( TFLOAT a_fDeltaTime ) OVERRIDE;        // +0x68

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Places the target at a_rcPosition (where it goes back to when it's reset)
	void SetPosition( const Toshi::TVector4& a_rcPosition );

	// TTRUE: the milk of the squirt a_iStreamId has hit the target, it flies away from the player.
	// TFALSE: puts the target back at its position (it fades in).
	void SetHit( TBOOL a_bHit, TINT a_iStreamId );

	// a_bKeep TFALSE stops the target at its position and hides it, TTRUE does nothing
	void Park( TBOOL a_bKeep );

	// TTRUE once for every hit (the hit is counted by APrecisionSquirt)
	TBOOL ConsumeHit();

	// The target is updated and rendered only while it's active
	void SetActive( TBOOL a_bActive );

	// $Barnyard: FUNCTION 00507970
	void SetPlayer( AUnit* a_pPlayer ) { m_pPlayer = a_pPlayer; }

	// NOTE: folded with a CRT function (00507940)
	TBOOL IsHit() const { return m_bHit; }

	// NOTE: folded with a CRT function (00507980)
	TINT GetStreamId() const { return m_iStreamId; }

	TBOOL IsActive() const { return m_bActive; }

public:
	TBOOL           m_bHit;      // 0x98
	TBOOL           m_bCounted;  // 0x99
	TBOOL           m_bActive;   // 0x9a
	TFLOAT          m_fOpacity;  // 0x9c
	AUnit*          m_pPlayer;   // 0xa0
	Toshi::TVector4 m_vPosition; // 0xa4
	TINT            m_iStreamId; // 0xb4, AMilkProjectile::m_iStreamId of the squirt that hit it
	TINT            m_iMapIcon;  // 0xb8
};

TSTATICASSERT( sizeof( APrecisionTargetItem ) == 0xbc );
