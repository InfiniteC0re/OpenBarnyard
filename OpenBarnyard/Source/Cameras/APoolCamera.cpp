#include "pch.h"
#include "APoolCamera.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( APoolCamera );

// $Barnyard: FUNCTION 00465ec0
APoolCamera::APoolCamera()
    : m_vPosition( TVector4::VEC_ZERO )
    , m_vLookAt( TVector4::VEC_POSX )
    , m_fPositionSpeed( 2.5f )
    , m_fLookAtSpeed( 2.5f )
    , m_iCurrentView( -1 )
    , m_iNumViews( 0 )
    , m_bMoving( TTRUE )
{
}

// $Barnyard: FUNCTION 00465eb0
// $Barnyard: FUNCTION 00465f50
APoolCamera::~APoolCamera()
{
}

// $Barnyard: FUNCTION 004658f0
TBOOL APoolCamera::OnUpdate( TFLOAT a_fDeltaTime )
{
	m_bMoving = TFALSE;

	static TFLOAT s_fRotationSpeed = TMath::PI;

	const TVector4& rcViewPosition = *m_apViewPositions[ m_iCurrentView ];
	const TVector4& rcViewLookAt   = *m_apViewLookAts[ m_iCurrentView ];

	// Direction of the view from its look at point
	TVector4 vecViewDirection( rcViewPosition.x - rcViewLookAt.x, rcViewPosition.y - rcViewLookAt.y, rcViewPosition.z - rcViewLookAt.z, rcViewPosition.w );
	vecViewDirection.Normalise();

	// Current direction and distance from the look at point of the view
	TVector4 vecDirection( m_vPosition.x - rcViewLookAt.x, m_vPosition.y - rcViewLookAt.y, m_vPosition.z - rcViewLookAt.z, m_vPosition.w );
	TFLOAT   fDistance = vecDirection.NormaliseMag();

	TFLOAT fPositionStep = a_fDeltaTime * m_fPositionSpeed;
	TFLOAT fLookAtStep   = a_fDeltaTime * m_fLookAtSpeed;

	// The goal keeps the current horizontal direction, at the horizontal distance and the height of the view
	TFLOAT   fViewDistanceXZ = TVector4::DistanceXZ( rcViewPosition, rcViewLookAt );
	TVector4 vecDirectionXZ( m_vPosition.x - rcViewLookAt.x, 0.0f, m_vPosition.z - rcViewLookAt.z, m_vPosition.w );
	vecDirectionXZ.Normalise();

	TVector4 vecOffsetXZ;
	vecOffsetXZ.Multiply( vecDirectionXZ, fViewDistanceXZ );

	TVector4 vecGoal( vecOffsetXZ.x + rcViewLookAt.x, rcViewPosition.y, vecOffsetXZ.z + rcViewLookAt.z, rcViewLookAt.w );

	TVector4 vecToGoal( vecGoal.x - m_vPosition.x, vecGoal.y - m_vPosition.y, vecGoal.z - m_vPosition.z, vecGoal.w );
	TFLOAT   fDistanceToGoal = vecToGoal.NormaliseMag();

	if ( fDistanceToGoal <= 0.001f )
	{
		m_vPosition = vecGoal;
	}
	else
	{
		if ( fPositionStep <= fDistanceToGoal )
		{
			TVector4 vecStep;
			vecStep.Multiply( vecToGoal, fPositionStep );

			m_vPosition.x += vecStep.x;
			m_vPosition.y += vecStep.y;
			m_vPosition.z += vecStep.z;
		}
		else
		{
			m_vPosition = vecGoal;
		}

		vecDirection.Set( m_vPosition.x - rcViewLookAt.x, m_vPosition.y - rcViewLookAt.y, m_vPosition.z - rcViewLookAt.z, m_vPosition.w );
		fDistance = vecDirection.NormaliseMag();
		m_bMoving = TTRUE;
	}

	// Move the look at point
	if ( !m_vLookAt.IsEqual( rcViewLookAt ) )
	{
		TVector4 vecToLookAt( rcViewLookAt.x - m_vLookAt.x, rcViewLookAt.y - m_vLookAt.y, rcViewLookAt.z - m_vLookAt.z, rcViewLookAt.w );

		if ( vecToLookAt.NormaliseMag() >= fLookAtStep )
		{
			TVector4 vecStep;
			vecStep.Multiply( vecToLookAt, fLookAtStep );

			m_vLookAt.x += vecStep.x;
			m_vLookAt.y += vecStep.y;
			m_vLookAt.z += vecStep.z;
		}
		else
		{
			m_vLookAt = rcViewLookAt;
		}

		m_bMoving = TTRUE;
	}

	// Rotate around the look at point towards the horizontal direction of the view
	TVector4 vecViewDirectionXZ( vecViewDirection.x, 0.0f, vecViewDirection.z, vecViewDirection.w );
	TVector4 vecCurrentDirectionXZ( vecDirection.x, 0.0f, vecDirection.z, vecDirection.w );

	TFLOAT fAngle    = vecViewDirectionXZ.GetAngleBetweenVector( vecCurrentDirectionXZ );
	TFLOAT fRotation = s_fRotationSpeed * a_fDeltaTime;

	if ( fAngle > 0.001f )
	{
		if ( fRotation > fAngle )
			fRotation = fAngle;

		if ( vecDirection.z * vecViewDirection.x - vecViewDirection.z * vecDirection.x < 0.0f )
			vecDirection.AsVector3().RotateY( -fRotation );
		else
			vecDirection.AsVector3().RotateY( fRotation );

		TVector4 vecOffset;
		vecOffset.Multiply( vecDirection, fDistance );

		m_vPosition.Set( rcViewLookAt.x + vecOffset.x, rcViewLookAt.y + vecOffset.y, rcViewLookAt.z + vecOffset.z, rcViewLookAt.w );
		m_bMoving = TTRUE;
	}

	m_pCamera->LookAt( m_vPosition, m_vLookAt );
	return TTRUE;
}

// $Barnyard: FUNCTION 004657e0
void APoolCamera::AddView( const TVector4* a_pPosition, const TVector4* a_pLookAt, TINT a_iUserData )
{
	if ( m_iNumViews < MAX_NUM_VIEWS )
	{
		m_apViewPositions[ m_iNumViews ] = a_pPosition;
		m_apViewLookAts[ m_iNumViews ]   = a_pLookAt;
		m_aiViewUserData[ m_iNumViews ]  = a_iUserData;
		m_iNumViews++;

		if ( m_iCurrentView < 0 )
			m_iCurrentView = 0;
	}
}

// $Barnyard: FUNCTION 004658c0
void APoolCamera::ClearViews()
{
	m_iCurrentView = -1;
	m_iNumViews    = 0;
}

// $Barnyard: FUNCTION 00465840
void APoolCamera::SetView( TINT a_iView )
{
	if ( a_iView < m_iNumViews && a_iView >= 0 )
	{
		m_iCurrentView = a_iView;
		m_vPosition    = *m_apViewPositions[ a_iView ];
		m_vLookAt      = *m_apViewLookAts[ a_iView ];
	}
}

// $Barnyard: FUNCTION 004658a0
void APoolCamera::SetCurrentView( TINT a_iView )
{
	if ( a_iView < m_iNumViews && a_iView >= 0 )
		m_iCurrentView = a_iView;
}

// $Barnyard: FUNCTION 004658e0
TINT APoolCamera::GetCurrentView() const
{
	return m_iCurrentView;
}

// $Barnyard: FUNCTION 004657d0
TINT APoolCamera::GetCurrentViewUserData() const
{
	return m_aiViewUserData[ m_iCurrentView ];
}

// $Barnyard: FUNCTION 00465790
TVector4 APoolCamera::GetDirection()
{
	TVector4 vDirection;
	m_pCamera->GetDirection( vDirection );

	return vDirection;
}

// $Barnyard: FUNCTION 00465ea0
void APoolCamera::SetSpeed( TFLOAT a_fSpeed )
{
	m_fPositionSpeed = a_fSpeed;
	m_fLookAtSpeed   = a_fSpeed;
}
