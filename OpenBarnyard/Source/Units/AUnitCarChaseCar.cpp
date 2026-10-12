#include "pch.h"
#include <Render/TCameraObject.h>
#include "AUnitCarChaseCar.h"
#include "AUnitTransform.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AAnimatableObjectType.h"
#include "Animation/AModel.h"
#include "FX/AParticleEffectManager.h"
#include "FX/AParticleSystem.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/carchase.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"
#include "Input/AInputHandler.h"
#include "Render/AShadowManager.h"
#include "Render/AGlowViewport.h"
#include "UnitMovement/Steering/ARacingCarSteer.h"
#include "Physics/AVehicleSkidManager.h"
#include "Physics/APhysicsCar.h"
#include "Physics/AVehicleManager.h"

#include <Math/TMathInline.h>
#include <Math/TVector2.h>
#include <Render/TSkeleton.h>

#ifdef PI
#  undef PI
#endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 004f5cd0, CreateObject 004f7130, CreateObjectInPlace 004f7190
TDEFINE_CLASS( AUnitCarChaseCar );

TPSTRING8_DECLARE( carwheel_truck );
TPSTRING8_DECLARE( carwheel_cop );
TPSTRING8_DECLARE( carwheel_beady );
TPSTRING8_DECLARE( wheel01 );
TPSTRING8_DECLARE( wheel02 );
TPSTRING8_DECLARE( wheel03 );
TPSTRING8_DECLARE( wheel04 );
TPSTRING8_DECLARE( base_bone );
TPSTRING8_DECLARE( jr_jerseycar );
TPSTRING8_DECLARE( jr_americanfcar );
TPSTRING8_DECLARE( jr_americanmcar );
TPSTRING8_DECLARE( jr_otiscar );
TPSTRING8_DECLARE( car_igg );
TPSTRING8_DECLARE( car_american_f );
TPSTRING8_DECLARE( car_american_m );
TPSTRING8_DECLARE( idle );
TPSTRING8_DECLARE( idle1 );
TPSTRING8_DECLARE( carchasecar1 );
TPSTRING8_DECLARE( carchasecar3 );

// Sparks are placed in the local space of the car (byte_773cc4)
static TBOOL s_bLocalSparks = TTRUE;

// Rumble of the controller when the car hits something
static const AInputHandler::RumbleParams s_oCrashRumble = { { 0.8f, 0.0f, 0.0f, 0.4f, 0.0f, 1.0f, 1.0f }, 1 };

// Packs the colour of a tyre track
// NOTE: same as PackColour of ABikeSteer.cpp (004083c0)
static TUINT32 PackTyreTrackColour( TFLOAT a_fRed, TFLOAT a_fGreen, TFLOAT a_fBlue, TFLOAT a_fAlpha )
{
	return ( ( ( TUINT32( a_fGreen * 255.0f ) | ( ( TUINT32( a_fRed * 255.0f ) | ( TUINT32( a_fAlpha * 255.0f ) << 8 ) ) << 8 ) ) << 8 ) | TUINT32( a_fBlue * 255.0f ) );
}

// $Barnyard: FUNCTION 004f5d90
AUnitCarChaseCar::AUnitCarChaseCar()
{
	m_iEngineCue     = -1;
	m_iEngineLoopCue = -1;
	m_iUnkC0Cue      = -1;
	m_iIdleCue       = -1;
	m_iSuspensionCue = -1;
	m_iCrashCue      = -1;
	m_iUnkD0Cue      = -1;
	m_iSirenCue      = -1;
	m_iSkidCue       = -1;

	m_iIgnitionCue     = -1;
	m_iGearCue         = -1;
	m_eEngineState     = ENGINESTATE_OFF;
	m_iLastGear        = 1;
	m_iEngineLoopSound = -1;
	m_pGlowObject      = TNULL;
	m_iUnk188          = 4;
	m_bUnk18C          = TFALSE;
	m_fMaxSpeed        = 0.0f;
	m_iGravelCue       = -1;
	m_iUnk168          = -1;

	for ( TINT i = 0; i < NUM_WHEELS; i++ )
		m_apWheels[ i ] = TNULL;

	// NOTE: only the first bone is initialised in retail
	m_aiWheelBones[ 0 ] = -1;
	m_pDriver           = TNULL;
	m_pPassenger1       = TNULL;
	m_pPassenger2       = TNULL;
	m_iBaseBone         = -1;
	m_eCarType          = CARTYPE_BEADY;

	m_iHeadlightEffect = AParticleEffectManager::FindEffect( "car_headlight" );
	m_iTaillightEffect = AParticleEffectManager::FindEffect( "car_taillight" );
	m_iSirenEffect     = AParticleEffectManager::FindEffect( "car_copsiren" );

	for ( TINT i = 0; i < 2; i++ )
	{
		m_apHeadlights[ i ]    = TNULL;
		m_apTaillights[ i ]    = TNULL;
		m_apBrakeLights[ i ]   = TNULL;
		m_apSirens[ i ]        = TNULL;
		m_apReverseLights[ i ] = TNULL;

		m_aiHeadlightBones[ i ]    = -1;
		m_aiTaillightBones[ i ]    = -1;
		m_aiBrakeLightBones[ i ]   = -1;
		m_aiReverseLightBones[ i ] = -1;
	}

	for ( TINT i = 0; i < NUM_WHEELS; i++ )
	{
		m_avTyreTrackPositions[ i ] = TVector4::VEC_ZERO;
		m_afTyreTrackOffsets[ i ]   = 0.0f;
		m_afTyreTrackWidths[ i ]    = 0.3f;
		m_aiWheelSurfaces[ i ]      = -1;
		m_afDustTimers[ i ]         = 0.0f;
		m_afGrassTimers[ i ]        = 0.0f;
	}

	m_iDustEffect   = AParticleEffectManager::FindEffect( "dustpuffsmall" );
	m_iGrassEffect  = AParticleEffectManager::FindEffect( "grasswalk" );
	m_iSparksEffect = AParticleEffectManager::FindEffect( "carsparks" );
}

// $Barnyard: FUNCTION 004f71d0
// $Barnyard: FUNCTION 004f81c0
// $Barnyard: FUNCTION 004f6090
AUnitCarChaseCar::~AUnitCarChaseCar()
{
	if ( m_pGlowObject )
		AGlowViewport::GetSingleton()->RemoveGlowObject( m_pGlowObject );
}

// $Barnyard: FUNCTION 004f6160
ARacingCarSteer* AUnitCarChaseCar::GetRacingCarSteer()
{
	TASSERT( m_pSteer->IsA( &TGetClass( ARacingCarSteer ) ) );
	return TSTATICCAST( ARacingCarSteer, m_pSteer );
}

// $Barnyard: FUNCTION 004f5d10
void AUnitCarChaseCar::KillLights()
{
	for ( TINT i = 0; i < 2; i++ )
	{
		if ( m_apHeadlights[ i ] )
		{
			m_apHeadlights[ i ]->GetInstance()->Kill();
			m_apHeadlights[ i ] = TNULL;
		}

		if ( m_apTaillights[ i ] )
		{
			m_apTaillights[ i ]->GetInstance()->Kill();
			m_apTaillights[ i ] = TNULL;
		}

		if ( m_apBrakeLights[ i ] )
		{
			m_apBrakeLights[ i ]->GetInstance()->Kill();
			m_apBrakeLights[ i ] = TNULL;
		}

		if ( m_apReverseLights[ i ] )
		{
			m_apReverseLights[ i ]->GetInstance()->Kill();
			m_apReverseLights[ i ] = TNULL;
		}

		if ( m_apSirens[ i ] )
		{
			m_apSirens[ i ]->GetInstance()->Kill();
			m_apSirens[ i ] = TNULL;
		}
	}
}

// $Barnyard: FUNCTION 004f84e0
void AUnitCarChaseCar::CreateLights()
{
	APhysicsCar* pCar = GetRacingCarSteer()->GetCar();

	// The shadow
	TVector4 vSize;
	vSize.Multiply( pCar->m_oParams.vBoxExtents, 2.0f );

	AShadowManager::GetSingleton()->AddUnitShadow( this, TVector2( vSize.x, vSize.z ), -1, -0.125f, AShadowManager::FLAGS_NONUNIFORM_SCALE, AShadowManager::TYPE_CAR );
	AShadowManager::GetSingleton()->SetShadowScale( this, TVector2( vSize.x * 1.075f, vSize.z * 1.075f ) );

	TSkeletonInstance* pSkeletonInstance = GetAnimatableObject()->GetModelInstance()->GetSkeletonInstance();

	// The headlights
	if ( m_eCarType == CARTYPE_BEADY || m_eCarType == CARTYPE_COP )
		m_aiHeadlightBones[ 0 ] = pSkeletonInstance->GetBoneID( "headight_02" );
	else
		m_aiHeadlightBones[ 0 ] = pSkeletonInstance->GetBoneID( "headight_01" );

	m_aiHeadlightBones[ 1 ] = pSkeletonInstance->GetBoneID( "headight_04" );

	for ( TINT i = 0; i < 2; i++ )
	{
		if ( !m_apHeadlights[ i ] )
		{
			m_apHeadlights[ i ] = AParticleEffectManager::GetSingleton()->PlayEffect( m_iHeadlightEffect, TVector4::VEC_ZERO, TVector4::VEC_NEGZ );

			if ( m_apHeadlights[ i ] )
			{
				m_apHeadlights[ i ]->AttachToBone( GetAnimatableObject()->GetModelInstance(), m_aiHeadlightBones[ i ], TTRUE, TFALSE );
				m_apHeadlights[ i ]->m_eFlags |= AParticleEffectHandle::FLAGS_FACE_CAMERA;
			}
		}
	}

	// The tail lights and the brake lights share the bones
	TINT iBone               = pSkeletonInstance->GetBoneID( "rearlight_1" );
	m_aiTaillightBones[ 0 ]  = iBone;
	m_aiBrakeLightBones[ 0 ] = iBone;
	iBone                    = pSkeletonInstance->GetBoneID( "rearlight_04" );
	m_aiTaillightBones[ 1 ]  = iBone;
	m_aiBrakeLightBones[ 1 ] = iBone;

	if ( GetAnimatableObjectType()->GetName() == TPS8( carchasecar3 ) || GetAnimatableObjectType()->GetName() == TPS8( carchasecar1 ) )
	{
		m_aiReverseLightBones[ 0 ] = pSkeletonInstance->GetBoneID( "rearlight_02" );
		m_aiReverseLightBones[ 1 ] = pSkeletonInstance->GetBoneID( "rearlight_05" );
	}

	for ( TINT i = 0; i < 2; i++ )
	{
		if ( !m_apTaillights[ i ] )
		{
			// NOTE: retail doesn't check the handle here
			m_apTaillights[ i ]                                = AParticleEffectManager::GetSingleton()->PlayEffect( m_iTaillightEffect, TVector4::VEC_ZERO, TVector4::VEC_NEGZ );
			m_apTaillights[ i ]->GetInstance()->GetTransform() = TMatrix44::IDENTITY;
			m_apTaillights[ i ]->AttachToBone( GetAnimatableObject()->GetModelInstance(), m_aiTaillightBones[ i ], TTRUE, TFALSE );
			m_apTaillights[ i ]->m_eFlags |= AParticleEffectHandle::FLAGS_FACE_CAMERA;
		}
	}

	// The sirens of the cop car
	if ( m_eCarType == CARTYPE_COP )
	{
		TINT aiSirenBones[ 2 ];
		aiSirenBones[ 0 ] = pSkeletonInstance->GetBoneID( "siren_01" );
		aiSirenBones[ 1 ] = pSkeletonInstance->GetBoneID( "siren_02" );

		for ( TINT i = 0; i < 2; i++ )
		{
			if ( !m_apSirens[ i ] )
			{
				// NOTE: retail doesn't check the handle here
				m_apSirens[ i ] = AParticleEffectManager::GetSingleton()->PlayEffect( m_iSirenEffect, TVector4::VEC_ZERO, TVector4::VEC_NEGZ );
				m_apSirens[ i ]->AttachToBone( GetAnimatableObject()->GetModelInstance(), aiSirenBones[ i ], TTRUE, TFALSE );
				m_apSirens[ i ]->m_eFlags |= AParticleEffectHandle::FLAGS_FACE_CAMERA;
			}
		}
	}
}

// $Barnyard: FUNCTION 004f60a0
void AUnitCarChaseCar::OnDestroy()
{
	KillLights();

	AAnimatableObjectManager* pManager = AAnimatableObjectManager::GetSingleton();

	for ( TINT i = 0; i < NUM_WHEELS; i++ )
	{
		if ( m_apWheels[ i ] )
		{
			m_apWheels[ i ]->SetSkeletonUpdating( TFALSE, TTRUE );
			pManager->DestroyAnimatableObject( m_apWheels[ i ] );
			m_apWheels[ i ] = TNULL;
		}
	}

	if ( m_pDriver )
	{
		m_pDriver->SetSkeletonUpdating( TFALSE, TTRUE );
		pManager->DestroyAnimatableObject( m_pDriver );
		m_pDriver = TNULL;
	}

	if ( m_pPassenger1 )
	{
		pManager->DestroyAnimatableObject( m_pPassenger1 );
		m_pPassenger1 = TNULL;
	}

	if ( m_pPassenger2 )
	{
		pManager->DestroyAnimatableObject( m_pPassenger2 );
		m_pPassenger2 = TNULL;
	}

	AAnimatedUnit::OnDestroy();
}

// $Barnyard: FUNCTION 004f89a0
TBOOL AUnitCarChaseCar::Create( AUnitTransform* a_pTransform )
{
	m_eCarType = CARTYPE_( a_pTransform->m_iModelIndex );

	if ( !AAnimatedUnit::Create( a_pTransform ) )
		return TFALSE;

	m_eFlags |= FLAGS_AI_ENABLED;
	m_eFlags |= FLAGS_THINK_ENABLED;

	AAnimatableObjectManager* pManager = AAnimatableObjectManager::GetSingleton();

	// NOTE: the front and the rear wheels are the same type
	AAnimatableObjectType* pFrontWheelType;
	AAnimatableObjectType* pRearWheelType;

	if ( m_eCarType == CARTYPE_PLAYER )
	{
		pFrontWheelType = pManager->FindType( TPS8( carwheel_truck ) );
		pRearWheelType  = pManager->FindType( TPS8( carwheel_truck ) );
	}
	else if ( m_eCarType == CARTYPE_COP )
	{
		pFrontWheelType = pManager->FindType( TPS8( carwheel_cop ) );
		pRearWheelType  = pManager->FindType( TPS8( carwheel_cop ) );
	}
	else
	{
		pFrontWheelType = pManager->FindType( TPS8( carwheel_beady ) );
		pRearWheelType  = pManager->FindType( TPS8( carwheel_beady ) );
	}

	for ( TINT i = 0; i < NUM_WHEELS; i++ )
	{
		m_apWheels[ i ] = pManager->CreateAnimatableObject( ( i >= 2 ) ? pRearWheelType : pFrontWheelType, TNULL, 0 );

		if ( m_apWheels[ i ] )
			m_apWheels[ i ]->SetSkeletonUpdating( TTRUE, TTRUE );
	}

	TSkeletonInstance* pSkeletonInstance = GetAnimatableObject()->GetModelInstance()->GetSkeletonInstance();

	m_aiWheelBones[ 0 ] = pSkeletonInstance->GetBoneID( TPS8( wheel03 ).GetString() );
	m_aiWheelBones[ 1 ] = pSkeletonInstance->GetBoneID( TPS8( wheel04 ).GetString() );
	m_aiWheelBones[ 2 ] = pSkeletonInstance->GetBoneID( TPS8( wheel01 ).GetString() );
	m_aiWheelBones[ 3 ] = pSkeletonInstance->GetBoneID( TPS8( wheel02 ).GetString() );
	m_iBaseBone         = pSkeletonInstance->GetBoneID( TPS8( base_bone ).GetString() );

	m_fMaxSpeed = GetRacingCarSteer()->GetMaxSpeed();
	CreateLights();

	return TTRUE;
}

// $Barnyard: FUNCTION 004f88f0
void AUnitCarChaseCar::SetVisible( TBOOL a_bVisible )
{
	if ( m_pDriver )
		m_pDriver->SetSkeletonUpdating( a_bVisible, TTRUE );

	if ( m_pPassenger1 )
		m_pPassenger1->SetSkeletonUpdating( a_bVisible, TTRUE );

	if ( m_pPassenger2 )
		m_pPassenger2->SetSkeletonUpdating( a_bVisible, TTRUE );

	for ( TINT i = 0; i < NUM_WHEELS; i++ )
		m_apWheels[ i ]->SetSkeletonUpdating( a_bVisible, TTRUE );

	if ( a_bVisible )
		CreateLights();
	else
		KillLights();

	if ( m_pSteer->IsA( &TGetClass( ARacingCarSteer ) ) == TTRUE )
		TSTATICCAST( ARacingCarSteer, m_pSteer )->GetCar()->GetBody().m_bEnabled = a_bVisible;

	AAnimatedUnit::SetVisible( a_bVisible );
}

// NOTE: a thunk to AVisibleUnit::Think (004d7ae0) shared by several classes in retail
TBOOL AUnitCarChaseCar::Think( TFLOAT a_fDeltaTime )
{
	return AVisibleUnit::Think( a_fDeltaTime );
}

// $Barnyard: FUNCTION 004f6a30
void AUnitCarChaseCar::StartEngine()
{
	if ( m_eEngineState != ENGINESTATE_OFF )
		return;

	ASoundManager* pSoundManager = ASoundManager::GetSingleton();

	m_iIgnitionCue = pSoundManager->PlayCueAtPosition( ( m_eCarType != CARTYPE_BEADY ) ? soundbank::CARCHASE_IGNITION : soundbank::CARCHASE_B_IGNITION, *m_pSteer->GetPosition() );
	m_iIdleCue     = pSoundManager->PlayCueAtPositionEx( ( m_eCarType != CARTYPE_BEADY ) ? soundbank::CARCHASE_IDLE01 : soundbank::CARCHASE_B_IDLE, *m_pSteer->GetPosition(), 1.0f, TFALSE, 1.0f, -1 );
	m_eEngineState = ENGINESTATE_STARTING;

	// NOTE: an ARacingCarSteer turns the engine on in UpdateEngineSounds once the ignition cue is over
	if ( !m_pSteer->IsA( &TGetClass( ARacingCarSteer ) ) )
		TSTATICCAST( ACarSteer, m_pSteer )->SetEngineStarted( TTRUE );
}

// $Barnyard: FUNCTION 004f6b10
void AUnitCarChaseCar::UpdateGearSound()
{
	TINT iGear = TSTATICCAST( ARacingCarSteer, m_pSteer )->GetCar()->m_oGearBox.iGear;

	if ( iGear != m_iLastGear && iGear != 1 )
	{
		TBOOL           bShiftUp   = iGear > m_iLastGear;
		TBOOL           bShiftDown = iGear < m_iLastGear;
		ASoundId        iSound     = -1;
		const TVector4& rcPosition = *m_pSteer->GetPosition();

		if ( bShiftUp )
		{
			if ( iGear > 3 )
				iSound = ( m_eCarType != CARTYPE_BEADY ) ? soundbank::CARCHASE_ACCEL02 : soundbank::CARCHASE_B_ACCEL02;
			else if ( iGear == 3 )
				iSound = ( m_eCarType != CARTYPE_BEADY ) ? soundbank::CARCHASE_ACCEL01 : soundbank::CARCHASE_B_ACCEL01;
		}

		if ( bShiftDown )
		{
			if ( iGear > 4 )
				iSound = ( m_eCarType != CARTYPE_BEADY ) ? soundbank::CARCHASE_SHIFT_DOWN01 : soundbank::CARCHASE_B_SHIFT_DOWN01;
			else if ( iGear > 3 )
				iSound = ( m_eCarType != CARTYPE_BEADY ) ? soundbank::CARCHASE_SHIFT_DOWN02 : soundbank::CARCHASE_B_SHIFT_DOWN02;
			else
				iSound = ( m_eCarType != CARTYPE_BEADY ) ? soundbank::CARCHASE_SHIFT_DOWN03 : soundbank::CARCHASE_B_SHIFT_DOWN03;
		}

		if ( iSound != -1 )
			m_iGearCue = ASoundManager::GetSingleton()->PlayCueAtPosition( iSound, rcPosition );
	}

	m_iLastGear = iGear;
}

// $Barnyard: FUNCTION 004f6c10
void AUnitCarChaseCar::SetSirenOn( TBOOL a_bOn )
{
	if ( a_bOn == TTRUE )
	{
		if ( m_iSirenCue == -1 )
			m_iSirenCue = ASoundManager::GetSingleton()->PlayCueAtPositionEx( soundbank::CARCHASE_SIREN, *m_pSteer->GetPosition(), 0.0f, TFALSE, 0.0f, -1 );

		ASoundManager::GetSingleton()->FadeCue( m_iSirenCue, 1.0f, 1.0f, 3.0f );
	}
	else if ( a_bOn == TFALSE )
	{
		if ( m_iSirenCue != -1 )
			ASoundManager::GetSingleton()->FadeOutCue( m_iSirenCue, 1.0f, TTRUE, 3.0f );
	}
}

// $Barnyard: FUNCTION 004f6cb0
void AUnitCarChaseCar::UpdateSkidSound( TFLOAT a_fDeltaTime )
{
	static TFLOAT s_fRearSlipAngle  = TMath::DegToRad( 5.0f );
	static TFLOAT s_fFrontSlipAngle = TMath::DegToRad( 6.0f );

	APhysicsCar* pCar  = TSTATICCAST( ARacingCarSteer, m_pSteer )->GetCar();
	TBOOL        bSkid = TFALSE;

	for ( TINT i = 0; i < NUM_WHEELS; i++ )
	{
		APhysicsWheel& rWheel    = pCar->m_aWheels[ i ];
		TFLOAT         fMaxAngle = ( i > 1 ) ? s_fRearSlipAngle : s_fFrontSlipAngle;

		if ( rWheel.m_bOnGround && rWheel.m_vContactVelocity.Magnitude() > pCar->m_fLowSpeed && TMath::Abs( rWheel.m_fSlipAngle ) > fMaxAngle )
			bSkid = TTRUE;
	}

	if ( bSkid == TTRUE )
	{
		if ( m_iSkidCue == -1 )
			m_iSkidCue = ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::CARCHASE_SKID_SHORT, *m_pSteer->GetPosition() );
		else
			ASoundManager::GetSingleton()->SetCuePosition( m_iSkidCue, *m_pSteer->GetPosition(), 0.0f );
	}
	else if ( m_iSkidCue != -1 )
	{
		ASoundManager::GetSingleton()->StopCue( m_iSkidCue );
		m_iSkidCue = -1;
	}
}

// $Barnyard: FUNCTION 004f6de0
void AUnitCarChaseCar::CreateDrivers( TINT a_iVariant )
{
	AAnimatableObjectManager* pManager        = AAnimatableObjectManager::GetSingleton();
	APlayerProfileManager*    pProfiles       = APlayerProfileManager::GetSingleton();
	APlayerProfile::GENDER    eGender         = pProfiles->GetMainGender();
	AAnimatableObjectType*    pDriverType     = TNULL;
	AAnimatableObjectType*    pPassenger1Type = TNULL;
	AAnimatableObjectType*    pPassenger2Type = TNULL;

	if ( m_eCarType == CARTYPE_PLAYER )
	{
		pDriverType = pManager->FindType( ( eGender == APlayerProfile::GENDER_FEMALE ) ? TPS8( car_american_f ) : TPS8( car_american_m ) );
	}
	else if ( m_eCarType != CARTYPE_COP )
	{
		switch ( a_iVariant )
		{
			case 0:
			case 1:
			case 2:
			case 3:
				pDriverType = pManager->FindType( TPS8( jr_jerseycar ) );
				break;
			case 4:
			case 5:
			case 7:
				pDriverType     = pManager->FindType( ( eGender != APlayerProfile::GENDER_MALE ) ? TPS8( jr_americanfcar ) : TPS8( jr_americanmcar ) );
				pPassenger1Type = pManager->FindType( TPS8( jr_otiscar ) );
				pPassenger2Type = pManager->FindType( TPS8( jr_jerseycar ) );
				break;
			case 6:
				pDriverType = pManager->FindType( ( eGender != APlayerProfile::GENDER_MALE ) ? TPS8( jr_americanfcar ) : TPS8( jr_americanmcar ) );
				break;
			default:
				pDriverType = pManager->FindType( TPS8( car_igg ) );
				break;
		}
	}

	if ( m_eCarType != CARTYPE_COP )
	{
		if ( pDriverType )
		{
			m_pDriver = pManager->CreateAnimatableObject( pDriverType, TNULL, 0 );
			m_pDriver->SetSkeletonUpdating( TTRUE, TTRUE );

			// The player's costume
			TINT iSkin = -1;

			if ( pDriverType->GetNumSkins() > 1 )
			{
				iSkin = pProfiles->GetMainCostume();
				TMath::Clip( iSkin, 0, pDriverType->GetNumSkins() - 1 );
			}

			if ( iSkin != -1 )
				m_pDriver->ApplyMaterial( iSkin, TTRUE );

			m_pDriver->PlayAnimation( TPS8( idle ) );
		}

		if ( pPassenger1Type )
		{
			m_pPassenger1 = pManager->CreateAnimatableObject( pPassenger1Type, TNULL, 0 );
			m_pPassenger1->PlayAnimation( TPS8( idle ) );
			m_pPassenger1->SetSkeletonUpdating( TTRUE, TTRUE );
		}
	}

	if ( pPassenger2Type )
	{
		m_pPassenger2 = pManager->CreateAnimatableObject( pPassenger2Type, TNULL, 0 );
		m_pPassenger2->PlayAnimation( TPS8( idle1 ) );
		m_pPassenger2->SetSkeletonUpdating( TTRUE, TTRUE );
	}
}

// $Barnyard: FUNCTION 004f6fb0
void AUnitCarChaseCar::StopSounds()
{
	ASoundManager* pSoundManager = ASoundManager::GetSingleton();

	pSoundManager->StopCue( m_iEngineCue );
	m_iEngineCue = -1;
	pSoundManager->StopCue( m_iEngineLoopCue );
	m_iEngineLoopCue = -1;
	pSoundManager->StopCue( m_iUnkC0Cue );
	m_iUnkC0Cue = -1;
	pSoundManager->StopCue( m_iIdleCue );
	m_iIdleCue = -1;
	pSoundManager->StopCue( m_iUnkD0Cue );
	m_iUnkD0Cue = -1;
	pSoundManager->StopCue( m_iSuspensionCue );
	m_iSuspensionCue = -1;
	pSoundManager->StopCue( m_iCrashCue );
	m_iCrashCue = -1;
	pSoundManager->StopCue( m_iGravelCue );
	m_iGravelCue = -1;
}

// $Barnyard: FUNCTION 004f7080
void AUnitCarChaseCar::PauseSounds( TBOOL a_bPause )
{
	ASoundManager* pSoundManager = ASoundManager::GetSingleton();

	pSoundManager->PauseCue( m_iEngineCue, a_bPause );
	pSoundManager->PauseCue( m_iEngineLoopCue, a_bPause );
	pSoundManager->PauseCue( m_iUnkC0Cue, a_bPause );
	pSoundManager->PauseCue( m_iIdleCue, a_bPause );
	pSoundManager->PauseCue( m_iUnkD0Cue, a_bPause );
	pSoundManager->PauseCue( m_iSuspensionCue, a_bPause );
	pSoundManager->PauseCue( m_iCrashCue, a_bPause );
	pSoundManager->PauseCue( m_iGravelCue, a_bPause );
}

// $Barnyard: FUNCTION 004f7240
void AUnitCarChaseCar::Reset()
{
	m_iFinishPosition = 0;
	m_iUnkA8          = 5;
	m_iUnkAC          = 5;
	m_iUnkB0          = 0;
	m_iState          = STATE_NONE;
	m_iDriveState     = DRIVESTATE_IDLE;
	m_iUnk15C         = 0;
	m_fSteering       = 0.0f;
	m_iUnk160         = 0;
	m_bWheelEffects   = TTRUE;
	m_bUnk16D         = TFALSE;
	m_iUnk14C         = 0;
	m_iUnk150         = 0;
	m_fEnginePitch    = 0.0f;

	StopSounds();

	ASoundManager* pSoundManager = ASoundManager::GetSingleton();
	pSoundManager->StopCue( m_iIgnitionCue );
	m_iIgnitionCue = -1;
	pSoundManager->StopCue( m_iGearCue );
	m_iGearCue = -1;
	pSoundManager->StopCue( m_iSirenCue );
	m_iSirenCue = -1;

	if ( m_pSteer )
		GetRacingCarSteer()->SetEngineStarted( TFALSE );

	m_eEngineState                                = ENGINESTATE_OFF;
	GetRacingCarSteer()->GetCar()->m_fMaxThrottle = 1.0f;

	for ( TINT i = 0; i < NUM_WHEELS; i++ )
	{
		m_avTyreTrackPositions[ i ] = TVector4::VEC_ZERO;
		m_afTyreTrackOffsets[ i ]   = 0.0f;

#ifdef BARNYARD_COMMUNITY_PATCH
		m_aiWheelSurfaces[ i ]   = -1;
		m_afTyreTrackWidths[ i ] = 0.3f;
#else
		// NOTE: retail writes the surface and the width of every wheel 16 bytes too far, into
		// m_afDustTimers[ 0 ] and m_aiWheelSurfaces[ 0 ] (both are overwritten before they are used)
		*TREINTERPRETCAST( TINT*, &m_afDustTimers[ 0 ] )      = -1;
		*TREINTERPRETCAST( TFLOAT*, &m_aiWheelSurfaces[ 0 ] ) = 0.3f;
#endif

		m_afDustTimers[ i ]  = 0.0f;
		m_afGrassTimers[ i ] = 0.0f;
	}
}

// $Barnyard: FUNCTION 004f6180
void AUnitCarChaseCar::UpdateWheelEffects( TFLOAT a_fDeltaTime )
{
	if ( !m_pSteer->IsA( &TGetClass( ARacingCarSteer ) ) )
		return;

	APhysicsCar* pCar = TSTATICCAST( ARacingCarSteer, m_pSteer )->GetCar();

	// Dust and grass under the wheels
	for ( TINT i = 0; i < NUM_WHEELS; i++ )
	{
		APhysicsWheel& rWheel = pCar->m_aWheels[ i ];

		TVector4 vecDirection = TVector4::VEC_NEGY;
		TVector4 vecUp;
		vecUp.Multiply( vecDirection, 0.2f );

		TVector4 vecVelocityOffset;
		vecVelocityOffset.Multiply( pCar->GetBody().m_vVelocity, a_fDeltaTime );

		m_afDustTimers[ i ] -= a_fDeltaTime;
		if ( !( m_afDustTimers[ i ] > 0.0f ) ) m_afDustTimers[ i ] = 0.0f;

		m_afGrassTimers[ i ] -= a_fDeltaTime;
		if ( !( m_afGrassTimers[ i ] > 0.0f ) ) m_afGrassTimers[ i ] = 0.0f;

		if ( pCar->GetBody().m_vVelocity.Magnitude() > 0.5555556f && rWheel.m_bOnGround == TTRUE )
		{
			TVector4 vecPosition;
			vecPosition.x = rWheel.m_vPosition.x + vecUp.x - vecVelocityOffset.x;
			vecPosition.y = rWheel.m_vPosition.y + vecUp.y - vecVelocityOffset.y;
			vecPosition.z = rWheel.m_vPosition.z + vecUp.z - vecVelocityOffset.z;
			vecPosition.w = rWheel.m_vPosition.w;

			if ( rWheel.m_iSurfaceType == AVehicleManager::SURFACE_DIRT && m_afDustTimers[ i ] <= 0.0f )
			{
				AParticleEffectManager::GetSingleton()->PlayEffect( m_iDustEffect, vecPosition, vecDirection );
				m_afDustTimers[ i ] = 0.05f;
			}
			else if ( rWheel.m_iSurfaceType == AVehicleManager::SURFACE_GRASS && m_afGrassTimers[ i ] <= 0.0f )
			{
				AParticleEffectManager::GetSingleton()->PlayEffect( m_iGrassEffect, vecPosition, vecDirection );
				m_afGrassTimers[ i ] = 0.1f;
			}
		}
	}

	// The tyre tracks
	AVehicleSkidManager* pSkidManager = AVehicleSkidManager::GetSingleton();

	if ( pSkidManager )
	{
		for ( TINT i = 0; i < NUM_WHEELS; i++ )
		{
			APhysicsWheel& rWheel   = pCar->m_aWheels[ i ];
			TVector4&      rLastPos = m_avTyreTrackPositions[ i ];
			TFLOAT&        rfOffset = m_afTyreTrackOffsets[ i ];
			TFLOAT         fWidth   = m_afTyreTrackWidths[ i ];

			if ( rWheel.m_bOnGround )
			{
				TBOOL  bLeaveTrack = TFALSE;
				TFLOAT fRed;
				TFLOAT fGreen;
				TFLOAT fBlue;
				TFLOAT fMaxAlpha;

				switch ( rWheel.m_iSurfaceType )
				{
					case AVehicleManager::SURFACE_DIRT:
					case AVehicleManager::SURFACE_SAND:
						fRed        = 0.71f;
						fGreen      = 0.64f;
						fBlue       = 0.39f;
						fMaxAlpha   = 0.5f;
						bLeaveTrack = TTRUE;
						break;
					case AVehicleManager::SURFACE_WATER:
					case AVehicleManager::SURFACE_MUD:
						fRed      = 0.71f;
						fGreen    = 0.64f;
						fBlue     = 0.39f;
						fMaxAlpha = 0.5f;
						break;
					default:
						fRed      = 0.0f;
						fGreen    = 0.0f;
						fBlue     = 0.0f;
						fMaxAlpha = 1.0f;
						break;
				}

				// Dark tracks while sliding
				TBOOL bSliding = TFALSE;

				if ( TMath::Abs( rWheel.m_fSlipAngle ) > 0.08726646f && rWheel.m_vContactVelocity.Magnitude() > pCar->m_fLowSpeed )
					bSliding = TTRUE;

				if ( bLeaveTrack == TTRUE || bSliding )
				{
					const TVector4& rcContact = rWheel.m_vContactPosition;

					TVector4 vecDelta;
					vecDelta.Substract( rLastPos, rcContact );

					if ( vecDelta.MagnitudeSq() > 0.36f )
					{
						if ( !rLastPos.IsEqual( TVector4::VEC_ZERO ) )
						{
							AVehicleSkidManager::TyreTrack* pTrack = pSkidManager->GetFreeTrack();

							if ( pTrack )
							{
								pTrack->vPosition = rcContact;
								pTrack->vNormal   = rWheel.m_vGroundNormal;

								TVector4 vecTrackDirection;
								vecTrackDirection.Substract( rLastPos, rcContact );
								TFLOAT fLength = vecTrackDirection.NormaliseMag();

								TFLOAT fAlpha = rWheel.m_vContactVelocity.Magnitude() * 0.2f;
								TMath::Clip( fAlpha, 0.0f, fMaxAlpha );

								pTrack->vDirection = vecTrackDirection;
								pTrack->fWidth     = fWidth;
								pTrack->fLength    = fLength;
								pTrack->uiColour   = PackTyreTrackColour( fRed, fGreen, fBlue, fAlpha );
								pTrack->fUVOffset  = rfOffset;

								rfOffset += fLength / fWidth;
								rfOffset -= TMath::FloorToInt( rfOffset );
							}
						}

						rLastPos = rcContact;
					}

					continue;
				}
			}

			rLastPos = TVector4::VEC_ZERO;
			rfOffset = 0.0f;
		}
	}

	for ( TINT i = 0; i < NUM_WHEELS; i++ )
		m_aiWheelSurfaces[ i ] = pCar->m_aWheels[ i ].m_bOnGround ? pCar->m_aWheels[ i ].m_iSurfaceType : AVehicleManager::SURFACE_NONE;
}

// $Barnyard: FUNCTION 004f6820
void AUnitCarChaseCar::PlaySparks( APhysicsVehicleContact* a_pContact )
{
	if ( a_pContact->iSurfaceType != AVehicleManager::SURFACE_NONE && a_pContact->iSurfaceType != AVehicleManager::SURFACE_STONE && a_pContact->iSurfaceType != AVehicleManager::SURFACE_TIN )
		return;

	if ( !m_pSteer->IsA( &TGetClass( ARacingCarSteer ) ) )
		return;

	APhysicsCar*     pCar        = TSTATICCAST( ARacingCarSteer, m_pSteer )->GetCar();
	const TMatrix44& rcTransform = pCar->GetBody().m_oTransform;

	TMatrix44 matInvTransform = rcTransform;
	matInvTransform.Invert( matInvTransform );

	// The sparks fly along the contact surface
	TVector4 vecDirection;
	vecDirection.Multiply( a_pContact->vRelativeVelocity, -1.0f );

	TVector4 vecNormalVelocity;
	vecNormalVelocity.Multiply( a_pContact->vNormal, TVector4::DotProduct3( a_pContact->vRelativeVelocity, a_pContact->vNormal ) );

	vecDirection.x += vecNormalVelocity.x;
	vecDirection.y += vecNormalVelocity.y;
	vecDirection.z += vecNormalVelocity.z;

	if ( vecDirection.MagnitudeSq() < 0.001f )
		return;

	vecDirection.Normalise();

	if ( s_bLocalSparks )
	{
		TVector4 vecLocal = a_pContact->vPosition;

		if ( cos( 1.745329238474369 ) <= TVector4::DotProduct3( rcTransform.AsBasisVector4( BASISVECTOR_UP ), TVector4::VEC_POSY ) )
		{
			// Not too far below the car
			TMatrix44::TransformVector( vecLocal, matInvTransform, vecLocal );
			vecLocal.y = TMath::Max( vecLocal.y, -1.3f );
		}
		else
		{
			TMatrix44::TransformVector( vecLocal, matInvTransform, vecLocal );
			TMath::Clip( vecLocal.z, -1.0f, 1.0f );
		}

		TVector4 vecPosition;
		TMatrix44::TransformVector( vecPosition, rcTransform, vecLocal );
		AParticleEffectManager::GetSingleton()->PlayEffect( m_iSparksEffect, vecPosition, vecDirection );
	}
	else
	{
		AParticleEffectManager::GetSingleton()->PlayEffect( m_iSparksEffect, a_pContact->vPosition, vecDirection );
	}
}

// $Barnyard: FUNCTION 004f7460
void AUnitCarChaseCar::UpdateCollisionSounds( TFLOAT a_fDeltaTime )
{
	static TFLOAT s_fSparksCos = TFLOAT( cos( 0.5235987715423107 ) );

	const TVector4* pPosition     = m_pSteer->GetPosition();
	ASoundManager*  pSoundManager = ASoundManager::GetSingleton();

	if ( m_pSteer->IsA( &TGetClass( ARacingCarSteer ) ) )
	{
		ARacingCarSteer* pSteer         = TSTATICCAST( ARacingCarSteer, m_pSteer );
		APhysicsCar*     pCar           = pSteer->GetCar();
		TVector4         vecCarPosition = pCar->GetBody().m_oTransform.AsBasisVector4( BASISVECTOR_TRANSLATION );

		// The suspension bottomed out
		if ( pCar->m_oBottomOut.iCount > 0 && pCar->m_oBottomOut.vImpulse.Magnitude() > 10.0f &&
		     ( m_iSuspensionCue == -1 || !pSoundManager->IsCueUsed( m_iSuspensionCue ) ) )
		{
			TFLOAT fVolume = pSteer->GetSpeed() / pSteer->GetMaxSpeed();
			TMath::Clip( fVolume, 0.25f, 1.0f );

			m_iSuspensionCue = pSoundManager->PlayCueAtPositionEx( soundbank::CARCHASE_SUSPENSION, vecCarPosition, fVolume * 0.5f, TFALSE, 0.0f, -1 );

			if ( APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) == this )
				AInputHandler::GetSingleton()->SetRumble( AInputHandler::GetSingleton()->GetMainController(), &s_oCrashRumble );
		}

		TINT iNumContacts = pCar->m_iNumContacts;

		if ( iNumContacts > 0 )
		{
			// Impulses from the other bodies and from the static collision
			TVector4 vecStaticImpulse = TVector4::VEC_ZERO;
			TVector4 vecBodyImpulse   = TVector4::VEC_ZERO;

			for ( TINT i = 0; i < iNumContacts; i++ )
			{
				APhysicsVehicleContact* pContact = pCar->GetContact( i );

				TVector4 vecImpulse;
				vecImpulse.Multiply( pContact->vNormal, pContact->fImpulse );

				if ( pContact->pOtherBody )
				{
					vecBodyImpulse.x += vecImpulse.x;
					vecBodyImpulse.y += vecImpulse.y;
					vecBodyImpulse.z += vecImpulse.z;
				}
				else
				{
					vecStaticImpulse.x += vecImpulse.x;
					vecStaticImpulse.y += vecImpulse.y;
					vecStaticImpulse.z += vecImpulse.z;
				}

				if ( pContact->vRelativeVelocity.MagnitudeSq() > 94.52161f &&
				     TMath::Abs( TVector4::DotProduct3( pContact->vNormal, pContact->vRelativeVelocity ) ) > s_fSparksCos )
				{
					PlaySparks( pContact );
				}
			}

			TFLOAT fVolume = pSteer->GetSpeed() / pSteer->GetMaxSpeed();
			TMath::Clip( fVolume, 0.25f, 1.0f );

			if ( vecStaticImpulse.Magnitude() > 500.0f && ( m_iCrashCue == -1 || !pSoundManager->IsCueUsed( m_iCrashCue ) ) )
			{
				m_iCrashCue = pSoundManager->PlayCueAtPositionEx( soundbank::CARCHASE_CRASH, vecCarPosition, fVolume * 0.5f, TFALSE, 0.0f, -1 );

				if ( APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) == this )
					AInputHandler::GetSingleton()->SetRumble( AInputHandler::GetSingleton()->GetMainController(), &s_oCrashRumble );
			}

			if ( vecBodyImpulse.Magnitude() > 75.0f && ( m_iCrashCue == -1 || !pSoundManager->IsCueUsed( m_iCrashCue ) ) )
			{
				m_iCrashCue = pSoundManager->PlayCueAtPositionEx( soundbank::CARCHASE_CAR_IMPACT, vecCarPosition, fVolume * 0.5f, TFALSE, 0.0f, -1 );

				if ( APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) == this )
					AInputHandler::GetSingleton()->SetRumble( AInputHandler::GetSingleton()->GetMainController(), &s_oCrashRumble );
			}
		}
	}

	// Move the engine cues with the car
	pSoundManager->SetCuePosition( m_iIgnitionCue, *pPosition, 0.0f );
	pSoundManager->SetCuePosition( m_iIdleCue, *pPosition, 0.0f );
	pSoundManager->SetCuePosition( m_iEngineCue, *pPosition, 0.0f );
	pSoundManager->SetCuePosition( m_iEngineLoopCue, *pPosition, 0.0f );
	pSoundManager->SetCuePosition( m_iGearCue, *pPosition, 0.0f );
	pSoundManager->SetCuePosition( m_iSirenCue, *pPosition, 0.0f );

	if ( m_iSirenCue != -1 && !pSoundManager->IsCueUsed( m_iSirenCue ) )
		m_iSirenCue = -1;
}

// $Barnyard: FUNCTION 004f7a60
void AUnitCarChaseCar::UpdateEngineSounds( TFLOAT a_fDeltaTime )
{
	const TVector4& rcPosition    = *m_pSteer->GetPosition();
	ASoundManager*  pSoundManager = ASoundManager::GetSingleton();

	if ( m_pSteer->IsA( &TGetClass( ARacingCarSteer ) ) == TTRUE )
	{
		// Pitch of the engine loop
		TFLOAT fMinPitch;
		TFLOAT fMaxPitch;
		TFLOAT fMaxPitchHighGear;
		TFLOAT fMaxPitchLowGear;
		TFLOAT fNeutralMinPitch;
		TFLOAT fNeutralMaxPitch;

		if ( m_eCarType != CARTYPE_PLAYER )
		{
			fMinPitch         = 0.3f;
			fMaxPitch         = 2.0f;
			fMaxPitchHighGear = 1.7f;
			fMaxPitchLowGear  = 1.3f;
			fNeutralMinPitch  = 0.1f;
			fNeutralMaxPitch  = 1.5f;
		}
		else
		{
			fMinPitch         = 0.90000004f;
			fMaxPitch         = 2.6f;
			fMaxPitchHighGear = 2.3000002f;
			fMaxPitchLowGear  = 1.9f;
			fNeutralMinPitch  = 0.70000005f;
			fNeutralMaxPitch  = 2.1f;
		}

		APhysicsCar* pCar  = TSTATICCAST( ARacingCarSteer, m_pSteer )->GetCar();
		TINT         iGear = pCar->m_oGearBox.iGear;

		switch ( m_eEngineState )
		{
			case ENGINESTATE_STARTING:
				if ( m_iIgnitionCue == -1 || pSoundManager->IsCueUsed( m_iIgnitionCue ) )
				{
					// Pressing the throttle skips the ignition
					if ( pCar->m_fThrottle <= 0.0f )
						break;

					m_eEngineState = ENGINESTATE_IDLE;
				}
				else
				{
					GetRacingCarSteer()->SetEngineStarted( TTRUE );
					m_eEngineState = ENGINESTATE_IDLE;
				}
				break;
			case ENGINESTATE_IDLE:
			{
				if ( ( pCar->m_fThrottle <= 0.0f || iGear < 1 ) && ( pCar->m_fBrake <= 0.0f || iGear != 0 ) )
					break;

				pSoundManager->FadeOutCue( m_iIdleCue, 0.5f, TTRUE, 3.0f );
				m_iIdleCue = -1;

				TFLOAT fRPM = 60.0f / ( 2.0f * TMath::PI ) * pCar->m_oEngine2.m_fRPM;
				TFLOAT fPitch;

				if ( iGear == 1 )
				{
					fPitch = ( fNeutralMaxPitch - fNeutralMinPitch ) / pCar->m_oEngine2.m_fMaxRPM * fRPM;
				}
				else
				{
					fPitch = ( fMaxPitch - fMinPitch ) / pCar->m_oEngine2.m_fMaxRPM * fRPM;
					TMath::Clip( fPitch, fMinPitch, ( iGear <= 1 ) ? fMaxPitchLowGear : fMaxPitchHighGear );
				}

				m_iEngineLoopCue = pSoundManager->PlayCueAtPositionEx( m_iEngineLoopSound, rcPosition, 0.0f, TFALSE, 0.0f, -1 );
				pSoundManager->FadeCue( m_iEngineLoopCue, 0.2f, 1.0f, 3.0f );
				pSoundManager->SetCueFrequency( m_iEngineLoopCue, fPitch );
				m_eEngineState = ENGINESTATE_REVVING;
				break;
			}
			case ENGINESTATE_REVVING:
				if ( m_iDriveState == DRIVESTATE_ACCELERATE || m_iDriveState == DRIVESTATE_BRAKE )
				{
					TFLOAT fRPM = 60.0f / ( 2.0f * TMath::PI ) * pCar->m_oEngine2.m_fRPM;
					TFLOAT fPitch;

					if ( iGear == 1 )
					{
						fPitch = ( fNeutralMaxPitch - fNeutralMinPitch ) / pCar->m_oEngine2.m_fMaxRPM * fRPM;
					}
					else
					{
						fPitch = ( fMaxPitch - fMinPitch ) / pCar->m_oEngine2.m_fMaxRPM * fRPM;
						TMath::Clip( fPitch, fMinPitch, ( iGear <= 1 ) ? fMaxPitchLowGear : fMaxPitchHighGear );
					}

					pSoundManager->SetCueFrequency( m_iEngineLoopCue, fPitch );
				}
				else
				{
					m_eEngineState = ENGINESTATE_RUNNING;
				}
				break;
			case ENGINESTATE_RUNNING:
			{
				TFLOAT fRPM = 60.0f / ( 2.0f * TMath::PI ) * pCar->m_oEngine2.m_fRPM;

				if ( fRPM < 2000.0f && pCar->m_fThrottle <= 0.0f && iGear == 1 )
				{
					// Back to idle in neutral
					if ( m_iEngineLoopCue != -1 )
					{
						pSoundManager->FadeOutCue( m_iEngineLoopCue, 0.5f, TTRUE, 3.0f );
						m_iEngineLoopCue = -1;
					}

					m_iIdleCue = pSoundManager->PlayCueAtPositionEx( ( m_eCarType != CARTYPE_BEADY ) ? soundbank::CARCHASE_IDLE01 : soundbank::CARCHASE_B_IDLE, rcPosition, 0.0f, TFALSE, 0.0f, -1 );
					pSoundManager->FadeCue( m_iIdleCue, 0.5f, 1.0f, 3.0f );
					m_eEngineState = ENGINESTATE_IDLE;
					break;
				}

				TFLOAT fRatio = fRPM / pCar->m_oEngine2.m_fMaxRPM;
				TFLOAT fPitch = ( fMaxPitch - fMinPitch ) * fRatio;

				if ( iGear == 1 )
				{
					fPitch = ( fNeutralMaxPitch - fNeutralMinPitch ) * fRatio;
				}
				else
				{
					// Lower pitch while the pedal is pressed
					TFLOAT fPedal = 1.0f;

					if ( iGear > 1 )
						fPedal = 1.0f - pCar->m_fThrottle;
					else if ( iGear == 0 )
						fPedal = 1.0f - pCar->m_fBrake;

					fPitch *= TMath::LERP( 1.0f, 0.8f, fPedal );
					TMath::Clip( fPitch, fMinPitch, ( iGear <= 1 ) ? fMaxPitchLowGear : fMaxPitchHighGear );
				}

				pSoundManager->SetCueFrequency( m_iEngineLoopCue, fPitch );

				if ( ( m_iDriveState == DRIVESTATE_ACCELERATE && iGear > 1 ) || ( m_iDriveState == DRIVESTATE_BRAKE && iGear == 0 ) )
					m_eEngineState = ENGINESTATE_REVVING;

				break;
			}
			default:
				break;
		}

		UpdateGearSound();
	}
	else
	{
		// NOTE: a car without physics (ACarSteer)
		ACarSteer* pSteer = TSTATICCAST( ACarSteer, m_pSteer );

		switch ( m_iDriveState )
		{
			case DRIVESTATE_ACCELERATE:
			case DRIVESTATE_BRAKE:
				if ( m_iUnkD0Cue != -1 )
				{
					pSoundManager->StopCue( m_iUnkD0Cue );
					m_iUnkD0Cue = -1;
				}

				if ( m_iDriveState == DRIVESTATE_BRAKE )
					m_fEnginePitch += a_fDeltaTime * -0.5f;
				else
					m_fEnginePitch += a_fDeltaTime * 0.75f;

				TMath::Clip( m_fEnginePitch, 0.3f, 2.0f );

				if ( m_fEnginePitch >= 2.0f )
				{
					TFLOAT fSpeed = pSteer->GetSpeed();

					if ( pSteer->GetMaxSpeed() * 0.8f > fSpeed )
						m_fEnginePitch = 0.3f;
				}

				if ( m_iEngineCue == -1 )
				{
					m_iEngineCue = pSoundManager->PlayCueAtPositionEx( m_iEngineLoopSound, pSteer->GetBodyPosition(), 0.0f, TFALSE, 0.0f, -1 );
					pSoundManager->SetCuePosition( m_iEngineCue, pSteer->GetBodyPosition(), a_fDeltaTime );
					pSoundManager->SetCueFrequency( m_iEngineCue, m_fEnginePitch );
				}

				pSoundManager->SetCuePosition( m_iEngineCue, pSteer->GetBodyPosition(), a_fDeltaTime );
				pSoundManager->SetCueFrequency( m_iEngineCue, m_fEnginePitch );
				pSoundManager->SetCueVolume( m_iEngineCue, 0.2f );
				break;
			case DRIVESTATE_UNK7:
				if ( m_iEngineCue != -1 )
				{
					m_fEnginePitch -= a_fDeltaTime * 0.5f;
					TMath::Clip( m_fEnginePitch, 0.3f, 2.0f );
					pSoundManager->SetCueFrequency( m_iEngineCue, m_fEnginePitch );
				}

				// NOTE: the result is unused in retail
				pSteer->GetSpeed();
				break;
			case DRIVESTATE_IDLE:
				if ( m_iEngineCue != -1 )
				{
					m_fEnginePitch -= a_fDeltaTime * 0.5f;
					TMath::Clip( m_fEnginePitch, 0.3f, 2.0f );
					pSoundManager->SetCueFrequency( m_iEngineCue, m_fEnginePitch );
				}

				if ( m_iUnkD0Cue != -1 )
				{
					pSoundManager->StopCue( m_iUnkD0Cue );
					m_iUnkD0Cue = -1;
				}
				break;
			default:
				break;
		}
	}
}

// $Barnyard: FUNCTION 004f81e0
void AUnitCarChaseCar::GetWheelTransform( TINT a_iWheel, TMatrix44& a_rOutTransform )
{
	a_rOutTransform = TMatrix44::IDENTITY;

	if ( !m_apWheels[ a_iWheel ] )
		return;

	TSkeletonInstance* pSkeletonInstance = GetAnimatableObject()->GetModelInstance()->GetSkeletonInstance();

	if ( !( pSkeletonInstance->GetBones() + m_aiWheelBones[ a_iWheel ] ) )
		return;

	TMatrix44 matCar;
	TSTATICCAST( ACarSteer, m_pSteer )->GetSeatMatrix( matCar );

	TMatrix44 matWheel = TMatrix44::IDENTITY;
	matWheel.Multiply( matCar, pSkeletonInstance->GetBoneTransformCurrent( m_aiWheelBones[ a_iWheel ] ) );

	// The wheels on the left side are turned around
	if ( a_iWheel == 0 || a_iWheel == 2 )
		matWheel.RotateZ( TMath::PI );

	if ( m_pSteer->IsA( &TGetClass( ARacingCarSteer ) ) )
	{
		APhysicsCar*   pCar   = TSTATICCAST( ARacingCarSteer, m_pSteer )->GetCar();
		APhysicsWheel& rWheel = pCar->m_aWheels[ a_iWheel ];

		// The suspension
		TMatrix44 matSuspension = TMatrix44::IDENTITY;
		TFLOAT    fExtension    = TMath::Max( rWheel.m_fPrevSpringLength, rWheel.m_oParams.fMaxExtension * -0.5f );
		matSuspension.AsBasisVector4( BASISVECTOR_TRANSLATION ).Multiply( TVector4::VEC_POSZ, fExtension );

		// The rotation of the wheel
		TMatrix44 matRotation = TMatrix44::IDENTITY;
		TFLOAT    fRotation   = -rWheel.m_fRotation;

		if ( a_iWheel == 0 || a_iWheel == 2 )
			fRotation *= -1.0f;

		matRotation.RotateX( fRotation );

		// The steering of the front wheels
		TMatrix44 matSteering = TMatrix44::IDENTITY;

		if ( a_iWheel == 0 || a_iWheel == 1 )
			matSteering.RotateZ( pCar->m_fSteeringAngle );

		TMatrix44 matTemp = matWheel;
		matWheel.Multiply( matTemp, matSuspension );

		matTemp = matWheel;
		matWheel.Multiply( matTemp, matSteering );

		matTemp = matWheel;
		matWheel.Multiply( matTemp, matRotation );
	}

	a_rOutTransform = matWheel;
}

// $Barnyard: FUNCTION 004f8ba0
void AUnitCarChaseCar::UpdateAI( TFLOAT a_fDeltaTime )
{
	if ( !IsVisible() )
		return;

	ARacingCarSteer* pSteer = GetRacingCarSteer();

	if ( m_iState == STATE_RACING )
	{
		UpdateCollisionSounds( a_fDeltaTime );
		UpdateEngineSounds( a_fDeltaTime );
		UpdateSkidSound( a_fDeltaTime );
	}

	if ( m_bWheelEffects == TTRUE )
		UpdateWheelEffects( a_fDeltaTime );

	// The gravel sound of the player's car
	if ( APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) == this )
	{
		APhysicsCar* pCar = pSteer->GetCar();

		// NOTE: the fourth wheel isn't checked in retail
		TINT iNumOnGravel = 0;
		if ( pCar->m_aWheels[ 0 ].m_iSurfaceType == AVehicleManager::SURFACE_DIRT ) iNumOnGravel++;
		if ( pCar->m_aWheels[ 1 ].m_iSurfaceType == AVehicleManager::SURFACE_DIRT ) iNumOnGravel++;
		if ( pCar->m_aWheels[ 2 ].m_iSurfaceType == AVehicleManager::SURFACE_DIRT ) iNumOnGravel++;

		ASoundManager* pSoundManager = ASoundManager::GetSingleton();

		if ( iNumOnGravel < 2 )
		{
			if ( m_iGravelCue != -1 )
			{
				pSoundManager->StopCue( m_iGravelCue );
				m_iGravelCue = -1;
			}
		}
		else
		{
			TFLOAT fVolume = pSteer->GetSpeed() / pSteer->GetMaxSpeed() * iNumOnGravel;
			TMath::Clip( fVolume, 0.0f, 1.0f );

			if ( m_iGravelCue == -1 )
				m_iGravelCue = pSoundManager->PlayCueAtPositionEx( soundbank::CARCHASE_GRAVEL_LOOP, *pSteer->GetPosition(), fVolume, TFALSE, 0.0f, -1 );

			if ( m_iGravelCue != -1 )
			{
				fVolume = pSteer->GetSpeed() / pSteer->GetMaxSpeed() * iNumOnGravel;
				TMath::Clip( fVolume, 0.0f, 1.0f );

				pSoundManager->SetCueVolume( m_iGravelCue, fVolume );
				pSoundManager->SetCuePosition( m_iGravelCue, *pSteer->GetPosition(), 0.0f );
			}
		}
	}

	// The body
	TMatrix44 matCar;
	pSteer->GetSeatMatrix( matCar );
	GetAnimatableObject()->GetModelInstance()->SetTransform( matCar );

	// The drivers sit on the base bone
	TSkeletonInstance* pSkeletonInstance = GetAnimatableObject()->GetModelInstance()->GetSkeletonInstance();

	TMatrix44 matSeat = TMatrix44::IDENTITY;
	matSeat.Multiply( matCar, pSkeletonInstance->GetBoneTransformCurrent( m_iBaseBone ) );

	if ( m_pDriver )
		m_pDriver->GetModelInstance()->SetTransform( matSeat );

	if ( m_pPassenger1 )
		m_pPassenger1->GetModelInstance()->SetTransform( matSeat );

	if ( m_pPassenger2 )
		m_pPassenger2->GetModelInstance()->SetTransform( matSeat );

	// The wheels
	for ( TINT i = 0; i < NUM_WHEELS; i++ )
	{
		TMatrix44 matWheel;
		GetWheelTransform( i, matWheel );
		m_apWheels[ i ]->GetModelInstance()->SetTransform( matWheel );
	}

	// The brake and the reverse lights
	TBOOL bReverse = TSTATICCAST( ARacingCarSteer, m_pSteer )->GetCar()->m_oGearBox.iGear == 0;

	for ( TINT i = 0; i < 2; i++ )
	{
		if ( m_iDriveState != DRIVESTATE_BRAKE || bReverse )
		{
			if ( m_apBrakeLights[ i ] )
			{
				m_apBrakeLights[ i ]->GetInstance()->Kill();
				m_apBrakeLights[ i ] = TNULL;
			}
		}
		else if ( !m_apBrakeLights[ i ] )
		{
			m_apBrakeLights[ i ] = AParticleEffectManager::GetSingleton()->PlayEffect( m_iTaillightEffect, TVector4::VEC_ZERO, TVector4::VEC_NEGZ );
			m_apBrakeLights[ i ]->AttachToBone( GetAnimatableObject()->GetModelInstance(), m_aiBrakeLightBones[ i ], TTRUE, TFALSE );
		}

		if ( GetAnimatableObjectType()->GetName() == TPS8( carchasecar3 ) || GetAnimatableObjectType()->GetName() == TPS8( carchasecar1 ) )
		{
			if ( bReverse == TTRUE )
			{
				if ( !m_apReverseLights[ i ] )
				{
					m_apReverseLights[ i ] = AParticleEffectManager::GetSingleton()->PlayEffect( m_iHeadlightEffect, TVector4::VEC_ZERO, TVector4::VEC_NEGZ );
					m_apReverseLights[ i ]->AttachToBone( GetAnimatableObject()->GetModelInstance(), m_aiReverseLightBones[ i ], TTRUE, TFALSE );
				}
			}
			else if ( m_apReverseLights[ i ] )
			{
				m_apReverseLights[ i ]->GetInstance()->Kill();
				m_apReverseLights[ i ] = TNULL;
			}
		}
	}

	AAnimatedUnit::UpdateAI( a_fDeltaTime );
}
