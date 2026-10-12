#include "pch.h"
#include "ARelationshipActivity.h"
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

// NOTE: GetClass 005bc380, CreateObject 005bc470, CreateObjectInPlace 005bc4f0
TDEFINE_CLASS( ARelationshipActivity );

// NOTE: inlined into CreateObject 005bc470 / CreateObjectInPlace 005bc4f0
ARelationshipActivity::ARelationshipActivity()
    : m_uiFlags( 0 )
    , m_uiGroupFlags( 0 )
    , m_eAction( ACTION_NONE )
{
}

// $Barnyard: FUNCTION 005bc390
// $Barnyard: FUNCTION 005bc3a0
ARelationshipActivity::~ARelationshipActivity()
{
}

// $Barnyard: FUNCTION 005bc560
TBOOL ARelationshipActivity::Create( const PBProperties* a_pProperties )
{
	AUnitFlagSet* pFlagSet = AUnitFlagSetsManager::GetSingleton()->GetFlagSet( AUnitFlagSetsManager::FLAGSET_RELATIONSHIPS );

	m_uiFlags      = 0;
	m_uiGroupFlags = 0;

	for ( auto it = a_pProperties->Begin(); it != a_pProperties->End(); it++ )
	{
		if ( TStringManager::String8CompareNoCase( it->GetName().GetString(), "flags" ) == 0 )
		{
			TINT iFlag = pFlagSet->GetFlagIndex( it->GetValue()->GetTPString8() );

			// Only one flag of a group
			if ( pFlagSet->CanSetFlag( m_uiFlags, iFlag ) )
			{
				m_uiFlags |= 1U << ( iFlag & 31 );
				pFlagSet->SetGroupFlags( m_uiGroupFlags, iFlag );
			}
		}
	}

	TPString8 strAction;
	a_pProperties->GetOptionalPropertyValue( strAction, TPS8D( "action" ) );

	if ( strAction == TPS8D( "add" ) )
	{
		m_eAction |= ACTION_ADD;
		m_uiFlags |= 1U << ( pFlagSet->GetFlagIndex( TPS8D( "know" ) ) & 31 );
	}
	else if ( strAction == TPS8D( "remove" ) )
	{
		m_eAction |= ACTION_REMOVE;
	}

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005bc3c0
AInteractionActivity::RESULT ARelationshipActivity::Update( AInteractionSequence* a_pSequence )
{
	ACTION eAction = m_eAction;

	if ( eAction & ( ACTION_ADD | ACTION_REMOVE ) )
	{
		for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
		{
			AUnitProfileComponent* pProfile = TSTATICCAST( AUnitProfileComponent, it->GetOwner()->FindComponent( &TGetClass( AUnitProfileComponent ) ) );

			if ( pProfile )
			{
				TUINT uiRelationships = pProfile->GetRelationships() & ~m_uiGroupFlags;

				if ( eAction & ACTION_ADD )
					uiRelationships |= m_uiFlags;
				else
					uiRelationships &= ~m_uiFlags;

				pProfile->SetRelationships( uiRelationships );
			}
		}
	}

	return RESULT_DONE;
}
