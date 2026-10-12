#pragma once
#include "AHudElement.h"
#include "GUI/AGUI2TextBox.h"

#include <GUI/T2GUIRectangle.h>
#include <GUI/T2GUITextureSection.h>

//-----------------------------------------------------------------------------
// "Ready / Set / Go" countdown shown before a minigame round starts.
// AMiniGameState stays in its pre-start phase (UpdatePreStart) until this
// element force-hides itself (AHudElement::IsForceHidden()).
// Not registered in the TClass system: its GetClass slot is AHudElement's.
// vtable 0071c7dc.
//-----------------------------------------------------------------------------
class AStartMiniGameHudElement
    : public AHudElement
{
public:
	enum STAGE
	{
		STAGE_READY,
		STAGE_SET,
		STAGE_GO,
		STAGE_NUMOF
	};

	using TYPE = TINT;
	enum TYPE_ : TYPE
	{
		TYPE_COUNTDOWN        = 0, // "Ready", "Set", "Go" with beeps; "Ready!"
		TYPE_READY_SILENT     = 1, // only "Ready", no beeps; "Ready!"
		TYPE_PLAYER_COUNTDOWN = 2, // "Ready", "Set", "Go" with beeps; "Player N Ready!"
		TYPE_PLAYER_READY     = 3, // only "Ready", no beeps; "Player N Get Ready!"
		TYPE_GET_READY        = 4, // only "Ready"; "Get Ready!"
		TYPE_GO               = 5, // only "Go"; "Go!"
	};

	// Seconds each stage image stays before it starts to fade.
	static constexpr TFLOAT STAGE_TIME = 1.0f; // DAT_007736fc
	// Pause after the fade before the next stage starts.
	static constexpr TFLOAT STAGE_GAP = 0.25f; // DAT_00773700
	// Vertical offset of the images from the screen centre.
	static constexpr TFLOAT IMAGE_OFFSET_Y = -100.0f; // DAT_00773704

public:
	AStartMiniGameHudElement();
	~AStartMiniGameHudElement();

	//-----------------------------------------------------------------------------
	// AHudElement
	//-----------------------------------------------------------------------------
	virtual TBOOL OnAttach() override;
	virtual void  OnDetach() override;
	virtual void  Show() override;
	virtual void  Hide() override;
	virtual void  Update( TFLOAT a_fDeltaTime ) override;

	// Resets the countdown and picks the text for the given player.
	void Setup( TINT a_iNumPlayers, TINT a_iCurrentPlayer );

	void SetType( TYPE a_eType ) { m_eType = a_eType; }
	TYPE GetType() const { return m_eType; }

	TBOOL IsFinished() const { return m_bFinished; }

public:
	TBOOL                       m_bFinished;              // 0x14, set together with m_bForceHide once the last image faded out
	TINT                        m_iImageIndex;            // 0x18, STAGE
	TYPE                        m_eType;                  // 0x1c, set by AMiniGameState::OnInsertion from AMiniGame +0xfc
	Toshi::T2GUIRectangle       m_oImage;                 // 0x20, current stage image
	Toshi::T2GUIRectangle       m_oPrevImage;             // 0x98, previous stage image, fades out while scaling up
	AGUI2TextBox                m_oText;                  // 0x110
	TFLOAT                      m_fTimer;                 // 0x188, time spent in the current stage
	TWCHAR                      m_wszText[ 32 ];          // 0x18c, formatted "player N" text
	Toshi::T2GUITextureSection* m_aImages[ STAGE_NUMOF ]; // 0x1cc, "Ready", "Set", "Go"
};

TSTATICASSERT( sizeof( AStartMiniGameHudElement ) == 0x1d8 );
