#include "pch.h"
#include "ASpecificUnitRequirement.h"
#include "AUnit.h"
#include "AUnitFactory.h"
#include "AUnitManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b03f0, CreateObject 005b04c0
// NOTE: GetClass 005b03f0, CreateObject 005b04c0, CreateObjectInPlace 005b0520
TDEFINE_CLASS( ASpecificUnitRequirement );

// $Barnyard: FUNCTION 005b0400
ASpecificUnitRequirement::ASpecificUnitRequirement()
    : m_pUnitType( TNULL )
{
}

// $Barnyard: FUNCTION 005b0460
// $Barnyard: FUNCTION 005b0560
ASpecificUnitRequirement::~ASpecificUnitRequirement()
{
}

// $Barnyard: FUNCTION 005b05d0
TBOOL ASpecificUnitRequirement::Create( const PBProperties* a_pProperties )
{
	const PBPropertyValue* pScreenName = a_pProperties->GetProperty( "screenname" );

	if ( pScreenName )
		m_strScreenName = pScreenName->GetTPString8().GetString();

	const PBPropertyValue* pType = a_pProperties->GetProperty( "type" );

	if ( pType )
		m_pUnitType = AUnitManager::GetSingleton()->GetUnitFactory()->Find( pType->GetTPString8() );

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005b0580
void ASpecificUnitRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	TINT iNumUnits = a_pParticipants->iNumUnits;

	for ( TINT i = 0; i < iNumUnits; i++ )
		SetUnitAllowed( m_eCategory, i, m_pUnitType == a_pParticipants->Get( i )->GetType() );
}
