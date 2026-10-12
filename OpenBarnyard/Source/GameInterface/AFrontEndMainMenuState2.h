#pragma once
#include "ABYardMenuState.h"
#include "Helpers/AWindmillHelper.h"
#include "GUI/AGUI2Button.h"
#include "GUI/AFade.h"
#include "AMessagePopupState.h"

class AFrontEndMainMenuState2 : public ABYardMenuState
{
public:
	TDECLARE_CLASS( AFrontEndMainMenuState2, ABYardMenuState );

	enum FRONTENDBUTTON
	{
		FRONTENDBUTTON_NEWGAME,
		FRONTENDBUTTON_LOADGAME,
		FRONTENDBUTTON_OPTIONS,
		FRONTENDBUTTON_BONUSES,
		FRONTENDBUTTON_ANTICS,
		FRONTENDBUTTON_QUIT,
		FRONTENDBUTTON_NUMOF
	};

	inline constexpr static TINT FRONTENDBUTTON_TEXTS[ FRONTENDBUTTON_NUMOF ] = {
		292, 293, 294, 296, 298, 333
	};

public:
	// constructors/destructor
	AFrontEndMainMenuState2( AWindmillHelper* a_pWindmillHelper = TNULL, TBOOL a_bFlag = TFALSE );
	~AFrontEndMainMenuState2();

	TBOOL StartIntroVideo();

	// Sets camera to it's position it should have in front end
	static void SetupCamera();

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
	virtual void OnMenuItemFocused( AGUI2MenuItem& a_rOldFocus, AGUI2MenuItem& a_rNewFocus ) OVERRIDE {} // +0x70, DOES_NOTHING
	virtual void OnMenuClose() OVERRIDE;

private:
	// Quits the game if Yes was chosen in the quit popup
	static TBOOL __stdcall OnQuitPopupResponse( AFrontEndMainMenuState2* a_pState, AMessagePopupState* a_pPopup, AMessagePopupState::ResponseEvent* a_pEvent );

public:
	inline static TBOOL  ms_bIsInserted       = TFALSE;
	inline static TFLOAT ms_fPlayIntroAFKTime = 30.0f;

private:
	AGUI2Button                                                                                      m_aButtons[ FRONTENDBUTTON_NUMOF ];
	TINT                                                                                             m_iActivatedButtonID;
	Toshi::T2GUIRectangle                                                                            m_Rectangle1;
	Toshi::T2GUIRectangle                                                                            m_Rectangle2;
	Toshi::TListener<AMessagePopupState, AMessagePopupState::ResponseEvent, AFrontEndMainMenuState2> m_oQuitPopupListener;
	AWindmillHelper*                                                                                 m_pWindmillHelper;
	TBOOL                                                                                            m_bFlag;
	TFLOAT                                                                                           m_fAFKTime;
	Toshi::TListener<AFade, TBOOL, AFrontEndMainMenuState2>                                          m_oFadeOverListener;
};
