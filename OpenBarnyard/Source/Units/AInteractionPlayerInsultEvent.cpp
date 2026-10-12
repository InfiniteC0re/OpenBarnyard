#include "pch.h"
#include "AInteractionPlayerInsultEvent.h"
#include "AUnit.h"

#include <Toshi/TSystem.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005c7290, CreateObject 005c7450, CreateObjectInPlace 005c74b0
TDEFINE_CLASS( AInteractionPlayerInsultEvent );

// $Barnyard: FUNCTION 005c73b0
AInteractionPlayerInsultEvent::AInteractionPlayerInsultEvent()
{
	m_uiTopicFlags = TOPICFLAGS_PLAYER | TOPICFLAGS_HATE;
	m_uiPriority   = 0;

	// NOTE: inlined
	Reset();
}

// $Barnyard: FUNCTION 005c72a0
// $Barnyard: FUNCTION 005c7410
AInteractionPlayerInsultEvent::~AInteractionPlayerInsultEvent()
{
}

// $Barnyard: FUNCTION 005c7330
void AInteractionPlayerInsultEvent::Save( AHackedQuickSave::Buffer* a_pBuffer )
{
	// NOTE: retail calls WriteInt with the raw bits of the floats (same code as WriteFloat)
	a_pBuffer->WriteFloat( m_fGlobalKnowledge );
	a_pBuffer->WriteFloat( m_fKnowledgeTimer );
	a_pBuffer->WriteFloat( m_fInsultTime );

	AInteractionEvent::Save( a_pBuffer );
}

// $Barnyard: FUNCTION 005c7370
void AInteractionPlayerInsultEvent::Load( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->ReadFloat( m_fGlobalKnowledge );
	a_pBuffer->ReadFloat( m_fKnowledgeTimer );
	a_pBuffer->ReadFloat( m_fInsultTime );

	AInteractionEvent::Load( a_pBuffer );
}

// $Barnyard: FUNCTION 005c7430
AInteractionEventText AInteractionPlayerInsultEvent::GetEventText( AUnit* a_pUnit )
{
	// NOTE: retail reads the timer and doesn't use it
	g_oSystemManager.GetTimer().GetCurrentSeconds();

	return AInteractionEventText( L"You insulted me!" );
}

// NOTE: folded with AInteractionGiftEvent::Initialise (005c6ed0)
TBOOL AInteractionPlayerInsultEvent::Initialise( TINT a_iUnk )
{
	m_fDuration   = 480.0f;
	m_uiFadeCurve = 2;

	return TTRUE;
}

// NOTE: folded with the Update of the other events that keep the global knowledge (005c5170)
TBOOL AInteractionPlayerInsultEvent::Update( TFLOAT a_fDeltaTime )
{
	m_fKnowledgeTimer -= a_fDeltaTime;

	if ( m_fKnowledgeTimer <= 0.0f )
	{
		m_fGlobalKnowledge = ComputeGlobalKnowledge();
		m_fKnowledgeTimer  = 15.0f;
	}

	return AInteractionEvent::Update( a_fDeltaTime );
}

// $Barnyard: FUNCTION 005c72b0
void AInteractionPlayerInsultEvent::SetId( TUINT a_uiId )
{
	m_fInsultTime = g_oSystemManager.GetTimer().GetCurrentSeconds();
	AInteractionEvent::SetId( a_uiId );
}

// $Barnyard: FUNCTION 005c72e0
void AInteractionPlayerInsultEvent::Reset()
{
	m_fGlobalKnowledge = 0.0f;
	m_fKnowledgeTimer  = 15.0f;
	m_fInsultTime      = 0.0f;

	AInteractionEvent::Reset();
}

// NOTE: folded with AInteractionGiftEvent::SetUnit (005c6fd0)
void AInteractionPlayerInsultEvent::SetUnit( AUnit* a_pUnit )
{
	if ( a_pUnit && !a_pUnit->IsDying() )
		Notify( a_pUnit, 0 );
}

// $Barnyard: FUNCTION 005c7300
TBOOL AInteractionPlayerInsultEvent::IsRecent()
{
	return g_oSystemManager.GetTimer().GetCurrentSeconds() - m_fInsultTime <= MEMORY_DURATION;
}
