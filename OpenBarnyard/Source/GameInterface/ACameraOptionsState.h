#pragma once
#include "ABYardMenuState.h"
#include "GUI/AGUI2MenuGridTextItem.h"
#include "AControlOptionsState.h"

#ifdef BARNYARD_LATEST

//-----------------------------------------------------------------------------
// Purpose: the camera speed slider of ACameraOptionsState: a left arrow, five
// bar icons and a right arrow. The arrows spin once when the value changes.
// Only exists in the latest version of the game (Wii), size 0x3cc there.
// NOTE: the Wii calls it ACameraSlider (vtable 80507928). That's the newer version of the PC ACameraSlider
// (AControlOptionsState), which holds an AGUI2Slider instead, so it has another name here.
//-----------------------------------------------------------------------------
class ACameraSpeedSlider : public AGUI2MenuGridItem
{
public:
	using ArrowFocusCallback = void ( * )( void* a_pUserData, TINT a_iSliderID );

	static constexpr TINT NUM_BARS = 5;

public:
	ACameraSpeedSlider();
	~ACameraSpeedSlider();

	TBOOL Create( const TCHAR* a_szLeftArrow, const TCHAR* a_szRightArrow, const TCHAR* a_szBarIcon );

	// Unlinks the arrows and the bars
	void RemoveElements();

	// Lights the first a_iNumBars bars and darkens the others
	void SetNumLitBars( TINT a_iNumBars );

	// Spins the right arrow if a_iDirection >= 0, the left arrow otherwise
	void SpinArrow( TINT a_iDirection );

	void SetId( TINT a_iID ) { m_iID = a_iID; }
	TINT GetId() const { return m_iID; }

	void SetCallbackUserData( void* a_pUserData ) { m_pCallbackUserData = a_pUserData; }
	void SetArrowFocusedCallback( ArrowFocusCallback a_fnCallback ) { m_fnArrowFocusCallback = a_fnCallback; }

	Toshi::T2GUIRectangle& GetBar( TINT a_iIndex ) { return m_aBars[ a_iIndex ]; }

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void SetAlpha( TFLOAT a_fAlpha ) OVERRIDE;

	//-----------------------------------------------------------------------------
	// AGUI2MenuGridItem
	//-----------------------------------------------------------------------------
	virtual void   OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual TFLOAT GetFlowVisualOffset() OVERRIDE;
	virtual TFLOAT GetFlowOffset() OVERRIDE;

private:
	// NOTE: the original keeps the id at 0x70 of AGUI2MenuGridItem
	TINT                  m_iID;
	Toshi::T2GUIRectangle m_aArrows[ 2 ];         // 0x74
	Toshi::T2GUIRectangle m_aBars[ NUM_BARS ];    // 0x164
	ArrowFocusCallback    m_fnArrowFocusCallback; // 0x3bc, nothing calls it in the original
	void*                 m_pCallbackUserData;    // 0x3c0
	TFLOAT                m_aArrowSpin[ 2 ];      // 0x3c4, progress of the spin of each arrow (0-1, 1 when idle)
};

//-----------------------------------------------------------------------------
// Purpose: options screen with the camera speed slider (AOptions::SetCameraSpeed).
// Opened from AOptionsState. Only exists in the latest version of the game (Wii),
// where it's an AGUI2 menu like the audio options (not XUI), size 0xfa8 there.
// NOTE: the "Wii Style" and "Follow Jumps" toggles are not in the original. They switch
// the camera behaviours of the latest version on and off at runtime (AOptions). The speed
// slider of the original is hidden for now (SHOW_SPEED_SLIDER).
//-----------------------------------------------------------------------------
class ACameraOptionsState : public ABYardMenuState
{
public:
	TDECLARE_CLASS( ACameraOptionsState, ABYardMenuState );

	// Text of the slider label and of the options menu button in the latest version ("Camera Speed")
	static constexpr TINT CAMERA_SPEED_TEXT = 3482;

	// Rows of the menu, the ids of their labels
	enum CAMERAOPTIONS
	{
		CAMERAOPTIONS_SPEED,
		// NOTE: the toggles are not in the original
		CAMERAOPTIONS_WIISTYLE,
		CAMERAOPTIONS_FOLLOWJUMPS,
		CAMERAOPTIONS_NUMOF
	};

	static constexpr TINT NUM_TOGGLES = CAMERAOPTIONS_NUMOF - CAMERAOPTIONS_WIISTYLE;

	// NOTE: the speed row (the only one of the original) is hidden for now. The Wii Style camera uses the default
	// speed (AOptions::CAMERA_SPEED_DEFAULT). Set to TTRUE to bring the slider back.
	static constexpr TBOOL SHOW_SPEED_SLIDER = TFALSE;

	// First row shown in the menu
	static constexpr TINT FIRST_OPTION = SHOW_SPEED_SLIDER ? CAMERAOPTIONS_SPEED : CAMERAOPTIONS_WIISTYLE;
	static constexpr TINT NUM_ROWS     = CAMERAOPTIONS_NUMOF - FIRST_OPTION;

	// Title of the dialog
	static constexpr TINT TITLE_TEXT = 294;

public:
	ACameraOptionsState();
	~ACameraOptionsState();

	// Text of the button in the options menu
	static const TWCHAR* GetCameraText();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) OVERRIDE;
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnRemoval() OVERRIDE;
	virtual void  OnActivate() OVERRIDE;
	virtual void  OnDeactivate() OVERRIDE;

	//-----------------------------------------------------------------------------
	// ABYardMenuState
	//-----------------------------------------------------------------------------
	virtual void OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem ) OVERRIDE;
	virtual void OnMenuItemFocused( AGUI2MenuItem& a_rOldFocus, AGUI2MenuItem& a_rNewFocus ) OVERRIDE;
	virtual void OnMenuClose() OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void OnMenuGridItemFocused( AGUI2MenuGridItem& a_rOldFocus, AGUI2MenuGridItem& a_rNewFocus );

private:
	// Changes the speed for AInputCommand_Left/AInputCommand_Right
	void ChangeCameraSpeed( AInputCommand a_eInputCommand );

	// Switches a toggle (CAMERAOPTIONS_WIISTYLE or CAMERAOPTIONS_FOLLOWJUMPS) and applies it
	void SwitchToggle( TINT a_iOption );

	// Applies the value of a toggle to AOptions and shows it
	void SetToggle( TINT a_iOption, TBOOL a_bValue );

	AGUI2MenuGridTextItem& GetLabel( TINT a_iOption ) { return ( a_iOption == CAMERAOPTIONS_SPEED ) ? m_oTextItem : m_aToggleTexts[ a_iOption - CAMERAOPTIONS_WIISTYLE ]; }

	void UpdateLabelScale( AGUI2MenuGridTextItem& a_rLabel, TFLOAT a_fDeltaTime );

#  ifdef TOSHI_SKU_WII
	// Wii only: tilting the Wii Remote left/right changes the speed (with auto repeat)
	void UpdateRemoteInput( TFLOAT a_fDeltaTime );
#  endif

private:
	AGUI2MenuGrid         m_oGridMenu;                   // 0x7e4
	AGUI2MenuGridTextItem m_oTextItem;                   // 0x8a8
	ACameraSpeedSlider    m_oSlider;                     // 0xac8
	TINT                  m_iCameraSpeed;                // 0xe94
	TINT                  m_iOriginalCameraSpeed;        // 0xe98, restored if the menu is cancelled
	TINT                  m_iFocusedItemId;              // 0xe9c
	TBOOL                 m_bSettingsChangedOnInsertion; // 0xea0

	// NOTE: not in the original
	AGUI2MenuGridTextItem m_aToggleTexts[ NUM_TOGGLES ];
	AToggleSlider         m_aToggleSliders[ NUM_TOGGLES ];
	TBOOL                 m_abToggles[ NUM_TOGGLES ];
	TBOOL                 m_abOriginalToggles[ NUM_TOGGLES ]; // restored if the menu is cancelled
#  ifdef TOSHI_SKU_WII
	TBOOL                  m_bRemoteRight;    // 0xea1
	TBOOL                  m_bRemoteLeft;     // 0xea2
	TBOOL                  m_bRemoteNeutral;  // 0xea3
	AGUI2RevControllerAnim m_oControllerAnim; // 0xea4, shows how to hold the Wii Remote
#  endif
};

#endif // BARNYARD_LATEST
