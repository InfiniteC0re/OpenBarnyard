#include "pch.h"
#include "AWalkActivity.h"
#include "AUnit.h"
#include "ACharacterUnit.h"
#include "ACharacterMovementComponent.h"
#include "AInteractionSequence.h"
#include "AKinematicsObstructionActivityMessage.h"
#include "AWorldPositionActivityMessage.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "Terrain/ATerrainInterface.h"
#include "UnitMovement/Steering/ASteer.h"
#include "UnitMovement/Steering/ASteerTraits.h"

#include <Math/TMathInline.h>
#include <Toshi/TScheduler.h>

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005c2ed0, CreateObject 005c39a0, CreateObjectInPlace 005c3a40
TDEFINE_CLASS( AWalkActivity );

// 0x00775330
static const TCHAR* s_aWalkTypeNames[ AWalkActivity::TYPE_NUMOF ] = {
	"stop",
	"checkcentral",
	"central",
	"manual",
	"bipolar",
	"bipolarrun",
	"tocamera",
	"facetarget",
	"walkbehind",
};

// NOTE: inlined in CreateObject
AWalkActivity::AWalkActivity()
    : m_eType( TYPE_STOP )
    , m_fInitialForward( 0.0f )
    , m_fInitialGoalToleranceSq( 0.25f )
    , m_fGoalToleranceSq( 0.25f )
    , m_fMinRadius( 0.85f )
    , m_fMaxRadius( 0.85f )
    , m_eTarget( AInteractionSequence::CATEGORY_PRIMARY )
    , m_uiFlags( FLAGS_SNAP_POSITION_TO_GOAL )
{
}

// $Barnyard: FUNCTION 005c2ee0
// $Barnyard: FUNCTION 005c2ef0
AWalkActivity::~AWalkActivity()
{
}

// $Barnyard: FUNCTION 005c3ad0
TBOOL AWalkActivity::Create( const PBProperties* a_pProperties )
{
	// Anything that isn't a float is read as an integer
	auto fnGetFloat = []( const PBPropertyValue* a_pValue ) -> TFLOAT {
		if ( a_pValue->GetType() == PBPropertyValue::Type::Float )
			return a_pValue->GetFloat();

		return TFLOAT( a_pValue->GetInteger() );
	};

	const PBPropertyValue* pType = a_pProperties->GetProperty( "type" );

	if ( pType )
	{
		TPString8 strType = pType->GetTPString8();

		for ( TINT i = 0; i < TYPE_NUMOF; i++ )
		{
			if ( TStringManager::String8CompareNoCase( strType.GetString(), s_aWalkTypeNames[ i ] ) == 0 )
			{
				m_eType = i;
				break;
			}
		}
	}

	if ( m_eType == TYPE_CENTRAL || m_eType == TYPE_CHECKCENTRAL )
		m_fGoalToleranceSq = 0.25f;

	const PBPropertyValue* pInitialForward = a_pProperties->GetProperty( "initialforward" );

	if ( pInitialForward )
		m_fInitialForward = fnGetFloat( pInitialForward );

	const PBPropertyValue* pInitialGoalTolerance = a_pProperties->GetProperty( "initialgoaltolerance" );

	if ( pInitialGoalTolerance )
	{
		TFLOAT fTolerance         = fnGetFloat( pInitialGoalTolerance );
		m_fInitialGoalToleranceSq = fTolerance * fTolerance;
	}

	const PBPropertyValue* pGoalTolerance = a_pProperties->GetProperty( "goaltolerance" );

	if ( pGoalTolerance )
	{
		TFLOAT fTolerance  = fnGetFloat( pGoalTolerance );
		m_fGoalToleranceSq = fTolerance * fTolerance;
	}

	const PBPropertyValue* pSnapPositionToGoal = a_pProperties->GetProperty( "snappositiontogoal" );

	if ( pSnapPositionToGoal )
	{
		if ( pSnapPositionToGoal->GetRaw() )
			m_uiFlags |= FLAGS_SNAP_POSITION_TO_GOAL;
		else
			m_uiFlags &= ~FLAGS_SNAP_POSITION_TO_GOAL;
	}

	const PBPropertyValue* pRadius = a_pProperties->GetProperty( "radius" );

	if ( pRadius )
	{
		m_fMinRadius = fnGetFloat( pRadius );
		m_fMaxRadius = fnGetFloat( pRadius );
	}

	// NOTE: retail calls the TPString8 overload (006b1f50)
	a_pProperties->GetOptionalFloatOrIntValue( m_fMinRadius, TPS8D( "minradius" ) );
	a_pProperties->GetOptionalFloatOrIntValue( m_fMaxRadius, TPS8D( "maxradius" ) );

	TBOOL bIgnoreInvalidPosition = TFALSE;
	a_pProperties->GetOptionalPropertyValue( bIgnoreInvalidPosition, TPS8D( "ignoreinvalidposition" ) );

	if ( bIgnoreInvalidPosition )
		m_uiFlags |= FLAGS_IGNORE_INVALID_POSITION;
	else
		m_uiFlags &= ~FLAGS_IGNORE_INVALID_POSITION;

	const PBPropertyValue* pFaceTarget = a_pProperties->GetProperty( "facetarget" );

	if ( pFaceTarget )
		m_eTarget = AInteractionSequence::GetCategoryFromName( pFaceTarget->GetTPString8().GetString() );

	const PBPropertyValue* pTarget = a_pProperties->GetProperty( "target" );

	if ( pTarget )
		m_eTarget = AInteractionSequence::GetCategoryFromName( pTarget->GetTPString8().GetString() );

	const PBPropertyValue* pNoFail = a_pProperties->GetProperty( "nofail" );

	if ( pNoFail )
	{
		if ( pNoFail->GetRaw() )
			m_uiFlags |= FLAGS_NO_FAIL;
		else
			m_uiFlags &= ~FLAGS_NO_FAIL;
	}

	// NOTE: retail looks the property up and ignores it
	a_pProperties->GetProperty( "scnofail" );

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005c2f10
void AWalkActivity::GetDataSize( TINT* a_pSize )
{
	*a_pSize = 0x2c;
}

// $Barnyard: FUNCTION 005c30d0
TVector3& AWalkActivity::GetCenter( TVector3& a_rOutCenter, AInteractionSequence* a_pSequence )
{
	TVector4 vSum      = TVector4::VEC_ZERO;
	TINT     iNumUnits = 0;

	for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
	{
		AUnit* pUnit = it->GetOwner();

		if ( pUnit )
		{
			const TVector4* pPosition = pUnit->GetSteer()->GetPosition();
			iNumUnits++;
			vSum.x += pPosition->x;
			vSum.y += pPosition->y;
			vSum.z += pPosition->z;
		}
	}

	TFLOAT fOneOverNum = 1.0f / iNumUnits;
	a_rOutCenter.x     = vSum.x * fOneOverNum;
	a_rOutCenter.y     = vSum.y * fOneOverNum;
	a_rOutCenter.z     = vSum.z * fOneOverNum;

	return a_rOutCenter;
}

// $Barnyard: FUNCTION 005c31e0
AWalkActivity::Slot* AWalkActivity::FindClosestSlot( const TVector4& a_rcDirection, Slot* a_pSlots, TINT a_iNumSlots )
{
	TFLOAT fClosestDistSq = TMath::MAXFLOAT;
	Slot*  pClosest       = TNULL;

	for ( TINT i = 0; i < a_iNumSlots; i++ )
	{
		TFLOAT fDZ     = a_pSlots[ i ].vDirection.z - a_rcDirection.z;
		TFLOAT fDX     = a_pSlots[ i ].vDirection.x - a_rcDirection.x;
		TFLOAT fDistSq = fDZ * fDZ + fDX * fDX;

		if ( fDistSq < fClosestDistSq )
		{
			pClosest       = &a_pSlots[ i ];
			fClosestDistSq = fDistSq;
		}
	}

	return pClosest;
}

// $Barnyard: FUNCTION 005c3340
TVector3& AWalkActivity::TurnAwayFromSlot( TVector3& a_rOut, const Slot* a_pSlot, TFLOAT a_fRadius, TFLOAT a_fMinRadius, const TVector4& a_rcDirection )
{
	TFLOAT fSin   = ( a_fRadius + a_pSlot->fRadius ) * 0.5f / a_fMinRadius;
	TFLOAT fAngle = TMath::ASin( ( fSin >= 1.0f ) ? 1.0f : fSin ) * 2.0f;

	TVector4 vSlot( a_pSlot->vDirection.x, a_pSlot->vDirection.y, a_pSlot->vDirection.z, 1.0f );
	vSlot.Normalise();

	TVector3 vLeft  = vSlot.AsVector3();
	TVector3 vRight = vSlot.AsVector3();
	vLeft.RotateY( -fAngle );
	vRight.RotateY( fAngle );

	TFLOAT fLeftDX  = a_rcDirection.x - vLeft.x;
	TFLOAT fLeftDZ  = a_rcDirection.z - vLeft.z;
	TFLOAT fRightDX = a_rcDirection.x - vRight.x;
	TFLOAT fRightDZ = a_rcDirection.z - vRight.z;
	TFLOAT fLength  = a_rcDirection.Magnitude();

	// The side that is closer to a_rcDirection
	if ( fRightDZ * fRightDZ + fRightDX * fRightDX <= fLeftDZ * fLeftDZ + fLeftDX * fLeftDX )
	{
		a_rOut.x = vRight.x * fLength;
		a_rOut.y = vRight.y * fLength;
		a_rOut.z = vRight.z * fLength;
	}
	else
	{
		a_rOut.x = vLeft.x * fLength;
		a_rOut.y = vLeft.y * fLength;
		a_rOut.z = vLeft.z * fLength;
	}

	return a_rOut;
}

// $Barnyard: FUNCTION 005c34a0
TBOOL AWalkActivity::IsNoUnitSeeking( AInteractionSequence* a_pSequence )
{
	for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
	{
		AUnit* pUnit = it->GetOwner();

		if ( pUnit && pUnit->GetSteer()->IsSeekingConstant() )
			return TFALSE;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005c2f20
TBOOL AWalkActivity::IsAnyUnitTurning( AInteractionSequence* a_pSequence )
{
	TBOOL bAllFacing = TTRUE;

	for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
	{
		AUnit* pUnit = it->GetOwner();

		if ( pUnit && !pUnit->GetSteer()->IsFacingAtDesiredFacing() )
			bAllFacing = TFALSE;
	}

	return !bAllFacing;
}

// $Barnyard: FUNCTION 005c2fa0
TBOOL AWalkActivity::IsAnyOtherUnitTurning( AInteractionSequence* a_pSequence )
{
	TBOOL  bTurning = TFALSE;
	AUnit* pTarget  = a_pSequence->GetUnit( m_eTarget );

	for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
	{
		AUnit* pUnit = it->GetOwner();

		if ( pUnit && pUnit != pTarget && !pUnit->GetSteer()->IsFacingAtDesiredFacing() )
			bTurning = TTRUE;
	}

	return bTurning;
}

// $Barnyard: FUNCTION 005c3030
TBOOL AWalkActivity::IsAnyOtherUnitSeeking( AInteractionSequence* a_pSequence )
{
	AUnit* pTarget = a_pSequence->GetUnit( m_eTarget );

	for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
	{
		AUnit* pUnit = it->GetOwner();

		if ( pUnit && pUnit != pTarget && pUnit->GetSteer()->IsSeekingConstant() == TTRUE )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005c3880
TBOOL AWalkActivity::SeekPosition( AUnit* a_pUnit, const TVector3& a_rcPosition, TFLOAT a_fTolerance )
{
	if ( !a_pUnit )
		return TTRUE;

	ASteer* pSteer = a_pUnit->GetSteer();
	pSteer->SetInteracting( 1 );
	pSteer->SetIgnoreInvalidPosition( ( m_uiFlags & FLAGS_IGNORE_INVALID_POSITION ) != 0 );

	TVector4 vTarget( a_rcPosition.x, a_rcPosition.y, a_rcPosition.z, 1.0f );
	return pSteer->Seek( vTarget, a_fTolerance );
}

// $Barnyard: FUNCTION 005c3900
TBOOL AWalkActivity::SeekPositionFacing( AUnit* a_pUnit, const TVector3& a_rcPosition, const TVector3& a_rcDirection, TFLOAT a_fUnused )
{
	if ( !a_pUnit )
		return TTRUE;

	ASteer* pSteer = a_pUnit->GetSteer();
	pSteer->SetInteracting( 1 );
	pSteer->SetIgnoreInvalidPosition( ( m_uiFlags & FLAGS_IGNORE_INVALID_POSITION ) != 0 );

	TVector4 vTarget( a_rcPosition.x, a_rcPosition.y, a_rcPosition.z, 1.0f );
	TVector4 vFacing( a_rcDirection.x, a_rcDirection.y, a_rcDirection.z, 1.0f );
	return pSteer->SeekFacing( vTarget, vFacing );
}

// $Barnyard: FUNCTION 005c41f0
TBOOL AWalkActivity::SeekAroundPosition( AInteractionSequence* a_pSequence, const TVector3& a_rcPosition, TFLOAT a_fTolerance )
{
	for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
	{
		AUnit*   pUnit   = it->GetOwner();
		TVector3 vTarget = a_rcPosition;

		// With more units each one stops on its own side of the position
		if ( a_pSequence->GetComponents().Size() > 1 )
		{
			const TVector4* pPosition = pUnit->GetSteer()->GetPosition();

			TVector4 vDir( pPosition->x - a_rcPosition.x, 0.0f, pPosition->z - a_rcPosition.z, 1.0f );
			vDir.Normalise();

			TFLOAT fRadius = pUnit->GetSteer()->GetTraits()->m_fRadius;
			vTarget.x      = vDir.x * fRadius * 1.2f + a_rcPosition.x;
			vTarget.y      = vDir.y * fRadius * 1.2f + a_rcPosition.y;
			vTarget.z      = vDir.z * fRadius * 1.2f + a_rcPosition.z;
		}

		if ( !SeekPosition( pUnit, vTarget, a_fTolerance ) )
			return TFALSE;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005c4350
TBOOL AWalkActivity::SeekPositionFacingAll( AInteractionSequence* a_pSequence, const TVector3& a_rcPosition, const TVector3& a_rcDirection, TFLOAT a_fUnused )
{
	for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
	{
		if ( !SeekPositionFacing( it->GetOwner(), a_rcPosition, a_rcDirection, a_fUnused ) )
			return TFALSE;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005c43e0
TBOOL AWalkActivity::CheckSpace( AInteractionSequence* a_pSequence, const TVector3& a_rcCenter, TFLOAT a_fMinRadius, TFLOAT a_fMaxRadius )
{
	if ( !( m_uiFlags & FLAGS_NO_FAIL ) )
	{
		// NOTE: retail finds the biggest radius of the units and doesn't use it
		TFLOAT fMaxUnitRadius = 0.0f;

		for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
		{
			AUnit* pUnit = it->GetOwner();

			if ( pUnit )
				fMaxUnitRadius = TMath::Max( fMaxUnitRadius, pUnit->GetSteer()->GetTraits()->m_fRadius );
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005c4480
TBOOL AWalkActivity::PlaceAroundCenter( AInteractionSequence* a_pSequence, const TVector3& a_rcCenter, TFLOAT a_fMinRadius, TFLOAT a_fMaxRadius )
{
	Slot aSlots[ MAX_NUM_SLOTS ];
	TINT iNumSlots = 0;

	if ( !CheckSpace( a_pSequence, a_rcCenter, a_fMinRadius, a_fMaxRadius ) )
		return TFALSE;

	for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
	{
		AUnit* pUnit = it->GetOwner();

		if ( !pUnit )
			continue;

		ASteer*         pSteer    = pUnit->GetSteer();
		const TVector4* pPosition = pSteer->GetPosition();

		// Direction from the unit to the middle
		TVector4 vDirection( a_rcCenter.x - pPosition->x, a_rcCenter.y - pPosition->y, a_rcCenter.z - pPosition->z, 1.0f );
		TFLOAT   fDistance = vDirection.Magnitude();
		TFLOAT   fRadius   = pSteer->GetTraits()->m_fRadius;

		if ( fDistance < 0.001f )
			vDirection = *pSteer->GetDirection();

		vDirection.y = 0.0f;
		vDirection.Normalise();

		TVector3 vInitialDirection = vDirection.AsVector3();

		// Turn away from the units placed already
		TINT iNumTries = 0;

		while ( TTRUE )
		{
			Slot* pClosest = FindClosestSlot( vDirection, aSlots, iNumSlots );

			if ( !pClosest )
				break;

			TBOOL    bOverlaps;
			TVector4 vClosest( pClosest->vDirection.x, pClosest->vDirection.y, pClosest->vDirection.z, 1.0f );

			if ( TVector4::DistanceXZ( vClosest, vDirection ) > fRadius + pClosest->fRadius )
			{
				bOverlaps = TFALSE;
			}
			else
			{
				TVector3 vTurned;
				TurnAwayFromSlot( vTurned, pClosest, fRadius, a_fMinRadius, vDirection );
				vDirection.x = vTurned.x;
				vDirection.y = vTurned.y;
				vDirection.z = vTurned.z;
				bOverlaps    = TTRUE;
			}

			if ( ++iNumTries > 5 )
			{
				if ( !( m_uiFlags & FLAGS_NO_FAIL ) )
					return TFALSE;

				vDirection.x = vInitialDirection.x;
				vDirection.y = vInitialDirection.y;
				vDirection.z = vInitialDirection.z;
				break;
			}

			if ( !bOverlaps )
				break;
		}

		// NOTE: retail doesn't check the number of slots
		aSlots[ iNumSlots ].vDirection = vDirection.AsVector3();
		aSlots[ iNumSlots ].fRadius    = fRadius;
		iNumSlots++;

		TMath::Clip( fDistance, a_fMinRadius, a_fMaxRadius );

		TVector4 vTarget( vDirection.x, vDirection.y, vDirection.z, 1.0f );
		vTarget.Multiply( -fDistance );
		vTarget.x += a_rcCenter.x;
		vTarget.y += a_rcCenter.y;
		vTarget.z += a_rcCenter.z;

		pSteer->SetInteracting( 1 );
		pSteer->SetIgnoreInvalidPosition( ( m_uiFlags & FLAGS_IGNORE_INVALID_POSITION ) != 0 );

		TVector4 vFacing( vDirection.x, vDirection.y, vDirection.z, 1.0f );

		if ( !pSteer->SeekFacing( vTarget, vFacing ) )
			return TFALSE;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005c4980
void AWalkActivity::CreateObstruction( AInteractionSequence* a_pSequence )
{
	if ( !a_pSequence->FindActivityMessage<AKinematicsObstructionActivityMessage>( TPString8() ) )
	{
		AKinematicsObstructionActivityMessage* pMessage = a_pSequence->GetActivityMessage<AKinematicsObstructionActivityMessage>( TPString8() );
		pMessage->CreateObstruction();
	}
}

// $Barnyard: FUNCTION 005c3520
AInteractionActivity::RESULT AWalkActivity::StartStop( AInteractionSequence* a_pSequence )
{
	for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
	{
		AUnit* pUnit = it->GetOwner();

		if ( pUnit )
		{
			ASteer* pSteer = pUnit->GetSteer();
			pSteer->SetInteracting( 1 );
			pSteer->SetIgnoreInvalidPosition( ( m_uiFlags & FLAGS_IGNORE_INVALID_POSITION ) != 0 );
			pSteer->Halt();
		}
	}

	return RESULT_RUNNING;
}

// $Barnyard: FUNCTION 005c49c0
AInteractionActivity::RESULT AWalkActivity::StartCheckCentral( AInteractionSequence* a_pSequence )
{
	TVector3 vCenter;
	return CheckSpace( a_pSequence, GetCenter( vCenter, a_pSequence ), m_fMinRadius, m_fMaxRadius ) ? RESULT_DONE : RESULT_FINISH_SEQUENCE;
}

// $Barnyard: FUNCTION 005c4a10
AInteractionActivity::RESULT AWalkActivity::StartCentral( AInteractionSequence* a_pSequence )
{
	TVector3 vCenter;
	return PlaceAroundCenter( a_pSequence, GetCenter( vCenter, a_pSequence ), m_fMinRadius, m_fMaxRadius ) ? RESULT_RUNNING : RESULT_FINISH_SEQUENCE;
}

// $Barnyard: FUNCTION 005c4ae0
AInteractionActivity::RESULT AWalkActivity::StartBipolarRun( AInteractionSequence* a_pSequence )
{
	TVector3 vCenter;

	if ( !PlaceAroundCenter( a_pSequence, GetCenter( vCenter, a_pSequence ), m_fMinRadius, m_fMaxRadius ) )
		return RESULT_FINISH_SEQUENCE;

	AUnit* pUnit = a_pSequence->GetUnit( 0 );

	if ( pUnit && pUnit->IsA( &TGetClass( ACharacterUnit ) ) == TTRUE )
		TSTATICCAST( ACharacterMovementComponent, pUnit->FindComponent( &TGetClass( ACharacterMovementComponent ) ) )->SetDesireRun();

	return RESULT_RUNNING;
}

// $Barnyard: FUNCTION 005c4850
AInteractionActivity::RESULT AWalkActivity::StartManual( AInteractionSequence* a_pSequence )
{
	Data* pData = TSTATICCAST( Data, a_pSequence->GetActivityData() );

	AWorldPositionActivityMessage* pMessage = a_pSequence->FindActivityMessage<AWorldPositionActivityMessage>( TPString8( "manualpoint" ) );

	pData->vPosition  = pMessage->GetPosition();
	pData->vDirection = pMessage->GetDirection();
	pData->bTurning   = TFALSE;

	TBOOL bStarted;

	if ( m_uiFlags & FLAGS_SNAP_POSITION_TO_GOAL )
		bStarted = SeekPositionFacingAll( a_pSequence, pData->vPosition, pData->vDirection, m_fGoalToleranceSq );
	else
		bStarted = SeekAroundPosition( a_pSequence, pData->vPosition, m_fGoalToleranceSq );

	return bStarted ? RESULT_RUNNING : RESULT_FINISH_SEQUENCE;
}

// $Barnyard: FUNCTION 005c35c0
AInteractionActivity::RESULT AWalkActivity::StartToCamera( AInteractionSequence* a_pSequence )
{
	TVector4 vCamera = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation();

	for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
	{
		AUnit* pUnit = it->GetOwner();

		if ( pUnit )
		{
			ASteer*         pSteer    = pUnit->GetSteer();
			const TVector4* pPosition = pSteer->GetPosition();

			TVector4 vFacing( vCamera.x - pPosition->x, 0.0f, vCamera.z - pPosition->z, vCamera.w );
			vFacing.Normalise();

			pSteer->SetInteracting( 1 );
			pSteer->SetIgnoreInvalidPosition( ( m_uiFlags & FLAGS_IGNORE_INVALID_POSITION ) != 0 );
			pSteer->Halt();
			pSteer->HaltFacingDesired( vFacing.AsVector3() );
		}
	}

	return RESULT_RUNNING;
}

// $Barnyard: FUNCTION 005c36f0
AInteractionActivity::RESULT AWalkActivity::StartFaceTarget( AInteractionSequence* a_pSequence )
{
	AUnit* pTarget = a_pSequence->GetUnit( m_eTarget );

	if ( !pTarget )
		return RESULT_DONE;

	// The point in front of the target
	ASteer*  pTargetSteer = pTarget->GetSteer();
	TVector4 vPoint       = *pTargetSteer->GetDirection();
	vPoint.x += pTargetSteer->GetPosition()->x;
	vPoint.y += pTargetSteer->GetPosition()->y;
	vPoint.z += pTargetSteer->GetPosition()->z;

	for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
	{
		AUnit* pUnit = it->GetOwner();

		if ( pUnit && pUnit != pTarget )
		{
			ASteer*         pSteer    = pUnit->GetSteer();
			const TVector4* pPosition = pSteer->GetPosition();

			TVector4 vFacing( vPoint.x - pPosition->x, 0.0f, vPoint.z - pPosition->z, vPoint.w );
			vFacing.Normalise();

			pSteer->SetInteracting( 1 );
			pSteer->SetIgnoreInvalidPosition( ( m_uiFlags & FLAGS_IGNORE_INVALID_POSITION ) != 0 );
			pSteer->Halt();
			pSteer->HaltFacingDesired( vFacing.AsVector3() );
		}
	}

	return RESULT_RUNNING;
}

// $Barnyard: FUNCTION 005c40b0
AInteractionActivity::RESULT AWalkActivity::StartWalkBehind( AInteractionSequence* a_pSequence )
{
	AUnit* pTarget = a_pSequence->GetUnit( m_eTarget );

	if ( !pTarget )
		return RESULT_DONE;

	// The point m_fMinRadius behind the target, on the ground
	ASteer*  pTargetSteer = pTarget->GetSteer();
	TVector4 vPoint       = *pTargetSteer->GetDirection();
	vPoint.Multiply( m_fMinRadius * -1.0f );
	vPoint.x += pTargetSteer->GetPosition()->x;
	vPoint.y += pTargetSteer->GetPosition()->y;
	vPoint.z += pTargetSteer->GetPosition()->z;

	// NOTE: ATerrainInterface::SetToGroundHeightTop inlined
	vPoint.y = ATerrainInterface::GetSingleton()->GetGroundHeightTop( vPoint.x, vPoint.z, 0 );

	for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
	{
		AUnit* pUnit = it->GetOwner();

		if ( pUnit && pUnit != pTarget )
		{
			pUnit->GetSteer()->SetIgnoreInvalidPosition( ( m_uiFlags & FLAGS_IGNORE_INVALID_POSITION ) != 0 );
			SeekPosition( pUnit, vPoint.AsVector3(), m_fGoalToleranceSq );
		}
	}

	return RESULT_RUNNING;
}

// $Barnyard: FUNCTION 005c4a60
AInteractionActivity::RESULT AWalkActivity::UpdateCentral( AInteractionSequence* a_pSequence )
{
	Data* pData = TSTATICCAST( Data, a_pSequence->GetActivityData() );

	if ( !IsNoUnitSeeking( a_pSequence ) && pData->fTimeLeft > 0.0f )
		return RESULT_RUNNING;

	CreateObstruction( a_pSequence );
	return RESULT_DONE;
}

// $Barnyard: FUNCTION 005c4ab0
AInteractionActivity::RESULT AWalkActivity::UpdateBipolar( AInteractionSequence* a_pSequence )
{
	if ( !IsNoUnitSeeking( a_pSequence ) )
		return RESULT_RUNNING;

	CreateObstruction( a_pSequence );
	return RESULT_DONE;
}

// $Barnyard: FUNCTION 005c3e40
AInteractionActivity::RESULT AWalkActivity::UpdateManual( AInteractionSequence* a_pSequence )
{
	Data* pData    = TSTATICCAST( Data, a_pSequence->GetActivityData() );
	TBOOL bStarted = TTRUE;
	TBOOL bDone    = TTRUE;

	auto it    = a_pSequence->GetCategoryBegin( m_eCategory );
	auto itEnd = a_pSequence->GetCategoryEnd( m_eCategory );

	if ( it == itEnd )
		return RESULT_DONE;

	for ( ; it != itEnd; it++ )
	{
		AUnit* pUnit = it->GetOwner();

		if ( !pUnit )
			continue;

		ASteer* pSteer = pUnit->GetSteer();

		if ( pSteer->IsSeekingConstant() )
		{
			bDone = TFALSE;
			continue;
		}

		TVector4 vPosition   = *pSteer->GetPosition();
		TBOOL    bWasTurning = TFALSE;

		if ( pData->bTurning == TTRUE )
		{
			// Turn to the point first
			bWasTurning = TTRUE;
			pSteer->Halt();

			TVector4 vFacing( pData->vPosition.x - vPosition.x, 0.0f, pData->vPosition.z - vPosition.z, 1.0f );

			if ( vFacing.z * vFacing.z + vFacing.x * vFacing.x <= 0.0f )
			{
				pData->bTurning = TFALSE;
			}
			else
			{
				vFacing.Normalise();
				pSteer->HaltFacingDesired( vFacing.AsVector3() );

				if ( pSteer->IsFacingAtDesiredFacing() )
					pData->bTurning = TFALSE;
			}

			bDone = TFALSE;
		}

		if ( pData->bTurning == TFALSE )
		{
			if ( m_uiFlags & FLAGS_SNAP_POSITION_TO_GOAL )
				bStarted &= SeekPositionFacing( pUnit, pData->vPosition, pData->vDirection, m_fGoalToleranceSq );
			else
				bStarted &= SeekPosition( pUnit, pData->vPosition, m_fGoalToleranceSq );
		}

		if ( !bWasTurning )
		{
			pSteer->Halt();

			if ( !( m_uiFlags & FLAGS_SNAP_POSITION_TO_GOAL ) )
			{
				TVector4 vFacing( pData->vPosition.x - vPosition.x, 0.0f, pData->vPosition.z - vPosition.z, 1.0f );

				if ( vFacing.z * vFacing.z + vFacing.x * vFacing.x > 0.0f )
				{
					vFacing.Normalise();
					pSteer->HaltFacingDesired( vFacing.AsVector3() );

					if ( !pSteer->IsFacingAtDesiredFacing() )
						bDone = TFALSE;
				}
			}
		}
	}

	if ( !bStarted )
		return RESULT_FINISH_SEQUENCE;

	return bDone ? RESULT_DONE : RESULT_RUNNING;
}

// $Barnyard: FUNCTION 005c4b80
AInteractionActivity::RESULT AWalkActivity::Update( AInteractionSequence* a_pSequence )
{
	Data*                      pData  = TSTATICCAST( Data, a_pSequence->GetActivityData() );
	AInteractionActivityState& rState = a_pSequence->GetActivityState();

	// NOTE: the sequence flag 4 turns TYPE_TOCAMERA into TYPE_STOP
	TYPE eType = m_eType;

	if ( eType == TYPE_TOCAMERA && ( a_pSequence->GetFlags() & AInteractionSequence::FLAGS_UNK2 ) )
		eType = TYPE_STOP;

	if ( !( rState.iFlags & AInteractionActivityState::FLAGS_INITIALISED ) )
	{
		pData->fTimeLeft = 2.0f;

		RESULT eResult = RESULT_FINISH_SEQUENCE;

		switch ( eType )
		{
			case TYPE_STOP: eResult = StartStop( a_pSequence ); break;
			case TYPE_CHECKCENTRAL: eResult = StartCheckCentral( a_pSequence ); break;
			case TYPE_CENTRAL:
			case TYPE_BIPOLAR: eResult = StartCentral( a_pSequence ); break;
			case TYPE_MANUAL: eResult = StartManual( a_pSequence ); break;
			case TYPE_BIPOLARRUN: eResult = StartBipolarRun( a_pSequence ); break;
			case TYPE_TOCAMERA: eResult = StartToCamera( a_pSequence ); break;
			case TYPE_FACETARGET: eResult = StartFaceTarget( a_pSequence ); break;
			case TYPE_WALKBEHIND: eResult = StartWalkBehind( a_pSequence ); break;
		}

		rState.iFlags |= AInteractionActivityState::FLAGS_INITIALISED;
		return eResult;
	}

	pData->fTimeLeft -= g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();

	if ( pData->fTimeLeft <= 0.0f )
		return RESULT_DONE;

	switch ( eType )
	{
		case TYPE_STOP:
			CreateObstruction( a_pSequence );
			return RESULT_DONE;
		case TYPE_CENTRAL: return UpdateCentral( a_pSequence );
		case TYPE_MANUAL: return UpdateManual( a_pSequence );
		case TYPE_BIPOLAR:
		case TYPE_BIPOLARRUN: return UpdateBipolar( a_pSequence );
		case TYPE_TOCAMERA: return IsAnyUnitTurning( a_pSequence ) ? RESULT_RUNNING : RESULT_DONE;
		case TYPE_FACETARGET: return IsAnyOtherUnitTurning( a_pSequence ) ? RESULT_RUNNING : RESULT_DONE;
		case TYPE_WALKBEHIND: return IsAnyOtherUnitSeeking( a_pSequence ) ? RESULT_RUNNING : RESULT_DONE;
		default: return RESULT_RUNNING;
	}
}
