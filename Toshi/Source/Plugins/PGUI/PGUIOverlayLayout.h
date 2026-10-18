#pragma once
#include <TGUI/TGUILayoutManager.h>

//-----------------------------------------------------------------------------
// Purpose: places the managed children on top of each other, each justified
// in the interior of the widget. The free children get the size of the
// largest child (same size options), the interior (fill options) or keep
// theirs (limited to the interior).
//-----------------------------------------------------------------------------
class PGUIOverlayLayout : public Toshi::TGUILayoutManager
{
public:
	TDECLARE_CLASS( PGUIOverlayLayout, Toshi::TGUILayoutManager );

public:
	PGUIOverlayLayout();
	PGUIOverlayLayout( const PGUIOverlayLayout& a_rcOther );
	~PGUIOverlayLayout();

	//-----------------------------------------------------------------------------
	// Toshi::TGUIDeserialisable
	//-----------------------------------------------------------------------------
	virtual TBOOL        Deserialise( const PProperties* a_pProperties ) override;
	virtual TBOOL        Serialise( PProperties* a_pProperties ) override;
	virtual PropertySet* GetPropertyInfoSet() const override;
	virtual PropertySet* GetPropertyInfoOrderedSet() const override;

	//-----------------------------------------------------------------------------
	// Toshi::TGUILayoutManager
	//-----------------------------------------------------------------------------
	virtual PGUIOverlayLayout* Clone() const override;
	virtual void               DoPack( Toshi::TGUIWidget* a_pWidget ) override;
	virtual void               DoLayout( Toshi::TGUIWidget* a_pWidget, TINT& a_rWidth, TINT& a_rHeight ) override;

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
	void           Set_justifyhoriz_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_justifyhoriz_property() const;
	TBOOL          Val_justifyhoriz_property( const PPropertyValue& a_rcValue );
	void           Set_justifyvert_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_justifyvert_property() const;
	TBOOL          Val_justifyvert_property( const PPropertyValue& a_rcValue );

	// $JPOG: FUNCTION PGUICore.dll+00025110
	TBOOL GetForceSameWidth() const { return m_bSameWidth; }

	// $JPOG: FUNCTION PGUICore.dll+00025100
	TBOOL GetForceSameHeight() const { return m_bSameHeight; }

	// $JPOG: FUNCTION PGUICore.dll+00025150
	void SetForceSameWidth( TBOOL a_bSameWidth ) { m_bSameWidth = a_bSameWidth; }

	// $JPOG: FUNCTION PGUICore.dll+00025140
	void SetForceSameHeight( TBOOL a_bSameHeight ) { m_bSameHeight = a_bSameHeight; }

	// $JPOG: FUNCTION PGUICore.dll+000250f0
	Alignment GetJustifyHoriz() const { return m_eJustifyHoriz; }

	// $JPOG: FUNCTION PGUICore.dll+000250e0
	Alignment GetJustifyVert() const { return m_eJustifyVert; }

	// $JPOG: FUNCTION PGUICore.dll+00025120
	void SetJustifyHoriz( Alignment a_eAlignment ) { m_eJustifyHoriz = a_eAlignment; }

	// $JPOG: FUNCTION PGUICore.dll+00025130
	void SetJustifyVert( Alignment a_eAlignment ) { m_eJustifyVert = a_eAlignment; }

public:
	static PropertySet*           ms_oPropertySet;
	static PropertySet*           ms_oPropertySetOrdered;
	static Toshi::TGUILayoutInfo* ms_pLayoutInfo;

	static const TCHAR* const ms_csaJustifyHorizNameStr[ 3 ];
	static const TCHAR* const ms_csaJustifyVertNameStr[ 3 ];

private:
	TBOOL     m_bSameWidth;      // 0xC
	TBOOL     m_bSameHeight;     // 0xD
	TBOOL     m_bFillHorizontal; // 0xE
	TBOOL     m_bFillVertical;   // 0xF
	Alignment m_eJustifyHoriz;   // 0x10
	Alignment m_eJustifyVert;    // 0x14
};
