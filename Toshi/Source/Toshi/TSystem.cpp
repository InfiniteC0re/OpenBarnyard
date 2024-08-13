#include "ToshiPCH.h"
#include "TSystem.h"
#include "TScheduler.h"

#include "Toshi/TPString8.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TSystemManager g_oSystemManager;

TSystemManager::TSystemManager() :
	m_PauseEmitter( this )
{
	m_pKernelInterface = TNULL;
}

void TSystemManager::Update()
{
	m_pKernelInterface->Update();
}

TBOOL TSystemManager::Create()
{
	TASSERT( g_oSystemManager.m_pKernelInterface == TNULL );
	g_oSystemManager.m_pKernelInterface = new TKernelInterface();
	g_oSystemManager.m_pKernelInterface->SetScheduler( new TScheduler() );
	return TTRUE;
}

TOSHI_NAMESPACE_END
