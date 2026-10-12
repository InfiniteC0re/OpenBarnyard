#include "pch.h"
#include "AAddUnitsActivity.h"
#include "AInteraction.h"
#include "AInteractionComponent.h"
#include "AUnit.h"
#include "AUnitManager.h"
#include "AUnitNameManager.h"
#include "UnitMovement/Steering/ASteer.h"
#include "Player/APlayerManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b23e0, CreateObject 005b26e0
// NOTE: GetClass 005b23e0, CreateObject 005b26e0, CreateObjectInPlace 005b2760
TDEFINE_CLASS( AAddUnitsActivity );

TPSTRING8_DECLARE( unitname );
TPSTRING8_DECLARE( force );
TPSTRING8_DECLARE( block );
TPSTRING8_DECLARE( player );

AAddUnitsActivity::AAddUnitsActivity()
    : m_uiFlags( FLAGS_NONE )
{
}

// $Barnyard: FUNCTION 005b23f0
// $Barnyard: FUNCTION 005b27d0
AAddUnitsActivity::~AAddUnitsActivity()
{
}

// $Barnyard: FUNCTION 005b2460
TBOOL AAddUnitsActivity::Create( const PBProperties* a_pProperties )
{
	a_pProperties->GetOptionalPropertyValue( m_strUnitName, TPS8( unitname ) );

	TBOOL bValue;

	if ( a_pProperties->GetOptionalPropertyValue( bValue, TPS8( force ) ) && bValue )
		m_uiFlags |= FLAGS_FORCE;

	if ( a_pProperties->GetOptionalPropertyValue( bValue, TPS8( block ) ) && bValue )
		m_uiFlags |= FLAGS_BLOCK;

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005b2500
AInteractionActivity::RESULT AAddUnitsActivity::Update( AInteractionSequence* a_pSequence )
{
	if ( m_strUnitName.IsEmpty() )
	{
		// Add the first suitable unit around the primary unit
		AUnit* pPrimaryUnit = a_pSequence->GetUnit( 0 );

		if ( !pPrimaryUnit )
			return RESULT_DONE;

		TINT                    iNumFound = 0;
		AUnitHash::QueryResult* pFound    = AUnitManager::GetSingleton()->FindUnits( *pPrimaryUnit->GetSteer()->GetPosition(), 7.0f, &iNumFound, TFALSE, 16, TFALSE, TNULL );

		for ( TUINT i = 0; i < TUINT( iNumFound ); i++ )
		{
			AUnit* pUnit = pFound[ i ].pUnit;

			if ( pUnit != pPrimaryUnit && AInteractionComponent::IsUnitSuitableForInteraction( pUnit, TFALSE ) )
			{
				a_pSequence->AddUnit( pUnit );
				break;
			}
		}

		return RESULT_DONE;
	}

	AUnit* pUnit = TNULL;

	if ( m_strUnitName == TPS8( player ) )
	{
		pUnit = APlayerManager::GetSingleton()->GetPlayerSet( 0 )->GetHumanPlayerUnit( 0 );
	}
	else
	{
		pUnit = AUnitNameManager::GetSingleton()->RetrieveByName( m_strUnitName );
	}

	TBOOL bForce = ( m_uiFlags & FLAGS_FORCE ) != 0;
	TBOOL bBlock = ( m_uiFlags & FLAGS_BLOCK ) != 0;

	if ( bForce )
	{
		AInteractionComponent* pComponent = TSTATICCAST( AInteractionComponent, pUnit->FindComponent( &TGetClass( AInteractionComponent ) ) );

		if ( pComponent )
			pComponent->LeaveSequence( TTRUE );
	}

	TBOOL bCanAdd = TFALSE;

	// The player can be added while it's free
	if ( pUnit->IsPlayer() && !pUnit->HasInteraction() && !pUnit->IsDying() && !pUnit->Unk_0x4c() )
	{
		AInteractionComponent* pComponent = TSTATICCAST( AInteractionComponent, pUnit->FindComponent( &TGetClass( AInteractionComponent ) ) );

		if ( pComponent && pComponent->GetSequence() == TNULL && !pComponent->IsInteractionsDisabled() )
			bCanAdd = TTRUE;
	}

	if ( bCanAdd || AInteractionComponent::IsUnitSuitableForInteraction( pUnit, bForce ) )
	{
		a_pSequence->AddUnit( pUnit );

		if ( a_pSequence->GetInteraction() )
			a_pSequence->GetInteraction()->IncreaseMaxUnits();

		return RESULT_DONE;
	}

	// Wait for the unit unless it's forced
	if ( bBlock && !bForce )
		return RESULT_RUNNING;

	return RESULT_DONE;
}
