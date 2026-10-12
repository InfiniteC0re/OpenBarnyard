#include "pch.h"
#include "AQuestInstance.h"
#include "AQuestManager.h"
#include "AQuest.h"
#include "AQuestElement.h"
#include "AGameEventQuestElement.h"
#include "ACheatActivator.h"
#include "Helpers/AGameLoader.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"
#include "Units/AAnimalPopulationManager.h"
#include "Units/APlayerControlComponent.h"
#include "Units/AUnit.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitNameManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005c9be0, CreateObject 005cb180, CreateObjectInPlace 005cb1e0
TDEFINE_CLASS( AQuestInstance );

// $Barnyard: FUNCTION 005ca870
// NOTE: retail leaves m_iQuest, m_eState, m_fTime, m_pEndSeq and m_pStartSeq uninitialised
AQuestInstance::AQuestInstance()
    : m_iQuest( 0 )
    , m_iId( -1 )
    , m_eState( STATE_START )
    , m_fTime( 0.0f )
    , m_uiActiveElements( 0 )
    , m_pEndSeq( TNULL )
    , m_pStartSeq( TNULL )
{
	m_oQuestGiver.pType  = TNULL;
	m_oQuestGiver.szName = TNULL;
}

// $Barnyard: FUNCTION 005ca290
// $Barnyard: FUNCTION 005ca8f0
AQuestInstance::~AQuestInstance()
{
	m_oListener.Disconnect();
	m_pEndSeq   = TNULL;
	m_pStartSeq = TNULL;

	// NOTE: retail frees the data through 005c9ba0 (AInteractionActivityData's destructor)
	TFree( m_oData.pData );
	m_oData.pData = TNULL;
}

// $Barnyard: FUNCTION 005ca340
void AQuestInstance::Reset()
{
	DeinitElements();

	// NOTE: retail frees the data through 005c9ba0 (AInteractionActivityData's destructor)
	TFree( m_oData.pData );
	m_oData.pData = TNULL;

	m_oListener.Disconnect();
}

// $Barnyard: FUNCTION 005c9ca0
AQuest* AQuestInstance::GetQuest()
{
	return AQuestManager::GetSingleton()->GetQuest( m_iQuest );
}

// $Barnyard: FUNCTION 005c9cb0
TBOOL AQuestInstance::Save( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->WriteInt( m_eState );
	a_pBuffer->WriteInt( m_iQuest );
	a_pBuffer->WriteInt( m_iId );
	a_pBuffer->WriteFloat( m_fTime );
	a_pBuffer->WriteUInt16( m_uiActiveElements );
	AUnitManager::GetSingleton()->SaveUnitRef( a_pBuffer, &m_oQuestGiver );

	AQuest* pQuest = GetQuest();

	for ( TINT i = 0; i < pQuest->GetNumElements(); i++ )
		pQuest->GetElement( i )->Save( a_pBuffer, &m_oData );

	return TTRUE;
}

// $Barnyard: FUNCTION 005caaa0
TBOOL AQuestInstance::Load( AHackedQuickSave::Buffer* a_pBuffer, TBOOL a_bSpawnUnits )
{
	TUINT uiState = 0;
	a_pBuffer->ReadUInt( uiState );
	m_eState = STATE( uiState );

	a_pBuffer->ReadInt( m_iQuest );
	a_pBuffer->ReadInt( m_iId );
	a_pBuffer->ReadFloat( m_fTime );
	a_pBuffer->ReadUInt16( m_uiActiveElements );

	if ( a_bSpawnUnits )
		AAnimalPopulationManager::GetSingleton()->SpawnQuestUnits( GetQuest()->GetName() );

	AUnitManager::GetSingleton()->LoadUnitRef( a_pBuffer, &m_oQuestGiver );

	AQuest* pQuest = GetQuest();

	m_oData.AllocateFromSize( pQuest->GetDataSize() );

	for ( TINT i = 0; i < pQuest->GetNumElements(); i++ )
		pQuest->GetElement( i )->Load( a_pBuffer, &m_oData );

	m_pStartSeq = TNULL;
	m_pEndSeq   = TNULL;

	if ( m_eState != STATE_COMPLETE )
		m_oListener.Connect( this, OnInteractionEventCallback );

	return TTRUE;
}

// $Barnyard: FUNCTION 005ca910
void AQuestInstance::Create( TINT a_iQuest, TINT a_iId, AUnit* a_pQuestGiver )
{
	m_eState = STATE_START;
	m_fTime  = 0.0f;
	m_iQuest = a_iQuest;
	m_iId    = a_iId;

	if ( !a_pQuestGiver )
	{
		m_oQuestGiver.oProxy = TNULL;
		m_oQuestGiver.pType  = TNULL;
		m_oQuestGiver.szName = TNULL;
	}
	else
	{
		m_oQuestGiver.oProxy = a_pQuestGiver->GetSelfProxy();

		// NOTE: keeps the string of the pooled name, it's referenced by AUnitNameManager
		TPString8 strName    = AUnitNameManager::GetSingleton()->GetUnitName( a_pQuestGiver->GetId() );
		m_oQuestGiver.szName = strName.GetString();
	}

	AQuest* pQuest = GetQuest();

	m_oData.AllocateFromSize( pQuest->GetDataSize() );

	m_uiActiveElements = 0;

	for ( TINT i = 0; i < pQuest->GetNumElements(); i++ )
	{
		m_uiActiveElements |= 1 << i;

		AQuestElement* pElement = pQuest->GetElement( i );
		pElement->InitData( &m_oData );
		pElement->GetLocator().AddMarker();
	}

	m_pEndSeq = TNULL;
	m_oListener.Connect( this, OnInteractionEventCallback );
}

// $Barnyard: FUNCTION 005c9f80
TBOOL AQuestInstance::Update( TFLOAT a_fDeltaTime )
{
	if ( !AGameLoader::IsGameStateLibraryLoaded() && !AQuestManager::GetSingleton()->IsTimerPaused() )
		m_fTime += a_fDeltaTime;

	AQuest* pQuest = GetQuest();

	if ( m_eState == STATE_START )
	{
		// The quest giver doesn't offer the quest anymore
		APlayerProfileManager::GetSingleton()->GetMainProfile()->ReleaseItem( pQuest->GetQuestGiver() );

		if ( !pQuest->GetStartSeq().IsEmpty() )
		{
			m_pStartSeq = AInteractionManager::GetSingleton()->AssignInteractionToAllUnits( pQuest->GetStartSeq() );

			if ( m_pStartSeq )
				m_eState = STATE_STARTSEQ;
		}
		else
		{
			m_eState = STATE_RUNNING;
		}
	}
	else if ( m_eState == STATE_STARTSEQ || m_eState == STATE_ENDSEQ )
	{
		return TTRUE;
	}

	// Run the elements
	TBOOL bAllDone = TTRUE;

	for ( TINT i = 0; i < pQuest->GetNumElements(); i++ )
	{
		AQuestElement* pElement = pQuest->GetElement( i );

		if ( bAllDone )
		{
			AQuestElement::STATE eState = pElement->GetProgress( &m_oData )->eState;

			if ( eState != AQuestElement::STATE_COMPLETE && eState != AQuestElement::STATE_FAILED )
				bAllDone = TFALSE;
		}

		if ( m_uiActiveElements & ( 1 << i ) )
		{
			if ( pElement->Update( &m_oData, a_fDeltaTime ) == TTRUE )
			{
				m_uiActiveElements &= ~( 1 << i );
			}
			else if ( pElement->IsBlocking() )
			{
				// The next elements wait for this one
				break;
			}
		}
	}

	if ( m_uiActiveElements != 0 )
	{
		// Some elements failed for good
		if ( bAllDone == TTRUE )
			m_eState = STATE_FAILED;

		return TTRUE;
	}

	if ( m_eState == STATE_COMPLETE )
		return TTRUE;

	// Every element is complete
	if ( pQuest->GetEndSeq().IsEmpty() )
	{
		m_eState = STATE_COMPLETE;
		return TTRUE;
	}

	m_pEndSeq = AInteractionManager::GetSingleton()->AssignInteractionToAllUnits( pQuest->GetEndSeq() );

	if ( m_pEndSeq )
	{
		m_eState = STATE_ENDSEQ;

		if ( !pQuest->IsEndSeqBlocking() )
			m_eState = STATE_COMPLETE;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005c9d50
void AQuestInstance::SetElementState( TINT a_iElement, ELEMENTSTATE a_eState, TBOOL a_bCheckBlocking )
{
	AQuest* pQuest = GetQuest();

	if ( a_bCheckBlocking == TTRUE && a_iElement > 0 && pQuest->GetNumElements() > 1 )
	{
		// Ignored while a blocking element before it still runs
		for ( TINT i = a_iElement - 1; i >= 0; i-- )
		{
			if ( pQuest->GetElement( i )->IsBlocking() && ( m_uiActiveElements & ( 1 << i ) ) )
				return;
		}
	}

	AQuestElement::Progress* pProgress = pQuest->GetElement( a_iElement )->GetProgress( &m_oData );

	if ( a_eState == ELEMENTSTATE_COMPLETE )
	{
		if ( !pProgress->bComplete )
		{
			AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

			if ( pPlayer )
			{
				APlayerControlComponent* pPlayerControl = TSTATICCAST( APlayerControlComponent, pPlayer->FindComponent( &TGetClass( APlayerControlComponent ) ) );

				if ( pPlayerControl )
					pPlayerControl->SetUseItemCooldown( 1.0f );
			}

			pProgress->bComplete = TTRUE;
		}
	}
	else if ( a_eState == ELEMENTSTATE_FAILED )
	{
		pProgress->bFailed = TTRUE;
	}
	else if ( a_eState == ELEMENTSTATE_FAILEDFORGOOD )
	{
		pProgress->bFailedForGood = TTRUE;
	}
}

// $Barnyard: FUNCTION 005ca1e0
void AQuestInstance::SetElementsState( ELEMENTSTATE a_eState )
{
	for ( TINT i = 0; i < GetQuest()->GetNumElements(); i++ )
		SetElementState( i, a_eState, TFALSE );
}

// $Barnyard: FUNCTION 005c9e20
TBOOL AQuestInstance::IsElementDone( TINT a_iElement ) const
{
	return ( m_uiActiveElements & TUINT16( 1 << a_iElement ) ) == 0;
}

// $Barnyard: FUNCTION 005c9c30
void AQuestInstance::DeinitElements()
{
	AQuest* pQuest = GetQuest();

	if ( pQuest )
	{
		for ( TINT i = 0; i < pQuest->GetNumElements(); i++ )
			pQuest->GetElement( i )->DeinitData( &m_oData );
	}
}

// $Barnyard: FUNCTION 005c9c70
void AQuestInstance::RemoveMarkers()
{
	AQuest* pQuest = GetQuest();

	if ( pQuest )
	{
		for ( TINT i = 0; i < pQuest->GetNumElements(); i++ )
			pQuest->GetElement( i )->GetLocator().RemoveMarker();
	}
}

// $Barnyard: FUNCTION 005c9e40
void AQuestInstance::OnMiniGameResult( AMiniGameState::S1* a_pResults )
{
	AQuest* pQuest = GetQuest();

	for ( TINT i = 0; i < pQuest->GetNumElements(); i++ )
	{
		if ( m_uiActiveElements & ( 1 << i ) )
		{
			AQuestElement* pElement = pQuest->GetElement( i );

			if ( pElement->IsA( &TGetClass( AGameEventQuestElement ) ) )
				TSTATICCAST( AGameEventQuestElement, pElement )->OnMiniGameResult( &m_oData, a_pResults );
		}
	}
}

// $Barnyard: FUNCTION 005c9eb0
void AQuestInstance::OnMiniGameStart( const TPString8& a_rcName )
{
	AQuest* pQuest = GetQuest();

	for ( TINT i = 0; i < pQuest->GetNumElements(); i++ )
	{
		if ( m_uiActiveElements & ( 1 << i ) )
		{
			AQuestElement* pElement = pQuest->GetElement( i );

			if ( pElement->IsA( &TGetClass( AGameEventQuestElement ) ) )
				TSTATICCAST( AGameEventQuestElement, pElement )->OnMiniGameStart( a_rcName, &m_oData );
		}
	}
}

// $Barnyard: FUNCTION 005c9c00
AUnit* AQuestInstance::GetQuestGiver()
{
	return m_oQuestGiver.GetUnit();
}

// $Barnyard: FUNCTION 005ca380
void AQuestInstance::OnInteractionEvent( const AInteractionManagerEvent& a_rcEvent )
{
	AInteractionManagerEvent::TYPE eType = a_rcEvent.GetType();

	if ( HASALLFLAGS( eType, AInteractionManagerEvent::TYPE_SEQUENCE | AInteractionManagerEvent::TYPE_ENDED ) )
	{
		// A sequence ended
		if ( m_eState == STATE_ENDSEQ )
		{
			if ( a_rcEvent.GetSequence() == m_pEndSeq )
			{
				m_oListener.Remove();
				m_pEndSeq = TNULL;
				m_eState  = STATE_COMPLETE;

				if ( GetQuest()->GetName() == TPString8( "Sleepy Cows" ) )
				{
					// The player can move again
					APlayerControlComponent* pPlayerControl = TSTATICCAST( APlayerControlComponent, APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->FindComponent( &TGetClass( APlayerControlComponent ) ) );

					if ( pPlayerControl )
					{
						pPlayerControl->SetMovementEnabled( TTRUE );
						pPlayerControl->SetInventoryDisabled( TFALSE );
						ACheatActivator::ResetRequirement( ACheatActivator::GetSingleton()->GetRequirement( ACheatActivator::REQUIREMENT_ROCK_CLOSE_TO_SECONDARY ) );
					}
				}
			}

			return;
		}

		if ( m_eState == STATE_STARTSEQ )
		{
			if ( a_rcEvent.GetSequence() == m_pStartSeq )
			{
				m_pStartSeq = TNULL;
				m_eState    = STATE_RUNNING;
			}

			return;
		}

		// The sequences of the elements
		AQuest* pQuest = GetQuest();

		for ( TINT i = 0; i < pQuest->GetNumElements(); i++ )
		{
			AQuestElement::Progress* pProgress = pQuest->GetElement( i )->GetProgress( &m_oData );
			TUINT                    uiSeqId   = a_rcEvent.GetSequence()->GetId();

			if ( pProgress->eState == AQuestElement::STATE_ENDSEQ && uiSeqId == pProgress->uiEndSeqId )
			{
				pProgress->eState = AQuestElement::STATE_COMPLETE;
			}
			else if ( pProgress->eState == AQuestElement::STATE_STARTSEQ && uiSeqId == pProgress->uiStartSeqId )
			{
				pProgress->eState = AQuestElement::STATE_RUNNING;
			}
			else if ( pProgress->eState == AQuestElement::STATE_FAILSEQ && uiSeqId == pProgress->uiFailSeqId )
			{
				if ( !pProgress->bFailedForGood )
				{
					// Try again
					pProgress->bFailed = TFALSE;
					pProgress->eState  = AQuestElement::STATE_RUNNING;
				}
				else
				{
					pProgress->eState = AQuestElement::STATE_FAILED;
				}
			}
		}
	}
	else if ( HASALLFLAGS( eType, AInteractionManagerEvent::TYPE_SEQUENCE | AInteractionManagerEvent::TYPE_STARTED ) )
	{
		// A sequence started
		if ( m_eState == STATE_ENDSEQ || m_eState == STATE_STARTSEQ )
			return;

		AQuest* pQuest = GetQuest();

		for ( TINT i = 0; i < pQuest->GetNumElements(); i++ )
		{
			AQuestElement::Progress* pProgress = pQuest->GetElement( i )->GetProgress( &m_oData );

			if ( pProgress->eState != AQuestElement::STATE_ENDSEQ || a_rcEvent.GetSequence()->GetId() != pProgress->uiEndSeqId )
				continue;

			if ( GetQuest()->GetName() != TPString8( "Sleepy Cows" ) )
				continue;

			// The player can't move once the last cow is woken up
			TINT iNumDone = 0;

			for ( TINT k = 0; k < GetQuest()->GetNumElements(); k++ )
			{
				if ( ( m_uiActiveElements & ( 1 << k ) ) == 0 ||
				     GetQuest()->GetElement( k )->GetProgress( &m_oData )->eState == AQuestElement::STATE_ENDSEQ )
				{
					iNumDone++;
				}
			}

			if ( iNumDone == 3 )
			{
				APlayerControlComponent* pPlayerControl = TSTATICCAST( APlayerControlComponent, APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->FindComponent( &TGetClass( APlayerControlComponent ) ) );

				if ( pPlayerControl )
				{
					pPlayerControl->SetMovementEnabled( TFALSE );
					pPlayerControl->SetInventoryDisabled( TTRUE );
					ACheatActivator::SetRequirementSatisfied( ACheatActivator::GetSingleton()->GetRequirement( ACheatActivator::REQUIREMENT_ROCK_CLOSE_TO_SECONDARY ), TFALSE );
				}
			}
		}
	}
}

// $Barnyard: FUNCTION 005ca700
void AQuestInstance::OnInteractionEventCallback( void* a_pInstance, const AInteractionManagerEvent& a_rcEvent )
{
	TSTATICCAST( AQuestInstance, a_pInstance )->OnInteractionEvent( a_rcEvent );
}
