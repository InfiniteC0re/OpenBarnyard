#pragma once
#include "ABYardMenuState.h"
#include "GUI/AGUI2MenuGridTextItem.h"

class AVolumeSlider :
    public AGUI2MenuGridItem
{
public:
	using ArrowFocusCallback = void ( * )( void* a_pUserData, TINT a_iSliderID );

public:
	AVolumeSlider();
	~AVolumeSlider();

	TBOOL Create( const TCHAR* a_szLeftArrow, const TCHAR* a_szRightArrow, const TCHAR* a_szBarIcon );

	void SetId( TINT a_iID ) { m_iID = a_iID; }
	TINT GetId() const { return m_iID; }

	void SetCallbackUserData( void* a_pUserData ) { m_pCallbackUserData = a_pUserData; }
	void SetArrowFocusedCallback( ArrowFocusCallback a_fnCallback ) { m_fnArrowFocusCallback = a_fnCallback; }

	//-----------------------------------------------------------------------------
	// AGUI2MenuGridItem
	//-----------------------------------------------------------------------------
	virtual void OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;

private:
	inline static TFLOAT ms_fBarScale = 0.5f;

private:
	TINT               m_iID;
	AGUI2Rectangle     m_aArrows[ 2 ];
	AGUI2Rectangle     m_aBars[ 5 ];
	ArrowFocusCallback m_fnArrowFocusCallback;
	void*              m_pCallbackUserData;
	TBOOL              m_bMouseDirty;
	TINT               m_iLastPressedArrowIndex;
};

class AAudioOptionsState :
    public ABYardMenuState
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
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnRemoval() OVERRIDE;

	//-----------------------------------------------------------------------------
	// ABYardMenuState
	//-----------------------------------------------------------------------------
	virtual void OnMenuItemActivated( AGUI2MenuItem& a_rMenuItem ) OVERRIDE;
	virtual void OnMenuItemFocused( AGUI2MenuItem& a_rOldFocus, AGUI2MenuItem& a_rNewFocus ) OVERRIDE;
	virtual void OnMenuClose() OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void OnMenuGridItemFocused( AGUI2MenuGridItem& a_rOldFocus, AGUI2MenuGridItem& a_rNewFocus );

private:
	AGUI2MenuGrid         m_oGridMenu;
	AGUI2MenuGridTextItem m_aTextItems[ AUDIOOPTIONS_NUMOF ];
	AVolumeSlider         m_aVolumeSliders[ AUDIOOPTIONS_NUMOF ];
	// ...
	TINT m_iFocusedItemId;
};
