#pragma once
#include "PGUILayoutManager.h"

//-----------------------------------------------------------------------------
// Purpose: arranges the managed children in a grid of "rows" x "columns"
// cells (row by row), each row as high and each column as wide as its largest
// child. The space left goes to the free rows and columns (fill options), the
// cells shrink proportionally when there isn't enough.
//-----------------------------------------------------------------------------
class PGUIGridLayout : public PGUILayoutManager
{
public:
	TDECLARE_CLASS( PGUIGridLayout, PGUILayoutManager );

	static constexpr TINT MAX_ROWS    = 64;
	static constexpr TINT MAX_COLUMNS = 16;

	// The size of a row or a column and its size mode (the most constraining one of its children)
	struct Cell
	{
		TINT16 iSize;
		TINT16 iSizeMode;
	};

public:
	PGUIGridLayout();
	PGUIGridLayout( const PGUIGridLayout& a_rcOther );
	~PGUIGridLayout();

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
	virtual PGUIGridLayout* Clone() const override;
	virtual void            DoPack( Toshi::TGUIWidget* a_pWidget ) override;
	virtual void            DoLayout( Toshi::TGUIWidget* a_pWidget, TINT& a_rWidth, TINT& a_rHeight ) override;

	void           Set_rows_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_rows_property() const;
	TBOOL          Val_rows_property( const PPropertyValue& a_rcValue );
	void           Set_columns_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_columns_property() const;
	TBOOL          Val_columns_property( const PPropertyValue& a_rcValue );

	// $JPOG: FUNCTION PGUICore.dll+00021d50
	TINT GetNumRows() const { return m_iNumRows; }

	// $JPOG: FUNCTION PGUICore.dll+00021d40
	TINT GetNumCols() const { return m_iNumColumns; }

	// $JPOG: FUNCTION PGUICore.dll+00021d70
	void SetNumRows( TINT a_iNumRows ) { m_iNumRows = TINT16( a_iNumRows ); }

	// $JPOG: FUNCTION PGUICore.dll+00021d60
	void SetNumCols( TINT a_iNumColumns ) { m_iNumColumns = TINT16( a_iNumColumns ); }

public:
	static PropertySet*           ms_oPropertySet;
	static PropertySet*           ms_oPropertySetOrdered;
	static Toshi::TGUILayoutInfo* ms_pLayoutInfo;

private:
	TINT16 m_iNumRows;    // 0x14
	TINT16 m_iNumColumns; // 0x16
};
