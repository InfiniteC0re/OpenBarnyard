#pragma once
#include "AHudElement.h"
#include "GUI/AGUI2TextBox.h"

#include <GUI/T2GUIRectangle.h>
#include <Toshi/TPString8.h>

//-----------------------------------------------------------------------------
// Purpose: item counter below ASimPlayerPickUpHudElement (an icon and "n" or
// "n/max"). Set up by AItemCountHudActivity. Owned by ASimState (+0xa138).
// TClass 007820b0, vtable 0071c3b8.
//-----------------------------------------------------------------------------
class AItemCountHudElement
    : public AHudElement
{
public:
	TDECLARE_CLASS( AItemCountHudElement, AHudElement );

	// What the counter counts, set by AItemCountHudActivity
	using MODE = TINT;
	enum MODE_ : MODE
	{
		MODE_MANUAL         = 1, // the count is set by AItemCountHudActivity
		MODE_INVENTORY      = 2, // items called m_strItem (or "<m_strItem>_bunch") in the player's inventory
		MODE_SQUIRT         = 4, // APlayerSimProfile::GetNumUnk150
		MODE_NUMJUNK        = 8,
		MODE_NUMTLPARTS     = 0x10,
		MODE_NUMLEAKS       = 0x20,
		MODE_NUMROCKS       = 0x40, // AHintManager
		MODE_NUMFREDDYVEG   = 0x80,
		MODE_NUMFREDDYFRUIT = 0x100,
	};

	// State saved while the sim isn't on the stack (+0x10 of the singleton at DAT_00783e64)
	struct State
	{
		TCHAR szIcon[ 32 ]; // 0x00
		TCHAR szItem[ 32 ]; // 0x20
		TBOOL bSaved;       // 0x40
		TBOOL bVisible;     // 0x41
		TINT  iMaxCount;    // 0x44
		TINT  iCount;       // 0x48
		MODE  eMode;        // 0x4c
	};

public:
	AItemCountHudElement();
	~AItemCountHudElement();

	//-----------------------------------------------------------------------------
	// AHudElement
	//-----------------------------------------------------------------------------
	virtual TBOOL OnAttach() override;
	virtual void  OnDetach() override;
	virtual void  Show() override;
	virtual void  Hide() override;
	virtual void  Update( TFLOAT a_fDeltaTime ) override;

	// Shows "a_iCount/a_iMaxCount", a_iCount is clamped to a_iMaxCount
	void SetCountAndMax( TINT a_iCount, TINT a_iMaxCount );

	// Shows "a_iCount", or "a_iCount/m_iMaxCount" when a_bWithMax is set
	void SetCount( TINT a_iCount, TBOOL a_bWithMax );

	// Adds a_iCount to the count
	void AddCount( TINT a_iCount );

	// $Barnyard: FUNCTION 004548f0
	void SetMaxCount( TINT a_iMaxCount ) { m_iMaxCount = a_iMaxCount; }
	// $Barnyard: FUNCTION 00454900
	void SetMode( MODE a_eMode ) { m_eMode = a_eMode; }

	// Sets the icon from the texture section called a_szIcon ("Place_Holder" if there's no such section)
	void SetIcon( const TCHAR* a_szIcon );
	void SetIcon( Toshi::TPString8 a_strIcon );

	// Sets the name of the item counted in MODE_INVENTORY
	void SetItemName( Toshi::TPString8 a_strItem );

	// Sets whether the counter is shown when AHudManager shows the element
	// $Barnyard: FUNCTION 00454950
	void SetVisible( TBOOL a_bVisible ) { m_bVisible = a_bVisible; }

	// Same as SetVisible but also shows or hides the counter immediately
	void SetVisibleForce( TBOOL a_bVisible );

	void SaveState( State* a_pState );
	void LoadState( State* a_pState );

public:
	Toshi::T2GUIRectangle m_oBacking;       // 0x14, "Score_Wood_Thin"
	Toshi::T2GUIRectangle m_oIcon;          // 0x8c
	AGUI2TextBox          m_oText;          // 0x104
	TWCHAR                m_wszCount[ 32 ]; // 0x17c
	TCHAR                 m_szIcon[ 32 ];   // 0x1bc
	TBOOL                 m_bVisible;       // 0x1dc
	TINT                  m_iCount;         // 0x1e0
	TINT                  m_iMaxCount;      // 0x1e4, <= 0 when there's no max
	MODE                  m_eMode;          // 0x1e8
	Toshi::TPString8      m_strItem;        // 0x1ec
};

TSTATICASSERT( sizeof( AItemCountHudElement::State ) == 0x50 );
TSTATICASSERT( sizeof( AItemCountHudElement ) == 0x1f0 );
