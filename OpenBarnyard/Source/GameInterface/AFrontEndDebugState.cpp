#include "pch.h"
#include "AFrontEndDebugState.h"
#include "AFrontEndCharacterSelectState.h"
#include "AFrontEndDebugPrevTerrainState.h"
#include "AGameStateController.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/ui.h"
#include "GUI/AGUI2Style.h"

#include <GUI/T2GUIFontManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

#ifdef BARNYARD_COMMUNITY_PATCH
TDEFINE_CLASS( AFrontEndDebugState );
#else
// NOTE: CreateObject returns TNULL in all builds (the constructor is stripped)
TDEFINE_CLASS_NORUNTIME( AFrontEndDebugState );
#endif

TPSTRING8_DECLARE( DialogInputMap );

// PC 0x0077263c (unreferenced), PS2 0x005411a0
static const TWCHAR* s_aItemLabels[ AFrontEndDebugState::ITEM_NUMOF ] = {
	L"Debug MiniGames",
	L"Preview Scripted Sequences",
#ifdef BARNYARD_COMMUNITY_PATCH
	L"Preview Terrains",
#endif
};

// NOTE: constructor stripped from all builds, reconstructed from the PS2 virtuals.
// The buttons and everything else are set up in OnInsertion, the selection in OnActivate.
AFrontEndDebugState::AFrontEndDebugState()
    : m_iSelectedItem( -1 )
{
}

// $BarnyardPS2: FUNCTION 0010dab0
AFrontEndDebugState::~AFrontEndDebugState()
{
}

// $BarnyardPS2: FUNCTION 0010ddb8
TBOOL AFrontEndDebugState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( m_eMenuState == MENUSTATE_MENU_VISIBLE && a_eInputCommand == AInputCommand_Cancel )
		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUBACK );

	return BaseClass::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

// $BarnyardPS2: FUNCTION 0010dd98
TBOOL AFrontEndDebugState::OnUpdate( TFLOAT a_fDeltaTime )
{
	return BaseClass::OnUpdate( a_fDeltaTime );
}

// $BarnyardPS2: FUNCTION 0010db40
void AFrontEndDebugState::OnInsertion()
{
	SetInputMap( TPS8( DialogInputMap ) );

	T2GUIFont* pFont = T2GUIFontManager::FindFont( AGUI2STYLE_FONT_PRIMARY );

	for ( TINT i = 0; i < ITEM_NUMOF; i++ )
	{
		AGUI2Button& rButton = m_aButtons[ i ];

		rButton.Create( pFont, pFont->GetTextWidth( s_aItemLabels[ i ] ), s_aItemLabels[ i ], 0.0f );
		rButton.SetFocusedColour( AGUI2STYLE_COLOR_FOCUSED );
		rButton.SetEnabledColour( AGUI2STYLE_COLOR_SECONDARY );
		rButton.SetDisabledColour( AGUI2STYLE_COLOR_DISABLED );
		rButton.SetFocusedScale( 1.0f );
		rButton.OnFocusLost();
		rButton.SetId( i );
		rButton.SetTextShadow( TTRUE, 0xa0000000, s_fShadowAlpha );
		rButton.SetTextShadowOffset( 5.0f );

		m_oMenu.AddMenuItem( rButton );
	}

	m_oMenu.SetFocusAt( m_aButtons[ 0 ] );
	m_oDialogTitle.SetText( L"DEBUG MENU" );

	BaseClass::OnInsertion();
}

// $BarnyardPS2: FUNCTION 0010dcf8
void AFrontEndDebugState::OnRemoval()
{
	for ( TINT i = 0; i < ITEM_NUMOF; i++ )
		m_aButtons[ i ].RemoveSelf();

	BaseClass::OnRemoval();
}

// $BarnyardPS2: FUNCTION 0010dd58
void AFrontEndDebugState::OnActivate()
{
	m_iSelectedItem = -1;
	BaseClass::OnActivate();
}

// $BarnyardPS2: FUNCTION 0010dd78
void AFrontEndDebugState::OnDeactivate()
{
	BaseClass::OnDeactivate();
}

// $BarnyardPS2: FUNCTION 0010de38
void AFrontEndDebugState::OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem )
{
	m_iSelectedItem = TSTATICCAST( AGUI2Button, a_pMenuItem )->GetId();
	OnButtonActivated( TSTATICCAST( AGUI2Button, a_pMenuItem ) );
}

// $BarnyardPS2: FUNCTION 0010de60
void AFrontEndDebugState::OnMenuClose()
{
	if ( m_iSelectedItem == -1 )
	{
		Remove();
		return;
	}

	AGameState* pState = TNULL;

	switch ( m_iSelectedItem )
	{
		case ITEM_DEBUG_MINIGAMES:
			// The character selection, then AFrontEndMiniGameState2 in debug mode
			pState = new AFrontEndCharacterSelectState( TFALSE, TFALSE, TFALSE );
			break;
		case ITEM_PREVIEW_SSEQ:
			// The character selection, then AFrontEndDebugSSeqState
			pState = new AFrontEndCharacterSelectState( TFALSE, TFALSE, TTRUE );
			break;
#ifdef BARNYARD_COMMUNITY_PATCH
		case ITEM_PREVIEW_TERRAIN:
			// NOTE: not in any build, see ITEM_PREVIEW_TERRAIN
			pState = new AFrontEndDebugPrevTerrainState();
			break;
#endif
	}

	// NOTE: the original pushes TNULL for any other item (there's none)
	AGameStateController::GetSingleton()->PushState( pState );
}
