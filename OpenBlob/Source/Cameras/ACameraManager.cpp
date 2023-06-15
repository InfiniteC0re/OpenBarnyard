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