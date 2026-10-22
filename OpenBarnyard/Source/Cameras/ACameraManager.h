#pragma once
#include "ACameraHelper.h"

#include <Toshi/TTask.h>

//-----------------------------------------------------------------------------
// Purpose: Owns the cameras (one per split screen player) and every camera helper.
// The current helper (and the split screen helpers) move the current camera every frame.
//
// TClass 0x007822e4 (size 0xe4), vtable 0x0071c918.
//-----------------------------------------------------------------------------
class MODAPI ACameraManager : public Toshi::TTask
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
	virtual TBOOL Reset() override;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;

	//-----------------------------------------------------------------------------
	// Own Methods
	//-----------------------------------------------------------------------------
	void SetCameraHelper( ACameraHelper* a_pHelper );

	// Makes m_apCameraHelpers[ a_eCameraHelper ] the current helper
	void SetCameraHelper( CAMERAHELPER a_eCameraHelper );

	// Makes the first player's helper of a split screen helper group (0 - projectile, 1 - GTA, 2 - car chase) the current helper
	void SetSplitscreenCameraHelper( TUINT a_uiGroup );

	// Detaches the current helper and the split screen helpers
	void DetachCameraHelpers();

	// Detaches the current split screen helpers
	void DetachSplitscreenCameraHelpers();

	ACamera*       GetCamera( TUINT a_uiCameraIndex ) const;
	ACameraHelper* GetCameraHelper( CAMERAHELPER a_eCameraHelper ) const;

	// Returns the first player's helper of a split screen helper group (0 - projectile, 1 - GTA, 2 - car chase)
	// $Barnyard: FUNCTION 0045c0e0
	ACameraHelper* GetSplitscreenCameraHelper( TUINT a_uiGroup ) const { return m_apSplitscreenCameraHelpers[ a_uiGroup * SPLITSCREEN_MAX_CAMERAS ]; }

	ACameraHelper* GetCurrentCameraHelper() const;
	ACamera*       GetCurrentCamera() const;

	void StopPositionLerp( TFLOAT a_flDeltaTime );

	// Starts a smooth transition from the current camera state to a_pHelper that takes a_fDuration seconds
	void SetSmoothTransition( ACameraHelper* a_pHelper, TFLOAT a_fDuration );

	// Calls ResetTarget on every camera helper
	void ResetHelperTargets();

public:
	ACameraHelper*  m_apCameraHelpers[ CAMERAHELPER_NUM_OF ];                        // 0x20
	ACameraHelper*  m_apSplitscreenCameraHelpers[ SPLITSCREEN_CAMERAHELPER_NUM_OF ]; // 0x58
	ACamera*        m_pCurrentCamera;                                                // 0x88
	ACamera*        m_apCameras[ SPLITSCREEN_MAX_CAMERAS ];                          // 0x8c
	ACameraHelper*  m_pCurrentHelper;                                                // 0x9c
	ACameraHelper*  m_apCurrentSplitscreenCameraHelpers[ SPLITSCREEN_MAX_CAMERAS ];  // 0xa0
	ACameraHelper*  m_pPreviousCameraHelper;                                         // 0xb0
	Toshi::TVector4 m_vLerpPosFrom;                                                  // 0xb4
	Toshi::TVector4 m_vInitialLookDirection;                                         // 0xc4
	TFLOAT          m_fLerpDuration;                                                 // 0xd4
	TFLOAT          m_fLerpTime;                                                     // 0xd8
	TFLOAT          m_fLerpFOVFrom;                                                  // 0xdc
	TBOOL           m_bIsPosLerping;                                                 // 0xe0
};
