#pragma once
#include "ABYardMenuState.h"
#include "GUI/AGUI2Button.h"

class MODAPI AOptionsState : public ABYardMenuState
{
public:
	TDECLARE_CLASS( AOptionsState, ABYardMenuState );

	enum OPTIONSBUTTON
	{
		OPTIONSBUTTON_AUDIO,
		OPTIONSBUTTON_CONTROLS,
#ifdef BARNYARD_COMMUNITY_PATCH
		// NOTE: only the EU PS2 builds have it (item 2, opens AFrontEndLanguageState)
		OPTIONSBUTTON_LANGUAGE,
#endif
#ifdef BARNYARD_LATEST
		// NOTE: the latest version (Wii) hides the controls button and adds this one
		OPTIONSBUTTON_CAMERA,
		// NOTE: not in the latest version either, the Wii asks for the difficulty when a new game starts
		OPTIONSBUTTON_DIFFICULTY,
#endif
		OPTIONSBUTTON_NUMOF
	};

	inline constexpr static TINT OPTIONSBUTTON_TEXTS[ OPTIONSBUTTON_NUMOF ] = {
		282,
		281,
#ifdef BARNYARD_COMMUNITY_PATCH
		1120, // "Language"
#endif
#ifdef BARNYARD_LATEST
		3482, // ACameraOptionsState::CAMERA_SPEED_TEXT (the button shows ACameraOptionsState::GetCameraText)
		3476, // ADifficultyOptionState::TITLE_TEXT (the button shows ADifficultyOptionState::GetText)
#endif
	};

public:
	AOptionsState();
	~AOptionsState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnRemoval() OVERRIDE;
	virtual void  OnActivate() OVERRIDE;

	//-----------------------------------------------------------------------------
	// ABYardMenuState
	//-----------------------------------------------------------------------------
	virtual void OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem ) OVERRIDE;
	virtual void OnMenuClose() OVERRIDE;

#ifdef BARNYARD_COMMUNITY_PATCH
	// Shows the texts of the current language (AFrontEndLanguageState). Not on PC
	void UpdateTexts();
#else
	void UpdateTexts() {}
#endif

private:
	// NOTE: inlined in OnInsertion
	static const TWCHAR* GetButtonText( TINT a_iButton );

public:
	// Set by the audio/control option screens once a setting was changed,
	// closing the options menu then pushes the save state to store them
	inline static TBOOL ms_bSettingsChanged = TFALSE; // 0x00781ca0

private:
	AGUI2Button m_aButtons[ OPTIONSBUTTON_NUMOF ];
	TINT        m_iActivatedButtonID;
};
