#include "pch.h"
#include "ASteerKinematics.h"
#include "ASteer.h"
#include "Units/AKinematicsObstructionManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( ASteerKinematics );

AKinematicsObstructionLists ASteerKinematics::s_oObstructionLists;

// $Barnyard: FUNCTION 0055f880
void AKinematicsObstructionLists::Add( AInteriorNetwork* a_pNetwork )
{
	if ( a_pNetwork )
	{
		List* pNetworkList = TREINTERPRETCAST( List*, &a_pNetwork->m_vecObstructions );

		if ( pNetworkList->iNumObstructions > 0 )
			apLists[ iNumLists++ ] = pNetworkList;

		List* pManagerList = TREINTERPRETCAST( List*, &AKinematicsObstructionManager::GetSingleton()->GetObstructions() );

		if ( pManagerList->iNumObstructions > 0 )
			apLists[ iNumLists++ ] = pManagerList;
	}
}

// $Barnyard: FUNCTION 0055f8c0
void AKinematicsObstructionLists::Clear()
{
	for ( TINT i = 0; i < iNumLists; i++ )
		apLists[ i ] = TNULL;

	iNumLists = 0;
}

// $Barnyard: FUNCTION 005747a0
ASteerKinematics::ASteerKinematics()
    : m_pSteer( TNULL )
    , m_pTraits( TNULL )
{
}

// $Barnyard: FUNCTION 00574860
// $Barnyard: FUNCTION 00574890
ASteerKinematics::~ASteerKinematics()
{
}

// $Barnyard: FUNCTION 00574440
TBOOL ASteerKinematics::Create( const TVector4& a_rcPosition, const TVector4& a_rcDirection, const ASteerTraits* a_pTraits, TFLOAT a_fSpeed )
{
	m_pTraits = a_pTraits;
	m_pSteer  = TNULL;

	return TTRUE;
}

// $Barnyard: FUNCTION 00574420
TBOOL ASteerKinematics::CreateFrom( ASteer* a_pOther )
{
	m_pSteer  = a_pOther;
	m_pTraits = a_pOther->GetTraits();

	return TTRUE;
}

// Folded with AKinematicsObstruction::GetPosition (005747d0)
const TVector4* ASteerKinematics::GetPosition()
{
	return &TVector4::VEC_ZERO;
}

// $Barnyard: FUNCTION 005747e0
const TVector4* ASteerKinematics::GetFacingDir()
{
	return &TVector4::VEC_POSX;
}

// NOTE: folded with GetFacingDir (005747e0)
const TVector4* ASteerKinematics::GetMovementDir()
{
	return &TVector4::VEC_POSX;
}

// $Barnyard: FUNCTION 005747f0
const TVector4* ASteerKinematics::GetSeekTarget()
{
	return GetPosition();
}

// $Barnyard: FUNCTION 00574800
void ASteerKinematics::CopyPosition( TVector4& a_rOut )
{
	a_rOut = *GetPosition();
}

// $Barnyard: FUNCTION 00574830
void ASteerKinematics::CopyFacingDir( TVector4& a_rOut )
{
	a_rOut = *GetFacingDir();
}

// $Barnyard: FUNCTION 00574780
TFLOAT ASteerKinematics::GetNHolonomicRate()
{
	return m_pTraits->m_fNHolonomicRate;
}

// $Barnyard: FUNCTION 00574490
TVector4 ASteerKinematics::GetSeekDirection( const TVector4& a_rcStart, const TVector4& a_rcTarget, const TVector4& a_rcPosition, const TVector4& a_rcFacing, TFLOAT* a_pOutDistance, TBOOL* a_pOutPassed, TBOOL a_bDirect )
{
	// The line to follow
	TVector4 vLine = a_rcTarget - a_rcStart;
	vLine.w        = a_rcTarget.w;

	TFLOAT fLineLength = vLine.AsVector3().Magnitude();
	vLine.Divide( fLineLength );

	TVector4 vToPosition = a_rcPosition - a_rcStart;
	vToPosition.w        = a_rcPosition.w;

	// Project the position onto the line
	TFLOAT   fProjection = TVector4::DotProduct3( vToPosition, vLine );
	TVector4 vOnLine;
	vOnLine.Multiply( vLine, fProjection );

	*a_pOutPassed = fProjection >= fLineLength;

	TVector4 vFromLine = vToPosition - vOnLine;
	vFromLine.w        = vToPosition.w;
	// NOTE: sqrtf instead of Magnitude (TMath::Sqrt asserts on zero, the position is often right on the line)
	*a_pOutDistance = sqrtf( vFromLine.AsVector3().MagnitudeSq() );

	TFLOAT fFacingDot = TVector4::DotProduct3( a_rcFacing, vLine );

	if ( *a_pOutDistance > 0.0f || fFacingDot <= 0.0f )
	{
		TVector4 vDirection;

		if ( a_bDirect )
		{
			vDirection.x = a_rcTarget.x - a_rcPosition.x;
			vDirection.y = 0.0f;
			vDirection.z = a_rcTarget.z - a_rcPosition.z;
			vDirection.w = 1.0f;
		}
		else
		{
			// Back to the line while moving along it
			TVector4 vBackToLine;
			vBackToLine.Multiply( vLine, fProjection );

			vDirection   = vBackToLine - vToPosition;
			vDirection.w = vBackToLine.w;
			vDirection.Lerp4( vDirection, vLine, 0.9f );
		}

		vDirection.Normalise();
		return vDirection;
	}

	return TVector4::VEC_ZERO;
}
