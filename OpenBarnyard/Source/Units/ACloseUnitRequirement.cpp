#include "pch.h"
#include "ACloseUnitRequirement.h"
#include "AUnitManager.h"
#include "AUnitNameManager.h"
#include "AUnitType.h"
#include "AVisibleUnit.h"
#include "UnitMovement/Steering/ASteer.h"
#include "AUnitFactory.h"
#include "Player/APlayerManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005a8cf0, CreateObject 005a9060
// NOTE: GetClass 005a8cf0, CreateObject 005a9060, CreateObjectInPlace 005a90e0
TDEFINE_CLASS( ACloseUnitRequirement );

TPSTRING8_DECLARE( unittype );
TPSTRING8_DECLARE( player );
TPSTRING8_DECLARE( maxradius );
TPSTRING8_DECLARE( unitname );

ACloseUnitRequirement::ACloseUnitRequirement()
    : m_pUnitType( TNULL )
    , m_fMaxRadius( 0.0f )
    , m_uiFlags( FLAGS_NONE )
{
}

// $Barnyard: FUNCTION 005a8d00
// $Barnyard: FUNCTION 005a9150
ACloseUnitRequirement::~ACloseUnitRequirement()
{
}

// $Barnyard: FUNCTION 005a8d70
TBOOL ACloseUnitRequirement::Create( const PBProperties* a_pProperties )
{
	TPString8 strUnitType;
	a_pProperties->GetOptionalPropertyValue( strUnitType, TPS8( unittype ) );

	m_pUnitType = AUnitManager::GetSingleton()->GetUnitFactory()->Find( strUnitType );

	if ( m_pUnitType == AUnitManager::GetSingleton()->GetUnitFactory()->Find( TPS8( player ) ) )
		m_uiFlags |= FLAGS_PLAYER;

	a_pProperties->GetOptionalFloatOrIntValue( m_fMaxRadius, TPS8( maxradius ) );
	a_pProperties->GetOptionalPropertyValue( m_strUnitName, TPS8( unitname ) );

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005a8e50
void ACloseUnitRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	TINT iNumUnits = a_pParticipants->iNumUnits;

	if ( m_uiFlags & FLAGS_PLAYER )
	{
		AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerSet( 0 )->GetHumanPlayerUnit( 0 );

		if ( !pPlayer )
		{
			for ( TINT i = 0; i < iNumUnits; i++ )
				SetUnitAllowed( m_eCategory, i, TFALSE );

			return;
		}

		const TVector4* pPlayerPos   = pPlayer->GetSteer()->GetPosition();
		TFLOAT          fMaxRadiusSq = m_fMaxRadius * m_fMaxRadius;

		for ( TINT i = 0; i < iNumUnits; i++ )
		{
			TBOOL  bClose = TFALSE;
			AUnit* pUnit  = a_pParticipants->Get( i );

			if ( pUnit != pPlayer && TVector4::DistanceSq( *pUnit->GetSteer()->GetPosition(), *pPlayerPos ) < fMaxRadiusSq )
				bClose = TTRUE;

			SetUnitAllowed( m_eCategory, i, bClose );
		}
	}
	else
	{
		for ( TINT i = 0; i < iNumUnits; i++ )
		{
			TBOOL bClose = TFALSE;

			TINT                    iNumFound;
			AUnitHash::QueryResult* pFound = AUnitManager::GetSingleton()->FindUnits( *a_pParticipants->Get( i )->GetSteer()->GetPosition(), m_fMaxRadius, &iNumFound, TFALSE, 420, TFALSE, TNULL );

			for ( TUINT k = 0; k < TUINT( iNumFound ); k++ )
			{
				AUnit* pUnit = pFound[ k ].pUnit;

				if ( pUnit->GetType() != m_pUnitType )
					continue;

				if ( pUnit->IsA( &TGetClass( AVisibleUnit ) ) && !TSTATICCAST( AVisibleUnit, pUnit )->IsVisible() )
					continue;

				if ( m_strUnitName.IsEmpty() || pUnit == AUnitNameManager::GetSingleton()->RetrieveByName( m_strUnitName ) )
				{
					bClose = TTRUE;
					break;
				}
			}

			SetUnitAllowed( m_eCategory, i, bClose );
		}
	}
}
