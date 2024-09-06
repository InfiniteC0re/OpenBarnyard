#pragma once
#include "AGUI2Element.h"
#include "Input/AInputMap.h"
#include "Sound/ASound.h"

class AGUI2MenuItemGrid;

class AGUI2MenuGrid :
    public AGUI2Element
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

	using ActivateCallback = void ( * )( void* a_pUserData, AGUI2MenuItemGrid* a_pItem );
	using FocusCallback    = void ( * )( void* a_pUserData, AGUI2MenuItemGrid* a_pOldFocus, AGUI2MenuItemGrid* a_pNewFocus );

public:
};

class AGUI2MenuGridItem :
    public AGUI2Element
{
public:
	friend class AGUI2MenuGrid;

	enum COMMANDRESULT
	{
		COMMANDRESULT_NONE,
		COMMANDRESULT_OK,
		COMMANDRESULT_CANCEL,
	};

public:
	// constructors/destructor
	AGUI2MenuGridItem();
	~AGUI2MenuGridItem();

	void LinkMenuItemBefore( AGUI2MenuGridItem& a_rLinkAfter );

	void SetSideElements( AGUI2MenuGridItem* a_pLeft, AGUI2MenuGridItem* a_pRight, AGUI2MenuGridItem* a_pTop, AGUI2MenuGridItem* a_pBottom );

	TBOOL IsEnabled() const { return m_bEnabled; }
	TBOOL CanFocus() const { return IsEnabled() && IsVisible(); }

	AGUI2MenuGridItem* GetNextMenuItem() const { return m_pNextMenuGridItem; }
	AGUI2MenuGridItem* GetPrevMenuItem() const { return m_pPrevMenuGridItem; }

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void          OnFocus();
	virtual void          OnFocusLost();
	virtual void          OnUpdate( TFLOAT a_fDeltaTime );
	virtual COMMANDRESULT OnInputCommand( AGUI2MenuGrid::ACTIONFLAGS& a_rActionFlags );
	virtual void          SetEnabled( TBOOL a_bEnabled );
	virtual TFLOAT        GetFlowVisualOffset();
	virtual TFLOAT        GetFlowOffset();

private:
	AGUI2MenuGridItem* m_pNextMenuGridItem;
	AGUI2MenuGridItem* m_pPrevMenuGridItem;
	AGUI2MenuGridItem* m_pLeftElement;
	AGUI2MenuGridItem* m_pRightElement;
	AGUI2MenuGridItem* m_pTopElement;
	AGUI2MenuGridItem* m_pBottomElement;
	TBOOL              m_bEnabled;
};
