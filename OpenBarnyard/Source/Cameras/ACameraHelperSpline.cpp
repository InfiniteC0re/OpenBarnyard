#include "pch.h"
#include "ACameraHelperSpline.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0045fe50, CreateObject 004602a0, CreateObjectInPlace 00460300
TDEFINE_CLASS( ACameraHelperSpline );

// $Barnyard: FUNCTION 0045fe60
ACameraHelperSpline::ACameraHelperSpline()
    : m_vStartDirection( TVector4::VEC_ZERO )
    , m_vEndDirection( TVector4::VEC_ZERO )
    , m_pCamera( TNULL )
    , m_fProgress( -1.0f )
    , m_fSpeed( -1.0f )
{
}

// $Barnyard: FUNCTION 0045ff10
// $Barnyard: FUNCTION 00460340
ACameraHelperSpline::~ACameraHelperSpline()
{
}

// $Barnyard: FUNCTION 0045ff70
void ACameraHelperSpline::Setup( ACamera* a_pCamera, const TVector4& a_rcDestination, const TVector4& a_rcDirection, TFLOAT a_fTangentScale, TFLOAT a_fSpeed )
{
	TVector4 vStart = a_pCamera->GetMatrix().GetTranslation();
	TVector4 vStartDirection( a_pCamera->GetMatrix().AsBasisVector3( BASISVECTOR_FORWARD ) );

	TVector4 vStartTangent;
	vStartTangent.Multiply( vStartDirection, a_fTangentScale );

	TVector4 vEndTangent;
	vEndTangent.Multiply( a_rcDirection, a_fTangentScale );

	m_oSegment.Set( vStart, vStartTangent, a_rcDestination, vEndTangent );

	m_fSpeed          = a_fSpeed;
	m_vStartDirection = vStartDirection;
	m_pCamera         = a_pCamera;
	m_fProgress       = 0.0f;
	m_vEndDirection   = a_rcDirection;
}

// $Barnyard: FUNCTION 00460090
void ACameraHelperSpline::SetDestination( const TVector4& a_rcDestination, const TVector4& a_rcEndTangent, const TVector4& a_rcDirection, TFLOAT a_fMaxDistance )
{
	TVector4 vStart = m_pCamera->GetMatrix().GetTranslation();
	TVector4 vStartDirection( m_pCamera->GetMatrix().AsBasisVector3( BASISVECTOR_FORWARD ) );

	TVector3 vEndOffset(
	    a_rcDestination.x - m_oSegment.m_vEnd.x,
	    a_rcDestination.y - m_oSegment.m_vEnd.y,
	    a_rcDestination.z - m_oSegment.m_vEnd.z
	);

	if ( vEndOffset.Magnitude() <= a_fMaxDistance )
	{
		// The destination hasn't moved much, keep going and only update the end
		m_oSegment.SetEnd( a_rcDestination, a_rcEndTangent );
		m_vEndDirection = a_rcDirection;
	}
	else
	{
		// Restart from the current position of the camera
		m_oSegment.Set( vStart, vStartDirection, a_rcDestination, a_rcEndTangent );
		m_vStartDirection = vStartDirection;
		m_vEndDirection   = a_rcDirection;
		m_fProgress       = 0.0f;
	}
}

// $Barnyard: FUNCTION 004601d0
TBOOL ACameraHelperSpline::Update( TFLOAT a_fDeltaTime )
{
	m_fProgress += m_fSpeed * m_oSegment.m_fInvLength * a_fDeltaTime;
	TMath::Clip( m_fProgress, 0.0f, 1.0f );

	TVector4 vPosition = m_oSegment.GetPoint( TMath::EaseInOut( m_fProgress ) );

	TVector4 vDirection;
	vDirection.Lerp4( m_vStartDirection, m_vEndDirection, m_fProgress );
	vDirection.Normalise();

	m_pCamera->GetMatrix().GetTranslation() = vPosition;

	if ( vDirection.MagnitudeSq() > 0.01f )
		m_pCamera->LookAtDirection( vDirection );

	return m_fProgress < 1.0f;
}
