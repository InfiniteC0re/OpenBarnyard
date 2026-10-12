#include "pch.h"
#include "AVictoryCameraHelper.h"
#include "ACameraManager.h"
#include "Input/AInputMap.h"
#include "Player/APlayerManager.h"
#include "Units/AUnit.h"
#include "UnitMovement/Steering/ASteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( AVictoryCameraHelper );

static constexpr TFLOAT DEG2RAD = 0.017453292f;

// $Barnyard: FUNCTION 004690b0
AVictoryCameraHelper::AVictoryCameraHelper()
    : m_vUnk10( TVector4::VEC_POSX )
    , m_fSpeed( 5.0f )
    , m_fAngle( 0.0f )
{
}

// $Barnyard: FUNCTION 00468dd0
// $Barnyard: FUNCTION 00469100
AVictoryCameraHelper::~AVictoryCameraHelper()
{
}

// $Barnyard: FUNCTION 00468de0
TBOOL AVictoryCameraHelper::OnUpdate( TFLOAT a_fDeltaTime )
{
	m_fSpeed = 3.0f;

	if ( !m_pTarget->GetUnit() )
		return TFALSE;

	m_fAngle += a_fDeltaTime * 100.1f;

	if ( m_fAngle > 360.0f )
		m_fAngle = 0.0f;

	TVector4& rCameraPos    = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation();
	TVector4  vOldCameraPos = rCameraPos;

	ASteer* pSteer = m_pTarget->GetUnit()->GetSteer();

	TVector4 vTargetPos;
	TVector4 vTargetDir;
	pSteer->CopyPosition( vTargetPos );
	pSteer->CopyDirection( vTargetDir );

	vTargetDir.AsVector3().RotateY( m_fAngle * DEG2RAD );

	// Behind and above the target
	m_vPosition = vTargetPos;

	TVector4 vOffset;
	vOffset.Multiply( vTargetDir, -7.5f );

	m_vPosition.x += vOffset.x;
	m_vPosition.y += vOffset.y;
	m_vPosition.z += vOffset.z;
	m_vPosition.y -= 5.0f;

	TVector4 vNewCameraPos;

	if ( a_fDeltaTime * m_fSpeed >= 1.0f )
	{
		vNewCameraPos = m_vPosition;
	}
	else
	{
		TVector4 vMove;
		vMove.Multiply( m_vPosition - vOldCameraPos, a_fDeltaTime );
		vMove.Multiply( vMove, m_fSpeed );

		vNewCameraPos = vOldCameraPos + vMove;
	}

	vTargetPos.y -= 2.0f;

	TVector4 vDirection;
	vDirection.x = vTargetPos.x - vNewCameraPos.x;
	vDirection.y = vTargetPos.y - vNewCameraPos.y;
	vDirection.z = vTargetPos.z - vNewCameraPos.z;
	vDirection.w = vTargetPos.w;
	vDirection.Normalise();

	rCameraPos = vNewCameraPos;

	LookAtDirection( vDirection );
	return TTRUE;
}

// NOTE: shares 00468d90 with ALerpCameraHelper::ProcessInputCommand
TBOOL AVictoryCameraHelper::ProcessInputCommand( void* a_pCommand, void* a_pEvent )
{
	AInputCommand eCommand = AInputCommand( TUINTPTR( a_pCommand ) );

	if ( eCommand == AInputCommand_ZoomIn || eCommand == AInputCommand_ZoomOut )
	{
		TSTATICCAST( TInputInterface::InputEvent, a_pEvent )->StartRepeat( 0.1f, 0.1f );
		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00468dc0
void AVictoryCameraHelper::OnCreate()
{
	// NOTE: the result isn't used
	APlayerManager::GetSingleton()->GetPlayerUnit( 0 );
}
