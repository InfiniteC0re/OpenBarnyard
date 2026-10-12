#include "pch.h"
#include "AUnitNameRequirement.h"
#include "AUnitNameManager.h"
#include "AUnit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b19f0, CreateObject 005b1ba0
// NOTE: GetClass 005b19f0, CreateObject 005b1ba0, CreateObjectInPlace 005b1c10
TDEFINE_CLASS( AUnitNameRequirement );

AUnitNameRequirement::AUnitNameRequirement()
{
}

// $Barnyard: FUNCTION 005b1a00
// $Barnyard: FUNCTION 005b1c80
AUnitNameRequirement::~AUnitNameRequirement()
{
}

// $Barnyard: FUNCTION 005b1a70
TBOOL AUnitNameRequirement::Create( const PBProperties* a_pProperties )
{
	const PBPropertyValue* pMatch = a_pProperties->GetProperty( "match" );

	if ( pMatch )
		m_strMatch = pMatch->GetTPString8();

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005b1b30
void AUnitNameRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	TINT iUnitId   = AUnitNameManager::GetSingleton()->GetUnitId( m_strMatch );
	TINT iNumUnits = a_pParticipants->iNumUnits;

	for ( TINT i = 0; i < iNumUnits; i++ )
		SetUnitAllowed( m_eCategory, i, iUnitId == a_pParticipants->Get( i )->GetId() );
}
