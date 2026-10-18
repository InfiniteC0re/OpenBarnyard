#include "ToshiPCH.h"
#include "PGUIRowData.h"
#include "PGUIPropertyValueFactory.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

// $JPOG: FUNCTION PGUICore.dll+00007830
PGUIRowData::PGUIRowData( TINT a_iNumColumns, const TINT* const a_piNumValues, TUINT a_uiState )
    : m_iNumColumns( a_iNumColumns )
{
	PGUIPropertyValueFactory::GetSingleton()->AllocateRowData( a_piNumValues, a_iNumColumns, m_ppValues, m_pStates );
	SetState( a_uiState );
}

// NOTE: the original leaves the column count uninitialised, it is 0 here
// $JPOG: FUNCTION PGUICore.dll+000060a0
PGUIRowData::PGUIRowData( const PGUIRowData& a_rcOther )
    : m_iNumColumns( 0 )
    , m_ppValues( a_rcOther.m_ppValues )
    , m_pStates( a_rcOther.m_pStates )
{
}

// $JPOG: FUNCTION PGUICore.dll+000078c0
PGUIRowData::~PGUIRowData()
{
	if ( m_ppValues )
		PGUIPropertyValueFactory::GetSingleton()->DeallocateRowData( m_iNumColumns, m_ppValues, m_pStates );
}

// $JPOG: FUNCTION PGUICore.dll+00006060
PGUIRowData* PGUIRowData::Clone() const
{
	return new PGUIRowData( *this );
}

// $JPOG: FUNCTION PGUICore.dll+00006050
TINT PGUIRowData::GetColumnCount() const
{
	return m_iNumColumns;
}

// $JPOG: FUNCTION PGUICore.dll+00006040
PPropertyValue** PGUIRowData::GetParameterValues()
{
	return m_ppValues;
}

// $JPOG: FUNCTION PGUICore.dll+00006030
PPropertyValue* PGUIRowData::GetParameterValues( TINT a_iColumn )
{
	return m_ppValues[ a_iColumn ];
}

// $JPOG: FUNCTION PGUICore.dll+00007930
void PGUIRowData::SetState( TUINT a_uiState )
{
	for ( TINT i = 0; i < m_iNumColumns; i++ )
		m_pStates[ i ] = a_uiState;
}

// $JPOG: FUNCTION PGUICore.dll+00007950
void PGUIRowData::AddState( TUINT a_uiState )
{
	for ( TINT i = 0; i < m_iNumColumns; i++ )
		m_pStates[ i ] |= a_uiState;
}

// $JPOG: FUNCTION PGUICore.dll+00007970
void PGUIRowData::RemoveState( TUINT a_uiState )
{
	for ( TINT i = 0; i < m_iNumColumns; i++ )
		m_pStates[ i ] &= ~a_uiState;
}

// The state of the first column
// $JPOG: FUNCTION PGUICore.dll+00005fb0
TUINT PGUIRowData::GetState() const
{
	return m_pStates[ 0 ];
}

// $JPOG: FUNCTION PGUICore.dll+00006000
void PGUIRowData::SetState( TINT a_iColumn, TUINT a_uiState )
{
	m_pStates[ a_iColumn ] = a_uiState;
}

// $JPOG: FUNCTION PGUICore.dll+00005fe0
void PGUIRowData::AddState( TINT a_iColumn, TUINT a_uiState )
{
	m_pStates[ a_iColumn ] |= a_uiState;
}

// $JPOG: FUNCTION PGUICore.dll+00005fc0
void PGUIRowData::RemoveState( TINT a_iColumn, TUINT a_uiState )
{
	m_pStates[ a_iColumn ] &= ~a_uiState;
}

// $JPOG: FUNCTION PGUICore.dll+00006020
TUINT PGUIRowData::GetState( TINT a_iColumn ) const
{
	return m_pStates[ a_iColumn ];
}
