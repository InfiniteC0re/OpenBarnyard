#include "pch.h"
#include "ATrackingCameraHelper.h"
#include "Units/AUnit.h"
#include "UnitMovement/Steering/ASteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( ATrackingCameraHelper );

// $Barnyard: FUNCTION 00468c10
ATrackingCameraHelper::ATrackingCameraHelper()
    : m_bSmooth( TFALSE )
    , m_vLookAt( TVector4::VEC_ZERO )
    , m_fSpeed( 1.0f )
    , m_fHeightOffset( 0.0f )
{
	m_pTarget = TNULL;
}

// $Barnyard: FUNCTION 00468cc0
// $Barnyard: FUNCTION 00468ca0
ATrackingCameraHelper::~ATrackingCameraHelper()
{
}

// $Barnyard: FUNCTION 00468b10
TBOOL ATrackingCameraHelper::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_pTarget && m_pTarget->GetUnit() )
	{
		ASteer* pSteer = m_pTarget->GetUnit()->GetSteer();

		TVector4 vTargetPos;
		pSteer->CopyPosition( vTargetPos );

		if ( m_bSmooth )
		{
			vTargetPos.y -= m_fHeightOffset;

			TVector4 vMove;
			vMove.x = vTargetPos.x - m_vLookAt.x;
			vMove.y = vTargetPos.y - m_vLookAt.y;
			vMove.z = vTargetPos.z - m_vLookAt.z;
			vMove.w = vTargetPos.w;
			vMove.Multiply( a_fDeltaTime );
			vMove.Multiply( m_fSpeed );

			m_vLookAt.x += vMove.x;
			m_vLookAt.y += vMove.y;
			m_vLookAt.z += vMove.z;

			m_pCamera->LookAtPoint( m_vLookAt );
			return TTRUE;
		}

		m_pCamera->LookAtPoint( vTargetPos );
	}

	return TTRUE;
}

// NOTE: shares 0045b680 with ACameraHelper::ResetTarget
void ATrackingCameraHelper::OnDestroy()
{
	m_pTarget = TNULL;
}

// $Barnyard: FUNCTION 00468cd0
void ATrackingCameraHelper::SetTarget( TRef<AUnitProxy> a_pTarget )
{
	if ( m_bSmooth )
	{
		ACameraHelper::SetTarget( a_pTarget );
	}
	else
	{
		ACameraHelper::SetTarget( a_pTarget );

		// Start looking at the target at once
		if ( m_pTarget )
		{
			m_pTarget->GetUnit()->GetSteer()->CopyPosition( m_vLookAt );
			m_vLookAt.y -= m_fHeightOffset;
		}
	}
}
