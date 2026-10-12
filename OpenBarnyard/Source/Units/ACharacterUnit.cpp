#include "pch.h"
#include "ACharacterUnit.h"
#include "AUnitManager.h"
#include "AUnitFactory.h"
#include "AUnitType.h"
#include "AInteractionManager.h"
#include "ACharacterMovementComponent.h"
#include "ARandom.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "Render/AShadowManager.h"
#include "UnitMovement/Steering/ASteer.h"
#include "FSM/AFunctionTable.h"
#include "FSM/AMethodPointer.h"

#include <Toshi/TSceneObject.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( pip );
TPSTRING8_DECLARE( generic_chick );
TPSTRING8_DECLARE( twink );

// NOTE: GetClass 00416c20, CreateObject 00416d50, CreateObjectInPlace 00416de0 (the constructor is inlined in both)
TDEFINE_CLASS_INIT( ACharacterUnit, 1, 1 );

AFunctionTable ACharacterUnit::ms_oFunctionTable( &TGetClass( ACharacterUnit ), &AAnimatedUnit::ms_oFunctionTable );

// Opacity change per second while fading
static constexpr TFLOAT FADE_SPEED = 10.0f;

// $Barnyard: FUNCTION 004179e0
void ACharacterUnit::InitialiseClass()
{
	ms_oFunctionTable.Register( TNULL );

	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TBOOL, ACharacterUnit>::Create( "StartConversationWithTarget", &ACharacterUnit::StartConversationWithTarget ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<ACharacterUnit*, ACharacterUnit>::Create( "GetNearbyCharacter", &ACharacterUnit::GetNearbyCharacter ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<ACharacterUnit*, ACharacterUnit>::Create( "GetNearbyAvailableCharacter", &ACharacterUnit::GetNearbyAvailableCharacter ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1ArgsRT<ACharacterUnit*, ACharacterUnit, const TPString8&>::Create( "GetNearbyCharacterOfType", &ACharacterUnit::GetNearbyCharacterOfType ) );
	ms_oFunctionTable.AddFunction( AMethodPointer2ArgsRT<TBOOL, ACharacterUnit, AUnit*, TFLOAT>::Create( "IsCloseToUnit", &ACharacterUnit::IsCloseToUnit ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0Args<ACharacterUnit>::Create( "SetDesireWalk", &ACharacterUnit::SetDesireWalk ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0Args<ACharacterUnit>::Create( "SetDesireRun", &ACharacterUnit::SetDesireRun ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<ACharacterUnit, TFLOAT>::Create( "SetDesiredSpeed", &ACharacterUnit::SetDesiredSpeed ) );
}

// $Barnyard: FUNCTION 00416c30
void ACharacterUnit::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 00416c40
ACharacterUnit::ACharacterUnit()
    : m_fOpacity( 1.0f )
    , m_eFadeFlags( FADEFLAGS_NONE )
{
}

// $Barnyard: FUNCTION 00416e60
// $Barnyard: FUNCTION 00416c80
ACharacterUnit::~ACharacterUnit()
{
}

// $Barnyard: FUNCTION 00416c70
AFunctionTable* ACharacterUnit::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// $Barnyard: FUNCTION 004176b0
TBOOL ACharacterUnit::Create( AUnitTransform* a_pTransform )
{
	if ( !AAnimatedUnit::Create( a_pTransform ) )
		return TFALSE;

	m_eFadeFlags = FADEFLAGS_NONE;
	m_fOpacity   = 1.0f;

	TSTATICCAST( ACharacterMovementComponent, FindComponent( &TGetClass( ACharacterMovementComponent ) ) )->ResetSpeed( ACharacterMovementComponent::APPLY_IF_MOVING );

	m_eFlags |= FLAGS_AI_ENABLED;
	m_eFlags |= FLAGS_THINK_ENABLED;

	const TPString8& strTypeName = m_pType->GetName();

	if ( strTypeName != TPS8( pip ) )
	{
		// Smaller shadows for the chicks
		if ( strTypeName == TPS8( generic_chick ) || strTypeName == TPS8( twink ) )
			AShadowManager::GetSingleton()->AddUnitShadow( this, 0.5f, -1, -0.05f, AShadowManager::FLAGS_UNIFORM_SCALE, AShadowManager::TYPE_DEFAULT );
		else
			AShadowManager::GetSingleton()->AddUnitShadow( this, 1.25f, -1, -0.1f, AShadowManager::FLAGS_UNIFORM_SCALE, AShadowManager::TYPE_DEFAULT );
	}

	// Every character but the generic chicks can be aimed at with the milk reticle
	if ( m_pType->GetName() == TPS8( generic_chick ) )
		m_eFlags &= ~FLAGS_MILK_TARGET;
	else
		m_eFlags |= FLAGS_MILK_TARGET;

	return TTRUE;
}

// NOTE: identical code folding shares this one with AUnitActor::OnDestroy
// $Barnyard: FUNCTION 00507930
void ACharacterUnit::OnDestroy()
{
	AAnimatedUnit::OnDestroy();
}

// $Barnyard: FUNCTION 00416e80
void ACharacterUnit::UpdateAI( TFLOAT a_fDeltaTime )
{
	AVisibleUnit::UpdateAI( a_fDeltaTime );

	if ( m_eFadeFlags == FADEFLAGS_NONE )
		return;

	ACameraManager* pCameraManager = ACameraManager::GetSingleton();
	AModelInstance* pModelInstance = GetModelInstance();

	if ( !pModelInstance )
		return;

	if ( m_eFadeFlags & FADEFLAGS_FADE_OUT )
	{
		m_fOpacity -= FADE_SPEED * a_fDeltaTime;

		if ( m_fOpacity < 0.0f )
			m_fOpacity = 0.0f;

		pModelInstance->DisableDrawWithDefaultMaterials(); // NOTE: retail passes an unused argument (TTRUE)
		SetOpacity( m_fOpacity );

		ACameraHelper* pHelper = pCameraManager->GetSplitscreenCameraHelper( 1 );
		ACamera*       pCamera = pCameraManager->GetCurrentCamera();
		ASteer*        pSteer  = m_pSteer;

		if ( pHelper != pCameraManager->GetCurrentCameraHelper() || !pCamera || !pSteer )
		{
			// Fade back in
			m_eFadeFlags = ( m_eFadeFlags & ~FADEFLAGS_FADE_OUT ) | FADEFLAGS_FADE_IN;
			return;
		}

		// Fade back in once the camera is far enough
		if ( !ACameraHelper::IsPositionInFadeRange( pCamera->GetMatrix().GetTranslation(), *pSteer->GetPosition() ) )
			m_eFadeFlags = ( m_eFadeFlags & ~FADEFLAGS_FADE_OUT ) | FADEFLAGS_FADE_IN;
	}
	else if ( m_eFadeFlags & FADEFLAGS_FADE_IN )
	{
		m_fOpacity += FADE_SPEED * a_fDeltaTime;

		if ( m_fOpacity > 1.0f )
		{
			m_fOpacity = 1.0f;
			pModelInstance->DisableDrawWithDefaultMaterials(); // NOTE: retail passes an unused argument (TFALSE)
			SetOpacity( 1.0f );

			// NOTE: inlined AModelInstance method 00610700
			pModelInstance->ResetOpacity();
			m_eFadeFlags &= ~FADEFLAGS_FADE_IN;
		}
		else
		{
			pModelInstance->DisableDrawWithDefaultMaterials(); // NOTE: retail passes an unused argument (TTRUE)
			SetOpacity( m_fOpacity );
		}
	}
}

// NOTE: identical code folding shares this one with AUnitPlayer::Think
// $Barnyard: FUNCTION 004149e0
TBOOL ACharacterUnit::Think( TFLOAT a_fDeltaTime )
{
	ACharacterMovementComponent* pMovement = TSTATICCAST( ACharacterMovementComponent, FindComponent( &TGetClass( ACharacterMovementComponent ) ) );

	// Timed run desire, falls back to walking once it runs out
	if ( pMovement->GetRunTimer() > 0.0f )
	{
		TFLOAT fTimeLeft = pMovement->GetRunTimer() - a_fDeltaTime;

		if ( fTimeLeft <= 0.0f )
			pMovement->SetDesireWalk();

		pMovement->SetRunTimer( fTimeLeft );
	}

	return AVisibleUnit::Think( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00416cf0
void ACharacterUnit::SetFadeOut( TBOOL a_bFadeOut )
{
	if ( a_bFadeOut )
	{
		m_eFadeFlags = ( m_eFadeFlags & ~FADEFLAGS_FADE_IN ) | FADEFLAGS_FADE_OUT;
	}
	else if ( m_eFadeFlags != FADEFLAGS_NONE )
	{
		m_eFadeFlags = ( m_eFadeFlags & ~FADEFLAGS_FADE_OUT ) | FADEFLAGS_FADE_IN;
	}
}

// $Barnyard: FUNCTION 00417970
TBOOL ACharacterUnit::StartConversationWithTarget()
{
	AUnit* pTarget = m_pSteer->GetTargetedUnit();

	if ( pTarget != this && pTarget && pTarget->IsA( &TGetClass( ACharacterUnit ) ) )
	{
		AUnit* apUnits[ 2 ] = { this, pTarget };

		if ( AInteractionManager::GetSingleton()->AssignInteractionToUnits( apUnits, 2, TPString8(), AInteractionManager::ASSIGNFLAGS_FIRST_IS_INITIATOR | AInteractionManager::ASSIGNFLAGS_FORCED ) )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00417780
ACharacterUnit* ACharacterUnit::GetNearbyCharacter()
{
	TINT                    iNumFound;
	AUnitHash::QueryResult* pResults = AUnitManager::GetSingleton()->FindUnits( *m_pSteer->GetPosition(), 50.0f, &iNumFound, TFALSE, 50, TFALSE, TNULL );

	// NOTE: retail doesn't check for an empty result
	AUnit* pUnit = pResults[ ARandom::GetSingleton()->m_oRandom.GetInt( iNumFound ) ].pUnit;

	if ( !( pUnit->GetFlags() & FLAGS_NO_QUERY ) && pUnit->IsA( &TGetClass( ACharacterUnit ) ) )
		return TSTATICCAST( ACharacterUnit, pUnit );

	return TNULL;
}

// Filter of GetNearbyAvailableCharacter: units that have an interaction component
// and don't ignore s_pUnk0077e460.
// $Barnyard: FUNCTION 00414170
static TBOOL __stdcall IsUnitAvailableFilter( AUnit* a_pUnit )
{
	// NOTE: retail never writes this global, so it's always TNULL
	static AUnit* s_pUnk0077e460 = TNULL;

	AInteractionComponent* pInteraction = TSTATICCAST( AInteractionComponent, a_pUnit->FindComponent( &TGetClass( AInteractionComponent ) ) );

	if ( pInteraction )
		return !pInteraction->IsUnitIgnored( s_pUnk0077e460 );

	return TFALSE;
}

// $Barnyard: FUNCTION 004177f0
ACharacterUnit* ACharacterUnit::GetNearbyAvailableCharacter()
{
	TINT                    iNumFound;
	AUnitHash::QueryResult* pResults = AUnitManager::GetSingleton()->FindUnits( *m_pSteer->GetPosition(), 50.0f, &iNumFound, TFALSE, 50, TFALSE, IsUnitAvailableFilter );

	// NOTE: retail doesn't check for an empty result
	AUnit* pUnit = pResults[ ARandom::GetSingleton()->m_oRandom.GetInt( iNumFound ) ].pUnit;

	if ( !( pUnit->GetFlags() & FLAGS_NO_QUERY ) && pUnit->IsA( &TGetClass( ACharacterUnit ) ) )
		return TSTATICCAST( ACharacterUnit, pUnit );

	return TNULL;
}

// $Barnyard: FUNCTION 00417870
ACharacterUnit* ACharacterUnit::GetNearbyCharacterOfType( const TPString8& a_rcType )
{
	TINT                    iNumFound;
	AUnitHash::QueryResult* pResults = AUnitManager::GetSingleton()->FindUnits( *m_pSteer->GetPosition(), 50.0f, &iNumFound, TFALSE, 50, TFALSE, TNULL );

	AUnitType* pType = AUnitManager::GetSingleton()->GetUnitFactory()->Find( a_rcType );

	AUnit* pClosest         = TNULL;
	TFLOAT fClosestDistance = 0.0f;

	for ( TINT i = 0; i < iNumFound; i++ )
	{
		AUnit* pUnit = pResults[ i ].pUnit;

		if ( !( pUnit->GetFlags() & FLAGS_NO_QUERY ) && pUnit->IsA( &TGetClass( ACharacterUnit ) ) && pUnit->GetType() == pType )
		{
			TFLOAT fDistance = TVector4::DistanceSq( *pUnit->GetSteer()->GetPosition(), *m_pSteer->GetPosition() );

			if ( !pClosest || fDistance < fClosestDistance )
			{
				pClosest         = pUnit;
				fClosestDistance = fDistance;
			}
		}
	}

	return TSTATICCAST( ACharacterUnit, pClosest );
}

// $Barnyard: FUNCTION 00416ca0
TBOOL ACharacterUnit::IsCloseToUnit( AUnit* a_pUnit, TFLOAT a_fDistance )
{
	if ( !a_pUnit )
		return TFALSE;

	return TVector4::DistanceSq( *a_pUnit->GetSteer()->GetPosition(), *m_pSteer->GetPosition() ) < a_fDistance * a_fDistance;
}

// $Barnyard: FUNCTION 00417010
void ACharacterUnit::SetDesireWalk()
{
	TSTATICCAST( ACharacterMovementComponent, FindComponent( &TGetClass( ACharacterMovementComponent ) ) )->SetDesireWalk();
}

// $Barnyard: FUNCTION 00417030
void ACharacterUnit::SetDesireRun()
{
	TSTATICCAST( ACharacterMovementComponent, FindComponent( &TGetClass( ACharacterMovementComponent ) ) )->SetDesireRun();
}

// $Barnyard: FUNCTION 00417050
void ACharacterUnit::SetDesiredSpeed( TFLOAT a_fSpeed )
{
	TSTATICCAST( ACharacterMovementComponent, FindComponent( &TGetClass( ACharacterMovementComponent ) ) )->SetRunningSpeed( ACharacterMovementComponent::APPLY_IF_MOVING, a_fSpeed );
}
