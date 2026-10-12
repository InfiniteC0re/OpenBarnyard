#pragma once
#include "ACameraHelper.h"
#include "ACameraHelperSpline.h"

//-----------------------------------------------------------------------------
// Purpose: Camera of the NPC conversations, moves along a spline to a position that
// shows the target and the unit it talks to (ACameraManager::CAMERAHELPER_CONVERSATION).
//
// TClass 0x00782418 (size 0x94), vtable 0x0071cd40.
//-----------------------------------------------------------------------------
class AConversationCameraHelper
    : public ACameraHelper
{
public:
	TDECLARE_CLASS( AConversationCameraHelper, ACameraHelper );

public:
	AConversationCameraHelper();
	~AConversationCameraHelper();

	//-----------------------------------------------------------------------------
	// ACameraHelper
	//-----------------------------------------------------------------------------
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;                       // +0x10
	virtual TBOOL Unk2( void* ) override { return TFALSE; }                       // +0x14, RETURNS_FALSE 006ce000
	virtual TBOOL ProcessInputCommand( void*, void* ) override { return TFALSE; } // +0x18, RETURNS_FALSE 00556a00
	virtual void  OnCreate() override;                                            // +0x1c
	virtual void  OnDestroy() override {}                                         // +0x20, DOES_NOTHING 005acd80
	virtual void  OnAttach() override {}                                          // +0x24, DOES_NOTHING 005acd80

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void ConversationHelper_Unknown() {} // +0x44, DOES_NOTHING 005acd80

private:
	ACameraHelperSpline     m_oSpline;      // 0x10
	Toshi::TRef<AUnitProxy> m_pOtherTarget; // 0x90
};

TSTATICASSERT( sizeof( AConversationCameraHelper ) == 0x94 );
