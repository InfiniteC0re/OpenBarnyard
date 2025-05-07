#pragma once
#include "ACameraHelper.h"
#include "Animation/AModel.h"

#include <Render/TAnimation.h>

class ARenderer;

//-----------------------------------------------------------------------------
// ABoneAttachCameraHelper
//-----------------------------------------------------------------------------
// Helper class to attach a camera to a specific bone of a model.
// This class allows a camera to follow a bone in a model, updating its position and orientation
// according to the bone's transform. It also supports setting animations and handling animation events.
class ABoneAttachCameraHelper
    : public ACameraHelper
{
public:
	TDECLARE_CLASS( ABoneAttachCameraHelper, ACameraHelper );

public:
	// Constructs a new ABoneAttachCameraHelper object.
	ABoneAttachCameraHelper();

	// Destroys the ABoneAttachCameraHelper object and detaches from the model/bone.
	~ABoneAttachCameraHelper();

	//-----------------------------------------------------------------------------
	// ACameraHelper
	//-----------------------------------------------------------------------------
	// Updates the camera's position and orientation based on the attached bone.
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;

	// Unknown override method (purpose not documented).
	virtual TBOOL Unk2( void* ) OVERRIDE;

	// Unknown override method (purpose not documented).
	virtual TBOOL Unk3( void*, void* ) OVERRIDE;

	// Called when the helper is created.
	virtual void OnCreate() OVERRIDE;

	// Called when the helper is destroyed.
	virtual void OnDestroy() OVERRIDE;

	// Called when the helper is attached to a camera.
	virtual void OnAttach() OVERRIDE;

	// Called when the helper is detached from a camera.
	virtual void OnDetach() OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own Methods
	//-----------------------------------------------------------------------------
	// Unknown method (purpose not documented).
	virtual void BoneHelper_Unknown();

	// Sets the model and bone to which the camera will be attached.
	void SetModel( const Toshi::TPString8& a_strModelName, const TCHAR* a_szBoneName );

	// Sets the animation sequence for the attached model.
	void SetAnimation( const TCHAR* a_szSeqName );

private:
	// Handles animation start events for the attached model.
	static TBOOL __stdcall AnimationStartEventHandler( ABoneAttachCameraHelper* a_pCaller, ARenderer* a_pOwner, TBOOL** a_pData );

private:
	Toshi::TAnimation*                                           m_pAnimation;
	AModelInstance*                                              m_pAModelInstance;
	Toshi::TSceneObject*                                         m_pSceneObject;
	Toshi::TSkeletonInstance*                                    m_pSkeletonInstance;
	TINT                                                         m_iBoneId;
	Toshi::TMatrix44                                             m_oTransform;
	Toshi::TListener<ARenderer, TBOOL*, ABoneAttachCameraHelper> m_oAnimStartListener;
	AAnimatableObject*                                           m_pAnimatableObject;
};
