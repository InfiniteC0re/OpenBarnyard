#include "pch.h"
#include "AUnitExistsRequirement.h"
#include "AUnitManager.h"
#include "AUnitType.h"
#include "AUnitFactory.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b17a0, CreateObject 005b1810
// NOTE: GetClass 005b17a0, CreateObject 005b1810, CreateObjectInPlace 005b1890
TDEFINE_CLASS( AUnitExistsRequirement );

TPSTRING8_DECLARE( unittype );
TPSTRING8_DECLARE( minpopulation );
TPSTRING8_DECLARE( maxpopulation );

AUnitExistsRequirement::AUnitExistsRequirement()
    : m_pUnitType( TNULL )
    , m_iMinPopulation( 1 )
    , m_iMaxPopulation( -1 )
{
}

// $Barnyard: FUNCTION 005b17b0
// $Barnyard: FUNCTION 005b1910
AUnitExistsRequirement::~AUnitExistsRequirement()
{
}

// $Barnyard: FUNCTION 005b1930
TBOOL AUnitExistsRequirement::Create( const PBProperties* a_pProperties )
{
	TPString8 strUnitType;
	a_pProperties->GetOptionalPropertyValue( strUnitType, TPS8( unittype ) );

	m_pUnitType = AUnitManager::GetSingleton()->GetUnitFactory()->Find( strUnitType );

	a_pProperties->GetOptionalPropertyValue( m_iMinPopulation, TPS8( minpopulation ) );
	a_pProperties->GetOptionalPropertyValue( m_iMaxPopulation, TPS8( maxpopulation ) );

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005b17c0
void AUnitExistsRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	TFLOAT fWeight = 0.0f;

	if ( m_pUnitType )
	{
		TINT  iNumInstances = m_pUnitType->GetInstanceCount();
		TBOOL bMinReached   = m_iMinPopulation < 0 || m_iMinPopulation <= iNumInstances;

		if ( m_iMaxPopulation < 0 )
		{
			if ( bMinReached )
				fWeight = 1.0f;
		}
		else if ( bMinReached && iNumInstances <= m_iMaxPopulation )
		{
			fWeight = 1.0f;
		}
	}

	if ( m_bInvert == TTRUE )
		fWeight = 1.0f - fWeight;

	m_oRoles.fWeight = fWeight;
}
