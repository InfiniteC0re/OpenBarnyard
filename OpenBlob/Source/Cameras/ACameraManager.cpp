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

ACamera* ACameraManager::SetCurrentCamera(ACamera::CameraType a_eCamType)
{
	for (size_t i = 0; i < MAX_CAMERA; i++)
	{
		if (m_ppCameras[i] == TNULL)
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
				m_ppCameras[i] = new AFreeCamera();
				break;
			case ACamera::CT_KEYFRAME:
				TIMPLEMENT();
				break;
			default:
				break;
			}
			
			if (m_iCurrentCamera == -1)
				m_iCurrentCamera = i;

			if (m_ppCameras[i] != TNULL)
			{
				if (m_ppCameras[i]->GetSomeFlag())
					m_iSomeCount--;

				m_ppCameras[i]->SetSomeFlag(TFALSE);
			}

			return m_ppCameras[i];
		}
		
	}
}
