#pragma once
#include "AGameState.h"
#include "GUI/T2GUIElement.h"
#include "GUI/AGUI2Dialog.h"
#include "GUI/T2GUIRectangle.h"
#include "GUI/AGUI2TextBox.h"
#include "GUI/AGUI2Menu.h"
#include "GUI/AGUI2Button.h"

class MODAPI ABYardMenuState : public AGameState
{
public:
	TDECLARE_CLASS( ABYardMenuState, AGameState );

	enum MENUSTATE
	{
		MENUSTATE_DIALOG_APPEAR,
		MENUSTATE_DIALOG_DISAPPEAR,
		MENUSTATE_MENU_APPEAR,
		MENUSTATE_MENU_DISAPPEAR,
		MENUSTATE_NOTHING,
		MENUSTATE_MENU_VISIBLE,
	};

public:
	// constructors/destructor
	ABYardMenuState();
	~ABYardMenuState();

	void SetHasRectangle2( TBOOL a_bHasRectangle2 ) { m_bHasRectangle2 = a_bHasRectangle2; }

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
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void OnMenuOpen();
	virtual void OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem ) = 0;
	virtual void OnMenuItemFocused( AGUI2MenuItem& a_rOldFocus, AGUI2MenuItem& a_rNewFocus );
	virtual void OnMenuClose();

	// AGUI2Menu activation callback, calls OnMenuItemActivated of the state (a_pUserData)
	static void MenuItemActivationCallback( void* a_pUserData, AGUI2MenuItem* a_pItem );

	// AGUI2Menu focus callback, calls OnMenuItemFocused of the state (a_pUserData)
	static void MenuItemFocusCallback( void* a_pUserData, AGUI2MenuItem* a_pOldFocus, AGUI2MenuItem* a_pNewFocus );

	MENUSTATE GetMenuState() const { return m_eMenuState; }

protected:
	void OnButtonActivated( AGUI2Button* a_pButton );

	// Scales the focused button up and the other ones back
	void UpdateMenuButtons( TFLOAT a_fDeltaTime );

	// Animates the text of the activated button, then closes the menu
	void UpdateFocusedButton( TFLOAT a_fDeltaTime );

private:
	void UpdateMenuOpacity();
	void SetDialogOpacity( TFLOAT a_fOpacity );

public:
	// Alpha of the text shadows relative to the alpha of the text
	inline static constexpr TFLOAT s_fShadowAlpha = 0.6f;

protected:
	inline static Toshi::TVector2 ms_vecActivatedButtonTranslation;
	inline static TFLOAT          ms_fAnimationTime;
	inline static Toshi::TVector2 ms_vecActivatedButtonShadow;
	inline static TFLOAT          ms_fFocusedButtonMaxOffset    = 3.0f;  // 0x00773460
	inline static TFLOAT          ms_fFocusedButtonMaxScale     = 1.25f; // 0x00773464
	inline static TFLOAT          ms_fFocusedButtonTextMaxScale = 1.25f; // 0x00773468
	inline static TFLOAT          ms_fFocusedButtonAnimSpeed    = 20.0f; // 0x0077346c

protected:
	Toshi::T2GUIElement   m_oRootElement;
	AGUI2Dialog           m_oDialog;
	AGUI2Menu             m_oMenu;
	Toshi::T2GUIRectangle m_oDialogTitleBackground;
	AGUI2TextBox          m_oDialogTitle;
	MENUSTATE             m_eMenuState;
	TFLOAT                m_fOscillatorOffset;
	TFLOAT                m_fMenuOpacity;
	TFLOAT                m_fTotalTime;
	TBOOL                 m_bFocusedElementBloated;
	TBOOL                 m_bIgnoreInputs;
	TFLOAT*               m_pButtonRotations;
	TBOOL                 m_bHasRectangle2;
	Toshi::T2GUIRectangle m_oRectangle2;
};
