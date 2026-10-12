#include "pch.h"
#include "APrecisionTargetItem.h"
#include "AUnitManager.h"
#include "Animation/AAnimatableObject.h"
#include "Sound/ASoundManager.h"
#include "Terrain/ATerrainInterface.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/Steering/ASteer.h"

#include <Math/TMatrix44.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005078b0, CreateObject 005079d0, CreateObjectInPlace 00507a30 (the constructor is inlined in both)
TDEFINE_CLASS( APrecisionTargetItem );

// Sound of a target hit by the milk
static constexpr ASoundId SOUND_TARGET_HIT = 485;

// NOTE: engine TVector4 method of the original binary (006c7540), see AGTAStyleCameraHelper.cpp
static void NormaliseXZ( TVector4& a_rVector )
{
	a_rVector.y = 0.0f;

	TFLOAT fMagSq = a_rVector.x * a_rVector.x + a_rVector.z * a_rVector.z;

	if ( fMagSq == 0.0f )
	{
		a_rVector.x = 0.0f;
		a_rVector.z = 1.0f;
		return;
	}

	TFLOAT fMag = TMath::Sqrt( fMagSq );
	a_rVector.x = 1.0f / fMag * a_rVector.x;
	a_rVector.z = 1.0f / fMag * a_rVector.z;
}

// $Barnyard: FUNCTION 005078c0
APrecisionTargetItem::APrecisionTargetItem()
{
	m_bHit      = TFALSE;
	m_pPlayer   = TNULL;
	m_fOpacity  = 1.0f;
	m_vPosition = TVector4::VEC_ZERO;
	m_iStreamId = -1;
	m_iMapIcon  = -1;
	m_bActive   = TTRUE;
}

APrecisionTargetItem::~APrecisionTargetItem()
{
}

// $Barnyard: FUNCTION 00507ad0
TBOOL APrecisionTargetItem::Create( AUnitTransform* a_pTransform )
{
	if ( !AAnimatedUnit::Create( a_pTransform ) )
		return TFALSE;

	GetModelInstance()->SetUnlit();
	AddFlags( FLAGS_AI_ENABLED );
	return TTRUE;
}

// $Barnyard: FUNCTION 00507a80
void APrecisionTargetItem::SetPosition( const TVector4& a_rcPosition )
{
	m_vPosition = a_rcPosition;
	GetSteer()->SetPosition( m_vPosition );

	// NOTE: retail also passes IsUnk34Set() to AssignHash, which doesn't use it
	IsUnk34Set();
	AUnitManager::GetSingleton()->GetUnitHash()->AssignHash( this );
}

// $Barnyard: FUNCTION 00507b00
void APrecisionTargetItem::SetHit( TBOOL a_bHit, TINT a_iStreamId )
{
	if ( !a_bHit )
	{
		// Back at its position, fades in
		m_fOpacity = 0.0f;
		SetMovement( TFALSE );
		SetVisible( TTRUE );
		AddFlags( FLAGS_MILK_TARGET );
		GetAnimatableObject()->SetOpacity( m_fOpacity, TTRUE );
		GetSteer()->SetVelocity( TVector4::VEC_ZERO );
		GetSteer()->SetPosition( m_vPosition );
		SetHashed( TTRUE );
		SetNoQuery( TFALSE );
		m_bHit      = TFALSE;
		m_bCounted  = TFALSE;
		m_iStreamId = -1;
		return;
	}

	if ( m_bHit == a_bHit )
		return;

	m_bHit      = a_bHit;
	m_bCounted  = TFALSE;
	m_fOpacity  = 1.0f;
	m_iStreamId = a_iStreamId;
	SetMovement( TTRUE );
	RemoveFlags( FLAGS_MILK_TARGET );
	ASoundManager::GetSingleton()->PlayCueAtPosition( SOUND_TARGET_HIT, *GetSteer()->GetPosition() );
	SetHashed( TFALSE );
	SetNoQuery( TTRUE );

	// Flies away from the player
	TVector4 vDirection = TVector4::VEC_POSZ;

	if ( m_pPlayer )
	{
		vDirection.Substract( *GetSteer()->GetPosition(), *m_pPlayer->GetSteer()->GetPosition() );
		NormaliseXZ( vDirection );
		vDirection.y = -2.0f;
		vDirection.Normalise();
	}

	TVector4 vVelocity;
	vVelocity.Multiply( vDirection, 5.0f );
	GetSteer()->SetVelocity( vVelocity );
}

// $Barnyard: FUNCTION 00507cd0
void APrecisionTargetItem::Park( TBOOL a_bKeep )
{
	if ( a_bKeep )
		return;

	GetSteer()->SetVelocity( TVector4::VEC_ZERO );
	GetSteer()->SetPosition( m_vPosition );
	RemoveFlags( FLAGS_MILK_TARGET );
	SetVisible( TFALSE );
}

// $Barnyard: FUNCTION 00507950
TBOOL APrecisionTargetItem::ConsumeHit()
{
	if ( m_bCounted || !m_bHit )
		return TFALSE;

	m_bCounted = TTRUE;
	return TTRUE;
}

// $Barnyard: FUNCTION 00507990
void APrecisionTargetItem::SetActive( TBOOL a_bActive )
{
	m_bActive = a_bActive;

	if ( !a_bActive )
		GetAnimatableObject()->SetOpacity( 0.0f, TTRUE );
}

// $Barnyard: FUNCTION 00507d20
void APrecisionTargetItem::UpdateAI( TFLOAT a_fDeltaTime )
{
	if ( !m_bActive )
		return;

	if ( IsVisible() )
	{
		TVector4 vPosition  = *GetSteer()->GetPosition();
		TVector4 vDirection = *GetSteer()->GetDirection();

		// Keep it above the ground
		AInteriorNetwork* pNetwork = AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork();
		TINT              iArea    = pNetwork->FindAreaAtPosition( *GetSteer()->GetPosition(), -1 );
		TFLOAT            fGround;

		if ( iArea > -1 )
			fGround = ATerrainInterface::GetSingleton()->GetGroundHeight( vPosition.x, pNetwork->m_pINetwork->m_pAreas[ iArea ].m_fBaseHeight, vPosition.z, TCollisionCommon::TOSHICGROUPFLAG( 0 ) );
		else
			fGround = ATerrainInterface::GetSingleton()->GetGroundHeightTop( vPosition.x, vPosition.z, TCollisionCommon::TOSHICGROUPFLAG( 0 ) );

		if ( vPosition.y > fGround )
		{
			vPosition.y = fGround - 1.0f;
			GetSteer()->SetPosition( vPosition );
		}

		// Spin
		vDirection.AsVector3().RotateY( 3.0f * a_fDeltaTime );
		GetSteer()->SetDirection( vDirection );

		if ( m_bHit )
		{
			// Tumbles and fades out
			TMatrix44 oTransform = TMatrix44::IDENTITY;
			oTransform.RotateX( TMath::TWO_PI * m_fOpacity );
			oTransform.RotateZ( TMath::TWO_PI * m_fOpacity );
			oTransform.RotateY( TMath::TWO_PI * m_fOpacity );
			oTransform.SetTranslation( vPosition );

			if ( m_fOpacity <= 0.0f )
			{
				SetVisible( TFALSE );
			}
			else
			{
				GetAnimatableObject()->SetOpacity( m_fOpacity, TTRUE );
				m_fOpacity -= 1.0f * a_fDeltaTime;
			}

			GetAnimatableObject()->GetModelInstance()->SetTransform( oTransform );
		}
		else
		{
			// Fades in
			if ( m_fOpacity >= 1.0f )
			{
				GetAnimatableObject()->SetOpacity( 1.0f, TTRUE );
			}
			else
			{
				GetAnimatableObject()->SetOpacity( m_fOpacity, TTRUE );
				m_fOpacity = 1.0f * a_fDeltaTime + m_fOpacity;
			}

			GetAnimatableObject()->GetModelInstance()->SetTransform( vPosition, vDirection );
		}
	}

	AUnit::UpdateAI( a_fDeltaTime );
}
