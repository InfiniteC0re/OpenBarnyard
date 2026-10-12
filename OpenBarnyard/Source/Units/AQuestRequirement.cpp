#include "pch.h"
#include "AQuestRequirement.h"
#include "AInteractionEventManager.h"
#include "AInteractionQuestEvent.h"
#include "AUnit.h"
#include "AUnitType.h"
#include "Quests/AQuestManager.h"
#include "Quests/AQuestInstance.h"
#include "Quests/AQuest.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005ae630, CreateObject 005aef50, CreateObjectInPlace 005aefe0 (the constructor is inlined into them)
TDEFINE_CLASS( AQuestRequirement );

TPSTRING8_DECLARE( event );
TPSTRING8_DECLARE( exists );
TPSTRING8_DECLARE( given );
TPSTRING8_DECLARE( givenbyunit );
TPSTRING8_DECLARE( quest );
TPSTRING8_DECLARE( questelement );

AQuestRequirement::AQuestRequirement()
    : m_iQuestElement( -1 )
    , m_uiFlags( FLAGS_NONE )
    , m_fInterestWeight( -1.0f )
{
}

// $Barnyard: FUNCTION 005aeb10
// $Barnyard: FUNCTION 005af060
AQuestRequirement::~AQuestRequirement()
{
}

// $Barnyard: FUNCTION 005aeb80
TBOOL AQuestRequirement::Create( const PBProperties* a_pProperties )
{
	for ( auto it = a_pProperties->Begin(); it != a_pProperties->End(); it++ )
	{
		const PBPropertyValue* pValue = it->GetValue();
		const TCHAR*           szName = it->GetName().GetString();

		switch ( szName[ 0 ] )
		{
			case 'e':
			{
				TPString8 strName( szName );

				if ( strName == TPS8( event ) )
				{
					if ( pValue->GetRaw() )
						m_uiFlags |= FLAGS_EVENT;
				}
				else if ( strName == TPS8( exists ) )
				{
					m_uiFlags |= FLAGS_CHECK_EXISTS;

					if ( pValue->GetRaw() )
						m_uiFlags |= FLAGS_EXISTS;
				}

				break;
			}
			case 'g':
			{
				TPString8 strName( szName );

				if ( strName == TPS8( given ) )
				{
					m_uiFlags |= FLAGS_CHECK_GIVEN;
					m_uiFlags |= pValue->GetRaw() ? FLAGS_GIVEN : FLAGS_NOT_GIVEN;
				}
				else if ( strName == TPS8( givenbyunit ) )
				{
					if ( pValue->GetRaw() )
						m_uiFlags |= FLAGS_GIVEN_BY_UNIT;
					else
						m_uiFlags &= ~FLAGS_GIVEN_BY_UNIT;
				}

				break;
			}
			case 'i':
				if ( TStringManager::String8Compare( szName, "interestweight" ) == 0 )
				{
					m_uiFlags |= FLAGS_INTEREST_WEIGHT;
					m_fInterestWeight = ( pValue->GetType() == PBPropertyValue::Type::Float ) ? pValue->GetFloat() : TFLOAT( pValue->GetInteger() );
				}

				break;
			case 'q':
			{
				TPString8 strName( szName );

				if ( strName == TPS8( quest ) )
				{
					m_strQuest = pValue->GetTPString8();
				}
				else if ( strName == TPS8( questelement ) )
				{
					m_uiFlags |= FLAGS_QUEST_ELEMENT;
					m_iQuestElement = pValue->GetInteger();
				}

				break;
			}
			case 's':
				if ( TStringManager::String8Compare( szName, "satisfied" ) == 0 )
					m_uiFlags |= pValue->GetRaw() ? FLAGS_SATISFIED : FLAGS_NOT_SATISFIED;

				break;
			case 't':
				if ( TStringManager::String8Compare( szName, "thanked" ) == 0 )
					m_uiFlags |= pValue->GetRaw() ? FLAGS_THANKED : FLAGS_NOT_THANKED;

				break;
		}
	}

	return AInteractionRequirement::Create( a_pProperties );
}

void AQuestRequirement::SetAllRoles( TINT a_iNumUnits, TBOOL a_bAllowed )
{
	if ( m_bInvert == TTRUE )
		a_bAllowed = !a_bAllowed;

	for ( TINT i = 0; i < a_iNumUnits; i++ )
	{
		for ( TINT k = 0; k < AInteractionRoles::NUM_ROLES; k++ )
			m_oRoles.SetUnit( k, i, a_bAllowed );
	}
}

// $Barnyard: FUNCTION 005ae640
void AQuestRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	TINT                               iNumUnits     = a_pParticipants->iNumUnits;
	AInteractionEventManager*          pEventManager = AInteractionEventManager::GetSingleton();
	AInteractionEventManager::Iterator oIterator;
	AQuestManager*                     pQuestManager = AQuestManager::GetSingleton();

	if ( m_uiFlags & FLAGS_QUEST_ELEMENT )
	{
		// The element of the quest is done
		TBOOL bDone = pQuestManager->IsQuestComplete( m_strQuest, 1 );

		if ( !bDone )
		{
			AQuestInstance* pInstance = pQuestManager->FindAssignedQuest( m_strQuest, 1 );
			bDone                     = pInstance && pInstance->IsElementDone( m_iQuestElement - 1 );
		}

		SetAllRoles( iNumUnits, bDone == ( ( m_uiFlags & FLAGS_SATISFIED ) != 0 ) );
	}
	else if ( m_uiFlags & FLAGS_EVENT )
	{
		// The quest event of the unit
		for ( TINT i = 0; i < iNumUnits; i++ )
		{
			AUnit* pUnit    = a_pParticipants->Get( i );
			TBOOL  bDefault = m_bInvert == TTRUE;

			for ( TINT k = 0; k < AInteractionRoles::NUM_ROLES; k++ )
				m_oRoles.SetUnit( k, i, bDefault );

			TBOOL bAllowed = TFALSE;

			for ( AInteractionQuestEvent* pEvent = TSTATICCAST( AInteractionQuestEvent, pEventManager->FindFirstEvent( &TGetClass( AInteractionQuestEvent ), oIterator ) );
			      pEvent;
			      pEvent = TSTATICCAST( AInteractionQuestEvent, pEventManager->FindNextEvent( &TGetClass( AInteractionQuestEvent ), oIterator ) ) )
			{
				if ( pEvent->GetGiverTypeName() == pUnit->GetType()->GetName().GetString() && pEvent->GetQuest()->GetName() == m_strQuest )
				{
					bAllowed = TTRUE;

					if ( m_uiFlags & ( FLAGS_SATISFIED | FLAGS_NOT_SATISFIED ) )
						bAllowed = pEvent->IsCompleted() == ( ( m_uiFlags & FLAGS_SATISFIED ) != 0 );

					if ( ( m_uiFlags & ( FLAGS_THANKED | FLAGS_NOT_THANKED ) ) && pEvent->WasThanked() != ( ( m_uiFlags & FLAGS_THANKED ) != 0 ) )
					{
						bAllowed = TFALSE;
						continue;
					}
				}

				if ( bAllowed )
					break;
			}

			SetUnitAllowed( m_eCategory, i, bAllowed );
		}
	}
	else if ( m_uiFlags & FLAGS_CHECK_EXISTS )
	{
		// The quest event exists
		AInteractionQuestEvent* pEvent = TSTATICCAST( AInteractionQuestEvent, pEventManager->FindFirstEvent( &TGetClass( AInteractionQuestEvent ), oIterator ) );
		TBOOL                   bFound = TFALSE;

		for ( ; pEvent; pEvent = TSTATICCAST( AInteractionQuestEvent, pEventManager->FindNextEvent( &TGetClass( AInteractionQuestEvent ), oIterator ) ) )
		{
			if ( pEvent->GetQuest()->GetName() == m_strQuest )
			{
				bFound = TTRUE;
				break;
			}
		}

		TBOOL bAllowed = bFound == ( ( m_uiFlags & FLAGS_EXISTS ) != 0 );

		if ( bFound )
		{
			if ( m_uiFlags & ( FLAGS_SATISFIED | FLAGS_NOT_SATISFIED ) )
				bAllowed = bAllowed && pEvent->IsCompleted() == ( ( m_uiFlags & FLAGS_SATISFIED ) != 0 );

			if ( m_uiFlags & ( FLAGS_THANKED | FLAGS_NOT_THANKED ) )
				bAllowed = bAllowed && pEvent->WasThanked() == ( ( m_uiFlags & FLAGS_THANKED ) != 0 );

			if ( m_uiFlags & FLAGS_INTEREST_WEIGHT )
				bAllowed = bAllowed && pEvent->GetInterest() > m_fInterestWeight;
		}

		SetAllRoles( iNumUnits, bAllowed );
	}
	else if ( !( m_uiFlags & FLAGS_CHECK_GIVEN ) )
	{
		if ( m_uiFlags & FLAGS_GIVEN_BY_UNIT )
		{
			// The unit gave one of the assigned quests
			for ( TINT i = 0; i < iNumUnits; i++ )
			{
				AUnit* pUnit    = a_pParticipants->Get( i );
				TBOOL  bAllowed = TFALSE;

				for ( TINT k = 0; k < pQuestManager->GetNumAssignedQuests(); k++ )
				{
					if ( pQuestManager->GetAssignedQuest( k )->GetQuestGiver() == pUnit )
					{
						bAllowed = TTRUE;
						break;
					}
				}

				SetUnitAllowed( m_eCategory, i, bAllowed );
			}
		}
	}
	else
	{
		// The quest is assigned or complete (any quest is assigned if there's no name)
		TBOOL bGiven;

		if ( !m_strQuest.IsEmpty() )
			bGiven = pQuestManager->FindAssignedQuest( m_strQuest, 1 ) != TNULL || pQuestManager->IsQuestComplete( m_strQuest, 1 );
		else
			bGiven = pQuestManager->GetNumAssignedQuests() != 0;

		SetAllRoles( iNumUnits, bGiven == ( ( m_uiFlags & FLAGS_GIVEN ) != 0 ) );
	}
}
