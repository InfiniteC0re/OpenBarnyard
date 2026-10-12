#include "pch.h"
#include "AReputationActivity.h"
#include "AInteractionComponent.h"
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

// NOTE: GetClass 005bc9d0, CreateObject 005bcac0, CreateObjectInPlace 005bcb40
TDEFINE_CLASS( AReputationActivity );

// NOTE: inlined into CreateObject 005bcac0 / CreateObjectInPlace 005bcb40
AReputationActivity::AReputationActivity()
    : m_uiReputations( 0 )
    , m_eFlags( FLAGS_NONE )
{
}

// $Barnyard: FUNCTION 005bc9e0
// $Barnyard: FUNCTION 005bc9f0
AReputationActivity::~AReputationActivity()
{
}

// $Barnyard: FUNCTION 005bcbb0
TBOOL AReputationActivity::Create( const PBProperties* a_pProperties )
{
	m_uiReputations = 0;

	AUnitFlagSet* pFlagSet = AUnitFlagSetsManager::GetSingleton()->GetFlagSet( AUnitFlagSetsManager::FLAGSET_REPUTATIONS );

	for ( auto it = a_pProperties->Begin(); it != a_pProperties->End(); it++ )
	{
		if ( TStringManager::String8CompareNoCase( it->GetName().GetString(), "flags" ) == 0 )
		{
			TINT iFlag = pFlagSet->GetFlagIndex( it->GetValue()->GetTPString8() );
			m_uiReputations |= 1U << ( iFlag & 31 );
		}
	}

	TPString8 strAction;
	a_pProperties->GetOptionalPropertyValue( strAction, TPS8D( "action" ) );

	if ( strAction == TPS8D( "add" ) )
		m_eFlags |= FLAGS_ADD;
	else if ( strAction == TPS8D( "remove" ) )
		m_eFlags |= FLAGS_REMOVE;

	TBOOL bPlayerReputation;

	if ( a_pProperties->GetOptionalPropertyValue( bPlayerReputation, TPS8D( "playerreputation" ) ) && bPlayerReputation )
		m_eFlags |= FLAGS_PLAYER_REPUTATION;

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005bca10
AInteractionActivity::RESULT AReputationActivity::Update( AInteractionSequence* a_pSequence )
{
	FLAGS eFlags = m_eFlags;

	if ( eFlags & ( FLAGS_ADD | FLAGS_REMOVE ) )
	{
		for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
		{
			AUnitProfileComponent* pProfile = TSTATICCAST( AUnitProfileComponent, it->GetOwner()->FindComponent( &TGetClass( AUnitProfileComponent ) ) );

			if ( pProfile )
			{
				TINT  iIndex       = ( m_eFlags & FLAGS_PLAYER_REPUTATION ) ? 1 : 0;
				TUINT uiReputation = pProfile->GetReputations( iIndex );

				if ( eFlags & FLAGS_ADD )
					uiReputation |= m_uiReputations;
				else
					uiReputation &= ~m_uiReputations;

				pProfile->SetReputations( iIndex, uiReputation );
			}
		}
	}

	return RESULT_DONE;
}
