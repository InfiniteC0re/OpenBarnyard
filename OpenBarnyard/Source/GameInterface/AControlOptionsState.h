#pragma once
#include "ABYardMenuState.h"
#include "GUI/AGUI2MenuGridTextItem.h"
#include "GUI/AGUI2Slider.h"

//-----------------------------------------------------------------------------
// A menu grid item that shows a text between a left and a right arrow, used
// to switch an option. Clicking an arrow emulates a press of the Left/Right key.
// Size 0x1ec, vtable 0071b5a0. Name from the Wii build.
//-----------------------------------------------------------------------------
class AToggleSlider : public AGUI2MenuGridItem
{
public:
	using ArrowFocusCallback = void ( * )( void* a_pUserData, TINT a_iSliderID );

public:
	AToggleSlider();
	~AToggleSlider();

	TBOOL Create( const TCHAR* a_szLeftArrow, const TCHAR* a_szRightArrow, const TWCHAR* a_wszText );

	// Handles mouse clicks on the arrows by emulating Left/Right key presses
	TBOOL ProcessInputEvent( const Toshi::TInputInterface::InputEvent* a_pInputEvent );

	// Changes the text without resizing the text box
	void SetText( const TWCHAR* a_wszText ) { m_oTextBox.SetText( a_wszText ); }

	void SetId( TINT a_iID ) { m_iID = a_iID; }
	TINT GetId() const { return m_iID; }

	// Unlinks the arrows, the text and the slider itself
	void RemoveElements()
	{
		for ( TINT i = 0; i < TARRAYSIZE( m_aArrows ); i++ )
			m_aArrows[ i ].RemoveSelf();

		m_oTextBox.RemoveSelf();
		RemoveSelf();
	}

	void SetCallbackUserData( void* a_pUserData ) { m_pCallbackUserData = a_pUserData; }
	void SetArrowFocusedCallback( ArrowFocusCallback a_fnCallback ) { m_fnArrowFocusCallback = a_fnCallback; }

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void SetAlpha( TFLOAT a_fAlpha ) OVERRIDE;

	//-----------------------------------------------------------------------------
	// AGUI2MenuGridItem
	//-----------------------------------------------------------------------------
	virtual void OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;

private:
	TINT                  m_iID;                    // 0x70
	Toshi::T2GUIRectangle m_aArrows[ 2 ];           // 0x74
	AGUI2TextBox          m_oTextBox;               // 0x164
	ArrowFocusCallback    m_fnArrowFocusCallback;   // 0x1dc
	void*                 m_pCallbackUserData;      // 0x1e0
	TBOOL                 m_bMouseDirty;            // 0x1e4
	TINT                  m_iLastPressedArrowIndex; // 0x1e8
};

//-----------------------------------------------------------------------------
// A menu grid item holding an AGUI2Slider, used for the mouse sensitivity.
// Size 0x1d8, vtable 0071b608. Name from the Wii build (ACameraSlider).
//-----------------------------------------------------------------------------
class ACameraSlider : public AGUI2MenuGridItem
{
public:
	ACameraSlider() = default;
	~ACameraSlider();

	AGUI2Slider& GetSlider() { return m_oSlider; }

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void SetAlpha( TFLOAT a_fAlpha ) OVERRIDE;

private:
	TINT        m_iUnk;    // 0x70
	AGUI2Slider m_oSlider; // 0x74
};

class AControlOptionsState : public ABYardMenuState
{
public:
	TDECLARE_CLASS( AControlOptionsState, ABYardMenuState );

	enum CONTROLOPTIONS
	{
		CONTROLOPTIONS_INVERTMOUSE,
		CONTROLOPTIONS_SENSITIVITY,
		CONTROLOPTIONS_NUMOF
	};

	// Number of the toggle options (only the mouse inversion)
	static constexpr TINT NUM_TOGGLES = 1;

public:
	AControlOptionsState();
	~AControlOptionsState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) OVERRIDE;
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnRemoval() OVERRIDE;
	virtual void  OnActivate() OVERRIDE;

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
	void UpdateLabelScale( AGUI2MenuGridTextItem& a_rLabel, TFLOAT a_fDeltaTime );

private:
	// Scale of the labels, shrunk when a label is wider than ms_fMaxLabelWidth
	inline static TFLOAT ms_fLabelScale        = 1.0f; // 007735cc
	inline static TFLOAT ms_fFocusedLabelScale = 1.1f; // 007735d0
	// NOTE: initialised with TMath::MAXFLOAT by a static initialiser (007079f0), so the labels are never shrunk
	inline static TFLOAT ms_fMaxLabelWidth = Toshi::TMath::MAXFLOAT; // 00781c6c

private:
	AGUI2MenuGrid         m_oGridMenu;                     // 0x7d8
	AGUI2MenuGridTextItem m_aToggleTexts[ NUM_TOGGLES ];   // 0x89c
	AToggleSlider         m_aToggleSliders[ NUM_TOGGLES ]; // 0xab0
	TINT                  m_iFocusedItemId;                // 0xc9c
	TBOOL                 m_bInvertMouse;                  // 0xca0
	AGUI2MenuGridTextItem m_oSensitivityText;              // 0xca4
	ACameraSlider         m_oSensitivitySlider;            // 0xeb8
	TFLOAT                m_fSensitivity;                  // 0x1090
};
