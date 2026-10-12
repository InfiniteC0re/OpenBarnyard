#include "pch.h"
#include "AGUI2RowLifeHud.h"

#include <GUI/T2GUITextureSectionManager.h>
#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// 007798f8
static const TCHAR* s_apBreakFrames[ AGUI2RowLifeHud::NUM_BREAK_FRAMES ] = {
	"Break_Glass_01",
	"Break_Glass_02",
	"Break_Glass_03",
	"Break_Glass_04"
};

// $Barnyard: FUNCTION 00649e80
AGUI2RowLifeHud::AGUI2RowLifeHud()
    : m_bCreated( TFALSE )
    , m_iLives( 0 )
    , m_uiBreakingMask( 0 )
    , m_fScale( 1.0f )
{
}

// $Barnyard: FUNCTION 00649f40
// $Barnyard: FUNCTION 00649a80
AGUI2RowLifeHud::~AGUI2RowLifeHud()
{
	if ( m_bCreated )
	{
		m_bCreated = TFALSE;
		TFree( m_ppBreakFrames );
	}
}

// $Barnyard: FUNCTION 0064a070
void AGUI2RowLifeHud::Tick( TFLOAT a_fDeltaTime )
{
	for ( TINT i = 0; i < MAX_LIVES; i++ )
		UpdateBreaking( a_fDeltaTime, i );
}

// $Barnyard: FUNCTION 00649b30
void AGUI2RowLifeHud::Create( T2GUIElement* a_pParent, T2GUIATTACHMENT a_eAnchor, T2GUIATTACHMENT a_ePivot, TINT a_iMaxLives, const TCHAR* a_szGlass, TFLOAT a_fScale, TFLOAT a_fOffsetY, TBOOL a_bDontScaleBroken )
{
	m_fScale    = a_fScale;
	m_iMaxLives = a_iMaxLives;
	m_bCreated  = TTRUE;

	// The plank
	T2GUITextureSection* pWood = T2GUITextureSectionManager::GetTextureSection( "Score_Wood" );
	m_oBacking.T2GUIElement::SetDimensions(
	    TMath::Abs( TFLOAT( pWood->GetPoint2X() - pWood->GetPoint1X() ) ),
	    TMath::Abs( TFLOAT( pWood->GetPoint2Y() - pWood->GetPoint1Y() ) )
	);
	m_oBacking.SetTextureSection( pWood );
	m_oBacking.SetAttachment( a_eAnchor, a_ePivot );
	m_oBacking.SetTranslation( 0.0f, a_fOffsetY );
	a_pParent->AddChildTail( m_oBacking );

	// The glasses
	T2GUITextureSection* pGlass  = T2GUITextureSectionManager::GetTextureSection( a_szGlass );
	T2GUITextureSection* pBroken = T2GUITextureSectionManager::GetTextureSection( "Tap_Broken_Glass" );

	TFLOAT fGlassHeight = TMath::Abs( TFLOAT( pGlass->GetPoint2Y() - pGlass->GetPoint1Y() ) );
	TFLOAT fWave        = TMath::Abs( TFLOAT( pGlass->GetPoint2X() - pGlass->GetPoint1X() ) ) * 0.125f;
	TFLOAT fX           = 5.0f;

	for ( TINT i = 0; i < m_iMaxLives; i++ )
	{
		TFLOAT fY = TMath::Abs( TMath::Sin( i * TMath::PI * 0.5f ) ) * fWave - 8.0f;
		TFLOAT fWidth, fHeight;

		m_aGlasses[ i ].SetTextureSection( *pGlass );
		m_aGlasses[ i ].GetDimensions( fWidth, fHeight );
		m_aGlasses[ i ].SetDimensions( fWidth * m_fScale, fHeight * m_fScale );
		m_aGlasses[ i ].SetTransform( fX, fY, 0.0f );
		m_aGlasses[ i ].SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );

		m_aBrokenGlasses[ i ].SetTextureSection( *pBroken );
		m_aBrokenGlasses[ i ].GetDimensions( fWidth, fHeight );

		if ( !a_bDontScaleBroken )
			m_aBrokenGlasses[ i ].SetDimensions( fWidth * m_fScale, fHeight * m_fScale );

		m_aBrokenGlasses[ i ].SetTransform( fX, fY, 0.0f );
		m_aBrokenGlasses[ i ].SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
		m_aBrokenGlasses[ i ].Hide();

		m_aBreakingGlasses[ i ].SetTextureSection( *pBroken );
		m_aBreakingGlasses[ i ].GetDimensions( fWidth, fHeight );

		if ( !a_bDontScaleBroken )
			m_aBreakingGlasses[ i ].SetDimensions( fWidth * m_fScale, fHeight * m_fScale );

		m_aBreakingGlasses[ i ].SetTransform( fX, fY, 0.0f );
		m_aBreakingGlasses[ i ].SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
		m_aBreakingGlasses[ i ].Hide();

		m_oBacking.AddChildTail( m_aBreakingGlasses[ i ] );
		m_oBacking.AddChildTail( m_aGlasses[ i ] );
		m_oBacking.AddChildTail( m_aBrokenGlasses[ i ] );

		fX = fGlassHeight * m_fScale * 0.55f + fX + 5.0f;
	}

	SetLives( m_iLives, TFALSE );
	m_oBacking.SetWidth( fX + 10.0f );

	m_ppBreakFrames = TSTATICCAST( T2GUITextureSection*, TMalloc( NUM_BREAK_FRAMES * sizeof( T2GUITextureSection* ) ) );

	for ( TINT i = 0; i < NUM_BREAK_FRAMES; i++ )
		m_ppBreakFrames[ i ] = T2GUITextureSectionManager::GetTextureSection( s_apBreakFrames[ i ] );
}

// $Barnyard: FUNCTION 006499e0
void AGUI2RowLifeHud::SetVisible( TBOOL a_bVisible )
{
	if ( a_bVisible == TTRUE )
		m_oBacking.Show();
	else
		m_oBacking.Hide();
}

// $Barnyard: FUNCTION 00649a00
void AGUI2RowLifeHud::SetLives( TINT a_iLives, TBOOL a_bHideLost )
{
	m_iLives = a_iLives;

	TINT i = 0;
	for ( ; i < a_iLives; i++ )
	{
		m_aGlasses[ i ].Show();
		m_aBrokenGlasses[ i ].Hide();
	}

	for ( ; i < MAX_LIVES; i++ )
	{
		m_aGlasses[ i ].Hide();

		if ( a_bHideLost == TTRUE )
			m_aBrokenGlasses[ i ].Hide();
		else
			m_aBrokenGlasses[ i ].Show();
	}
}

// $Barnyard: FUNCTION 0064a0a0
void AGUI2RowLifeHud::AddLives( TINT a_iCount )
{
	if ( a_iCount > 0 )
	{
		for ( TINT i = a_iCount; i != 0; i-- )
		{
			if ( m_iLives < MAX_LIVES && m_iLives < m_iMaxLives )
				SetLives( m_iLives + 1, TFALSE );
		}
	}
	else if ( a_iCount < 0 )
	{
		for ( TINT i = -a_iCount; i != 0; i-- )
		{
			TINT iIndex = --m_iLives;

			if ( iIndex >= 0 )
			{
				m_uiBreakingMask |= ( 1 << iIndex );
			}
			else
			{
				m_iLives = 0;
				iIndex   = 0;
			}

			m_afBreakTime[ iIndex ] = 0.0f;
			m_aBreakingGlasses[ iIndex ].Show();
		}
	}
}

// $Barnyard: FUNCTION 00649f60
void AGUI2RowLifeHud::UpdateBreaking( TFLOAT a_fDeltaTime, TINT a_iIndex )
{
	TUINT8 uiBit = TUINT8( 1 << a_iIndex );
	if ( !( m_uiBreakingMask & uiBit ) ) return;

	if ( a_iIndex < m_iLives )
	{
		// The life came back
		m_uiBreakingMask &= ~uiBit;
		m_aBreakingGlasses[ a_iIndex ].Hide();
		m_aBrokenGlasses[ a_iIndex ].Hide();
		return;
	}

	m_afBreakTime[ a_iIndex ] += BREAK_FRAMES_SPEED * a_fDeltaTime;
	TUINT uiFrame = TUINT( m_afBreakTime[ a_iIndex ] );

	if ( uiFrame < 3 )
	{
		if ( uiFrame >= 1 )
			m_aGlasses[ a_iIndex ].Hide();
	}
	else
	{
		if ( uiFrame > 3 )
		{
			m_uiBreakingMask &= ~uiBit;
			uiFrame = 3;
			m_aBreakingGlasses[ a_iIndex ].Hide();
		}

		m_aBrokenGlasses[ a_iIndex ].Show();
	}

	m_aBreakingGlasses[ a_iIndex ].SetTextureSection( m_ppBreakFrames[ uiFrame ] );
}
