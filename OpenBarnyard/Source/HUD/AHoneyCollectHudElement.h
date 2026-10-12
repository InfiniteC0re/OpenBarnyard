#pragma once
#include <GUI/T2GUIElement.h>
#include <GUI/T2GUIRectangle.h>
#include <Toshi/TBitArray.h>

class AHoneyCollectingGame;

namespace Toshi
{
class TTexture;
class T2GUIMaterial;
} // namespace Toshi

//-----------------------------------------------------------------------------
// The honey comb shown while the player collects honey from a hive in the
// Honey Collecting minigame (AHoneyCollectingGame). Name from the Wii build.
// vtable 0071c1c0, size 0x698, no TClass.
//
// The comb is a disc of NUM_SECTIONS sections (the bits of the hive's
// TBitArray). The player turns the stick (or sweeps the mouse cursor) around
// it: every section he points at that still has honey is emptied into the jar
// next to the comb. When the comb is empty, the hive loses one honey.
//-----------------------------------------------------------------------------
class AHoneyCollectHudElement
    : public Toshi::T2GUIElement
{
public:
	static constexpr TINT NUM_SECTIONS = 64;
	static constexpr TINT NUM_CORNERS  = 4;

	//-----------------------------------------------------------------------------
	// A white triangle of the comb (one section), "pie_section".
	// Name from the Wii build. vtable 0071c170, size 0x8c.
	//-----------------------------------------------------------------------------
	class GUI2Triangle
	    : public Toshi::T2GUIElement
	{
	public:
		GUI2Triangle();
		~GUI2Triangle();

		//-----------------------------------------------------------------------------
		// T2GUIElement
		//-----------------------------------------------------------------------------
		virtual void Render() OVERRIDE; // +0x0c

		// Whether the screen point (a_rfX, a_rfY) is inside the triangle
		virtual TBOOL IsPointInsideTriangle( const TFLOAT& a_rfX, const TFLOAT& a_rfY ); // +0x48

	public:
		Toshi::TVector2       m_aVertices[ 3 ]; // 0x54
		Toshi::TVector2       m_aUVs[ 3 ];      // 0x6c
		Toshi::T2GUIMaterial* m_pMaterial;      // 0x84
		Toshi::TTexture*      m_pTexture;       // 0x88, white 8x8
	};

public:
	AHoneyCollectHudElement();
	~AHoneyCollectHudElement();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x04
	virtual void Render() OVERRIDE;                    // +0x0c

	// Builds the comb and the jar and adds the element to the root of the GUI
	virtual TBOOL Create( AHoneyCollectingGame* a_pGame ); // +0x48

	// Removes the parts and the element from the GUI
	virtual void Destroy(); // +0x4c

	// Shows the comb of a hive: a_pBits are its sections, a_iHoney its honey out of
	// a_iMaxHoney (the jar shows how much has been collected)
	void SetHive( TINT a_iMaxHoney, TINT a_iHoney, Toshi::TBitArray* a_pBits );

	// Number of sections that still have honey
	TINT GetNumFullSections();

	// Empties the section a_iSection into the jar (if it has honey)
	void CollectSection( TINT a_iSection );

	// With a_bFull, every section gets its honey back right away. Otherwise they are
	// refilled one per frame (Tick)
	void Reset( TBOOL a_bFull );

	// Position of the comb (where the mouse cursor is held)
	void GetCentre( TFLOAT& a_rfX, TFLOAT& a_rfY );

	void SetHighlightedSection( TINT a_iSection ) { m_iHighlightedSection = a_iSection; }
	void SetPulse( TBOOL a_bPulse ) { m_bPulse = a_bPulse; }
	void SetMouseInput( TBOOL a_bMouseInput ) { m_bMouseInput = a_bMouseInput; }

	void  SetNumCollected( TINT a_iNumCollected ) { m_iNumCollected = a_iNumCollected; }
	TBOOL IsFull() const { return m_bFull; }
	TBOOL IsPulsing() const { return m_bPulse; }

	Toshi::TBitArray* GetSections() const { return m_pSections; }

private:
	Toshi::TBitArray*     m_pSections;               // 0x054, of the hive
	TINT                  m_aUnk58[ 4 ];             // 0x058
	TINT                  m_iHighlightedSection;     // 0x068, the stick points at it (-1 if none)
	TBOOL                 m_bFull;                   // 0x06c
	TINT                  m_iRefillSection;          // 0x070
	TBOOL                 m_bPulse;                  // 0x074, the corners pulse red
	TINT                  m_iNumCollected;           // 0x078, sections emptied since SetHive
	Toshi::T2GUIRectangle m_oBackground;             // 0x07c, "Hive_Background"
	Toshi::T2GUIElement   m_oComb;                   // 0x0f4
	Toshi::T2GUIRectangle m_aCorners[ NUM_CORNERS ]; // 0x148, "Honey_Corner"
	Toshi::T2GUIRectangle m_oScreen;                 // 0x328, black, covers the screen
	GUI2Triangle          m_oSection;                // 0x3a0, rendered once per empty section
	Toshi::T2GUIRectangle m_oArrow;                  // 0x42c, "Honey_Arrow"
	TINT                  m_iMouseSection;           // 0x4a4, under the mouse cursor (-1 if none)
	TBOOL                 m_bMouseInput;             // 0x4a8, the mouse button is held
	Toshi::T2GUIRectangle m_oJar;                    // 0x4ac, "Jar_Bkg"
	Toshi::T2GUIRectangle m_oJarMask;                // 0x524, "Jar_Mask"
	Toshi::T2GUIRectangle m_oJarHoney;               // 0x59c, "Honey_Rectangle"
	Toshi::T2GUIRectangle m_oJarGlint;               // 0x614, "Jar_Glint"
	TFLOAT                m_fJarLevel;               // 0x68c
	TINT                  m_iHoney;                  // 0x690
	AHoneyCollectingGame* m_pGame;                   // 0x694
};

TSTATICASSERT( sizeof( AHoneyCollectHudElement::GUI2Triangle ) == 0x8c );
TSTATICASSERT( sizeof( AHoneyCollectHudElement ) == 0x698 );
