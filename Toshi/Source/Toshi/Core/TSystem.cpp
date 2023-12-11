#include "ToshiPCH.h"
#include "TSystem.h"
#include "TScheduler.h"

#include "Toshi/Strings/TPString8.h"

namespace Toshi
{
	TSystemManager g_oSystemManager;

	TSystemManager::TSystemManager() :
		m_PauseEmitter(this)
	{
		m_pKernelInterface = TNULL;
	}

	void TSystemManager::Update()
	{
		m_pKernelInterface->Update();
	}

	TBOOL TSystemManager::Create()
	{
		TASSERT(g_oSystemManager.m_pKernelInterface == TNULL);
		g_oSystemManager.m_pKernelInterface = new TKernelInterface();
		g_oSystemManager.m_pKernelInterface->SetScheduler(new TScheduler());
		return TTRUE;
	}

}