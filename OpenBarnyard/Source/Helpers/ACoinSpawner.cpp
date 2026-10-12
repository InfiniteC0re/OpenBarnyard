#include "pch.h"
#include "ACoinSpawner.h"
#include "ARandom.h"
#include "FX/AAniEffectManager.h"
#include "FX/ADieAniEffect.h"
#include "FX/AFadeAniEffect.h"
#include "FX/AWaitAniEffect.h"
#include "FX/ARotateAniEffect.h"
#include "Player/APlayerManager.h"
#include "Render/AShadowManager.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitFactory.h"
#include "Units/AUnit.h"
#include "UnitMovement/Steering/ABallSteer2.h"
#include "Units/AVisibleUnit.h"
#include "Physics/ACollisionObject.h"
#include "Physics/ACollisionManager.h"
#include "Helpers/ARegrowthManager.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( buck_1 );

// Returns a random number in [0, a_fRange) the way the original inlines TRandom
static TFLOAT RandomFloat( TFLOAT a_fRange )
{
	return TFLOAT( ARandom::GetSingleton()->m_oRandom.RandRaw() >> 1 ) * ( a_fRange / 2147483648.0f );
}

// $Barnyard: FUNCTION 0066ec50
void ACoinSpawner::GetRandomThrowDirection( TVector4& a_rOutDirection )
{
	const TVector4* pPlayerDirection   = APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->GetSteer()->GetDirection();
	TVector4        vecPlayerDirection = *pPlayerDirection;

	a_rOutDirection.x = RandomFloat( 2.0f ) - 1.0f;
	a_rOutDirection.x = vecPlayerDirection.x + a_rOutDirection.x;
	// NOTE: the original also adds the direction of the player to y, which is overwritten below
	a_rOutDirection.z = RandomFloat( 2.0f ) - 1.0f + vecPlayerDirection.z;
	a_rOutDirection.y = -1.9f - RandomFloat( 0.2f );
	a_rOutDirection.Normalise();
}

// $Barnyard: FUNCTION 0066ed70
void ACoinSpawner::PushOutOfCollision( AUnit* a_pCoin )
{
	ABallSteer2*      pSteer           = TSTATICCAST( ABallSteer2, a_pCoin->GetSteer() );
	ACollisionObject* pCollisionObject = TSTATICCAST( AVisibleUnit, a_pCoin )->GetCollisionObject();

	if ( !pCollisionObject )
		return;

	// The center of the ball
	TVector4        vecOffset  = pSteer->GetOffset();
	const TVector4& rcPosition = *pSteer->GetPosition();
	TVector4        vecCenter( rcPosition.x - vecOffset.x, rcPosition.y - vecOffset.y, rcPosition.z - vecOffset.z, rcPosition.w );

	TVector4 vecNormal = TVector4::VEC_NEGY;
	TUINT    uiFlags   = pSteer->GetCollisionFlags();
	TFLOAT   fDistance = pCollisionObject->GetObjectRadius();

	if ( ACollisionManager::CollideSphereWithWorld( vecCenter, pCollisionObject, fDistance, vecNormal, uiFlags, 0x70000 ) == TTRUE )
	{
		// Move it out along the normal
		TFLOAT fPenetration = pCollisionObject->GetObjectRadius() - fDistance;

		if ( 0.0f >= fPenetration )
			fPenetration = 0.0f;

		TVector4 vecPush;
		vecPush.Multiply( vecNormal, fPenetration );
		vecCenter.x += vecPush.x;
		vecCenter.y += vecPush.y;
		vecCenter.z += vecPush.z;

		TVector4 vecPosition;
		vecPosition.Add( vecCenter, vecOffset );
		pSteer->SetPosition( vecPosition );
	}
}

// $Barnyard: FUNCTION 0066ef10
void ACoinSpawner::SpawnCoin( const TPString8& a_rcType, const TVector4& a_rcPosition )
{
	if ( !AAniEffectManager::GetSingleton()->ReserveEffects( 4 ) )
		return;

	TVector4 vecDirection;
	GetRandomThrowDirection( vecDirection );

	// NOTE: the result isn't used
	APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->GetSteer()->GetPosition();

	AUnitType* pType = AUnitManager::GetSingleton()->GetUnitFactory()->Find( a_rcType );

	TRandom& rRandom = ARandom::GetSingleton()->m_oRandom;
	TUINT32  uiRandZ = rRandom.RandRaw();
	TUINT32  uiRandX = rRandom.RandRaw();

	TVector4 vecPosition;
	vecPosition.x = TFLOAT( uiRandX >> 1 ) * ( 0.2f / 2147483648.0f ) - 0.1f + a_rcPosition.x;
	vecPosition.y = a_rcPosition.y - 0.35f;
	vecPosition.z = TFLOAT( uiRandZ >> 1 ) * ( 0.2f / 2147483648.0f ) - 0.1f + a_rcPosition.z;
	vecPosition.w = a_rcPosition.w;

	static const TVector4 s_vecCoinDirection( 1.0f, 0.0f, 0.0f, 1.0f ); // 0x0077cfe4

	AUnit* pCoin = AUnitManager::GetSingleton()->SpawnUnit( pType, vecPosition, s_vecCoinDirection );
	AShadowManager::GetSingleton()->AddUnitShadow( pCoin, 0.5f, -1, -0.1f, AShadowManager::FLAGS_UNIFORM_SCALE, 0 );

	// Throws it
	TVector4 vecVelocity;
	vecVelocity.Multiply( vecDirection, RandomFloat( 1.0f ) + 10.0f );
	pCoin->GetSteer()->SetVelocity( vecVelocity );

	// The model of the coin is above the ball
	ABallSteer2* pSteer = TSTATICCAST( ABallSteer2, pCoin->GetSteer() );

	TVector4 vecOffset;
	vecOffset.Multiply( TVector4::VEC_NEGY, RandomFloat( 0.05f ) + 0.15f );
	pSteer->SetOffset( vecOffset );
	pSteer->SetGravity( 19.6f );
	pSteer->SetCollisionFlags( ACollisionObject::TYPE_IGNORED | ACollisionObject::TYPE_PLAYER | ACollisionObject::TYPE_UNIT );
	pSteer->SetIgnoreHitsWhenLeaving( TTRUE );

	PushOutOfCollision( pCoin );

	// Waits, fades out and dies
	AWaitAniEffect* pWait = AAniEffectManager::GetSingleton()->CreateEffect<AWaitAniEffect>();
	AFadeAniEffect* pFade = AAniEffectManager::GetSingleton()->CreateEffect<AFadeAniEffect>();
	ADieAniEffect*  pDie  = AAniEffectManager::GetSingleton()->CreateEffect<ADieAniEffect>();

	pDie->SetUnit( pCoin );
	pFade->SetUnit( pCoin );
	pFade->SetSpeed( 0.5f );
	pFade->SetNextEffect( pDie );
	pWait->SetTime( RandomFloat( 15.0f ) + 30.0f );
	pWait->SetNextEffect( pFade );

	// Spins
	// NOTE: inlined CreateEffect 005b88d0
	ARotateAniEffect* pRotate = AAniEffectManager::GetSingleton()->CreateEffect<ARotateAniEffect>();
	pRotate->SetUnit( pCoin );

	TVector4 vecRotation;
	vecRotation.Multiply( TVector4::VEC_POSY, RandomFloat( 0.5f ) + 2.0f );
	pRotate->SetRotationSpeed( vecRotation );
	pRotate->SetActive( TTRUE );
}

// $Barnyard: FUNCTION 0066f2a0
void ACoinSpawner::SpawnMoney( TINT a_iNumBucks, const TVector4& a_rcPosition )
{
	TPString8 aTypes[ 3 ];
	aTypes[ 0 ] = TPS8( buck_1 );

	TINT aNumCoins[ 3 ] = { a_iNumBucks, 0, 0 };

	for ( TINT i = 0; i < 3; i++ )
	{
		for ( TINT k = aNumCoins[ i ]; k > 0; k-- )
			SpawnCoin( aTypes[ i ], a_rcPosition );
	}
}

// ARacingCarSteer.cpp
void KnockOverGrowthObject( ARegrowthManager::GrowthObject* a_pObject, TINT a_iType );

// Breaks the crate a_pObject (of regrowth type a_iType) the player kicked: its content
// is thrown out of it and the last crate of the type also gives 5 bucks.
// NOTE: a free function of the original binary, called by AShakeTreeMicroGame
// $Barnyard: FUNCTION 0066eab0
void KnockOverCrate( ARegrowthManager::GrowthObject* a_pObject, TINT a_iType )
{
	TVector4 vecPosition = a_pObject->vecPosition;
	vecPosition.y        = vecPosition.y - 0.3f;

	KnockOverGrowthObject( a_pObject, a_iType );

	ARegrowthManager::CrateSchema::Content* pContent = a_pObject->PickCrateContent();

	if ( pContent )
	{
		if ( TStringManager::String8CompareNoCase( pContent->szName, "coin" ) != 0 )
		{
			TPString8 strType( pContent->szName );

			for ( TINT i = 0; i < pContent->iUnk; i++ )
				ACoinSpawner::SpawnCoin( strType, vecPosition );
		}
		else
		{
			ACoinSpawner::SpawnMoney( pContent->iUnk, vecPosition );
		}
	}

	if ( ARegrowthManager::GetSingleton()->CountTypeObjectsAtStage( 10000, a_iType ) == 1 )
	{
		TPString8 strBuck = TPS8( buck_1 );

		ACoinSpawner::SpawnCoin( strBuck, vecPosition );
		ACoinSpawner::SpawnCoin( strBuck, vecPosition );
		ACoinSpawner::SpawnCoin( strBuck, vecPosition );
		ACoinSpawner::SpawnCoin( strBuck, vecPosition );
		ACoinSpawner::SpawnCoin( strBuck, vecPosition );
	}
}
