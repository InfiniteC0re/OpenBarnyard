#include "ToshiPCH.h"
#include "PGUIItemData.h"
#include "PGUIPropertyValueFactory.h"

#include <TGUI/TGUIWidget.h>
#include <TGUI/TGUIStyle.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

// $JPOG: FUNCTION PGUICore.dll+000061e0
PGUIItemData::PGUIItemData( TINT a_iNumValues, TUINT a_uiState )
{
	m_pValues = PGUIPropertyValueFactory::GetSingleton()->AllocateValueArray( a_iNumValues );
	m_uiState = a_uiState;
}

// $JPOG: FUNCTION PGUICore.dll+00006250
PGUIItemData::PGUIItemData( const PPropertyValue& a_rcValue, TUINT a_uiState )
{
	m_pValues = new PPropertyValue[ 1 ];
	m_uiState = a_uiState;

	m_pValues[ 0 ] = a_rcValue;
}

// $JPOG: FUNCTION PGUICore.dll+000061b0
PGUIItemData::PGUIItemData( const PGUIItemData& a_rcOther )
    : m_pValues( a_rcOther.m_pValues )
    , m_uiState( a_rcOther.m_uiState )
{
}

// $JPOG: FUNCTION PGUICore.dll+00007760
PGUIItemData::~PGUIItemData()
{
	if ( m_pValues )
		PGUIPropertyValueFactory::GetSingleton()->DeallocateValueArray( m_pValues );
}

// $JPOG: FUNCTION PGUICore.dll+00006170
PGUIItemData* PGUIItemData::Clone() const
{
	return new PGUIItemData( *this );
}

// $JPOG: FUNCTION PGUICore.dll+000060d0
TBOOL PGUIItemData::CanSelect() const
{
	return ( m_uiState & ( State_Enabled | State_Visible ) ) == ( State_Enabled | State_Visible );
}

// $JPOG: FUNCTION PGUICore.dll+000060e0
void PGUIItemData::SetState( TUINT a_uiMask, TUINT a_uiValue )
{
	m_uiState = ( a_uiMask & a_uiValue ) | ( m_uiState & ~a_uiMask );
}

// $JPOG: FUNCTION PGUICore.dll+00006100
void PGUIItemData::SetState( TUINT a_uiMask, TBOOL a_bSet )
{
	if ( a_bSet )
		m_uiState |= a_uiMask;
	else
		m_uiState &= ~a_uiMask;
}

// $JPOG: FUNCTION PGUICore.dll+00006140
void PGUIItemData::SetState( TUINT a_uiState )
{
	m_uiState = a_uiState;
}

// $JPOG: FUNCTION PGUICore.dll+00006130
void PGUIItemData::AddState( TUINT a_uiState )
{
	m_uiState |= a_uiState;
}

// $JPOG: FUNCTION PGUICore.dll+00006120
void PGUIItemData::RemoveState( TUINT a_uiState )
{
	m_uiState &= ~a_uiState;
}

// $JPOG: FUNCTION PGUICore.dll+00006150
TUINT PGUIItemData::GetState() const
{
	return m_uiState;
}

// $JPOG: FUNCTION PGUICore.dll+00006160
PPropertyValue* PGUIItemData::GetParameterValues()
{
	return m_pValues;
}

// $JPOG: FUNCTION PGUICore.dll+000077c0
void PGUIItemData::UpdateWidget( TGUIWidget* a_pWidget )
{
	if ( TGUIStyle* pStyle = a_pWidget->GetStyle() )
	{
		for ( TINT i = 0; i < pStyle->GetParameterCount(); i++ )
			a_pWidget->SetParameter( i, m_pValues[ i ] );
	}

	a_pWidget->SetState( State_WidgetMask, m_uiState );
}
