#pragma once
#include <GUI/T2GUIElement.h>
#include <GUI/T2GUIFont.h>
#include <Math/TVector4.h>
#include <Toshi/TNodeList.h>

class AUnit;

//-----------------------------------------------------------------------------
// Texts floating over 3D positions (score popups rising from where they were
// scored) and over units. Floats are sorted by camera distance and drawn
// with a single font. Plain T2GUIElement (no TClass), vtable 00739e20.
// Name from the Wii build (AGUI2FloatHudElement), JPOG has AFloatHudElement.
//-----------------------------------------------------------------------------
class AGUI2FloatHudElement
    : public Toshi::T2GUIElement
{
public:
	static constexpr TINT MAX_TIMED_FLOATS = 64;
	static constexpr TINT MAX_UNIT_FLOATS  = 16;

	// Screen state shared by both kinds of floats
	struct Float
	{
		TUINT32       uiColour;    // 0x00, alpha is recalculated every frame
		const TWCHAR* wszText;     // 0x04
		TFLOAT        fScreenX;    // 0x08
		TFLOAT        fScreenY;    // 0x0c
		TFLOAT        fDistanceSq; // 0x10, from the camera, sort key
		TUINT32       uiUnk;       // 0x14
	};

	// Text rising from a world position for a while
	struct TimedFloat
	    : public Float
	    , public Toshi::TNodeList<TimedFloat>::TNode // 0x18
	{
		Toshi::TVector4 vecPosition; // 0x24
		TFLOAT          fTimeLeft;   // 0x34
		TFLOAT          fRiseSpeed;  // 0x38
		TFLOAT          fFadeTime;   // 0x3c, fades out during the last fFadeTime seconds
	};

	// Text following a unit
	struct UnitFloat
	    : public Float
	    , public Toshi::TNodeList<UnitFloat>::TNode // 0x18
	{
		AUnit* pUnit;             // 0x24
		TFLOAT fHeight;           // 0x28, offset from the unit's position
		TFLOAT fSmoothedY;        // 0x2c
		TBOOL  bFadeWithDistance; // 0x30
	};

	// Floats on screen, nearest first
	template <class T, TINT MAX_SIZE>
	struct SortedArray
	{
		TINT iCount;
		T*   apItems[ MAX_SIZE ];

		void Clear()
		{
			for ( TINT i = 0; i < iCount; i++ ) apItems[ i ] = TNULL;
			iCount = 0;
		}

		void Insert( T* a_pItem );
	};

public:
	AGUI2FloatHudElement();
	~AGUI2FloatHudElement();

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Render() override;

	// Attaches the element to a_pParent (the GUI root if TNULL) and allocates the floats.
	void Create( Toshi::T2GUIElement* a_pParent, const TCHAR* a_szFontName );

	// Detaches the element and frees the floats.
	void Destroy();

	// Removes all the floats.
	void Reset();

	// Shows a_wszText at a_rcPosition for a_fDuration seconds, rising by a_fRiseSpeed per second
	// and fading out during its last a_fFadeTime seconds. Returns TFALSE if all the floats are in use.
	TBOOL AddTimedFloat( const TWCHAR* a_wszText, TUINT32 a_uiColour, const Toshi::TVector4& a_rcPosition, TFLOAT a_fDuration, TFLOAT a_fRiseSpeed, TFLOAT a_fFadeTime );

	TBOOL IsCreated() const { return m_bCreated; }

private:
	void UpdateTimedFloats();
	void UpdateUnitFloats();

	void DrawFloat( const Float* a_pFloat );

public:
	TBOOL                                     m_bCreated;          // 0x54
	Toshi::T2GUIFont*                         m_pFont;             // 0x58
	Toshi::TNodeList<TimedFloat>              m_llTimedFloats;     // 0x5c
	Toshi::TNodeList<TimedFloat>              m_llFreeTimedFloats; // 0x6c
	SortedArray<TimedFloat, MAX_TIMED_FLOATS> m_oVisibleTimed;     // 0x7c
	TINT                                      m_iNumVisibleTimed;  // 0x180
	Toshi::TNodeList<UnitFloat>               m_llUnitFloats;      // 0x184
	Toshi::TNodeList<UnitFloat>               m_llFreeUnitFloats;  // 0x194
	SortedArray<UnitFloat, MAX_UNIT_FLOATS>   m_oVisibleUnit;      // 0x1a4
	TINT                                      m_iNumVisibleUnit;   // 0x1e8
	TBOOL                                     m_bShowUnitFloats;   // 0x1ec
	TBOOL                                     m_bOcclusionTest;    // 0x1ed, hides unit floats behind collision
	Toshi::T2GUIElement*                      m_pAttachedTo;       // 0x1f0
};

TSTATICASSERT( sizeof( AGUI2FloatHudElement::TimedFloat ) == 0x40 );
TSTATICASSERT( sizeof( AGUI2FloatHudElement::UnitFloat ) == 0x34 );
TSTATICASSERT( sizeof( AGUI2FloatHudElement ) == 0x1f4 );
