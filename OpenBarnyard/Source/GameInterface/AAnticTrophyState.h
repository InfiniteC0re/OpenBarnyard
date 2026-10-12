#pragma once
#include "AGameState.h"
#include "GUI/AFade.h"
#include "GUI/AGUI2Menu.h"
#include "GUI/AGUI2Button.h"
#include "GUI/AGUI2TextBox.h"
#include "MiniGame/AMiniGameState.h"

#include <GUI/T2GUIRectangle.h>
#include <Toshi/TEvent.h>

//-----------------------------------------------------------------------------
// Final screen of a single player minigame (TClass 00781d70, vtable 0071b9e8).
// Shows the winner/loser cow picture with a title, the bucks won in story mode
// and a menu with "Try again" and/or "Return to Barnyard"/"Return to Main Menu".
//
// Added by AAnticGameOverState (story trophy, S1::bFlag5) or by
// AAnticHighScoreState once the player has pressed OK.
//
// "Try again" fades the screen out, removes this state and updates the current
// state right away so the owning AAnticGameOverState restarts the minigame (or
// lets it continue) while the screen is black. The other button fades out and
// leaves the minigame.
//-----------------------------------------------------------------------------
class AAnticTrophyState
    : public AGameState
{
public:
	TDECLARE_CLASS( AAnticTrophyState, AGameState );

	enum RESULT : TINT
	{
		RESULT_WON,        // "Congratulations!"
		RESULT_WON_BUCKS,  // "Congratulations!" and the bucks won (APlayerSimProfile::GetUnk10)
		RESULT_LOST,       // "Bad Luck!"
		RESULT_LOST_STORY, // "Game Over" (S1::bFlag6, or the final story minigame was lost)
	};

	enum BUTTON : TINT
	{
		BUTTON_TRY_AGAIN,
		BUTTON_LEAVE,
		BUTTON_NUMOF
	};

public:
	AAnticTrophyState( RESULT a_eResult );
	~AAnticTrophyState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) OVERRIDE;
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnRemoval() OVERRIDE;

	// Picks the trophy shown for these minigame results.
	// NOTE: inlined in AAnticGameOverState::OnUpdate and AAnticHighScoreState::OnUpdate
	static RESULT GetResult( const AMiniGameState::S1* a_pResults );

private:
	void  OnButtonActivated( AGUI2MenuItem* a_pItem );
	TBOOL OnFadeOver( AFade* a_pFade, TBOOL* a_pThrowFadeOver );

	static void OnMenuItemActivated( void* a_pUserData, AGUI2MenuItem* a_pItem );

private:
	AGUI2TextBox                                      m_oUnused1;                 // 0x0a4, never set up
	Toshi::T2GUIRectangle                             m_oUnused2;                 // 0x11c, never set up
	AGUI2TextBox                                      m_oTitle;                   // 0x194
	AGUI2TextBox                                      m_oBucks;                   // 0x20c, "x %d"
	AGUI2TextBox                                      m_oCoinsCollected;          // 0x284
	Toshi::T2GUIRectangle                             m_oBuckIcon;                // 0x2fc
	AGUI2Menu                                         m_oMenu;                    // 0x374
	AGUI2Button                                       m_aButtons[ BUTTON_NUMOF ]; // 0x418
	RESULT                                            m_eResult;                  // 0x820
	TBOOL                                             m_bLeave;                   // 0x824
	TBOOL                                             m_bTryAgain;                // 0x825
	TWCHAR                                            m_wszBucks[ 33 ];           // 0x826
	Toshi::T2GUIRectangle                             m_oPicture;                 // 0x868
	Toshi::TListener<AFade, TBOOL, AAnticTrophyState> m_oFadeOverListener;        // 0x8e0
	TBOOL                                             m_bFadingSound;             // 0x8f4
	TFLOAT                                            m_fVolume;                  // 0x8f8
	TFLOAT                                            m_fPulseTime;               // 0x8fc
};

TSTATICASSERT( sizeof( AAnticTrophyState ) == 0x900 );
