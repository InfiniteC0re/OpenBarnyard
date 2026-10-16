#include "pch.h"
#include "AQuestVictoryState.h"
#include "AGameStateController.h"
#include "GUI/AGUI2.h"
#include "Locale/ALocaleManager.h"
#include "Player/APlayerProfileManager.h"
#include "Tasks/ARootTask.h"
#include "SoundBank/ui.h"

#include <GUI/T2GUIFontManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

#ifdef BARNYARD_COMMUNITY_PATCH
TDEFINE_CLASS( AQuestVictoryState );
#else
// NOTE: CreateObject returns TNULL in all builds (the constructor is stripped)
TDEFINE_CLASS_NORUNTIME( AQuestVictoryState );
#endif

TPSTRING8_DECLARE( DialogInputMap );

// NOTE: constructor stripped from all builds, reconstructed from the PS2 virtuals.
// Everything else is set up in OnInsertion.
AQuestVictoryState::AQuestVictoryState()
    : m_pFont( TNULL )
    , m_pSmallFont( TNULL )
    , m_bDone( TFALSE )
    , m_bSave( TFALSE )
{
}

// $BarnyardPS2: FUNCTION 00154280
AQuestVictoryState::~AQuestVictoryState()
{
}

// $BarnyardPS2: FUNCTION 001548e0
TBOOL AQuestVictoryState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	return m_oMenu.ProcessInputCommand( a_eInputCommand, a_pInputEvent );
}

// $BarnyardPS2: FUNCTION 00154828
TBOOL AQuestVictoryState::OnUpdate( TFLOAT a_fDeltaTime )
{
	AGameState::OnUpdate( a_fDeltaTime );

	if ( m_bDone )
	{
		Remove();

		if ( m_bSave )
			AGameStateController::GetSingleton()->SaveCurrentGameState( TFALSE, TTRUE, TFALSE );
	}
	else
	{
		m_oMenu.Update( a_fDeltaTime );
	}

	return TTRUE;
}

// $BarnyardPS2: FUNCTION 00154310
void AQuestVictoryState::OnInsertion()
{
	AGameState::OnInsertion();
	SetInputMap( TPS8( DialogInputMap ) );

	m_pFont      = T2GUIFontManager::FindFont( "Rekord26" );
	m_pSmallFont = T2GUIFontManager::FindFont( "Rekord18" );

	T2GUIElement* pRoot = AGUI2::GetRootElement();

	TFLOAT fScreenWidth, fScreenHeight;
	pRoot->GetDimensions( fScreenWidth, fScreenHeight );

	// Dialog
	m_oDialog.Create( "glowframe", "glowframe4", "glowframe2", "glowframe8", "glowframe6", "glowframe7", "glowframe1", "glowframe5", "glowframe3" );
	m_oDialog.SetColour( 0x00d3dcad );
	m_oDialog.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );

	// "Mission accomplished. Save?"
	m_oText.Create( m_pFont, fScreenWidth * 0.8f );
	m_oText.SetText( g_pLocaleManager->GetString( 1075 ) );
	m_oText.SetTextAlign( T2GUIFont::TextAlign_Center );
	m_oText.SetColour( 0xffffea66 );
	m_oText.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oContent.AddChildTail( m_oText );

	// "Yes"
	m_oYes.Create( m_pSmallFont, fScreenWidth * 0.4f, g_pLocaleManager->GetString( 1218 ), 2.5f );
	m_oYes.SetFocusedColour( 0xfff76117 );
	m_oYes.SetEnabledColour( 0xffffea66 );
	m_oYes.SetFocusedScale( 1.0f );
	m_oYes.SetUseHighlight( TTRUE );
	m_oYes.SetHighlightImage( "grad2" );
	m_oYes.OnFocusLost();
	m_oMenu.AddMenuItem( m_oYes );

	// "No"
	m_oNo.Create( m_pSmallFont, fScreenWidth * 0.4f, g_pLocaleManager->GetString( 1219 ), 2.5f );
	m_oNo.SetFocusedColour( 0xfff76117 );
	m_oNo.SetEnabledColour( 0xffffea66 );
	m_oNo.SetFocusedScale( 1.0f );
	m_oNo.SetUseHighlight( TTRUE );
	m_oNo.SetHighlightImage( "grad2" );
	m_oNo.OnFocusLost();
	m_oMenu.AddMenuItem( m_oNo );

	// Menu
	m_oMenu.SetDefaultFocusElement( &m_oNo );
	m_oMenu.SetCancelItem( &m_oNo );
	m_oMenu.SetItemActivationCallback( MenuItemActivationCallback );
	m_oMenu.SetCallbackUserData( this );
	m_oMenu.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oMenu.ReflowChildrenVertically();
	m_oMenu.SetFocusSound( soundbank::UI_MENUCLICK );
	m_oMenu.SetSelectSound( soundbank::UI_MENUOK );
	m_oMenu.SetBackSound( soundbank::UI_MENUBACK );
	m_oMenu.SetNegativeSound( soundbank::UI_NEGATIVE );
	m_oContent.AddChildTail( m_oMenu );

	// The content is as wide as the text or the menu, whichever is wider, and as high as both
	TFLOAT fTextWidth, fTextHeight;
	m_oText.GetDimensions( fTextWidth, fTextHeight );

	TFLOAT fMenuWidth, fMenuHeight;
	m_oMenu.GetDimensions( fMenuWidth, fMenuHeight );

	m_oContent.SetDimensions( ( fTextWidth < fMenuWidth ) ? fMenuWidth : fTextWidth, fMenuHeight + fTextHeight );
	m_oContent.SetColour( 0 );
	m_oContent.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );

	m_oDialog.AddChildToRectangle( &m_oContent, T2GUIATTACHMENT_MIDDLECENTER );
	m_oDialog.FitToContent();
	pRoot->AddChildTail( m_oDialog );
	m_oDialog.SetFocusElement( &m_oMenu );
	m_oDialog.SetFocus( TTRUE );
	m_oDialog.Show();

	APlayerProfileManager::GetSingleton()->GetMainProfile()->SetEnableUpdates( TFALSE );
	ARootTask::GetSingleton()->SetPaused( TTRUE );
}

// $BarnyardPS2: FUNCTION 00154780
void AQuestVictoryState::OnRemoval()
{
	m_oDialog.RemoveSelf();
	m_oContent.RemoveSelf();
	m_oText.RemoveSelf();
	m_oMenu.RemoveAllItems();
	m_oMenu.RemoveSelf();

	APlayerProfileManager::GetSingleton()->GetMainProfile()->SetEnableUpdates( TTRUE );
	ARootTask::GetSingleton()->SetPaused( TFALSE );

	AGameState::OnRemoval();
}

// $BarnyardPS2: FUNCTION 001548a0
void AQuestVictoryState::OnActivate()
{
	AGameState::OnActivate();
}

// $BarnyardPS2: FUNCTION 001548c0
void AQuestVictoryState::OnDeactivate()
{
	AGameState::OnDeactivate();
}

// $BarnyardPS2: FUNCTION 00154900
void AQuestVictoryState::MenuItemActivationCallback( void* a_pUserData, AGUI2MenuItem* a_pItem )
{
	TSTATICCAST( AQuestVictoryState, a_pUserData )->OnMenuItemActivated( a_pItem );
}

// $BarnyardPS2: FUNCTION 00154920
void AQuestVictoryState::OnMenuItemActivated( AGUI2MenuItem* a_pItem )
{
	if ( a_pItem == &m_oYes )
		m_bSave = TTRUE;

	m_bDone = TTRUE;
}
