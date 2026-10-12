#include "pch.h"
#include "AFarmerTruck.h"
#include "UnitMovement/Steering/ABikeSteer.h"
#include "Animation/AModel.h"
#include "Animation/AAnimatableObject.h"
#include "Player/APlayerManager.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitNameManager.h"
#include "Units/AUnitActor.h"
#include "Units/ASSeqController.h"
#include "UnitMovement/Steering/ASteer.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"
#include "Physics/ACollisionObjectMesh.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( ben );
TPSTRING8_DECLARE( bud );
TPSTRING8_DECLARE( eddie );
TPSTRING8_DECLARE( igg );
TPSTRING8_DECLARE( daisy );
TPSTRING8_DECLARE( miles );
TPSTRING8_DECLARE( pig );
TPSTRING8_DECLARE( bessy );
TPSTRING8_DECLARE( freddy );
TPSTRING8_DECLARE( hanna );
TPSTRING8_DECLARE( otis );
TPSTRING8_DECLARE( peck );
TPSTRING8_DECLARE( twink );
TPSTRING8_DECLARE( duke );
TPSTRING8_DECLARE( root );
TPSTRING8_DECLARE( nb_bull );
TPSTRING8_DECLARE( park2 );
TPSTRING8_DECLARE( simtruckgoal );
TPSTRING8_DECLARE( farmersafepos );

const TPString8* AFarmerTruck::ms_apHiddenCharacters[ NUM_HIDDEN_CHARACTERS ] = {
	&TPS8( ben ),
	&TPS8( bud ),
	&TPS8( eddie ),
	&TPS8( igg ),
	&TPS8( daisy ),
	&TPS8( miles ),
	&TPS8( pig ),
	&TPS8( bessy ),
	&TPS8( freddy ),
	&TPS8( hanna ),
	&TPS8( otis ),
	&TPS8( peck ),
	&TPS8( twink ),
	&TPS8( duke ),
	&TPS8( root ),
	&TPS8( nb_bull ),
};

// $Barnyard: FUNCTION 00534470
TBOOL AFarmerTruck::Create( TBOOL a_bParked )
{
	ms_pInstance = new AFarmerTruck;

	ms_pInstance->m_pTruck               = TNULL;
	ms_pInstance->m_pCollision           = TNULL;
	ms_pInstance->m_uiUnk0D              = 0;
	ms_pInstance->m_bSequenceMode        = TFALSE;
	ms_pInstance->m_uiVisibleCharacters  = 0;
	ms_pInstance->m_bUnk0E               = TFALSE;
	ms_pInstance->m_vSavedPlayerPosition = TVector4::VEC_ZERO;

	Spawn();

	if ( !a_bParked )
	{
		Hide();
		ms_pInstance->m_uiUnk0D = 2;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00534530
void AFarmerTruck::Destroy()
{
	if ( ms_pInstance )
	{
		Despawn();

		if ( ms_pInstance )
			delete ms_pInstance;

		ms_pInstance = TNULL;
	}
}

// $Barnyard: FUNCTION 00534030
TBOOL AFarmerTruck::Spawn()
{
	ASSeqController* pSSeqController = ASSeqController::GetSingleton();
	pSSeqController->LoadScript( "farmer_arive.tso", TVector4::VEC_ZERO );
	pSSeqController->LoadScript( "farmer_depart.tso", TVector4::VEC_ZERO );

	AUnit* pTruck = AUnitManager::GetSingleton()->SpawnUnit( "actor_ss_truck_ntrailer", TVector4::VEC_ZERO, TVector4::VEC_POSX );

	AAnimatableObject* pAnimatableObject = TSTATICCAST( AAnimatedUnit, pTruck )->GetAnimatableObject();
	pAnimatableObject->GetModelInstance()->SetUnlit();

	ms_pInstance->m_pTruck = pTruck->GetSelfProxy();

	Park();

	if ( pAnimatableObject->GetModelInstance()->GetSceneObject()->GetModelRef()->GetModel()->GetNumCollisionMeshes() > 0 )
	{
		ms_pInstance->m_pCollision = new ACollisionObjectMesh();
		ms_pInstance->m_pCollision->Create( pAnimatableObject->GetModelInstance()->GetSceneObject() );
	}

	ms_pInstance->m_uiUnk0D              = 0;
	ms_pInstance->m_bSequenceMode        = TFALSE;
	ms_pInstance->m_uiVisibleCharacters  = 0;
	ms_pInstance->m_bUnk0E               = TFALSE;
	ms_pInstance->m_vSavedPlayerPosition = TVector4::VEC_ZERO;

	return TTRUE;
}

// $Barnyard: FUNCTION 00533db0
void AFarmerTruck::Despawn()
{
	if ( ms_pInstance->m_pCollision )
		delete ms_pInstance->m_pCollision;

	ms_pInstance->m_pCollision = TNULL;

	if ( AUnit* pTruck = GetTruck() )
		AUnitManager::GetSingleton()->DestroyUnit( pTruck );

	ms_pInstance->m_pTruck               = TNULL;
	ms_pInstance->m_bSequenceMode        = TFALSE;
	ms_pInstance->m_bUnk0E               = TFALSE;
	ms_pInstance->m_vSavedPlayerPosition = TVector4::VEC_ZERO;
}

// $Barnyard: FUNCTION 00533e50
void AFarmerTruck::Hide()
{
#ifdef BARNYARD_COMMUNITY_PATCH
	// The original expects the truck to exist
	if ( !ms_pInstance )
		return;
#endif

	if ( !ms_pInstance->m_pTruck )
		return;

	AUnit* pTruck = ms_pInstance->m_pTruck->GetUnit();

	if ( !pTruck )
		return;

	TSTATICCAST( AUnitActor, pTruck )->SetTransform( TMatrix44::IDENTITY );
	TSTATICCAST( AVisibleUnit, pTruck )->SetVisible( TFALSE );

	if ( ms_pInstance && ms_pInstance->m_pCollision && ms_pInstance->m_pCollision->IsCollisionObjectLinked() )
		ms_pInstance->m_pCollision->UnlinkCollisionObject();
}

// $Barnyard: FUNCTION 00533eb0
void AFarmerTruck::Show()
{
#ifdef BARNYARD_COMMUNITY_PATCH
	// The original expects the truck to exist
	if ( !ms_pInstance )
		return;
#endif

	if ( AUnit* pTruck = GetTruck() )
		TSTATICCAST( AVisibleUnit, pTruck )->SetVisible( TTRUE );
}

// $Barnyard: FUNCTION 00533ed0
void AFarmerTruck::Park()
{
	AInteriorGoal* pGoal = AInteriorNetworkManager::GetSingleton()->FindNetwork( TPS8( park2 ) )->FindGoalInArea( TPS8( simtruckgoal ), -1 );

	if ( !pGoal )
		return;

#ifdef BARNYARD_COMMUNITY_PATCH
	// The original expects the truck to exist
	if ( !ms_pInstance )
		return;
#endif

	// The rows are ( forward x down, forward, down, position )
	static constexpr TVector4 s_vUp = TVector4( 0.0f, -1.0f, 0.0f, 0.0f ); // 0x0077d0f4 (only xyz are stored)

	TMatrix44 mTransform = TMatrix44::IDENTITY;
	mTransform.m_f11     = pGoal->m_vForward.y * s_vUp.z - pGoal->m_vForward.z * s_vUp.y;
	mTransform.m_f12     = pGoal->m_vForward.z * s_vUp.x - pGoal->m_vForward.x * s_vUp.z;
	mTransform.m_f13     = pGoal->m_vForward.x * s_vUp.y - pGoal->m_vForward.y * s_vUp.x;
	mTransform.m_f21     = pGoal->m_vForward.x;
	mTransform.m_f22     = pGoal->m_vForward.y;
	mTransform.m_f23     = pGoal->m_vForward.z;
	mTransform.m_f31     = s_vUp.x;
	mTransform.m_f32     = s_vUp.y;
	mTransform.m_f33     = s_vUp.z;

	TVector4 vPosition;
	pGoal->GetPosition( vPosition );
	mTransform.m_f41 = vPosition.x;
	mTransform.m_f42 = vPosition.y;
	mTransform.m_f43 = vPosition.z;
	mTransform.m_f44 = vPosition.w;

	if ( !ms_pInstance->m_pTruck )
		return;

	AUnit* pTruck = ms_pInstance->m_pTruck->GetUnit();

	if ( !pTruck )
		return;

	TSTATICCAST( AUnitActor, pTruck )->SetTransform( mTransform );
	TSTATICCAST( AVisibleUnit, pTruck )->SetVisible( TTRUE );

	if ( ms_pInstance && ms_pInstance->m_pCollision )
	{
		if ( ms_pInstance->m_pCollision->IsCollisionObjectLinked() )
			ms_pInstance->m_pCollision->UnlinkCollisionObject();

		ms_pInstance->m_pCollision->Activate();
	}
}

// $Barnyard: FUNCTION 00533cc0
void AFarmerTruck::MovePlayerToSafePos( TBOOL a_bOnlyIfOnTruckGoal )
{
	AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayer( 0 );

	if ( !pPlayer )
		return;

	const TVector4* pPlayerPosition = pPlayer->GetSteer()->GetPosition();
	const TFLOAT    fPlayerX        = pPlayerPosition->x;
	const TFLOAT    fPlayerZ        = pPlayerPosition->z;

	if ( a_bOnlyIfOnTruckGoal )
	{
		AInteriorGoal* pTruckGoal = AInteriorNetworkManager::GetSingleton()->FindNetwork( TPS8( park2 ) )->FindGoalInArea( TPS8( simtruckgoal ), -1 );

		if ( !pTruckGoal )
			return;

		TVector4 vGoalPosition;
		pTruckGoal->GetPosition( vGoalPosition );

		if ( ( fPlayerZ - vGoalPosition.z ) * ( fPlayerZ - vGoalPosition.z ) + ( fPlayerX - vGoalPosition.x ) * ( fPlayerX - vGoalPosition.x ) > 16.0f )
			return;
	}

	AInteriorGoal* pSafeGoal = AInteriorNetworkManager::GetSingleton()->FindNetwork( TPS8( park2 ) )->FindGoalInArea( TPS8( farmersafepos ), -1 );

	if ( pSafeGoal )
	{
		TVector4 vSafePosition;
		pSafeGoal->GetPosition( vSafePosition );
		pPlayer->GetSteer()->SetPosition( vSafePosition );
	}
}

// $Barnyard: FUNCTION 00534270
void AFarmerTruck::SetSequenceMode( TBOOL a_bSequence )
{
	if ( AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayer( 0 ) )
	{
		TSTATICCAST( AVisibleUnit, pPlayer )->SetVisible( !a_bSequence );

		// The bike ride is stopped during the sequence
		if ( pPlayer->GetSteer()->GetClass()->IsA( &TGetClass( ABikeSteer ) ) )
		{
			if ( a_bSequence == TTRUE )
				TSTATICCAST( ABikeSteer, pPlayer->GetSteer() )->Stop( TTRUE );
			else
				TSTATICCAST( ABikeSteer, pPlayer->GetSteer() )->Start();
		}
	}

#ifdef BARNYARD_COMMUNITY_PATCH
	// The original expects the truck to exist
	if ( !ms_pInstance )
		return;
#endif

	if ( a_bSequence )
	{
		if ( ms_pInstance->m_bSequenceMode )
			return;

		// Hide the characters and remember which ones were visible
		for ( TINT i = 0; i < NUM_HIDDEN_CHARACTERS; i++ )
		{
			const TUINT32 uiFlag = 1U << i;
			ms_pInstance->m_uiVisibleCharacters &= ~uiFlag;

			AUnit* pUnit = AUnitNameManager::GetSingleton()->RetrieveByName( *ms_apHiddenCharacters[ i ] );

			if ( pUnit )
			{
				AVisibleUnit* pVisibleUnit = TSTATICCAST( AVisibleUnit, pUnit );

				if ( pVisibleUnit->IsVisible() )
				{
					ms_pInstance->m_uiVisibleCharacters |= uiFlag;
					pVisibleUnit->SetVisible( TFALSE );
				}
				else
				{
					ms_pInstance->m_uiVisibleCharacters &= ~uiFlag;
				}
			}
		}

		ms_pInstance->m_vSavedPlayerPosition = *APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->GetSteer()->GetPosition();
		MovePlayerToSafePos( TFALSE );

		ms_pInstance->m_bSequenceMode = TTRUE;
	}
	else if ( ms_pInstance->m_bSequenceMode )
	{
		// Show the characters that were visible again
		for ( TINT i = 0; i < NUM_HIDDEN_CHARACTERS; i++ )
		{
			AUnit* pUnit = AUnitNameManager::GetSingleton()->RetrieveByName( *ms_apHiddenCharacters[ i ] );

			if ( pUnit )
			{
				AVisibleUnit* pVisibleUnit = TSTATICCAST( AVisibleUnit, pUnit );
				const TBOOL   bVisible     = ( ms_pInstance->m_uiVisibleCharacters & ( 1U << i ) ) != 0;

				if ( pVisibleUnit->IsVisible() != bVisible )
					pVisibleUnit->SetVisible( bVisible );
			}
		}

		APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->GetSteer()->SetPosition( ms_pInstance->m_vSavedPlayerPosition );
		ms_pInstance->m_bSequenceMode = TFALSE;
	}
}
