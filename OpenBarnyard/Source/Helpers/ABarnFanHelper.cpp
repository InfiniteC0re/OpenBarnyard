#include "pch.h"
#include "ABarnFanHelper.h"
#include "AReplayManager.h"
#include "ARandom.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AModel.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "MiniGame/ABarnExploreMinigameState.h"

// Opcode (pulled in by the Physics headers) #defines PI, which breaks TMath::PI below
#ifdef PI
#  undef PI
#endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// 00783c60, how much the fans wobble (one degree)
static const TFLOAT s_fFanWobble = TMath::PI / 180.0f;

// 00783c64, the spin speeds a fan picks from
static const TFLOAT s_afFanSpeeds[] = {
	TMath::PI * 0.25f,
	TMath::PI * 0.5f,
	TMath::PI
};

// NOTE: inlined into the constructor of ABarnFanHelper in retail
ABarnFanHelper::Fan::Fan()
    : pObject( TNULL )
    , fAngle( 0.0f )
    , fSpeed( s_afFanSpeeds[ 0 ] )
{
}

// $Barnyard: FUNCTION 0053fbb0
void ABarnFanHelper::Fan::Create( AAnimatableObject* a_pObject, const TMatrix44& a_rcTransform )
{
	pObject   = a_pObject;
	matOrigin = a_rcTransform;

	pObject->GetModelInstance()->SetTransform( matOrigin );
	pObject->SetSkeletonUpdating( TTRUE, TTRUE );

	fSpeed = s_afFanSpeeds[ ARandom::GetSingleton()->m_oRandom.GetInt( TARRAYSIZE( s_afFanSpeeds ) ) ];
}

// $Barnyard: FUNCTION 0053f9f0
void ABarnFanHelper::Fan::Update( TFLOAT a_fDeltaTime, TBOOL a_bCheckFloor )
{
	if ( !pObject->IsInCameraFrustum() )
	{
		pObject->SetSkeletonUpdating( TFALSE, TFALSE );
		return;
	}

	if ( a_bCheckFloor )
	{
		// Don't animate the fans of the other floor
		TVector4 vecCamera = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation();

		TVector4 vecFan;
		pObject->GetModelInstance()->GetPosition( vecFan );

		if ( ( vecCamera.y > 0.0f && vecFan.y < 0.0f ) || ( vecCamera.y < 0.0f && vecFan.y > 0.0f ) )
		{
			pObject->SetSkeletonUpdating( TFALSE, TFALSE );
			return;
		}
	}

	pObject->SetSkeletonUpdating( TTRUE, TFALSE );

	TMatrix44 matTransform = matOrigin;
	matTransform.RotateY( TMath::Sin( fAngle ) * s_fFanWobble );
	matTransform.RotateZ( fAngle );
	pObject->GetModelInstance()->SetTransform( matTransform );

	fAngle = fmodf( a_fDeltaTime * fSpeed + fAngle, TMath::TWO_PI );
}

// $Barnyard: FUNCTION 0053fc60
TBOOL ABarnFanHelper::OnUpdate( TFLOAT a_fDeltaTime )
{
	AReplayManager::GetSingleton()->RotateStars( a_fDeltaTime );

	if ( ABarnExploreMinigameState::ms_iFloor != ABarnExploreMinigameState::FLOOR_LOFT )
	{
		for ( TINT i = m_iNumLowerFans - 1; i >= 0; i-- )
			m_aLowerFans[ i ].Update( a_fDeltaTime, m_bCheckFloor );
	}
	else
	{
		for ( TINT i = m_iNumUpperFans - 1; i >= 0; i-- )
			m_aUpperFans[ i ].Update( a_fDeltaTime, m_bCheckFloor );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0053fd00
void ABarnFanHelper::AddFan( AAnimatableObject* a_pObject, const TMatrix44& a_rcTransform )
{
	// NOTE: retail doesn't check the counts
	if ( a_rcTransform.m_f42 <= 0.0f )
	{
		TASSERT( m_iNumLowerFans < MAX_NUM_FANS );
		m_aLowerFans[ m_iNumLowerFans++ ].Create( a_pObject, a_rcTransform );
	}
	else
	{
		TASSERT( m_iNumUpperFans < MAX_NUM_FANS );
		m_aUpperFans[ m_iNumUpperFans++ ].Create( a_pObject, a_rcTransform );
	}
}
