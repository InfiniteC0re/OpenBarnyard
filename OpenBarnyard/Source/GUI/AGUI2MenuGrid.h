#pragma once
#include "AGUI2Element.h"
#include "Input/AInputMap.h"
#include "Sound/ASound.h"

class AGUI2MenuGridItem;

class AGUI2MenuGrid : public AGUI2Element
{
public:
	using ACTIONFLAGS = TUINT32;
	enum ACTIONFLAGS_ : ACTIONFLAGS
	{
		ACTIONFLAGS_NONE         = 0,
		ACTIONFLAGS_UP           = BITFLAG( 0 ),
		ACTIONFLAGS_DOWN         = BITFLAG( 1 ),
		ACTIONFLAGS_LEFT         = BITFLAG( 2 ),
		ACTIONFLAGS_RIGHT        = BITFLAG( 3 ),
		ACTIONFLAGS_OK           = BITFLAG( 4 ),
		ACTIONFLAGS_CANCEL       = BITFLAG( 5 ),
		ACTIONFLAGS_MOUSEUPDATED = BITFLAG( 6 ),
	};

	using ActivateCallback = void ( * )( void* a_pUserData, AGUI2MenuGridItem* a_pItem );
	using FocusCallback    = void ( * )( void* a_pUserData, AGUI2MenuGridItem* a_pOldFocus, AGUI2MenuGridItem* a_pNewFocus );

public:
	// constructors/destructor
	AGUI2MenuGrid();
	~AGUI2MenuGrid();

	TBOOL CreateGrid( TINT a_iNumColumns, TINT a_iNumRows, TFLOAT a_fColumnGap, TFLOAT a_fRowGap );

	void  Update( TFLOAT a_fDeltaTime );
	TBOOL ProcessInputEvent( const Toshi::TInputInterface::InputEvent* a_pEvent );
	TBOOL ProcessInputCommand( AInputCommand a_eCommand, const Toshi::TInputInterface::InputEvent* a_pEvent );

	void AddMenuGridItem( AGUI2MenuGridItem& a_rMenuItem );
	void SetFocusAt( AGUI2MenuGridItem* a_pMenuItem );

	// Reflows elements and calculates dimensions
	void ReflowChildren();

	TBOOL TriggerButtonPress( AGUI2MenuGridItem& a_rMenuItem );

	void SetMouseStateDirty() { m_bMouseStateDirty = TTRUE; }

	AGUI2MenuGridItem* GetFocusedMenuItem() const { return m_pFocusedMenuItem; }
	AGUI2MenuGridItem* GetHoveredMenuItem() const { return m_pHoveredMenuItem; }
	AGUI2MenuGridItem* GetLastMenuItem() const { return m_pLastMenuItem; }

	void SetCallbackUserData( void* a_pUserData ) { m_pCallbackUserData = a_pUserData; }
	void SetItemActivationCallback( ActivateCallback a_fnCallback ) { m_fnActivateCallback = a_fnCallback; }
	void SetItemFocusCallback( FocusCallback a_fnCallback ) { m_fnFocusCallback = a_fnCallback; }

	void SetFocusSound( ASoundId a_eSound ) { m_eFocusSound = a_eSound; }
	void SetSelectSound( ASoundId a_eSound ) { m_eSelectSound = a_eSound; }
	void SetBackSound( ASoundId a_eSound ) { m_eBackSound = a_eSound; }

	void BlockActivation( TBOOL a_bBlock ) { m_bBlockActivation = a_bBlock; }

private:
	TFLOAT             m_fItemSpacing;
	AGUI2MenuGridItem* m_pDefaultFocusElement;
	AGUI2MenuGridItem* m_pCancelItem;
	ASoundId           m_eFocusSound;
	ASoundId           m_eSelectSound;
	ASoundId           m_eBackSound;
	AGUI2MenuGridItem* m_pLastMenuItem;
	AGUI2MenuGridItem* m_pFocusedMenuItem;
	ACTIONFLAGS        m_eActionFlags;
	Toshi::TVector2    m_vecMousePos;
	ActivateCallback   m_fnActivateCallback;
	FocusCallback      m_fnFocusCallback;
	void*              m_pCallbackUserData;
	TFLOAT             m_fTime;
	TBOOL              m_bFlag1;
	TINT               m_iNumRows;
	TINT               m_iNumColumns;
	Toshi::TVector2    m_vecMaxItemDimensions;
	TFLOAT             m_fColumnGap;
	TFLOAT             m_fRowGap;
	TFLOAT*            m_pColumnsData;
	TFLOAT*            m_pRowsData;
	TFLOAT             m_fPaddingLeft;
	TFLOAT             m_fMaxItemWidth;
	TBOOL              m_bBlockActivation;
	TBOOL              m_bMouseStateDirty;
	AGUI2MenuGridItem* m_pHoveredMenuItem;
};

class AGUI2MenuGridItem : public AGUI2Element
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
