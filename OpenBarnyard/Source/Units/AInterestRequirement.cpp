#include "pch.h"
#include "AInterestRequirement.h"
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

// NOTE: GetClass 005ac170, CreateObject 005ac230
// NOTE: GetClass 005ac170, CreateObject 005ac230, CreateObjectInPlace 005ac2a0
TDEFINE_CLASS( AInterestRequirement );

// $Barnyard: FUNCTION 005ac180
// $Barnyard: FUNCTION 005ac190
AInterestRequirement::~AInterestRequirement()
{
}

// $Barnyard: FUNCTION 005ac310
TBOOL AInterestRequirement::Create( const PBProperties* a_pProperties )
{
	AUnitFlagSet* pInterests = AUnitFlagSetsManager::GetSingleton()->GetFlagSet( AUnitFlagSetsManager::FLAGSET_INTERESTS );

	for ( auto it = a_pProperties->Begin(); it != a_pProperties->End(); it++ )
	{
		if ( TStringManager::String8CompareNoCase( it->GetName().GetString(), "flags" ) == 0 )
			m_uiInterests |= 1 << pInterests->GetFlagIndex( it->GetValue()->GetTPString8() );
	}

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005ac1b0
void AInterestRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	TINT iNumUnits = a_pParticipants->iNumUnits;

	for ( TINT i = 0; i < iNumUnits; i++ )
	{
		AUnitProfileComponent* pProfile = TSTATICCAST( AUnitProfileComponent, a_pParticipants->Get( i )->FindComponent( &TGetClass( AUnitProfileComponent ) ) );
		TBOOL                  bAllowed = TFALSE;

		if ( pProfile )
			bAllowed = ( m_uiInterests & pProfile->GetInterests() ) == m_uiInterests;

		SetUnitAllowed( m_eCategory, i, bAllowed );
	}
}
