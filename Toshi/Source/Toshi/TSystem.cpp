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

TSystemTimer::TSystemTimer()
{
	m_Scheduler      = TNULL;
	m_bIsPaused      = TFALSE;
	m_fAverageFps    = 0.0f;
	m_uiNumFrames    = 0;
	m_Unk4           = TFALSE;
	m_fnQuitCallback = TNULL;
}

void TSystemTimer::Update()
{
	m_Timer.Update();

	TFLOAT fDeltaTime = m_Timer.GetDelta();
	m_fSecond += fDeltaTime;

	if ( m_fSecond > 1.0f )
	{
		m_fAverageFps += 1.0f / fDeltaTime;
		m_fAverageFps *= 0.5f;
		m_fSecond = 0.0;
	}

	m_uiNumFrames += 1;
	m_Scheduler->Update();
}

TSystemTimer::~TSystemTimer()
{
}

TSystemManager::TSystemManager()
    : m_PauseEmitter( this )
{
	m_pSystemTimer = TNULL;
}

void TSystemManager::Update()
{
	m_pSystemTimer->Update();
}

TBOOL TSystemManager::Create()
{
	TASSERT( g_oSystemManager.m_pSystemTimer == TNULL );
	g_oSystemManager.m_pSystemTimer = new TSystemTimer();
	g_oSystemManager.m_pSystemTimer->SetScheduler( new TScheduler() );
	return TTRUE;
}

TSystemManager::~TSystemManager()
{
}

TKernelInterfaceDLL* TKernelInterface::FindInterface( const TString8& a_rcName )
{
	T2_FOREACH( m_Interfaces, it )
	{
		if ( it->GetFileName() == a_rcName )
			return it;
	}

	return TNULL;
}

TKernelInterfaceDLL* TKernelInterface::LoadInterface( const TString8& a_rcName )
{
	TKernelInterfaceDLL* pInterface = FindInterface( a_rcName );

	if ( pInterface )
		return pInterface;

	pInterface = new TKernelInterfaceDLL();

	if ( !pInterface->Load( a_rcName ) )
	{
		delete pInterface;
		return TNULL;
	}

	m_Interfaces.InsertHead( pInterface );
	return pInterface;
}

TOSHI_NAMESPACE_END
