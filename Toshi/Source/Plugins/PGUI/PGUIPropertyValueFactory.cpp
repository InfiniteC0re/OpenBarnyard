#include "ToshiPCH.h"
#include "PGUIPropertyValueFactory.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

PGUIPropertyValueFactory* PGUIPropertyValueFactory::ms_pSingleton = TNULL;

// $JPOG: FUNCTION PGUICore.dll+000079f0
PGUIPropertyValueFactory::PGUIPropertyValueFactory()
{
}

// $JPOG: FUNCTION PGUICore.dll+000062c0
PGUIPropertyValueFactory::PGUIPropertyValueFactory( const PGUIPropertyValueFactory& a_rcOther )
{
}

// $JPOG: FUNCTION PGUICore.dll+000079e0
PGUIPropertyValueFactory::~PGUIPropertyValueFactory()
{
}

// $JPOG: FUNCTION PGUICore.dll+000062d0
PGUIPropertyValueFactory* PGUIPropertyValueFactory::GetSingleton()
{
	if ( !ms_pSingleton )
		ms_pSingleton = new PGUIPropertyValueFactory();

	return ms_pSingleton;
}

// $JPOG: FUNCTION PGUICore.dll+00007590
PPropertyValue* PGUIPropertyValueFactory::AllocateValueArray( TINT a_iCount )
{
	return new PPropertyValue[ a_iCount ];
}

// One array of a_piCounts[ i ] values per array
// $JPOG: FUNCTION PGUICore.dll+00007600
PPropertyValue** PGUIPropertyValueFactory::AllocateValueArray( const TINT* const a_piCounts, TINT a_iNumArrays )
{
	PPropertyValue** ppValues = new PPropertyValue*[ a_iNumArrays ];

	for ( TINT i = 0; i < a_iNumArrays; i++ )
		ppValues[ i ] = new PPropertyValue[ a_piCounts[ i ] ];

	return ppValues;
}

// $JPOG: FUNCTION PGUICore.dll+000076b0
void PGUIPropertyValueFactory::AllocateRowData( const TINT* const a_piCounts, TINT a_iNumColumns, PPropertyValue**& a_rppValues, TUINT*& a_rpStates )
{
	a_rppValues = AllocateValueArray( a_piCounts, a_iNumColumns );
	a_rpStates  = new TUINT[ a_iNumColumns ];
}

// $JPOG: FUNCTION PGUICore.dll+000079c0
void PGUIPropertyValueFactory::DeallocateValueArray( PPropertyValue* a_pValues )
{
	if ( a_pValues )
		delete[] a_pValues;
}

// $JPOG: FUNCTION PGUICore.dll+000076f0
void PGUIPropertyValueFactory::DeallocateValueArray( TINT a_iNumArrays, PPropertyValue** a_ppValues )
{
	for ( TINT i = 0; i < a_iNumArrays; i++ )
	{
		if ( a_ppValues[ i ] )
			delete[] a_ppValues[ i ];
	}

	delete[] a_ppValues;
}

// $JPOG: FUNCTION PGUICore.dll+000079a0
void PGUIPropertyValueFactory::DeallocateRowData( TINT a_iNumColumns, PPropertyValue** a_ppValues, TUINT* a_pStates )
{
	DeallocateValueArray( a_iNumColumns, a_ppValues );
	delete[] a_pStates;
}
