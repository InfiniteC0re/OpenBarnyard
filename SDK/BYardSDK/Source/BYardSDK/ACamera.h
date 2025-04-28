#pragma once
#include "THookedSingleton.h"

#include <Toshi/TTask.h>
#include <Math/TMatrix44.h>

struct ACamera
    : public Toshi::TObject
{
public:
	static const Toshi::TVector4 sm_vWorldUp;

public:
	virtual ~ACamera();

	Toshi::TVector4 GetOrientation() const;

	void LookAtPoint( const Toshi::TVector4& a_rPoint );
	void LookAtDirection( const Toshi::TVector4& a_rDirection );

public:
	Toshi::TMatrix44 m_Matrix;
	TFLOAT           m_fFOV;
	TFLOAT           m_fProjectionCentreX;
	TFLOAT           m_fProjectionCentreY;
};

class ACameraHelper
    : public Toshi::TObject
{
public:
	virtual void   UpdateCurrentCamera()                                      = 0;
	virtual TBOOL  OnUpdate( TFLOAT a_fDeltaTime )                            = 0;
	virtual TBOOL  Unk2( void* )                                              = 0;
	virtual TBOOL  Unk3( void*, void* )                                       = 0;
	virtual void   OnCreate()                                                 = 0;
	virtual void   OnDestroy()                                                = 0;
	virtual void   OnAttach()                                                 = 0;
	virtual void   GetCameraRotation( Toshi::TVector4& a_rOutRotation ) const = 0;
	virtual void   Unk7( void* )                                              = 0;
	virtual void   Unk8( void* )                                              = 0;
	virtual void   Unk9( void* )                                              = 0;
	virtual void   Unk10( void* )                                             = 0;
	virtual void   OnDetach()                                                 = 0;
	virtual TFLOAT GetFOV() const                                             = 0;

	ACamera* GetCamera() { return m_pCamera; }
	void     SetCamera( ACamera* a_pCamera ) { m_pCamera = a_pCamera; }

public:
	void*    unk1;
	ACamera* m_pCamera;
	void*    unk2;
};

class ACameraManager
    : public Toshi::TTask
    , public THookedSingleton<ACameraManager, 0x007822e0>
{
public:
	static constexpr TUINT SPLITSCREEN_MAX_CAMERAS = 4;
	using CAMERAHELPER                             = TUINT32;
	enum CAMERAHELPER_ : CAMERAHELPER
	{
		CAMERAHELPER_BIKE,
		CAMERAHELPER_GOLF,
		CAMERAHELPER_VICTORY,
		CAMERAHELPER_FOLLOW,
		CAMERAHELPER_CONVERSATION,
		CAMERAHELPER_SSEQ,
		CAMERAHELPER_BONEATTACH,
		CAMERAHELPER_GTA,
		CAMERAHELPER_LERP,
		CAMERAHELPER_POOL,
		CAMERAHELPER_SPEECH,
		CAMERAHELPER_TRACKING,
		CAMERAHELPER_MULTICAMERA,
		CAMERAHELPER_PROJECTILE_FIRE,
		CAMERAHELPER_NUM_OF
	};

	using SPLITSCREEN_CAMERAHELPER = TUINT32;
	enum SPLITSCREEN_CAMERAHELPER_ : SPLITSCREEN_CAMERAHELPER
	{
		SPLITSCREEN_CAMERAHELPER_PROJECTILE_1,
		SPLITSCREEN_CAMERAHELPER_PROJECTILE_2,
		SPLITSCREEN_CAMERAHELPER_PROJECTILE_3,
		SPLITSCREEN_CAMERAHELPER_PROJECTILE_4,
		SPLITSCREEN_CAMERAHELPER_GTA_1,
		SPLITSCREEN_CAMERAHELPER_GTA_2,
		SPLITSCREEN_CAMERAHELPER_GTA_3,
		SPLITSCREEN_CAMERAHELPER_GTA_4,
		SPLITSCREEN_CAMERAHELPER_CAR_CHASE_1,
		SPLITSCREEN_CAMERAHELPER_CAR_CHASE_2,
		SPLITSCREEN_CAMERAHELPER_CAR_CHASE_3,
		SPLITSCREEN_CAMERAHELPER_CAR_CHASE_4,
		SPLITSCREEN_CAMERAHELPER_NUM_OF
	};

public:
	//-----------------------------------------------------------------------------
	// Own Methods
	//-----------------------------------------------------------------------------
	void SetCameraHelper( ACameraHelper* a_pHelper );
	void DetachCameraHelpers();

	ACamera*       GetCamera( TUINT a_uiCameraIndex ) const;
	ACameraHelper* GetCameraHelper( CAMERAHELPER a_eCameraHelper ) const;

	ACameraHelper* GetCurrentCameraHelper() const;
	ACamera*       GetCurrentCamera() const;

	void StopPositionLerp( TFLOAT a_flDeltaTime );

public:
	ACameraHelper*  m_apCameraHelpers[ CAMERAHELPER_NUM_OF ];
	ACameraHelper*  m_apSplitscreenCameraHelpers[ SPLITSCREEN_CAMERAHELPER_NUM_OF ];
	ACamera*        m_pCurrentCamera;
	ACamera*        m_apCameras[ SPLITSCREEN_MAX_CAMERAS ];
	ACameraHelper*  m_pCurrentHelper;
	ACameraHelper*  m_apCurrentSplitscreenCameraHelpers[ SPLITSCREEN_MAX_CAMERAS ];
	ACameraHelper*  m_pPreviousCameraHelper;
	Toshi::TVector4 m_vLerpPosFrom;
	Toshi::TVector4 m_vInitialLookDirection;
	TFLOAT          m_fLerpDuration;
	TFLOAT          m_fLerpTime;
	TFLOAT          m_fUnk;
	TBOOL           m_bIsPosLerping;
};
