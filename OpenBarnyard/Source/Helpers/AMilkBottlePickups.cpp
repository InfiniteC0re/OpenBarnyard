#include "pch.h"
#include "Helpers/ABikeManager.h"
#include "AMilkBottlePickups.h"
#include "AInstanceManager.h"
#include "ARandom.h"
#include "Animation/AModel.h"
#include "Animation/AModelRepos.h"
#include "Player/APlayerManager.h"
#include "Sound/ASoundManager.h"
#include "Terrain/ATerrainLocator.h"
#include "Units/AUnitPlayer.h"
#include "Units/Projectiles/AProjectileLauncher.h"
#include "Units/Projectiles/ALaunchAmmo.h"
#include "UnitMovement/Steering/ASteer.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"

#include <Toshi/TSceneObject.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( powerup_full_milk );

// $Barnyard: FUNCTION 0040d6a0
AMilkBottlePickups::Bottle::Bottle( const TVector4& a_rcPosition, AInteriorGoal* a_pGoal )
{
	bCollected = TFALSE;
	vPosition  = a_rcPosition;
	fOpacity   = 1.0f;
	fUnk18     = std::numeric_limits<TFLOAT>::quiet_NaN();
	pGoal      = a_pGoal;
	fGoalTime  = 0.0f;
	fTime      = ARandom::GetSingleton()->m_oRandom.GetFloat();
}

// $Barnyard: FUNCTION 0040d680
void AMilkBottlePickups::Bottle::SetCollected( TBOOL a_bCollected )
{
	bCollected = a_bCollected;
	fTime      = a_bCollected ? 16.0f : 0.0f;
}

// $Barnyard: FUNCTION 0040d730
void AMilkBottlePickups::Bottle::Update( TFLOAT a_fDeltaTime )
{
	if ( bCollected )
	{
		// Wait before coming back
		fTime -= a_fDeltaTime;

		if ( fTime < 0.0f )
		{
			bCollected = TFALSE;
			fTime      = 0.0f;
		}
	}
	else
	{
		if ( fOpacity < 1.0f )
			fOpacity = a_fDeltaTime * 0.33f + fOpacity;

		// Take the position from the goal once its height is known
		if ( pGoal && TMath::MAXFLOAT == vPosition.y )
		{
			fGoalTime = a_fDeltaTime + fGoalTime;

			if ( fGoalTime > 2.0f )
			{
				fGoalTime = 0.0f;

				if ( pGoal->GetPosition( vPosition ) )
					vPosition.y = vPosition.y - 0.75f;
			}
		}

		// Spin
		fTime = a_fDeltaTime + a_fDeltaTime + fTime;
	}
}

// $Barnyard: FUNCTION 0041fb60
AMilkBottlePickups::AMilkBottlePickups()
    : m_iNumBottles( 0 )
    , m_pModel( TNULL )
    , m_bVisible( TTRUE )
{
}

// $Barnyard: FUNCTION 0041fb80
AMilkBottlePickups::~AMilkBottlePickups()
{
	if ( m_pModel )
	{
		AModelRepos::GetSingleton()->DestroyModelInstance( m_pModel );
		m_pModel = TNULL;
	}
}

// $Barnyard: FUNCTION 0041fbb0
AMilkBottlePickups::Bottle* AMilkBottlePickups::AddBottle( const TVector4& a_rcPosition, AInteriorGoal* a_pGoal )
{
	if ( m_iNumBottles >= MAX_BOTTLES )
		return TNULL;

	// NOTE: a_pGoal isn't passed to the bottle
	TVector4 vecPosition = a_rcPosition;
	vecPosition.y -= 0.75f;

	Bottle* pBottle = new Bottle( vecPosition, TNULL );
	pBottle->SetCollected( TFALSE );
	m_apBottles[ m_iNumBottles++ ] = pBottle;

	return pBottle;
}

// $Barnyard: FUNCTION 0041fc80
void AMilkBottlePickups::Clear()
{
	while ( m_iNumBottles != 0 )
	{
		TFree( m_apBottles[ m_iNumBottles - 1 ] );
		m_iNumBottles--;
		m_apBottles[ m_iNumBottles ] = TNULL;
	}
}

// $Barnyard: FUNCTION 0041fcb0
void AMilkBottlePickups::Update( TFLOAT a_fDeltaTime, TBOOL a_bCanPickUp )
{

	// Not while riding the bike
	if ( ABikeManager::GetSingleton()->IsPlayerOnBike() )
		return;

	AUnitPlayer* pPlayer = TSTATICCAST( AUnitPlayer, APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) );

	if ( !pPlayer )
		return;

	TVector4 vecPlayerPos = *pPlayer->GetSteer()->GetPosition();

	for ( TINT i = 0; i < m_iNumBottles; i++ )
	{
		Bottle* pBottle = m_apBottles[ i ];

		if ( a_bCanPickUp && !pBottle->bCollected && TVector4::DistanceSq( pBottle->vPosition, vecPlayerPos ) < 4.0f )
		{
			// Fill up the milk
			if ( ALaunchAmmo* pAmmo = pPlayer->GetActiveLauncher()->GetAmmo() )
			{
				// NOTE: the original sets the ammo directly
				pAmmo->SetAmmo( pAmmo->GetMaxAmmo() );
				ASoundManager::GetSingleton()->PlayCueAtPositionEx( 194, pBottle->vPosition, 1.0f, TFALSE, 0.0f, -1 );
				pBottle->SetCollected( TTRUE );
			}
		}

		pBottle->Update( a_fDeltaTime );
	}
}

// $Barnyard: FUNCTION 0041fde0
AMilkBottlePickups::Bottle* AMilkBottlePickups::FindClosestBottle( const TVector4& a_rcPosition, TFLOAT& a_rDistance )
{
	Bottle* pClosest    = TNULL;
	TFLOAT  fDistanceSq = a_rDistance * a_rDistance;

	for ( TINT i = 0; i < m_iNumBottles; i++ )
	{
		Bottle* pBottle = m_apBottles[ i ];

		if ( !pBottle->bCollected )
		{
			TFLOAT fBottleDistanceSq = TVector4::DistanceSq( pBottle->vPosition, a_rcPosition );

			if ( fBottleDistanceSq < fDistanceSq )
			{
				pClosest    = pBottle;
				fDistanceSq = fBottleDistanceSq;
			}
		}
	}

	if ( pClosest )
		a_rDistance = TMath::Sqrt( fDistanceSq );

	return pClosest;
}

// $Barnyard: FUNCTION 0041fe80
void AMilkBottlePickups::Render()
{
	if ( !m_pModel || !m_bVisible )
		return;

	// Stand the bottles up
	TMatrix44 mRotation(
	    1.0f, 0.0f, 0.0f, 0.0f,
	    0.0f, 0.0f, 1.0f, 0.0f,
	    0.0f, -1.0f, 0.0f, 0.0f,
	    0.0f, 0.0f, 0.0f, 1.0f
	);

	TSceneObject* pSceneObject = m_pModel->GetSceneObject();
	pSceneObject->EnableSkeletonUpdate();

	for ( TINT i = 0; i < m_iNumBottles; i++ )
	{
		Bottle* pBottle = m_apBottles[ i ];

		if ( !pBottle->bCollected && pBottle->fOpacity > 0.0f )
		{
			// NOTE: AModelInstance::ResetOpacity makes the model opaque
			if ( pBottle->fOpacity >= 1.0f )
				m_pModel->ResetOpacity();
			else
				m_pModel->SetOpacity( pBottle->fOpacity );

			TMatrix44 mTransform;
			mTransform.Identity();
			TMatrix44::TransformVector( mTransform.GetTranslation3(), mTransform, pBottle->vPosition.AsVector3() );
			mTransform.RotateY( pBottle->fTime );

			TMatrix44 mTemp = mTransform;
			mTransform.Multiply( mTemp, mRotation );

			pSceneObject->GetTransform().SetMatrix( mTransform );
			pSceneObject->RenderIfVisible();
		}
	}

	pSceneObject->DisableSkeletonUpdate();
}

// $Barnyard: FUNCTION 00420060
void AMilkBottlePickups::SetAllCollected( TBOOL a_bCollected )
{
	for ( TINT i = 0; i < m_iNumBottles; i++ )
		m_apBottles[ i ]->SetCollected( a_bCollected );
}

// NOTE: inlined in retail, loads the model of the bottles and makes it fade with the distance like the instances
void AMilkBottlePickups::SetupModel()
{
	m_pModel = AModelRepos::GetSingleton()->InstantiateNewModel( TPS8D( "data/models/milk_bottle_01" ), TNULL );

	TFLOAT fLOD0Distance, fBlendEnd, fBlendStart;
	AInstanceManager::GetSingleton()->GetVisibilityDistances( fLOD0Distance, fBlendEnd, fBlendStart );

	TSceneObject* pSceneObject = m_pModel->GetSceneObject();
	pSceneObject->EnableDistanceCulling();
	pSceneObject->GetFadeDistances().y = fBlendEnd;
	pSceneObject->GetFadeDistances().x = fBlendStart;
	pSceneObject->GetFadeParams().x    = 1.0f / ( fBlendEnd - pSceneObject->GetFadeDistances().x );

	m_pModel->SetSkeletonUpdating( TFALSE );
}

// $Barnyard: FUNCTION 004200c0
TINT AMilkBottlePickups::LoadFromNetwork( AInteriorNetwork* a_pNetwork )
{
	if ( !a_pNetwork )
		return 0;

	AInteriorNetwork::INetwork* pData = a_pNetwork->m_pINetwork;

	for ( TINT i = pData->m_iNumGoals; i != 0; i-- )
	{
		AInteriorGoal* pGoal = &a_pNetwork->m_pINetwork->m_pGoals[ i - 1 ];

		if ( *pGoal->m_strName == TPS8( powerup_full_milk ) )
		{
			TVector4 vecPosition;
			pGoal->GetPosition( vecPosition );
			AddBottle( vecPosition, pGoal );
		}
	}

	SetupModel();
	return m_iNumBottles;
}

// $Barnyard: FUNCTION 00420220
TINT AMilkBottlePickups::LoadFromLocators( ATerrainLocatorList* a_pLocators )
{
	if ( !a_pLocators )
		return 0;

	const TCHAR* szName = "powerup_full_milk";

	for ( TINT i = 0; i < a_pLocators->GetNumLocators(); i++ )
	{
		if ( TStringManager::String8Compare( a_pLocators->GetLocatorName( i ), szName, TStringManager::String8Length( szName ) ) == 0 )
		{
			TMatrix44 mLocator;
			a_pLocators->GetLocator( i )->GetMatrix( mLocator );

			TMatrix44 mTransform;
			mTransform.Identity();
			mTransform.RotateX( TMath::HALF_PI );

			TMatrix44 mTemp = mTransform;
			mTransform.Multiply( mTemp, mLocator );

			AddBottle( mTransform.AsBasisVector4( BASISVECTOR_TRANSLATION ), TNULL );
		}
	}

	SetupModel();
	return m_iNumBottles;
}
