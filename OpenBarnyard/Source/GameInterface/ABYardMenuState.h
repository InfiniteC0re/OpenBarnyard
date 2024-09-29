#pragma once
#include "AGameState.h"
#include "GUI/AGUI2Element.h"
#include "GUI/AGUI2Dialog.h"
#include "GUI/AGUI2Rectangle.h"
#include "GUI/AGUI2TextBox.h"
#include "GUI/AGUI2Menu.h"
#include "GUI/AGUI2Button.h"

class ABYardMenuState : public AGameState
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
	virtual void OnMenuItemActivated( AGUI2MenuItem& a_rMenuItem ) = 0;
	virtual void OnMenuItemFocused( AGUI2MenuItem& a_rOldFocus, AGUI2MenuItem& a_rNewFocus );
	virtual void OnMenuClose();

protected:
	void OnButtonActivated( AGUI2Button* a_pButton );

private:
	void UpdateMenuOpacity();
	void SetDialogOpacity( TFLOAT a_fOpacity );

protected:
	inline static Toshi::TVector2 ms_vecActivatedButtonTranslation;
	inline static TFLOAT          ms_fAnimationTime;
	inline static Toshi::TVector2 ms_vecActivatedButtonShadow;

protected:
	AGUI2Element   m_oRootElement;
	AGUI2Dialog    m_oDialog;
	AGUI2Menu      m_oMenu;
	AGUI2Rectangle m_oDialogTitleBackground;
	AGUI2TextBox   m_oDialogTitle;
	MENUSTATE      m_eMenuState;
	TFLOAT         m_fOscillatorOffset;
	TFLOAT         m_fMenuOpacity;
	TFLOAT         m_fTotalTime;
	TBOOL          m_bFocusedElementBloated;
	TBOOL          m_bIgnoreInputs;
	TFLOAT*        m_pButtonRotations;
	TBOOL          m_bHasRectangle2;
	AGUI2Rectangle m_oRectangle2;
};
