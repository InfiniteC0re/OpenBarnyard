#include "pch.h"
#include "AInteractionQuestEvent.h"
#include "AUnit.h"
#include "AUnitRef.h"
#include "AUnitType.h"
#include "AUnitManager.h"
#include "Quests/AQuest.h"
#include "Quests/AQuestManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005c74f0, CreateObject 005c76c0, CreateObjectInPlace 005c7720
TDEFINE_CLASS( AInteractionQuestEvent );

// $Barnyard: FUNCTION 005c7550
AInteractionQuestEvent::AInteractionQuestEvent()
{
	m_uiTopicFlags = TOPICFLAGS_QUEST;
	m_uiPriority   = 10;

	// NOTE: inlined
	Reset();
}

// $Barnyard: FUNCTION 005c7500
// $Barnyard: FUNCTION 005c75b0
AInteractionQuestEvent::~AInteractionQuestEvent()
{
}

// $Barnyard: FUNCTION 005c7630
void AInteractionQuestEvent::Save( AHackedQuickSave::Buffer* a_pBuffer )
{
	// The quest is saved as its index in AQuestManager
	if ( m_pQuest )
	{
		AQuestManager* pQuestManager = AQuestManager::GetSingleton();
		TINT           iQuestIndex   = -1;

		for ( TINT i = 0; i < pQuestManager->GetNumQuests(); i++ )
		{
			if ( pQuestManager->GetQuest( i )->GetName() == m_pQuest->GetName() )
			{
				iQuestIndex = i;
				break;
			}
		}

		a_pBuffer->WriteInt( iQuestIndex );
	}
	else
	{
		a_pBuffer->WriteInt( -1 );
	}

	a_pBuffer->WriteBool( m_bCompleted );
	a_pBuffer->WriteBool( m_bThanked );
	a_pBuffer->WriteString( m_szGiverTypeName );

	AInteractionEvent::Save( a_pBuffer );
}

// $Barnyard: FUNCTION 005c77b0
void AInteractionQuestEvent::Load( AHackedQuickSave::Buffer* a_pBuffer )
{
	TINT iQuestIndex = 0;
	m_pQuest         = TNULL;

	a_pBuffer->ReadInt( iQuestIndex );

	if ( iQuestIndex > -1 )
		m_pQuest = AQuestManager::GetSingleton()->GetQuest( iQuestIndex );

	a_pBuffer->ReadBool( m_bCompleted );
	a_pBuffer->ReadBool( m_bThanked );

	TCHAR szTypeName[ 32 ];
	a_pBuffer->ReadString( szTypeName );

	AUnitType* pType = AUnitManager::GetSingleton()->FindUnit( szTypeName );

#ifdef BARNYARD_COMMUNITY_PATCH
	// The type of a unit of an older save can be missing
	m_szGiverTypeName = pType ? pType->GetName().GetString() : TPString8().GetString();
#else
	m_szGiverTypeName = pType->GetName().GetString();
#endif

	AInteractionEvent::Load( a_pBuffer );
}

// NOTE: folded with AInteractionGiftEvent::GetEventText
AInteractionEventText AInteractionQuestEvent::GetEventText( AUnit* a_pUnit )
{
	return AInteractionEventText();
}

// NOTE: folded with AInteractionGiftEvent::Initialise
TBOOL AInteractionQuestEvent::Initialise( TINT a_iUnk )
{
	m_fDuration   = 480.0f;
	m_uiFadeCurve = 2;

	return TTRUE;
}

// $Barnyard: FUNCTION 005c7510
TBOOL AInteractionQuestEvent::Update( TFLOAT a_fDeltaTime )
{
	return AInteractionEvent::Update( a_fDeltaTime );
}

// $Barnyard: FUNCTION 005c7520
void AInteractionQuestEvent::Reset()
{
	m_pQuest          = TNULL;
	m_bCompleted      = TFALSE;
	m_bThanked        = TFALSE;
	m_szGiverTypeName = TNULL;

	AInteractionEvent::Reset();
}

// $Barnyard: FUNCTION 005c75d0
void AInteractionQuestEvent::SetUnitRef( AUnitRef* a_pUnitRef )
{
	if ( a_pUnitRef->GetUnit() )
	{
		SetUnit( a_pUnitRef->GetUnit() );
	}
	else if ( a_pUnitRef->pType )
	{
		// The giver isn't spawned, only its type is known
		m_szGiverTypeName = a_pUnitRef->pType->GetName().GetString();
	}
}

// $Barnyard: FUNCTION 005c7760
void AInteractionQuestEvent::SetUnit( AUnit* a_pUnit )
{
	if ( a_pUnit && !a_pUnit->IsDying() )
	{
		m_szGiverTypeName = a_pUnit->GetType()->GetName().GetString();
		Notify( a_pUnit, 0 );
	}
}
