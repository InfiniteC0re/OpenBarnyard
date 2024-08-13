#include "ToshiPCH.h"
#include "TKernelInterface.h"
#include "TScheduler.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TKernelInterface::TKernelInterface()
{
	m_Scheduler		 = TNULL;
	m_bIsPaused		 = TFALSE;
	m_fAverageFps	 = 0.0f;
	m_uiNumFrames	 = 0;
	m_Unk4			 = TFALSE;
	m_fnQuitCallback = TNULL;
}

void TKernelInterface::Update()
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

TOSHI_NAMESPACE_END
