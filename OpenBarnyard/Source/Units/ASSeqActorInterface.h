#pragma once
#include <Math/TMatrix44.h>
#include <Render/TAnimation.h>

class ASSeq;

// Interface of units that take part in scripted sequences (SSeq), see AUnitActor and ASSeqAttachmentManager.
// Original name from the Wii/PS2 builds; the slots aren't understood yet, so most of
// them keep placeholder names. It has no virtual destructor.
//
// vtable 0x007187ec (only seen while AUnitActor's ctor runs).
class ASSeqActorInterface
{
public:
	enum STATE : TINT
	{
		STATE_IDLE    = 0,
		STATE_PLAYING = 3, // playing a sequence animation (AUnitActor::PlaySequenceAnimation)
	};

	// Data of the sequence events handled by OnSequenceEvent.
	// NOTE: original name and the rest of the layout are unknown.
	struct SequenceEvent
	{
		TCHAR        PADDING[ 0xc ];
		const TCHAR* szCommand;   // 0x0c, EVENT_COMMAND
		const TCHAR* szArgument1; // 0x10, animation name (EVENT_ANIMATION) or command argument
		const TCHAR* szArgument2; // 0x14, command argument
	};

	enum EVENT : TINT
	{
		EVENT_ANIMATION = 4,
		EVENT_COMMAND   = 12,
	};

public:
	ASSeqActorInterface()
	    : m_eState( STATE_IDLE )
	    , m_bActive( TFALSE )
	{}

	virtual void  UpdateActor( TFLOAT a_fDeltaTime )                                          = 0; // +0x00, called each frame by ASSeqController for some actors
	virtual TBOOL OnSequenceEvent( TINT a_iEvent, const void* a_pEventData )                  = 0; // +0x04
	virtual TBOOL SetActive( TBOOL a_bActive, ASSeq* a_pSSeq, TINT a_iUnused )                = 0; // +0x08
	virtual TBOOL UpdateEvent( TINT a_iEvent, const void* a_pEventData, TFLOAT a_fDeltaTime ) = 0; // +0x0c
	// $Barnyard: FUNCTION 00419570
	virtual TBOOL IsIdle() { return m_eState == STATE_IDLE; }              // +0x10
	virtual void  SetTransform( const Toshi::TMatrix44& a_rcTransform ) {} // +0x14, DOES_NOTHING 00628f40
	virtual void  StopSequenceAnimation() = 0;                             // +0x18

protected:
	// TTRUE if a_pAnimation is blending out and its weight reaches zero within a_fDeltaTime
	// NOTE: TAnimation method 006ca1d0, not in TAnimation yet
	static TBOOL IsAnimationBlendingOutDone( Toshi::TAnimation* a_pAnimation, TFLOAT a_fDeltaTime )
	{
		if ( !a_pAnimation->IsManaged() || a_pAnimation->GetState() != Toshi::TAnimation::STATE_BLENDING_OUT )
			return TFALSE;

		if ( a_pAnimation->GetBlendOutSpeed() == 0.0f )
			return TTRUE;

		return a_pAnimation->GetWeight() - ( 1.0f / a_pAnimation->GetBlendOutSpeed() ) * a_fDeltaTime <= 0.0f;
	}

protected:
	STATE m_eState;  // 0x04
	TBOOL m_bActive; // 0x08
};
