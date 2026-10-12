#include "pch.h"
#include "AMiniGamePlayerFloat.h"
#include "Render/ARenderer.h"
#include "GUI/AGUI2.h"
#include "Units/AUnit.h"
#include "UnitMovement/Steering/ASteer.h"

#include <GUI/T2GUIFontManager.h>
#include <Math/TMathInline.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// 007736a0
static const TUINT32 s_aPlayerColours[ 4 ] = {
	0xFFFF0000,
	0xFF00FF00,
	0xFFFFFF00,
	0xFF00FFFF
};

// 007736b0
static const TUINT32 s_uiComputerColour = 0xFF0000FF;

// Projects the position above a_pUnit to the screen.
static void GetScreenPosition( AUnit* a_pUnit, TFLOAT a_fHeightOffset, TVector2& a_rOutPos )
{
	a_rOutPos            = TVector2::VEC_ZERO;
	TVector4 vecPosition = TVector4::VEC_ZERO;
	a_pUnit->GetSteer()->CopyPosition( vecPosition );
	vecPosition.y -= a_fHeightOffset;

	ARenderer::GetSingleton()->GetScreenPosition( vecPosition, a_rOutPos );
}

// $Barnyard: FUNCTION 004566e0
AMiniGamePlayerFloat::AMiniGamePlayerFloat()
    : m_pUnit( TNULL )
    , m_fHeightOffset( 0.0f )
    , m_fScale( 1.0f )
    , m_bEnabled( TFALSE )
{
}

// $Barnyard: FUNCTION 004567f0
// $Barnyard: FUNCTION 00456750
AMiniGamePlayerFloat::~AMiniGamePlayerFloat()
{
	m_oText.RemoveSelf();
	RemoveSelf();
}

// $Barnyard: FUNCTION 00456810
TBOOL AMiniGamePlayerFloat::SetPlayer( TINT a_iSlot, TBOOL a_bHuman, AUnit* a_pUnit )
{
	TUINT32 uiColour;

	if ( a_bHuman == TTRUE )
	{
		TStringManager::String16Format( m_wszLabel, 8, L"P%d", a_iSlot + 1 );
		uiColour = s_aPlayerColours[ a_iSlot ];
	}
	else
	{
		TStringManager::String16Format( m_wszLabel, 8, L"Com" );
		uiColour = s_uiComputerColour;
	}

	m_oText.SetColour( uiColour );
	m_pUnit = a_pUnit;

	TVector2 vecScreenPos;
	m_fHeightOffset = HEIGHT_OFFSET;
	GetScreenPosition( a_pUnit, HEIGHT_OFFSET, vecScreenPos );

	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord26" );
	m_oText.Create( pFont, pFont->GetTextWidth( m_wszLabel, 1.0f ) );
	m_oText.SetText( m_wszLabel );
	m_oText.SetWidth( pFont->GetTextWidth( m_wszLabel, 1.0f ) * m_oText.GetScale() );

	AddChildTail( m_oText );
	AGUI2::GetRootElement()->AddChildTail( *this );

	SetAlpha( 1.0f );
	SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	SetDimensions( m_oText.GetWidth(), m_oText.GetHeight() );
	SetTransform( vecScreenPos.x - GetWidth() * 0.5f, vecScreenPos.y - GetHeight() * 0.5f, 0.0f );

	m_bEnabled = TFALSE;
	m_fScale   = 1.0f;

	return TTRUE;
}

// $Barnyard: FUNCTION 004569d0
void AMiniGamePlayerFloat::Tick( TFLOAT a_fDeltaTime )
{
	if ( IsVisible() )
	{
		if ( m_bEnabled == TTRUE )
		{
			m_fScale -= a_fDeltaTime * 10.0f;
			TMath::Clip( m_fScale, 0.0f, 1.0f );
			m_oText.GetTransform().SetFromScale( m_fScale, m_fScale );
		}

		if ( m_fScale > 0.0f )
		{
			TVector2 vecScreenPos;
			GetScreenPosition( m_pUnit, m_fHeightOffset, vecScreenPos );
			SetTransform( vecScreenPos.x - GetWidth() * 0.5f, vecScreenPos.y - GetHeight() * 0.5f, 0.0f );

			T2GUIElement::Tick( a_fDeltaTime );
			return;
		}

		Hide();
	}

	T2GUIElement::Tick( a_fDeltaTime );
}

// $Barnyard: FUNCTION 004567d0
void AMiniGamePlayerFloat::SetAlpha( TFLOAT a_fAlpha )
{
	m_oText.SetAlpha( a_fAlpha );
	T2GUIElement::SetAlpha( a_fAlpha );
}
