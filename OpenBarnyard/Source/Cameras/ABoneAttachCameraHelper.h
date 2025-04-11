#pragma once
#include "ACameraHelper.h"
#include "Animation/AModel.h"

#include <Render/TAnimation.h>

class ARenderer;

class ABoneAttachCameraHelper
    : public ACameraHelper
{
public:
	TDECLARE_CLASS( ABoneAttachCameraHelper, ACameraHelper );

public:
	ABoneAttachCameraHelper();
	~ABoneAttachCameraHelper();

	//-----------------------------------------------------------------------------
	// ACameraHelper
	//-----------------------------------------------------------------------------
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual TBOOL Unk2( void* ) OVERRIDE;
	virtual TBOOL Unk3( void*, void* ) OVERRIDE;
	virtual void  OnCreate() OVERRIDE;
	virtual void  OnDestroy() OVERRIDE;
	virtual void  OnAttach() OVERRIDE;
	virtual void  OnDetach() OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own Methods
	//-----------------------------------------------------------------------------
	virtual void BoneHelper_Unknown();

	void SetModel( const Toshi::TPString8& a_strModelName, const TCHAR* a_szBoneName );
	void SetAnimation( const TCHAR* a_szSeqName );

private:
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
