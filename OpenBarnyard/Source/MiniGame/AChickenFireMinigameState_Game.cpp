#include "pch.h"
#include "Render/TCameraObject.h"
#include "AChickenFireMinigameState.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AAnimatableObjectType.h"
#include "Animation/AModel.h"
#include "ARandom.h"
#include "Cameras/ACamera.h"
#include "Cameras/ACameraManager.h"
#include "FX/AAniEffectManager.h"
#include "FX/AGameTimeFXManager.h"
#include "FX/AParticleSystem.h"
#include "FX/AParticleEffectManager.h"
#include "FX/ASplashManager.h"
#include "Helpers/ATreeManager.h"
#include "Input/AInputHandler.h"
#include "Input/AInputMapManager.h"
#include "Locale/ALocale.h"
#include "Locale/ALocaleManager.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"
#include "Sound/AMusicManager.h"
#include "Sound/APositionalSoundSet.h"
#include "Sound/ASoundManager.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/AAnimatedUnit.h"
#include "UnitMovement/Steering/ASteer.h"

#ifdef TRENDERINTERFACE_DX8
#  include "Platform/DX8/AWorldShader/AWorldShader_DX8.h"
#  include "Platform/DX8/AWorldShader/AWorldMaterial_DX8.h"
#endif

#include "Physics/ACollisionManager.h"
#include "Physics/ACollisionObject.h"
#include "Physics/ACollisionModelInstance.h"

#include <Math/TMathInline.h>
#include <Render/TModel.h>
#include <Toshi/TString8.h>
#include "SoundBank/launch.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

#undef PI

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( flightloop );
TPSTRING8_DECLARE( launchPullBack );
TPSTRING8_DECLARE( launch );
TPSTRING8_DECLARE( flightTumble );
TPSTRING8_DECLARE( flightFlap );
TPSTRING8_DECLARE( flightFlap2 );
TPSTRING8_DECLARE( flightFlapFast );
TPSTRING8_DECLARE( flightDive );
TPSTRING8_DECLARE( prelaunch );
TPSTRING8_DECLARE( impact );
TPSTRING8_DECLARE( prelaunchhold );
TPSTRING8_DECLARE( postImpactSuccess );
TPSTRING8_DECLARE( postImpactFail );
TPSTRING8_DECLARE( slide );
TPSTRING8_DECLARE( impacttotumble );
TPSTRING8_DECLARE( tumble_end );
TPSTRING8_DECLARE( tumble_endhit );
TPSTRING8_DECLARE( impactwobble );
TPSTRING8_DECLARE( ch_scorecard_01 );
TPSTRING8_DECLARE( ch_scorecard_02 );
TPSTRING8_DECLARE( ch_scorecard_03 );
TPSTRING8_DECLARE( ch_scorecard_04 );
TPSTRING8_DECLARE( ch_scorecard_05 );
TPSTRING8_DECLARE( ch_scorecard_06 );
TPSTRING8_DECLARE( ch_scorecard_07 );
TPSTRING8_DECLARE( ch_scorecard_08 );
TPSTRING8_DECLARE( ch_scorecard_09 );
TPSTRING8_DECLARE( ch_scorecard_10 );
TPSTRING8_DECLARE( cb_llocator );
TPSTRING8_DECLARE( score_bad );
TPSTRING8_DECLARE( score_good );
TPSTRING8_DECLARE( peckringlarge );
TPSTRING8_DECLARE( barnbuck );
TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( AChickenFireMinigameState );

static constexpr TINT   MAX_BOOSTS         = 5;      // 0x00773888
static constexpr TINT   RING_SCORE         = 5;      // 0x007738a8
static constexpr TFLOAT MAX_SPEED          = 3.0f;   // 0x0077388c
static constexpr TFLOAT MIN_FLIGHT_SPEED   = 10.0f;  // 0x00773890
static constexpr TFLOAT MAX_FLIGHT_SPEED   = 60.0f;  // 0x00773894
static constexpr TFLOAT MAX_HEIGHT         = -50.0f; // 0x00773898, Y goes down
static constexpr TFLOAT STALL_SPEED        = 0.2f;   // 0x0077389c
static constexpr TFLOAT SLOW_SPEED         = 0.1f;   // 0x007738a0
static constexpr TFLOAT TIME_BONUS_SECONDS = 45.0f;  // 0x007738a4
static constexpr TFLOAT TEXT_REFRESH_TIME  = 0.2f;   // 0x00773884
static constexpr TFLOAT PICKUP_DISTANCE    = 2.5f;
static constexpr TINT   NUM_SCORECARDS     = 10;

// NOTE: 0x00782824 is a float in .bss that nothing writes, so the speed is clipped to 0.0f
static constexpr TFLOAT MIN_SPEED = 0.0f;

static constexpr TINT LOCALE_BASH_INSTRUCTIONS = 1031;
static constexpr TINT LOCALE_OUT_OF_BOUNDS     = 1042;
static constexpr TINT LOCALE_WRONG_WAY         = 1044;
static constexpr TINT LOCALE_FLAP              = 1046;

// The vibration of the main controller after a collision (does nothing on PC)
static void Rumble()
{
	AInputHandler::RumbleParams oRumble = { { 1.0f, 0.0f, 0.0f, 0.5f, 0.0f, 1.0f, 1.0f }, 1 };
	AInputHandler::GetSingleton()->SetRumble( AInputHandler::GetSingleton()->GetMainController(), &oRumble );
}

// NOTE: inlined in retail
static void StopEffect( AParticleEffectHandle*& a_rpHandle )
{
	if ( a_rpHandle )
	{
		a_rpHandle->GetInstance()->Stop();
		a_rpHandle = TNULL;
	}
}

// NOTE: inlined in retail
static void KillEffect( AParticleEffectHandle*& a_rpHandle )
{
	if ( a_rpHandle )
	{
		a_rpHandle->GetInstance()->Kill();
		a_rpHandle = TNULL;
	}
}

// Sum of the normals of the terrain triangles touching a_rcSphere that face against a_rcDirection
// NOTE: inlined in retail
static TVector4 GetSphereNormal( const TVector4& a_rcSphere, const TVector4& a_rcDirection )
{
	ACollisionModelRayHitList oHitList;
	ATerrainInterface::GetSingleton()->CollideSphere( a_rcSphere, &oHitList, -1, 0 );

	TVector4 vNormal = TVector4::VEC_ZERO;

	for ( TINT i = 0; i < oHitList.iNumHits; i++ )
	{
		TVector4 vHitNormal = oHitList.aHits[ i ].GetNormal();

		if ( TVector4::DotProduct3( vHitNormal, a_rcDirection ) <= 0.0f )
		{
			vNormal.x += vHitNormal.x;
			vNormal.y += vHitNormal.y;
			vNormal.z += vHitNormal.z;
		}
	}

	return vNormal;
}

// $Barnyard: FUNCTION 00479f70
void AChickenFireMinigameState::SetPeckFlightTransform( TVector4 a_vPosition, TVector4 a_vDirection )
{
	AModelInstance* pModelInstance = TSTATICCAST( AAnimatedUnit, GetPeckUnit() )->GetModelInstance();

	// Peck's model lies along the X axis
	TQuaternion quatModel = TQuaternion::IDENTITY;
	quatModel.RotateAroundAxis( TVector4::VEC_POSX.AsVector3(), TMath::HALF_PI );

	TVector4 vLookDirection( -a_vDirection.x, -a_vDirection.y, -a_vDirection.z, a_vDirection.w );

	TMatrix44 matTransform;
	matTransform.LookAtDirection( vLookDirection, TVector4::VEC_NEGY );

	TQuaternion quatLook;
	TMatrix44::MatToQuat( quatLook, matTransform );

	// Roll around the flight direction, more the faster Peck is
	TQuaternion quatBank = TQuaternion::IDENTITY;
	TFLOAT      fSpeed   = m_fPower;
	TMath::Clip( fSpeed, 0.0f, 1.0f );
	quatBank.RotateAroundAxis( a_vDirection.AsVector3(), fSpeed * m_fBankAngle );

	quatBank *= quatLook * quatModel;

	matTransform.SetFromQuaternion( quatBank );
	matTransform.GetTranslation() = a_vPosition;
	pModelInstance->SetTransform( matTransform );

	GetPeckUnit()->GetSteer()->SetPosition( a_vPosition );
}

// $Barnyard: FUNCTION 0047a270
void AChickenFireMinigameState::PlayPeckAnim( TINT a_eAnim )
{
	AAnimatableObject* pPeck = TSTATICCAST( AAnimatedUnit, GetPeckUnit() )->GetAnimatableObject();

	const TPString8* pAnimName;

	switch ( a_eAnim )
	{
		case PECKANIM_FLIGHTLOOP: pAnimName = &TPS8( flightloop ); break;
		case PECKANIM_LAUNCHPULLBACK: pAnimName = &TPS8( launchPullBack ); break;
		case PECKANIM_LAUNCH: pAnimName = &TPS8( launch ); break;
		case PECKANIM_FLIGHTTUMBLE: pAnimName = &TPS8( flightTumble ); break;
		case PECKANIM_FLIGHTFLAP: pAnimName = &TPS8( flightFlap ); break;
		case PECKANIM_FLIGHTFLAP2: pAnimName = &TPS8( flightFlap2 ); break;
		case PECKANIM_FLIGHTFLAPFAST: pAnimName = &TPS8( flightFlapFast ); break;
		case PECKANIM_FLIGHTDIVE: pAnimName = &TPS8( flightDive ); break;
		case PECKANIM_PRELAUNCH: pAnimName = &TPS8( prelaunch ); break;
		case PECKANIM_IMPACT: pAnimName = &TPS8( impact ); break;
		case PECKANIM_PRELAUNCHHOLD: pAnimName = &TPS8( prelaunchhold ); break;
		case PECKANIM_POSTIMPACTSUCCESS: pAnimName = &TPS8( postImpactSuccess ); break;
		case PECKANIM_POSTIMPACTFAIL: pAnimName = &TPS8( postImpactFail ); break;
		case PECKANIM_SLIDE: pAnimName = &TPS8( slide ); break;
		case PECKANIM_IMPACTTOTUMBLE: pAnimName = &TPS8( impacttotumble ); break;
		case PECKANIM_TUMBLE_END: pAnimName = &TPS8( tumble_end ); break;
		case PECKANIM_TUMBLE_ENDHIT: pAnimName = &TPS8( tumble_endhit ); break;
		case PECKANIM_IMPACTWOBBLE: pAnimName = &TPS8( impactwobble ); break;
		default: return;
	}

	if ( !pPeck->IsAnimPlaying( *pAnimName, 0.0f ) )
		pPeck->PlayAnimation( *pAnimName );
}

// $Barnyard: FUNCTION 0047a590
void AChickenFireMinigameState::UpdateFlightCamera( TFLOAT a_fDeltaTime )
{
	ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

	TVector4 vCameraPos = pCamera->GetMatrix().GetTranslation();

	TVector4 vCameraDir;
	pCamera->GetDirection( vCameraDir );

	TMatrix44 matPeck;
	TSTATICCAST( AAnimatedUnit, GetPeckUnit() )->GetModelInstance()->GetLocalTransform( matPeck );

	// Moves sideways while turning
	TVector4 vRight( matPeck.m_f11, 0.0f, matPeck.m_f13, matPeck.m_f14 );
	vRight.Normalise();
	vRight.y = 0.0f;
	vRight.Normalise();

	TVector4 vSide;
	vSide.Multiply( vRight, m_fYawRate );

	TVector4 vUp;
	vUp.Multiply( TVector4::VEC_NEGY, 1.5f );

	TVector4 vBack;
	vBack.Multiply( m_vPeckDir, 1.0f );

	// Behind and above Peck
	TVector4 vTarget(
	    m_vPeckPos.x - vBack.x + vUp.x,
	    m_vPeckPos.y - vBack.y + vUp.y,
	    m_vPeckPos.z - vBack.z + vUp.z,
	    m_vPeckPos.w
	);

	vCameraPos.Lerp4( vCameraPos, vTarget, a_fDeltaTime * 5.0f );

	TVector4 vSideOffset;
	vSideOffset.Multiply( vSide, 0.5f );
	vCameraPos.x += vSideOffset.x;
	vCameraPos.y += vSideOffset.y;
	vCameraPos.z += vSideOffset.z;

	// Keep within 3 units of Peck
	TVector4 vToPeck( m_vPeckPos.x - vCameraPos.x, m_vPeckPos.y - vCameraPos.y, m_vPeckPos.z - vCameraPos.z, m_vPeckPos.w );
	TFLOAT   fDistance = vToPeck.Magnitude();
	vToPeck.Normalise();

	if ( fDistance > 3.0f )
	{
		TVector4 vOffset;
		vOffset.Multiply( vToPeck, fDistance - 3.0f );
		vCameraPos.x += vOffset.x;
		vCameraPos.y += vOffset.y;
		vCameraPos.z += vOffset.z;
	}

	TFLOAT fFOV = pCamera->GetFOV();

	if ( m_bBoosted == TTRUE || m_fPower > 1.0f )
		pCamera->SetFOV( TMath::LERP( fFOV, TMath::HALF_PI, a_fDeltaTime * 1.5f ) );
	else
		pCamera->SetFOV( TMath::LERP( fFOV, 1.3089969f, a_fDeltaTime * 1.5f ) );

	pCamera->GetMatrix().GetTranslation() = vCameraPos;

	if ( m_eState == STATE_STOPPED )
	{
		TVector4 vFlatDir( m_vPeckDir.x, 0.0f, m_vPeckDir.z, m_vPeckDir.w );
		vFlatDir.Normalise();

		TVector4 vAhead;
		vAhead.Multiply( vFlatDir, 2.0f );

		TVector4 vLookAt( vAhead.x + m_vPeckPos.x - vCameraPos.x, vAhead.y + m_vPeckPos.y - vCameraPos.y, vAhead.z + m_vPeckPos.z - vCameraPos.z, m_vPeckPos.w );

		vCameraDir.Lerp4( vCameraDir, vLookAt, a_fDeltaTime + a_fDeltaTime );
		vCameraDir.w = 1.0f;
		vCameraDir.Normalise();
		pCamera->LookAtDirection( vCameraDir );
	}
	else
	{
		pCamera->LookAtDirection( m_vPeckDir );
	}
}

// $Barnyard: FUNCTION 0047ab30
TBOOL AChickenFireMinigameState::CheckFlight( TFLOAT a_fDeltaTime )
{
	TFLOAT fDistanceX     = TMath::Abs( m_vPeckPos.x );
	TBOOL  bStateChanged  = TFALSE;
	TBOOL  bPlayFlapSound = TFALSE;

	if ( fDistanceX <= 30.0f )
	{
		TVector4 vFlatDir( m_vPeckDir.x, 0.0f, m_vPeckDir.z, m_vPeckDir.w );
		vFlatDir.Normalise();

		TBOOL bWrongWay = TVector4::DotProduct3( vFlatDir, TVector4::VEC_NEGZ ) < 0.0f;

		if ( bWrongWay )
		{
			SetText( ALocaleManager::Interface()->GetString( LOCALE_WRONG_WAY ) );
			m_fWrongWayTime += a_fDeltaTime;

			if ( m_vPeckPos.z >= 100.0f )
			{
				// Flew back past the slingshot
				m_eState = STATE_DESCEND;
				PlayPeckAnim( PECKANIM_FLIGHTTUMBLE );
				m_fPower      = 0.1f;
				bStateChanged = TTRUE;
			}
			else
			{
				bPlayFlapSound = TTRUE;
			}
		}
		else if ( m_fPower >= STALL_SPEED )
		{
			m_fWrongWayTime = 0.0f;
		}
		else
		{
			if ( m_iNumBoosts <= 0 )
				SetText( ALocaleManager::Interface()->GetString( LOCALE_FLAP ) );
			else
				SetText( m_wszInstructions );

			if ( m_fPower > SLOW_SPEED )
			{
				m_fStallTime = 0.0f;
				PlayPeckAnim( PECKANIM_FLIGHTFLAP );
				bPlayFlapSound = TTRUE;
			}
			else
			{
				// Stalled: Peck drops after 2 seconds of flapping
				PlayPeckAnim( PECKANIM_FLIGHTFLAPFAST );
				m_fStallTime += a_fDeltaTime;

				if ( m_fStallTime < 2.0f )
				{
					bPlayFlapSound = TTRUE;
				}
				else
				{
					m_eState      = STATE_DIVE;
					m_fPower      = 0.0f;
					bStateChanged = TTRUE;
					m_fStallTime  = 0.0f;
				}
			}
		}
	}
	else if ( fDistanceX > 43.0f )
	{
		// Out of bounds
		m_oTextBox.Hide();
		m_eState = STATE_FALLING;
		m_fTimer = 0.0f;
		ASoundManager::GetSingleton()->PlayCue( soundbank::LAUNCH_PECK_LOST );
		bStateChanged = TTRUE;

		StopEffect( m_pFlightFeathersL );
		StopEffect( m_pFlightFeathersR );

		m_HUDParams.SetFlags( 0 );
		m_oPortrait.Hide();
		m_oFeathers.SetVisible( TFALSE );
	}
	else
	{
		// Close to the bounds
		SetText( ALocaleManager::Interface()->GetString( LOCALE_OUT_OF_BOUNDS ) );
		bPlayFlapSound = TTRUE;
	}

	if ( bPlayFlapSound )
	{
		if ( m_iCue_C48 != -1 )
			return bStateChanged;

		m_iCue_C48 = ASoundManager::GetSingleton()->PlayCueEx( soundbank::LAUNCH_WARNING, 0.5f, TFALSE, 0.0f, -1 );
		return TFALSE;
	}

	if ( m_iCue_C48 != -1 )
	{
		ASoundManager::GetSingleton()->StopCue( m_iCue_C48 );
		m_iCue_C48 = -1;
	}

	return bStateChanged;
}

// $Barnyard: FUNCTION 0047aef0
void AChickenFireMinigameState::StopSounds( TBOOL a_bFadeOut )
{
	TINT* apCues[] = { &m_iCue_C3C, &m_iCue_C40, &m_iCue_C44, &m_iCue_C48, &m_iCue_C4C, &m_iCue_C50, &m_iCue_C54, &m_iCue_C58, &m_iCue_C5C, &m_iCue_C60 };

	ASoundManager* pSoundManager = ASoundManager::GetSingleton();

	if ( a_bFadeOut )
	{
		for ( TINT i = 0; i < TARRAYSIZE( apCues ); i++ )
			pSoundManager->FadeOutCue( *apCues[ i ], 0.5f, TTRUE, 3.0f );

		AMusicManager::GetSingleton()->ClearForcedSound( TTRUE );
	}
	else
	{
		for ( TINT i = 0; i < TARRAYSIZE( apCues ); i++ )
			pSoundManager->StopCue( *apCues[ i ] );

		AMusicManager::GetSingleton()->ClearForcedSound( TFALSE );
	}

	for ( TINT i = 0; i < TARRAYSIZE( apCues ); i++ )
		*apCues[ i ] = -1;
}

// $Barnyard: FUNCTION 0047b910
TINT AChickenFireMinigameState::CheckPeckCollision( TFLOAT a_fDeltaTime, TFLOAT* a_pOutDistance )
{
	TVector4 vDirection = m_vPeckDir;
	vDirection.Normalise();

	TFLOAT                    fMaxDistance = 2.0f;
	ACollisionModelRayHitList oTerrainHits;
	ATerrainInterface::GetSingleton()->CollideRay( m_vPeckPos, vDirection, fMaxDistance, &oTerrainHits, 0, TTRUE );

	TFLOAT fDistance = 50.0f;

	if ( oTerrainHits.iNumHits <= 0 )
	{
		// No terrain, check the objects (trees, power poles...)
		ARayData oRay( m_vPeckPos, vDirection, 2.0f );

		T2DynamicVector<ACollisionHit> vecHits( GetGlobalAllocator(), 64, 64 );
		ACollisionHit                  oClosestHit;

		if ( ACollisionManager::GetSingleton()->CollideRayObjects( GetPeckUnit(), oRay, &vecHits, oClosestHit, 0, ACollisionObject::TYPE_IGNORED ) )
		{
			for ( TINT i = 0; i < vecHits.Size(); i++ )
			{
				ACollisionHit& rHit  = vecHits[ i ];
				TUINT          eType = rHit.pObject->GetType();

				if ( !HASANYFLAG( eType, ACollisionObject::TYPE_SIMPLE ) )
					continue;

				if ( HASANYFLAG( eType, ACollisionObject::TYPE_FOLIAGE ) || HASANYFLAG( eType, ACollisionObject::TYPE_BUSH ) )
				{
					// Power pole
					*a_pOutDistance = rHit.fDistance;
					m_eState        = STATE_TUMBLE;
					m_fPitchRate    = 0.0f;
					m_fBankAngle    = 0.0f;
					m_fYawRate      = 0.0f;
					m_fTimer        = 0.5f;
					PlayPeckAnim( PECKANIM_FLIGHTTUMBLE );

					AParticleEffectManager::GetSingleton()->PlayEffect( m_iFeatherExplodeEffect, m_vPeckPos, TVector4::VEC_NEGY );

					if ( !ASoundManager::GetSingleton()->IsCueUsed( m_iCue_C58 ) )
						m_iCue_C58 = ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::LAUNCH_PECK_IMPACT, m_vPeckPos );

					return HIT_POWER;
				}

				if ( HASANYFLAG( eType, ACollisionObject::TYPE_TRUNK ) )
				{
					// Tree
					m_eState = STATE_SPLASHED;
					m_fTimer = 0.0f;
					PlayPeckAnim( PECKANIM_IMPACTWOBBLE );

					StopEffect( m_pFlightFeathersL );
					StopEffect( m_pFlightFeathersR );

					StopSounds( TFALSE );
					ASoundManager::GetSingleton()->PlayCueEx( soundbank::LAUNCH_RECORDSCRATCH, 0.5f, TFALSE, 0.0f, -1 );

					AParticleEffectManager::GetSingleton()->PlayEffect( m_iFeatherExplodeEffect, m_vPeckPos, rHit.vNormal );
					AParticleEffectManager::GetSingleton()->PlayEffect( m_iDustPuffEffect, m_vPeckPos, rHit.vNormal );

					// The leaves fall from every FOB of the tree
					ATreeManager::TreeInstance* pTree = ATreeManager::GetSingleton()->FindTreeByCollision( rHit.pObject->GetModelInstance() );

					if ( pTree )
					{
						TINT iNumFOBs = ATreeManager::GetSingleton()->GetNumFOBs( pTree );

						for ( TINT k = 0; k < iNumFOBs; k++ )
						{
							TVector4 vFOBPos = ATreeManager::GetSingleton()->GetFOBPosition( pTree, k );
							AParticleEffectManager::GetSingleton()->PlayEffect( m_iLeavesFallEffect, vFOBPos, TVector4::VEC_NEGY );
						}
					}

					// NOTE: retail faces Peck against the normal of the closest hit, not of this one
					TVector4 vFacing( -oClosestHit.vNormal.x, -oClosestHit.vNormal.y, -oClosestHit.vNormal.z, oClosestHit.vNormal.w );

					TVector4 vPushBack;
					vPushBack.Multiply( vFacing, 0.2f );

					TVector4 vStep;
					vStep.Multiply( m_vPeckDir, rHit.fDistance );
					vStep.Multiply( 0.75f );

					TVector4 vPosition;
					vPosition.Add( m_vPeckPos, vStep );
					vPosition.Substract( vPushBack );
					m_vPeckPos = vPosition;

					TSTATICCAST( AAnimatedUnit, GetPeckUnit() )->GetModelInstance()->SetTransformUnusedUp( vPosition, vFacing, TVector4::VEC_NEGY );

					if ( !ASoundManager::GetSingleton()->IsCueUsed( m_iCue_C58 ) )
						m_iCue_C58 = ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::LAUNCH_PECK_IMPACT, m_vPeckPos );

					ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::LAUNCH_PECK_FAIL, m_vPeckPos );

					*a_pOutDistance = rHit.fDistance;
					return HIT_BOUNCE;
				}
			}
		}

		*a_pOutDistance = -1.0f;
		return HIT_NONE;
	}

	// The closest terrain hit
	TINT iClosest = -1;

	for ( TINT i = 0; i < oTerrainHits.iNumHits; i++ )
	{
		if ( oTerrainHits.aHits[ i ].fDistance < fDistance )
		{
			fDistance = oTerrainHits.aHits[ i ].fDistance;
			iClosest  = i;
		}
	}

	*a_pOutDistance = fDistance;

	if ( iClosest <= -1 )
	{
		*a_pOutDistance = -1.0f;
		return HIT_NONE;
	}

	ACollisionModelRayHitList::Hit& rHit      = oTerrainHits.aHits[ iClosest ];
	TModelCollisionData*            pCollData = rHit.pInstance->GetCollisionData( rHit.iModel );
	const TString8&                 strGroup  = pCollData->GetCollGroup( pCollData->GetGroupForTriangleImp( rHit.uiFaceId ) ).strName;

	TINT iSuccess = GetSuccessIndex( strGroup );

	if ( iSuccess != 0 )
	{
		Rumble();
		return iSuccess;
	}

	if ( strGroup.Compare( "board" ) == 0 )
	{
		Rumble();
		return HIT_BOARD;
	}

	if ( strGroup.Compare( "water" ) == 0 )
	{
		TSTATICCAST( AAnimatedUnit, GetPeckUnit() )->SetVisible( TFALSE );
		m_eState = STATE_SPLASHED;
		m_fTimer = 0.0f;

		StopEffect( m_pFlightFeathersL );
		StopEffect( m_pFlightFeathersR );

		StopSounds( TFALSE );
		m_iCue_C58 = ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::LAUNCH_SPLASH, m_vPeckPos );

		TVector4 vStep;
		vStep.Multiply( m_vPeckDir, fDistance );
		vStep.Multiply( 0.95f );

		TVector4 vSplashPos;
		vSplashPos.Add( m_vPeckPrevPos, vStep );

		AParticleEffectManager::GetSingleton()->PlayEffect( m_iWaterSplashAEffect, vSplashPos, TVector4::VEC_NEGY );
		AParticleEffectManager::GetSingleton()->PlayEffect( m_iWaterSplashBEffect, vSplashPos, TVector4::VEC_NEGY );
		AParticleEffectManager::GetSingleton()->PlayEffect( m_iWaterRippleEffect, vSplashPos, TVector4::VEC_NEGY );
		ASplashManager::GetSingleton()->Spawn( m_vPeckPos, TVector4::VEC_NEGY, 1.0f, ASplashManager::SPLASHTYPE_WATER );

		Rumble();
		return HIT_WATER;
	}

	if ( strGroup.Compare( "col_power" ) == 0 )
	{
		m_eState     = STATE_TUMBLE;
		m_fPitchRate = 0.0f;
		m_fBankAngle = 0.0f;
		m_fYawRate   = 0.0f;
		m_fTimer     = 0.5f;
		PlayPeckAnim( PECKANIM_FLIGHTTUMBLE );

		AParticleEffectManager::GetSingleton()->PlayEffect( m_iFeatherExplodeEffect, m_vPeckPos, TVector4::VEC_NEGY );

		if ( !ASoundManager::GetSingleton()->IsCueUsed( m_iCue_C58 ) )
			m_iCue_C58 = ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::LAUNCH_PECK_IMPACT, m_vPeckPos );

		Rumble();
		return HIT_POWER;
	}

	if ( strGroup.Compare( "col_tower" ) == 0 )
	{
		m_fPitchRate = 0.0f;
		m_fBankAngle = 0.0f;
		m_fYawRate   = 0.0f;
		m_eState     = STATE_DESCEND;
		PlayPeckAnim( PECKANIM_FLIGHTTUMBLE );

		AParticleEffectManager::GetSingleton()->PlayEffect( m_iFeatherExplodeEffect, m_vPeckPos, TVector4::VEC_NEGY );
		m_fPower = 0.0f;

		StopEffect( m_pFlightFeathersL );
		StopEffect( m_pFlightFeathersR );

		StopSounds( TFALSE );
		ASoundManager::GetSingleton()->PlayCueEx( soundbank::LAUNCH_RECORDSCRATCH, 0.5f, TFALSE, 0.0f, -1 );

		if ( !ASoundManager::GetSingleton()->IsCueUsed( m_iCue_C58 ) )
			m_iCue_C58 = ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::LAUNCH_PECK_IMPACT, m_vPeckPos );

		Rumble();
		return HIT_TOWER;
	}

	if ( strGroup.Compare( "collision_barn" ) == 0 )
	{
		Rumble();
		return HIT_BARN;
	}

	// Any other terrain: get the normal of the surface around the hit point
	TVector4 vStep;
	vStep.Multiply( m_vPeckDir, fDistance );

	TVector4 vSphere;
	vSphere.Add( m_vPeckPos, vStep );
	vSphere.w = 0.5f;

	TVector4 vNormal = GetSphereNormal( vSphere, m_vPeckDir );
	vNormal.Normalise();

	TVector4 vInvDir( -m_vPeckDir.x, -m_vPeckDir.y, -m_vPeckDir.z, m_vPeckDir.w );
	TFLOAT   fHeadOn = TVector4::DotProduct3( vInvDir, vNormal );
	TFLOAT   fFloor  = TVector4::DotProduct3( vNormal, TVector4::VEC_NEGY );

	TBOOL bCrash = TMath::Abs( fHeadOn ) > 0.6f;

	if ( !bCrash )
	{
		m_fPitchRate = 0.0f;
		m_fBankAngle = 0.0f;
		m_fYawRate   = 0.0f;

		AParticleEffectManager::GetSingleton()->PlayEffect( m_iFeatherExplodeEffect, m_vPeckPos, vNormal );
		AParticleEffectManager::GetSingleton()->PlayEffect( m_iDustPuffEffect, m_vPeckPos, vNormal );

		if ( !ASoundManager::GetSingleton()->IsCueUsed( m_iCue_C58 ) )
			m_iCue_C58 = ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::LAUNCH_PECK_IMPACT, m_vPeckPos );

		Rumble();

		if ( TMath::Abs( fFloor ) <= 0.5f )
		{
			// Glanced off a wall
			Reflect( m_vPeckDir, m_vPeckDir, vNormal );
			m_eState = STATE_TUMBLE;
			m_fTimer = 0.5f;
			PlayPeckAnim( PECKANIM_FLIGHTTUMBLE );

			return HIT_BOUNCE;
		}

		m_fTimer = 0.0f;

		if ( TMath::Abs( m_vPeckPos.x ) <= 40.0f )
		{
			// Hit the ground: slide along it
			m_eState    = STATE_ROLLING;
			m_fUnk_2044 = 5.0f;

			TVector4 vSlideStep;
			vSlideStep.Multiply( m_vPeckDir, fDistance - 0.55f );
			m_vPeckPos.AsVector3().Add( vSlideStep.AsVector3() );

			TVector4 vPeckSphere = m_vPeckPos;
			vPeckSphere.w        = 0.5f;
			ATerrainInterface::GetSingleton()->PushOutSphere( vPeckSphere, m_vPeckPos, -1, 0 );

			SetPeckTransform( m_vPeckPos, m_vPeckDir );

			// Remove the part of the direction going into the ground
			TVector4 vIntoGround = m_vPeckDir;
			vIntoGround.ProjectOnto( vNormal );
			m_vPeckDir.x -= vIntoGround.x;
			m_vPeckDir.y -= vIntoGround.y;
			m_vPeckDir.z -= vIntoGround.z;
			m_vPeckDir.Normalise();

			PlayPeckAnim( PECKANIM_IMPACTTOTUMBLE );

			return HIT_BOUNCE;
		}

		// Out of bounds: crash. NOTE: retail duplicates the crash code below here
	}

	// Crashed into the terrain
	m_eState = STATE_SPLASHED;

	if ( bCrash )
		m_fTimer = 0.0f;

	PlayPeckAnim( PECKANIM_IMPACTWOBBLE );

	StopEffect( m_pFlightFeathersL );
	StopEffect( m_pFlightFeathersR );

	StopSounds( TFALSE );
	ASoundManager::GetSingleton()->PlayCueEx( soundbank::LAUNCH_RECORDSCRATCH, 0.5f, TFALSE, 0.0f, -1 );

	vStep.Multiply( m_vPeckDir, fDistance );
	vSphere.Add( m_vPeckPos, vStep );
	vSphere.w = 0.5f;

	TVector4 vFacing = GetSphereNormal( vSphere, m_vPeckDir );

	// Don't face straight up
	if ( TVector4::DotProduct3( vFacing, TVector4::VEC_NEGY ) >= 0.9f )
		vFacing.AsVector3().RotateX( 0.017453292f );

	vFacing.Normalise();

	AParticleEffectManager::GetSingleton()->PlayEffect( m_iFeatherExplodeEffect, m_vPeckPos, vFacing );
	AParticleEffectManager::GetSingleton()->PlayEffect( m_iDustPuffEffect, m_vPeckPos, vFacing );

	vFacing.x = -vFacing.x;
	vFacing.y = -vFacing.y;
	vFacing.z = -vFacing.z;

	TVector4 vPushBack;
	vPushBack.Multiply( vFacing, 0.2f );

	TVector4 vCrashStep;
	vCrashStep.Multiply( m_vPeckDir, fDistance );
	vCrashStep.Multiply( 0.75f );

	TVector4 vPosition;
	vPosition.Add( m_vPeckPos, vCrashStep );
	vPosition.Substract( vPushBack );
	m_vPeckPos = vPosition;

	if ( bCrash )
		TSTATICCAST( AAnimatedUnit, GetPeckUnit() )->GetModelInstance()->SetTransformUnusedUp( vPosition, vFacing, TVector4::VEC_NEGY );
	else
		SetPeckTransform( m_vPeckPos, vFacing );

	if ( !ASoundManager::GetSingleton()->IsCueUsed( m_iCue_C58 ) )
		m_iCue_C58 = ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::LAUNCH_PECK_IMPACT, m_vPeckPos );

	ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::LAUNCH_PECK_FAIL, m_vPeckPos );

	Rumble();
	return HIT_BOUNCE;
}

// $Barnyard: FUNCTION 0047d8c0
void AChickenFireMinigameState::UpdateFlight( TFLOAT a_fDeltaTime )
{
	AAnimatableObject* pPeck = TSTATICCAST( AAnimatedUnit, GetPeckUnit() )->GetAnimatableObject();

	TMatrix44 matPeck;
	pPeck->GetModelInstance()->GetLocalTransform( matPeck );

	TINT        iPlayer      = APlayerManager::GetSingleton()->GetCurrentPlayerIterator();
	TFLOAT      fSteerX      = m_aInputs[ iPlayer ].fAimX;
	TFLOAT      fSteerY      = m_aInputs[ iPlayer ].fAimY;
	TQuaternion quatRotation = TQuaternion::IDENTITY;

	// Bank
	if ( TMath::Abs( fSteerX ) > 0.0f && m_eState == STATE_FLYING )
	{
		m_fBankAngle = TMath::LERP( m_fBankAngle, fSteerX, a_fDeltaTime * 3.0f );
		TMath::Clip( m_fBankAngle, -TMath::HALF_PI, TMath::HALF_PI );
	}
	else
	{
		m_fBankAngle = TMath::LERP( m_fBankAngle, 0.0f, a_fDeltaTime * 3.0f );
	}

	// Yaw, once banked in the steering direction
	if ( ( fSteerX < 0.0f && m_fBankAngle < 0.0f ) || ( fSteerX > 0.0f && m_fBankAngle > 0.0f ) )
	{
		TFLOAT fYaw = fSteerX * a_fDeltaTime * -1.0f;

		if ( m_eState == STATE_TUMBLE )
			m_fYawRate = TMath::LERP( m_fYawRate, fYaw * 0.75f, a_fDeltaTime + a_fDeltaTime );
		else
			m_fYawRate = TMath::LERP( m_fYawRate, fYaw, a_fDeltaTime * 20.0f );

		TMath::Clip( m_fYawRate, -0.017453292f, 0.017453292f );
	}
	else
	{
		m_fYawRate = TMath::LERP( m_fYawRate, 0.0f, a_fDeltaTime + a_fDeltaTime );
	}

	TQuaternion quatYaw = TQuaternion::IDENTITY;
	quatYaw.RotateAroundAxis( TVector4::VEC_NEGY.AsVector3(), m_fYawRate );
	quatRotation *= quatYaw;

	// Pitch
	if ( TMath::Abs( fSteerY ) <= 0.0f )
	{
		m_fPitchRate = TMath::LERP( m_fPitchRate, 0.0f, a_fDeltaTime + a_fDeltaTime );
	}
	else
	{
		TFLOAT fUp = TVector4::DotProduct3( m_vPeckDir, TVector4::VEC_NEGY );

		if ( ( fSteerY > 0.0f && fUp > -0.75f ) || ( fSteerY < 0.0f && fUp < 0.75f ) )
		{
			TFLOAT      fPitch    = fSteerY * a_fDeltaTime;
			TQuaternion quatPitch = TQuaternion::IDENTITY;

			if ( m_eState == STATE_TUMBLE )
				m_fPitchRate = TMath::LERP( m_fPitchRate, fPitch * 0.1f, a_fDeltaTime + a_fDeltaTime );
			else
				m_fPitchRate = TMath::LERP( m_fPitchRate, fPitch, a_fDeltaTime * 20.0f );

			TMath::Clip( m_fPitchRate, -0.034906585f, 0.034906585f );

			TVector4 vAxis( matPeck.m_f11, 0.0f, matPeck.m_f13, matPeck.m_f14 );
			vAxis.Normalise();

			quatPitch.RotateAroundAxis( vAxis.AsVector3(), m_fPitchRate );
			quatRotation *= quatPitch;
		}
	}

	RotateVector( m_vPeckDir, quatRotation, m_vPeckDir );

	// Climbing or diving changes the speed
	TVector4 vSide( 0.0f, m_vPeckDir.y, m_vPeckDir.z, m_vPeckDir.w );
	vSide.Normalise();

	TFLOAT fForward = TMath::Abs( TVector4::DotProduct3( vSide, TVector4::VEC_NEGZ ) );

	if ( fForward < 0.99f )
	{
		TVector4 vCross;
		vCross.CrossProduct( vSide, TVector4::VEC_NEGZ );
		vCross.Normalise();

		TFLOAT fChange = 1.0f - fForward;
		fChange        = ( vCross.x <= 0.01f ) ? fChange + fChange : fChange * -1.0f;

		m_fPower += fChange * a_fDeltaTime + fChange * a_fDeltaTime;
	}

	if ( m_bBoosted == TTRUE )
		m_fPower += m_fPeckBoostAccel * m_fPeckBoostAccel * a_fDeltaTime;
	else
		m_fPower = TMath::LERP( m_fPower, 0.0f, a_fDeltaTime * 0.2f );

	if ( m_fPower >= 1.5f && m_bBoosted == TTRUE )
	{
		m_bBoosted = TFALSE;
		StopEffect( m_pFlightFeathersL );
		StopEffect( m_pFlightFeathersR );
	}

	TMath::Clip( m_fPower, MIN_SPEED, MAX_SPEED );

	TFLOAT fSpeed = m_fPower * 30.0f;
	TMath::Clip( fSpeed, MIN_FLIGHT_SPEED, MAX_FLIGHT_SPEED );

	m_fFlightStep  = fSpeed * a_fDeltaTime;
	m_vPeckPrevPos = m_vPeckPos;

	TVector4 vStep;
	vStep.Multiply( m_vPeckDir, m_fFlightStep );
	m_vPeckPos.x += vStep.x;
	m_vPeckPos.y += vStep.y;
	m_vPeckPos.z += vStep.z;

	if ( m_eState == STATE_FLYING )
	{
		TBOOL bComputeBlend = TTRUE;

		if ( m_bBoosted == TTRUE && m_fPower < 0.9f )
		{
			PlayPeckAnim( PECKANIM_FLIGHTFLAP2 );

			if ( pPeck->IsAnimPlaying( TPS8( flightFlap2 ), 0.0f ) == TTRUE )
				pPeck->SetAnimationSpeed( TPS8( flightFlap2 ), 2.0f );
		}
		else if ( fSpeed - 30.0f >= 4.0f )
		{
			PlayPeckAnim( PECKANIM_FLIGHTDIVE );
		}
		else
		{
			if ( fSpeed - 30.0f > -2.0f )
				PlayPeckAnim( PECKANIM_FLIGHTLOOP );
			else
				PlayPeckAnim( PECKANIM_FLIGHTFLAP );

			bComputeBlend = MIN_FLIGHT_SPEED + 1.0f < fSpeed;
		}

		if ( bComputeBlend )
		{
			// NOTE: retail computes this and never uses it
			TFLOAT fBlend = ( fSpeed - MIN_FLIGHT_SPEED ) / MAX_FLIGHT_SPEED;
			TMath::Clip( fBlend, 0.0f, 1.0f );
		}
	}

	static TFLOAT s_fGlideHeight = MAX_HEIGHT + 5.0f;

	if ( m_eState == STATE_FLYING )
	{
		if ( fSteerY > 0.0f || m_vPeckPos.y > s_fGlideHeight || m_vPeckPos.y < MAX_HEIGHT )
		{
			if ( m_fPower <= STALL_SPEED )
			{
				// Too slow: level out
				TFLOAT fLevel = a_fDeltaTime + a_fDeltaTime;
				m_vPeckDir.y  = TMath::LERP( m_vPeckDir.y, 0.0f, fLevel );
				m_vPeckDir.Normalise();
				m_fPitchRate = TMath::LERP( m_fPitchRate, 0.0f, fLevel );
			}
		}
		else
		{
			// Close to the ceiling: level out
			TFLOAT fLevel = TMath::Abs( m_vPeckPos.y - s_fGlideHeight ) / TMath::Abs( MAX_HEIGHT - s_fGlideHeight );
			TMath::Clip( fLevel, 0.0f, 1.0f );

			TVector4 vFlat( m_vPeckDir.x, 0.0f, m_vPeckDir.z, m_vPeckDir.w );
			vFlat.Normalise();

			m_vPeckDir.Lerp4( m_vPeckDir, vFlat, fLevel );
			m_vPeckDir.Normalise();
			m_fPitchRate = TMath::LERP( m_fPitchRate, 0.0f, fLevel );
		}

		if ( m_vPeckPos.y < MAX_HEIGHT )
		{
			m_vPeckPos.y                                                                  = MAX_HEIGHT;
			m_fPitchRate                                                                  = 0.0f;
			m_aInputs[ APlayerManager::GetSingleton()->GetCurrentPlayerIterator() ].fAimY = 0.0f;
			m_vPeckDir.y                                                                  = 0.0f;
			m_vPeckDir.Normalise();
		}
	}

	SetPeckFlightTransform( m_vPeckPos, m_vPeckDir );
}

// $Barnyard: FUNCTION 0047e270
void AChickenFireMinigameState::CreateScorecards()
{
	const TPString8* apScorecards[ NUM_SCORECARDS ] = {
		&TPS8( ch_scorecard_01 ),
		&TPS8( ch_scorecard_02 ),
		&TPS8( ch_scorecard_03 ),
		&TPS8( ch_scorecard_04 ),
		&TPS8( ch_scorecard_05 ),
		&TPS8( ch_scorecard_06 ),
		&TPS8( ch_scorecard_07 ),
		&TPS8( ch_scorecard_08 ),
		&TPS8( ch_scorecard_09 ),
		&TPS8( ch_scorecard_10 ),
	};

	AAnimatableObjectManager* pManager = AAnimatableObjectManager::GetSingleton();

	for ( TINT i = 0; i < NUM_SCORECARDS; i++ )
	{
		AAnimatableObjectType* pType   = pManager->FindType( *apScorecards[ i ] );
		AAnimatableObject*     pObject = pManager->CreateAnimatableObject( pType, TNULL, 0 );

		pObject->GetModelInstance()->SetTransform( TVector4::VEC_ZERO, TVector4::VEC_POSZ );
		pObject->SetSkeletonUpdating( TFALSE, TTRUE );
		pObject->SetVisible( TFALSE );

		m_vecObjects4.PushBack( pObject );
	}
}

// $Barnyard: FUNCTION 0047e380
void AChickenFireMinigameState::ShowScorecard( TINT a_iIndex, TVector4 a_vUnused )
{
	for ( TINT i = 0; i < NUM_SCORECARDS; i++ )
	{
		AAnimatableObject* pScorecard = m_vecObjects4[ i ];

		pScorecard->GetModelInstance()->SetTransform( TVector4::VEC_ZERO, TVector4::VEC_POSX );
		pScorecard->SetSkeletonUpdating( TFALSE, TTRUE );
		pScorecard->SetVisible( TFALSE );
	}

	if ( a_iIndex != -1 )
	{
		// Freddy holds up the scorecard
		TINT iBone = m_pFreddy->GetModelInstance()->GetBoneID( TPS8( cb_llocator ) );

		TMatrix44 matBone;
		m_pFreddy->GetModelInstance()->GetBoneTransform( iBone, matBone );

		TVector4 vPosition = matBone.GetTranslation();
		TVector4 vDirection( -matBone.m_f21, -matBone.m_f22, -matBone.m_f23, 1.0f );

		// NOTE: retail rotates by 5156.62 radians, the 90 degrees converted the wrong way (* 180 / PI)
		vDirection.AsVector3().RotateY( 5156.6201f );

		AAnimatableObject* pScorecard = m_vecObjects4[ a_iIndex ];
		pScorecard->SetSkeletonUpdating( TTRUE, TTRUE );
		pScorecard->GetModelInstance()->SetTransform( vPosition, vDirection );
		pScorecard->SetSkeletonUpdating( TTRUE, TTRUE );
		pScorecard->SetVisible( TTRUE );
	}
}

// $Barnyard: FUNCTION 0047e5e0
void AChickenFireMinigameState::UpdateRings( TFLOAT a_fDeltaTime )
{
	if ( m_vecObjects1.Size() <= m_iNextRing )
		return;

	// Fade out the rings Peck has passed
	for ( TINT i = 0; i < m_iNextRing; i++ )
	{
		TFLOAT fOpacity = m_vecObjects1[ i ]->GetModelInstance()->GetOpacity();
		fOpacity        = TMath::LERP( fOpacity, 0.0f, a_fDeltaTime * 5.0f );
		TMath::Clip( fOpacity, 0.0f, 1.0f );

		m_vecObjects1[ i ]->GetModelInstance()->SetOpacity( fOpacity );
		m_vecObjects2[ i ]->GetModelInstance()->SetOpacity( fOpacity );
	}

	TVector4 vRingPos;
	m_vecObjects1[ m_iNextRing ]->GetModelInstance()->GetPosition( vRingPos );

	// Peck flies along -Z
	if ( m_vPeckPos.z < vRingPos.z )
	{
		m_iNextRing++;

		if ( TVector4::Distance( vRingPos, m_vPeckPos ) <= PICKUP_DISTANCE )
		{
			// Flew through: one more boost
			m_iNumBoosts = TMath::Min( m_iNumBoosts + 1, MAX_BOOSTS );
			m_oFeathers.SetNumFeathers( m_iNumBoosts );

			m_iScore += RING_SCORE;
			m_iRingScore += RING_SCORE;
			m_oPortrait.SetScore( m_iScore );

			ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::LAUNCH_WOOSH, vRingPos );
			ASoundManager::GetSingleton()->PlayCueAtPositionEx( soundbank::LAUNCH_RING_TING, vRingPos, 0.5f, TFALSE, 0.0f, -1 );
			m_fPower = 1.0f;
		}
	}

	// Spin the rings ahead
	for ( TINT i = m_vecObjects1.Size() - 1; i >= m_iNextRing; i-- )
	{
		TMatrix44 matTransform;

		AAnimatableObject* pOuter = m_vecObjects1[ i ];
		pOuter->GetModelInstance()->GetLocalTransform( matTransform );
		matTransform.RotateY( a_fDeltaTime * 0.3f );
		pOuter->GetModelInstance()->SetTransform( matTransform );

		AAnimatableObject* pInner = m_vecObjects2[ i ];
		pInner->GetModelInstance()->GetLocalTransform( matTransform );
		matTransform.RotateY( a_fDeltaTime * -0.3f );
		pInner->GetModelInstance()->SetTransform( matTransform );
	}
}

// $Barnyard: FUNCTION 0047eaa0
void AChickenFireMinigameState::UpdateBarnbucks( TFLOAT a_fDeltaTime )
{
	if ( m_vecObjects3.Size() <= m_iNextCoin )
		return;

	TVector4 vCoinPos;
	m_vecObjects3[ m_iNextCoin ]->GetModelInstance()->GetPosition( vCoinPos );

	if ( m_vPeckPos.z < vCoinPos.z )
	{
		if ( TVector4::Distance( vCoinPos, m_vPeckPos ) <= PICKUP_DISTANCE )
		{
			m_iScore++;
			m_oPortrait.SetScore( m_iScore );
			StartCollectEffect( m_vecObjects3[ m_iNextCoin ], 1, 20.0f );
		}

		m_iNextCoin++;
		m_iCoinsPassed++;
	}

	// Spin the barnbucks ahead
	for ( TINT i = m_vecObjects3.Size() - 1; i >= m_iNextCoin; i-- )
	{
		AAnimatableObject* pCoin = m_vecObjects3[ i ];

		TMatrix44 matTransform;
		pCoin->GetModelInstance()->GetLocalTransform( matTransform );
		matTransform.RotateZ( a_fDeltaTime * 1.5f );
		pCoin->GetModelInstance()->SetTransform( matTransform );
	}
}

// $Barnyard: FUNCTION 0047ecb0
void AChickenFireMinigameState::OnLandedOnBoard( TFLOAT a_fDistance )
{
	StopSounds( TFALSE );

	TVector4    vFreddyPos;
	TQuaternion quatFreddyRot;
	ATerrainInterface::GetSingleton()->GetLocatorPosAndRot( "CFP_freddy_position", vFreddyPos, quatFreddyRot );

	// NOTE: retail hands the locator rotation to SetTransform (00610b90) as the direction
	m_pFreddy->GetModelInstance()->SetTransform( vFreddyPos, *TREINTERPRETCAST( TVector4*, &quatFreddyRot ) );
	m_pFreddy->SetSkeletonUpdating( TTRUE, TTRUE );

	TVector4 vToTarget( m_vTargetPoint.x - m_vPeckPos.x, m_vTargetPoint.y - m_vPeckPos.y, m_vTargetPoint.z - m_vPeckPos.z, m_vTargetPoint.w );
	TFLOAT   fTargetDistance = vToTarget.Magnitude();

	StopEffect( m_pFlightFeathersL );
	StopEffect( m_pFlightFeathersR );

	m_eState = STATE_CRASH;

	// 9 at m_fTargetDist1 from the target, 0 at m_fTargetDist1 + m_fTargetDist2
	m_iLandingScore = TINT( ( 1.0f - ( fTargetDistance - m_fTargetDist1 ) / m_fTargetDist2 ) * 9.0f );

	if ( m_iLandingScore < 0 )
		m_iLandingScore = 0;

	TMatrix44 matTarget;
	ATerrainInterface::GetSingleton()->GetLocatorTransform( "target_point00", matTarget );
	TVector4 vDirection( -matTarget.m_f21, -matTarget.m_f22, -matTarget.m_f23, 1.0f );

	TVector4 vStep;
	vStep.Multiply( m_vPeckDir, a_fDistance );

	m_vPeckPos.Set( vStep.x + m_vPeckPrevPos.x, vStep.y + m_vPeckPrevPos.y, vStep.z + m_vPeckPrevPos.z, m_vPeckPrevPos.w );

	TSTATICCAST( AAnimatedUnit, GetPeckUnit() )->GetModelInstance()->SetTransformUnusedUp( m_vPeckPos, vDirection, TVector4::VEC_NEGY );
	ASoundManager::GetSingleton()->PlayCueEx( soundbank::LAUNCH_RECORDSCRATCH, 0.5f, TFALSE, 0.0f, -1 );
}

// $Barnyard: FUNCTION 0047efb0
void AChickenFireMinigameState::OnLandedOnBarn( TFLOAT a_fDistance )
{
	// NOTE: identical to OnLandedOnBoard in retail apart from m_bWon
	OnLandedOnBoard( a_fDistance );
	m_bWon = TTRUE;
}

// $Barnyard: FUNCTION 0047f2c0
void AChickenFireMinigameState::OnLandedInRing( TINT a_iHit )
{
	StopSounds( TFALSE );

	TVector4    vFreddyPos;
	TQuaternion quatFreddyRot;
	ATerrainInterface::GetSingleton()->GetLocatorPosAndRot( "CFP_freddy_position", vFreddyPos, quatFreddyRot );

	// NOTE: retail hands the locator rotation to SetTransform (00610b90) as the direction
	m_pFreddy->GetModelInstance()->SetTransform( vFreddyPos, *TREINTERPRETCAST( TVector4*, &quatFreddyRot ) );
	m_pFreddy->SetSkeletonUpdating( TTRUE, TTRUE );

	StopEffect( m_pFlightFeathersL );
	StopEffect( m_pFlightFeathersR );

	m_eState        = STATE_END_CAMERA;
	m_bWon          = TTRUE;
	m_iLandingScore = a_iHit - HIT_SUCCESS;

	if ( m_iLandingScore < 5 )
	{
		ASoundManager::GetSingleton()->PlayCue( soundbank::LAUNCH_CLAP );
		m_pFreddy->PlayAnimation( TPS8( score_bad ) );
	}
	else
	{
		ASoundManager::GetSingleton()->PlayCue( soundbank::LAUNCH_CROWD );
		m_pFreddy->PlayAnimation( TPS8( score_good ) );
		ASoundManager::GetSingleton()->PlayCue( soundbank::LAUNCH_FREDDY_LAUNCH1 );
	}

	TINT iTimeBonus = TINT( TIME_BONUS_SECONDS - m_fFlightTime );

	if ( iTimeBonus > 0 )
		m_iTimeBonus = iTimeBonus;

	// Peck sits in the middle of the target
	TVector4 vOffset;
	vOffset.Multiply( TVector4::VEC_POSY, -1.0f );

	TVector4 vPosition( m_vTargetPoint.x - vOffset.x, m_vTargetPoint.y - vOffset.y, m_vTargetPoint.z - vOffset.z, m_vTargetPoint.w );
	TSTATICCAST( AAnimatedUnit, GetPeckUnit() )->GetModelInstance()->SetTransformUnusedUp( vPosition, TVector4::VEC_POSZ, TVector4::VEC_NEGY );

	PlayPeckAnim( PECKANIM_POSTIMPACTSUCCESS );
	m_fPower = 0.0f;
}

// $Barnyard: FUNCTION 0047f600
void AChickenFireMinigameState::CreateRings()
{
	AAnimatableObjectManager* pManager = AAnimatableObjectManager::GetSingleton();

	while ( m_vecObjects1.Size() != 0 )
	{
		AAnimatableObject* pObject = m_vecObjects1[ m_vecObjects1.Size() - 1 ];
		m_vecObjects1.PopBack();
		pManager->DestroyAnimatableObject( pObject );
	}

	while ( m_vecObjects2.Size() != 0 )
	{
		AAnimatableObject* pObject = m_vecObjects2[ m_vecObjects2.Size() - 1 ];
		m_vecObjects2.PopBack();
		pManager->DestroyAnimatableObject( pObject );
	}

	AAnimatableObjectType* pType = pManager->FindType( TPS8( peckringlarge ) );

	TString8 strName;
	TString8 strPrefix;
	strPrefix.Format( "%s%d", "ring_", ( m_iVariant == 4 ) ? 2 : m_iVariant );

	ATerrainLocatorList* pLocatorList = ATerrainInterface::GetSingleton()->GetLocatorList();
	TINT                 iNumLocators = pLocatorList->GetNumLocators();

	TMatrix44 matWorld;
	pLocatorList->GetWorldTransform().GetLocalMatrixImp( matWorld );

	for ( TINT i = 0; i < iNumLocators; i++ )
	{
		strName = pLocatorList->GetLocatorName( i );

		if ( strName.Find( strPrefix.GetString() ) == -1 )
			continue;

		AAnimatableObject* pOuter = pManager->CreateAnimatableObject( pType, TNULL, 0 );
		AAnimatableObject* pInner = pManager->CreateAnimatableObject( pType, TNULL, 0 );

		TMatrix44 matLocator;
		pLocatorList->FindLocator( strName )->GetMatrix( matLocator );

		TMatrix44 matTransform;
		matTransform.Multiply( matWorld, matLocator );
		matTransform.RotateZ( TMath::PI );

		pOuter->GetModelInstance()->SetTransform( matTransform );
		pOuter->SetSkeletonUpdating( TTRUE, TTRUE );
		pOuter->SetVisible( TTRUE );

		pInner->GetModelInstance()->SetTransform( matTransform );
		pInner->SetSkeletonUpdating( TTRUE, TTRUE );
		pInner->SetVisible( TTRUE );

		// NOTE: retail loops over the meshes of every LOD of pOuter's model here without doing anything

		m_vecObjects1.Push( pOuter );
		m_vecObjects2.Push( pInner );
	}

#ifdef TRENDERINTERFACE_DX8
	// The rings are drawn in the 4th order table of the world shader
	TModel*          pModel  = pType->GetModel()->GetManagedModel()->GetModel();
	AWorldShaderHAL* pShader = TSTATICCAST( AWorldShaderHAL, AWorldShader::GetSingleton() );

	for ( TINT i = 0; i < pModel->GetNumLODs(); i++ )
	{
		TModelLOD& rLOD = pModel->GetLOD( i );

		for ( TINT k = 0; k < rLOD.iNumMeshes; k++ )
			TSTATICCAST( AWorldMaterialHAL, rLOD.ppMeshes[ k ]->GetMaterial() )->SetOrderTable( pShader->GetOrderTable( 3 ) );
	}
#endif
}

// $Barnyard: FUNCTION 0047fa60
void AChickenFireMinigameState::CreateBarnbucks()
{
	AAniEffectManager::GetSingleton()->DeleteAllEffects();

	AAnimatableObjectManager* pManager = AAnimatableObjectManager::GetSingleton();

	while ( m_vecObjects3.Size() != 0 )
	{
		AAnimatableObject* pObject = m_vecObjects3[ m_vecObjects3.Size() - 1 ];
		m_vecObjects3.PopBack();
		pManager->DestroyAnimatableObject( pObject );
	}

	AAnimatableObjectType* pType = pManager->FindType( TPS8( barnbuck ) );

	TString8 strName;
	TString8 strPrefix( "coin_" );

	ATerrainLocatorList* pLocatorList = ATerrainInterface::GetSingleton()->GetLocatorList();
	TINT                 iNumLocators = pLocatorList->GetNumLocators();

	TMatrix44 matWorld;
	pLocatorList->GetWorldTransform().GetLocalMatrixImp( matWorld );

	for ( TINT i = 0; i < iNumLocators; i++ )
	{
		strName = pLocatorList->GetLocatorName( i );

		if ( strName.Find( strPrefix.GetString() ) == -1 )
			continue;

		AAnimatableObject* pCoin = pManager->CreateAnimatableObject( pType, TNULL, 0 );

		TMatrix44 matLocator;
		pLocatorList->FindLocator( strName )->GetMatrix( matLocator );

		TMatrix44 matTransform;
		matTransform.Multiply( matWorld, matLocator );

		TFLOAT fAngle = ( ARandom::GetSingleton()->m_oRandom.RandRaw() >> 1 ) * TMath::TWO_PI * 4.6566129e-10f;
		matTransform.RotateZ( fAngle );
		matTransform.Scale( 2.0f, 2.0f, 2.0f );

		pCoin->GetModelInstance()->SetTransform( matTransform );
		pCoin->SetSkeletonUpdating( TTRUE, TTRUE );
		pCoin->SetVisible( TTRUE );

		m_vecObjects3.Push( pCoin );
	}
}

// $Barnyard: FUNCTION 0047fd50
void AChickenFireMinigameState::ResetTurn( TBOOL a_bNextPlayer )
{
	TINT iPlayer     = APlayerManager::GetSingleton()->GetCurrentPlayerIterator();
	TINT iNextPlayer = iPlayer;

	TSTATICCAST( AAnimatedUnit, GetPeckUnit() )->SetVisible( TTRUE );

	if ( a_bNextPlayer == TTRUE )
	{
		PlayerInfo& rInfo = m_oResults.oPlayerList[ iPlayer ];

		if ( !rInfo.bUnk1 )
			rInfo.bUnk1 = TTRUE;

		rInfo.iUnk2 += m_iLandingScore + m_iScore + 1;
	}

	m_iNextRing     = 0;
	m_iNextCoin     = 0;
	m_iRingScore    = 0;
	m_iCoinsPassed  = 0;
	m_iScore        = 0;
	m_iLandingScore = -1;
	m_bUnk_C38      = TFALSE;

	// Peck back on the slingshot
	TSTATICCAST( AAnimatedUnit, GetPeckUnit() )->GetAnimatableObject()->KillAllAnimations();
	AttachPeckToPlayer();
	m_pPlayerModel->SetSkeletonUpdating( TTRUE, TTRUE );
	m_pPlayerModel->PlayAnimation( TPS8( idle_lp ) );

	m_vPeckDir   = TVector4::VEC_ZERO;
	m_fPitchRate = 0.0f;
	m_fYawRate   = 0.0f;
	m_fBankAngle = 0.0f;
	m_fPower     = 0.0f;
	m_fStateTime = 0.0f;

	m_aInputs[ iPlayer ].fAimX = TVector2::VEC_ZERO.x;
	m_aInputs[ iPlayer ].fAimY = TVector2::VEC_ZERO.y;

	// The start camera
	ACamera* pCamera                      = ACameraManager::GetSingleton()->GetCurrentCamera();
	TVector4 vCamPos                      = ATerrainInterface::GetSingleton()->GetLocatorPos( "start_cam_pos" );
	TVector4 vLookAt                      = ATerrainInterface::GetSingleton()->GetLocatorPos( "start_cam_lookat" );
	pCamera->GetMatrix().GetTranslation() = vCamPos;
	pCamera->LookAtPoint( vLookAt );
	pCamera->SetFOV( 1.0471976f );

	m_bEndCameraStarted = TFALSE;
	m_pFreddy->SetSkeletonUpdating( TFALSE, TTRUE );

	PlayPeckAnim( PECKANIM_PRELAUNCH );
	ShowScorecard( -1, TVector4::VEC_ZERO );

	m_fStallTime          = 0.0f;
	m_fUnk_1FD4           = 0.0f;
	m_fPlayerPullbackTime = 0.0f;
	m_fPeckPullbackTime   = 0.0f;

	ALocale::FormatInputTags( m_wszBashText, TARRAYSIZE( m_wszBashText ), ALocaleManager::Interface()->GetString( LOCALE_BASH_INSTRUCTIONS ) );
	m_oBashHud.SetText( m_wszBashText );

	m_bReleased = TFALSE;
	m_bBoosted  = TFALSE;
	StopSounds( TFALSE );

	m_fTimer     = 0.0f;
	m_iCountdown = 4;
	m_fTextTimer = TEXT_REFRESH_TIME;
	m_iNumBoosts = 1;
	m_oFeathers.SetNumFeathers( 1 );
	m_oTextBox.Hide();
	m_bOrbitCamera = TFALSE;

	KillEffect( m_pFlightFeathersL );
	KillEffect( m_pFlightFeathersR );
	AParticleEffectManager::GetSingleton()->KillAll();

	m_fWrongWayTime = 0.0f;

	for ( TINT i = 0; i < m_vecObjects1.Size(); i++ )
	{
		m_vecObjects1[ i ]->GetModelInstance()->SetOpacity( 1.0f );
		m_vecObjects2[ i ]->GetModelInstance()->SetOpacity( 1.0f );
	}

	for ( TINT i = 0; i < m_vecObjects3.Size(); i++ )
	{
		m_vecObjects3[ i ]->GetModelInstance()->SetOpacity( 1.0f );
		m_vecObjects3[ i ]->GetModelInstance()->SetSkeletonUpdating( TTRUE );
	}

	AGameTimeFXManager* pGameTimeFX = AGameTimeFXManager::GetSingleton();
	pGameTimeFX->SetForcedFX( pGameTimeFX->FindFX( "PeckRange" ) );

	m_HUDParams.SetFlags( AHudElement::MASK_MINIGAME_DEFAULT );

	// DOES_NOTHING (00628f40) is called on AInputHandler with 0

	m_pInputMap = AInputMapManager::GetSingleton()->FindMap( TPS8( AChickenFireMinigameState ) );
	m_eState    = STATE_WAIT_FOR_HUD;

	if ( m_iNumPlayers2 > 1 )
	{
		if ( a_bNextPlayer )
		{
			iNextPlayer = APlayerManager::GetSingleton()->GetNextPlayer();
		}
		else
		{
			APlayerManager::GetSingleton()->ResetPlayerIterator();
			iNextPlayer = 0;

			for ( TINT i = 0; i < m_iNumPlayers2; i++ )
			{
				if ( m_oResults.oPlayerList[ i ].bUnk1 )
					m_oResults.oPlayerList[ i ].iUnk2 = 0;
			}
		}
	}

	m_oStartHudElement.Setup( m_iNumPlayers2, iNextPlayer );
	m_bStartHudWasHidden = TFALSE;
	m_oBashHud.SetForceHide( TTRUE );
	m_oPortrait.Hide();
	m_oFeathers.SetVisible( TFALSE );
	m_oStartHudElement.SetForceHide( TFALSE );
	m_bWon = TFALSE;
	APositionalSoundSet::GetSingleton()->SetPaused( TFALSE );

	m_fFlightTime = 0.0f;
	m_iTimeBonus  = 0;

	if ( m_iVariant == 4 )
	{
		// Charity: the barnbucks
		CreateBarnbucks();
		SetCharityActorsSkeletonUpdating( TTRUE );

		if ( m_iVariant == 4 )
		{
			APlayerProfileManager::GetSingleton()->GetMainProfile()->SetMiniGamePickups( 0 );
			m_oPickUpHudElement.SetNumber( 0 );
		}
	}

	m_oPortrait.SetScore( 0 );
	AMusicManager::GetSingleton()->SetEnabled( TTRUE, TTRUE );
	m_bSubmitHighScore = TTRUE;
}
