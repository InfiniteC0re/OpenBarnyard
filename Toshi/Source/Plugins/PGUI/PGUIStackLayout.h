#pragma once
#include "PGUILayoutManager.h"

//-----------------------------------------------------------------------------
// Purpose: arranges the managed children in a row ("flow = horizontal") or a
// column ("flow = vertical"), justified in the other direction. The free
// children share the remaining space (fill options) or get the same size.
//-----------------------------------------------------------------------------
class PGUIStackLayout : public PGUILayoutManager
{
public:
	TDECLARE_CLASS( PGUIStackLayout, PGUILayoutManager );

	enum FlowDirection : TINT
	{
		FlowDirection_Horizontal = 0,
		FlowDirection_Vertical   = 1,
		FlowDirection_NUMOF      = 2
	};

public:
	PGUIStackLayout();
	PGUIStackLayout( const PGUIStackLayout& a_rcOther );
	~PGUIStackLayout();

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
	virtual PGUIStackLayout* Clone() const override;
	virtual void             DoPack( Toshi::TGUIWidget* a_pWidget ) override;
	virtual void             DoLayout( Toshi::TGUIWidget* a_pWidget, TINT& a_rWidth, TINT& a_rHeight ) override;

	//-----------------------------------------------------------------------------
	// PGUIStackLayout
	//-----------------------------------------------------------------------------

	// +0x40: The sizes of the managed children: the largest size, the total size of the fixed and preferred ones and of
	// the free ones in the flow direction (a_bSameSize: all of them, from the largest), the number of free, managed and
	// not laid out ones
	virtual void CalculateLayoutSizes( Toshi::TGUIWidget* a_pWidget, TBOOL a_bSameSize, TINT& a_rMaxWidth, TINT& a_rMaxHeight, TINT& a_rFixedWidth, TINT& a_rFixedHeight, TINT& a_rFreeWidth, TINT& a_rFreeHeight, TINT& a_rNumFree, TINT& a_rNumManaged, TINT& a_rNumInvalid );

	void           Set_flow_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_flow_property() const;
	TBOOL          Val_flow_property( const PPropertyValue& a_rcValue );
	void           Set_justifyhoriz_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_justifyhoriz_property() const;
	TBOOL          Val_justifyhoriz_property( const PPropertyValue& a_rcValue );
	void           Set_justifyvert_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_justifyvert_property() const;
	TBOOL          Val_justifyvert_property( const PPropertyValue& a_rcValue );

	// $JPOG: FUNCTION PGUICore.dll+00011e40
	FlowDirection GetFlowDirection() const { return m_eFlowDirection; }

	// $JPOG: FUNCTION PGUICore.dll+00011e60
	Alignment GetJustifyHoriz() const { return m_eJustifyHoriz; }

	// $JPOG: FUNCTION PGUICore.dll+00011e50
	Alignment GetJustifyVert() const { return m_eJustifyVert; }

	// $JPOG: FUNCTION PGUICore.dll+00011e70
	void SetFlowDirection( FlowDirection a_eFlowDirection ) { m_eFlowDirection = a_eFlowDirection; }

	// $JPOG: FUNCTION PGUICore.dll+00011e80
	void SetJustifyHoriz( Alignment a_eAlignment ) { m_eJustifyHoriz = a_eAlignment; }

	// $JPOG: FUNCTION PGUICore.dll+00011e90
	void SetJustifyVert( Alignment a_eAlignment ) { m_eJustifyVert = a_eAlignment; }

public:
	static PropertySet*           ms_oPropertySet;
	static PropertySet*           ms_oPropertySetOrdered;
	static Toshi::TGUILayoutInfo* ms_pLayoutInfo;

	static const TCHAR* const ms_csaFlowNameStr[ FlowDirection_NUMOF ];
	static const TCHAR* const ms_csaJustifyHorizNameStr[ 3 ];
	static const TCHAR* const ms_csaJustifyVertNameStr[ 3 ];

private:
	FlowDirection m_eFlowDirection; // 0x14
	Alignment     m_eJustifyHoriz;  // 0x18
	Alignment     m_eJustifyVert;   // 0x1C
};
