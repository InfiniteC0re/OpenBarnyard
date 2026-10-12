#pragma once
#include "AAnimatedUnit.h"
#include "ASSeqActorInterface.h"

#include <Render/TAnimation.h>

// A unit driven by the scripted sequence (SSeq) system; its type carries ASSeqActorData
// and AUnitManager::FindActorBySSeqId looks it up by that id.
//
// TClass 0x0077e888 (version 1.1, size 0xb0, runtime creatable), vtable 0x00718830,
// AGoalCollisionUpdateInterface vtable 0x00718824 (+0x40), ASSeqActorInterface vtable 0x00718808 (+0x98).
class AUnitActor
    : public AAnimatedUnit
    , public ASSeqActorInterface
{
public:
	TDECLARE_CLASS( AUnitActor, AAnimatedUnit );

public:
	AUnitActor();
	~AUnitActor();

	//-----------------------------------------------------------------------------
	// AUnit
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( AUnitTransform* a_pTransform ) override; // +0x58
	virtual void  OnDestroy() override;                            // +0x60
	virtual void  UpdateAI( TFLOAT a_fDeltaTime ) override;        // +0x68
	virtual TBOOL Think( TFLOAT a_fDeltaTime ) override;           // +0x6c

	//-----------------------------------------------------------------------------
	// ASSeqActorInterface
	//-----------------------------------------------------------------------------
	virtual void  UpdateActor( TFLOAT a_fDeltaTime ) override;                                          // +0x00
	virtual TBOOL OnSequenceEvent( TINT a_iEvent, const void* a_pEventData ) override;                  // +0x04
	virtual TBOOL SetActive( TBOOL a_bActive, ASSeq* a_pSSeq, TINT a_iUnused ) override;                // +0x08
	virtual TBOOL UpdateEvent( TINT a_iEvent, const void* a_pEventData, TFLOAT a_fDeltaTime ) override; // +0x0c
	virtual void  SetTransform( const Toshi::TMatrix44& a_rcTransform ) override;                       // +0x14
	virtual void  StopSequenceAnimation() override;                                                     // +0x18, stops the sequence animation

	// Sets the sequence animation the actor plays (ASSeqUI plays ss01_gui_idle on the player)
	void SetAnimation( Toshi::TAnimation* a_pAnimation ) { m_pAnimation = a_pAnimation; }

private:
	// Starts the animation of an EVENT_ANIMATION event
	void PlaySequenceAnimation( const SequenceEvent* a_pEvent );

	// Returns TTRUE once the sequence animation is over (a_pEventData +0x14: number of loops)
	TBOOL UpdateSequenceAnimation( const void* a_pEventData, TFLOAT a_fDeltaTime );

private:
	// 0x98: ASSeqActorInterface
	ASSeq*             m_pSSeq;              // 0xa4, set by SetActive
	Toshi::TAnimation* m_pAnimation;         // 0xa8, sequence animation
	TBOOL              m_bForceAlwaysUpdate; // 0xac, "forceAllwaysUpdate" command
	TBOOL              m_bSkeletonActive;    // 0xad, set by SetActive( TTRUE )
};

TSTATICASSERT( sizeof( AUnitActor ) == 0xb0 );
