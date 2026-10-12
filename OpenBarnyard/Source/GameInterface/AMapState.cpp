#include "pch.h"
#include "AMapState.h"
#include "AGameStateController.h"
#include "ALoadScreen.h"
#include "GUI/AGUI2.h"
#include "GUI/AGUISystem.h"
#include "GUI/AGUI2ModelRendererScreen.h"
#include "HUD/AHudManager.h"
#include "HUD/ASimMiniMap.h"
#include "Locale/ALocale.h"
#include "Locale/ALocaleManager.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/ui.h"
#include "Tasks/ARootTask.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/AUnit.h"
#include "UnitMovement/Steering/ASteer.h"

#include <File/TTRB.h>
#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Math/TMathInline.h>
#include <Math/TVector2.h>
#include <Toshi/TStringManager.h>

// NOTE: the physics headers pull in Opcode's PI macro, keep them last
#include "Helpers/AGateManager.h"
#include "Helpers/ABikeManager.h"
#include "Units/AInventoryGroupComponent.h"
#include "Helpers/AReplayManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00438700, CreateObject 00438e10, CreateObjectInPlace 00438e70 (the constructor is inlined in both)
TDEFINE_CLASS( AMapState );

TPSTRING8_DECLARE( ASimState );
TPSTRING8_DECLARE( DialogInputMap );
TPSTRING8_DECLARE( bkg_End_wood );
TPSTRING8_DECLARE( bikekey );

// Texture sections of the icons
static const TCHAR* s_aszIconSections[ AMapState::ICON_NUMOF ] = {
	"Player",
	"Challenge",
	"Item",
	"Mission",
	"Bike",
	"Gate",
	"Cabbage",
	"Gopher",
	"Save",
	"Map_Honey",
	"Sleep",
	"Apples",
	"Lemon",
	"Cherry",
	"Walnut",
	"Player_Bike",
	"Ping_Ring"
}; // 0x007734d8

// Scale of the map for each zoom level
static const TFLOAT s_afZoomScales[ 2 ] = { 1.2f, 2.7f }; // 0x0071a920

// Strings of the place names of mapdata.trb, indexed by type - AMapState::LABEL_FIRST (-1 is the name of the player)
// NOTE: the original indexes the table with the type from 0x0071a794, type 100 would read 2.7f of s_afZoomScales
static const TINT s_aiLabelStrings[] = {
	1757, 1758, 1759, 1760, 1761, 1762, 1763, 1764, 1765, 1766, 1767, 1768, 1769, 1770, 1771,
	265, 339, 492, 493, 494, 495, 496, 497, 498,
	130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157,
	-1
}; // 0x0071a928

// Converts (x, -z) world coordinates to UVs of Map_EnvMain (same as ASimMiniMap)
static constexpr TFLOAT WORLD_TO_UV_X = 0.0011972656f; // 0x007736dc
static constexpr TFLOAT WORLD_TO_UV_Y = 0.0023945312f; // 0x007736e0
static constexpr TFLOAT UV_OFFSET_X   = 0.4977539f;    // 0x007736e4
static constexpr TFLOAT UV_OFFSET_Y   = 0.5475781f;    // 0x007736e8

// Data/MinigameData/Map/mapdata.trb, symbol "Main"
struct MapData
{
	struct Entry
	{
		TINT iX;    // 0..512
		TINT iY;    // 0..256
		TINT iType; // ICON or a place name (>= LABEL_FIRST)
		TINT iMode; // icons: shown when zoomed in, place names: 0 small, 1 big, 2 shown when zoomed in
	};

	TINT  iNumEntries;
	Entry aEntries[ 1 ];
};

// $Barnyard: FUNCTION 00438710
AMapState::AMapState()
    : m_iZoom( -1 )
    , m_fScrollX( -0.08f )
    , m_fScrollY( 0.0f )
    , m_iNumIcons( 0 )
    , m_iNumLabels( 0 )
#ifdef BARNYARD_COMMUNITY_PATCH
    // NOTE: the original doesn't initialise it, OnRemoval checks it
    , m_pMapSection( TNULL )
#endif
    , m_fPingScale( 0.0f )
    , m_eState( STATE_START )
    , m_fLoadTime( 0.0f )
    , m_uiHUDMask( 0 )
{
}

// $Barnyard: FUNCTION 00438870
// $Barnyard: FUNCTION 00438eb0
AMapState::~AMapState()
{
}

// $Barnyard: FUNCTION 00439250
TBOOL AMapState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( m_eState != STATE_SHOWING )
		return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	switch ( a_eInputCommand )
	{
		case AInputCommand_Select:
		case AInputCommand_Cancel:
			ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUOK );
			m_eState = STATE_CLOSING;
			return TTRUE;
		case AInputCommand_ZoomIn:
			SetZoom( m_iZoom - 1, m_fScrollX, m_fScrollY );
			return TTRUE;
		case AInputCommand_ZoomOut:
			SetZoom( m_iZoom + 1, m_fScrollX, m_fScrollY );
			return TTRUE;
	}

	// The zoomed in map can be scrolled
	if ( m_iZoom != 1 )
		return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	switch ( a_eInputCommand )
	{
		case AInputCommand_Left:
			a_pInputEvent->StartRepeat( 0.0f, 0.01f );
			SetZoom( m_iZoom, m_fScrollX + 0.015f, m_fScrollY );
			return TTRUE;
		case AInputCommand_Right:
			a_pInputEvent->StartRepeat( 0.0f, 0.01f );
			SetZoom( m_iZoom, m_fScrollX - 0.015f, m_fScrollY );
			return TTRUE;
		case AInputCommand_Up:
			a_pInputEvent->StartRepeat( 0.0f, 0.01f );
			SetZoom( m_iZoom, m_fScrollX, m_fScrollY + 0.015f );
			return TTRUE;
		case AInputCommand_Down:
			a_pInputEvent->StartRepeat( 0.0f, 0.01f );
			SetZoom( m_iZoom, m_fScrollX, m_fScrollY - 0.015f );
			return TTRUE;
	}

	return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

// $Barnyard: FUNCTION 0043a9b0
TBOOL AMapState::OnUpdate( TFLOAT a_fDeltaTime )
{
	switch ( m_eState )
	{
		case STATE_START:
			AGUI2ModelRendererScreen::GetSingleton()->SetEnabled( TFALSE );
			m_HUDParams.SetFlags( 0 );

			if ( ATerrainInterface::GetSingleton() == TNULL || ATerrainInterface::GetSingleton()->IsLoaded() )
			{
				CreateGUI();
				m_eState = STATE_CREATED;
			}
			else
			{
				g_oLoadScreen.Reset();
				g_oLoadScreen.StartLoading( 0, TTRUE );
				m_eState = STATE_LOADING;
			}

			break;
		case STATE_LOADING:
			m_fLoadTime += a_fDeltaTime;
			g_oLoadScreen.Update( 1.0f, TTRUE );

			if ( ATerrainInterface::GetSingleton()->IsLoaded() && m_fLoadTime > 1.0f )
			{
				g_oLoadScreen.Reset();
				CreateGUI();
				m_eState = STATE_CREATED;
			}

			break;
		case STATE_CREATED:
			m_eState = STATE_SHOWING;
			// fallthrough
		case STATE_SHOWING:
		{
			// Pulse the ring around the player
			m_fPingScale += a_fDeltaTime + a_fDeltaTime;
			if ( m_fPingScale > 3.0f ) m_fPingScale = 0.0f;

			T2GUITransform& rTransform = m_oPing.GetTransform();
			rTransform.SetIdentity();
			rTransform.Scale( m_fPingScale, m_fPingScale );
			m_oPing.SetAlpha( ( 3.0f - m_fPingScale ) * ( 1.0f / 3.0f ) );
			break;
		}
		case STATE_CLOSING:
			m_oDialog.Hide();
			m_oClose.Hide();
			m_oZoomOut.Hide();
			m_oZoomIn.Hide();

			AGUISystem::GetSingleton()->SetPicture( TPString8() );
			AGUISystem::GetSingleton()->ResetPicture();
			AGUI2ModelRendererScreen::GetSingleton()->SetEnabled( TTRUE );
			m_HUDParams.SetFlags( m_uiHUDMask );
			m_eState = STATE_REMOVING;
			// fallthrough
		case STATE_REMOVING:
			Remove();
			break;
	}

	return AGameState::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00438ed0
void AMapState::OnInsertion()
{
	ARootTask::GetSingleton()->SetPaused( TTRUE );
	AGameStateController::ms_bPausedByMenu = TTRUE;

	// Hide the HUD of the sim
	m_uiHUDMask = AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ASimState ) )->GetHUDMask();
	m_HUDParams.SetFlags( m_uiHUDMask );

	SetInputMap( TPS8( DialogInputMap ) );
	AGameState::OnInsertion();
}

// $Barnyard: FUNCTION 00439100
void AMapState::OnRemoval()
{
	m_oPing.RemoveSelf();

	for ( TINT i = 0; i < m_iNumLabels; i++ )
		m_aLabels[ i ].RemoveSelf();

	m_iNumLabels = 0;

	for ( TINT i = 0; i < m_iNumIcons; i++ )
		m_aIcons[ i ].RemoveSelf();

	m_iNumIcons = 0;

	for ( TINT i = 0; i < LAYER_NUMOF; i++ )
		m_aLayers[ i ].RemoveSelf();

	// NOTE: the original moves the arrows back to the dialog
	for ( TINT i = 0; i < TARRAYSIZE( m_aArrows ); i++ )
		m_oDialog.AddChildTail( m_aArrows[ i ] );

	m_oZoomIn.RemoveSelf();
	m_oZoomOut.RemoveSelf();
	m_oClose.RemoveSelf();
	m_oMap.RemoveSelf();
	m_oDialog.RemoveSelf();

	if ( m_pMapSection )
		m_pMapSection->GetMaterial()->SetBlendState( m_uiMapBlendState );

	AGameStateController::ms_bPausedByMenu = TFALSE;
	ARootTask::GetSingleton()->SetPaused( TFALSE );
	AGameState::OnRemoval();
}

// $Barnyard: FUNCTION 00438980
void AMapState::AddIcon( ICON a_eIcon, TFLOAT a_fX, TFLOAT a_fY, LAYER a_eLayer, TFLOAT a_fRotation, TBOOL a_bCentred, TFLOAT a_fScale )
{
	if ( m_iNumIcons + 1 >= MAX_ICONS )
		return;

	T2GUITextureSection* pSection = m_apIconSections[ a_eIcon ];
	TFLOAT               fWidth   = pSection->GetWidth();
	TFLOAT               fHeight  = pSection->GetHeight();

	// Keep the size of the icons the same on both zoom levels
	TFLOAT fScale = ( a_eLayer < LAYER_ZOOMEDIN ) ? a_fScale * ( 1.0f / 1.2f ) : a_fScale * ( 1.0f / 2.7f );

	T2GUIRectangle& rIcon = m_aIcons[ m_iNumIcons ];
	rIcon.SetDimensions( fWidth, fHeight );
	rIcon.SetTextureSection( m_apIconSections[ a_eIcon ] );
	rIcon.SetAnchor( T2GUIATTACHMENT_TOPLEFT );

	if ( a_bCentred )
	{
		rIcon.SetPivot( T2GUIATTACHMENT_MIDDLECENTER );
		rIcon.SetTransform( a_fX * m_fMapWidth, a_fY * m_fMapHeight, a_fRotation );
	}
	else
	{
		rIcon.SetPivot( T2GUIATTACHMENT_TOPLEFT );
		rIcon.SetTransform( a_fX * m_fMapWidth - fScale * fWidth * 0.5f, a_fY * m_fMapHeight - fScale * fHeight * 0.5f, a_fRotation );
	}

	rIcon.GetTransform().Scale( fScale, fScale );
	m_aLayers[ a_eLayer ].AddChildTail( rIcon );
	m_iNumIcons++;
}

// $Barnyard: FUNCTION 00438f30
void AMapState::AddLabel( TINT a_iString, TFLOAT a_fX, TFLOAT a_fY, TBOOL a_bBig, LAYER a_eLayer )
{
	// The small names of the zoomed out map are only shown in English
	T2Locale::Lang eLanguage = ALocaleManager::Interface()->GetLanguage();

	if ( eLanguage != ALocaleManager::Lang_English && eLanguage != ALocaleManager::Lang_EnglishUK && a_eLayer < LAYER_ZOOMEDIN && !a_bBig )
		return;

	T2GUIFont*    pFont = T2GUIFontManager::FindFont( "Rekord18" );
	const TWCHAR* wszText;

	if ( a_iString == -1 )
		wszText = APlayerProfileManager::GetSingleton()->GetMainProfile()->GetName();
	else
		wszText = ALocaleManager::Interface()->GetString( a_iString );

	TFLOAT fScale = ( a_eLayer == LAYER_ZOOMEDOUT || a_eLayer == LAYER_ZOOMEDOUT_TOP ) ? 0.9f : 0.4f;

	AGUI2TextBox& rLabel = m_aLabels[ m_iNumLabels ];
	rLabel.Create( pFont, 2.0f );
	rLabel.SetScale( fScale );
	rLabel.SetText( wszText );
	rLabel.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	rLabel.SetTransform( a_fX * m_fMapWidth, a_fY * m_fMapHeight );

	if ( a_bBig )
	{
		rLabel.SetWidth( 200.0f );
		rLabel.SetTextAlign( T2GUIFont::TextAlign_Left );
		rLabel.SetColour( 0xFFFFC800 );
	}
	else
	{
		rLabel.SetTextAlign( T2GUIFont::TextAlign_Center );
	}

	m_aLayers[ a_eLayer ].AddChildTail( rLabel );
	m_iNumLabels++;
}

// $Barnyard: FUNCTION 00438b20
void AMapState::SetZoom( TINT a_iZoom, TFLOAT a_fScrollX, TFLOAT a_fScrollY )
{
	TMath::Clip( a_iZoom, 0, 1 );

	TFLOAT fMaxX = ( s_afZoomScales[ a_iZoom ] - 1.0f ) * 0.5f;
	TFLOAT fMinX = -fMaxX;
	TMath::Clip( a_fScrollX, fMinX, fMaxX );

	TFLOAT fMaxY = fMaxX - m_fScrollMarginY;
	TFLOAT fMinY = -fMaxY;
	TMath::Clip( a_fScrollY, fMinY, fMaxY );

	// Move the ring to the player icon of the zoom level
	if ( m_iZoom != a_iZoom )
		m_aIcons[ a_iZoom + m_iPlayerIcon ].AddChildTail( m_oPing );

	if ( m_iZoom == a_iZoom && a_fScrollX == m_fScrollX && a_fScrollY == m_fScrollY )
		return;

	m_iZoom = a_iZoom;

	T2GUITransform& rTransform = m_oMap.GetTransform();
	rTransform.SetIdentity();
	rTransform.Scale( s_afZoomScales[ m_iZoom ], s_afZoomScales[ m_iZoom ] );

	if ( a_iZoom != 0 )
	{
		m_fScrollX = a_fScrollX;
		m_fScrollY = a_fScrollY;
		rTransform.SetTranslation( a_fScrollX * m_fMapWidth, a_fScrollY * m_fMapHeight );

		m_aLayers[ LAYER_ZOOMEDOUT ].Hide();
		m_aLayers[ LAYER_ZOOMEDOUT_TOP ].Hide();
		m_aLayers[ LAYER_ZOOMEDIN ].Show();
		m_aLayers[ LAYER_ZOOMEDIN_TOP ].Show();

		// Show the directions the map can be scrolled in
		m_aArrows[ 0 ].SetVisible( m_fScrollX < fMaxX );
		m_aArrows[ 1 ].SetVisible( m_fScrollY < fMaxY );
		m_aArrows[ 2 ].SetVisible( m_fScrollX > fMinX );
		m_aArrows[ 3 ].SetVisible( m_fScrollY > fMinY );

		m_oZoomOut.Hide();
		m_oZoomIn.Show();
	}
	else
	{
		rTransform.SetTranslation( m_fMapWidth * -0.08f, m_fMapHeight * 0.0f );

		m_aLayers[ LAYER_ZOOMEDOUT ].Show();
		m_aLayers[ LAYER_ZOOMEDOUT_TOP ].Show();
		m_aLayers[ LAYER_ZOOMEDIN ].Hide();
		m_aLayers[ LAYER_ZOOMEDIN_TOP ].Hide();

		for ( TINT i = 0; i < TARRAYSIZE( m_aArrows ); i++ )
			m_aArrows[ i ].Hide();

		m_oZoomOut.Show();
		m_oZoomIn.Hide();
	}
}

// Returns the angle of the direction around the Y axis, [0; 2PI]
static TFLOAT GetDirectionAngle( TFLOAT a_fDirX, TFLOAT a_fDirZ )
{
	TFLOAT fDot = TVector2::VEC_POSY.y * a_fDirZ + TVector2::VEC_POSY.x * a_fDirX;
	TMath::Clip( fDot, -1.0f, 1.0f );

	TFLOAT fAngle = TMath::ACos( fDot );

	if ( TVector2::VEC_POSY.y * a_fDirX - TVector2::VEC_POSY.x * a_fDirZ < 0.0f )
		fAngle = TMath::TWO_PI - fAngle;

	return fAngle;
}

// $Barnyard: FUNCTION 00439420
void AMapState::CreateGUI()
{
	AGUISystem::GetSingleton()->SetPicture( TPS8( bkg_End_wood ) );

	TFLOAT fScreenWidth, fScreenHeight;
	AGUI2::GetSingleton()->GetDimensions( fScreenWidth, fScreenHeight );

	m_pMapSection         = T2GUITextureSectionManager::GetTextureSection( "Map_EnvMain" );
	TFLOAT fSectionWidth  = TFLOAT( m_pMapSection->GetPoint2X() - m_pMapSection->GetPoint1X() );
	TFLOAT fSectionHeight = TFLOAT( m_pMapSection->GetPoint2Y() - m_pMapSection->GetPoint1Y() );

	// Draw the map without blending, the original state is restored by OnRemoval
	m_uiMapBlendState = m_pMapSection->GetMaterial()->GetBlendState();
	m_pMapSection->GetMaterial()->SetBlendState( T2GUIMaterial::BLENDSTATE_OPAQUE );

	m_oDialog.Create();
	m_oDialog.SetDialogSize( fScreenWidth, fScreenHeight * 0.9f );
	m_fScrollMarginY = ( fScreenHeight / fScreenWidth * 0.9f - 0.5f ) * 0.5f;

	// The map keeps the aspect of the texture (whose pixels aren't square)
	TFLOAT fMapHeight = TMath::Abs( fSectionHeight ) * 0.5f / TMath::Abs( fSectionWidth ) * fScreenWidth;
	m_oMap.SetDimensions( fScreenWidth, fMapHeight );
	m_oMap.SetTextureSection( m_pMapSection );

	m_oDialog.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oDialog.SetTransform( 0.0f, fScreenHeight * 0.05f );
	m_oDialog.SetUseScissor( TTRUE );
	m_fMapWidth  = fScreenWidth;
	m_fMapHeight = fMapHeight;

	// The arrows on the edges of the dialog
	T2GUITextureSection* pArrow       = T2GUITextureSectionManager::GetTextureSection( "left_arrow" );
	TFLOAT               fArrowWidth  = pArrow->GetWidth();
	TFLOAT               fArrowHeight = pArrow->GetHeight();

	for ( TINT i = 0; i < TARRAYSIZE( m_aArrows ); i++ )
	{
		m_aArrows[ i ].SetDimensions( fArrowWidth, fArrowHeight );
		m_aArrows[ i ].SetTextureSection( pArrow );
	}

	m_aArrows[ 0 ].SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLERIGHT );
	m_aArrows[ 1 ].SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_BOTTOMLEFT );
	m_aArrows[ 2 ].SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLELEFT );
	m_aArrows[ 3 ].SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_TOPLEFT );

	m_aArrows[ 0 ].SetTransform( 34.0f, 0.0f, 0.0f );
	m_aArrows[ 1 ].SetTransform( fArrowHeight * -0.5f, 5.0f, TMath::HALF_PI );
	m_aArrows[ 2 ].SetTransform( -21.0f, 0.0f, TMath::PI );
	m_aArrows[ 3 ].SetTransform( fArrowHeight * -0.5f, -23.0f, -TMath::HALF_PI );

	for ( TINT i = 0; i < TARRAYSIZE( m_aArrows ); i++ )
		m_oDialog.AddChildTail( m_aArrows[ i ] );

	// What to press at the bottom
	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord26" );
	TWCHAR     wszBuffer[ 64 ];

	TStringManager::String16Format( wszBuffer, TARRAYSIZE( wszBuffer ), L"%s  %s", L"%c_DialogInputMap[Cancel]", ALocaleManager::Interface()->GetString( 1212 ) );
	ALocale::FormatInputTags( m_wszClose, TARRAYSIZE( m_wszClose ), wszBuffer );

	TStringManager::String16Format( wszBuffer, TARRAYSIZE( wszBuffer ), L"%s  %s", L"%c_DialogInputMap[ZoomOut]", ALocaleManager::Interface()->GetString( 1235 ) );
	ALocale::FormatInputTags( m_wszZoomOut, TARRAYSIZE( m_wszZoomOut ), wszBuffer );

	TStringManager::String16Format( wszBuffer, TARRAYSIZE( wszBuffer ), L"%s  %s", L"%c_DialogInputMap[ZoomIn]", ALocaleManager::Interface()->GetString( 1236 ) );
	ALocale::FormatInputTags( m_wszZoomIn, TARRAYSIZE( m_wszZoomIn ), wszBuffer );

	TFLOAT fCloseWidth   = pFont->GetTextWidth( m_wszClose, 1.0f );
	TFLOAT fZoomOutWidth = pFont->GetTextWidth( m_wszZoomOut, 1.0f );
	TFLOAT fZoomInWidth  = pFont->GetTextWidth( m_wszZoomIn, 1.0f );
	TFLOAT fTextY        = -( fScreenHeight * 0.03f );

	// Make the texts fit 40% of the screen
	TFLOAT fScale = TMath::Min( fScreenWidth * 0.4f / TMath::Max( fCloseWidth, TMath::Max( fZoomOutWidth, fZoomInWidth ) ), 0.75f );

	m_oClose.Create( pFont, fCloseWidth );
	m_oClose.SetScale( fScale );
	m_oClose.SetTextAlign( T2GUIFont::TextAlign_Left );
	m_oClose.SetText( m_wszClose );
	m_oClose.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oClose.SetTransform( AGUI2::GetRootElement()->GetWidth() * -0.25f, fTextY );

	m_oZoomOut.Create( pFont, fZoomOutWidth );
	m_oZoomOut.SetScale( fScale );
	m_oZoomOut.SetTextAlign( T2GUIFont::TextAlign_Left );
	m_oZoomOut.SetText( m_wszZoomOut );
	m_oZoomOut.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oZoomOut.SetTransform( AGUI2::GetRootElement()->GetWidth() * 0.25f + ( fZoomOutWidth - fZoomInWidth ) * 0.5f, fTextY );

	m_oZoomIn.Create( pFont, fZoomInWidth );
	m_oZoomIn.SetScale( fScale );
	m_oZoomIn.SetTextAlign( T2GUIFont::TextAlign_Left );
	m_oZoomIn.SetText( m_wszZoomIn );
	m_oZoomIn.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oZoomIn.SetTransform( AGUI2::GetRootElement()->GetWidth() * 0.25f, fTextY );
	m_oZoomIn.Hide();

	// The layers with the icons and the names
	for ( TINT i = 0; i < LAYER_NUMOF; i++ )
	{
		m_aLayers[ i ].SetDimensions( m_fMapWidth, m_fMapHeight );
		m_aLayers[ i ].Hide();
		m_oMap.AddChildTail( m_aLayers[ i ] );
	}

	m_aLayers[ LAYER_ZOOMEDOUT ].Show();
	m_aLayers[ LAYER_ZOOMEDOUT_TOP ].Show();

	for ( TINT i = 0; i < ICON_NUMOF; i++ )
		m_apIconSections[ i ] = T2GUITextureSectionManager::GetTextureSection( s_aszIconSections[ i ] );

	// The stars of the replays on the farm
	// NOTE: retail calls IsInBlockedTier twice
	AReplayManager* pReplayManager = AReplayManager::GetSingleton();

	if ( !pReplayManager->IsInBlockedTier() && !pReplayManager->IsInBlockedTier() )
	{
		for ( AReplayableGame* pReplay = pReplayManager->GetFirstReplay(); pReplay != TNULL; pReplay = pReplay->m_pNext )
		{
			if ( pReplay->m_eLocation == AReplayableGame::LOCATION_FARM && pReplay->m_eStarState != AReplayableGame::STARSTATE_HIDDEN && pReplay->IsTierReached() )
			{
				TVector4 vecPosition;
				pReplay->GetStarPosition( vecPosition );

				AddIcon( ICON_CHALLENGE, vecPosition.x * WORLD_TO_UV_X + UV_OFFSET_X, -vecPosition.z * WORLD_TO_UV_Y + UV_OFFSET_Y, LAYER_ZOOMEDOUT, 0.0f, TFALSE, 1.0f );
				AddIcon( ICON_CHALLENGE, vecPosition.x * WORLD_TO_UV_X + UV_OFFSET_X, -vecPosition.z * WORLD_TO_UV_Y + UV_OFFSET_Y, LAYER_ZOOMEDIN, 0.0f, TFALSE, 1.0f );
			}
		}
	}

	// The closed padlocks
	// NOTE: retail doesn't check if AGateManager exists
	AGateManager* pGateManager = AGateManager::GetSingleton();

	for ( TINT i = 0; i < pGateManager->GetNumGates(); i++ )
	{
		AGateManager::Gate* pGate = pGateManager->GetGate( i );

		if ( !pGate->bOpen && pGate->eModel == AGateManager::MODEL_PADLOCK )
		{
			const TVector3& rcPosition = pGate->GetPosition();

			AddIcon( ICON_GATE, rcPosition.x * WORLD_TO_UV_X + UV_OFFSET_X, -rcPosition.z * WORLD_TO_UV_Y + UV_OFFSET_Y, LAYER_ZOOMEDOUT, 0.0f, TFALSE, 1.0f );
			AddIcon( ICON_GATE, rcPosition.x * WORLD_TO_UV_X + UV_OFFSET_X, -rcPosition.z * WORLD_TO_UV_Y + UV_OFFSET_Y, LAYER_ZOOMEDIN, 0.0f, TFALSE, 1.0f );
		}
	}

	// The icons and the place names of mapdata.trb
	TTRB oMapData;
	oMapData.Load( "Data\\MinigameData\\Map\\mapdata.trb" );
	MapData* pMapData = oMapData.CastSymbol<MapData>( "Main" );

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: retail doesn't check if the file was loaded (it can fail when the map is opened during a minigame)
	TINT iNumEntries = 0;

	if ( pMapData )
		iNumEntries = pMapData->iNumEntries;
	else
		TWARN( "AMapState: couldn't load Data\\MinigameData\\Map\\mapdata.trb\n" );
#else
	TINT iNumEntries = pMapData->iNumEntries;
#endif

	for ( TINT i = 0; i < iNumEntries; i++ )
	{
		const MapData::Entry& rcEntry = pMapData->aEntries[ i ];

		TFLOAT fY = rcEntry.iY * ( 1.0f / 256.0f );
		TFLOAT fX = rcEntry.iX * ( 1.0f / 512.0f );

		if ( rcEntry.iType < 100 )
		{
			if ( rcEntry.iMode != 0 )
				AddIcon( rcEntry.iType, fX, fY, LAYER_ZOOMEDIN, 0.0f, TFALSE, 1.0f );
			else
				AddIcon( rcEntry.iType, fX, fY, LAYER_ZOOMEDOUT, 0.0f, TFALSE, 1.0f );
		}
		else if ( rcEntry.iMode == 0 )
		{
			AddLabel( s_aiLabelStrings[ rcEntry.iType - LABEL_FIRST ], fX, fY, TFALSE, LAYER_ZOOMEDOUT );
		}
		else if ( rcEntry.iMode == 1 )
		{
			AddLabel( s_aiLabelStrings[ rcEntry.iType - LABEL_FIRST ], fX, fY, TTRUE, LAYER_ZOOMEDOUT );
		}
		else
		{
			AddLabel( s_aiLabelStrings[ rcEntry.iType - LABEL_FIRST ], fX, fY, TFALSE, LAYER_ZOOMEDIN );
		}
	}

	oMapData.Close();

	// The visible icons of the minimap
	ASimMiniMap* pMiniMap = TSTATICCAST( ASimMiniMap, AHudManager::GetSingleton()->FindElement( &TGetClass( ASimMiniMap ) ) );

	for ( auto it = pMiniMap->m_llUsedIcons.Begin(); it != pMiniMap->m_llUsedIcons.End(); it++ )
	{
		ASimMiniMap::Icon* pIcon = it;

		if ( !pIcon->m_oRect.IsVisible() )
			continue;

		ICON eIcon;

		switch ( pIcon->m_eType )
		{
			case ASimMiniMap::ICON_CHALLENGE:
				eIcon = ICON_CHALLENGE;
				break;
			case ASimMiniMap::ICON_MISSION:
				eIcon = ICON_MISSION;
				break;
			case ASimMiniMap::ICON_ITEM:
				eIcon = ICON_ITEM;
				break;
			default:
				continue;
		}

		AUnit*          pUnit = pIcon->m_pUnit;
		const TVector4* pPosition;

		if ( !pUnit || pUnit->IsDying() )
			pPosition = &pIcon->m_vPosition;
		else
			pPosition = pUnit->GetSteer()->GetPosition();

		TFLOAT fX = WORLD_TO_UV_X * pPosition->x + UV_OFFSET_X;
		TFLOAT fY = UV_OFFSET_Y + WORLD_TO_UV_Y * -pPosition->z;
		AddIcon( eIcon, fX, fY, LAYER_ZOOMEDOUT, 0.0f, TFALSE, 1.0f );
		AddIcon( eIcon, fX, fY, LAYER_ZOOMEDIN, 0.0f, TFALSE, 1.0f );
	}

	TVector4 vecPlayerPos = *APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->GetSteer()->GetPosition();

	// The bike closest to the player if he has the key and isn't riding it
	if ( !ABikeManager::GetSingleton()->IsPlayerOnBike() )
	{
		AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->FindComponentExact( &TGetClass( AInventoryGroupComponent ) ) );

		if ( pInventory && pInventory->FindItemByType( TPS8( bikekey ) ) )
		{
			// NOTE: retail doesn't initialise them, FindClosestBike only sets them if there are bikes
			AUnit* pBike     = TNULL;
			TINT   iBikeType = 0;
			ABikeManager::GetSingleton()->FindClosestBike( vecPlayerPos, pBike, iBikeType );

			if ( pBike && !ABikeManager::GetSingleton()->IsBikeParked( pBike ) )
			{
				const TVector4* pBikePosition = pBike->GetSteer()->GetPosition();

				AddIcon( ICON_PLAYER_BIKE, WORLD_TO_UV_X * pBikePosition->x + UV_OFFSET_X, UV_OFFSET_Y + WORLD_TO_UV_Y * -pBikePosition->z, LAYER_ZOOMEDOUT_TOP, 0.0f, TFALSE, 1.0f );
				AddIcon( ICON_PLAYER_BIKE, WORLD_TO_UV_X * pBikePosition->x + UV_OFFSET_X, UV_OFFSET_Y + WORLD_TO_UV_Y * -pBikePosition->z, LAYER_ZOOMEDIN_TOP, 0.0f, TFALSE, 1.0f );
			}
		}
	}

	// The player, rotated to where he's facing
	TVector4 vecPlayerDir = *APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->GetSteer()->GetDirection();
	TFLOAT   fPlayerAngle = GetDirectionAngle( vecPlayerDir.x, vecPlayerDir.z );
	TFLOAT   fPlayerX     = UV_OFFSET_X + WORLD_TO_UV_X * vecPlayerPos.x;
	TFLOAT   fPlayerY     = UV_OFFSET_Y + WORLD_TO_UV_Y * -vecPlayerPos.z;
	AddIcon( ICON_PLAYER, fPlayerX, fPlayerY, LAYER_ZOOMEDOUT_TOP, fPlayerAngle, TTRUE, 1.0f );
	AddIcon( ICON_PLAYER, fPlayerX, fPlayerY, LAYER_ZOOMEDIN_TOP, fPlayerAngle, TTRUE, 1.0f );

	// The ring pulsing around the player
	m_iPlayerIcon = m_iNumIcons - 2;
	m_oPing.SetTextureSection( *m_apIconSections[ ICON_PING_RING ] );

	T2GUITransform oZero;
	oZero.SetZero();
	m_oPing.GetTransform().PreMultiply( oZero );
	m_aIcons[ m_iPlayerIcon ].AddChildTail( m_oPing );

	// Centre the zoomed in map on the player
	m_fScrollX = ( 0.5f - fPlayerX ) * 2.7f;
	m_fScrollY = ( 0.5f - fPlayerY ) * 2.7f;
	SetZoom( 0, m_fScrollX, m_fScrollY );

	T2GUIElement* pRoot = AGUI2::GetRootElement();
	pRoot->AddChildTail( m_oDialog );
	m_oDialog.AddChildToRectangle( &m_oMap, T2GUIATTACHMENT_MIDDLECENTER );
	pRoot->AddChildTail( m_oClose );
	pRoot->AddChildTail( m_oZoomOut );
	pRoot->AddChildTail( m_oZoomIn );
}
