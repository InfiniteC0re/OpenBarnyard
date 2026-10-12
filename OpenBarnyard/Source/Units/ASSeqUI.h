#pragma once
#include "GUI/AGUI2Menu.h"
#include "GUI/AGUI2MenuGrid.h"
#include "GUI/AGUI2MenuGridTextItem.h"
#include "GUI/AGUI2Button.h"
#include "GUI/AGUI2Dialog.h"
#include "GUI/AGUI2TextBox.h"
#include "Input/AInputMapManager.h"

#include <Toshi/TObject.h>
#include <Toshi/TString16.h>
#include <GUI/T2GUIRectangle.h>

// The player creation screens that the scripted sequences open with the "UI" command
// (gender, skin and name of the player). Owned by ASSeq (+0x60).
//
// TClass 0x007b4cb8 (size 0x5710), vtable 0x00739390.
class ASSeqUI : public Toshi::TObject
{
public:
	TDECLARE_CLASS( ASSeqUI, Toshi::TObject );

	friend class ASSeqState;

	static constexpr TINT NUM_GENDERS = 2;
	static constexpr TINT NUM_SKINS   = 7;
	static constexpr TINT NUM_LETTERS = 28; // 26 letters, delete and done

	using PAGE = TINT;
	enum PAGE_ : PAGE
	{
		PAGE_GENDER,
		PAGE_SKIN,
		PAGE_NAME,
	};

public:
	ASSeqUI( TBOOL a_bSkipName = TFALSE, TBOOL a_bNoSound = TFALSE );
	~ASSeqUI();

	// Opens the gender page
	TBOOL Show( const TCHAR* a_szUnused );

	// Hides every element
	void Reset();

	void Update( TFLOAT a_fDeltaTime );

	TBOOL ProcessInputCommand( AInputCommand a_eCommand, const Toshi::TInputInterface::InputEvent* a_pEvent );

	// Mouse clicks on the current page
	TBOOL ProcessInputEvent( const Toshi::TInputInterface::InputEvent* a_pEvent );

	TBOOL IsActive() const { return m_bActive; }
	PAGE  GetPage() const { return m_ePage; }

private:
	// Shows the elements of a_ePage
	void SetPage( PAGE a_ePage );

	// Moves the skin selection by a_iDirection, skipping skins 3 and 4
	void CycleSkin( TINT a_iDirection );

	// Applies the gender (0: male, 1: female) to the player character
	void SetGender( TINT a_iGender );

	void OnLetterActivated( AGUI2MenuGridItem* a_pItem );
	void OnMenuItemActivated( AGUI2MenuItem* a_pItem );

	static void OnLetterActivatedCallback( void* a_pUserData, AGUI2MenuGridItem* a_pItem );
	static void OnMenuItemActivatedCallback( void* a_pUserData, AGUI2MenuItem* a_pItem );
	static void OnMenuItemFocusedCallback( void* a_pUserData, AGUI2MenuItem* a_pOldFocus, AGUI2MenuItem* a_pNewFocus );

	// Shows m_strName in the name box (NOTE: inlined in the original)
	void UpdateNameText();

private:
	inline static const TWCHAR* ms_awszSkinNames[ NUM_SKINS ]; // 0x007b4c20
	inline static const TWCHAR* ms_awszLetters[ NUM_LETTERS ]; // 0x007b4c40
	inline static TUINT16       ms_auiUnk[ 4 ];                // 0x007b4cb0, set by the constructor, never read

private:
	AGUI2Menu             m_oGenderMenu;                   // 0x0004
	AGUI2Menu             m_oSkinMenu;                     // 0x00a8
	AGUI2MenuGrid         m_oNameGrid;                     // 0x014c
	AGUI2TextBox          m_oTitle;                        // 0x0210
	Toshi::T2GUIRectangle m_oTitleBackground;              // 0x0288
	AGUI2Button           m_aGenderButtons[ NUM_GENDERS ]; // 0x0300
	AGUI2Button           m_aSkinButtons[ NUM_SKINS ];     // 0x0708
	AGUI2MenuGridTextItem m_aLetters[ NUM_LETTERS ];       // 0x1524
	AGUI2TextBox          m_oName;                         // 0x4f54
	Toshi::T2GUIRectangle m_oNameBackground;               // 0x4fcc
	TBOOL                 m_bActive;                       // 0x5044
	PAGE                  m_ePage;                         // 0x5048
	Toshi::TString16      m_strName;                       // 0x504c
	TBOOL                 m_bNameEdited;                   // 0x5058
	AGUI2Dialog           m_oDialog;                       // 0x505c
	TFLOAT                m_fDialogWidth;                  // 0x5514
	TFLOAT                m_fDialogHeight;                 // 0x5518
	TBOOL                 m_bDefaultNameSet;               // 0x551c
	TBOOL                 m_bSkipName;                     // 0x551d, the gender and skin pages only
	TBOOL                 m_bNoSound;                      // 0x551e, no voice-over and no HUD changes
	Toshi::T2GUIRectangle m_oSkinPicture;                  // 0x5520
	AGUI2TextBox          m_oSkinName;                     // 0x5598
	Toshi::T2GUIRectangle m_aSkinArrows[ 2 ];              // 0x5610
	TINT                  m_iSkin;                         // 0x5700
	TFLOAT                m_fTime;                         // 0x5704
	TFLOAT                m_fVOTime;                       // 0x5708
	TBOOL                 m_bUnk2;                         // 0x570c
	TBOOL                 m_bSkinPictureHovered;           // 0x570d
	TBOOL                 m_bLeftArrowHovered;             // 0x570e
	TBOOL                 m_bRightArrowHovered;            // 0x570f
};


TSTATICASSERT( sizeof( ASSeqUI ) == 0x5710 );
