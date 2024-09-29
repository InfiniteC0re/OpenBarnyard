#pragma once
#include "AGUI2Element.h"
#include "Input/AInputMap.h"
#include "Sound/ASound.h"

class AGUI2MenuItem;

class AGUI2Menu : public AGUI2Element
{
public:
	using ACTIONFLAGS = TUINT32;
	enum ACTIONFLAGS_ : ACTIONFLAGS
	{
		ACTIONFLAGS_NONE   = 0,
		ACTIONFLAGS_UP     = BITFLAG( 0 ),
		ACTIONFLAGS_DOWN   = BITFLAG( 1 ),
		ACTIONFLAGS_LEFT   = BITFLAG( 2 ),
		ACTIONFLAGS_RIGHT  = BITFLAG( 3 ),
		ACTIONFLAGS_OK     = BITFLAG( 4 ),
		ACTIONFLAGS_CANCEL = BITFLAG( 5 ),
	};

	using ActivateCallback = void ( * )( void* a_pUserData, AGUI2MenuItem* a_pItem );
	using FocusCallback    = void ( * )( void* a_pUserData, AGUI2MenuItem* a_pOldFocus, AGUI2MenuItem* a_pNewFocus );

public:
	// constructors/destructor
	AGUI2Menu();
	~AGUI2Menu();

	void  Update( TFLOAT a_fDeltaTime );
	TBOOL ProcessInputEvent( const Toshi::TInputInterface::InputEvent* a_pEvent );
	TBOOL ProcessInputCommand( AInputCommand a_eCommand, const Toshi::TInputInterface::InputEvent* a_pEvent );

	void AddMenuItem( AGUI2MenuItem& a_rMenuItem );
	void SetFocusAt( AGUI2MenuItem& a_rMenuItem );

	// Reflows elements vertically and calculates dimensions
	void ReflowChildrenVertically();

	TBOOL TriggerButtonPress( AGUI2MenuItem& a_rMenuItem );

	void SetMenuAlpha( TFLOAT a_fAlpha, TFLOAT a_fShadowAlpha = -1.0f );
	void SetMouseStateDirty() { m_bMouseStateDirty = TTRUE; }

	AGUI2MenuItem* GetFocusedMenuItem() const { return m_pFocusedMenuItem; }
	AGUI2MenuItem* GetHoveredMenuItem() const { return m_pHoveredMenuItem; }
	AGUI2MenuItem* GetLastMenuItem() const { return m_pLastMenuItem; }

	void SetCallbackUserData( void* a_pUserData ) { m_pCallbackUserData = a_pUserData; }
	void SetItemActivationCallback( ActivateCallback a_fnCallback ) { m_fnActivateCallback = a_fnCallback; }
	void SetItemFocusCallback( FocusCallback a_fnCallback ) { m_fnFocusCallback = a_fnCallback; }

	void SetFocusSound( ASoundId a_eSound ) { m_eFocusSound = a_eSound; }
	void SetSelectSound( ASoundId a_eSound ) { m_eSelectSound = a_eSound; }
	void SetBackSound( ASoundId a_eSound ) { m_eBackSound = a_eSound; }
	void SetNegativeSound( ASoundId a_eSound ) { m_eNegativeSound = a_eSound; }

private:
	void ActivateFocusedButton();

private:
	TFLOAT           m_fItemSpacing;
	AGUI2MenuItem*   m_pDefaultFocusElement;
	AGUI2MenuItem*   m_pCancelItem;
	ASoundId         m_eFocusSound;
	ASoundId         m_eSelectSound;
	ASoundId         m_eBackSound;
	ASoundId         m_eNegativeSound;
	AGUI2MenuItem*   m_pLastMenuItem;
	AGUI2MenuItem*   m_pFocusedMenuItem;
	ACTIONFLAGS      m_eActionFlags;
	TINT             m_Unk3;
	TINT             m_Unk4;
	ActivateCallback m_fnActivateCallback;
	FocusCallback    m_fnFocusCallback;
	void*            m_pCallbackUserData;
	TFLOAT           m_fTime;
	TBOOL            m_bFlag1;
	TBOOL            m_bHorizontalFlow;
	TINT             m_iNumMenuItems;
	TBOOL            m_bMouseStateDirty;
	AGUI2MenuItem*   m_pHoveredMenuItem;
};

class AGUI2MenuItem : public AGUI2Element
{
public:
	friend class AGUI2Menu;

	enum COMMANDRESULT
	{
		COMMANDRESULT_NONE,
		COMMANDRESULT_OK,
		COMMANDRESULT_CANCEL,
	};

public:
	// constructors/destructor
	AGUI2MenuItem();
	~AGUI2MenuItem();

	void LinkMenuItemBefore( AGUI2MenuItem& a_rLinkAfter );

	TBOOL IsEnabled() const { return m_bEnabled; }
	TBOOL CanFocus() const { return IsEnabled() && IsVisible(); }

	AGUI2MenuItem* GetNextMenuItem() const { return m_pNextMenuItem; }
	AGUI2MenuItem* GetPrevMenuItem() const { return m_pPrevMenuItem; }

	//-----------------------------------------------------------------------------
	// AGUI2Element
	//-----------------------------------------------------------------------------
	virtual void SetAlpha( TFLOAT a_fAlpha ) OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void          OnFocus();
	virtual void          OnFocusLost();
	virtual void          OnUpdate( TFLOAT a_fDeltaTime );
	virtual COMMANDRESULT OnInputCommand( AGUI2Menu::ACTIONFLAGS& a_rActionFlags );
	virtual void          SetEnabled( TBOOL a_bEnabled );
	virtual TFLOAT        GetFlowVisualOffset();
	virtual TFLOAT        GetFlowOffset();

private:
	AGUI2MenuItem* m_pNextMenuItem;
	AGUI2MenuItem* m_pPrevMenuItem;
	TBOOL          m_bEnabled;
};
