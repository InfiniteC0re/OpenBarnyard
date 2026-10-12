#include "pch.h"
#include "AUnitTypeRequirement.h"
#include "AUnitManager.h"
#include "AUnitType.h"
#include "AVisibleUnit.h"
#include "AUnitFactory.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b1ca0, CreateObject 005b20f0
// NOTE: GetClass 005b1ca0, CreateObject 005b20f0, CreateObjectInPlace 005b2170
TDEFINE_CLASS( AUnitTypeRequirement );

TPSTRING8_DECLARE( match );
TPSTRING8_DECLARE( skin );
TPSTRING8_DECLARE( model );
TPSTRING8_DECLARE( class );

AUnitTypeRequirement::AUnitTypeRequirement()
    : m_pUnitType( TNULL )
    , m_iSkin( -1 )
    , m_pClass( TNULL )
{
}

// $Barnyard: FUNCTION 005b1da0
// $Barnyard: FUNCTION 005b21f0
AUnitTypeRequirement::~AUnitTypeRequirement()
{
}

// $Barnyard: FUNCTION 005b1e10
TBOOL AUnitTypeRequirement::Create( const PBProperties* a_pProperties )
{
	for ( auto it = a_pProperties->Begin(); it != a_pProperties->End(); it++ )
	{
		const PBPropertyValue* pValue = it->GetValue();
		TPString8              strName( it->GetName().GetString() );

		if ( strName == TPS8( match ) )
		{
			m_pUnitType = AUnitManager::GetSingleton()->GetUnitFactory()->Find( pValue->GetTPString8() );
		}
		else if ( strName == TPS8( skin ) )
		{
			m_iSkin = TINT( TREINTERPRETCAST( TUINTPTR, pValue->GetRaw() ) );
		}
		else if ( strName == TPS8( model ) )
		{
			TString8 strModel( "data/models/" );
			strModel += pValue->GetString();
			strModel += ".tmd";

			m_strModel = TPString8( strModel.GetString() );
		}
		else if ( strName == TPS8( class ) )
		{
			m_pClass = TClass::Find( pValue->GetString(), &TGetClass( AUnit ) );
		}
	}

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005b20c0
void AUnitTypeRequirement::GetUnitTypeNames( AInteractionUnitTypeNames* a_pNames )
{
	AUnitType* pUnitType = m_pUnitType;

	if ( !pUnitType )
		pUnitType = AUnitManager::GetSingleton()->FindUnit( "lamb" );

	a_pNames->Set( m_eCategory, pUnitType->GetName() );
}

// $Barnyard: FUNCTION 005b1cb0
void AUnitTypeRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	// The primary unit's type if no type is set
	AUnitType* pUnitType = m_pUnitType;

	if ( !pUnitType )
		pUnitType = a_pParticipants->Get( 0 )->GetType();

	TINT iNumUnits = a_pParticipants->iNumUnits;

	for ( TINT i = 0; i < iNumUnits; i++ )
	{
		AUnit* pUnit  = a_pParticipants->Get( i );
		TBOOL  bMatch = pUnitType == pUnit->GetType();

		if ( m_pClass )
			bMatch = bMatch && pUnit->GetClass()->IsA( m_pClass );

		if ( !m_strModel.IsEmpty() )
		{
			if ( pUnit->GetClass()->IsA( &TGetClass( AVisibleUnit ) ) )
				bMatch &= TSTATICCAST( AVisibleUnit, pUnit )->GetModelInstance()->GetModelName() == m_strModel;
			else
				bMatch = TFALSE;
		}

		SetUnitAllowed( m_eCategory, i, bMatch );
	}
}
