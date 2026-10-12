#pragma once
#include "ABYardMenuState.h"

//-----------------------------------------------------------------------------
// Carousel of the Bonuses menu (TClass 0077df1c, vtable 007163b0) with nine
// unlockable items: the concept art categories (AGalleryState) or the movies.
// The items are unlocked by the bits 13-21 (art) and 22-30 (movies) of the
// unlocked flags of APlayerProgress.
// NOTE: works like AFrontEndMiniGameState2.
//-----------------------------------------------------------------------------
class AFrontEndArtGalleryState : public ABYardMenuState
{
public:
	TDECLARE_CLASS( AFrontEndArtGalleryState, ABYardMenuState );

	static constexpr TINT NUM_ITEMS = 9;

	using MODE = TINT;
	enum MODE_ : MODE
	{
		MODE_ART,
		MODE_MOVIES,
	};

	enum SCROLLSTATE
	{
		SCROLLSTATE_READY,
		SCROLLSTATE_INIT_NEXT, // the items move left, the right one becomes the current one
		SCROLLSTATE_NEXT,
		SCROLLSTATE_INIT_PREV, // the items move right, the left one becomes the current one
		SCROLLSTATE_PREV,
	};

	enum ARROW
	{
		ARROW_LEFT,
		ARROW_RIGHT,
		ARROW_NUMOF
	};

public:
	AFrontEndArtGalleryState( MODE a_eMode );
	~AFrontEndArtGalleryState();

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
	virtual void OnMenuClose() OVERRIDE;

private:
	TBOOL IsCurrentUnlocked() const;

	// Places the current item and the ones next to it without animation
	void ResetItems();

	// Shows the name and the description of the current item
	void UpdateTexts();

	// Moves the items towards their places for a step of the scrolling animation (NOTE: inlined twice in the original)
	void UpdateScrolling( TFLOAT a_fDeltaTime, TFLOAT a_fLerpRate, TFLOAT a_fOldItemX );

private:
	// 0x007725b0
	inline static const TCHAR* ms_aArtIcons[ NUM_ITEMS ] = {
		"Bonus_Gallery_Cows", "Bonus_Gallery_Horses", "Bonus_Pigs", "Bonus_World", "Bonus_Story", "Bonus_Minigame", "Bonus_Original", "Bonus_Barnyard", "Bonus_Clothes"
	};

	// 0x007725d4
	inline static const TCHAR* ms_aMovieIcons[ NUM_ITEMS ] = {
		"TV_Igg", "TV_Bud", "TV_Evil_Rac", "TV_Root", "TV_Goat", "TV_Golf", "TV_Human", "TV_Igg_Talking", "TV_Otis"
	};

	// 0x007725f8
	inline static const TCHAR* ms_aMovies[ NUM_ITEMS ] = {
		"IggSquirt", "BudHosing", "RacEvil", "RootMarch", "GoatOnFence", "Golf", "BuckingHuman", "IggTalk", "GetOnBike"
	};

	// 0x007161cc, 0x007161f0
	inline static constexpr TINT ms_aArtNames[ NUM_ITEMS ]        = { 104, 105, 106, 107, 108, 109, 110, 111, 85 };
	inline static constexpr TINT ms_aArtDescriptions[ NUM_ITEMS ] = { 112, 113, 114, 115, 116, 117, 118, 119, 84 };

	// 0x00716214, 0x00716238
	inline static constexpr TINT ms_aMovieNames[ NUM_ITEMS ]        = { 86, 87, 88, 89, 90, 91, 92, 93, 94 };
	inline static constexpr TINT ms_aMovieDescriptions[ NUM_ITEMS ] = { 75, 76, 77, 78, 79, 80, 81, 82, 83 };

	inline static TFLOAT ms_fXOffset       = 100.0f; // 0x0077261c
	inline static TFLOAT ms_fSelectedScale = 1.1f;   // 0x00772620
	inline static TFLOAT ms_fMinDiff       = 0.1f;   // 0x00772624
	inline static TFLOAT ms_fPrevLerpRate  = 5.0f;   // 0x00772628
	inline static TFLOAT ms_fNextLerpRate  = 5.0f;   // 0x0077262c

private:
	MODE                   m_eMode;                  // 0x7d8
	AGUI2MenuItem          m_oMenuItem;              // 0x7dc
	AGUI2MenuItem          m_oUnusedMenuItem;        // 0x840, never used
	Toshi::T2GUIRectangle* m_pItems;                 // 0x8a4
	TINT                   m_iNumItems;              // 0x8a8
	TINT                   m_iCurrent;               // 0x8ac
	TINT                   m_iPrev;                  // 0x8b0
	TINT                   m_iNext;                  // 0x8b4
	TINT                   m_iOld;                   // 0x8b8, the item that fades out
	SCROLLSTATE            m_eScrollState;           // 0x8bc
	Toshi::T2GUIRectangle  m_aArrows[ ARROW_NUMOF ]; // 0x8c0
	AGUI2TextBox           m_oName;                  // 0x9b0
	AGUI2TextBox           m_oDescription;           // 0xa28
	TFLOAT                 m_fAnalogTime;            // 0xaa8
	TBOOL                  m_bMouseMoved;            // 0xaac
	TBOOL                  m_bMouseOverCurrent;      // 0xaad
	TBOOL                  m_bMouseOverLeftArrow;    // 0xaae
	TBOOL                  m_bMouseOverRightArrow;   // 0xaaf
};
