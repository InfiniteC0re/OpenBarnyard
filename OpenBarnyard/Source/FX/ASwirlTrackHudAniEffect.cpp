#include "pch.h"
#include "ASwirlTrackHudAniEffect.h"
#include "Units/AAnimatedUnit.h"
#include "Units/ACarryableUnitHelper.h"
#include "Units/AObjectTraits.h"
#include "Animation/AAnimatableObject.h"
#include "GUI/AGUI2ModelRendererScreen.h"
#include "Cameras/ACameraManager.h"
#include "Render/ARenderer.h"
#include "UnitMovement/Steering/ASteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 006321c0, CreateObject 00632cf0, CreateObjectInPlace 00632d50
TDEFINE_CLASS( ASwirlTrackHudAniEffect );

// Gravity used for the arc of the throw (0x00720384)
static constexpr TFLOAT GRAVITY = 9.8f;

// $Barnyard: FUNCTION 00632c50
ASwirlTrackHudAniEffect::ASwirlTrackHudAniEffect()
    : m_pObject( TNULL )
    , m_fSpinSpeed( 0.0f )
    , m_fSpeed( 15.0f )
    , m_fUnk18( 1.0f )
    , m_fMinDistance( 0.5f )
    , m_fFadeDistance( 1.0f )
    , m_fDepth( 12.0f )
    , m_bStarted( TFALSE )
{
	m_oUnitDestroyListener.Connect( this, OnUnitDestroyedCallback );
}

// $Barnyard: FUNCTION 00632cd0
// $Barnyard: FUNCTION 00632b00
ASwirlTrackHudAniEffect::~ASwirlTrackHudAniEffect()
{
	ReleaseObject();
	m_oUnitDestroyListener.Disconnect();
}

// $Barnyard: FUNCTION 00632bc0
AUnit* ASwirlTrackHudAniEffect::GetUnit()
{
	return m_oUnit->GetUnit();
}

// $Barnyard: FUNCTION 006324c0
TBOOL ASwirlTrackHudAniEffect::Update( TFLOAT a_fDeltaTime )
{
	if ( ( !m_oUnit || !m_oUnit->GetUnit() ) && !m_pObject )
	{
		ReleaseObject();
		Finish();
		return TFALSE;
	}

	AAnimatedUnit* pUnit = TSTATICCAST( AAnimatedUnit, m_oUnit ? m_oUnit->GetUnit() : TNULL );

	if ( !m_pObject )
	{
		// Start throwing the model of the unit
		m_pObject = pUnit->GetAnimatableObject();
		AGUI2ModelRendererScreen::GetSingleton()->AddObject( m_pObject );
		m_pObject->SetSkeletonUpdating( TFALSE, TTRUE );

		TVector4 vDirection;
		ARenderer::GetSingleton()->GetScreenRay( ACameraManager::GetSingleton()->GetCurrentCamera(), m_vScreenPos, m_vEndPos, vDirection );

		TVector4 vOffset;
		vOffset.Multiply( vDirection, m_fDepth );
		m_vEndPos.x += vOffset.x;
		m_vEndPos.y += vOffset.y;
		m_vEndPos.z += vOffset.z;

		m_fTime = 0.0f;
		pUnit->GetAnimatableObject()->GetModelInstance()->GetPosition( m_vStartPos );

		ACarryableUnitHelper* pHelper = pUnit->GetCarryableHelper();
		m_fEndScale                   = 1.0f;

		if ( pHelper )
			m_fEndScale = pHelper->GetObjectTraits()->m_fItemScale;

		m_fStartScale = pUnit->GetAnimatableObject()->GetModelInstance()->GetScale().x;
		m_fDistance   = TVector4::Distance( m_vEndPos, m_vStartPos );

		TMatrix44 matTransform;
		m_pObject->GetModelInstance()->GetLocalTransform( matTransform );
		TMatrix44::MatToQuat( m_qStartRot, matTransform );
	}
	else if ( !m_oUnit && !m_bStarted )
	{
		// Start throwing the object
		m_bStarted = TTRUE;
		AGUI2ModelRendererScreen::GetSingleton()->AddObject( m_pObject );
		m_pObject->SetSkeletonUpdating( TFALSE, TTRUE );

		TVector4 vDirection;
		ARenderer::GetSingleton()->GetScreenRay( ACameraManager::GetSingleton()->GetCurrentCamera(), m_vScreenPos, m_vEndPos, vDirection );

		TVector4 vOffset;
		vOffset.Multiply( vDirection, m_fDepth );
		m_vEndPos.x += vOffset.x;
		m_vEndPos.y += vOffset.y;
		m_vEndPos.z += vOffset.z;

		m_fTime = 0.0f;
		m_pObject->GetModelInstance()->GetPosition( m_vStartPos );

		m_fEndScale   = 1.0f;
		m_fStartScale = m_pObject->GetModelInstance()->GetScale().x;
		m_fDistance   = TVector4::Distance( m_vEndPos, m_vStartPos );

		TMatrix44 matTransform;
		m_pObject->GetModelInstance()->GetLocalTransform( matTransform );
		TMatrix44::MatToQuat( m_qStartRot, matTransform );
		TREINTERPRETCAST( TVector4*, &m_qStartRot )->Normalise4();
	}

	AModelInstance* pModelInstance = m_pObject->GetModelInstance();

	if ( pModelInstance )
	{
		// The end position is lowered so the model flies in an arc
		TFLOAT   fDuration = m_fDistance / m_fSpeed;
		TVector4 vEndPos   = m_vEndPos;
		vEndPos.y -= fDuration * fDuration * GRAVITY;

		m_fTime += a_fDeltaTime;
		TFLOAT fProgress = m_fTime / m_fDistance * m_fSpeed;

		if ( fProgress > 1.0f )
			fProgress = 1.0f;

		TVector4 vPos;
		vPos.Lerp4( m_vStartPos, vEndPos, fProgress );
		vPos.y += m_fTime * m_fTime * GRAVITY;

		TFLOAT fScale = TMath::LERP( m_fStartScale, m_fEndScale, fProgress );
		pModelInstance->SetScale( TVector4( fScale, fScale, fScale, 1.0f ) );

		m_qEndRot.RotateAroundAxis( TVector3::VEC_POSZ, a_fDeltaTime * m_fSpinSpeed );

		TQuaternion qRotation;
		TQuaternion::Slerp( qRotation, m_qStartRot, m_qEndRot, fProgress );
		m_pObject->GetModelInstance()->SetTransform( vPos, qRotation );

		if ( m_oUnit )
		{
			// Keep the unit at the model
			m_oUnit->GetUnit()->GetSteer()->SetPosition( vPos );

			TVector4 vUp;
			m_pObject->GetModelInstance()->GetUpVector( vUp );
			m_oUnit->GetUnit()->GetSteer()->SetDirection( vUp );
		}

		TFLOAT fDistance = TVector4::Distance( vPos, m_vEndPos );

		if ( m_fFadeDistance > 0.0f )
		{
			TFLOAT fFade = fDistance - m_fFadeDistance - m_fMinDistance;

			if ( fFade < m_fFadeDistance )
			{
				fFade = TMath::Max( fFade, 0.0f );
				m_pObject->SetOpacity( fFade / m_fFadeDistance, TTRUE );
			}
		}

		if ( fProgress < 1.0f )
			return TTRUE;
	}

	ReleaseObject();
	Finish();
	return TFALSE;
}

// $Barnyard: FUNCTION 00632bd0
void ASwirlTrackHudAniEffect::Setup( AUnit* a_pUnit, const TVector2& a_rcScreenPos, const TQuaternion& a_rcRotation, TFLOAT a_fDepth, TFLOAT a_fSpeed, TFLOAT a_fFadeDistance, TFLOAT a_fMinDistance )
{
	m_fFadeDistance = a_fFadeDistance;
	SetUnit( a_pUnit );
	m_fSpeed       = a_fSpeed;
	m_vScreenPos   = a_rcScreenPos;
	m_fDepth       = a_fDepth;
	m_qEndRot      = a_rcRotation;
	m_fMinDistance = a_fMinDistance;
}

// $Barnyard: FUNCTION 006321d0
void ASwirlTrackHudAniEffect::Setup( AAnimatableObject* a_pObject, const TVector2& a_rcScreenPos, const TQuaternion& a_rcRotation, TFLOAT a_fDepth, TFLOAT a_fSpeed, TFLOAT a_fFadeDistance, TFLOAT a_fMinDistance )
{
	m_fFadeDistance = a_fFadeDistance;
	m_fSpeed        = a_fSpeed;
	m_vScreenPos    = a_rcScreenPos;
	m_fDepth        = a_fDepth;
	m_qEndRot       = a_rcRotation;
	m_fMinDistance  = a_fMinDistance;
	m_pObject       = a_pObject;
	m_oUnit         = TNULL;
}

// $Barnyard: FUNCTION 00632430
void ASwirlTrackHudAniEffect::SetUnit( AUnit* a_pUnit )
{
	m_oUnit = a_pUnit->GetSelfProxy();
}

// $Barnyard: FUNCTION 00632250
void ASwirlTrackHudAniEffect::ReleaseObject()
{
	if ( !m_pObject || !m_pObject->GetModelInstance() )
		return;

	if ( m_oUnit && m_oUnit->GetUnit() )
	{
		// Move the unit to where the model is
		AUnit* pUnit = m_oUnit ? m_oUnit->GetUnit() : TNULL;

		TVector4 vPos;
		m_pObject->GetModelInstance()->GetPosition( vPos );
		pUnit->SetSteerPosition( vPos );

		TVector4 vUp;
		m_pObject->GetModelInstance()->GetUpVector( vUp );
		pUnit->SetSteerDirection( vUp );
	}

	m_pObject->SetSkeletonUpdating( TTRUE, TTRUE );

	TMatrix44 matTransform;
	AGUI2ModelRendererScreen::GetSingleton()->GetObjectWorldTransform( m_pObject, matTransform );
	m_pObject->GetModelInstance()->SetTransform( matTransform );
	AGUI2ModelRendererScreen::GetSingleton()->RemoveObject( m_pObject );

	m_pObject->SetAnimControllersDisabled( TFALSE );
	m_pObject->SetVisible( TTRUE );
	m_pObject->SetSkeletonUpdating( TTRUE, TTRUE );
	m_pObject = TNULL;
}

// $Barnyard: FUNCTION 00632c30
void ASwirlTrackHudAniEffect::OnUnitDestroyedCallback( void* a_pEffect, const AUnitDestroyEvent& a_rcEvent )
{
	ASwirlTrackHudAniEffect* pEffect = TSTATICCAST( ASwirlTrackHudAniEffect, a_pEffect );

	if ( ( pEffect->m_oUnit ? pEffect->m_oUnit->GetUnit() : TNULL ) == a_rcEvent.pUnit )
		pEffect->ReleaseObject();
}
