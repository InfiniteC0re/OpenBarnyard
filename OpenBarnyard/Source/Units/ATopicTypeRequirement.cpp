#include "pch.h"
#include "ATopicTypeRequirement.h"
#include "AInteractionEvent.h"
#include "AUnit.h"
#include "AUnitGameEventComponent.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b1080, CreateObject 005b10a0, CreateObjectInPlace 005b1120
TDEFINE_CLASS( ATopicTypeRequirement );

// $Barnyard: FUNCTION 005b1090
// $Barnyard: FUNCTION 005b1190
ATopicTypeRequirement::~ATopicTypeRequirement()
{
}

// $Barnyard: FUNCTION 005b1280
TBOOL ATopicTypeRequirement::Create( const PBProperties* a_pProperties )
{
	for ( auto it = a_pProperties->Begin(); it != a_pProperties->End(); it++ )
	{
		if ( TStringManager::String8CompareNoCase( it->GetName().GetString(), "type" ) == 0 )
			m_uiTopics |= AInteractionEvent::GetTopicFlag( it->GetValue()->GetTPString8() );
		else if ( TStringManager::String8CompareNoCase( it->GetName().GetString(), "notopicoftype" ) == 0 )
			m_uiNoTopics |= AInteractionEvent::GetTopicFlag( it->GetValue()->GetTPString8() );
	}

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005b11b0
void ATopicTypeRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	TINT   iNumUnits = a_pParticipants->iNumUnits;
	TFLOAT fWeight   = 0.0f;

	for ( TINT i = 0; i < iNumUnits; i++ )
	{
		AUnit* pUnit    = a_pParticipants->Get( i );
		TBOOL  bAllowed = TFALSE;

		AUnitGameEventComponent* pComponent = pUnit ? TSTATICCAST( AUnitGameEventComponent, pUnit->FindComponent( &TGetClass( AUnitGameEventComponent ) ) ) : TNULL;

		if ( pComponent )
		{
			bAllowed = pComponent->HasEventWithAllTopics( m_uiTopics );

			if ( m_uiNoTopics != 0 )
			{
				if ( m_uiTopics == 0 )
					bAllowed = TTRUE;

				if ( pComponent->HasEventWithAnyTopic( m_uiNoTopics ) )
				{
					SetUnitAllowed( m_eCategory, i, TFALSE );
					continue;
				}
			}

			if ( bAllowed )
				fWeight = 1.0f;
		}

		SetUnitAllowed( m_eCategory, i, bAllowed );
	}

	m_oRoles.fWeight = ( m_bInvert == TTRUE ) ? 1.0f - fWeight : fWeight;
}
