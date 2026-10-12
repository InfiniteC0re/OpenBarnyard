#include "pch.h"
#include "ACloseInstanceRequirement.h"
#include "AUnit.h"
#include "UnitMovement/Steering/ASteer.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005a8940, CreateObject 005a8a30
// NOTE: GetClass 005a8940, CreateObject 005a8a30, CreateObjectInPlace 005a8ab0
TDEFINE_CLASS( ACloseInstanceRequirement );

TPSTRING8_DECLARE( maxradius );

// $Barnyard: FUNCTION 005a8950
// $Barnyard: FUNCTION 005a8960
ACloseInstanceRequirement::~ACloseInstanceRequirement()
{
}

// $Barnyard: FUNCTION 005a8b20
TBOOL ACloseInstanceRequirement::Create( const PBProperties* a_pProperties )
{
	for ( auto it = a_pProperties->Begin(); it != a_pProperties->End(); it++ )
	{
		if ( TStringManager::String8CompareNoCase( it->GetName().GetString(), "type" ) == 0 )
			m_oTypes |= ARegrowthManager::TypeMask::FromName( it->GetValue()->GetTPString8() );
	}

	a_pProperties->GetOptionalFloatOrIntValue( m_fMaxRadius, TPS8( maxradius ) );
	a_pProperties->GetOptionalFloatOrIntValue( m_fFacingThreshold, TPS8D( "facingthresh" ) );

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005a8980
void ACloseInstanceRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	TINT iNumUnits = a_pParticipants->iNumUnits;

	for ( TINT i = 0; i < iNumUnits; i++ )
	{
		ASteer*         pSteer    = a_pParticipants->Get( i )->GetSteer();
		const TVector4* pPosition = pSteer->GetPosition();

		if ( pSteer )
		{
			TVector2 vecDirection( pSteer->GetDirection()->x, pSteer->GetDirection()->z );

			TINT  iType;
			TBOOL bFound = ARegrowthManager::GetSingleton()->FindObjectInFront( *pPosition, vecDirection, m_oTypes, iType, m_fMaxRadius, m_fFacingThreshold ) != TNULL;

			SetUnitAllowed( m_eCategory, i, bFound );
		}
	}
}
