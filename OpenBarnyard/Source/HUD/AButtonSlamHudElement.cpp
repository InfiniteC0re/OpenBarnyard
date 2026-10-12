#include "pch.h"
#include "AButtonSlamHudElement.h"
#include "GUI/AGUI2.h"

#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( AButtonSlamHudElement );

static TFLOAT s_fBarLength       = 288.0f;              // 0x00773644
static TFLOAT s_fBracketX        = 54.0f;               // 0x00773648
static TFLOAT s_fBracketY        = 12.0f;               // 0x0077364c
static TFLOAT s_fHorizontalWidth = s_fBarLength + 3.0f; // 0x00781f08

// $Barnyard: FUNCTION 0044fd70
AButtonSlamHudElement::AButtonSlamHudElement()
    : AHudElement( MASK_UNK2 | MASK_UNK15 )
{
	m_fTargetValue     = 0.0f;
	m_fValue           = 0.0f;
	m_fBracketPos      = 0.0f;
	m_bHorizontal      = TFALSE;
	m_fTargetBandStart = -1.0f;
	m_fFillSpeed       = 1.5f;
}

// $Barnyard: FUNCTION 0044f640
// $Barnyard: FUNCTION 0044fe80
AButtonSlamHudElement::~AButtonSlamHudElement()
{
	m_oBar.RemoveSelf();
	m_oBracket.RemoveSelf();
	m_oBracket2.RemoveSelf();
	m_oBand.RemoveSelf();
}

// $Barnyard: FUNCTION 0044fea0
TBOOL AButtonSlamHudElement::OnAttach()
{
	// The bar
	T2GUITextureSection* pBarSection = T2GUITextureSectionManager::GetTextureSection( "Power_Bar_New" );
	m_oBar.T2GUIElement::SetDimensions(
	    TMath::Abs( TFLOAT( pBarSection->GetPoint2X() - pBarSection->GetPoint1X() ) ),
	    TMath::Abs( TFLOAT( pBarSection->GetPoint2Y() - pBarSection->GetPoint1Y() ) )
	);
	m_oBar.SetTextureSection( pBarSection );
	m_oBar.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oBar.Hide();
	m_oBar.SetTransform( 0.0f, -40.0f, 0.0f );
	AGUI2::GetRootElement()->AddChildTail( m_oBar );

	// The band showing the value
	T2GUITextureSection* pBandSection = T2GUITextureSectionManager::GetTextureSection( "Power_Band_Blue" );
	TFLOAT               fBandWidth   = TMath::Abs( TFLOAT( pBandSection->GetPoint2X() - pBandSection->GetPoint1X() ) );
	m_oBand.T2GUIElement::SetDimensions( fBandWidth, s_fBarLength );
	m_oBand.SetTextureSection( pBandSection );
	m_oBand.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oBand.SetTranslation( 54.0f, 38.0f );
	m_oBand.GetTransform().Rotate( -TMath::HALF_PI );
	m_oBar.AddChildTail( m_oBand );

	// The band showing the target value
	T2GUITextureSection* pTargetSection = T2GUITextureSectionManager::GetTextureSection( "Power_Band_Yel" );
	m_oBandTarget.T2GUIElement::SetDimensions( fBandWidth, s_fBarLength );
	m_oBandTarget.SetTextureSection( pTargetSection );
	m_oBandTarget.SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_BOTTOMLEFT );
	m_oBand.AddChildTail( m_oBandTarget );
	m_oBandTarget.Hide();

	// The brackets
	T2GUITextureSection* pBracketSection = T2GUITextureSectionManager::GetTextureSection( "Power_Bracket" );
	TFLOAT               fBracketWidth   = TMath::Abs( TFLOAT( pBracketSection->GetPoint2X() - pBracketSection->GetPoint1X() ) );
	TFLOAT               fBracketHeight  = TMath::Abs( TFLOAT( pBracketSection->GetPoint2Y() - pBracketSection->GetPoint1Y() ) );

	m_oBracket.T2GUIElement::SetDimensions( fBracketWidth, fBracketHeight );
	m_oBracket.SetTextureSection( pBracketSection );
	m_oBracket.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oBracket.SetTranslation( s_fBracketX, s_fBracketY );
	m_oBar.AddChildTail( m_oBracket );

	m_oBracket2.T2GUIElement::SetDimensions( fBracketWidth, fBracketHeight );
	m_oBracket2.SetTextureSection( pBracketSection );
	m_oBracket2.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oBracket2.SetTranslation( s_fBracketX, s_fBracketY );
	m_oBar.AddChildTail( m_oBracket2 );
	m_oBracket2.Hide();

	// The arrow
	m_oArrowRoot.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_BOTTOMLEFT );
	m_oBar.AddChildTail( m_oArrowRoot );
	m_oArrowRoot.Hide();

	m_oArrow.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( "Golf_Tee_Arrow" ) );
	m_oArrow.SetDimensions( 32.0f, 64.0f );
	m_oArrow.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oArrowRoot.AddChildTail( m_oArrow );
	m_oArrow.Show();

	// NOTE: unused in retail
	T2GUITextureSectionManager::GetTextureSection( "ps2_cross" );
	m_oArrowRoot.SetDimensions( 32.0f, 64.0f );

	// The results
	for ( TINT i = 0; i < NUM_RESULTS; i++ )
	{
		m_aResults[ i ].SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( "Golf_Tick" ) );
		m_aResults[ i ].SetDimensions( 32.0f, 32.0f );
		m_aResults[ i ].SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_MIDDLELEFT );
		m_aResults[ i ].Hide();
		m_oBar.AddChildTail( m_aResults[ i ] );
	}

	// The text
	m_oText.SetFont( T2GUIFontManager::FindFont( "Rekord26" ) );
	m_oText.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oText.SetColour( 0xffffff00 );
	m_oText.Hide();
	m_oBar.AddChildTail( m_oText );

	m_fFadeTime    = 0.0f;
	m_fFadeDelay   = -1.0f;
	m_fAlphaChange = 1.0f;

	return TTRUE;
}

// $Barnyard: FUNCTION 0044f770
void AButtonSlamHudElement::Show()
{
	if ( m_fBracketPos >= 0.0f )
		m_oBar.Show();

	m_bCurrentlyVisible = TTRUE;
}

// NOTE: folded with AMenuNavHudElement::Hide (00455b60)
void AButtonSlamHudElement::Hide()
{
	m_oBar.Hide();
	m_bCurrentlyVisible = TFALSE;
}

// $Barnyard: FUNCTION 00450200
void AButtonSlamHudElement::Update( TFLOAT a_fDeltaTime )
{
	static TFLOAT s_fArrowScrollSpeed = 64.0f;

	if ( !m_oBar.IsVisible() )
		return;

	// Move the value towards the target value
	TFLOAT fStep  = TMath::Min( a_fDeltaTime * m_fFillSpeed, 1.0f );
	TFLOAT fValue = m_fValue;

	if ( fValue < m_fTargetValue )
	{
		fValue += fStep;
		if ( fValue > m_fTargetValue ) fValue = m_fTargetValue;
	}
	else if ( fValue != m_fTargetValue )
	{
		fValue -= fStep;
		if ( fValue < m_fTargetValue ) fValue = m_fTargetValue;
	}

	m_fValue = fValue;

	if ( m_bHorizontal )
	{
		if ( fValue > 0.0f )
		{
			m_oBand.Show();

			// Stretch the texture so it doesn't get squashed
			TVector2 vUV1, vUV2, vUV3, vUV4;
			m_oBand.GetUVs( vUV1, vUV2, vUV3, vUV4 );

			TFLOAT fScale = 1.0f / fValue;
			vUV2.x        = ( m_vBandUV2.x - vUV1.x ) * fScale + vUV1.x;
			vUV2.y        = ( m_vBandUV2.y - vUV1.y ) + vUV1.y;
			vUV4.x        = fScale * ( m_vBandUV4.x - vUV3.x ) + vUV3.x;
			vUV4.y        = ( m_vBandUV4.y - vUV3.y ) + vUV3.y;
			m_oBand.SetUVs( vUV1, vUV2, vUV3, vUV4 );

			m_oBand.SetWidth( s_fHorizontalWidth * fValue );
		}
		else
		{
			m_oBand.Hide();
		}
	}
	else
	{
		m_oBand.SetHeight( s_fBarLength * fValue );
		m_oBandTarget.SetHeight( ( m_fTargetValue - m_fTargetBandStart ) * s_fBarLength );
	}

	// Scroll the arrow
	TFLOAT fArrowHeight = m_oArrow.GetHeight() - s_fArrowScrollSpeed * a_fDeltaTime;
	while ( fArrowHeight < 32.0f ) fArrowHeight += 32.0f;
	m_oArrow.SetHeight( fArrowHeight );

	// Fade the text out
	if ( m_fTextShowTime > 0.0f )
	{
		m_fTextShowTime -= a_fDeltaTime;
		if ( m_fTextShowTime <= 0.0f ) m_fTextShowTime = 0.0f;
	}

	if ( 0.0f == m_fTextShowTime )
	{
		TFLOAT fAlpha = m_oText.GetAlpha() - a_fDeltaTime / m_fTextFadeTime;

		if ( fAlpha <= 0.0f )
		{
			fAlpha          = 0.0f;
			m_fTextShowTime = -1.0f;
		}

		m_oText.SetAlpha( fAlpha );
	}

	// Fade the bar
	if ( m_fFadeDelay > 0.0f )
	{
		m_fFadeDelay -= a_fDeltaTime;
		if ( m_fFadeDelay <= 0.0f ) m_fFadeDelay = 0.0f;
	}

	if ( 0.0f == m_fFadeDelay )
	{
		TFLOAT fAlpha = m_fAlphaChange / m_fFadeTime * a_fDeltaTime + m_oBar.GetAlpha();

		if ( fAlpha <= 0.0f )
		{
			fAlpha       = 0.0f;
			m_fFadeDelay = -1.0f;
		}
		else if ( fAlpha >= 1.0f )
		{
			fAlpha       = 1.0f;
			m_fFadeDelay = -1.0f;
		}

		m_oBar.SetAlpha( fAlpha );
		SetChildrenAlpha( &m_oBar, fAlpha );
	}

	// Pulse the results
	for ( TINT i = 0; i < NUM_RESULTS; i++ )
	{
		TFLOAT fSize = TMath::Max( m_aResults[ i ].GetHeight() - ( s_fArrowScrollSpeed * a_fDeltaTime + s_fArrowScrollSpeed * a_fDeltaTime ), 32.0f );
		m_aResults[ i ].SetDimensions( fSize, fSize );
	}
}

// $Barnyard: FUNCTION 0044f790
void AButtonSlamHudElement::SetBracketPos( TFLOAT a_fPos )
{
	m_fBracketPos = a_fPos;

	if ( a_fPos >= 0.0f )
	{
		m_oBracket.SetTranslation( s_fBarLength * a_fPos + s_fBracketX - m_oBracket2.GetWidth() * 0.5f, s_fBracketY );
		m_aResults[ 1 ].SetTranslation( s_fBarLength * m_fBracketPos + s_fBracketX - m_aResults[ 1 ].GetWidth() * 0.5f, 0.0f );
	}
	else
	{
		m_oBar.Hide();
	}
}

// $Barnyard: FUNCTION 0044f840
void AButtonSlamHudElement::SetBracket2Pos( TFLOAT a_fPos )
{
	m_fBracket2Pos = a_fPos;

	if ( a_fPos >= 0.0f )
	{
		m_oBracket2.Show();
		m_oBracket2.SetTranslation( s_fBarLength * m_fBracket2Pos + s_fBracketX - m_oBracket2.GetWidth() * 0.5f, s_fBracketY );
		m_aResults[ 0 ].SetTranslation( s_fBarLength * m_fBracket2Pos + s_fBracketX - m_aResults[ 0 ].GetWidth() * 0.5f, 0.0f );
	}
	else
	{
		m_oBracket2.Hide();
	}
}

// $Barnyard: FUNCTION 0044f900
void AButtonSlamHudElement::SetBracketsVisible( TBOOL a_bVisible )
{
	m_oBracket.SetVisible( a_bVisible == TTRUE );
	m_oBracket2.SetVisible( a_bVisible == TTRUE );
}

// $Barnyard: FUNCTION 0044f950
void AButtonSlamHudElement::SetArrowTarget( TINT a_iBracket )
{
	if ( a_iBracket == -1 )
	{
		m_oArrowRoot.Hide();
	}
	else if ( a_iBracket == 0 )
	{
		m_oArrowRoot.Show();
		TFLOAT fX = m_oBracket.GetTransform().GetTranslation().x + m_oBracket.GetWidth() * 0.5f;
		m_oArrowRoot.SetTransform( fX - m_oArrowRoot.GetWidth() * 0.5f, 0.0f, 0.0f );
		m_oArrow.SetHeight( 64.0f );
	}
	else if ( a_iBracket == 1 )
	{
		m_oArrowRoot.Show();
		TFLOAT fX = m_oBracket2.GetTransform().GetTranslation().x + m_oBracket.GetWidth() * 0.5f;
		m_oArrowRoot.SetTransform( fX - m_oArrowRoot.GetWidth() * 0.5f, 0.0f, 0.0f );
		m_oArrow.SetHeight( 64.0f );
	}
}

// $Barnyard: FUNCTION 0044fa70
void AButtonSlamHudElement::SetResult( TUINT a_uiIndex, TBOOL a_bTick )
{
	if ( a_uiIndex > 1 )
	{
		for ( TINT i = 0; i < NUM_RESULTS; i++ )
			m_aResults[ i ].Hide();
	}
	else
	{
		T2GUIRectangle& rResult = m_aResults[ a_uiIndex ];
		rResult.Show();
		rResult.SetTextureSection( T2GUITextureSectionManager::GetTextureSection( a_bTick ? "Golf_Tick" : "Golf_Cross" ) );
		rResult.SetDimensions( 48.0f, 48.0f );
	}
}

// $Barnyard: FUNCTION 0044fae0
void AButtonSlamHudElement::SetText( const TWCHAR* a_wszText, TFLOAT a_fShowTime, TFLOAT a_fFadeTime )
{
	if ( a_wszText )
	{
		m_oText.SetText( a_wszText );
		m_oText.Show();
		m_fTextShowTime = a_fShowTime;
		m_fTextFadeTime = a_fFadeTime;
	}
	else
	{
		m_oText.Hide();
	}

	m_oText.SetAlpha( 1.0f );
}

// $Barnyard: FUNCTION 0044fb40
void AButtonSlamHudElement::SetChildrenAlpha( T2GUIElement* a_pElement, TFLOAT a_fAlpha )
{
	for ( T2GUIElement* pChild = a_pElement->ChildRBegin(); pChild; pChild = a_pElement->GetPrevChild( pChild ) )
	{
		pChild->SetAlpha( a_fAlpha );
		SetChildrenAlpha( pChild, a_fAlpha );
	}
}

// $Barnyard: FUNCTION 0044fb80
void AButtonSlamHudElement::SetAlpha( TFLOAT a_fAlpha )
{
	m_oBar.SetAlpha( a_fAlpha );
	SetChildrenAlpha( &m_oBar, a_fAlpha );
}

// $Barnyard: FUNCTION 0044fba0
void AButtonSlamHudElement::StartFade( TFLOAT a_fDelay, TFLOAT a_fFadeTime, TFLOAT a_fAlphaChange )
{
	if ( a_fDelay < 0.0f )
	{
		m_oBar.SetAlpha( 1.0f );
		SetChildrenAlpha( &m_oBar, 1.0f );
	}

	m_fFadeDelay   = a_fDelay;
	m_fFadeTime    = a_fFadeTime;
	m_fAlphaChange = a_fAlphaChange;
}

// $Barnyard: FUNCTION 0044fc00
void AButtonSlamHudElement::SetOrientation( TBOOL a_bHorizontal, const TCHAR* a_szBand, const TCHAR* a_szSize )
{
	if ( m_bHorizontal == a_bHorizontal )
		return;

	m_bHorizontal = a_bHorizontal;

	T2GUITextureSection* pBandSection = T2GUITextureSectionManager::GetTextureSection( a_szBand );
	T2GUITextureSection* pSizeSection = T2GUITextureSectionManager::GetTextureSection( a_szSize );

	m_oBand.GetTransform().SetIdentity();

	TFLOAT fBandWidth     = TFLOAT( pBandSection->GetPoint2X() - pBandSection->GetPoint1X() );
	TFLOAT fBandThickness = TMath::Abs( TFLOAT( pSizeSection->GetPoint2Y() - pSizeSection->GetPoint1Y() ) );

	if ( !a_bHorizontal )
	{
		m_oBand.SetTextureSection( pBandSection );
		m_oBandTarget.SetTextureSection( pBandSection );
		m_oBand.SetTranslation( 54.0f, 38.0f );
		m_oBand.GetTransform().Rotate( -TMath::HALF_PI );
		m_oBand.SetDimensions( TMath::Abs( fBandWidth ), s_fBarLength );
	}
	else
	{
		m_oBand.SetTextureSection( pBandSection );
		m_oBandTarget.SetTextureSection( pBandSection );
		m_oBand.SetTranslation( 54.0f, 38.0f - fBandThickness );
		m_oBand.SetDimensions( s_fHorizontalWidth, fBandThickness + 1.0f );

		TVector2 vUV1, vUV3;
		m_oBand.GetUVs( vUV1, m_vBandUV2, vUV3, m_vBandUV4 );
	}
}
