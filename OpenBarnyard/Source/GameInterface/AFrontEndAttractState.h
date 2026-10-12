#pragma once
#include "AGameState.h"
#include "Helpers/AWindmillHelper.h"
#include "GUI/AGUI2TextBox.h"
#include "GUI/AFade.h"

#include <GUI/T2GUIRectangle.h>

//-----------------------------------------------------------------------------
// Purpose: the "press start" title screen of the front end. It shows the barn
// sign of the windmill helper and a blinking plank with locale string 290.
// Start opens the main menu (AFrontEndMainMenuState2, which takes over the
// windmill helper). After ms_fAttractIdleTime seconds without input the
// "Attract" movie plays and a new AFrontEndAttractState follows it.
// NOTE: nothing in the retail game creates it, it can only be created by name
// (the TClass has a create function), e.g. with -devattract.
// TClass 0077df4c, vtable 007164a0, size 0x2a4.
//-----------------------------------------------------------------------------
class AFrontEndAttractState
    : public AGameState
{
public:
	TDECLARE_CLASS( AFrontEndAttractState, AGameState );

	using FadeOverListener = Toshi::TListener<AFade, TBOOL, AFrontEndAttractState>;

public:
	AFrontEndAttractState( AWindmillHelper* a_pWindmillHelper = TNULL );
	~AFrontEndAttractState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) OVERRIDE;                                                        // +0x0c
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE; // +0x10
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                                // +0x4c
	virtual void  OnInsertion() OVERRIDE;                                                                                                                  // +0x50
	virtual void  OnRemoval() OVERRIDE;                                                                                                                    // +0x54
	virtual void  OnActivate() OVERRIDE;                                                                                                                   // +0x60
	virtual void  OnDeactivate() OVERRIDE;                                                                                                                 // +0x64

private:
	// Replaces the state with the main menu
	void OpenMainMenu();

	// Replaces the state with the "Attract" movie followed by a new attract state
	TBOOL StartAttractMovie();

	static TBOOL __stdcall OnFadeOver( AFrontEndAttractState* a_pState, AFade* a_pFade, TBOOL* a_pData );

public:
	inline static TFLOAT ms_fAttractIdleTime = 30.0f; // 0x00772630

private:
	TFLOAT                m_fBlinkTime;        // 0xa4
	TFLOAT                m_fIdleTime;         // 0xa8
	FadeOverListener      m_oFadeOverListener; // 0xac
	Toshi::T2GUIRectangle m_oPlank;            // 0xc0, "Wood_Thin", blinks
	AGUI2TextBox          m_oText;             // 0x138, child of m_oPlank
	AWindmillHelper*      m_pWindmillHelper;   // 0x1b0
	Toshi::T2GUIRectangle m_oTitle;            // 0x1b4, "BY_TITLE" (languages other than English)
	Toshi::T2GUIRectangle m_oTitleShadow;      // 0x22c
};
