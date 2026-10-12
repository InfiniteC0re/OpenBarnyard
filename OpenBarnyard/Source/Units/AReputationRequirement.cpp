#include "pch.h"
#include "AReputationRequirement.h"
#include "AUnit.h"
#include "AUnitFlagSetsManager.h"
#include "AUnitProfileComponent.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005af580, CreateObject 005af650
// NOTE: GetClass 005af580, CreateObject 005af650, CreateObjectInPlace 005af6d0
TDEFINE_CLASS( AReputationRequirement );

// $Barnyard: FUNCTION 005af590
// $Barnyard: FUNCTION 005af5a0
AReputationRequirement::~AReputationRequirement()
{
}

// $Barnyard: FUNCTION 005af740
TBOOL AReputationRequirement::Create( const PBProperties* a_pProperties )
{
	AUnitFlagSet* pReputations = AUnitFlagSetsManager::GetSingleton()->GetFlagSet( AUnitFlagSetsManager::FLAGSET_REPUTATIONS );

	for ( auto it = a_pProperties->Begin(); it != a_pProperties->End(); it++ )
	{
		if ( TStringManager::String8CompareNoCase( it->GetName().GetString(), "flags" ) == 0 )
			m_uiReputation |= 1 << pReputations->GetFlagIndex( it->GetValue()->GetTPString8() );
	}

	TBOOL bPlayerReputation;

	if ( a_pProperties->GetOptionalPropertyValue( bPlayerReputation, TPS8D( "playerreputation" ) ) && bPlayerReputation )
		m_uiFlags |= FLAGS_PLAYER_REPUTATION;

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005af5c0
void AReputationRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	TINT iNumUnits = a_pParticipants->iNumUnits;

	for ( TINT i = 0; i < iNumUnits; i++ )
	{
		AUnitProfileComponent* pProfile = TSTATICCAST( AUnitProfileComponent, a_pParticipants->Get( i )->FindComponent( &TGetClass( AUnitProfileComponent ) ) );
		TBOOL                  bAllowed = TFALSE;

		if ( pProfile )
		{
			TUINT uiReputation = pProfile->GetReputations( ( m_uiFlags & FLAGS_PLAYER_REPUTATION ) ? 1 : 0 );
			bAllowed           = ( m_uiReputation & uiReputation ) == m_uiReputation;
		}

		SetUnitAllowed( m_eCategory, i, bAllowed );
	}
}
