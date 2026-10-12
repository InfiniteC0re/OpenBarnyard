#include "pch.h"
#include "ATerrainSectionHudElement.h"
#include "GUI/AGUI2.h"
#include "Player/APlayerManager.h"
#include "Units/AUnit.h"
#include "UnitMovement/Steering/ASteer.h"
#include "Terrain/ATerrainInterface.h"

#include <GUI/T2GUIFontManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( ATerrainSectionHudElement );

// $Barnyard: FUNCTION 0045a5e0
ATerrainSectionHudElement::ATerrainSectionHudElement()
    : AHudElement( MASK_TERRAIN_SECTION )
    , m_bShowText( TFALSE )
    , m_bFadingIn( TFALSE )
    , m_pSection( TNULL )
{
}

// $Barnyard: FUNCTION 0045a650
// $Barnyard: FUNCTION 0045a6f0
ATerrainSectionHudElement::~ATerrainSectionHudElement()
{
}

// $Barnyard: FUNCTION 0045a710
TBOOL ATerrainSectionHudElement::OnAttach()
{
	m_oText.Create( T2GUIFontManager::FindFont( "Rekord26" ), 0.0f );
	m_oText.T2GUIElement::Hide();
	m_oText.SetAlpha( 0.0f );
	m_oText.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oText.SetTranslation( 0.0f, AGUI2::GetRootElement()->GetHeight() * 0.25f );
	AGUI2::GetRootElement()->AddChildTail( m_oText );

	return TTRUE;
}

// NOTE: folded with ATimeLeftHudElement::OnDetach (0045a6b0)
void ATerrainSectionHudElement::OnDetach()
{
	m_oText.RemoveSelf();
}

// $Barnyard: FUNCTION 0045a6d0
void ATerrainSectionHudElement::Show()
{
	if ( m_bShowText == TTRUE )
		m_oText.T2GUIElement::Show();

	m_bCurrentlyVisible = TTRUE;
}

// NOTE: folded with AMenuNavHudElement::Hide (00455b60)
void ATerrainSectionHudElement::Hide()
{
	m_oText.T2GUIElement::Hide();
	m_bCurrentlyVisible = TFALSE;
}

// $Barnyard: FUNCTION 0045a780
void ATerrainSectionHudElement::SetText( const TWCHAR* a_wszText )
{
	// Don't show the name while the player is busy
	if ( APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->HasInteraction() )
		return;

	if ( a_wszText )
	{
		m_oText.SetTextAndFitWidth( a_wszText );
		m_oText.T2GUIElement::Show();
		m_bFadingIn     = TTRUE;
		m_bShowText     = TTRUE;
		m_fShowTimeLeft = SHOW_TIME;
	}
	else
	{
		m_oText.T2GUIElement::Hide();
		m_bShowText = TFALSE;
	}
}

// $Barnyard: FUNCTION 0045a810
void ATerrainSectionHudElement::Update( TFLOAT a_fDeltaTime )
{
	TVector4 vecPosition = *APlayerManager::GetSingleton()->GetPlayer( 0 )->GetSteer()->GetPosition();
	vecPosition.y -= 1.0f;

	// Show the name of the area of the terrain once the player enters a differently named one
	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();
	ATerrainSection*   pSection = pTerrain->FindSectionAtPoint( vecPosition, TTRUE );

	if ( pSection && pSection != m_pSection )
	{
		const TWCHAR* wszName = pTerrain->GetSectionLocalisedName( pSection->GetName() );
		const TWCHAR* wszOldName;

		if ( !m_pSection ||
		     !wszName ||
		     !( wszOldName = pTerrain->GetSectionLocalisedName( m_pSection->GetName() ) ) ||
		     T2String16::Compare( wszName, wszOldName ) != 0 )
		{
			SetText( wszName );
		}
	}

	m_pSection = pSection;

	if ( !m_oText.IsVisible() )
		return;

	TFLOAT fAlpha = m_oText.GetAlpha();

	if ( fAlpha < 1.0f && m_bFadingIn == TTRUE )
	{
		// Fade in
		fAlpha = a_fDeltaTime + a_fDeltaTime + fAlpha;
		TMath::Clip( fAlpha, 0.0f, 1.0f );

		if ( fAlpha >= 1.0f )
		{
			fAlpha      = 1.0f;
			m_bFadingIn = TFALSE;
		}
	}
	else
	{
		m_fShowTimeLeft -= a_fDeltaTime;

		if ( m_fShowTimeLeft > 0.0f )
			return;

		// Fade out
		m_fShowTimeLeft = 0.0f;
		fAlpha          = fAlpha - ( a_fDeltaTime + a_fDeltaTime );

		if ( fAlpha < 0.0f )
		{
			m_oText.T2GUIElement::Hide();
			return;
		}
	}

	m_oText.SetAlpha( fAlpha );
}
