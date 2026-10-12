#include "pch.h"
#include "AGUI2Flare.h"
#include "ARandom.h"

#include <GUI/T2GUITextureSectionManager.h>
#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 006395f0
AGUI2Flare::AGUI2Flare()
{
}

// $Barnyard: FUNCTION 006397e0
// $Barnyard: FUNCTION 00639640
AGUI2Flare::~AGUI2Flare()
{
}

// $Barnyard: FUNCTION 00639b80
void AGUI2Flare::Tick( TFLOAT a_fDeltaTime )
{
	Update( a_fDeltaTime );
	T2GUIElement::Tick( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00639720
void AGUI2Flare::Render()
{
	if ( m_oFlare.IsVisible() )
		T2GUIElement::Render();
}

// $Barnyard: FUNCTION 00639800
void AGUI2Flare::Create( const TCHAR* a_szTexture, TFLOAT a_fRotation, TFLOAT a_fRotationSpeed, TFLOAT a_fAlphaPhase, TFLOAT a_fAlphaSpeed, TFLOAT a_fScaleAmount, TFLOAT a_fScaleSpeed, TFLOAT a_fScale )
{
	m_oFlare.T2GUIElement::SetDimensions( 1.0f, 1.0f );
	m_oFlare.Hide();
	SetTexture( a_szTexture );
	AddChildTail( m_oFlare );

	m_fRotation      = TMath::PI * a_fRotation;
	m_fRotationSpeed = a_fRotationSpeed;
	m_fScaleAmount   = a_fScaleAmount;
	m_fScaleSpeed    = a_fScaleSpeed;
	m_fAlphaPhase    = TMath::PI * a_fAlphaPhase;
	m_fAlphaSpeed    = a_fAlphaSpeed;
	m_fScalePhase    = 0.0f;
	m_fTime          = 0.0f;
	m_fFade          = 0.0f;
	m_fFadeSpeed     = 0.0f;

	m_oTransform.SetIdentity();
	m_oTransform.Scale( a_fScale, a_fScale );

	m_oFlare.GetTransform().SetIdentity();

	T2GUITransform oZero;
	oZero.SetZero();
	m_oFlare.GetTransform().PreMultiply( oZero );
}

// $Barnyard: FUNCTION 006396a0
void AGUI2Flare::SetTexture( const TCHAR* a_szTexture )
{
	if ( a_szTexture )
	{
		T2GUITextureSection* pSection = T2GUITextureSectionManager::GetTextureSection( a_szTexture );
		TFLOAT               fWidth   = TFLOAT( pSection->GetPoint2X() - pSection->GetPoint1X() );
		TFLOAT               fHeight  = TFLOAT( pSection->GetPoint2Y() - pSection->GetPoint1Y() );

		m_oFlare.SetTextureSection( pSection );
		m_oFlare.SetDimensions( TMath::Abs( fWidth ), TMath::Abs( fHeight ) );
	}
	else
	{
		m_oFlare.Hide();
	}
}

// $Barnyard: FUNCTION 00639740
void AGUI2Flare::Start( TBOOL a_bFadeIn, TFLOAT a_fFadeSpeed, TFLOAT a_fDuration )
{
	TFLOAT fSpeed = TMath::Abs( a_fFadeSpeed );

	m_fFadeSpeed = a_fFadeSpeed;
	m_fDuration  = a_fDuration;
	m_fTime      = 0.0f;

	if ( a_bFadeIn )
	{
		m_fFadeSpeed  = fSpeed;
		m_fScalePhase = 0.0f;
		m_fFade       = ( 0.0f == fSpeed ) ? 1.0f : 0.0f;

		m_oFlare.GetTransform().SetIdentity();
		m_oFlare.GetTransform().Scale( 0.0f, 0.0f );
		m_oFlare.Hide();
	}
	else
	{
		m_fFadeSpeed = -fSpeed;
	}
}

// $Barnyard: FUNCTION 00639920
void AGUI2Flare::Update( TFLOAT a_fDeltaTime )
{
	m_fFade += a_fDeltaTime * m_fFadeSpeed;

	if ( 0.0f != m_fFadeSpeed && m_fFade <= 0.0f )
	{
		m_oFlare.Hide();
		return;
	}

	TFLOAT fFade = 1.0f;

	if ( m_fFadeSpeed != 0.0f )
	{
		TMath::Clip( m_fFade, 0.0f, 1.0f );
		fFade = m_fFade;

		if ( m_fFade <= 0.0f )
		{
			m_oFlare.Hide();
			return;
		}
	}

	m_fTime -= a_fDeltaTime;
	m_fRotation += a_fDeltaTime * m_fRotationSpeed;

	TFLOAT fLife;

	if ( m_fDuration <= 0.0f )
		fLife = 1.0f;
	else
		fLife = TMath::Min( m_fTime / m_fDuration + 0.5f, 1.0f );

	m_fAlphaPhase += a_fDeltaTime * m_fAlphaSpeed;
	TFLOAT fAlpha = ( TMath::Abs( TMath::Sin( m_fAlphaPhase ) ) + 1.0f ) * 0.5f;
	TMath::Clip( fAlpha, 0.4f, 0.5f );

	m_oFlare.GetTransform().SetIdentity();
	m_oFlare.GetTransform().Rotate( m_fRotation );

	m_fScalePhase += ( ARandom::GetSingleton()->m_oRandom.GetFloat() * 0.1f + m_fScaleSpeed ) * a_fDeltaTime;
	TFLOAT fScale = ( TMath::Abs( TMath::Sin( m_fScalePhase ) ) * m_fScaleAmount + 1.0f - m_fScaleAmount ) * fLife * fFade;
	TMath::Clip( fScale, 0.0f, 1.0f );

	m_oFlare.GetTransform().Scale( fScale, fScale );
	m_oFlare.SetAlpha( fAlpha );
	m_oFlare.SetVisible( fScale > 0.0f && fAlpha > 0.0f );
}
