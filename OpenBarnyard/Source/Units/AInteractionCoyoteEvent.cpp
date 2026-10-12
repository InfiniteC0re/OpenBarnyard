#include "pch.h"
#include "AInteractionCoyoteEvent.h"
#include "AUnit.h"
#include "AUnitManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005c5110, CreateObject 005c5510, CreateObjectInPlace 005c5570
TDEFINE_CLASS( AInteractionCoyoteEvent );

// $Barnyard: FUNCTION 005c53f0
AInteractionCoyoteEvent::AInteractionCoyoteEvent()
{
	m_uiTopicFlags = TOPICFLAGS_FOOD;
	m_uiPriority   = 118;

	AInteractionCoyoteEvent::Reset();
}

// $Barnyard: FUNCTION 005c5350
// $Barnyard: FUNCTION 005c5460
AInteractionCoyoteEvent::~AInteractionCoyoteEvent()
{
}

// $Barnyard: FUNCTION 005c5210
void AInteractionCoyoteEvent::Save( AHackedQuickSave::Buffer* a_pBuffer )
{
	// NOTE: retail writes the floats with WriteInt (their bits)
	a_pBuffer->WriteInt( *TREINTERPRETCAST( TINT*, &m_fGlobalKnowledge ) );
	a_pBuffer->WriteInt( *TREINTERPRETCAST( TINT*, &m_fKnowledgeTimer ) );

	for ( TINT i = 0; i < MAX_NUM_UNITS; i++ )
	{
		AUnitManager::GetSingleton()->SaveUnitRef( a_pBuffer, &m_aUnits[ i ] );
		a_pBuffer->WriteInt( m_aUnitStates[ i ] );
	}

	AInteractionEvent::Save( a_pBuffer );
}

// $Barnyard: FUNCTION 005c5280
void AInteractionCoyoteEvent::Load( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->ReadFloat( m_fGlobalKnowledge );
	a_pBuffer->ReadFloat( m_fKnowledgeTimer );

	for ( TINT i = 0; i < MAX_NUM_UNITS; i++ )
	{
		AUnitManager::GetSingleton()->LoadUnitRef( a_pBuffer, &m_aUnits[ i ] );
		a_pBuffer->ReadInt( m_aUnitStates[ i ] );
	}

	AInteractionEvent::Load( a_pBuffer );
}

// $Barnyard: FUNCTION 005c5130
AInteractionEventText AInteractionCoyoteEvent::GetEventText( AUnit* a_pUnit )
{
	switch ( TUINT( a_pUnit->GetId() ) % 3 )
	{
		case 0: return AInteractionEventText( L"Those Coyotes definately aint vegans!" );
		case 1: return AInteractionEventText( L"I hope Ben can defend us from the Coyotes!" );
		default: return AInteractionEventText( L"The Coyotes are coming!" );
	}
}

// NOTE: folded with AInteractionGiftEvent::Initialise (005c6ed0)
TBOOL AInteractionCoyoteEvent::Initialise( TINT a_iUnk )
{
	m_fDuration   = 480.0f;
	m_uiFadeCurve = 2;

	return TTRUE;
}

// NOTE: folded with AInteractionGiftEvent::Update (005c5170)
TBOOL AInteractionCoyoteEvent::Update( TFLOAT a_fDeltaTime )
{
	m_fKnowledgeTimer -= a_fDeltaTime;

	if ( m_fKnowledgeTimer <= 0.0f )
	{
		m_fGlobalKnowledge = ComputeGlobalKnowledge();
		m_fKnowledgeTimer  = 15.0f;
	}

	return AInteractionEvent::Update( a_fDeltaTime );
}

// $Barnyard: FUNCTION 005c51b0
void AInteractionCoyoteEvent::Reset()
{
	m_fGlobalKnowledge = 0.0f;
	m_fKnowledgeTimer  = 15.0f;

	for ( TINT i = 0; i < MAX_NUM_UNITS; i++ )
	{
		m_aUnits[ i ].oProxy = TNULL;
		m_aUnits[ i ].pType  = TNULL;
		m_aUnits[ i ].szName = TNULL;
		m_aUnitStates[ i ]   = 0;
	}

	AInteractionEvent::Reset();
}

// $Barnyard: FUNCTION 005c53d0
void AInteractionCoyoteEvent::SetUnit( AUnit* a_pUnit )
{
	if ( a_pUnit && !a_pUnit->IsDying() )
		AddUnit( a_pUnit );
}

// $Barnyard: FUNCTION 005c5480
void AInteractionCoyoteEvent::AddUnit( AUnit* a_pUnit )
{
	TINT i = 0;

	// Find a free slot
	while ( m_aUnits[ i ].GetUnit() || m_aUnits[ i ].pType )
	{
		if ( ++i >= MAX_NUM_UNITS )
		{
			AInteractionEvent::Notify( a_pUnit, 0 );
			return;
		}
	}

	m_aUnits[ i ].oProxy = a_pUnit->GetSelfProxy();
	m_aUnitStates[ i ]   = 0;

	AInteractionEvent::Notify( a_pUnit, 0 );
}
