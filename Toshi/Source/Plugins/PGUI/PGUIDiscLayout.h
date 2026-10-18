#pragma once
#include <TGUI/TGUILayoutManager.h>
#include <TGUI/TGUIRectangle.h>

//-----------------------------------------------------------------------------
// Purpose: places the managed children around an ellipse (radiusx, radiusy),
// centred on their slots: "slotcount" slots (or one per child) from the
// offset angle, clockwise or anticlockwise. The size is the bounding box of
// the children (and of the empty slots with the full width/height options).
//-----------------------------------------------------------------------------
class PGUIDiscLayout : public Toshi::TGUILayoutManager
{
public:
	TDECLARE_CLASS( PGUIDiscLayout, Toshi::TGUILayoutManager );

	enum FlowDirection : TINT
	{
		FlowDirection_Clockwise     = 0,
		FlowDirection_AntiClockwise = 1,
		FlowDirection_NUMOF         = 2
	};

	enum SlotAllocation : TINT
	{
		SlotAllocation_Sequential  = 0,
		SlotAllocation_Distributed = 1,
		SlotAllocation_NUMOF       = 2
	};

public:
	PGUIDiscLayout();
	PGUIDiscLayout( const PGUIDiscLayout& a_rcOther );
	~PGUIDiscLayout();

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
	virtual PGUIDiscLayout* Clone() const override;
	virtual void            DoPack( Toshi::TGUIWidget* a_pWidget ) override;
	virtual void            DoLayout( Toshi::TGUIWidget* a_pWidget, TINT& a_rWidth, TINT& a_rHeight ) override;

	void           Set_samewidth_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_samewidth_property() const;
	TBOOL          Val_samewidth_property( const PPropertyValue& a_rcValue );
	void           Set_sameheight_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_sameheight_property() const;
	TBOOL          Val_sameheight_property( const PPropertyValue& a_rcValue );
	void           Set_fullwidth_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_fullwidth_property() const;
	TBOOL          Val_fullwidth_property( const PPropertyValue& a_rcValue );
	void           Set_fullheight_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_fullheight_property() const;
	TBOOL          Val_fullheight_property( const PPropertyValue& a_rcValue );
	void           Set_radiusx_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_radiusx_property() const;
	TBOOL          Val_radiusx_property( const PPropertyValue& a_rcValue );
	void           Set_radiusy_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_radiusy_property() const;
	TBOOL          Val_radiusy_property( const PPropertyValue& a_rcValue );

	// In degrees (kept in radians)
	void           Set_offsetangle_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_offsetangle_property() const;
	TBOOL          Val_offsetangle_property( const PPropertyValue& a_rcValue );

	void           Set_slotcount_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_slotcount_property() const;
	TBOOL          Val_slotcount_property( const PPropertyValue& a_rcValue );
	void           Set_flowdirection_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_flowdirection_property() const;
	TBOOL          Val_flowdirection_property( const PPropertyValue& a_rcValue );
	void           Set_slotallocation_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_slotallocation_property() const;
	TBOOL          Val_slotallocation_property( const PPropertyValue& a_rcValue );

	// $JPOG: FUNCTION PGUICore.dll+00005cc0
	TBOOL GetForceSameWidth() const { return m_bSameWidth; }

	// $JPOG: FUNCTION PGUICore.dll+00005cb0
	TBOOL GetForceSameHeight() const { return m_bSameHeight; }

	// $JPOG: FUNCTION PGUICore.dll+00005ca0
	TFLOAT GetRadiusX() const { return m_fRadiusX; }

	// $JPOG: FUNCTION PGUICore.dll+00005c90
	TFLOAT GetRadiusY() const { return m_fRadiusY; }

	// $JPOG: FUNCTION PGUICore.dll+00005c80
	TFLOAT GetOffsetAngle() const { return m_fOffsetAngle; }

	// $JPOG: FUNCTION PGUICore.dll+00005c70
	TINT GetSlotCount() const { return m_iSlotCount; }

	// $JPOG: FUNCTION PGUICore.dll+00005c60
	FlowDirection GetFlowDirection() const { return m_eFlowDirection; }

	// $JPOG: FUNCTION PGUICore.dll+00005c50
	SlotAllocation GetSlotAllocation() const { return m_eSlotAllocation; }

	// $JPOG: FUNCTION PGUICore.dll+00005d40
	void SetForceSameWidth( TBOOL a_bSameWidth ) { m_bSameWidth = a_bSameWidth; }

	// $JPOG: FUNCTION PGUICore.dll+00005d30
	void SetForceSameHeight( TBOOL a_bSameHeight ) { m_bSameHeight = a_bSameHeight; }

	// $JPOG: FUNCTION PGUICore.dll+00005d20
	void SetRadiusX( TFLOAT a_fRadius ) { m_fRadiusX = a_fRadius; }

	// $JPOG: FUNCTION PGUICore.dll+00005d10
	void SetRadiusY( TFLOAT a_fRadius ) { m_fRadiusY = a_fRadius; }

	// $JPOG: FUNCTION PGUICore.dll+00005d00
	void SetOffsetAngle( TFLOAT a_fAngle ) { m_fOffsetAngle = a_fAngle; }

	// $JPOG: FUNCTION PGUICore.dll+00005cf0
	void SetSlotCount( TINT a_iSlotCount ) { m_iSlotCount = a_iSlotCount; }

	// $JPOG: FUNCTION PGUICore.dll+00005ce0
	void SetFlowDirection( FlowDirection a_eFlowDirection ) { m_eFlowDirection = a_eFlowDirection; }

	// $JPOG: FUNCTION PGUICore.dll+00005cd0
	void SetSlotAllocation( SlotAllocation a_eSlotAllocation ) { m_eSlotAllocation = a_eSlotAllocation; }

protected:
	// The bounding box of the children placed on the disc relative to its centre (with the margins of the widget), the
	// largest size of the managed children and their number
	void CalculateLayoutSizes( Toshi::TGUIWidget* a_pWidget, Toshi::TGUIRectangle& a_rRect, TINT& a_rMaxWidth, TINT& a_rMaxHeight, TINT& a_rNumChildren ) const;

public:
	static PropertySet*           ms_oPropertySet;
	static PropertySet*           ms_oPropertySetOrdered;
	static Toshi::TGUILayoutInfo* ms_pLayoutInfo;

	static const TCHAR* const ms_csaFlowDirectionNameStr[ FlowDirection_NUMOF ];
	static const TCHAR* const ms_csaSlotAllocationNameStr[ SlotAllocation_NUMOF ];

private:
	TBOOL          m_bSameWidth;      // 0x0C
	TBOOL          m_bSameHeight;     // 0x0D
	TBOOL          m_bFullWidth;      // 0x0E
	TBOOL          m_bFullHeight;     // 0x0F
	TFLOAT         m_fRadiusX;        // 0x10
	TFLOAT         m_fRadiusY;        // 0x14
	TFLOAT         m_fOffsetAngle;    // 0x18 in radians
	TINT           m_iSlotCount;      // 0x1C
	FlowDirection  m_eFlowDirection;  // 0x20
	SlotAllocation m_eSlotAllocation; // 0x24 NOTE: not used by the layout
};
