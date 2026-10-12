#include "pch.h"
#include "AFacingRequirement.h"
#include "AUnit.h"
#include "AUnitFactory.h"
#include "AUnitManager.h"
#include "AUnitType.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerSet.h"
#include "UnitMovement/Steering/ACommonSteer.h"
#include "UnitMovement/PathfindingNetworks/AInteriorArea.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005a9a00, CreateObject 005aa1e0
// NOTE: GetClass 005a9a00, CreateObject 005aa1e0, CreateObjectInPlace 005aa260
TDEFINE_CLASS( AFacingRequirement );

TPSTRING8_DECLARE( player );

// $Barnyard: FUNCTION 005a9d90
// $Barnyard: FUNCTION 005aa2e0
AFacingRequirement::~AFacingRequirement()
{
}

// $Barnyard: FUNCTION 005a9e00
TBOOL AFacingRequirement::Create( const PBProperties* a_pProperties )
{
	TPString8 strValue;
	m_pTargetType = TNULL;

	a_pProperties->GetOptionalPropertyValue( strValue, TPS8D( "target" ) );

	if ( strValue == TPS8D( "goal" ) )
	{
		a_pProperties->GetOptionalPropertyValue( m_strGoal, TPS8D( "goal" ) );
		m_uiFlags |= FLAGS_GOAL;
	}
	else if ( strValue == TPS8D( "search" ) )
	{
		a_pProperties->GetOptionalPropertyValue( strValue, TPS8D( "targettype" ) );

		AUnitFactory* pUnitFactory = AUnitManager::GetSingleton()->GetUnitFactory();
		m_pTargetType              = pUnitFactory->Find( strValue );
		TASSERT( m_pTargetType && "AFacingRequirement: unknown targettype" );

		if ( m_pTargetType->GetName() != TPS8( player ) )
			a_pProperties->GetOptionalFloatOrIntValue( m_fMaxRadius, TPS8D( "maxradius" ) );

		if ( m_pTargetType == pUnitFactory->Find( TPS8( player ) ) )
			m_uiFlags |= FLAGS_TARGET_PLAYER;

		m_uiFlags |= FLAGS_SEARCH;
	}
	else if ( strValue == TPS8D( "interaction" ) )
	{
		a_pProperties->GetOptionalPropertyValue( strValue, TPS8D( "targetcategory" ) );
		m_eTargetCategory = AInteractionSequence::GetCategoryFromName( strValue.GetString() );
		m_uiFlags |= FLAGS_INTERACTION;
	}

	a_pProperties->GetOptionalPropertyValue( strValue, TPS8D( "facingtype" ) );

	if ( strValue == TPS8D( "targetpos" ) )
		m_uiFlags |= FLAGS_TARGET_POS;
	else if ( strValue == TPS8D( "targetfacing" ) )
		m_uiFlags |= FLAGS_TARGET_FACING;

	a_pProperties->GetOptionalFloatOrIntValue( m_fThreshold, TPS8D( "threshold" ) );

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005a9b30
void AFacingRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	TINT iNumUnits = a_pParticipants->iNumUnits;

	if ( m_uiFlags & FLAGS_GOAL )
	{
		// The player faces the goal
		AUnit*            pPlayer  = APlayerManager::GetSingleton()->GetPlayerSet( 0 )->GetHumanPlayerUnit( 0 );
		AInteriorNetwork* pNetwork = AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork();
		AInteriorArea*    pArea    = TSTATICCAST( ACommonSteer, pPlayer->GetSteer() )->GetCurrentArea();
		TBOOL             bFacing  = TFALSE;

		if ( pArea )
		{
			AInteriorGoal* pGoal = pNetwork->FindGoalInArea( m_strGoal, pArea->m_iIndex );

			if ( pGoal )
			{
				const TVector4* pDirection;
				TVector4        vecDirection;

				if ( m_uiFlags & FLAGS_TARGET_FACING )
				{
					// The goal's direction
					pDirection = &pGoal->m_vForward;
				}
				else
				{
					// The direction to the goal
					pGoal->GetPosition( vecDirection );
					vecDirection.Substract( vecDirection, *pPlayer->GetSteer()->GetPosition() );
					vecDirection.Normalise();

					pDirection = &vecDirection;
				}

				bFacing = IsFacingDirection( pPlayer, *pDirection );
			}
		}

		for ( TINT i = 0; i < iNumUnits; i++ )
		{
			if ( a_pParticipants->Get( i ) == pPlayer )
				SetUnitAllowed( m_eCategory, i, bFacing );
		}
	}
	else if ( m_uiFlags & FLAGS_SEARCH )
	{
		// The units face the player
		if ( m_pTargetType->GetName() != TPS8( player ) )
			return;

		AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerSet( 0 )->GetHumanPlayerUnit( 0 );

		for ( TINT i = 0; i < iNumUnits; i++ )
		{
			AUnit* pUnit   = a_pParticipants->Get( i );
			TBOOL  bFacing = TFALSE;

			if ( pUnit != pPlayer )
				bFacing = IsFacingUnit( pUnit, pPlayer );

			SetUnitAllowed( m_eCategory, i, bFacing );
		}
	}
	else if ( m_uiFlags & FLAGS_INTERACTION )
	{
		// The units of the two categories face each other
		TBOOL bDefault = m_bInvert == TTRUE;

		for ( TINT i = 0; i < iNumUnits; i++ )
		{
			m_oRoles.SetUnit( AInteractionSequence::CATEGORY_PRIMARY, i, bDefault );
			m_oRoles.SetUnit( AInteractionSequence::CATEGORY_SECONDARY, i, bDefault );
			m_oRoles.SetUnit( AInteractionSequence::CATEGORY_AUXILIARY, i, bDefault );
		}

		// NOTE: the categories are used as the indices of the units
		AUnit* pUnit   = a_pParticipants->Get( m_eCategory );
		AUnit* pTarget = a_pParticipants->Get( m_eTargetCategory );

		if ( IsFacingUnit( pUnit, pTarget ) )
		{
			SetUnitAllowed( m_eCategory, m_eCategory, TTRUE );
			SetUnitAllowed( m_eTargetCategory, m_eTargetCategory, TTRUE );
		}

		if ( IsFacingUnit( pTarget, pUnit ) )
		{
			SetUnitAllowed( m_eTargetCategory, m_eCategory, TTRUE );
			SetUnitAllowed( m_eCategory, m_eTargetCategory, TTRUE );
		}
	}
}

// $Barnyard: FUNCTION 005a9a10
TBOOL AFacingRequirement::IsFacingDirection( AUnit* a_pUnit, const TVector4& a_rcDirection )
{
	TVector4 vecDirection = a_rcDirection;
	vecDirection.y        = 0.0f;
	vecDirection.Normalise();

	return TVector4::DotProduct3( *a_pUnit->GetSteer()->GetDirection(), vecDirection ) > m_fThreshold;
}

// $Barnyard: FUNCTION 005a9a80
TBOOL AFacingRequirement::IsFacingUnit( AUnit* a_pUnit, AUnit* a_pTarget )
{
	TVector4 vecDirection = *a_pTarget->GetSteer()->GetPosition();
	vecDirection.Substract( *a_pUnit->GetSteer()->GetPosition() );
	vecDirection.y = 0.0f;
	vecDirection.Normalise();

	return TVector4::DotProduct3( *a_pUnit->GetSteer()->GetDirection(), vecDirection ) > m_fThreshold;
}
