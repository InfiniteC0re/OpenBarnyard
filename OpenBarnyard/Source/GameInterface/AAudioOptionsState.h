#pragma once
#include "ABYardMenuState.h"
#include "GUI/AGUI2MenuGridTextItem.h"

class AVolumeSlider : public AGUI2MenuGridItem
{
public:
	using ArrowFocusCallback = void ( * )( void* a_pUserData, TINT a_iSliderID );

public:
	AVolumeSlider();
	~AVolumeSlider();

	TBOOL Create( const TCHAR* a_szLeftArrow, const TCHAR* a_szRightArrow, const TCHAR* a_szBarIcon );

	void SetId( TINT a_iID ) { m_iID = a_iID; }
	TINT GetId() const { return m_iID; }

	// Handles mouse clicks on the arrows by emulating Left/Right key presses
	TBOOL ProcessInputEvent( const Toshi::TInputInterface::InputEvent* a_pInputEvent );

	Toshi::T2GUIRectangle& GetBar( TINT a_iIndex ) { return m_aBars[ a_iIndex ]; }
	static constexpr TINT  GetNumBars() { return 5; }

	// Unlinks the arrows, the bars and the slider itself
	void RemoveElements()
	{
		for ( TINT i = 0; i < TARRAYSIZE( m_aArrows ); i++ )
			m_aArrows[ i ].RemoveSelf();

		for ( TINT i = 0; i < TARRAYSIZE( m_aBars ); i++ )
			m_aBars[ i ].RemoveSelf();

		RemoveSelf();
	}

	void SetCallbackUserData( void* a_pUserData ) { m_pCallbackUserData = a_pUserData; }
	void SetArrowFocusedCallback( ArrowFocusCallback a_fnCallback ) { m_fnArrowFocusCallback = a_fnCallback; }

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void SetAlpha( TFLOAT a_fAlpha ) OVERRIDE; // +0x38

	//-----------------------------------------------------------------------------
	// AGUI2MenuGridItem
	//-----------------------------------------------------------------------------
	virtual void OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;

private:
	inline static TFLOAT ms_fBarScale = 0.5f;

private:
	TINT                  m_iID;
	Toshi::T2GUIRectangle m_aArrows[ 2 ];
	Toshi::T2GUIRectangle m_aBars[ 5 ];
	ArrowFocusCallback    m_fnArrowFocusCallback;
	void*                 m_pCallbackUserData;
	TBOOL                 m_bMouseDirty;
	TINT                  m_iLastPressedArrowIndex;
};

class MODAPI AAudioOptionsState : public ABYardMenuState
{
public:
	TDECLARE_CLASS( AAudioOptionsState, ABYardMenuState );

	enum AUDIOOPTIONS
	{
		AUDIOOPTIONS_EFFECTS,
		AUDIOOPTIONS_MUSIC,
		AUDIOOPTIONS_SPEECH,
		AUDIOOPTIONS_NUMOF
	};

	inline constexpr static TINT AUDIOOPTIONS_TEXTS[ AUDIOOPTIONS_NUMOF ] = {
		299,
		300,
		301
	};

public:
	AAudioOptionsState();
	~AAudioOptionsState();

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
	void ChangeFocusedVolume( TINT a_iDelta );
	void UpdateBarColours( TINT a_iSlider, TFLOAT a_fDeltaTime );

private:
	AGUI2MenuGrid         m_oGridMenu;
	AGUI2MenuGridTextItem m_aTextItems[ AUDIOOPTIONS_NUMOF ];
	AVolumeSlider         m_aVolumeSliders[ AUDIOOPTIONS_NUMOF ];
	TINT                  m_aVolumes[ AUDIOOPTIONS_NUMOF ];         // 0x1a3c
	TINT                  m_aOriginalVolumes[ AUDIOOPTIONS_NUMOF ]; // 0x1a48
	TINT                  m_iFocusedItemId;                         // 0x1a54
	TINT                  m_iMusicCue;                              // 0x1a58, music sample played while the music volume is focused
	TBOOL                 m_bSettingsChangedOnInsertion;            // 0x1a5c
	TBOOL                 m_bLoadedMusicSamples;                    // 0x1a5d
};
