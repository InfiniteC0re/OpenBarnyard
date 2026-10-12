#include "pch.h"
#include "AInteractionFearEvent.h"
#include "AUnit.h"
#include "AUnitFactory.h"
#include "AUnitManager.h"
#include "AUnitType.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005c6c30, CreateObject 005c6d80, CreateObjectInPlace 005c6de0
TDEFINE_CLASS( AInteractionFearEvent );

// $Barnyard: FUNCTION 005c6c70
AInteractionFearEvent::AInteractionFearEvent()
{
	m_uiTopicFlags     = TOPICFLAGS_FEAR | TOPICFLAGS_UNHAPPY;
	m_uiPriority       = 0;
	m_fGlobalKnowledge = 0.0f;
	m_fKnowledgeTimer  = 15.0f;
	m_pUnitType        = TNULL;

	AInteractionEvent::Reset();
}

// $Barnyard: FUNCTION 005c6c40
// $Barnyard: FUNCTION 005c6cd0
AInteractionFearEvent::~AInteractionFearEvent()
{
}

// $Barnyard: FUNCTION 005c6d30
void AInteractionFearEvent::Save( AHackedQuickSave::Buffer* a_pBuffer )
{
#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: retail dereferences m_pUnitType without checking it
	a_pBuffer->WriteString( m_pUnitType ? m_pUnitType->GetName().GetString() : TPString8().GetString() );
#else
	a_pBuffer->WriteString( m_pUnitType->GetName().GetString() );
#endif

	// NOTE: retail writes the floats with WriteInt (their bits)
	a_pBuffer->WriteInt( *TREINTERPRETCAST( TINT*, &m_fGlobalKnowledge ) );
	a_pBuffer->WriteInt( *TREINTERPRETCAST( TINT*, &m_fKnowledgeTimer ) );

	AInteractionEvent::Save( a_pBuffer );
}

// $Barnyard: FUNCTION 005c6e20
void AInteractionFearEvent::Load( AHackedQuickSave::Buffer* a_pBuffer )
{
	TPString8 strTypeName;
	a_pBuffer->ReadString( strTypeName );
	m_pUnitType = AUnitManager::GetSingleton()->GetUnitFactory()->Find( strTypeName );

	a_pBuffer->ReadFloat( m_fGlobalKnowledge );
	a_pBuffer->ReadFloat( m_fKnowledgeTimer );

	AInteractionEvent::Load( a_pBuffer );
}

// NOTE: folded with AInteractionGiftEvent::GetEventText (005c7540)
AInteractionEventText AInteractionFearEvent::GetEventText( AUnit* a_pUnit )
{
	return AInteractionEventText();
}

// NOTE: folded with AInteractionGiftEvent::Initialise (005c6ed0)
TBOOL AInteractionFearEvent::Initialise( TINT a_iUnk )
{
	m_fDuration   = 480.0f;
	m_uiFadeCurve = 2;

	return TTRUE;
}

// $Barnyard: FUNCTION 005c6cf0
TBOOL AInteractionFearEvent::Update( TFLOAT a_fDeltaTime )
{
	m_fKnowledgeTimer -= a_fDeltaTime;

	if ( m_fKnowledgeTimer <= 0.0f )
	{
		m_fGlobalKnowledge = ComputeGlobalKnowledge();
		m_fKnowledgeTimer  = 15.0f;
	}

	return AInteractionEvent::Update( a_fDeltaTime );
}

// $Barnyard: FUNCTION 005c6c50
void AInteractionFearEvent::Reset()
{
	m_fGlobalKnowledge = 0.0f;
	m_fKnowledgeTimer  = 15.0f;
	m_pUnitType        = TNULL;

	AInteractionEvent::Reset();
}

// NOTE: folded with AInteractionGiftEvent::SetUnit (005c6fd0)
void AInteractionFearEvent::SetUnit( AUnit* a_pUnit )
{
	if ( a_pUnit && !a_pUnit->IsDying() )
		Notify( a_pUnit, 0 );
}
