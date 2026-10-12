#include "pch.h"
#include "AGUI2ElementPopup.h"

#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Pop curve: sin( POP_ANGLE * scale ) / sin( POP_ANGLE ) overshoots 1.0 before settling
static constexpr TFLOAT POP_ANGLE = TMath::PI * 0.75f;

AGUI2ElementPopup::AGUI2ElementPopup()
    : m_uiEffects( 0 )
{
}

// $Barnyard: FUNCTION 004ab250
AGUI2ElementPopup::~AGUI2ElementPopup()
{
}

// $Barnyard: FUNCTION 00637090
void AGUI2ElementPopup::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_bShown != IsVisible() )
		SetShownInstantly( IsVisible() );

	if ( !IsVisible() && m_fTargetScale == 0.0f )
	{
		T2GUIElement::Tick( a_fDeltaTime );
		return;
	}

	TFLOAT fPulse;

	if ( ( m_uiEffects & ( 1 << EFFECT_PULSE ) ) && m_fTargetScale == m_fScale )
	{
		m_fPulsePhase += a_fDeltaTime * m_fPulseSpeed;
		fPulse = TMath::Sin( m_fPulsePhase ) * 0.1f + 1.0f;
	}
	else
	{
		fPulse = 1.0f;
	}

	if ( m_fScale > 0.0f || m_fTargetScale != m_fScale )
	{
		TFLOAT fNorm = 1.0f / TMath::Sin( POP_ANGLE );

		if ( m_fTargetScale > m_fScale )
			m_fScale += a_fDeltaTime + a_fDeltaTime;
		else if ( m_fTargetScale < m_fScale )
			m_fScale -= a_fDeltaTime + a_fDeltaTime;

		TMath::Clip( m_fScale, 0.0f, 1.0f );

		m_pElement->GetTransform().SetIdentity();
		TFLOAT fPop = TMath::Sin( POP_ANGLE * m_fScale ) * fNorm;

		SetVisible( fPop > 0.0f );

		if ( fPop > 0.0f )
		{
			TFLOAT fScale = fPop * fPulse;
			m_pElement->GetTransform().Scale( fScale, fScale );
		}
		else
		{
			m_pElement->GetTransform().Scale( 0.001f, 0.001f );
		}
	}

	T2GUIElement::Tick( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00637030
void AGUI2ElementPopup::SetShown( TBOOL a_bShown )
{
	m_fTargetScale = a_bShown ? 1.0f : 0.0f;
}

// $Barnyard: FUNCTION 00637050
void AGUI2ElementPopup::SetShownInstantly( TBOOL a_bShown )
{
	m_bShown = a_bShown;

	if ( a_bShown )
	{
		Show();
		m_fTargetScale = 1.0f;
		m_fScale       = 1.0f;
	}
	else
	{
		Hide();
		m_fTargetScale = 0.0f;
		m_fScale       = 0.0f;
	}
}

// $Barnyard: FUNCTION 00637370
void AGUI2ElementPopup::Create( T2GUIElement* a_pElement )
{
	m_pElement = a_pElement;
	Setup();
}

// $Barnyard: FUNCTION 00637260
void AGUI2ElementPopup::SetEffect( TINT a_iEffect, TBOOL a_bEnable )
{
	if ( a_iEffect == EFFECT_PULSE )
		m_fPulsePhase = 1.0f;

	if ( a_bEnable )
		m_uiEffects |= ( 1 << a_iEffect );
	else
		m_uiEffects &= ~( 1 << a_iEffect );
}

// $Barnyard: FUNCTION 006372a0
void AGUI2ElementPopup::Setup()
{
	m_fPulsePhase = 1.0f;
	m_uiEffects |= ( 1 << EFFECT_PULSE );

	AddChildTail( *m_pElement );
	m_pElement->SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	SetDimensions( m_pElement->GetWidth(), m_pElement->GetHeight() );

	m_fTargetScale = 0.0f;
	m_fScale       = 0.0f;

	m_pElement->GetTransform().SetIdentity();

	T2GUITransform oScale;
	oScale.Set( TVector2( 0.01f, 0.0f ), TVector2( 0.0f, 0.01f ), TVector2::VEC_ZERO );
	m_pElement->GetTransform().PreMultiply( oScale );

	Hide();
	m_bShown      = TFALSE;
	m_fPulseSpeed = 5.0f;
}
