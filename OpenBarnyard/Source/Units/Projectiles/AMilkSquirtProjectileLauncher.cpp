#include "pch.h"
#include "AMilkSquirtProjectileLauncher.h"
#include "AMilkProjectile.h"
#include "ARandom.h"
#include "FX/AMilkManager.h"
#include "FX/AParticleEffectManager.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/am_m.h"
#include "SoundBank/am_f.h"
#include "Player/APlayerManager.h"
#include "Units/AUnit.h"
#include "Units/AAnimatedUnit.h"
#include "UnitMovement/Steering/ASteer.h"
#include "Animation/AAnimatableObject.h"
#include "Helpers/AHintManager.h"

#include <Math/TMatrix44.h>
#include <Math/TQuaternion.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( cb_udder );

// NOTE: GetClass 00668670, CreateObject 00668a50, CreateObjectInPlace 00668ab0
TDEFINE_CLASS( AMilkSquirtProjectileLauncher );

TINT AMilkSquirtProjectileLauncher::ms_iLastStreamId = 0;

// Rotates a_rcVector (xyz) by a_rcRotation, the result has w = 1.
// NOTE: retail calls RotateVector (006c9740, AModel.cpp)
static TVector4 RotateVectorByQuaternion( const TQuaternion& a_rcRotation, const TVector4& a_rcVector )
{
	TQuaternion qInverse( -a_rcRotation.x, -a_rcRotation.y, -a_rcRotation.z, a_rcRotation.w );

	TQuaternion qResult = a_rcRotation;
	qResult *= TQuaternion( a_rcVector.x, a_rcVector.y, a_rcVector.z, 0.0f );
	qResult *= qInverse;

	return TVector4( qResult.x, qResult.y, qResult.z, 1.0f );
}

// $Barnyard: FUNCTION 006687d0
AMilkSquirtProjectileLauncher::AMilkSquirtProjectileLauncher()
    : m_fSquirtTimeLeft( -TMath::TFLOAT_EPSILON )
    , m_pStreamProjectile( TNULL )
    , m_bNewStream( TFALSE )
{
	m_oMilkAmmo.SetMaxAmmo( 20 );
	m_oMilkAmmo.SetAmmo( 10 ); // NOTE: inlined in retail
	SetAmmo( &m_oMilkAmmo );

	m_fLaunchSpeed = 15.0f;

	m_iMuzzleFlashEffect = AParticleEffectManager::FindEffect( "milkmussleflash" );
	m_iOutOfMilkEffect   = AParticleEffectManager::FindEffect( "outofmilksquirt" );
}

// $Barnyard: FUNCTION 006689d0
// $Barnyard: FUNCTION 00668680
AMilkSquirtProjectileLauncher::~AMilkSquirtProjectileLauncher()
{
}

// $Barnyard: FUNCTION 006686e0
void AMilkSquirtProjectileLauncher::Activate()
{
	m_fSquirtTimeLeft = -TMath::TFLOAT_EPSILON;
	m_bNewStream      = TFALSE;
	AProjectileLauncher::Activate();
}

// NOTE: identical to Activate, both slots point at 006686e0 in retail
void AMilkSquirtProjectileLauncher::Deactivate()
{
	m_fSquirtTimeLeft = -TMath::TFLOAT_EPSILON;
	m_bNewStream      = TFALSE;
	AProjectileLauncher::Deactivate();
}

// $Barnyard: FUNCTION 00668e50
TBOOL AMilkSquirtProjectileLauncher::Update( TFLOAT a_fDeltaTime )
{
	AProjectileLauncher::Update( a_fDeltaTime );

	m_fSquirtTimeLeft -= a_fDeltaTime;

	if ( m_fSquirtTimeLeft < 0.0f )
	{
		m_pStreamProjectile = TNULL;
		return TTRUE;
	}

	if ( !m_pStreamProjectile && !ContinueStream() )
	{
		m_bNewStream = TFALSE;
		return TTRUE;
	}

	AMilkManager* pMilkManager = AMilkManager::GetSingleton();

	if ( !m_pStreamProjectile || !pMilkManager )
		return TTRUE;

	TRandom* pRandom = &ARandom::GetSingleton()->m_oRandom;

	TVector4        vLaunchPosition = GetLaunchPosition();
	const TVector4* pOwnerDirection = GetOwnerUnit()->GetSteer()->GetDirection();

	TVector4 vRight;
	vRight.CrossProduct( *pOwnerDirection, TVector4::VEC_NEGY );

	TVector4 vUp;
	vUp.Multiply( TVector4::VEC_NEGY, 0.05f );

	TVector4 vSide;
	vSide.Multiply( vRight, 0.05f );

	// The stream is made of four particle trails around the udder
	TVector4 aSpawnPoints[ 4 ];
	TVector4 vForward;
	TVector4 vTemp1;
	TVector4 vTemp2;

	vForward.Multiply( *pOwnerDirection, 0.25f );
	vTemp1.Substract( vLaunchPosition, vSide );
	vTemp2.Add( vTemp1, vUp );
	aSpawnPoints[ 0 ].Add( vTemp2, vForward );

	vForward.Multiply( *pOwnerDirection, 0.15f );
	vTemp1.Add( vLaunchPosition, vSide );
	vTemp2.Add( vTemp1, vUp );
	aSpawnPoints[ 1 ].Add( vTemp2, vForward );

	vForward.Multiply( *pOwnerDirection, 0.05f );
	vTemp1.Substract( vLaunchPosition, vSide );
	vTemp2.Substract( vTemp1, vUp );
	aSpawnPoints[ 2 ].Add( vTemp2, vForward );

	vTemp1.Add( vLaunchPosition, vSide );
	aSpawnPoints[ 3 ].Substract( vTemp1, vUp );

	TVector4 vDirection = GetLaunchDirection();

	if ( m_pStreamProjectile && m_pStreamProjectile->GetAge() > 0.05f )
	{
		if ( !ContinueStream() )
		{
			m_bNewStream = TFALSE;
			return TTRUE;
		}
	}

	const AMilkManager::Data* pData = pMilkManager->GetData();

	for ( TINT i = 0; i < TARRAYSIZE( m_aStreams ); i++ )
	{
		MilkStream& rStream = m_aStreams[ i ];

		TVector4 vSpawnPosition = aSpawnPoints[ i ];
		rStream.fSpawnAccumulator += a_fDeltaTime * pData->fSpawnRate;

		TINT   iNumToSpawn = TINT( rStream.fSpawnAccumulator );
		TFLOAT fProgress   = 1.0f / TFLOAT( iNumToSpawn );
		TFLOAT fTimeStep   = fProgress * a_fDeltaTime;

		TVector4    vSpawnStep;
		TQuaternion qDirection;
		TQuaternion qPrevDirection;

		if ( !m_bNewStream )
		{
			// Spread the particles between the current and the previous spawn point
			vSpawnStep.x = rStream.vPosition.x - vSpawnPosition.x;
			vSpawnStep.y = rStream.vPosition.y - vSpawnPosition.y;
			vSpawnStep.z = rStream.vPosition.z - vSpawnPosition.z;
			vSpawnStep.w = rStream.vPosition.w;
			vSpawnStep.Multiply( fProgress );

			qDirection.SetVectorDirection( vDirection.AsVector3(), TVector3::VEC_POSZ );
			qPrevDirection.SetVectorDirection( rStream.vDirection.AsVector3(), TVector3::VEC_POSZ );
		}

		for ( TINT j = 0; rStream.fSpawnAccumulator >= 1.0f; j++ )
		{
			if ( m_pStreamProjectile->GetNumParticles() >= AMilkProjectile::MAX_NUM_PARTICLES )
			{
				// NOTE: unlike the other places, m_bNewStream isn't reset here
				if ( !ContinueStream() )
					return TTRUE;
			}

			TVector4 vParticleDirection;

			if ( m_bNewStream )
			{
				vParticleDirection = vDirection;
			}
			else
			{
				TQuaternion qParticleDirection;
				TQuaternion::Slerp( qParticleDirection, qDirection, qPrevDirection, TFLOAT( j ) * fProgress );
				vParticleDirection = RotateVectorByQuaternion( qParticleDirection, TVector4::VEC_POSZ );
			}

			TFLOAT fSpeed = m_fLaunchSpeed;

			TVector4 vVelocity;
			vVelocity.Multiply( vParticleDirection, pRandom->GetFloatMinMax( 1.0f - pData->fSpeedVariation, pData->fSpeedVariation + 1.0f ) * fSpeed );

			TVector4 vSideVelocity;
			vSideVelocity.Multiply( vRight, pRandom->GetFloatMinMax( -pData->fSideVariation, pData->fSideVariation ) );
			vVelocity.x += vSideVelocity.x;
			vVelocity.y += vSideVelocity.y;
			vVelocity.z += vSideVelocity.z;

			vVelocity.y += pRandom->GetFloatMinMax( -pData->fUpVariation, pData->fUpVariation );

			TINT iParticle = pMilkManager->SpawnParticle( vSpawnPosition, vVelocity );

			if ( iParticle > -1 )
			{
				m_pStreamProjectile->AddParticle( iParticle );
				pMilkManager->UpdateParticle( iParticle, TFLOAT( j ) * fTimeStep );
			}

			if ( !m_bNewStream )
			{
				vSpawnPosition.x += vSpawnStep.x;
				vSpawnPosition.y += vSpawnStep.y;
				vSpawnPosition.z += vSpawnStep.z;
			}

			rStream.fSpawnAccumulator -= 1.0f;
		}

		rStream.vPosition  = aSpawnPoints[ i ];
		rStream.vDirection = vDirection;
	}

	m_bNewStream = TFALSE;
	return TTRUE;
}

// NOTE: inlined in retail (00668e50)
TBOOL AMilkSquirtProjectileLauncher::ContinueStream()
{
	// Launching uses up ammo, give it back since the stream continues
	GetAmmo()->AddAmmo();
	AProjectile* pProjectile = LaunchProjectile();

	if ( !pProjectile )
	{
		GetAmmo()->UseAmmo();
		return TFALSE;
	}

	m_pStreamProjectile = TSTATICCAST( AMilkProjectile, pProjectile );
	m_pStreamProjectile->SetStreamId( ms_iLastStreamId );

	return TTRUE;
}

// $Barnyard: FUNCTION 00668af0
void AMilkSquirtProjectileLauncher::OnLaunched( AProjectile* a_pProjectile )
{
	a_pProjectile->OnLaunched();
	PlaySquirtSound();
}

// $Barnyard: FUNCTION 00668700
void AMilkSquirtProjectileLauncher::OnThrown( AProjectile* a_pProjectile )
{
	a_pProjectile->OnLaunched();
}

// $Barnyard: FUNCTION 00669640
Toshi::TVector4 AMilkSquirtProjectileLauncher::GetLaunchPosition()
{
	AAnimatableObject* pAnimatableObject = TSTATICCAST( AAnimatedUnit, GetOwnerUnit() )->GetAnimatableObject();

	TINT iBone = pAnimatableObject->GetModelInstance()->GetBoneIndex( TPS8( cb_udder ) );

	TMatrix44 matBone;
	pAnimatableObject->GetModelInstance()->GetBoneTransform( iBone, matBone );

	return TVector4( matBone.m_f41, matBone.m_f42, matBone.m_f43, 1.0f );
}

// $Barnyard: FUNCTION 00668b10
Toshi::TVector4 AMilkSquirtProjectileLauncher::GetLaunchDirection()
{
	TVector4 vDirection = *GetOwnerUnit()->GetSteer()->GetDirection();

	// NOTE: retail calls TVector4::NormaliseXZ (006c7540) here, which flattens the vector onto the XZ plane and normalises it
	vDirection.y        = 0.0f;
	TFLOAT fMagnitudeSq = vDirection.MagnitudeSqXZ();

	if ( fMagnitudeSq != 0.0f )
	{
		TFLOAT fMultiplier = 1.0f / TMath::Sqrt( fMagnitudeSq );
		vDirection.x *= fMultiplier;
		vDirection.z *= fMultiplier;
	}
	else
	{
		vDirection.x = 0.0f;
		vDirection.z = 1.0f;
	}

	vDirection.Multiply( TMath::Cos( m_fVerticalAngle ) );

	return TVector4( vDirection.x, vDirection.y - TMath::Sin( m_fVerticalAngle ), vDirection.z, vDirection.w );
}

// $Barnyard: FUNCTION 00668bf0
void AMilkSquirtProjectileLauncher::Fire()
{
	if ( CanFire() )
	{
		AProjectile* pProjectile = LaunchProjectile();

		if ( !pProjectile )
			return;

		m_pStreamProjectile = TSTATICCAST( AMilkProjectile, pProjectile );
		ms_iLastStreamId += 1;
		m_pStreamProjectile->SetStreamId( ms_iLastStreamId );

		OnFired();

		TVector4 vDirection = GetLaunchDirection();
		TVector4 vPosition  = GetLaunchPosition();
		AParticleEffectManager::GetSingleton()->PlayEffect( m_iMuzzleFlashEffect, vPosition, vDirection );

		m_fSquirtTimeLeft = 0.3f;
		m_bNewStream      = TTRUE;
		return;
	}

	// NOTE: retail rounds with FISTP (round half to even)
	if ( TMath::Round( GetAmmo()->GetAmmo() ) > 0 )
		return;

	// Out of milk
	const TVector4* pDirection = GetOwnerUnit()->GetSteer()->GetDirection();
	TVector4        vPosition  = GetLaunchPosition();
	AParticleEffectManager::GetSingleton()->PlayEffect( m_iOutOfMilkEffect, vPosition, *pDirection );

	ASoundId eSound = ( m_iGender == 1 ) ? soundbank::AM_F_COWMILKEMPTY_F : soundbank::AM_M_COWMILKEMPTY_M;
	ASoundManager::GetSingleton()->PlayCueAtPosition( eSound, GetLaunchPosition() );

	// NOTE: inlined APlayerManager::GetPlayer (0061fae0)
	if ( GetOwnerUnit() && GetOwnerUnit() == APlayerManager::GetSingleton()->GetCurrentPlayerSet()->GetHumanPlayerUnit( 0 ) )
		AHintManager::GetSingleton()->ShowOutOfMilkMessage();
}

// $Barnyard: FUNCTION 00668710
TBOOL AMilkSquirtProjectileLauncher::CanFire()
{
	// NOTE: retail rounds with FISTP (round half to even)
	if ( TMath::Round( GetAmmo()->GetAmmo() ) < 1 )
		return TFALSE;

	return AProjectileLauncher::CanFire();
}

// $Barnyard: FUNCTION 006689f0
void AMilkSquirtProjectileLauncher::PlaySquirtSound()
{
	if ( m_iGender != 0 && m_iGender == 1 )
	{
		ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::AM_F_COWMILKSQUIRT_F, GetLaunchPosition() );
		return;
	}

	ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::AM_M_COWMILKSQUIRT_M, GetLaunchPosition() );
}
