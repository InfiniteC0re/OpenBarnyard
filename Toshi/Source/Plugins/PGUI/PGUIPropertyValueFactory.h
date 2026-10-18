#pragma once
#include <Toshi/TObject.h>
#include <Plugins/PPropertyParser/PPropertyValue.h>

//-----------------------------------------------------------------------------
// Purpose: allocates the parameter values of the items of the list boxes,
// table boxes and menus (PGUIItemData, PGUIRowData): arrays of values, and
// for the rows an array of value arrays (one per column) with their states.
// Created on first use.
//-----------------------------------------------------------------------------
class PGUIPropertyValueFactory : public Toshi::TObject
{
public:
	PGUIPropertyValueFactory();
	PGUIPropertyValueFactory( const PGUIPropertyValueFactory& a_rcOther );
	~PGUIPropertyValueFactory();

	PPropertyValue*  AllocateValueArray( TINT a_iCount );
	PPropertyValue** AllocateValueArray( const TINT* const a_piCounts, TINT a_iNumArrays );
	void             AllocateRowData( const TINT* const a_piCounts, TINT a_iNumColumns, PPropertyValue**& a_rppValues, TUINT*& a_rpStates );

	void DeallocateValueArray( PPropertyValue* a_pValues );
	void DeallocateValueArray( TINT a_iNumArrays, PPropertyValue** a_ppValues );
	void DeallocateRowData( TINT a_iNumColumns, PPropertyValue** a_ppValues, TUINT* a_pStates );

	static PGUIPropertyValueFactory* GetSingleton();

private:
	static PGUIPropertyValueFactory* ms_pSingleton;
};
