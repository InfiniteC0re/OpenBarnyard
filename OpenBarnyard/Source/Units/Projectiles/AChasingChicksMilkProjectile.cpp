#include "pch.h"
#include "AChasingChicksMilkProjectile.h"
#include "AProjectileLauncher.h"
#include "MiniGame/AChasingChicksMiniGame.h"
#include "GameInterface/AGameStateController.h"
#include "FX/AParticleEffectManager.h"
#include "FX/ASplashManager.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/AChasingChicksAI.h"
#include "Units/AUnitPlayer.h"
#include "Units/AUnitManager.h"
#include "Physics/ACollisionManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0050ddf0, CreateObject 0050deb0, CreateObjectInPlace 0050df10 (the constructor is inlined in both)
TDEFINE_CLASS( AChasingChicksMilkProjectile );

// $Barnyard: FUNCTION 0050de30
AChasingChicksMilkProjectile::AChasingChicksMilkProjectile()
{
	m_oHitUnit = TNULL;
}

// $Barnyard: FUNCTION 0050de90
// $Barnyard: FUNCTION 0050de00
AChasingChicksMilkProjectile::~AChasingChicksMilkProjectile()
{
}

// $Barnyard: FUNCTION 0050e140
void AChasingChicksMilkProjectile::OnHit()
{
	AChasingChicksMiniGame* pMiniGame = TNULL;
	AGameState*             pState    = AGameStateController::GetSingleton()->GetCurrentState();

	if ( pState && pState->IsA( &TGetClass( AChasingChicksMiniGame ) ) )
		pMiniGame = TSTATICCAST( AChasingChicksMiniGame, pState );

	// Tells the AI who it hit
	if ( m_pLauncher->GetOwner()->GetUnit()->IsA( &TGetClass( AChasingChicksAI ) ) )
		TSTATICCAST( AChasingChicksAI, m_pLauncher->GetOwner()->GetUnit() )->OnHitUnit( m_oHitUnit );

	if ( !pMiniGame )
		return;

	TVector4 vDirection = GetDirection();
	vDirection.y *= -1.0f;
	vDirection.Normalise();
	AParticleEffectManager::GetSingleton()->PlayEffect( m_iSplashEffect, m_vPosition, vDirection );

	// The milk passes through everything but the players
	TBOOL bSplash = TTRUE;

	if ( m_oHitUnit )
	{
		if ( m_oHitUnit->GetUnit()->IsA( &TGetClass( AUnitPlayer ) ) )
			pMiniGame->OnPlayerHit( m_oHitUnit );
		else
			bSplash = TFALSE;
	}

	// Splashes the players around too
	TINT                    iNumFound = 0;
	AUnitHash::QueryResult* pResults  = AUnitManager::GetSingleton()->FindUnits( m_vPosition, 2.0f, &iNumFound, TFALSE, 420, TFALSE, TNULL );

	for ( TUINT i = 0; i < TUINT( iNumFound ); i++ )
	{
		AUnit* pUnit = pResults[ i ].pUnit;

		if ( pUnit && m_pLauncher->GetOwner()->GetUnit() != pUnit && pUnit->IsA( &TGetClass( AUnitPlayer ) ) )
		{
			// NOTE: retail passes m_oHitUnit here, not pUnit, so the players around aren't affected (the one hit directly is hit again)
			pMiniGame->OnPlayerHit( m_oHitUnit );
		}
	}

	if ( !bSplash )
		return;

	PlaySplashSound();

	TVector4 vSplashDirection = GetDirection();
	vSplashDirection.y *= -1.0f;

	if ( m_oHitUnit )
	{
		AParticleEffectManager::GetSingleton()->PlayEffect( m_iSplatEffect, m_vPosition, vSplashDirection );
		AParticleEffectManager::GetSingleton()->PlayEffect( m_iSplashEffect, m_vPosition, vSplashDirection );
	}
	else
	{
		TVector4 vSphere( m_vPosition.x, m_vPosition.y, m_vPosition.z, 0.25f );
		TFLOAT   fDistance;
		TVector4 vNormal;

		if ( ATerrainInterface::GetSingleton()->CollideSphere( vSphere, fDistance, vNormal, -1, TCollisionCommon::TOSHICGROUPFLAG( 0 ) ) && vNormal.y < -0.707f )
		{
			// A puddle on the ground
			ASplashManager::GetSingleton()->Spawn( m_vPosition, TVector4::VEC_NEGY, 1.5f, ASplashManager::SPLASHTYPE_MILK );
		}
		else
		{
			TVector4 vPosition;
			vPosition.Multiply( vSplashDirection, 0.1f );
			vPosition.x += m_vPosition.x;
			vPosition.y += m_vPosition.y;
			vPosition.z += m_vPosition.z;

			AParticleEffectManager::GetSingleton()->PlayEffect( m_iSplatEffect, vPosition, vSplashDirection );
			AParticleEffectManager::GetSingleton()->PlayEffect( m_iSplashEffect, vPosition, vSplashDirection );
		}
	}

	Kill();
}

// $Barnyard: FUNCTION 0050de20
TINT AChasingChicksMilkProjectile::GetType()
{
	return 9;
}

// $Barnyard: FUNCTION 0050df50
TBOOL AChasingChicksMilkProjectile::CheckUnitCollision( Toshi::TRef<AUnitProxy>& a_rHitUnit )
{
	TBOOL bHit = TFALSE;

	// NOTE: same queries as AProjectile::CheckUnitCollision (inlined by the original)
	if ( m_vPrevPosition.MagnitudeSq() > 0.1f && !m_vPrevPosition.IsEqual( m_vPosition ) )
	{
		ARayData oRay( m_vPrevPosition, m_vPosition );

		if ( ACollisionManager::CollideRayUnits( m_pLauncher->GetOwner()->GetUnit(), oRay, 10.0f, a_rHitUnit ) )
			bHit = TTRUE;
		else
			a_rHitUnit = TNULL;
	}

	if ( !bHit )
	{
		AUnit* pOwner = m_pLauncher->GetOwner()->GetUnit();

		if ( !ACollisionManager::CollideSphereUnits( m_vPosition, GetRadius(), a_rHitUnit, 1, &pOwner ) )
			return bHit;

		bHit = TTRUE;
	}

	// Only the players can be hit
	if ( a_rHitUnit->GetUnit()->GetClass()->IsA( &TGetClass( AUnitPlayer ) ) )
		return bHit;

	a_rHitUnit = TNULL;
	return TFALSE;
}
