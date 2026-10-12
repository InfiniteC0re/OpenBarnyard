#include "pch.h"
#include "AFacingActivity.h"
#include "AUnit.h"
#include "AUnitManager.h"
#include "AUnitFactory.h"
#include "AUnitType.h"
#include "Player/APlayerManager.h"
#include "UnitMovement/Steering/ASteer.h"
#include "UnitMovement/Steering/ACommonSteer.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorArea.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"

// NOTE: the physics headers pull in Opcode's PI macro, keep them last
#include "Helpers/AGateManager.h"
#include "Helpers/ALadderBeansGame.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/ASimState.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b5180, CreateObject 005b5c10
// NOTE: GetClass 005b5180, CreateObject 005b5c10, CreateObjectInPlace 005b5c90
TDEFINE_CLASS( AFacingActivity );

TPSTRING8_DECLARE( faceclosestpadlock );
TPSTRING8_DECLARE( secondary );
TPSTRING8_DECLARE( primary );
TPSTRING8_DECLARE( search );
TPSTRING8_DECLARE( network );
TPSTRING8_DECLARE( interaction );
TPSTRING8_DECLARE( goal );
TPSTRING8_DECLARE( goalfacing );
TPSTRING8_DECLARE( area );
TPSTRING8_DECLARE( milkweed );
TPSTRING8_DECLARE( player );
TPSTRING8_DECLARE( ASimState );

// NOTE: retail leaves m_pTargetType, m_fMaxRadius and m_eTargetCategory uninitialised
AFacingActivity::AFacingActivity()
    : m_uiFlags( FLAGS_NONE )
{
}

// $Barnyard: FUNCTION 005b5770
// $Barnyard: FUNCTION 005b5d00
AFacingActivity::~AFacingActivity()
{
}

// $Barnyard: FUNCTION 005b5820
TBOOL AFacingActivity::Create( const PBProperties* a_pProperties )
{
	m_pTargetType = TNULL;

	TPString8 strTarget;
	a_pProperties->GetOptionalPropertyValue( strTarget, TPS8D( "target" ) );

	if ( strTarget == TPS8( faceclosestpadlock ) )
	{
		m_uiFlags |= FLAGS_FACE_CLOSEST_PADLOCK;
	}
	else if ( strTarget == TPS8( secondary ) )
	{
		a_pProperties->GetOptionalFloatOrIntValue( m_fMaxRadius, "maxradius" );
		m_uiFlags |= FLAGS_SECONDARY;
	}
	else if ( strTarget == TPS8( primary ) )
	{
		a_pProperties->GetOptionalFloatOrIntValue( m_fMaxRadius, "maxradius" );
		m_uiFlags |= FLAGS_PRIMARY;
	}
	else if ( strTarget == TPS8( search ) )
	{
		a_pProperties->GetOptionalPropertyValue( strTarget, TPS8D( "targettype" ) );
		m_pTargetType = AUnitManager::GetSingleton()->GetUnitFactory()->Find( strTarget );

		a_pProperties->GetOptionalFloatOrIntValue( m_fMaxRadius, "maxradius" );
		m_uiFlags |= FLAGS_SEARCH;
	}
	else if ( strTarget == TPS8( network ) )
	{
		a_pProperties->GetOptionalPropertyValue( m_strNetwork, TPS8D( "network" ) );
		a_pProperties->GetOptionalPropertyValue( m_strArea, TPS8D( "area" ) );
		m_uiFlags |= FLAGS_NETWORK;
	}
	else if ( strTarget == TPS8( interaction ) )
	{
		// NOTE: without "targetcategory" the category is looked up from "interaction"
		a_pProperties->GetOptionalPropertyValue( strTarget, TPS8D( "targetcategory" ) );
		m_eTargetCategory = AInteractionSequence::GetCategoryFromName( strTarget.GetString() );
		m_uiFlags |= FLAGS_INTERACTION;
	}
	else if ( strTarget == TPS8( goal ) )
	{
		a_pProperties->GetOptionalPropertyValue( m_strGoalType, TPS8D( "goaltype" ) );
		m_uiFlags |= FLAGS_GOAL;
	}
	else if ( strTarget == TPS8( goalfacing ) )
	{
		a_pProperties->GetOptionalPropertyValue( m_strGoalType, TPS8D( "goaltype" ) );
		m_uiFlags |= FLAGS_GOAL_FACING;
	}
	else if ( strTarget == TPS8( area ) )
	{
		m_uiFlags |= FLAGS_AREA;
	}
	else if ( strTarget == TPS8( milkweed ) )
	{
		m_uiFlags |= FLAGS_MILKWEED;
	}

	TBOOL bBlock;
	if ( a_pProperties->GetOptionalPropertyValue( bBlock, TPS8D( "block" ) ) && bBlock )
		m_uiFlags |= FLAGS_BLOCK;

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005b5370
AInteractionActivity::RESULT AFacingActivity::OnStart( AInteractionSequence* a_pSequence )
{
	TVector4 vTarget = TVector4::VEC_ZERO;
	TBOOL    bBlock  = HASANYFLAG( m_uiFlags, FLAGS_BLOCK );

	// NOTE: retail only computes the center of the category for these two, the units aren't turned
	if ( HASANYFLAG( m_uiFlags, FLAGS_SECONDARY ) )
	{
		GetCategoryCenter( vTarget, a_pSequence, AInteractionSequence::CATEGORY_SECONDARY );
		return bBlock ? RESULT_RUNNING : RESULT_DONE;
	}

	if ( HASANYFLAG( m_uiFlags, FLAGS_PRIMARY ) )
	{
		GetCategoryCenter( vTarget, a_pSequence, AInteractionSequence::CATEGORY_PRIMARY );
		return bBlock ? RESULT_RUNNING : RESULT_DONE;
	}

	if ( HASANYFLAG( m_uiFlags, FLAGS_SEARCH ) )
	{
		GetCategoryCenter( vTarget, a_pSequence, m_eCategory );

		if ( !m_pTargetType )
			return RESULT_DONE;

		AUnit* pClosestUnit = FindClosestUnit( vTarget, m_pTargetType, m_fMaxRadius );

		if ( !pClosestUnit )
			return RESULT_DONE;

		vTarget = *pClosestUnit->GetSteer()->GetPosition();
	}
	else if ( HASANYFLAG( m_uiFlags, FLAGS_NETWORK ) )
	{
		AInteriorNetwork* pNetwork;
		AInteriorArea*    pArea = AInteriorNetworkManager::GetSingleton()->FindArea( m_strNetwork, m_strArea, pNetwork );

		pArea->GetPosition( vTarget );
	}
	else if ( HASANYFLAG( m_uiFlags, FLAGS_INTERACTION ) )
	{
		if ( !GetCategoryCenter( vTarget, a_pSequence, m_eTargetCategory ) )
			return RESULT_DONE;
	}
	else if ( HASANYFLAG( m_uiFlags, FLAGS_GOAL ) )
	{
		AInteriorGoal* pGoal = AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork()->FindGoal( m_strGoalType );
		pGoal->GetPosition( vTarget );
	}
	else if ( HASANYFLAG( m_uiFlags, FLAGS_GOAL_FACING ) )
	{
		// A point in front of the goal
		AInteriorGoal* pGoal    = AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork()->FindGoal( m_strGoalType );
		TVector4       vForward = pGoal->m_vForward;

		TVector4 vGoalPosition;
		pGoal->GetPosition( vGoalPosition );

		vTarget.Add( vForward, vGoalPosition );
	}
	else if ( HASANYFLAG( m_uiFlags, FLAGS_AREA ) )
	{
		// NOTE: retail doesn't check if the category is empty
		AUnit* pUnit = AInteractionSequence::GetOwner( a_pSequence->GetCategoryBegin( m_eCategory ) );

		if ( !pUnit->GetSteer()->IsA( &TGetClass( ACommonSteer ) ) )
			return RESULT_DONE;

		AInteriorArea* pArea = TSTATICCAST( ACommonSteer, pUnit->GetSteer() )->GetCurrentArea();

		if ( !pArea )
			return RESULT_DONE;

		pArea->GetPosition( vTarget );
	}
	else if ( HASANYFLAG( m_uiFlags, FLAGS_MILKWEED ) )
	{
		// NOTE: retail doesn't check if the category is empty
		AUnit* pUnit = AInteractionSequence::GetOwner( a_pSequence->GetCategoryBegin( m_eCategory ) );

		if ( !pUnit->GetSteer()->IsA( &TGetClass( ACommonSteer ) ) )
			return RESULT_DONE;

		AInteriorArea* pArea = TSTATICCAST( ACommonSteer, pUnit->GetSteer() )->GetCurrentArea();

		if ( !pArea )
			return RESULT_DONE;

		// Face the milkweed of the patch the unit stands in
		ASimState* pSimState = TSTATICCAST( ASimState, AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ASimState ) ) );

		if ( !pSimState || !pSimState->GetLadderBeansGame() || !ALadderBeansGame::GetMilkweedPosition( pArea, vTarget ) )
			return RESULT_DONE;
	}
	else
	{
		if ( !HASANYFLAG( m_uiFlags, FLAGS_FACE_CLOSEST_PADLOCK ) )
			return bBlock ? RESULT_RUNNING : RESULT_DONE;

		// The padlock the player stands next to
		AGateManager::Gate* pPadlock = AGateManager::GetSingleton()->FindNearbyPadlock( 150.0f );

		if ( !pPadlock )
			return bBlock ? RESULT_RUNNING : RESULT_DONE;

		// NOTE: TVector4( const TVector3& ) (0041cd50) copies the position
		vTarget = pPadlock->GetPosition();
	}

	// Turn the units to the target
	for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
	{
		AUnit* pUnit = it->GetOwner();

		TVector4        vDirection = vTarget;
		const TVector4* pPosition  = pUnit->GetSteer()->GetPosition();

		vDirection.x -= pPosition->x;
		vDirection.z -= pPosition->z;
		vDirection.y = 0.0f;

		TFLOAT fMagnitudeSq = vDirection.MagnitudeSq();

		if ( fMagnitudeSq > 0.0001f )
		{
			vDirection.Divide( TMath::Sqrt( fMagnitudeSq ) );
			pUnit->GetSteer()->HaltFacingDesired( vDirection.AsVector3() );
		}
	}

	return bBlock ? RESULT_RUNNING : RESULT_DONE;
}

// $Barnyard: FUNCTION 005b5190
AInteractionActivity::RESULT AFacingActivity::Update( AInteractionSequence* a_pSequence )
{
	if ( HASANYFLAG( m_uiFlags, FLAGS_BLOCK ) )
	{
		for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
		{
			if ( !it->GetOwner()->GetSteer()->IsFacingDesired( 0.99f ) )
				return RESULT_RUNNING;
		}
	}

	return RESULT_DONE;
}

// $Barnyard: FUNCTION 005b52a0
TBOOL AFacingActivity::GetCategoryCenter( TVector4& a_rOutPosition, AInteractionSequence* a_pSequence, AInteractionSequence::CATEGORY a_eCategory )
{
	a_rOutPosition = TVector4::VEC_ZERO;

	TINT iNumUnits = 0;

	for ( auto it = a_pSequence->GetCategoryBegin( a_eCategory ); it != a_pSequence->GetCategoryEnd( a_eCategory ); it++ )
	{
		const TVector4* pPosition = it->GetOwner()->GetSteer()->GetPosition();

		iNumUnits++;
		a_rOutPosition.x += pPosition->x;
		a_rOutPosition.y += pPosition->y;
		a_rOutPosition.z += pPosition->z;
	}

	if ( iNumUnits > 0 )
	{
		a_rOutPosition.Divide( TFLOAT( iNumUnits ) );
		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005b5220
AUnit* AFacingActivity::FindClosestUnit( const TVector4& a_rcPosition, AUnitType* a_pType, TFLOAT a_fMaxRadius )
{
	if ( a_pType->GetName() != TPS8( player ) )
		return AUnitManager::GetSingleton()->FindClosestUnit( a_rcPosition, a_pType, a_fMaxRadius, TNULL );

	AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerSet( 0 )->GetHumanPlayerUnit( 0 );

	if ( TVector4::DistanceSq( a_rcPosition, *pPlayer->GetSteer()->GetPosition() ) < a_fMaxRadius * a_fMaxRadius )
		return pPlayer;

	return TNULL;
}
