#include "pch.h"
#include "AGUI2TargetIndicator.h"
#include "AGUI2.h"
#include "Locale/ALocale.h"
#include "Locale/ALocaleManager.h"
#include "Render/ARenderer.h"

#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Toshi/TStringManager.h>
#include <Plugins/PPropertyParser/PPropertyValue.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 006480b0
AGUI2TargetIndicator::AGUI2TargetIndicator()
    : m_vPosition( TVector4::VEC_ZERO )
    , m_eSide( SIDE_NONE )
    , m_bVisible( TFALSE )
{
}

// $Barnyard: FUNCTION 00648150
AGUI2TargetIndicator::~AGUI2TargetIndicator()
{
	m_oText.RemoveSelf();
	m_oPointer.RemoveSelf();
	m_oPointerContainer.RemoveSelf();
	m_oRoot.RemoveSelf();
}

// $Barnyard: FUNCTION 00648580
TBOOL AGUI2TargetIndicator::Create( T2GUIElement* a_pParent )
{
	if ( a_pParent == TNULL )
		a_pParent = AGUI2::GetRootElement();

	m_eSide    = SIDE_NONE;
	m_bVisible = TFALSE;

	ms_pRedPoint    = T2GUITextureSectionManager::GetTextureSection( "Putt_Red_Point" );
	ms_pYellowPoint = T2GUITextureSectionManager::GetTextureSection( "Putt_Yel_Point" );

	T2GUIElement* pRoot  = AGUI2::GetRootElement();
	TFLOAT        fHalfH = pRoot->GetHeight() * 0.5f;
	m_vScreenPosition.x  = pRoot->GetWidth() * 0.5f;
	m_vScreenPosition.y  = fHalfH;
	m_vScreenPosition.z  = 0.0f;
	m_vScreenPosition.w  = 1.0f;
	m_fWidth             = 0.0f;
	m_fHeight            = 0.0f;

	ALocale::FormatTags( m_wszText, MAX_TEXT_LENGTH, 254, PPropertyValue( 0.0f ) );

	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord18" );
	m_fHeight        = pFont->GetTextHeightWrapped( m_wszText, pFont->GetTextWidth( m_wszText, 1.0f ), 1.0f ) + 4.0f;
	m_fWidth         = pFont->GetTextWidth( m_wszText, 1.0f ) + 4.0f;

	m_oText.Create( pFont, pFont->GetTextWidth( m_wszText, 1.0f ) );
	m_oText.SetText( m_wszText );

	// The pointer is 3% of the screen width
	m_oPointerContainer.SetDimensions( AGUI2::GetRootElement()->GetWidth() * 0.03f, AGUI2::GetRootElement()->GetWidth() * 0.03f );
	m_oPointer.SetDimensions( AGUI2::GetRootElement()->GetWidth() * 0.03f, AGUI2::GetRootElement()->GetWidth() * 0.03f );
	m_oPointer.SetTextureSection( ms_pRedPoint );
	m_oPointerContainer.AddChildTail( m_oPointer );

	// NOTE: retail adds the width of the pointer to the height and the height to the width
	TFLOAT fPointerWidth, fPointerHeight;
	m_oPointerContainer.GetDimensions( fPointerWidth, fPointerHeight );
	m_fHeight = fPointerWidth + fPointerWidth + m_fHeight;
	m_fWidth  = fPointerHeight + fPointerHeight + m_fWidth;
	m_oPointerContainer.SetAlpha( 0.0f );

	m_oRoot.SetDimensions( m_fWidth, m_fHeight );
	m_oRoot.AddChildTail( m_oText );
	m_oRoot.AddChildTail( m_oPointerContainer );
	m_oRoot.SetAlpha( 0.0f );
	a_pParent->AddChildTail( m_oRoot );

	SetPointerSide( SIDE_DEFAULT );

	// NOTE: inlined SetVisible( TFALSE ) except that the text is hidden unconditionally
	m_bVisible = TFALSE;
	m_oText.Hide();
	m_oPointer.SetVisible( m_bVisible == TTRUE );
	m_oPointerContainer.SetVisible( m_bVisible == TTRUE );
	m_oRoot.SetVisible( m_bVisible == TTRUE );

	return TTRUE;
}

// $Barnyard: FUNCTION 00648970
void AGUI2TargetIndicator::Update( TFLOAT a_fDeltaTime )
{
	if ( !m_bVisible )
		return;

	TVector2 vScreenPos;
	ARenderer::GetSingleton()->GetScreenPosition( m_vPosition, vScreenPos );
	m_vScreenPosition.x = vScreenPos.x;
	m_vScreenPosition.y = vScreenPos.y;

	T2GUIElement* pRoot = AGUI2::GetRootElement();
	TMath::Clip( m_vScreenPosition.x, 0.0f, pRoot->GetWidth() );
	TMath::Clip( m_vScreenPosition.y, 0.0f, pRoot->GetHeight() );

	// Yellow if the position is on (or clamped to) the edge of the screen
	if ( m_vScreenPosition.x <= 0.0f || m_vScreenPosition.y <= 0.0f ||
	     pRoot->GetWidth() <= m_vScreenPosition.x || pRoot->GetHeight() <= m_vScreenPosition.y )
		m_oPointer.SetTextureSection( ms_pYellowPoint );
	else
		m_oPointer.SetTextureSection( ms_pRedPoint );

	// Turn the pointer towards the closest edge of the screen
	if ( m_vScreenPosition.x < m_fWidth )
	{
		if ( m_vScreenPosition.y < m_fHeight )
			SetPointerSide( SIDE_TOPLEFT );
		else if ( pRoot->GetHeight() - m_fHeight >= m_vScreenPosition.y )
			SetPointerSide( SIDE_LEFT );
		else
			SetPointerSide( SIDE_BOTTOMLEFT );
	}
	else if ( pRoot->GetWidth() - m_fWidth < m_vScreenPosition.x )
	{
		if ( m_vScreenPosition.y < m_fHeight )
			SetPointerSide( SIDE_TOPRIGHT );
		else if ( pRoot->GetHeight() - m_fHeight >= m_vScreenPosition.y )
			SetPointerSide( SIDE_RIGHT );
		else
			SetPointerSide( SIDE_BOTTOMRIGHT );
	}
	else
	{
		if ( m_vScreenPosition.y < m_fHeight )
			SetPointerSide( SIDE_TOP );
		else if ( pRoot->GetHeight() - m_fHeight >= m_vScreenPosition.y )
			SetPointerSide( SIDE_DEFAULT );
		else
			SetPointerSide( SIDE_BOTTOM );
	}

	m_oRoot.GetTransform().SetTranslation( m_vScreenPosition.x, m_vScreenPosition.y );
}

// $Barnyard: FUNCTION 00648500
void AGUI2TargetIndicator::SetVisible( TBOOL a_bVisible )
{
	m_bVisible = a_bVisible;
	m_oText.SetVisible( m_bVisible == TTRUE );
	m_oPointer.SetVisible( m_bVisible == TTRUE );
	m_oPointerContainer.SetVisible( m_bVisible == TTRUE );
	m_oRoot.SetVisible( m_bVisible == TTRUE );
}

// $Barnyard: FUNCTION 00648200
void AGUI2TargetIndicator::SetText( const TWCHAR* a_wszText, const TVector4& a_rcPosition )
{
	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord18" );
	m_oText.Create( pFont, pFont->GetTextWidth( a_wszText, 1.0f ) );
	m_oText.SetText( a_wszText );

	// NOTE: retail measures the height of m_wszText, not a_wszText (they are the same in SetDistance)
	m_fHeight = pFont->GetTextHeightWrapped( m_wszText, pFont->GetTextWidth( a_wszText, 1.0f ), 1.0f ) + 4.0f;
	m_fWidth  = pFont->GetTextWidth( a_wszText, 1.0f ) + 4.0f;

	// NOTE: retail adds the width of the pointer to the height and the height to the width
	TFLOAT fPointerWidth, fPointerHeight;
	m_oPointerContainer.GetDimensions( fPointerWidth, fPointerHeight );
	m_fHeight = fPointerWidth + fPointerWidth + m_fHeight;
	m_fWidth  = fPointerHeight + fPointerHeight + m_fWidth;

	m_oRoot.SetDimensions( m_fWidth, m_fHeight );
	m_vPosition = a_rcPosition;
}

// $Barnyard: FUNCTION 006488c0
void AGUI2TargetIndicator::SetDistance( const TVector4& a_rcPosition, TFLOAT a_fDistance )
{
	ALocaleManager::Lang eLang = g_pLocaleManager->GetLanguage();

	if ( eLang == ALocaleManager::Lang_Spanish || eLang == ALocaleManager::Lang_Italian )
	{
		// Decimal comma
		TINT iWhole = TMath::FloorToInt( a_fDistance );
		TINT iFrac  = TMath::FloorToInt( ( a_fDistance - iWhole ) * 100.0f );
		TStringManager::String16Format( m_wszText, MAX_TEXT_LENGTH, L"%d,%02d", iWhole, iFrac );
	}
	else
	{
		TStringManager::String16Format( m_wszText, MAX_TEXT_LENGTH, L"%0.2f", a_fDistance );
	}

	// The unit
	TINT iLength = TStringManager::String16Length( m_wszText );
	TStringManager::String16CopySafe( m_wszText + iLength, ALocaleManager::Interface()->GetString( 1181 ), MAX_TEXT_LENGTH - iLength );

	SetText( m_wszText, a_rcPosition );
}

// $Barnyard: FUNCTION 00648310
void AGUI2TargetIndicator::SetPointerSide( SIDE a_eSide )
{
	if ( m_eSide == a_eSide )
		return;

	m_eSide = a_eSide;
	m_oText.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_oPointer.GetTransform().SetIdentity();
	m_oPointer.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );

	// The pointer container is attached to the corner/side of the label that is pivoted on the screen position
	switch ( a_eSide )
	{
		case SIDE_TOPRIGHT:
			m_oPointerContainer.SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT );
			m_oPointer.GetTransform().Rotate( TMath::HALF_PI * 0.5f );
			m_oRoot.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPRIGHT );
			break;
		case SIDE_TOP:
			m_oPointerContainer.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
			m_oRoot.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPCENTER );
			break;
		case SIDE_TOPLEFT:
		case SIDE_DEFAULT:
			m_oPointerContainer.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
			m_oPointer.GetTransform().Rotate( TMath::HALF_PI * -0.5f );
			m_oRoot.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
			break;
		case SIDE_LEFT:
			m_oPointerContainer.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
			m_oPointer.GetTransform().Rotate( -TMath::HALF_PI );
			m_oRoot.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_MIDDLELEFT );
			break;
		case SIDE_BOTTOMLEFT:
			m_oPointerContainer.SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_BOTTOMLEFT );
			m_oPointer.GetTransform().Rotate( TMath::HALF_PI * -1.5f );
			m_oRoot.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_BOTTOMLEFT );
			break;
		case SIDE_BOTTOM:
			m_oPointerContainer.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
			m_oPointer.GetTransform().Rotate( TMath::PI );
			m_oRoot.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_BOTTOMCENTER );
			break;
		case SIDE_BOTTOMRIGHT:
			m_oPointerContainer.SetAttachment( T2GUIATTACHMENT_BOTTOMRIGHT, T2GUIATTACHMENT_BOTTOMRIGHT );
			m_oPointer.GetTransform().Rotate( TMath::HALF_PI * 1.5f );
			m_oRoot.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_BOTTOMRIGHT );
			break;
		case SIDE_RIGHT:
			m_oPointerContainer.SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLERIGHT );
			m_oPointer.GetTransform().Rotate( TMath::HALF_PI );
			m_oRoot.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_MIDDLERIGHT );
			break;
	}
}
