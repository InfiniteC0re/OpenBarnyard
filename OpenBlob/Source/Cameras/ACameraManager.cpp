#include "pch.h"
#include "ACameraManager.h"
#include "AFreeCamera.h"

using namespace Toshi;

ACameraManager::ACameraManager()
{
	TTODO("Some function calls");

	for (size_t i = 0; i < MAX_CAMERA; i++)
	{
		m_ppCameras[i] = TNULL;
	}

	m_bFlag = TFALSE;
	m_iCurrentCamera = -1;
	m_iSomeCount = 0;

	CreateCamera<AFreeCamera>();
	TTODO("CreateCamera<AScriptedCamera>()");
}

ACamera* ACameraManager::CreateCamera(ACamera::CameraType a_eCamType)
{
	switch (a_eCamType)
	{
	case ACamera::CT_DEFAULT:
		TIMPLEMENT();
		break;
	case ACamera::CT_BLOBFOLLOW:
		TIMPLEMENT();
		break;
	case ACamera::CT_SCRIPTED:
		TIMPLEMENT();
		break;
	case ACamera::CT_FREE:
		return CreateCamera<AFreeCamera>();
		break;
	case ACamera::CT_KEYFRAME:
		TIMPLEMENT();
		break;
	default:
		return TNULL;
		break;
	}

	return TNULL;
}
