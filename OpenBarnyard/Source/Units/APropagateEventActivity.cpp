#include "pch.h"
#include "APropagateEventActivity.h"
#include "AInteractionComponent.h"
#include "AInteractionEvent.h"
#include "AInteractionEventManager.h"
#include "AUnit.h"
#include "AUnitGameEventComponent.h"
#include "ARandom.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005bb460, CreateObject 005bb480, CreateObjectInPlace 005bb500
TDEFINE_CLASS( APropagateEventActivity );

// NOTE: inlined into CreateObject 005bb480 / CreateObjectInPlace 005bb500
// NOTE: retail leaves m_pEvent uninitialised
APropagateEventActivity::APropagateEventActivity()
    : m_eUnitCategory( AInteractionSequence::CATEGORY_PRIMARY )
    , m_pEventClass( TNULL )
    , m_pEvent( TNULL )
    , m_fMutateChance( 0.0f )
    , m_iForceMutateLevel( -1 )
{
}

// $Barnyard: FUNCTION 005bb470
// $Barnyard: FUNCTION 005bb570
APropagateEventActivity::~APropagateEventActivity()
{
}

// $Barnyard: FUNCTION 005bb620
TBOOL APropagateEventActivity::Create( const PBProperties* a_pProperties )
{
	const PBPropertyValue* pUnit = a_pProperties->GetProperty( "unit" );

	if ( pUnit )
		m_eUnitCategory = AInteractionSequence::GetCategoryFromName( pUnit->GetTPString8().GetString() );

	const PBPropertyValue* pEvent = a_pProperties->GetProperty( "event" );

	if ( pEvent )
	{
		TString8 strClassName;
		strClassName.Format( "ainteraction%sevent", pEvent->GetTPString8().GetString() );

		m_pEventClass = TClass::Find( strClassName, &TGetClass( AInteractionEvent ) );

		if ( !m_pEventClass )
			return TFALSE;
	}

	const PBPropertyValue* pMutateChance = a_pProperties->GetProperty( "mutatechance" );

	if ( pMutateChance )
	{
		m_fMutateChance = ( pMutateChance->GetType() == PBPropertyValue::Type::Float ) ? pMutateChance->GetFloat() : TFLOAT( pMutateChance->GetInteger() );
		TMath::Clip( m_fMutateChance, 0.0f, 1.0f );
	}

	const PBPropertyValue* pForceMutateLevel = a_pProperties->GetProperty( "forcemutatelevel" );

	if ( pForceMutateLevel )
		m_iForceMutateLevel = TINT8( TREINTERPRETCAST( TUINTPTR, pForceMutateLevel->GetRaw() ) );

	// NOTE: retail doesn't call AInteractionActivity::Create, the category stays CATEGORY_ALL
	return TTRUE;
}

// $Barnyard: FUNCTION 005bb7d0
AInteractionActivity::RESULT APropagateEventActivity::Update( AInteractionSequence* a_pSequence )
{
	m_pEvent = AInteractionEventManager::GetSingleton()->FindEvent( m_pEventClass );

	if ( m_pEvent )
	{
		TINT8 iLevel = GetKnowledgeLevel( m_pEvent, a_pSequence );

		if ( ARandom::GetSingleton()->m_oRandom.GetFloat() < m_fMutateChance )
		{
			// NOTE: the slot returns the highest level of the event
			TINT8 iMaxLevel = TINT8( m_pEvent->GetMaxKnowledgeLevel() );

			if ( m_iForceMutateLevel < 0 )
			{
				if ( iLevel < iMaxLevel )
					iLevel++;
			}
			else
			{
				if ( m_iForceMutateLevel > iMaxLevel )
					m_iForceMutateLevel = iMaxLevel;

				iLevel = m_iForceMutateLevel;
			}
		}

		for ( auto it = a_pSequence->GetCategoryBegin( m_eUnitCategory ); it != a_pSequence->GetCategoryEnd( m_eUnitCategory ); it++ )
		{
			AUnit* pUnit = it->GetOwner();

			if ( pUnit )
				m_pEvent->Notify( pUnit, TUINT8( iLevel ) );
		}

		m_pEvent = TNULL;
	}

	return RESULT_DONE;
}

// $Barnyard: FUNCTION 005bb590
TINT8 APropagateEventActivity::GetKnowledgeLevel( AInteractionEvent* a_pEvent, AInteractionSequence* a_pSequence )
{
	for ( auto it = a_pSequence->GetComponents().Begin(); it != a_pSequence->GetComponents().End(); it++ )
	{
		AUnit* pUnit = it->GetOwner();

		if ( pUnit )
		{
			AUnitGameEventComponent* pComponent = TSTATICCAST( AUnitGameEventComponent, pUnit->FindComponent( &TGetClass( AUnitGameEventComponent ) ) );

			if ( pComponent && pComponent->KnowsEvent( a_pEvent ) )
			{
				return pComponent->GetEventKnowledge( a_pEvent );
			}
		}
	}

	return 0;
}
