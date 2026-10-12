#include "pch.h"
#include "ADartFireHudElement.h"
#include "GUI/AGUI2.h"
#include "Cameras/ACameraManager.h"
#include "Render/ARenderer.h"

#include <GUI/T2GUITextureSectionManager.h>
#include <GUI/T2GUIFontManager.h>
#include <Toshi/TStringManager.h>
#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( ADartFireHudElement );

// Scale of the bullseye textures (00773668)
static constexpr TFLOAT BULLSEYE_SCALE = 0.3f;

static TUINT32 MakeColour( TFLOAT a_fRed, TFLOAT a_fGreen, TFLOAT a_fBlue )
{
	return ( ( ( TUINT32( a_fRed * 255.0f ) | 0xFFFFFF00 ) << 8 | TUINT32( a_fGreen * 255.0f ) ) << 8 ) | TUINT32( a_fBlue * 255.0f );
}

// $Barnyard: FUNCTION 004517b0
ADartFireHudElement::ADartFireHudElement()
    : AHudElement( MASK_UNK4 )
{
}

// $Barnyard: FUNCTION 00451d60
// $Barnyard: FUNCTION 004518a0
ADartFireHudElement::~ADartFireHudElement()
{
	m_oBoard.RemoveSelf();
}

// $Barnyard: FUNCTION 00451a40
void ADartFireHudElement::Canvas::Render()
{
	// NOTE: the canvas is a member of the HUD element at 0x14
	ADartFireHudElement* pHud = TREINTERPRETCAST( ADartFireHudElement*, TREINTERPRETCAST( TUINTPTR, this ) - 0x14 );

	T2GUIRectangle* apRects[] = { &pHud->m_oBullseyeMask, &pHud->m_oBullseye, &pHud->m_oBarY, &pHud->m_oBarX };

	for ( TINT i = 0; i < TARRAYSIZE( apRects ); i++ )
	{
		if ( apRects[ i ]->IsVisible() )
		{
			apRects[ i ]->PreRender();
			apRects[ i ]->Render();
			apRects[ i ]->PostRender();
		}
	}

	T2GUIElement::Render();
}

// $Barnyard: FUNCTION 00451d80
TBOOL ADartFireHudElement::OnAttach()
{
	T2GUITextureSection* pMaskSection = T2GUITextureSectionManager::GetTextureSection( "Bullseye_Mask" );
	m_oBullseyeMask.SetDimensions(
	    TMath::Abs( TFLOAT( pMaskSection->GetPoint2X() - pMaskSection->GetPoint1X() ) ) * BULLSEYE_SCALE,
	    TMath::Abs( TFLOAT( pMaskSection->GetPoint2Y() - pMaskSection->GetPoint1Y() ) ) * BULLSEYE_SCALE
	);
	m_oBullseyeMask.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_oBullseyeMask.SetTextureSection( pMaskSection );
	m_oBullseyeMask.GetMaterial()->SetBlendState( T2GUIMaterial::BLENDSTATE_MASK );

	T2GUITextureSection* pBullseyeSection = T2GUITextureSectionManager::GetTextureSection( "Bullseye" );
	m_oBullseye.SetDimensions(
	    TMath::Abs( TFLOAT( pBullseyeSection->GetPoint2X() - pBullseyeSection->GetPoint1X() ) ) * BULLSEYE_SCALE,
	    TMath::Abs( TFLOAT( pBullseyeSection->GetPoint2Y() - pBullseyeSection->GetPoint1Y() ) ) * BULLSEYE_SCALE
	);
	m_oBullseye.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_oBullseye.SetTextureSection( pBullseyeSection );
	m_oBullseye.GetMaterial()->SetBlendState( T2GUIMaterial::BLENDSTATE_MASKED );

	m_oBullseye.Show();
	m_oBullseyeMask.Show();
	m_oCanvas.Show();

	m_oBoard.SetDimensions( 10.0f, 10.0f );
	m_oBoard.SetTransform( 0.0f, 0.0f, 0.0f );
	m_pParent->AddChildTail( m_oBoard );
	m_oBoard.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_oBoard.SetAlpha( 0.0f );
	m_oBoard.SetTransform( 0.0f, 0.0f, 0.0f );

	m_oScoreFrame.SetDimensions( 256.0f, 128.0f );
	m_oScoreFrame.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_oScoreFrame.SetAlpha( 0.0f );

	T2GUITextureSection* pLockSection = T2GUITextureSectionManager::GetTextureSection( "Darts_Target_Lock" );
	m_oTargetLock.SetDimensions(
	    TMath::Abs( TFLOAT( pLockSection->GetPoint2X() - pLockSection->GetPoint1X() ) ),
	    TMath::Abs( TFLOAT( pLockSection->GetPoint2Y() - pLockSection->GetPoint1Y() ) )
	);
	m_oTargetLock.SetTextureSection( pLockSection );
	m_oTargetLock.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_oTargetLock.Hide();
	m_oCanvas.AddChildTail( m_oTargetLock );

	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord26" );
	m_oScoreText.Create( pFont, m_oScoreFrame.GetWidth() );
	m_oScoreText.SetScale( 1.5f );
	m_oScoreText.SetColour( 0xFFE60000 );
	m_oScoreText.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oScoreFrame.AddChildTail( m_oScoreText );
	m_oCanvas.AddChildTail( m_oScoreFrame );

	T2GUITextureSection* pBarSection = T2GUITextureSectionManager::GetTextureSection( "Darts_Bar" );
	TFLOAT               fBarHeight  = TFLOAT( pBarSection->GetPoint2Y() - pBarSection->GetPoint1Y() );
	m_fBarLength                     = m_pParent->GetWidth();
	m_oBarY.SetDimensions( m_fBarLength, TMath::Abs( fBarHeight ) );
	m_oBarY.SetTextureSection( pBarSection );
	m_oBarY.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );

	pBarSection = T2GUITextureSectionManager::GetTextureSection( "Darts_Bar" );
	m_oBarX.SetDimensions( m_fBarLength, TMath::Abs( TFLOAT( pBarSection->GetPoint2Y() - pBarSection->GetPoint1Y() ) ) );
	m_oBarX.SetTextureSection( pBarSection );

	m_oBarY.Hide();
	m_oBarX.Hide();

	m_oBarY.SetTransform( 0.0f, 0.0f, TMath::PI * 0.5f );
	m_oBarX.SetTransform( 0.0f, 0.0f, 0.0f );

	m_oBarY.GetMaterial()->SetBlendState( T2GUIMaterial::BLENDSTATE_MASKED );
	m_oBarX.GetMaterial()->SetBlendState( T2GUIMaterial::BLENDSTATE_MASKED );

	m_oBarY.SetColour( 0xD8FFD800 );
	m_oBarX.SetColour( 0xD8FFD800 );
	m_oBullseye.SetColour( 0xD8FFD800 );

	m_pParent->AddChildHead( m_oCanvas );

	return TTRUE;
}

// $Barnyard: FUNCTION 004519b0
void ADartFireHudElement::OnDetach()
{
	m_oTargetLock.RemoveSelf();
	m_oBarY.RemoveSelf();
	m_oBarX.RemoveSelf();
	m_oBullseyeMask.RemoveSelf();
	m_oBullseye.RemoveSelf();
}

// $Barnyard: FUNCTION 00451a10
void ADartFireHudElement::Show()
{
	m_oBoard.Show();
	m_bCurrentlyVisible = TTRUE;
}

// $Barnyard: FUNCTION 00451a30
void ADartFireHudElement::Hide()
{
	m_oBoard.Hide();
	m_bCurrentlyVisible = TFALSE;
}

// $Barnyard: FUNCTION 00451b10
void ADartFireHudElement::SetBarYVisible( TBOOL a_bVisible )
{
	m_oBarY.SetVisible( a_bVisible == TTRUE );
}

// $Barnyard: FUNCTION 00451b30
void ADartFireHudElement::SetBarXVisible( TBOOL a_bVisible )
{
	m_oBarX.SetVisible( a_bVisible == TTRUE );
}

// $Barnyard: FUNCTION 00451b50
void ADartFireHudElement::SetBarXLocked( TBOOL a_bLocked )
{
	m_oBarX.SetColour( a_bLocked ? 0xFFFFD800 : 0xFFFFFFFF );
}

// $Barnyard: FUNCTION 00451b90
void ADartFireHudElement::SetColour( TFLOAT a_fRed, TFLOAT a_fGreen, TFLOAT a_fBlue )
{
	TUINT32 uiColour = MakeColour( a_fRed, a_fGreen, a_fBlue );

	m_oBarY.SetColour( uiColour );
	m_oBarX.SetColour( uiColour );
	m_oBullseye.SetColour( uiColour );
}

// $Barnyard: FUNCTION 00451c10
void ADartFireHudElement::SetTargetLockVisible( TBOOL a_bVisible )
{
	m_oTargetLock.SetVisible( a_bVisible == TTRUE );
}

// $Barnyard: FUNCTION 00451c30
void ADartFireHudElement::ResetColour()
{
	m_oBarY.SetColour( 0xFFFFFFFF );
	m_oBarX.SetColour( 0xFFFFFFFF );
	m_oBullseye.SetColour( 0xFFFFFFFF );
}

// $Barnyard: FUNCTION 00451c70
void ADartFireHudElement::SetTargetLockAlpha( TFLOAT a_fAlpha )
{
	m_oTargetLock.SetAlpha( a_fAlpha );
}

// $Barnyard: FUNCTION 00451c80
void ADartFireHudElement::SetBullseyeVisible( TBOOL a_bVisible )
{
	m_oBullseye.SetVisible( a_bVisible == TTRUE );
	m_oBullseyeMask.SetVisible( a_bVisible == TTRUE );
}

// $Barnyard: FUNCTION 00451cd0
void ADartFireHudElement::SetBullseyePosition( TFLOAT a_fAimX, TFLOAT a_fAimY )
{
	TFLOAT fHalfWidth = m_oBoard.GetWidth() * 0.5f;
	TFLOAT fY         = m_oBoard.GetHeight() * 0.5f * a_fAimY;
	TFLOAT fX         = fHalfWidth * a_fAimX;

	m_oBullseye.SetTransform( fX, fY, 0.0f );
	m_oBullseyeMask.SetTransform( fX, fY, 0.0f );
}

// $Barnyard: FUNCTION 00451d40
void ADartFireHudElement::SetScoreAlpha( TFLOAT a_fAlpha )
{
	m_oScoreText.SetAlpha( a_fAlpha );
}

// $Barnyard: FUNCTION 00452110
void ADartFireHudElement::SetBarY( TFLOAT a_fAimY )
{
	m_oBarY.SetTransform( TVector2::VEC_ZERO.x, m_oBoard.GetHeight() * 0.5f * a_fAimY + TVector2::VEC_ZERO.y, 0.0f );
}

// $Barnyard: FUNCTION 00452160
void ADartFireHudElement::SetBarX( TFLOAT a_fAimX )
{
	m_oBarX.SetTransform( m_oBoard.GetWidth() * 0.5f * a_fAimX + TVector2::VEC_ZERO.x, TVector2::VEC_ZERO.y, TMath::PI * 0.5f );
}

// $Barnyard: FUNCTION 004521c0
void ADartFireHudElement::SetTargetLockPosition( const TVector4& a_rcPosition )
{
	TVector2 vScreen;
	ARenderer::GetSingleton()->ProjectToGUI( ACameraManager::GetSingleton()->GetCurrentCamera(), a_rcPosition, vScreen );

	TFLOAT fWidth;
	TFLOAT fHeight;
	AGUI2::GetRootElement()->GetDimensions( fWidth, fHeight );

	m_oTargetLock.SetTransform( vScreen.x - fWidth * 0.5f, -( fHeight * 0.5f - vScreen.y ), 0.0f );
}

// $Barnyard: FUNCTION 00452240
void ADartFireHudElement::ShowScore( TFLOAT a_fAimX, TFLOAT a_fAimY, TINT a_iScore, const TWCHAR* a_wszText, TFLOAT a_fRed, TFLOAT a_fGreen, TFLOAT a_fBlue )
{
	TFLOAT fHalfWidth  = m_oBoard.GetWidth() * 0.5f;
	TFLOAT fHalfHeight = m_oBoard.GetHeight() * 0.5f;

	TStringManager::String16Format( m_wszScore, 16, L"+%i ", a_iScore );
	TStringManager::String16Format( m_wszText, 16, L"%s", a_wszText );

	m_oScoreText.SetColour( ( ( ( TUINT32( a_fRed * 255.0f ) | 0xFFFFD800 ) << 8 | TUINT32( a_fGreen * 255.0f ) ) << 8 ) | TUINT32( a_fBlue * 255.0f ) );
	m_oScoreText.SetTextAndFitWidth( m_wszScore );

	m_oScoreFrame.SetTransform( fHalfWidth * a_fAimX, fHalfHeight * a_fAimY, 0.0f );
	m_oScoreText.SetAlpha( 1.0f );
}

// $Barnyard: FUNCTION 00452360
void ADartFireHudElement::SetBoardCorners( const TVector4& a_rcTop, const TVector4& a_rcRight, const TVector4& a_rcLeft, const TVector4& a_rcBottom )
{
	const TVector4* apCorners[ 4 ] = { &a_rcTop, &a_rcRight, &a_rcLeft, &a_rcBottom };
	TVector2        aScreen[ 4 ];

	m_vBoardPosition = TVector2::VEC_ZERO;

	for ( TINT i = 0; i < 4; i++ )
	{
		ARenderer::GetSingleton()->ProjectToGUI( ACameraManager::GetSingleton()->GetCurrentCamera(), *apCorners[ i ], aScreen[ i ] );
		m_vBoardPosition.x += aScreen[ i ].x;
		m_vBoardPosition.y += aScreen[ i ].y;
	}

	m_vBoardPosition.x *= 0.25f;
	m_vBoardPosition.y *= 0.25f;

	TFLOAT fX          = m_vBoardPosition.x;
	m_vBoardPosition.x = fX - m_pParent->GetWidth() * 0.5f;
	TFLOAT fY          = m_vBoardPosition.y;
	m_vBoardPosition.y = fY - m_pParent->GetHeight() * 0.5f;

	m_oBoard.SetDimensions( TMath::Abs( aScreen[ 2 ].x - aScreen[ 1 ].x ), TMath::Abs( aScreen[ 0 ].y - aScreen[ 3 ].y ) );
	m_oBoard.SetTransform( m_vBoardPosition.x, m_vBoardPosition.y, 0.0f );

	T2GUITextureSection* pBarSection = T2GUITextureSectionManager::GetTextureSection( "Darts_Bar" );
	TFLOAT               fBarHeight  = TMath::Abs( TFLOAT( pBarSection->GetPoint2Y() - pBarSection->GetPoint1Y() ) );

	m_fBarLength = m_oBoard.GetWidth() * 1.3f;
	m_oBarY.SetDimensions( m_fBarLength, fBarHeight );
	m_oBarX.SetDimensions( m_fBarLength, fBarHeight );
}
