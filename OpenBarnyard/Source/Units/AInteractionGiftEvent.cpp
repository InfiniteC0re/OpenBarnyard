#include "pch.h"
#include "AInteractionGiftEvent.h"
#include "AUnit.h"
#include "AUnitType.h"
#include "AUnitManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005c6ec0, CreateObject 005c7100, CreateObjectInPlace 005c7160
TDEFINE_CLASS( AInteractionGiftEvent );

// $Barnyard: FUNCTION 005c6ff0
AInteractionGiftEvent::AInteractionGiftEvent()
{
	m_uiTopicFlags = TOPICFLAGS_GIFT | TOPICFLAGS_HAPPY;
	m_uiPriority   = 118;

	// NOTE: inlined
	Reset();
}

// $Barnyard: FUNCTION 005c7060
// $Barnyard: FUNCTION 005c71a0
AInteractionGiftEvent::~AInteractionGiftEvent()
{
}

// $Barnyard: FUNCTION 005c6f10
void AInteractionGiftEvent::Save( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->WriteFloat( m_fGlobalKnowledge );
	a_pBuffer->WriteFloat( m_fKnowledgeTimer );
	a_pBuffer->WriteString( m_strGiftType );
	a_pBuffer->WriteInt( m_iCount );
	a_pBuffer->WriteString( m_szGiverTypeName );
	a_pBuffer->WriteString( m_szReceiverTypeName );

	AInteractionEvent::Save( a_pBuffer );
}

// Reads the name of a unit type and returns the pooled one
// NOTE: inlined into Load
static const TCHAR* ReadUnitTypeName( AHackedQuickSave::Buffer* a_pBuffer )
{
	TCHAR szTypeName[ 32 ];
	a_pBuffer->ReadString( szTypeName );

	AUnitType* pType = AUnitManager::GetSingleton()->FindUnit( szTypeName );

#ifdef BARNYARD_COMMUNITY_PATCH
	// The type of a unit of an older save can be missing
	if ( !pType )
		return TPString8().GetString();
#endif

	return pType->GetName().GetString();
}

// $Barnyard: FUNCTION 005c71c0
void AInteractionGiftEvent::Load( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->ReadFloat( m_fGlobalKnowledge );
	a_pBuffer->ReadFloat( m_fKnowledgeTimer );
	a_pBuffer->ReadString( m_strGiftType );
	a_pBuffer->ReadInt( m_iCount );

	m_szGiverTypeName    = ReadUnitTypeName( a_pBuffer );
	m_szReceiverTypeName = ReadUnitTypeName( a_pBuffer );

	AInteractionEvent::Load( a_pBuffer );
}

// $Barnyard: FUNCTION 005c7540
AInteractionEventText AInteractionGiftEvent::GetEventText( AUnit* a_pUnit )
{
	return AInteractionEventText();
}

// $Barnyard: FUNCTION 005c6ed0
TBOOL AInteractionGiftEvent::Initialise( TINT a_iUnk )
{
	m_fDuration   = 480.0f;
	m_uiFadeCurve = 2;

	return TTRUE;
}

// NOTE: folded with the Update of the other events that keep the global knowledge
// $Barnyard: FUNCTION 005c5170
TBOOL AInteractionGiftEvent::Update( TFLOAT a_fDeltaTime )
{
	m_fKnowledgeTimer -= a_fDeltaTime;

	if ( m_fKnowledgeTimer <= 0.0f )
	{
		m_fGlobalKnowledge = ComputeGlobalKnowledge();
		m_fKnowledgeTimer  = 15.0f;
	}

	return AInteractionEvent::Update( a_fDeltaTime );
}

// $Barnyard: FUNCTION 005c6ee0
void AInteractionGiftEvent::Reset()
{
	m_fGlobalKnowledge   = 0.0f;
	m_fKnowledgeTimer    = 15.0f;
	m_szGiverTypeName    = TNULL;
	m_szReceiverTypeName = TNULL;
	m_iCount             = 0;

	AInteractionEvent::Reset();
}

// $Barnyard: FUNCTION 005c6fd0
void AInteractionGiftEvent::SetUnit( AUnit* a_pUnit )
{
	if ( a_pUnit && !a_pUnit->IsDying() )
		Notify( a_pUnit, 0 );
}

// $Barnyard: FUNCTION 005c6fa0
void AInteractionGiftEvent::SetGiver( AUnit* a_pUnit )
{
	m_szGiverTypeName = a_pUnit->GetType()->GetName().GetString();
	SetUnit( a_pUnit );
}

// $Barnyard: FUNCTION 005c6f70
void AInteractionGiftEvent::SetReceiver( AUnit* a_pUnit )
{
	m_szReceiverTypeName = a_pUnit->GetType()->GetName().GetString();
	SetUnit( a_pUnit );
}
