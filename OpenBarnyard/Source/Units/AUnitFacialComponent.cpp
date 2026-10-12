#include "pch.h"
#include "AUnitFacialComponent.h"
#include "AGeneralUnitComponentManager.h"
#include "AAnimatedUnit.h"
#include "AUnit.h"
#include "AUnitManager.h"
#include "AUnitNameManager.h"
#include "AUnitType.h"
#include "AVisibleUnit.h"
#include "ARandom.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AModel.h"
#include "FSM/AFunctionTable.h"
#include "FSM/AMethodPointer.h"
#include "Player/APlayerManager.h"
#include "UnitMovement/Steering/ASteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00594070, CreateObject 00594df0, CreateObjectInPlace 00594e50
TDEFINE_CLASS_INIT( AUnitFacialComponent, 1, 0 );

// NOTE: GetClass 00594060, CreateObject 005955e0, CreateObjectInPlace 00595640
TDEFINE_CLASS( AUnitFacialComponentData );

TPSTRING8_DECLARE( LookUpLimit );
TPSTRING8_DECLARE( LookDownLimit );
TPSTRING8_DECLARE( LookLeftLimit );
TPSTRING8_DECLARE( LookRightLimit );
TPSTRING8_DECLARE( LookSpeed );
TPSTRING8_DECLARE( BoneName );
TPSTRING8_DECLARE( c_head );
TPSTRING8_DECLARE( blink );
TPSTRING8_DECLARE( lookl );
TPSTRING8_DECLARE( lookr );
TPSTRING8_DECLARE( looku );
TPSTRING8_DECLARE( lookd );

AFunctionTable AUnitFacialComponent::ms_oFunctionTable( &TGetClass( AUnitFacialComponent ), &AUnitComponent::ms_oFunctionTable );

// Turns the bone transforms of the models into the unit's space (0x00775118)
static const TMatrix44 s_matBoneToUnit(
    1.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    -1.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    1.0f
);

// $Barnyard: FUNCTION 00595880
void AUnitFacialComponent::InitialiseClass()
{
	ms_oFunctionTable.Register( TNULL );

	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AUnitFacialComponent, TFLOAT>::Create( "SetLookRandom", &AUnitFacialComponent::SetLookRandom ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AUnitFacialComponent, TFLOAT>::Create( "SetLookTargetRandom", &AUnitFacialComponent::SetLookTargetRandom ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0Args<AUnitFacialComponent>::Create( "StopLook", &AUnitFacialComponent::StopLook ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AUnitFacialComponent, TBOOL>::Create( "SetBlink", &AUnitFacialComponent::SetBlink ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AUnitFacialComponent, const TPString8&>::Create( "SetLookTargetByName", &AUnitFacialComponent::SetLookTargetByName ) );
}

// $Barnyard: FUNCTION 00594100
void AUnitFacialComponent::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 00594c00
AUnitFacialComponent::AUnitFacialComponent()
    : m_uiFlags( FLAGS_NONE )
    , m_fBlinkTimer( 0.0f )
    , m_fLookYawTarget( 0.0f )
    , m_fLookYaw( 0.0f )
    , m_fLookPitchTarget( 0.0f )
    , m_fLookPitch( 0.0f )
    , m_fLookSpeed( 0.0f )
    , m_fLookTimer( 0.0f )
    , m_fLookDuration( 0.0f )
    , m_fHeadHeight( 0.0f )
    , m_vLookTarget( TVector4::VEC_ZERO )
    , m_pData( TNULL )
{
}

// $Barnyard: FUNCTION 00594c70
// $Barnyard: FUNCTION 00594f20
AUnitFacialComponent::~AUnitFacialComponent()
{
	m_pLookTarget = TNULL;
}

// $Barnyard: FUNCTION 00594c60
AFunctionTable* AUnitFacialComponent::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// NOTE: folded with AUnitProfileComponent::GetManager (005d2300)
AUnitComponentManager* AUnitFacialComponent::GetManager()
{
	return AGeneralUnitComponentManager::GetSingletonSafe();
}

// $Barnyard: FUNCTION 00594f40
TBOOL AUnitFacialComponent::Create( AUnit* a_pUnit, AUnitFacialComponentData* a_pData )
{
	m_uiFlags        = FLAGS_NONE;
	m_pData          = a_pData;
	m_fBlinkTimer    = 0.0f;
	m_fLookYawTarget = 0.0f;
	m_fLookYaw       = 0.0f;
	m_fLookSpeed     = a_pData->m_fLookSpeed;
	m_fLookTimer     = 0.0f;
	m_fLookDuration  = 0.0f;
	m_pLookTarget    = TNULL;
	m_vLookTarget    = TVector4::VEC_ZERO;

	TVector4 vecHead;
	GetHeadPosition( a_pUnit, vecHead );
	m_fHeadHeight = vecHead.y;

	m_uiFlags |= FLAGS_BLINK;
	m_fBlinkTimer = 0.0f;

	return TTRUE;
}

// $Barnyard: FUNCTION 005951a0
void AUnitFacialComponent::UpdateAI( TFLOAT a_fDeltaTime )
{
	AUnit*              pOwner            = GetOwner();
	AAnimatableObject*  pAnimatableObject = TSTATICCAST( AAnimatedUnit, pOwner )->GetAnimatableObject();
	ANamedAnimationSet* pAnimationSet     = pAnimatableObject->GetToshiAnimInterface().GetAnimationSet();

	if ( ( m_uiFlags & FLAGS_BLINK ) && ( pAnimationSet->GetOverlayType() & ANamedAnimationSet::OVERLAYTYPE_BLINK ) )
	{
		m_fBlinkTimer -= a_fDeltaTime;

		if ( m_fBlinkTimer < 0.0f )
		{
			m_fBlinkTimer = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( 1.0f, 3.0f );
			pAnimatableObject->PlayAnimation( TPS8( blink ) );
		}
	}

	constexpr ANamedAnimationSet::OVERLAYTYPE OVERLAYTYPE_LOOK_YAW   = ANamedAnimationSet::OVERLAYTYPE_LOOK_LEFT | ANamedAnimationSet::OVERLAYTYPE_LOOK_RIGHT;
	constexpr ANamedAnimationSet::OVERLAYTYPE OVERLAYTYPE_LOOK_PITCH = ANamedAnimationSet::OVERLAYTYPE_LOOK_UP | ANamedAnimationSet::OVERLAYTYPE_LOOK_DOWN;

	if ( !( m_uiFlags & FLAGS_LOOKING ) ||
	     ( ( pAnimationSet->GetOverlayType() & OVERLAYTYPE_LOOK_YAW ) != OVERLAYTYPE_LOOK_YAW && ( pAnimationSet->GetOverlayType() & OVERLAYTYPE_LOOK_PITCH ) != OVERLAYTYPE_LOOK_PITCH ) )
	{
		return;
	}

	if ( m_uiFlags & FLAGS_LOOK_RANDOM )
	{
		m_fLookTimer -= a_fDeltaTime;

		if ( m_fLookTimer < 0.0f )
		{
			m_fLookTimer       = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( 3.0f, 6.0f );
			m_fLookYawTarget   = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( -0.5f, 0.5f );
			m_fLookPitchTarget = 0.0f;
		}
	}
	else if ( m_uiFlags & FLAGS_LOOK_AT_POSITION )
	{
		UpdateLookTarget( pOwner, m_vLookTarget );
	}
	else if ( m_uiFlags & FLAGS_LOOK_AT_UNIT )
	{
		AUnit* pTarget = m_pLookTarget ? m_pLookTarget->GetUnit() : TNULL;

		if ( !pTarget )
		{
			// The unit is gone
			SetLooking( TFALSE );
			return;
		}

		TVector4 vecTarget = *pTarget->GetSteer()->GetPosition();
		vecTarget.x += m_vLookTarget.x;
		vecTarget.y += m_vLookTarget.y;
		vecTarget.z += m_vLookTarget.z;

		UpdateLookTarget( pOwner, vecTarget );
	}
	else if ( m_uiFlags & FLAGS_LOOK_TARGET_RANDOM )
	{
		m_fLookTimer -= a_fDeltaTime;

		AUnit* pCurrentTarget = TNULL;

		if ( m_pLookTarget && m_pLookTarget->GetUnit() )
			pCurrentTarget = m_pLookTarget->GetUnit();

		AUnit* pNewTarget = FindLookTarget( *pOwner->GetSteer()->GetPosition(), 4.0f, 0.3f );

		// Look at someone else right away
		if ( pCurrentTarget && pCurrentTarget != pNewTarget )
			m_fLookTimer = -1.0f;

		if ( m_fLookTimer < 0.0f )
		{
			m_pLookTarget = TNULL;

			if ( pNewTarget && pNewTarget != pCurrentTarget )
				m_pLookTarget = pNewTarget->GetSelfProxy();

			if ( !m_pLookTarget )
			{
				// Nobody to look at, look somewhere to the side
				m_fLookYawTarget = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( 0.1f, 0.5f );

				if ( ARandom::GetSingleton()->m_oRandom.GetInt( 100 ) < 50 )
					m_fLookYawTarget *= -1.0f;

				m_fLookPitchTarget = 0.0f;
			}

			m_fLookTimer = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( 2.0f, 3.0f );
			m_fLookSpeed = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( 3.0f, 4.0f );
		}

		if ( m_pLookTarget )
		{
			AUnit* pTarget = m_pLookTarget->GetUnit();

			if ( pTarget )
			{
				TVector4 vecTarget = *pTarget->GetSteer()->GetPosition();
				vecTarget.y += GetHeadHeight( pTarget );

				UpdateLookTarget( pOwner, vecTarget );
			}
		}
	}

	TBOOL bYawReached   = TTRUE;
	TBOOL bPitchReached = TTRUE;

	if ( ( pAnimationSet->GetOverlayType() & OVERLAYTYPE_LOOK_YAW ) == OVERLAYTYPE_LOOK_YAW )
		bYawReached = UpdateLookAnim( pAnimatableObject, m_fLookYaw, m_fLookYawTarget, TPS8( lookl ), TPS8( lookr ), m_fLookSpeed, a_fDeltaTime );

	if ( ( pAnimationSet->GetOverlayType() & OVERLAYTYPE_LOOK_PITCH ) == OVERLAYTYPE_LOOK_PITCH )
		bPitchReached = UpdateLookAnim( pAnimatableObject, m_fLookPitch, m_fLookPitchTarget, TPS8( lookd ), TPS8( looku ), m_fLookSpeed, a_fDeltaTime );

	if ( m_uiFlags & FLAGS_STOPPING )
	{
		if ( bYawReached && bPitchReached )
		{
			// The head is back in the centre
			m_uiFlags &= ~( FLAGS_STOPPING | FLAGS_LOOKING );

			pAnimatableObject->FinishAnimation( TPS8( lookl ) );
			pAnimatableObject->FinishAnimation( TPS8( lookr ) );
			pAnimatableObject->FinishAnimation( TPS8( looku ) );
			pAnimatableObject->FinishAnimation( TPS8( lookd ) );
		}

		return;
	}

	if ( m_uiFlags & FLAGS_TIMED )
	{
		m_fLookDuration -= a_fDeltaTime;

		if ( m_fLookDuration <= 0.0f && ( m_uiFlags & FLAGS_LOOKING ) )
			SetLooking( TFALSE );
	}
}

// $Barnyard: FUNCTION 00594d50
TBOOL AUnitFacialComponent::SetLooking( TBOOL a_bLook )
{
	m_uiFlags &= ( FLAGS_BLINK | FLAGS_LOOKING | FLAGS_STOPPING );
	m_pLookTarget   = TNULL;
	m_vLookTarget   = TVector4::VEC_ZERO;
	m_fLookDuration = 0.0f;
	m_fLookTimer    = 0.0f;
	m_fLookSpeed    = m_pData->m_fLookSpeed;

	TBOOL bLooking = ( m_uiFlags & FLAGS_LOOKING ) != 0;

	if ( bLooking == a_bLook )
	{
		// Starts looking again while it's turning the head back
		if ( a_bLook )
			m_uiFlags &= ~FLAGS_STOPPING;
	}
	else if ( bLooking )
	{
		// Turn the head back first
		m_fLookYawTarget   = 0.0f;
		m_fLookPitchTarget = 0.0f;
		m_uiFlags |= FLAGS_STOPPING;
	}
	else
	{
		m_uiFlags |= FLAGS_LOOKING;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00594110
void AUnitFacialComponent::UpdateLookTarget( AUnit* a_pOwner, const TVector4& a_rcTarget )
{
	TVector4 vecHead = *a_pOwner->GetSteer()->GetPosition();
	vecHead.y += m_fHeadHeight;

	TVector4 vecDirection;
	a_pOwner->GetSteer()->CopyDirection( vecDirection );

	TVector4 vecToTarget( a_rcTarget.x - vecHead.x, a_rcTarget.y - vecHead.y, a_rcTarget.z - vecHead.z, a_rcTarget.w );

	TVector4 vecToTargetXZ( vecToTarget.x, 0.0f, vecToTarget.z, 1.0f );
	vecToTargetXZ.Normalise();

	TVector4 vecDirectionXZ( vecDirection.x, 0.0f, vecDirection.z, 1.0f );
	vecDirectionXZ.Normalise();

	TFLOAT fDot = TVector4::DotProduct3( vecDirectionXZ, vecToTargetXZ );
	TMath::Clip( fDot, -1.0f, 1.0f );

	TFLOAT fAngle = TMath::ACos( fDot );
	TMath::Clip( fAngle, -m_pData->m_fLookLeftLimit, m_pData->m_fLookRightLimit );

	vecToTarget.Normalise();

	if ( fDot < -0.707f )
	{
		// The target is behind the unit
		m_fLookYawTarget   = 0.0f;
		m_fLookPitchTarget = 0.0f;
		return;
	}

	if ( fDot < -0.998f || TMath::Abs( vecToTarget.y ) > 0.9f )
	{
		if ( fDot >= 0.0f )
			m_fLookYawTarget = fAngle / m_pData->m_fLookRightLimit;
		else
			m_fLookYawTarget = -1.0f / m_pData->m_fLookLeftLimit * fAngle;
	}
	else
	{
		TVector4 vecCross;
		vecCross.CrossProduct( vecToTargetXZ, vecDirectionXZ );

		// NOTE: retail uses the left limit for both sides here
		if ( vecCross.y >= 0.0f )
			m_fLookYawTarget = -1.0f / m_pData->m_fLookLeftLimit * fAngle;
		else
			m_fLookYawTarget = fAngle / m_pData->m_fLookLeftLimit;
	}

	TFLOAT fPitch = TMath::ASin( -vecToTarget.y );
	TMath::Clip( fPitch, -m_pData->m_fLookDownLimit, m_pData->m_fLookUpLimit );

	m_fLookPitchTarget = fPitch / ( ( fPitch >= 0.0f ) ? m_pData->m_fLookUpLimit : m_pData->m_fLookDownLimit );
}

// $Barnyard: FUNCTION 00594390
TBOOL AUnitFacialComponent::UpdateLookAnim( AAnimatableObject* a_pAnimatableObject, TFLOAT& a_rfCurrent, const TFLOAT& a_rfTarget, const TPString8& a_rcNegativeAnim, const TPString8& a_rcPositiveAnim, TFLOAT a_fSpeed, TFLOAT a_fDeltaTime )
{
	TFLOAT fSpeed   = a_fSpeed * a_fDeltaTime;
	TFLOAT fMinStep = a_fDeltaTime * 0.1f;
	TBOOL  bReached = TFALSE;

	if ( a_rfCurrent > a_rfTarget )
	{
		TFLOAT fStep = fSpeed * ( a_rfCurrent - a_rfTarget );

		if ( fStep < fMinStep )
			fStep = fMinStep;

		a_rfCurrent -= fStep;

		if ( a_rfCurrent < a_rfTarget )
			a_rfCurrent = a_rfTarget;
	}
	else if ( a_rfCurrent >= a_rfTarget )
	{
		bReached = TTRUE;
	}
	else
	{
		TFLOAT fStep = fSpeed * ( a_rfTarget - a_rfCurrent );

		if ( fStep < fMinStep )
			fStep = fMinStep;

		a_rfCurrent += fStep;

		if ( a_rfCurrent > a_rfTarget )
			a_rfCurrent = a_rfTarget;
	}

	if ( a_rfCurrent > 0.0f )
	{
		TFLOAT fWeight = a_rfCurrent;
		a_pAnimatableObject->PlayAnimation( a_rcPositiveAnim );
		a_pAnimatableObject->SetAnimWeight( a_rcPositiveAnim, fWeight );
		a_pAnimatableObject->FinishAnimation( a_rcNegativeAnim );
	}
	else
	{
		TFLOAT fWeight = -a_rfCurrent;
		a_pAnimatableObject->PlayAnimation( a_rcNegativeAnim );
		a_pAnimatableObject->SetAnimWeight( a_rcNegativeAnim, fWeight );
		a_pAnimatableObject->FinishAnimation( a_rcPositiveAnim );
	}

	return bReached;
}

// $Barnyard: FUNCTION 005944a0
TBOOL AUnitFacialComponent::GetHeadPosition( AUnit* a_pUnit, TVector4& a_rOutPosition )
{
	if ( !a_pUnit || !a_pUnit->IsA( &TGetClass( AVisibleUnit ) ) )
	{
		a_rOutPosition = TVector4::VEC_ZERO;
		return TFALSE;
	}

	AModelInstance* pModelInstance = TSTATICCAST( AVisibleUnit, a_pUnit )->GetModelInstance();

	if ( !pModelInstance )
	{
		a_rOutPosition = TVector4::VEC_ZERO;
		return TFALSE;
	}

	AUnitFacialComponentData* pData = TSTATICCAST( AUnitFacialComponentData, a_pUnit->GetType()->FindStaticData( &TGetClass( AUnitFacialComponentData ) ) );
	TINT                      iBone = pModelInstance->GetBoneID( pData ? pData->GetBoneName() : TPS8( c_head ) );

	if ( iBone == -1 )
	{
		a_rOutPosition = TVector4::VEC_ZERO;
		return TFALSE;
	}

	TMatrix44 matBone;
	pModelInstance->GetScaledBoneTransform( iBone, matBone );

	matBone.PreMultiply( s_matBoneToUnit );

	a_rOutPosition = matBone.GetTranslation();
	return TTRUE;
}

// $Barnyard: FUNCTION 00594d10
TFLOAT AUnitFacialComponent::GetHeadHeight( AUnit* a_pUnit )
{
	TVector4 vecHead;
	GetHeadPosition( a_pUnit, vecHead );

	return vecHead.y;
}

// $Barnyard: FUNCTION 005945e0
AUnit* AUnitFacialComponent::FindLookTarget( const TVector4& a_rcPosition, TFLOAT a_fRadius, TFLOAT a_fMinDot )
{
	TINT   iBest       = -1;
	TINT   iFound      = -1;
	TFLOAT fBestDistSq = TMath::MAXFLOAT;

	AUnit*          pOwner = GetOwner();
	const TVector4* pPos   = pOwner->GetSteer()->GetPosition();

	TVector4 vecDirection;
	pOwner->GetSteer()->CopyDirection( vecDirection );

	TINT                    iNumUnits = 0;
	AUnitHash::QueryResult* pUnits    = AUnitManager::GetSingleton()->FindUnits( *pPos, a_fRadius, &iNumUnits, TFALSE, 420, TFALSE, TNULL );

	for ( TINT i = 0; i < iNumUnits; i++ )
	{
		AUnit* pUnit = pUnits[ i ].pUnit;

		if ( pUnit == pOwner )
			continue;

		TVector4 vecToUnit = *pUnit->GetSteer()->GetPosition();
		vecToUnit.x -= pPos->x;
		vecToUnit.y -= pPos->y;
		vecToUnit.z -= pPos->z;
		vecToUnit.Normalise();

		if ( TVector4::DotProduct3( vecDirection, vecToUnit ) > a_fMinDot )
		{
			TFLOAT fDistSq = TVector4::DistanceSq( *pUnit->GetSteer()->GetPosition(), *pPos );

			// NOTE: retail never updates fBestDistSq, so the last unit in front of the owner is picked
			if ( iFound == -1 || fDistSq < fBestDistSq )
				iFound = i;
		}
	}

	iBest = iFound;

	AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayer( 0 );

	if ( pPlayer && pPlayer != pOwner && a_fRadius * a_fRadius > TVector4::DistanceSq( *pPos, *pPlayer->GetSteer()->GetPosition() ) )
	{
		TVector4 vecToPlayer = *pPlayer->GetSteer()->GetPosition();
		vecToPlayer.x -= pPos->x;
		vecToPlayer.y -= pPos->y;
		vecToPlayer.z -= pPos->z;
		vecToPlayer.Normalise();

		if ( TVector4::DotProduct3( vecDirection, vecToPlayer ) > a_fMinDot && TVector4::DistanceSq( *pPlayer->GetSteer()->GetPosition(), *pPos ) < fBestDistSq )
			return pPlayer;
	}

	if ( iBest == -1 )
		return TNULL;

	return pUnits[ iBest ].pUnit;
}

// $Barnyard: FUNCTION 00594fd0
void AUnitFacialComponent::LookAtUnit( AUnit* a_pUnit, const TVector4& a_rcOffset, TFLOAT a_fDuration )
{
	if ( !a_pUnit || !SetLooking( TTRUE ) )
		return;

	m_uiFlags |= FLAGS_LOOK_AT_UNIT;
	m_fLookDuration = a_fDuration;
	m_pLookTarget   = a_pUnit->GetSelfProxy();
	m_vLookTarget   = a_rcOffset;

	if ( a_fDuration >= 0.0f )
		m_uiFlags |= FLAGS_TIMED;
}

// $Barnyard: FUNCTION 00595810
void AUnitFacialComponent::LookAtUnitHead( AUnit* a_pUnit, TFLOAT a_fDuration )
{
	if ( a_pUnit )
	{
		TVector4 vecOffset = TVector4::VEC_ZERO;

		TVector4 vecHead;
		GetHeadPosition( a_pUnit, vecHead );
		vecOffset.y = vecHead.y;

		LookAtUnit( a_pUnit, vecOffset, a_fDuration );
	}
}

// $Barnyard: FUNCTION 005950c0
void AUnitFacialComponent::LookAtPosition( const TVector4& a_rcPosition, TFLOAT a_fDuration )
{
	if ( SetLooking( TTRUE ) )
	{
		m_uiFlags |= FLAGS_LOOK_AT_POSITION;
		m_fLookDuration = a_fDuration;
		m_vLookTarget   = a_rcPosition;

		if ( a_fDuration >= 0.0f )
			m_uiFlags |= FLAGS_TIMED;
	}
}

// $Barnyard: FUNCTION 00595150
void AUnitFacialComponent::SetLookRandom( TFLOAT a_fDuration )
{
	if ( SetLooking( TTRUE ) )
	{
		m_uiFlags |= FLAGS_LOOK_RANDOM;
		m_fLookDuration = a_fDuration;

		if ( a_fDuration >= 0.0f )
			m_uiFlags |= FLAGS_TIMED;
	}
}

// $Barnyard: FUNCTION 00595110
void AUnitFacialComponent::SetLookTargetRandom( TFLOAT a_fDuration )
{
	if ( SetLooking( TTRUE ) )
	{
		m_uiFlags |= FLAGS_LOOK_TARGET_RANDOM;
		m_fLookDuration = a_fDuration;

		if ( a_fDuration >= 0.0f )
			m_uiFlags |= FLAGS_TIMED;
	}
}

// $Barnyard: FUNCTION 00595190
void AUnitFacialComponent::StopLook()
{
	if ( m_uiFlags & FLAGS_LOOKING )
		SetLooking( TFALSE );
}

// $Barnyard: FUNCTION 00594d30
void AUnitFacialComponent::SetBlink( TBOOL a_bBlink )
{
	if ( a_bBlink )
		m_uiFlags |= FLAGS_BLINK;
	else
		m_uiFlags &= ~FLAGS_BLINK;

	m_fBlinkTimer = 0.0f;
}

// $Barnyard: FUNCTION 00595780
void AUnitFacialComponent::SetLookTargetByName( const TPString8& a_rcName )
{
	AUnit* pUnit = AUnitNameManager::GetSingleton()->RetrieveByName( a_rcName );

	if ( pUnit )
	{
		TVector4 vecOffset = TVector4::VEC_ZERO;

		TVector4 vecHead;
		GetHeadPosition( pUnit, vecHead );
		vecOffset.y = vecHead.y;

		LookAtUnit( pUnit, vecOffset, -1.0f );
	}
}

// $Barnyard: FUNCTION 00594e90
AUnitFacialComponentData::AUnitFacialComponentData()
    : m_fLookUpLimit( TMath::HALF_PI * 0.5f )
    , m_fLookDownLimit( TMath::HALF_PI * 0.5f )
    , m_fLookLeftLimit( TMath::HALF_PI )
    , m_fLookRightLimit( TMath::HALF_PI )
    , m_fLookSpeed( 5.0f )
    , m_strBoneName( TPS8( c_head ) )
{
}

// $Barnyard: FUNCTION 005956a0
// $Barnyard: FUNCTION 00595680
AUnitFacialComponentData::~AUnitFacialComponentData()
{
}

// $Barnyard: FUNCTION 00594080
TBOOL AUnitFacialComponentData::Create( const PBProperties* a_pProperties )
{
	// NOTE: retail uses the TPString8 overload at 006b1f50, same as GetOptionalFloatOrIntValue
	a_pProperties->GetOptionalFloatOrIntValue( m_fLookUpLimit, TPS8( LookUpLimit ) );
	a_pProperties->GetOptionalFloatOrIntValue( m_fLookDownLimit, TPS8( LookDownLimit ) );
	a_pProperties->GetOptionalFloatOrIntValue( m_fLookLeftLimit, TPS8( LookLeftLimit ) );
	a_pProperties->GetOptionalFloatOrIntValue( m_fLookRightLimit, TPS8( LookRightLimit ) );
	a_pProperties->GetOptionalFloatOrIntValue( m_fLookSpeed, TPS8( LookSpeed ) );
	a_pProperties->GetOptionalPropertyValue( m_strBoneName, TPS8( BoneName ) );

	return AUnitStaticData::Create( a_pProperties );
}

// $Barnyard: FUNCTION 00595710
void AUnitFacialComponentData::OnUnitCreate( AUnit* a_pUnit )
{
	AUnitFacialComponent* pComponent = new AUnitFacialComponent();
	pComponent->Create( a_pUnit, this );

	a_pUnit->AddComponent( pComponent );
}
