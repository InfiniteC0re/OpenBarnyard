#pragma once
#include "ASSeqActorInterface.h"
#include "Animation/AModel.h"
#include "Render/ARenderer.h"

#include <Toshi/TEvent.h>
#include <Math/TVector4.h>

// Owned by ASSeqController (+0x70). Takes part in scripted sequences as an actor of its own
// (a model with an animation, set by the "parentNode_Config" command) and moves the actors
// attached to the bones of that model (the "attachToParentNode" command) every frame.
//
// TClass 0x007b4b2c (size 0x10c, runtime creatable), vtable 0x00737e0c,
// ASSeqActorInterface vtable 0x00737df0 (+0x4).
class ASSeqAttachmentManager
    : public Toshi::TObject
    , public ASSeqActorInterface
{
public:
	TDECLARE_CLASS( ASSeqAttachmentManager, Toshi::TObject );

	static constexpr TINT MAX_NUM_ATTACHMENTS = 25;

	struct Attachment
	{
		ASSeqActorInterface* pActor; // 0x0
		TINT                 iBone;  // 0x4
	};

public:
	ASSeqAttachmentManager();
	~ASSeqAttachmentManager();

	//-----------------------------------------------------------------------------
	// ASSeqActorInterface
	//-----------------------------------------------------------------------------
	virtual void  UpdateActor( TFLOAT a_fDeltaTime ) override {} // +0x00, DOES_NOTHING 00628f40
	virtual TBOOL OnSequenceEvent( TINT a_iEvent, const void* a_pEventData ) override;
	virtual TBOOL SetActive( TBOOL a_bActive, ASSeq* a_pSSeq, TINT a_iUnused ) override;
	virtual TBOOL UpdateEvent( TINT a_iEvent, const void* a_pEventData, TFLOAT a_fDeltaTime ) override;
	virtual void  StopSequenceAnimation() override;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Detaches a_pActor from the model (the "detachFromParentNode" command)
	void RemoveAttachment( ASSeqActorInterface* a_pActor );

	// Destroys the model and detaches every actor
	void Reset();

	// $Barnyard: FUNCTION 00440de0
	void SetPosition( const Toshi::TVector4& a_rcPosition ) { m_vecPosition = a_rcPosition; }

	AModelInstance* GetModelInstance() const { return m_pModelInstance; }

private:
	// Starts the animation of an EVENT_ANIMATION event
	void PlayAnimation( const SequenceEvent* a_pEvent );

	// Returns TTRUE once the animation is over (a_pEventData +0x14: number of loops)
	TBOOL UpdateAnimation( const void* a_pEventData, TFLOAT a_fDeltaTime );

	// Updates the skeleton and moves the attached actors to their bones
	TBOOL OnAnimationUpdateStart( ARenderer* a_pRenderer, TBOOL** a_ppData );

	static TBOOL __stdcall AnimationUpdateStartListener( ASSeqAttachmentManager* a_pCaller, ARenderer* a_pOwner, TBOOL** a_ppData );

private:
	// 0x4: ASSeqActorInterface
	ASSeq*                                                      m_pSSeq;                               // 0x10, set by SetActive
	AModelInstance*                                             m_pModelInstance;                      // 0x14
	Attachment                                                  m_aAttachments[ MAX_NUM_ATTACHMENTS ]; // 0x18
	TINT                                                        m_iNumAttachments;                     // 0xe0
	Toshi::TAnimation*                                          m_pAnimation;                          // 0xe4
	Toshi::TListener<ARenderer, TBOOL*, ASSeqAttachmentManager> m_oAnimUpdateStartListener;            // 0xe8
	Toshi::TVector4                                             m_vecPosition;                         // 0xfc
};

TSTATICASSERT( sizeof( ASSeqAttachmentManager ) == 0x10c );
