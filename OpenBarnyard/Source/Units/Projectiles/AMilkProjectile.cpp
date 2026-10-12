#include "pch.h"
#include "AMilkProjectile.h"
#include "AProjectileLauncher.h"
#include "FX/AMilkManager.h"
#include "FX/AParticleEffectManager.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/am_m.h"
#include "SoundBank/am_f.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "GUI/AFadeManager.h"
#include "FX/ASplashManager.h"
#include "Terrain/ATerrainInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00666890, CreateObject 00666be0, CreateObjectInPlace 00666c40
TDEFINE_CLASS( AMilkProjectile );

// $Barnyard: FUNCTION 00666940
AMilkProjectile::AMilkProjectile()
    : m_iNumParticles( 0 )
    , m_bHit( TFALSE )
    , m_bSplashedCamera( TFALSE )
    , m_iStreamId( 0 )
{
	m_oHitUnit = TNULL;

	m_iSplatEffect  = AParticleEffectManager::FindEffect( "milksplat" );
	m_iSplashEffect = AParticleEffectManager::FindEffect( "milksplash" );
}

// $Barnyard: FUNCTION 00666c80
// $Barnyard: FUNCTION 006669d0
AMilkProjectile::~AMilkProjectile()
{
	ReleaseParticles();
}

// $Barnyard: FUNCTION 006668a0
TBOOL AMilkProjectile::Update( TFLOAT a_fDeltaTime )
{
	if ( !m_bSplashedCamera && IsAboutToHitCamera() )
		SplashCamera();

	if ( !m_bHit )
	{
		if ( CheckUnitCollision( m_oHitUnit ) )
		{
			m_bHit = TTRUE;
			OnHit();
		}

		if ( !m_bHit && CheckWorldCollision() )
		{
			m_bHit = TTRUE;
			OnHit();
		}
	}

	AProjectile::Update( a_fDeltaTime );
	return m_bHit;
}

// $Barnyard: FUNCTION 00666bb0
void AMilkProjectile::Kill()
{
	ReleaseParticles();
	AProjectile::Kill();
}

// $Barnyard: FUNCTION 00666a40
void AMilkProjectile::Reset()
{
	m_bSplashedCamera = TFALSE;
	m_bHit            = TFALSE;
	ReleaseParticles();
	m_iUnk50 = -1;
	AProjectile::Reset();
}

// $Barnyard: FUNCTION 00666ca0
void AMilkProjectile::OnHit()
{
	PlaySplashSound();

	TVector4 vDirection = GetDirection();
	vDirection.y *= -1.0f;

	if ( m_oHitUnit )
	{
		AParticleEffectManager::GetSingleton()->PlayEffect( m_iSplatEffect, m_vPosition, vDirection );
		AParticleEffectManager::GetSingleton()->PlayEffect( m_iSplashEffect, m_vPosition, vDirection );
	}
	else
	{
		TVector4 vSphere( m_vPosition.x, m_vPosition.y, m_vPosition.z, 0.25f );

		// Leaves a milk puddle on the floor
		TFLOAT   fDistance;
		TVector4 vNormal;

		if ( ATerrainInterface::GetSingleton()->CollideSphere( vSphere, fDistance, vNormal, -1, 0 ) && vNormal.y < -0.707f )
		{
			ASplashManager::GetSingleton()->Spawn( m_vPosition, TVector4::VEC_NEGY, 0.9f, ASplashManager::SPLASHTYPE_MILK );
			Kill();
			return;
		}

		TVector4 vPosition;
		vPosition.Multiply( vDirection, 0.1f );
		vPosition.x += m_vPosition.x;
		vPosition.y += m_vPosition.y;
		vPosition.z += m_vPosition.z;

		AParticleEffectManager::GetSingleton()->PlayEffect( m_iSplatEffect, vPosition, vDirection );
		AParticleEffectManager::GetSingleton()->PlayEffect( m_iSplashEffect, vPosition, vDirection );
	}

	Kill();
}

// $Barnyard: FUNCTION 00666a80
TBOOL AMilkProjectile::IsAboutToHitCamera()
{
	ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

	TVector4 vCameraForward( pCamera->GetMatrix().m_f31, pCamera->GetMatrix().m_f32, pCamera->GetMatrix().m_f33, 1.0f );

	if ( TVector4::DotProduct3( vCameraForward, m_vVelocity ) < 0.0f )
	{
		if ( TVector4::DistanceSq( pCamera->GetMatrix().GetTranslation(), m_vPosition ) < 4.0f )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00666b60
void AMilkProjectile::SplashCamera()
{
	m_bSplashedCamera = TTRUE;

	AFade::Color oFadeFrom( 127, 255, 255, 255 );
	AFade::Color oFadeTo( 0, 255, 255, 255 );
	AFadeManager::GetSingleton()->StartFade( oFadeFrom, oFadeTo, 0.25f );
}

// $Barnyard: FUNCTION 00666920
void AMilkProjectile::AddParticle( TINT a_iParticle )
{
	if ( m_iNumParticles < MAX_NUM_PARTICLES )
		m_aParticles[ m_iNumParticles++ ] = a_iParticle;
}

// $Barnyard: FUNCTION 00666b10
void AMilkProjectile::PlaySplashSound()
{
	TINT iGender = m_pLauncher->GetGender();

	if ( iGender == 0 )
	{
		ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::AM_M_COWMILKSPLASH_M, m_vPosition );
		return;
	}

	ASoundManager::GetSingleton()->PlayCueAtPosition( ( iGender == 1 ) ? soundbank::AM_F_COWMILKSPLASH_F : soundbank::AM_M_COWMILKSPLASH_M, m_vPosition );
}

// NOTE: inlined in retail
void AMilkProjectile::ReleaseParticles()
{
	if ( AMilkManager::GetSingleton() )
		AMilkManager::GetSingleton()->ReleaseParticles( m_iNumParticles, m_aParticles );

	m_iNumParticles = 0;
}
