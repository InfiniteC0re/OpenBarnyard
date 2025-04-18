#pragma once
#include "ACameraHelper.h"

#include <Toshi/TTask.h>

class ACameraManager : public Toshi::TTask
    , public Toshi::TSingleton<ACameraManager>
{
public:
	TDECLARE_CLASS( ACameraManager, Toshi::TTask );

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
	ACameraManager();
	~ACameraManager();

	//-----------------------------------------------------------------------------
	// Toshi::TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;

	//-----------------------------------------------------------------------------
	// Own Methods
	//-----------------------------------------------------------------------------
	void SetCameraHelper( ACameraHelper* a_pHelper );

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
	// ...

	TBOOL m_bIsPosLerping;
};
