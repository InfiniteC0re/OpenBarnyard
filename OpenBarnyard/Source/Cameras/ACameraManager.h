#pragma once
#include "ACameraHelper.h"

#include <Toshi/TTask.h>

class ACameraManager :
	public Toshi::TTask,
	public Toshi::TSingleton<ACameraManager>
{
public:
	TDECLARE_CLASS(Toshi::TTask);

	static constexpr TUINT MAX_CAMERAS = 4;

public:
	ACameraManager();

	//-----------------------------------------------------------------------------
	// Toshi::TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL OnUpdate(TFLOAT a_fDeltaTime) override;

	ACamera* GetCamera(TUINT a_uiCameraIndex) const;
	ACamera* GetCurrentCamera() const;

public:
	ACameraHelper* m_pBikePhysicsCamHelper;
	ACameraHelper* m_pGolfCameraHelper;
	ACameraHelper* m_pVictoryCameraHelper;
	ACameraHelper* m_pFollowCameraHelper;
	ACameraHelper* m_pConversationCameraHelper;
	ACameraHelper* m_pSSeqCameraHelper;
	ACameraHelper* m_pBoneAttachCameraHelper;
	ACameraHelper* m_pGTAStyleCameraHelper;
	ACameraHelper* m_pLerpCameraHelper;
	ACameraHelper* m_pPoolCamera;
	ACameraHelper* m_pSpeechCameraHelper;
	ACameraHelper* m_pTrackingCameraHelper;
	ACameraHelper* m_pMultiCameraHelper;
	ACameraHelper* m_pProjectileFireCameraHelper;
	ACameraHelper* m_apProjectileHelpers[MAX_CAMERAS];
	ACameraHelper* m_apGTAHelpers[MAX_CAMERAS];
	ACameraHelper* m_apCarChaseHelpers[MAX_CAMERAS];
	ACamera* m_pCurrentCamera;
	ACamera* m_apCameras[MAX_CAMERAS];
	// ...
	void* m_apSomeArray[MAX_CAMERAS];
	// ...
	Toshi::TVector4 m_vLookDirection;
	// ...
};