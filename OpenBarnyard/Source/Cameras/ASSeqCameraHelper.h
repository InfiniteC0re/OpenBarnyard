#pragma once
#include "ACameraHelper.h"
#include "Animation/AModel.h"
#include "Units/ASSeqActorInterface.h"

#include <Toshi/TEvent.h>
#include <Toshi/TPString8.h>
#include <Render/TAnimation.h>

class ARenderer;
class ASSeq;

//-----------------------------------------------------------------------------
// Camera of the scripted sequences (ACameraManager::CAMERAHELPER_SSEQ). It's an actor of
// the sequence: the camera follows a bone ("bindCamera" command) of an animated camera model.
//
// TClass 0x007825dc (size 0x88), vtable 0x0071d2b8, ASSeqActorInterface vtable 0x0071d298 (+0x10).
//-----------------------------------------------------------------------------
class ASSeqCameraHelper
    : public ACameraHelper
    , public ASSeqActorInterface
{
public:
	TDECLARE_CLASS( ASSeqCameraHelper, ACameraHelper );

public:
	ASSeqCameraHelper();
	~ASSeqCameraHelper();

	//-----------------------------------------------------------------------------
	// ACameraHelper
	//-----------------------------------------------------------------------------
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;                      // +0x10
	virtual TBOOL Unk2( void* ) override { return TTRUE; }                       // +0x14, RETURNS_TRUE 00413920
	virtual TBOOL ProcessInputCommand( void*, void* ) override { return TTRUE; } // +0x18, RETURNS_TRUE 00558f80
	virtual void  OnCreate() override {}                                         // +0x1c, DOES_NOTHING 005acd80
	virtual void  OnDestroy() override {}                                        // +0x20, DOES_NOTHING 005acd80
	virtual void  OnAttach() override {}                                         // +0x24, DOES_NOTHING 005acd80
	virtual void  OnDetach() override;                                           // +0x3c

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
	virtual void SSeqHelper_Unknown() {} // +0x44, DOES_NOTHING 005acd80

	// Creates the camera model (a_szModelName with its path) of the sequence a_pSSeq at a_rcPosition
	void Setup( const TCHAR* a_szModelName, ASSeq* a_pSSeq, const Toshi::TVector4& a_rcPosition );

	// Terrain VIS group callback used while the camera is active (see the "SetTerrain" command)
	static TINT GetVISGroup();

	void SetUseTarget( TBOOL a_bUseTarget ) { m_bUseTarget = a_bUseTarget; }

private:
	// Moves the camera model by the offset of an INSTRUCTION_ACTOREVENT0 event
	void SetOffset( const void* a_pEventData );

	// Starts the camera animation of an EVENT_ANIMATION event
	void PlayAnimation( const SequenceEvent* a_pEvent );

	// Moves the camera to its bone, returns TTRUE once the animation is over
	TBOOL UpdateAnimation( const void* a_pEventData, TFLOAT a_fDeltaTime );

	static TBOOL __stdcall AnimationUpdateStartListener( ASSeqCameraHelper* a_pCaller, ARenderer* a_pOwner, TBOOL** a_ppData );

private:
	inline static const TCHAR* ms_szTerrain = TNULL; // 0x007825d8, "SetTerrain" command

private:
	// 0x10: ASSeqActorInterface
	ASSeq*                                                 m_pSSeq;                    // 0x1c
	AModelInstance*                                        m_pModelInstance;           // 0x20
	Toshi::TSkeletonInstance*                              m_pSkeletonInstance;        // 0x24
	Toshi::TAnimation*                                     m_pAnimation;               // 0x28
	Toshi::TVector4                                        m_vecPosition;              // 0x2c
	Toshi::TVector4                                        m_vecTarget;                // 0x3c
	TINT                                                   m_iCameraBone;              // 0x4c
	TINT                                                   m_iTargetBone;              // 0x50
	Toshi::TListener<ARenderer, TBOOL*, ASSeqCameraHelper> m_oAnimUpdateStartListener; // 0x54
	Toshi::TPString8                                       m_strModelName;             // 0x68
	TFLOAT                                                 m_fSavedFOV;                // 0x6c
	TFLOAT                                                 m_fFOV;                     // 0x70
	TFLOAT                                                 m_fTargetFOV;               // 0x74, -1.0 if the FOV isn't animated
	TFLOAT                                                 m_fFOVSpeed;                // 0x78
	TFLOAT                                                 m_fSavedNear;               // 0x7c
	TINT                                                   m_iUnk;                     // 0x80
	TBOOL                                                  m_bUseTarget;               // 0x84, the camera looks at m_iTargetBone
};

TSTATICASSERT( sizeof( ASSeqCameraHelper ) == 0x88 );
