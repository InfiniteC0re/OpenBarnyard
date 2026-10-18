#include "pch.h"
#include "AUIManager.h"
#include "AModSettings.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AUIManager::AUIManager()
{
}

AUIManager::~AUIManager()
{
}

void AUIManager::Create()
{
	m_Timer.Create();
	m_Speedometer.Create();
}

void AUIManager::Destroy()
{
	m_Timer.Destroy();
}

void AUIManager::Update( TFLOAT a_fDeltaTime )
{
	m_Timer.Update();
	m_Speedometer.Update();
}

void AUIManager::Render()
{
	// Reset stack heigth
	m_LeftSideList.Reset();

	// Render RTA+LRT timers
	if ( g_oSettings.bShowLRTTimer || g_oSettings.bShowRTATimer )
	{
		m_Timer.Render();
	}

	// Render speedometer
	if ( g_oSettings.bShowSpeedometer )
	{
		m_Speedometer.UpdateUIPosition( m_LeftSideList.flHeight );
		m_Speedometer.Render();
	}
}

void AUIManager::ApplyUIStyle()
{
	m_Timer.GetLRTTimerElement().ApplyUIStyle();
	m_Timer.GetRTATimerElement().ApplyUIStyle();
	m_Speedometer.ApplyUIStyle();
}
