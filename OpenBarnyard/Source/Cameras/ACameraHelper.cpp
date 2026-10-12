#include "pch.h"
#include "ACameraHelper.h"
#include "ACameraManager.h"
#include "Units/AUnit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( ACameraHelper );

// $Barnyard: FUNCTION 0045b6e0
ACameraHelper::ACameraHelper()
    : m_pCamera( TNULL )
    , m_bUnk( TFALSE )
{
}

// $Barnyard: FUNCTION 0045b700
// $Barnyard: FUNCTION 0045b720
ACameraHelper::~ACameraHelper()
{
}

// $Barnyard: FUNCTION 0045b660
void ACameraHelper::UpdateCurrentCamera()
{
	m_pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();
}

// $Barnyard: FUNCTION 0045b6a0
void ACameraHelper::GetCameraRotation( Toshi::TVector4& a_rOutRotation ) const
{
	a_rOutRotation   = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation();
	a_rOutRotation.w = 1.0f;
}

// $Barnyard: FUNCTION 00462d80
TBOOL ACameraHelper::IsPositionInFadeRange( const Toshi::TVector4& a_rcCameraPos, const Toshi::TVector4& a_rcPosition )
{
	TFLOAT fDistX = a_rcPosition.x - a_rcCameraPos.x;
	TFLOAT fDistZ = a_rcPosition.z - a_rcCameraPos.z;

	if ( fDistX * fDistX + fDistZ * fDistZ <= 2.56f )
	{
		if ( TMath::Abs( a_rcCameraPos.y - a_rcPosition.y ) <= 2.5f )
			return TTRUE;
	}

	return TFALSE;
}

// DOES_NOTHING
void ACameraHelper::ResetPosition( TBOOL a_bSnapCamera )
{
}

// $Barnyard: FUNCTION 0045b790
void ACameraHelper::SetTarget( TRef<AUnitProxy> a_pTarget )
{
	m_pTarget = a_pTarget;
}

// $Barnyard: FUNCTION 004642b0
void ACameraHelper::SetTargetUnit( AUnit* a_pUnit )
{
	SetTarget( a_pUnit->GetSelfProxy() );
}

// $Barnyard: FUNCTION 0045b680
void ACameraHelper::ResetTarget()
{
	m_pTarget = TNULL;
}

// $Barnyard: FUNCTION 0045b540
void ACameraHelper::LookAt( const TVector4& a_rcPosition, const TVector4& a_rcTarget )
{
	TVector4 vecDirection = a_rcTarget - a_rcPosition;
	vecDirection.w        = a_rcTarget.w;
	vecDirection.Normalise();

	const TVector4& rcUp = m_pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_UP );
	TVector4        vecDown( -rcUp.x, -rcUp.y, -rcUp.z, 1.0f );

	TFLOAT fDot = TVector4::DotProduct3( vecDirection, vecDown );

	if ( fDot < 0.9f && fDot > -0.9f )
	{
		m_pCamera->LookAtDirection( vecDirection );
		m_pCamera->GetMatrix().SetTranslation( a_rcPosition );
	}
}

// $Barnyard: FUNCTION 0045b3b0
void ACameraHelper::LookAtDirection( const TVector4& a_rcDirection )
{
	const TVector4& rcUp = m_pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_UP );
	TVector4        vecDown( -rcUp.x, -rcUp.y, -rcUp.z, 1.0f );

	TFLOAT fDot = TVector4::DotProduct3( a_rcDirection, vecDown );

	if ( fDot < 0.9f && fDot > -0.9f )
		m_pCamera->LookAtDirection( a_rcDirection );
}

// $Barnyard: FUNCTION 0045b450
void ACameraHelper::LookAtPoint( const TVector4& a_rcPoint )
{
	const TVector4& rcPosition = m_pCamera->GetMatrix().GetTranslation();

	TVector4 vecDirection;
	vecDirection.x = a_rcPoint.x - rcPosition.x;
	vecDirection.y = a_rcPoint.y - rcPosition.y;
	vecDirection.z = a_rcPoint.z - rcPosition.z;
	vecDirection.w = a_rcPoint.w;
	vecDirection.Normalise();

	const TVector4& rcUp = m_pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_UP );
	TVector4        vecDown( -rcUp.x, -rcUp.y, -rcUp.z, 1.0f );

	TFLOAT fDot = TVector4::DotProduct3( vecDirection, vecDown );

	if ( fDot < 0.9f && fDot > -0.9f )
		m_pCamera->LookAtDirection( vecDirection );
}

// $Barnyard: FUNCTION 0045b8c0
TBOOL ACameraHelper::MoveTowards( TFLOAT& a_rfOutValue, TFLOAT a_fTarget, TFLOAT a_fCurrent, TFLOAT a_fSpeedDown, TFLOAT a_fSpeedUp, TFLOAT a_fDeltaTime )
{
	TFLOAT fDiff = a_fTarget - a_fCurrent;

	if ( fDiff >= 0.0f )
	{
		if ( fDiff <= 0.0f )
		{
			a_rfOutValue = a_fTarget;
			return TTRUE;
		}

		TFLOAT fValue = fDiff * a_fSpeedUp * a_fDeltaTime + a_fCurrent;
		a_rfOutValue  = fValue;

		if ( fValue > a_fTarget )
		{
			a_rfOutValue = a_fTarget;
			return TTRUE;
		}
	}
	else
	{
		TFLOAT fValue = fDiff * a_fSpeedDown * a_fDeltaTime + a_fCurrent;
		a_rfOutValue  = fValue;

		if ( fValue < a_fTarget )
		{
			a_rfOutValue = a_fTarget;
			return TTRUE;
		}
	}

	return TFALSE;
}

// DOES_NOTHING
void ACameraHelper::OnDetach()
{
}

// NOTE: shares 0045b650 with AGameState::GetFOV
TFLOAT ACameraHelper::GetFOV() const
{
	return ACamera::sm_fCurrentFOV;
}
