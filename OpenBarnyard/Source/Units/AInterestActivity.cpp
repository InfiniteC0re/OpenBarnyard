#include "pch.h"
#include "AInterestActivity.h"
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

// NOTE: GetClass 005b83f0, CreateObject 005b84e0, CreateObjectInPlace 005b8560
TDEFINE_CLASS( AInterestActivity );

// NOTE: inlined into CreateObject
AInterestActivity::AInterestActivity()
    : m_uiInterests( 0 )
    , m_uiFlags( FLAGS_NONE )
{
}

// $Barnyard: FUNCTION 005b8400
// $Barnyard: FUNCTION 005b8410
AInterestActivity::~AInterestActivity()
{
}

// $Barnyard: FUNCTION 005b85d0
TBOOL AInterestActivity::Create( const PBProperties* a_pProperties )
{
	m_uiInterests = 0;

	AUnitFlagSet* pFlagSet = AUnitFlagSetsManager::GetSingleton()->GetFlagSet( AUnitFlagSetsManager::FLAGSET_INTERESTS );

	// Every "flags" property adds an interest
	for ( auto it = a_pProperties->Begin(); it != a_pProperties->End(); it++ )
	{
		if ( TStringManager::String8CompareNoCase( it->GetName().GetString(), "flags" ) == 0 )
		{
			// NOTE: an unknown flag (-1) sets bit 31
			TINT iFlag = pFlagSet->GetFlagIndex( it->GetValue()->GetTPString8() );
			m_uiInterests |= 1U << ( iFlag & 31 );
		}
	}

	TPString8 strAction;
	a_pProperties->GetOptionalPropertyValue( strAction, TPS8D( "action" ) );

	if ( strAction == TPS8D( "add" ) )
		m_uiFlags |= FLAGS_ADD;
	else if ( strAction == TPS8D( "remove" ) )
		m_uiFlags |= FLAGS_REMOVE;

	TBOOL bPlayerInterest;

	if ( a_pProperties->GetOptionalPropertyValue( bPlayerInterest, TPS8D( "playerinterest" ) ) && bPlayerInterest )
		m_uiFlags |= FLAGS_PLAYERINTEREST;

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005b8430
AInteractionActivity::RESULT AInterestActivity::Update( AInteractionSequence* a_pSequence )
{
	if ( m_uiFlags & ( FLAGS_ADD | FLAGS_REMOVE ) )
	{
		for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
		{
			AUnitProfileComponent* pProfile = TSTATICCAST( AUnitProfileComponent, it->GetOwner()->FindComponent( &TGetClass( AUnitProfileComponent ) ) );

			if ( pProfile )
			{
				TUINT& rMask = pProfile->GetInterestMask( m_uiFlags & FLAGS_PLAYERINTEREST );

				if ( m_uiFlags & FLAGS_ADD )
					rMask |= m_uiInterests;
				else
					rMask &= ~m_uiInterests;
			}
		}
	}

	return RESULT_DONE;
}
