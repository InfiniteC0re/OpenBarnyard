#include "pch.h"
#include "ARumourRequirement.h"
#include "AInteractionEvent.h"
#include "AInteractionEventManager.h"
#include "AUnit.h"
#include "AUnitGameEventComponent.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005af880, CreateObject 005af8a0, CreateObjectInPlace 005af930
TDEFINE_CLASS( ARumourRequirement );

// $Barnyard: FUNCTION 005af890
// $Barnyard: FUNCTION 005af9b0
ARumourRequirement::~ARumourRequirement()
{
}

// $Barnyard: FUNCTION 005afb00
TBOOL ARumourRequirement::Create( const PBProperties* a_pProperties )
{
	const PBPropertyValue* pEvent = a_pProperties->GetProperty( "event" );

	if ( pEvent )
	{
		TString8 strClassName;
		strClassName.Format( "ainteraction%sevent", pEvent->GetTPString8().GetString() );

		m_pEventClass = TClass::Find( strClassName, &TGetClass( AInteractionEvent ) );

		if ( !m_pEventClass )
			return TFALSE;
	}

	const PBPropertyValue* pOccuring = a_pProperties->GetProperty( "occuring" );

	if ( pOccuring )
		m_bOccuring = pOccuring->GetRaw() != TNULL;

	const PBPropertyValue* pKnowledge = a_pProperties->GetProperty( "knowledge" );

	if ( pKnowledge )
		m_fKnowledge = ( pKnowledge->GetType() == PBPropertyValue::Type::Float ) ? pKnowledge->GetFloat() : TFLOAT( pKnowledge->GetInteger() );

	const PBPropertyValue* pUnitKnows = a_pProperties->GetProperty( "unitknows" );

	if ( pUnitKnows )
		m_eUnitKnows = ( pUnitKnows->GetRaw() == TNULL ) ? UNITKNOWS_NO : UNITKNOWS_YES;

	const PBPropertyValue* pMutateLevel = a_pProperties->GetProperty( "mutatelevel" );

	if ( pMutateLevel )
		m_iMutateLevel = TINT8( pMutateLevel->GetInteger() );

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005af9d0
void ARumourRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	AInteractionEvent* pEvent  = AInteractionEventManager::GetSingleton()->FindEvent( m_pEventClass );
	TFLOAT             fWeight = 1.0f;

	if ( m_bOccuring != ( pEvent != TNULL ) )
	{
		fWeight = 0.0f;
	}
	else if ( pEvent )
	{
		fWeight = pEvent->GetInterest();

		TINT iNumUnits = a_pParticipants->iNumUnits;

		for ( TINT i = 0; i < iNumUnits; i++ )
		{
			AUnit* pUnit    = a_pParticipants->Get( i );
			TBOOL  bAllowed = TTRUE;

			if ( m_eUnitKnows != UNITKNOWS_DONTCARE )
			{
				// NOTE: inlined AInteractionEvent method (005c5650)
				AUnitGameEventComponent* pUnitComponent = TSTATICCAST( AUnitGameEventComponent, pUnit->FindComponent( &TGetClass( AUnitGameEventComponent ) ) );
				TBOOL                    bUnitKnows     = pUnitComponent ? pUnitComponent->KnowsEvent( pEvent ) : TFALSE;

				if ( m_iMutateLevel >= 0 )
				{
					AUnitGameEventComponent* pComponent = TSTATICCAST( AUnitGameEventComponent, pUnit->FindComponent( &TGetClass( AUnitGameEventComponent ) ) );

					if ( pComponent && pComponent->KnowsEvent( pEvent ) && m_iMutateLevel != pComponent->GetEventKnowledge( pEvent ) )
						bAllowed = TFALSE;
				}

				if ( ( !bUnitKnows && m_eUnitKnows == UNITKNOWS_YES ) || ( bUnitKnows && m_eUnitKnows == UNITKNOWS_NO ) )
					bAllowed = TFALSE;
			}

			SetUnitAllowed( m_eCategory, i, bAllowed );
		}
	}

	m_oRoles.fWeight = ( m_bInvert == TTRUE ) ? 1.0f - fWeight : fWeight;
}
