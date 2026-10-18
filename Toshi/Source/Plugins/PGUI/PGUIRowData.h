#pragma once
#include <Toshi/TNodeList.h>
#include <Plugins/PPropertyParser/PPropertyValue.h>

//-----------------------------------------------------------------------------
// Purpose: a row of a table box: per column, the values of the parameters of
// the style of the cell and its state.
//
// NOTE: the copy constructor and Clone share the values and the states with
// the original and don't copy the column count (as in the original).
//-----------------------------------------------------------------------------
class PGUIRowData : public Toshi::TNodeList<PGUIRowData>::TNode
{
public:
	// a_piNumValues: the number of values of each column
	PGUIRowData( TINT a_iNumColumns, const TINT* const a_piNumValues, TUINT a_uiState );
	virtual ~PGUIRowData();

	virtual PGUIRowData* Clone() const;

	TINT GetColumnCount() const;

	PPropertyValue** GetParameterValues();
	PPropertyValue*  GetParameterValues( TINT a_iColumn );

	// The state of every column
	void  SetState( TUINT a_uiState );
	void  AddState( TUINT a_uiState );
	void  RemoveState( TUINT a_uiState );
	TUINT GetState() const;

	// The state of a column
	void  SetState( TINT a_iColumn, TUINT a_uiState );
	void  AddState( TINT a_iColumn, TUINT a_uiState );
	void  RemoveState( TINT a_iColumn, TUINT a_uiState );
	TUINT GetState( TINT a_iColumn ) const;

protected:
	PGUIRowData( const PGUIRowData& a_rcOther );

protected:
	TINT             m_iNumColumns; // 0x10
	PPropertyValue** m_ppValues;    // 0x14
	TUINT*           m_pStates;     // 0x18
};
