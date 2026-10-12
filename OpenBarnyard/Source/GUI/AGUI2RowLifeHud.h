#pragma once
#include <GUI/T2GUIElement.h>
#include <GUI/T2GUIRectangle.h>
#include <GUI/T2GUITextureSection.h>

//-----------------------------------------------------------------------------
// Lives shown as a row of glasses on a wooden plank (BarnTender). A lost life
// plays a glass breaking animation and leaves a broken glass behind.
// Not a T2GUIElement itself, the plank (m_oBacking) is attached to a parent.
// vtable 0073a798: +0x00 dtor 00649f40, +0x04 Tick 0064a070.
// Name from the Wii build (AGUI2RowLifeHud), size 0x7c0.
//-----------------------------------------------------------------------------
class AGUI2RowLifeHud
{
public:
	static constexpr TINT   MAX_LIVES          = 5;
	static constexpr TINT   NUM_BREAK_FRAMES   = 4;
	static constexpr TFLOAT BREAK_FRAMES_SPEED = 10.0f; // 00779908, frames per second

public:
	AGUI2RowLifeHud();
	virtual ~AGUI2RowLifeHud();

	// Updates the breaking animations
	virtual void Tick( TFLOAT a_fDeltaTime );

	// Creates the plank with a_iMaxLives (up to MAX_LIVES) glasses using the a_szGlass section
	// scaled by a_fScale, and attaches it to a_pParent, a_fOffsetY down from the attachment point.
	// a_bDontScaleBroken keeps the broken glasses at the size of their section.
	void Create( Toshi::T2GUIElement* a_pParent, Toshi::T2GUIATTACHMENT a_eAnchor, Toshi::T2GUIATTACHMENT a_ePivot, TINT a_iMaxLives, const TCHAR* a_szGlass, TFLOAT a_fScale, TFLOAT a_fOffsetY, TBOOL a_bDontScaleBroken );

	void SetVisible( TBOOL a_bVisible );

	// Sets the lives without animating. The lost lives show broken glasses unless a_bHideLost is set.
	void SetLives( TINT a_iLives, TBOOL a_bHideLost );

	// Adds (positive, up to the max lives) or removes (negative, breaking the glasses) lives
	void AddLives( TINT a_iCount );

	TINT GetLives() const { return m_iLives; }

private:
	void UpdateBreaking( TFLOAT a_fDeltaTime, TINT a_iIndex );

public:
	TBOOL                        m_bCreated;                      // 0x04
	TINT                         m_iLives;                        // 0x08
	TUINT8                       PADDING1[ 16 ];                  // 0x0c
	Toshi::T2GUIRectangle        m_oBacking;                      // 0x1c, "Score_Wood"
	Toshi::T2GUIRectangle        m_aGlasses[ MAX_LIVES ];         // 0x94
	Toshi::T2GUIRectangle        m_aBrokenGlasses[ MAX_LIVES ];   // 0x2ec
	Toshi::T2GUIRectangle        m_aBreakingGlasses[ MAX_LIVES ]; // 0x544, the breaking animations
	TUINT8                       m_uiBreakingMask;                // 0x79c, bit per glass that is breaking
	TFLOAT                       m_afBreakTime[ MAX_LIVES ];      // 0x7a0, in frames
	Toshi::T2GUITextureSection** m_ppBreakFrames;                 // 0x7b4, [ NUM_BREAK_FRAMES ], TMalloc'd by Create
	TFLOAT                       m_fScale;                        // 0x7b8
	TINT                         m_iMaxLives;                     // 0x7bc
};

TSTATICASSERT( sizeof( AGUI2RowLifeHud ) == 0x7c0 );
