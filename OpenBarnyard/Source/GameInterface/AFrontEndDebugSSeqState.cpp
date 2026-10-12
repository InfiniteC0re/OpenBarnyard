#include "pch.h"
#include "AFrontEndDebugSSeqState.h"
#include "GUI/AGUI2.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/AGateManager.h"
#include "Helpers/AInstanceManager.h"
#include "Player/APlayerProfileManager.h"
#include "Sound/AMusicManager.h"
#include "SoundBank/music.h"

#include <GUI/T2GUIFontManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00407f60
TDEFINE_CLASS( AFrontEndDebugSSeqState );

TPSTRING8_DECLARE( DialogInputMap );

// $Barnyard: FUNCTION 00407f70
AFrontEndDebugSSeqState::AFrontEndDebugSSeqState()
{
}

// $Barnyard: FUNCTION 00407fe0
// $Barnyard: FUNCTION 004080f0
AFrontEndDebugSSeqState::~AFrontEndDebugSSeqState()
{
}

// $Barnyard: FUNCTION 00408070
TBOOL AFrontEndDebugSSeqState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( a_eInputCommand == AInputCommand_Quit || a_eInputCommand == AInputCommand_Cancel )
	{
		Remove();
		return TTRUE;
	}

	if ( m_oMenu.ProcessInputCommand( a_eInputCommand, a_pInputEvent ) )
	{
		// Scrolls the list to the focused button
		AGUI2Button* pFocused = TSTATICCAST( AGUI2Button, m_oMenu.GetFocusedMenuItem() );
		m_oMenu.SetTransform( 0.0f, TFLOAT( pFocused->GetId() ) * -23.0f - 50.0f );

		return TTRUE;
	}

	return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

// $Barnyard: FUNCTION 00408060
TBOOL AFrontEndDebugSSeqState::OnUpdate( TFLOAT a_fDeltaTime )
{
	return AGameState::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 004081e0
void AFrontEndDebugSSeqState::OnInsertion()
{
	m_bOwnsGateManager     = TFALSE;
	m_bOwnsInstanceManager = TFALSE;

	if ( !AInstanceManager::IsSingletonCreated() )
	{
		new AInstanceManager();
		m_bOwnsInstanceManager = TTRUE;
	}

	APlayerProfileManager* pProfileManager = APlayerProfileManager::GetSingleton();

	if ( pProfileManager->GetMainGender() == APlayerProfile::GENDER_NONE )
		pProfileManager->SetMainGender( APlayerProfile::GENDER_FEMALE );

	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord18" );

	for ( TINT i = 0; i < NUM_BUTTONS; i++ )
	{
		AGUI2Button& rButton = m_aButtons[ i ];
		rButton.Create( pFont, 200.0f, AScriptedSequenceActivity::ms_aSequences[ i ].wszName, 2.5f );

		TFLOAT fWidth, fHeight;
		rButton.GetDimensions( fWidth, fHeight );

		rButton.SetFocusedColour( 0xffa2ebfb );
		rButton.SetEnabledColour( 0xff556cd9 );
		rButton.SetFocusedScale( 1.0f );
		rButton.SetUseHighlight( TTRUE );
		rButton.SetHighlightImage( "grad2" );
		rButton.OnFocusLost();
		rButton.SetId( i );
		rButton.SetTransform( 0.0f, TFLOAT( i ) * fHeight * 1.25f );

		m_oMenu.AddMenuItem( rButton );
	}

	AGUI2::GetRootElement()->AddChildTail( m_oMenu );
	m_oMenu.SetFocusAt( m_aButtons[ 0 ] );
	m_oMenu.SetTransform( 0.0f, -50.0f );
	m_oMenu.SetFocus( TTRUE );
	m_oMenu.Show();
	m_oMenu.SetItemActivationCallback( OnButtonActivated );
	m_oMenu.SetCallbackUserData( this );

	SetInputMap( TPS8( DialogInputMap ) );
	AGameState::OnInsertion();
	m_HUDParams.SetFlags( AHudElement::MASK_MENU_NAV );
}

// $Barnyard: FUNCTION 00408110
void AFrontEndDebugSSeqState::OnRemoval()
{
	if ( m_bOwnsGateManager && AGateManager::IsSingletonCreated() )
		delete AGateManager::GetSingleton();

	if ( m_bOwnsInstanceManager && AInstanceManager::IsSingletonCreated() )
		delete AInstanceManager::GetSingleton();

	for ( TINT i = 0; i < NUM_BUTTONS; i++ )
		m_aButtons[ i ].RemoveSelf();

	m_oMenu.RemoveSelf();
	AGameState::OnRemoval();
}

// $Barnyard: FUNCTION 004081a0
void AFrontEndDebugSSeqState::OnActivate()
{
	AMusicManager::GetSingleton()->PlayBackgroundMusic( soundbank::MUSIC_FRONTEND );
	AGameState::OnActivate();
}

// $Barnyard: FUNCTION 004081c0
void AFrontEndDebugSSeqState::OnButtonActivated( void* a_pUserData, AGUI2MenuItem* a_pItem )
{
	AGameLoader::SetTransitionToScriptedSequence( TSTATICCAST( AGUI2Button, a_pItem )->GetId() );
	TSTATICCAST( AFrontEndDebugSSeqState, a_pUserData )->m_oMenu.Hide();
}
