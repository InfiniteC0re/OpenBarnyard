#include "pch.h"
#include "ASwirlTrackBoneAniEffect.h"
#include "Units/AAnimatedUnit.h"
#include "Animation/AAnimatableObject.h"
#include "Cameras/ACameraManager.h"
#include "Render/ARenderer.h"
#include "UnitMovement/Steering/ASteer.h"
#include "UnitMovement/Steering/AStationarySteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00631c00, CreateObject 00632070, CreateObjectInPlace 00632110 (the constructor is inlined in both)
TDEFINE_CLASS( ASwirlTrackBoneAniEffect );

// NOTE: m_iBone, m_fFadeDistance, m_fDepth and m_vScreenPos.y aren't initialised
ASwirlTrackBoneAniEffect::ASwirlTrackBoneAniEffect()
    : m_bFirstUpdate( TTRUE )
    , m_fSpeed( 1.0f )
    , m_fUnk18( 1.0f )
    , m_fMinDistance( 0.2f )
{
	m_vScreenPos.x = TMath::MAXFLOAT;
}

// $Barnyard: FUNCTION 006321a0
// $Barnyard: FUNCTION 00631fd0
ASwirlTrackBoneAniEffect::~ASwirlTrackBoneAniEffect()
{
}

// $Barnyard: FUNCTION 00631fc0
AUnit* ASwirlTrackBoneAniEffect::GetUnit()
{
	return m_oUnit->GetUnit();
}

// $Barnyard: FUNCTION 00631c10
TBOOL ASwirlTrackBoneAniEffect::Update( TFLOAT a_fDeltaTime )
{
	if ( !m_oUnit || !m_oUnit->GetUnit() )
	{
		Finish();
		return TFALSE;
	}

	AAnimatedUnit* pTarget = TSTATICCAST( AAnimatedUnit, m_oTarget ? m_oTarget->GetUnit() : TNULL );
	AAnimatedUnit* pUnit   = TSTATICCAST( AAnimatedUnit, m_oUnit ? m_oUnit->GetUnit() : TNULL );

	if ( m_bFirstUpdate )
	{
		pUnit->ChangeSteer( &TGetClass( AStationarySteer ) );
		pUnit->GetAnimatableObject()->SetAnimControllersDisabled( TFALSE );
		m_bFirstUpdate = TFALSE;
	}

	// The bone of the target, a bit below it and moved with the target
	TMatrix44 matBone;
	pTarget->GetModelInstance()->GetBoneTransform( m_iBone, matBone );

	TVector4 vBonePos = matBone.AsBasisVector4( BASISVECTOR_TRANSLATION );
	vBonePos.y -= 2.5f;

	TVector4 vVelocity;
	vVelocity.Multiply( pTarget->GetSteer()->GetVelocity(), a_fDeltaTime );
	vBonePos.x += vVelocity.x;
	vBonePos.y += vVelocity.y;
	vBonePos.z += vVelocity.z;

	TVector4 vPos    = *pUnit->GetSteer()->GetPosition();
	TFLOAT   fHeight = vPos.y - vBonePos.y;

	if ( fHeight <= 0.1f )
	{
		Finish();
		return TFALSE;
	}

	// Swirl around the bone, going down
	TVector4 vDirection;
	vDirection.Substract( vPos, vBonePos );
	vDirection.y = 0.0f;
	vDirection.Normalise();

	TVector4 vSwirlPos;
	vSwirlPos.Multiply( vDirection, fHeight * 0.5f );
	TREINTERPRETCAST( TVector3*, &vSwirlPos )->RotateY( 1.0f / fHeight );
	vSwirlPos.x += vBonePos.x;
	vSwirlPos.z += vBonePos.z;
	vSwirlPos.y = vPos.y - 0.3f;

	TVector4 vNewPos;
	vNewPos.Lerp4( vPos, vSwirlPos, a_fDeltaTime * m_fSpeed );
	pUnit->GetSteer()->SetPosition( vNewPos );

	TFLOAT fDistance = TVector4::Distance( vNewPos, vBonePos );

	if ( m_fFadeDistance > 0.0f )
	{
		TFLOAT fFade = fDistance - m_fMinDistance;

		if ( fFade < m_fFadeDistance )
		{
			fFade = TMath::Max( fFade, 0.0f );
			pUnit->GetModelInstance()->SetOpacity( fFade / m_fFadeDistance );
		}
	}

	if ( m_vScreenPos.x != TMath::MAXFLOAT && fDistance < 0.8f )
	{
		// Finish once the unit is past the point of the screen
		TVector4 vScreenPos;
		TVector4 vScreenDir;
		ARenderer::GetSingleton()->GetScreenRay( ACameraManager::GetSingleton()->GetCurrentCamera(), m_vScreenPos, vScreenPos, vScreenDir );

		TVector4 vOffset;
		vOffset.Multiply( vScreenDir, m_fDepth );
		vScreenPos.x += vOffset.x;
		vScreenPos.y += vOffset.y;
		vScreenPos.z += vOffset.z;

		TVector4 vToScreen;
		vToScreen.Substract( vScreenPos, vPos );

		TVector4 vToSwirl;
		vToSwirl.Substract( vSwirlPos, vPos );

		if ( TVector4::DotProduct3( vToScreen, vToSwirl ) > 0.3f )
		{
			Finish();
			return TFALSE;
		}
	}

	return TTRUE;
}
