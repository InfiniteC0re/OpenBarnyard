#pragma once
#include <TGUI/TGUILayoutManager.h>

//-----------------------------------------------------------------------------
// Purpose: base of the PGUI layout managers that arrange the children in a
// flow or a grid: the padding between the children, the "same width/height"
// options (every child gets the size of the largest) and the fill options
// (the children whose size mode is free share the remaining space).
//
// NOTE: JPOG declares it in the Toshi namespace. Its layout managers keep their
// layout info (JPOG +0x8), ours don't: the TGUILayoutManager of Nicktoons
// doesn't have that member.
//-----------------------------------------------------------------------------
class PGUILayoutManager : public Toshi::TGUILayoutManager
{
public:
	TDECLARE_CLASS( PGUILayoutManager, Toshi::TGUILayoutManager );

public:
	PGUILayoutManager();
	PGUILayoutManager( const PGUILayoutManager& a_rcOther );
	~PGUILayoutManager();

	//-----------------------------------------------------------------------------
	// Toshi::TGUIDeserialisable
	//-----------------------------------------------------------------------------
	virtual PropertySet* GetPropertyInfoSet() const override;
	virtual PropertySet* GetPropertyInfoOrderedSet() const override;

	void           Set_padx_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_padx_property() const;
	TBOOL          Val_padx_property( const PPropertyValue& a_rcValue );
	void           Set_pady_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_pady_property() const;
	TBOOL          Val_pady_property( const PPropertyValue& a_rcValue );
	void           Set_samewidth_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_samewidth_property() const;
	TBOOL          Val_samewidth_property( const PPropertyValue& a_rcValue );
	void           Set_sameheight_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_sameheight_property() const;
	TBOOL          Val_sameheight_property( const PPropertyValue& a_rcValue );
	void           Set_fillhorizontal_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_fillhorizontal_property() const;
	TBOOL          Val_fillhorizontal_property( const PPropertyValue& a_rcValue );
	void           Set_fillvertical_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_fillvertical_property() const;
	TBOOL          Val_fillvertical_property( const PPropertyValue& a_rcValue );

	// $JPOG: FUNCTION PGUICore.dll+00011dd0
	TINT GetPadX() const { return m_iPadX; }

	// $JPOG: FUNCTION PGUICore.dll+00011dc0
	TINT GetPadY() const { return m_iPadY; }

	// $JPOG: FUNCTION PGUICore.dll+00011e30
	void SetPadX( TINT a_iPadX ) { m_iPadX = TINT16( a_iPadX ); }

	// $JPOG: FUNCTION PGUICore.dll+00011e20
	void SetPadY( TINT a_iPadY ) { m_iPadY = TINT16( a_iPadY ); }

	// $JPOG: FUNCTION PGUICore.dll+00011e00
	void SetPad( TINT a_iPadX, TINT a_iPadY )
	{
		m_iPadX = TINT16( a_iPadX );
		m_iPadY = TINT16( a_iPadY );
	}

	// $JPOG: FUNCTION PGUICore.dll+00011db0
	TBOOL GetForceSameWidth() const { return m_bSameWidth; }

	// $JPOG: FUNCTION PGUICore.dll+00011da0
	TBOOL GetForceSameHeight() const { return m_bSameHeight; }

	// $JPOG: FUNCTION PGUICore.dll+00011df0
	void SetForceSameWidth( TBOOL a_bSameWidth ) { m_bSameWidth = a_bSameWidth; }

	// $JPOG: FUNCTION PGUICore.dll+00011de0
	void SetForceSameHeight( TBOOL a_bSameHeight ) { m_bSameHeight = a_bSameHeight; }

public:
	static PropertySet*           ms_oPropertySet;
	static PropertySet*           ms_oPropertySetOrdered;

protected:
	TINT16 m_iPadX;           // 0xC
	TINT16 m_iPadY;           // 0xE
	TBOOL  m_bSameWidth;      // 0x10
	TBOOL  m_bSameHeight;     // 0x11
	TBOOL  m_bFillHorizontal; // 0x12
	TBOOL  m_bFillVertical;   // 0x13
};
