#include "pch.h"
#include "ALerpCameraHelper.h"
#include "ACameraManager.h"
#include "Input/AInputMap.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( ALerpCameraHelper );

// $Barnyard: FUNCTION 004649d0
ALerpCameraHelper::ALerpCameraHelper()
    : m_vDestPosition( TVector4::VEC_ZERO )
    , m_vPosition( TVector4::VEC_ZERO )
    , m_vDestLookAt( TVector4::VEC_ZERO )
    , m_vLookAt( TVector4::VEC_ZERO )
    , m_uiUnk( 0 )
    , m_bReachedDestination( TFALSE )
{
}

// $Barnyard: FUNCTION 00464a90
// $Barnyard: FUNCTION 004649c0
ALerpCameraHelper::~ALerpCameraHelper()
{
}

// Moves a_rVector towards a_rcTarget, snapping when closer than 0.1. Returns TTRUE if snapped.
static TBOOL MoveTowards( TVector4& a_rVector, const TVector4& a_rcTarget, TFLOAT a_fDeltaTime )
{
	TVector4 vDelta;
	vDelta.x = a_rcTarget.x - a_rVector.x;
	vDelta.y = a_rcTarget.y - a_rVector.y;
	vDelta.z = a_rcTarget.z - a_rVector.z;
	vDelta.w = a_rcTarget.w;

	if ( vDelta.Magnitude() <= 0.1f )
	{
		a_rVector = a_rcTarget;
		return TTRUE;
	}

	vDelta.Multiply( 2.0f );
	vDelta.Multiply( a_fDeltaTime );

	a_rVector.x += vDelta.x;
	a_rVector.y += vDelta.y;
	a_rVector.z += vDelta.z;
	return TFALSE;
}

// $Barnyard: FUNCTION 00464790
TBOOL ALerpCameraHelper::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( ::MoveTowards( m_vPosition, m_vDestPosition, a_fDeltaTime ) )
		m_bReachedDestination = TTRUE;

	::MoveTowards( m_vLookAt, m_vDestLookAt, a_fDeltaTime );

	ACameraManager::GetSingleton()->GetCurrentCamera()->LookAt( m_vPosition, m_vLookAt );
	return TTRUE;
}

// $Barnyard: FUNCTION 00468d90
TBOOL ALerpCameraHelper::ProcessInputCommand( void* a_pCommand, void* a_pEvent )
{
	AInputCommand eCommand = AInputCommand( TUINTPTR( a_pCommand ) );

	if ( eCommand == AInputCommand_ZoomIn || eCommand == AInputCommand_ZoomOut )
	{
		TSTATICCAST( TInputInterface::InputEvent, a_pEvent )->StartRepeat( 0.1f, 0.1f );
		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00464700
void ALerpCameraHelper::SetPosition( const TVector4& a_rcPosition, const TVector4& a_rcLookAt )
{
	m_vPosition = a_rcPosition;
	m_vLookAt   = a_rcLookAt;
}

// $Barnyard: FUNCTION 00464740
void ALerpCameraHelper::SetDestination( const TVector4& a_rcPosition, const TVector4& a_rcLookAt )
{
	m_vDestPosition       = a_rcPosition;
	m_vDestLookAt         = a_rcLookAt;
	m_bReachedDestination = TFALSE;
}
