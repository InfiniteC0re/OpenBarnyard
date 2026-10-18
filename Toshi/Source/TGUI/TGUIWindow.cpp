#include "ToshiPCH.h"
#include "TGUIWindow.h"
#include "TGUIScreen.h"
#include "TGUIManager.h"
#include "TGUIWidgetInfo.h"
#include "TGUIPropertySpecSimple.h"

#include <Plugins/PPropertyParser/PProperties.h>
#include <Plugins/PPropertyParser/PProperty.h>
#include <Plugins/PPropertyParser/PPropertyValue.h>
#include <Plugins/PPropertyParser/PPropertyValueArray.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: CreateObject returns TNULL in Nicktoons (00376e10), TGUIWindow is abstract
TDEFINE_CLASS_INIT_NORUNTIME( TGUIWindow );

TGUIDeserialisable::PropertySet* TGUIWindow::ms_oPropertySet        = TNULL;
TGUIDeserialisable::PropertySet* TGUIWindow::ms_oPropertySetOrdered = TNULL;
TGUIWidgetInfo*                  TGUIWindow::ms_pWidgetInfo         = TNULL;

TPString8 TGUIWindow::str_focusmap;
TPString8 TGUIWindow::str_neighbour;
TPString8 TGUIWindow::str_default_focus;
TPString8 TGUIWindow::str_stateslaves;

// The data of serialise_stateslaves_callback
struct SerialiseStateSlavesData
{
	TGUIWindow*  pWindow;
	PProperties* pProperties;
};

// NOTE: Nicktoons creates the widget info and the strings in its static initialiser (00379b28)
// $NicktoonsPS2: FUNCTION 00376f08
// $JPOG: FUNCTION TGuiInterface.dll+00021660
void TGUIWindow::InitialiseClass()
{
	str_focusmap      = TPString8( "focusmap" );
	str_neighbour     = TPString8( "neighbour" );
	str_default_focus = TPString8( "default_focus" );
	str_stateslaves   = TPString8( "stateslaves" );

	ms_pWidgetInfo = new TGUIWidgetInfo();

	TPString8 strCategory( "TGUIWindow" );
	CreatePropertySets( ms_oPropertySet, ms_oPropertySetOrdered, *TGUIWidget::ms_oPropertySet );

	AddPropertySpec(
	    ms_oPropertySet,
	    ms_oPropertySetOrdered,
	    new TGUIPropertySpecSimple(
	        str_default_focus,
	        static_cast<TGUIPropertySpecSimple::t_Setter>( &TGUIWindow::Set_default_focus_property ),
	        static_cast<TGUIPropertySpecSimple::t_Getter>( &TGUIWindow::Get_default_focus_property ),
	        PPropertyValue::TYPE_TPSTRING8,
	        strCategory,
	        TFALSE,
	        &TGetClass( TGUIWindow ),
	        0
	    )
	);

	ms_pWidgetInfo->AddComponentName( TPString8( "background" ) );
	ms_pWidgetInfo->AddComponentName( TPString8( "border" ) );
	ms_pWidgetInfo->SetWidgetName( TPString8( "TGUIWindow" ) );
	ms_pWidgetInfo->SetWidgetLabel( TPString8( "Window" ) );
	ms_pWidgetInfo->SetProperties( ms_oPropertySet, ms_oPropertySetOrdered );
	ms_pWidgetInfo->AddStateVars( TGUIWidget::ms_pWidgetInfo->GetStateVarSet() );
	ms_pWidgetInfo->SetConcrete( TFALSE );

	// NOTE: the original also shrinks the capacity of the component name array of the widget info to its size here

	TGUIManager::AddWidgetClass( ms_pWidgetInfo );
}

// $NicktoonsPS2: FUNCTION 00377be0
// $JPOG: FUNCTION TGuiInterface.dll+00022300
void TGUIWindow::DeinitialiseClass()
{
	ms_pWidgetInfo->Clear();
	DestroyPropertySets( ms_oPropertySet, ms_oPropertySetOrdered );

	delete ms_pWidgetInfo;
	ms_pWidgetInfo = TNULL;

	str_stateslaves   = TPString8();
	str_default_focus = TPString8();
	str_neighbour     = TPString8();
	str_focusmap      = TPString8();
}

// $JPOG: FUNCTION TGuiInterface.dll+000223c0
TGUIWindow::TGUIWindow()
    : m_pDefaultFocus( TNULL )
    , m_pLastFocus( TNULL )
    , m_aFocusMap( 10, 0 )
{
}

// The focus map and the state slaves of the copy use the copied children
// $JPOG: FUNCTION TGuiInterface.dll+000229b0
TGUIWindow::TGUIWindow( const TGUIWindow& a_rcOther )
    : TGUIWidget( a_rcOther )
    , m_pDefaultFocus( TNULL )
    , m_pLastFocus( TNULL )
    , m_aFocusMap( 10, 0 )
{
	if ( a_rcOther.m_pDefaultFocus )
		m_pDefaultFocus = FindNamedChild( a_rcOther.m_pDefaultFocus->GetName() );

	for ( TINT i = 0; i < a_rcOther.m_aFocusMap.Size(); i++ )
	{
		const FocusNeighbour& rcEntry = a_rcOther.m_aFocusMap[ i ];

		if ( rcEntry.pWidget->GetName().IsEmpty() )
		{
			TGUIManager::Error( TString8( "TGUIWindow '" ) + m_strName.GetString() + "': Source widget has no name!" );
		}
		else if ( rcEntry.pNeighbour->GetName().IsEmpty() )
		{
			TGUIManager::Error( TString8( "TGUIWindow '" ) + m_strName.GetString() + "': " + GetDirectionName( rcEntry.eDirection ).GetString() + " neighbour of widget '" + rcEntry.pWidget->GetName().GetString() + "' does not have a name!" );
		}
		else
		{
			SetFocusNeighbour( FindNamedChild( rcEntry.pWidget->GetName() ), rcEntry.eDirection, FindNamedChild( rcEntry.pNeighbour->GetName() ) );
		}
	}

	RecurseTree( TNULL, TNULL, clone_stateslaves_callback, this );
}

// $NicktoonsPS2: FUNCTION 00376e40
// $JPOG: FUNCTION TGuiInterface.dll+000243a0
TGUIWindow::~TGUIWindow()
{
}

// $NicktoonsPS2: FUNCTION 00377d28
// $JPOG: FUNCTION TGuiInterface.dll+00022400
TBOOL TGUIWindow::Deserialise( const PProperties* a_pProperties )
{
	if ( !TGUIWidget::Deserialise( a_pProperties ) )
		return TFALSE;

	if ( const PPropertyValue* pValue = a_pProperties->GetProperty( str_default_focus ) )
	{
		if ( !pValue->IsType( PPropertyValue::TYPE_TPSTRING8 ) )
		{
			TGUIManager::Error( "Incorrect type for property 'default_focus'" );
			return TFALSE;
		}

		Set_default_focus_property( *pValue );
	}

	return DeserialiseFocusMap( a_pProperties ) && DeserialiseStateSlaves( a_pProperties );
}

// $NicktoonsPS2: FUNCTION 00378b80
// $JPOG: FUNCTION TGuiInterface.dll+000224e0
TBOOL TGUIWindow::Serialise( PProperties* a_pProperties )
{
	if ( !TGUIWidget::Serialise( a_pProperties ) )
		return TFALSE;

	if ( m_pDefaultFocus )
		a_pProperties->PutProperty( str_default_focus, PPropertyValue( m_pDefaultFocus->GetName() ) );

	if ( !SerialiseFocusMap( a_pProperties ) )
		return TFALSE;

	SerialiseStateSlavesData oData;
	oData.pWindow     = this;
	oData.pProperties = new PProperties( a_pProperties );

	RecurseTree( TNULL, TNULL, serialise_stateslaves_callback, &oData );

	if ( oData.pProperties->GetProperties().IsEmpty() )
	{
		delete oData.pProperties;
		return TTRUE;
	}

	oData.pProperties->Sort();
	a_pProperties->PutProperty( str_stateslaves, PPropertyValue( oData.pProperties ) );

	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 00376e20
// $JPOG: FUNCTION TGuiInterface.dll+000243e0
TGUIDeserialisable::PropertySet* TGUIWindow::GetPropertyInfoSet() const
{
	return ms_oPropertySet;
}

// $NicktoonsPS2: FUNCTION 00376e28
// $JPOG: FUNCTION TGuiInterface.dll+000243d0
TGUIDeserialisable::PropertySet* TGUIWindow::GetPropertyInfoOrderedSet() const
{
	return ms_oPropertySetOrdered;
}

// $NicktoonsPS2: FUNCTION 00379c70
const TGUIWidgetInfo* TGUIWindow::GetWidgetInfo() const
{
	return ms_pWidgetInfo;
}

// $NicktoonsPS2: FUNCTION 00376ee8
void TGUIWindow::ResetParameters()
{
	TGUIWidget::ResetParameters();
}

// $NicktoonsPS2: FUNCTION 00379c80
// $JPOG: FUNCTION TGuiInterface.dll+000153d0
TBOOL TGUIWindow::AcceptsFocus() const
{
	return m_pDefaultFocus != TNULL;
}

// The removed widget leaves the focus map
// $NicktoonsPS2: FUNCTION 003796c0
// $JPOG: FUNCTION TGuiInterface.dll+00023c80
void TGUIWindow::NotifyDescendantRemoval( TGUIWidget* a_pWidget )
{
	if ( a_pWidget == m_pLastFocus )
		m_pLastFocus = TNULL;

	if ( a_pWidget == m_pDefaultFocus )
		m_pDefaultFocus = TNULL;

	for ( TINT i = 0; i < m_aFocusMap.Size(); i++ )
	{
		if ( m_aFocusMap[ i ].pWidget == a_pWidget || m_aFocusMap[ i ].pNeighbour == a_pWidget )
		{
			for ( TINT j = i; j < m_aFocusMap.Size() - 1; j++ )
				m_aFocusMap[ j ] = m_aFocusMap[ j + 1 ];

			m_aFocusMap.SetSize( m_aFocusMap.Size() - 1 );
			i--;
		}
	}

	TGUIWidget::NotifyDescendantRemoval( a_pWidget );
}

// The window remembers the last focused widget (a_pOldFocus, unless it's the window)
// $NicktoonsPS2: FUNCTION 00379880
// $JPOG: FUNCTION TGuiInterface.dll+00024310
void TGUIWindow::NotifyFocusLost( TGUIWidget* a_pNewFocus, TGUIWidget* a_pOldFocus )
{
	m_pLastFocus = ( a_pOldFocus != this ) ? a_pOldFocus : TNULL;
	TGUIWidget::NotifyFocusLost( a_pNewFocus, a_pOldFocus );
}

// $NicktoonsPS2: FUNCTION 003798b0
// $JPOG: FUNCTION TGuiInterface.dll+000242f0
void TGUIWindow::NotifyFocusGained( TGUIWidget* a_pNewFocus, TGUIWidget* a_pOldFocus )
{
	TGUIWidget::NotifyFocusGained( a_pNewFocus, a_pOldFocus );
}

// NOTE: the result of IsA isn't used (as in the original), JPOG doesn't call the base
// $NicktoonsPS2: FUNCTION 00379650
// $JPOG: FUNCTION TGuiInterface.dll+00024330
void TGUIWindow::NotifyParentChange( TGUIWidget* a_pParent )
{
	if ( a_pParent )
	{
		a_pParent->IsA( &TGetClass( TGUIScreen ) );
		m_pLastFocus = m_pDefaultFocus;
	}

	TGUIWidget::NotifyParentChange( a_pParent );
}

// $NicktoonsPS2: FUNCTION 003798d0
// $JPOG: FUNCTION TGuiInterface.dll+000225e0
TGUIWidget* TGUIWindow::GetAutoFocus()
{
	if ( m_pLastFocus && m_pLastFocus->AcceptsFocus() && m_pLastFocus->CanFocus() )
		return m_pLastFocus;

	if ( !m_pDefaultFocus )
		return this;

	if ( m_pDefaultFocus->AcceptsFocus() && m_pDefaultFocus->CanFocus() )
		return m_pDefaultFocus;

	if ( TGUIWidget* pWidget = FindFocusNeighbour( m_pDefaultFocus, NeighbourDirection_South ) )
		return pWidget;

	if ( TGUIWidget* pWidget = FindFocusNeighbour( m_pDefaultFocus, NeighbourDirection_North ) )
		return pWidget;

	if ( TGUIWidget* pWidget = FindFocusNeighbour( m_pDefaultFocus, NeighbourDirection_East ) )
		return pWidget;

	if ( TGUIWidget* pWidget = FindFocusNeighbour( m_pDefaultFocus, NeighbourDirection_West ) )
		return pWidget;

	return this;
}

// NOTE: inlined in GetFocusNeighbour and SetFocusNeighbour
TINT TGUIWindow::FindFocusNeighbourEntry( TGUIWidget* a_pWidget, NeighbourDirection a_eDirection ) const
{
	TINT iLow  = 0;
	TINT iHigh = m_aFocusMap.Size() - 1;

	while ( iLow <= iHigh )
	{
		TINT                  iMiddle = ( iLow + iHigh ) / 2;
		const FocusNeighbour& rcEntry = m_aFocusMap[ iMiddle ];

		intptr_t iCompare = intptr_t( a_pWidget ) - intptr_t( rcEntry.pWidget );

		if ( iCompare == 0 )
			iCompare = a_eDirection - rcEntry.eDirection;

		if ( iCompare < 0 )
		{
			iHigh = iMiddle - 1;
		}
		else if ( iCompare > 0 )
		{
			iLow = iMiddle + 1;
		}
		else
		{
			// The first entry
			while ( iMiddle - 1 >= iLow && m_aFocusMap[ iMiddle - 1 ].pWidget == a_pWidget && m_aFocusMap[ iMiddle - 1 ].eDirection == a_eDirection )
				iMiddle--;

			return iMiddle;
		}
	}

	return -1;
}

// NOTE: inlined in DeserialiseFocusMap and SetFocusNeighbour
TINT TGUIWindow::GetFocusNeighbourInsertIndex( TGUIWidget* a_pWidget, NeighbourDirection a_eDirection ) const
{
	TINT iLow  = 0;
	TINT iHigh = m_aFocusMap.Size() - 1;

	while ( iLow <= iHigh )
	{
		TINT                  iMiddle = ( iLow + iHigh ) / 2;
		const FocusNeighbour& rcEntry = m_aFocusMap[ iMiddle ];

		intptr_t iCompare = intptr_t( a_pWidget ) - intptr_t( rcEntry.pWidget );

		if ( iCompare == 0 )
			iCompare = a_eDirection - rcEntry.eDirection;

		if ( iCompare < 0 )
		{
			iHigh = iMiddle - 1;
		}
		else
		{
			iLow = iMiddle + 1;

			if ( iCompare == 0 )
			{
				while ( iLow <= iHigh && m_aFocusMap[ iLow ].pWidget == a_pWidget && m_aFocusMap[ iLow ].eDirection == a_eDirection )
					iLow++;

				break;
			}
		}
	}

	return iLow;
}

// Sets the neighbour (or removes it if a_pNeighbour is TNULL)
// $JPOG: FUNCTION TGuiInterface.dll+000227a0
void TGUIWindow::SetFocusNeighbour( TGUIWidget* a_pWidget, NeighbourDirection a_eDirection, TGUIWidget* a_pNeighbour )
{
	TINT iIndex = FindFocusNeighbourEntry( a_pWidget, a_eDirection );

	if ( iIndex != -1 )
	{
		if ( a_pNeighbour )
		{
			m_aFocusMap[ iIndex ].pNeighbour = a_pNeighbour;
		}
		else
		{
			for ( TINT j = iIndex; j < m_aFocusMap.Size() - 1; j++ )
				m_aFocusMap[ j ] = m_aFocusMap[ j + 1 ];

			m_aFocusMap.SetSize( m_aFocusMap.Size() - 1 );
		}
	}
	else if ( a_pNeighbour )
	{
		FocusNeighbour oEntry;
		oEntry.pWidget    = a_pWidget;
		oEntry.eDirection = a_eDirection;
		oEntry.pNeighbour = a_pNeighbour;
		m_aFocusMap.InsertBefore( TArray<FocusNeighbour>::Iterator( GetFocusNeighbourInsertIndex( a_pWidget, a_eDirection ), m_aFocusMap ), oEntry );
	}
}

// $NicktoonsPS2: FUNCTION 003794c8
// $JPOG: FUNCTION TGuiInterface.dll+000228d0
TGUIWidget* TGUIWindow::GetFocusNeighbour( TGUIWidget* a_pWidget, NeighbourDirection a_eDirection )
{
	TINT iIndex = FindFocusNeighbourEntry( a_pWidget, a_eDirection );

	if ( iIndex == -1 )
		return TNULL;

	return m_aFocusMap[ iIndex ].pNeighbour;
}

// Follows the neighbours (32 at most) until one accepts the focus and can be focused
// $NicktoonsPS2: FUNCTION 003799c8
// $JPOG: FUNCTION TGuiInterface.dll+00022930
TGUIWidget* TGUIWindow::FindFocusNeighbour( TGUIWidget* a_pWidget, NeighbourDirection a_eDirection )
{
	TGUIWidget* pNeighbour = GetFocusNeighbour( a_pWidget, a_eDirection );

	for ( TINT i = 0; i < 32; i++ )
	{
		if ( !pNeighbour || pNeighbour == a_pWidget )
			return TNULL;

		if ( pNeighbour->AcceptsFocus() && pNeighbour->CanFocus() )
			return pNeighbour;

		pNeighbour = GetFocusNeighbour( pNeighbour, a_eDirection );
	}

	return TNULL;
}

// Reads the "focusmap" block: "<widget>.<direction> = <neighbour>;"
// $NicktoonsPS2: FUNCTION 00377df0
// $JPOG: FUNCTION TGuiInterface.dll+00022cc0
TBOOL TGUIWindow::DeserialiseFocusMap( const PProperties* a_pProperties )
{
	const PPropertyValue* pFocusMap = GetOptionalProperty( a_pProperties, str_focusmap, PPropertyValue::TYPE_PROPS );

	if ( !pFocusMap )
		return TTRUE;

	PProperties* pProperties = pFocusMap->GetProperties();

	for ( auto it = pProperties->GetProperties().Begin(); it != pProperties->GetProperties().End(); it++ )
	{
		TPString8             strWidget    = it->GetName().GetName();
		TPString8             strDirection = it->GetName().GetSubName();
		const PPropertyValue& rcValue      = it->GetValue();

		TGUIWidget* pWidget = FindNamedChild( strWidget );

		if ( !pWidget )
		{
			TGUIManager::Error( TString8( "TGUIWindow '" ) + m_strName.GetString() + "': Unknown widget '" + strWidget.GetString() + "' found in focusmap!" );
			continue;
		}

		NeighbourDirection eDirection = GetDirectionEnum( strDirection );

		if ( eDirection == NeighbourDirection_NUMOF )
		{
			TGUIManager::Error( TString8( "TGUIWindow '" ) + m_strName.GetString() + "': Child widget '" + strWidget.GetString() + "' has illegal neighbour direction '" + strDirection.GetString() + "' in focusmap!" );
			continue;
		}

		if ( !rcValue.IsType( PPropertyValue::TYPE_TPSTRING8 ) )
		{
			TGUIManager::Error( TString8( "TGUIWindow '" ) + m_strName.GetString() + "': Child widget '" + strWidget.GetString() + "' has illegal neighbour type in focusmap!" );
			continue;
		}

		TGUIWidget* pNeighbour = FindNamedChild( rcValue.GetTPString8() );

		if ( !pNeighbour )
		{
			TGUIManager::Error( TString8( "TGUIWindow '" ) + m_strName.GetString() + "': Child widget '" + strWidget.GetString() + "' has unkown neighbour '" + rcValue.GetTPString8().GetString() + "' in direction '" + strDirection.GetString() + "' in focusmap!" );
			continue;
		}

		FocusNeighbour oEntry;
		oEntry.pWidget    = pWidget;
		oEntry.eDirection = eDirection;
		oEntry.pNeighbour = pNeighbour;
		m_aFocusMap.InsertBefore( TArray<FocusNeighbour>::Iterator( GetFocusNeighbourInsertIndex( pWidget, eDirection ), m_aFocusMap ), oEntry );
	}

	return TTRUE;
}

// Reads the "stateslaves" block: "<master>.<slave> = <state>;" or "= [<states>];" (the window itself is found by its
// name too)
// NOTE: the error of an unknown state gives the name of the slave (as in the original)
// $NicktoonsPS2: FUNCTION 003786e0
// $JPOG: FUNCTION TGuiInterface.dll+00023460
TBOOL TGUIWindow::DeserialiseStateSlaves( const PProperties* a_pProperties )
{
	const PPropertyValue* pStateSlaves = GetOptionalProperty( a_pProperties, str_stateslaves, PPropertyValue::TYPE_PROPS );

	if ( !pStateSlaves )
		return TTRUE;

	PProperties* pProperties = pStateSlaves->GetProperties();

	for ( auto it = pProperties->GetProperties().Begin(); it != pProperties->GetProperties().End(); it++ )
	{
		const TPString8& rcMasterName = it->GetName().GetName();
		const TPString8& rcSlaveName  = it->GetName().GetSubName();

		TGUIWidget* pMaster = ( m_strName == rcMasterName ) ? this : FindNamedChild( rcMasterName );
		TGUIWidget* pSlave  = ( m_strName == rcSlaveName ) ? this : FindNamedChild( rcSlaveName );

		if ( !pMaster )
		{
			TGUIManager::Error( TString8( "TGUIWidget: Unknown child widget '" ) + rcSlaveName.GetString() + "' given for master of state slave" );
			continue;
		}

		if ( !pSlave )
		{
			TGUIManager::Error( TString8( "TGUIWidget: Unknown child widget '" ) + rcSlaveName.GetString() + "' given for slave of state master" );
			continue;
		}

		const PPropertyValue& rcValue = it->GetValue();
		TUINT                 uiMask  = 0;

		if ( rcValue.GetType() == PPropertyValue::TYPE_TPSTRING8 )
		{
			const TGUIStateVarInfo* pStateVar = pMaster->GetWidgetInfo()->GetStateVarInfo( rcValue.GetTPString8() );

			if ( !pStateVar )
			{
				TGUIManager::Error( TString8( "TGUIWidget: Unknown state '" ) + rcSlaveName.GetString() + "' for state slave" );
				continue;
			}

			uiMask = pStateVar->uiMask;
		}
		else if ( rcValue.GetType() == PPropertyValue::TYPE_ARRAY )
		{
			PPropertyValueArray* pArray = rcValue.GetArray();

			for ( TINT i = 0; i < pArray->GetSize(); i++ )
			{
				const PPropertyValue* pState = pArray->GetValue( i );

				if ( pState->GetType() != PPropertyValue::TYPE_TPSTRING8 )
					continue;

				const TGUIStateVarInfo* pStateVar = pMaster->GetWidgetInfo()->GetStateVarInfo( pState->GetTPString8() );

				if ( pStateVar )
					uiMask |= pStateVar->uiMask;
				else
					TGUIManager::Error( TString8( "TGUIWidget: Unknown state '" ) + rcSlaveName.GetString() + "' for state slave" );
			}
		}
		else
		{
			TGUIManager::Error( "TGUIWidget: Incorrect type of 'stateslave' property" );
			continue;
		}

		pMaster->AddStateSlave( pSlave, uiMask );
	}

	return TTRUE;
}

// Writes the "focusmap" block
// $NicktoonsPS2: FUNCTION 00378cd0
// $JPOG: FUNCTION TGuiInterface.dll+00023940
TBOOL TGUIWindow::SerialiseFocusMap( PProperties* a_pProperties )
{
	PProperties* pProperties = new PProperties();
	a_pProperties->PutProperty( str_focusmap, PPropertyValue( pProperties ) );

	for ( TINT i = 0; i < m_aFocusMap.Size(); i++ )
	{
		const FocusNeighbour& rcEntry = m_aFocusMap[ i ];

		if ( rcEntry.pWidget->GetName().IsEmpty() )
		{
			TGUIManager::Error( TString8( "TGUIWindow '" ) + m_strName.GetString() + "': Source widget has no name!" );
		}
		else if ( rcEntry.pNeighbour->GetName().IsEmpty() )
		{
			TGUIManager::Error( TString8( "TGUIWindow '" ) + m_strName.GetString() + "': " + GetDirectionName( rcEntry.eDirection ).GetString() + " neighbour of widget '" + rcEntry.pWidget->GetName().GetString() + "' does not have a name!" );
		}
		else
		{
			pProperties->PutProperty( rcEntry.pWidget->GetName(), GetDirectionName( rcEntry.eDirection ), PPropertyValue( rcEntry.pNeighbour->GetName() ) );
		}
	}

	pProperties->Sort();
	return TTRUE;
}

// The state slaves of the copied widgets are the copied widgets with the same names
// $JPOG: FUNCTION TGuiInterface.dll+00023e00
TBOOL __stdcall TGUIWindow::clone_stateslaves_callback( TGUIWidget* a_pWidget, void* a_pUserData )
{
	TGUIWindow* pWindow = TSTATICCAST( TGUIWindow, a_pUserData );

	for ( auto it = a_pWidget->GetStateSlaveList().Begin(); it != a_pWidget->GetStateSlaveList().End(); it++ )
		it->pSlave = pWindow->FindNamedChild( it->pSlave->GetName() );

	return TTRUE;
}

// Writes the state slaves of a widget: "<master>.<slave> = [<states>];"
// $NicktoonsPS2: FUNCTION 00379070
// $JPOG: FUNCTION TGuiInterface.dll+00023e60
TBOOL __stdcall TGUIWindow::serialise_stateslaves_callback( TGUIWidget* a_pWidget, void* a_pUserData )
{
	SerialiseStateSlavesData* pData = TSTATICCAST( SerialiseStateSlavesData, a_pUserData );

	if ( a_pWidget->GetName().IsEmpty() )
	{
		if ( !a_pWidget->GetStateSlaveList().IsEmpty() )
			TGUIManager::Error( "TGUIWindow: Can't store slave states of widget without a name!" );

		return TTRUE;
	}

	for ( auto it = a_pWidget->GetStateSlaveList().Begin(); it != a_pWidget->GetStateSlaveList().End(); it++ )
	{
		TGUIWidget* pSlave = it->pSlave;

		if ( pSlave->GetName().IsEmpty() )
		{
			TGUIManager::Error( "TGUIWindow: Can't store slave widget withou a name!" );
			continue;
		}

		// NOTE: an array of 8 values in the original
		PPropertyValueArray*  pStates     = new PPropertyValueArray();
		const TGUIWidgetInfo* pWidgetInfo = pSlave->GetWidgetInfo();

		for ( TINT i = 0; i < pWidgetInfo->GetStateVarCount(); i++ )
		{
			const TGUIStateVarInfo* pStateVar = pWidgetInfo->GetStateVarInfo( i );

			if ( pStateVar->uiMask & it->uiMask )
				pStates->Push( PPropertyValue( pStateVar->strName ) );
		}

		pData->pProperties->PutProperty( a_pWidget->GetName(), pSlave->GetName(), PPropertyValue( pStates ) );
	}

	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 00379a80
// $JPOG: FUNCTION TGuiInterface.dll+00024380
void TGUIWindow::Set_default_focus_property( const PPropertyValue& a_rcValue )
{
	m_pDefaultFocus = FindNamedChild( a_rcValue.GetTPString8() );
}

// $NicktoonsPS2: FUNCTION 00379ab8
// $JPOG: FUNCTION TGuiInterface.dll+000226b0
PPropertyValue TGUIWindow::Get_default_focus_property() const
{
	if ( m_pDefaultFocus )
		return PPropertyValue( m_pDefaultFocus->GetName() );

	return PPropertyValue( TPString8() );
}

// NOTE: checks that the value can be an integer before reading it as a string (as in the original), unused in Nicktoons
// $JPOG: FUNCTION TGuiInterface.dll+00022710
TBOOL TGUIWindow::Val_default_focus_property( const PPropertyValue& a_rcValue )
{
	if ( !a_rcValue.CanBeType( PPropertyValue::TYPE_INT ) )
		return TFALSE;

	if ( a_rcValue.GetTPString8().IsEmpty() )
		return TTRUE;

	return FindNamedChild( a_rcValue.GetTPString8() ) != TNULL;
}

TOSHI_NAMESPACE_END
