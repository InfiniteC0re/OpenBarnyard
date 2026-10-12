#include "pch.h"
#include "AGolfCameraHelper.h"
#include "ACameraManager.h"
#include "Units/AUnit.h"
#include "UnitMovement/Steering/ASteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( AGolfCameraHelper );

// $Barnyard: FUNCTION 004629a0
AGolfCameraHelper::AGolfCameraHelper()
    : m_vLookAt( TVector4::VEC_ZERO )
    , m_fSpeed( 1.0f )
{
}

// $Barnyard: FUNCTION 00462780
// $Barnyard: FUNCTION 004629e0
AGolfCameraHelper::~AGolfCameraHelper()
{
}

// $Barnyard: FUNCTION 00462790
TBOOL AGolfCameraHelper::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( !m_pTarget || !m_pTarget->GetUnit() )
		return TFALSE;

	TVector4& rCameraPos    = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation();
	TVector4  vOldCameraPos = rCameraPos;

	ASteer* pSteer = m_pTarget->GetUnit()->GetSteer();

	TVector4 vTargetPos;
	TVector4 vTargetDir;
	pSteer->CopyPosition( vTargetPos );
	pSteer->CopyDirection( vTargetDir );

	// Behind and above the target
	TVector4 vOffset;
	vOffset.Multiply( vTargetDir, m_fDistance * -1.0f );

	m_vPosition = vTargetPos;
	m_vPosition.x += vOffset.x;
	m_vPosition.y += vOffset.y;
	m_vPosition.z += vOffset.z;
	m_vPosition.y -= m_fHeight;

	TVector4 vNewCameraPos;

	if ( a_fDeltaTime * m_fSpeed >= 1.0f )
	{
		vNewCameraPos = m_vPosition;
	}
	else
	{
		TVector4 vMove = m_vPosition - vOldCameraPos;
		vMove.Multiply( a_fDeltaTime );
		vMove.Multiply( m_fSpeed );

		vNewCameraPos = vOldCameraPos + vMove;
	}

	vTargetPos.y -= m_fLookAtHeight;

	TVector4 vDirection = vTargetPos - vNewCameraPos;
	vDirection.Normalise();

	m_vLookAt  = vTargetPos;
	rCameraPos = vNewCameraPos;

	LookAtDirection( vDirection );
	return TTRUE;
}

// $Barnyard: FUNCTION 00462750
void AGolfCameraHelper::OnCreate()
{
	m_fSpeed = 1.0f;
}

// $Barnyard: FUNCTION 00462760
void AGolfCameraHelper::OnAttach()
{
	m_fLookAtHeight = DEFAULT_LOOKAT_HEIGHT;
	m_fDistance     = DEFAULT_DISTANCE;
	m_fHeight       = DEFAULT_HEIGHT;
}
