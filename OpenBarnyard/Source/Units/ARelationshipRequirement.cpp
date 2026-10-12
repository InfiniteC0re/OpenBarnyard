#include "pch.h"
#include "ARelationshipRequirement.h"
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

// NOTE: GetClass 005af080, CreateObject 005af180
// NOTE: GetClass 005af080, CreateObject 005af180, CreateObjectInPlace 005af1f0
TDEFINE_CLASS( ARelationshipRequirement );

// $Barnyard: FUNCTION 005af090
// $Barnyard: FUNCTION 005af0a0
ARelationshipRequirement::~ARelationshipRequirement()
{
}

// $Barnyard: FUNCTION 005af260
TBOOL ARelationshipRequirement::Create( const PBProperties* a_pProperties )
{
	AUnitFlagSet* pRelationships = AUnitFlagSetsManager::GetSingleton()->GetFlagSet( AUnitFlagSetsManager::FLAGSET_RELATIONSHIPS );
	m_uiFlags                    = 0;

	for ( auto it = a_pProperties->Begin(); it != a_pProperties->End(); it++ )
	{
		if ( TStringManager::String8CompareNoCase( it->GetName().GetString(), "flags" ) == 0 )
		{
			TINT iFlag = pRelationships->GetFlagIndex( it->GetValue()->GetTPString8() );

			if ( pRelationships->CanSetFlag( m_uiFlags, iFlag ) )
				m_uiFlags |= 1 << iFlag;
		}
	}

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005af0c0
void ARelationshipRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	TBOOL bNoFlags  = m_uiFlags == 0;
	TINT  iNumUnits = a_pParticipants->iNumUnits;

	for ( TINT i = 0; i < iNumUnits; i++ )
	{
		AUnitProfileComponent* pProfile = TSTATICCAST( AUnitProfileComponent, a_pParticipants->Get( i )->FindComponent( &TGetClass( AUnitProfileComponent ) ) );
		TBOOL                  bAllowed = TFALSE;

		if ( pProfile )
		{
			if ( bNoFlags )
				bAllowed = pProfile->GetRelationships() == 0;
			else
				bAllowed = ( m_uiFlags & pProfile->GetRelationships() ) == m_uiFlags;
		}

		SetUnitAllowed( m_eCategory, i, bAllowed );
	}
}
