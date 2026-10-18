#include "ToshiPCH.h"
#include "TGUIStyle.h"
#include "TGUIManager.h"
#include "TGUIWidget.h"
#include "TGUIWidgetInfo.h"
#include "TGUIComponent.h"
#include "TGUIPropertySpecSimple.h"
#include "TGUIDisplayContext.h"

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

TDEFINE_CLASS_INIT( TGUIStyle );

TGUIDeserialisable::PropertySet* TGUIStyle::ms_oPropertySet        = TNULL;
TGUIDeserialisable::PropertySet* TGUIStyle::ms_oPropertySetOrdered = TNULL;

TPString8 TGUIStyle::str_type;
TPString8 TGUIStyle::str_widget;
TPString8 TGUIStyle::str_name;
TPString8 TGUIStyle::str_widgetprop;
TPString8 TGUIStyle::str_component;
TPString8 TGUIStyle::str_state;
TPString8 TGUIStyle::str_element;
TPString8 TGUIStyle::str_param;
TPString8 TGUIStyle::str_marginleft;
TPString8 TGUIStyle::str_marginright;
TPString8 TGUIStyle::str_margintop;
TPString8 TGUIStyle::str_marginbottom;
TPString8 TGUIStyle::ms_strCategory;
TPString8 TGUIStyle::ms_strStyleParams;

// NOTE: Nicktoons creates the strings with the TClass object (static initialiser 0036dd60)
// $NicktoonsPS2: FUNCTION 0036a410
// $JPOG: FUNCTION TGuiInterface.dll+00015da0
void TGUIStyle::InitialiseClass()
{
	str_type         = TPString8( "type" );
	str_widget       = TPString8( "widget" );
	str_name         = TPString8( "name" );
	str_widgetprop   = TPString8( "widgetprop" );
	str_component    = TPString8( "component" );
	str_state        = TPString8( "state" );
	str_element      = TPString8( "element" );
	str_param        = TPString8( "param" );
	str_marginleft   = TPString8( "marginleft" );
	str_marginright  = TPString8( "marginright" );
	str_margintop    = TPString8( "margintop" );
	str_marginbottom = TPString8( "marginbottom" );

	ms_strCategory    = TPString8( "TGUIStyle" );
	ms_strStyleParams = TPString8( "Style Params" );

	ms_oPropertySet        = new PropertySet( 4, 0 );
	ms_oPropertySetOrdered = new PropertySet( 4, 0 );

	using t_Setter = TGUIPropertySpecSimple::t_Setter;
	using t_Getter = TGUIPropertySpecSimple::t_Getter;

	TINT iSortHint = 1;

	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( str_name, static_cast<t_Setter>( &TGUIStyle::Set_name_property ), static_cast<t_Getter>( &TGUIStyle::Get_name_property ), PPropertyValue::TYPE_TPSTRING8, ms_strCategory, TFALSE, &TGetClass( TGUIStyle ), 0 ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( str_marginleft, static_cast<t_Setter>( &TGUIStyle::Set_marginleft_property ), static_cast<t_Getter>( &TGUIStyle::Get_marginleft_property ), PPropertyValue::TYPE_INT, ms_strCategory, TTRUE, &TGetClass( TGUIStyle ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( str_marginright, static_cast<t_Setter>( &TGUIStyle::Set_marginright_property ), static_cast<t_Getter>( &TGUIStyle::Get_marginright_property ), PPropertyValue::TYPE_INT, ms_strCategory, TTRUE, &TGetClass( TGUIStyle ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( str_margintop, static_cast<t_Setter>( &TGUIStyle::Set_margintop_property ), static_cast<t_Getter>( &TGUIStyle::Get_margintop_property ), PPropertyValue::TYPE_INT, ms_strCategory, TTRUE, &TGetClass( TGUIStyle ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( str_marginbottom, static_cast<t_Setter>( &TGUIStyle::Set_marginbottom_property ), static_cast<t_Getter>( &TGUIStyle::Get_marginbottom_property ), PPropertyValue::TYPE_INT, ms_strCategory, TTRUE, &TGetClass( TGUIStyle ), iSortHint ) );
}

// $NicktoonsPS2: FUNCTION 0036b930
// $JPOG: FUNCTION TGuiInterface.dll+00016e80
void TGUIStyle::DeinitialiseClass()
{
	DestroyPropertySets( ms_oPropertySet, ms_oPropertySetOrdered );

	ms_strStyleParams = TPString8();
	ms_strCategory    = TPString8();
	str_marginbottom  = TPString8();
	str_margintop     = TPString8();
	str_marginright   = TPString8();
	str_marginleft    = TPString8();
	str_param         = TPString8();
	str_element       = TPString8();
	str_state         = TPString8();
	str_component     = TPString8();
	str_widgetprop    = TPString8();
	str_name          = TPString8();
	str_widget        = TPString8();
	str_type          = TPString8();
}

// $NicktoonsPS2: FUNCTION 00369eb0
// $JPOG: FUNCTION TGuiInterface.dll+00016f30
TGUIStyle::TGUIStyle()
    : m_pRootComponents( TNULL )
    , m_pWidgetInfo( TNULL )
    , m_uiNumParameters( 0 )
    , m_pParameters( TNULL )
    , m_iMarginTop( 0 )
    , m_iMarginBottom( 0 )
    , m_iMarginLeft( 0 )
    , m_iMarginRight( 0 )
{
}

// $NicktoonsPS2: FUNCTION 00369f30
// $JPOG: FUNCTION TGuiInterface.dll+00018780
TGUIStyle::TGUIStyle( const TGUIStyle& a_rcOther )
    : TGUIDeserialisable( a_rcOther )
    , m_pRootComponents( TNULL )
    , m_pWidgetInfo( a_rcOther.m_pWidgetInfo )
    , m_strName( a_rcOther.m_strName )
    , m_uiNumParameters( 0 )
    , m_pParameters( TNULL )
    , m_iMarginTop( a_rcOther.m_iMarginTop )
    , m_iMarginBottom( a_rcOther.m_iMarginBottom )
    , m_iMarginLeft( a_rcOther.m_iMarginLeft )
    , m_iMarginRight( a_rcOther.m_iMarginRight )
{
	// The parameters
	m_uiNumParameters = a_rcOther.m_uiNumParameters;
	m_pParameters     = new TGUIStyleParameterInfo[ m_uiNumParameters ];

	for ( TINT i = 0; i < m_uiNumParameters; i++ )
		m_pParameters[ i ] = a_rcOther.m_pParameters[ i ];

	// The fixed widget properties
	for ( auto it = a_rcOther.m_oFixedWidgetProperties.Begin(); it != a_rcOther.m_oFixedWidgetProperties.End(); it++ )
		m_oFixedWidgetProperties.InsertTail( new FixedWidgetProperty( it->oKey, it->oValue ) );

	// The root components
	TINT iNumRootComponents = m_pWidgetInfo->GetComponentCount();
	m_pRootComponents       = new TGUIStyleRootComponent[ iNumRootComponents ];

	for ( TINT i = 0; i < iNumRootComponents; i++ )
		m_pRootComponents[ i ] = a_rcOther.m_pRootComponents[ i ];
}

// $NicktoonsPS2: FUNCTION 0036a218
// $JPOG: FUNCTION TGuiInterface.dll+00016f80
TGUIStyle::~TGUIStyle()
{
	if ( m_pRootComponents )
		delete[] m_pRootComponents;

	if ( m_pParameters )
		delete[] m_pParameters;

	m_oFixedWidgetProperties.DeleteAll();
}

// $NicktoonsPS2: FUNCTION 0036ba68
// $JPOG: FUNCTION TGuiInterface.dll+00019b40
TGUIStyle* TGUIStyle::Clone() const
{
	return new TGUIStyle( *this );
}

// $NicktoonsPS2: FUNCTION 003698f0
// $JPOG: FUNCTION TGuiInterface.dll+00019b80
TGUIDeserialisable::PropertySet* TGUIStyle::GetPropertyInfoSet() const
{
	return ms_oPropertySet;
}

// $NicktoonsPS2: FUNCTION 003698f8
// $JPOG: FUNCTION TGuiInterface.dll+00019b70
TGUIDeserialisable::PropertySet* TGUIStyle::GetPropertyInfoOrderedSet() const
{
	return ms_oPropertySetOrdered;
}

// NOTE: JPOG also takes the interface
// $NicktoonsPS2: FUNCTION 0036baa0
// $JPOG: FUNCTION TGuiInterface.dll+00017020
void TGUIStyle::Create( const TGUIWidgetInfo* a_pWidgetInfo, const TPString8& a_rcName )
{
	m_pWidgetInfo     = a_pWidgetInfo;
	m_strName         = a_rcName;
	m_pRootComponents = new TGUIStyleRootComponent[ m_pWidgetInfo->GetComponentCount() ];
}

// NOTE: JPOG gets the manager from the interface of the style
// $NicktoonsPS2: FUNCTION 0036bb88
// $JPOG: FUNCTION TGuiInterface.dll+000170d0
TBOOL TGUIStyle::Deserialise( const PProperties* a_pProperties )
{
	TGUIManager* pManager = TGUIManager::GetSingleton();

	// The widget class
	const PPropertyValue* pWidget = GetRequiredProperty( a_pProperties, str_widget, PPropertyValue::TYPE_TPSTRING8 );

	if ( !pWidget )
		return TFALSE;

	TGUIWidgetInfo* pWidgetInfo = TGUIManager::FindWidgetClass( pWidget->GetTPString8() );

	if ( !pWidgetInfo )
	{
		TGUIManager::Error( GetClass()->GetName(), TString8( "Unknown widget class " ) + pWidget->GetTPString8().GetString() );
		return TFALSE;
	}

	// The name
	const PPropertyValue* pName = GetRequiredProperty( a_pProperties, str_name, PPropertyValue::TYPE_TPSTRING8 );

	if ( !pName )
		return TFALSE;

	TPString8 strName = pName->GetTPString8();

	if ( pManager->FindStyleInstance( strName ) )
	{
		TString8 strError;
		strError.Format( "Duplicate style '%s' defined; ignoring", strName.GetString() );
		TGUIManager::Error( GetClass()->GetName(), strError );

		return TFALSE;
	}

	Create( pWidgetInfo, strName );

	if ( !TGUIDeserialisable::Deserialise( a_pProperties ) )
		return TFALSE;

	// The parameters
	const PPropertyValue* pParams = GetOptionalProperty( a_pProperties, str_param, PPropertyValue::TYPE_ARRAY );

	if ( pParams )
	{
		PPropertyValueArray* pArray = pParams->GetArray();

		for ( TINT i = 0; i < pArray->GetSize(); i++ )
		{
			const PPropertyValue* pParam = pArray->GetValue( i );

			if ( pParam->IsType( PPropertyValue::TYPE_TPSTRING8 ) )
			{
				DeclareParameter( pParam->GetTPString8(), PPropertyValue::TYPE_UNDEF );
			}
			else
			{
				TString8 strError;
				strError.Format( "Style '%s' has item in param list of wrong type", strName.GetString() );
				TGUIManager::Error( GetClass()->GetName(), strError );
			}
		}
	}

	return DeserialiseRootComponents( a_pProperties ) && DeserialiseWidgetProps( a_pProperties );
}

// $NicktoonsPS2: FUNCTION 0036bf90
// $JPOG: FUNCTION TGuiInterface.dll+00018a50
TBOOL TGUIStyle::DeserialiseRootComponents( const PProperties* a_pProperties )
{
	TINT iNumRootComponents = m_pWidgetInfo->GetComponentCount();

	for ( TINT i = 0; i < iNumRootComponents; i++ )
	{
		const PPropertyValue* pRootComponent = GetOptionalProperty( a_pProperties, str_component, m_pWidgetInfo->GetComponentName( i ), PPropertyValue::TYPE_PROPS );

		if ( !pRootComponent )
			continue;

		const PProperties*      pRootProperties = pRootComponent->GetProperties();
		TGUIStyleRootComponent& rRootComponent  = m_pRootComponents[ i ];

		const PPropertyValue* pMargin = GetOptionalProperty( pRootProperties, str_marginleft, PPropertyValue::TYPE_INT );
		if ( pMargin ) rRootComponent.SetMarginLeft( pMargin->GetInteger() );

		pMargin = GetOptionalProperty( pRootProperties, str_marginright, PPropertyValue::TYPE_INT );
		if ( pMargin ) rRootComponent.SetMarginRight( pMargin->GetInteger() );

		pMargin = GetOptionalProperty( pRootProperties, str_margintop, PPropertyValue::TYPE_INT );
		if ( pMargin ) rRootComponent.SetMarginTop( pMargin->GetInteger() );

		pMargin = GetOptionalProperty( pRootProperties, str_marginbottom, PPropertyValue::TYPE_INT );
		if ( pMargin ) rRootComponent.SetMarginBottom( pMargin->GetInteger() );

		if ( !DeserialiseStates( pRootProperties, &rRootComponent ) )
			return TFALSE;
	}

	return TTRUE;
}

// The state variables of the widget class that the state block sets make its rule
// $NicktoonsPS2: FUNCTION 0036c168
// $JPOG: FUNCTION TGuiInterface.dll+00018bb0
TBOOL TGUIStyle::DeserialiseStates( const PProperties* a_pProperties, TGUIStyleRootComponent* a_pRootComponent )
{
	TINT iNumStateVars = m_pWidgetInfo->GetStateVarCount();

	auto&            rList     = const_cast<PProperties*>( a_pProperties )->GetProperties();
	auto             it        = rList.Begin();
	const PProperty* pProperty = a_pProperties->FindProperty( str_state, it );

	while ( pProperty )
	{
		if ( !pProperty->GetValue().IsType( PPropertyValue::TYPE_PROPS ) )
		{
			TString8 strError;
			strError.Format( "TGUIStyle: Incorrect type of 'state' property" );
			TGUIManager::Error( strError );

			return TFALSE;
		}

		const PProperties* pStateProperties = pProperty->GetValue().GetProperties();
		TGUIStateRule      oRule;

		for ( TINT i = 0; i < iNumStateVars; i++ )
		{
			const TGUIStateVarInfo* pStateVar = m_pWidgetInfo->GetStateVarInfo( i );
			const PPropertyValue*   pValue    = GetOptionalProperty( pStateProperties, pStateVar->strName, PPropertyValue::TYPE_BOOL );

			if ( pValue )
			{
				oRule.uiMask |= pStateVar->uiMask;

				if ( pValue->GetBoolean() )
					oRule.uiValue |= pStateVar->uiMask;
			}
		}

		TGUIStyleState* pState = a_pRootComponent->CreateState( oRule );

		if ( !DeserialiseComponents( pStateProperties, pState->GetComponents() ) )
			return TFALSE;

		pProperty = a_pProperties->FindProperty( str_state, it );
	}

	return TTRUE;
}

// NOTE: the component class isn't checked to be a TGUIComponent and the result of its Deserialise is ignored
// $NicktoonsPS2: FUNCTION 0036c308
// $JPOG: FUNCTION TGuiInterface.dll+000184c0
TBOOL TGUIStyle::DeserialiseComponents( const PProperties* a_pProperties, TNodeList<TGUIComponent>& a_rComponents )
{
	auto&            rList     = const_cast<PProperties*>( a_pProperties )->GetProperties();
	auto             it        = rList.Begin();
	const PProperty* pProperty = a_pProperties->FindProperty( str_component, it );

	while ( pProperty )
	{
		if ( !pProperty->GetValue().IsType( PPropertyValue::TYPE_PROPS ) )
		{
			TString8 strError;
			strError.Format( "TGUIStyle: Incorrect type of 'component' property" );
			TGUIManager::Error( strError );

			return TFALSE;
		}

		const PProperties*    pComponentProperties = pProperty->GetValue().GetProperties();
		const PPropertyValue* pType                = pComponentProperties->GetOptionalProperty( str_type, PPropertyValue::TYPE_TPSTRING8 );

		if ( !pType )
		{
			TGUIManager::Error( "TGUIStyle: Unspecified component class!" );
			return TFALSE;
		}

		const TClass* pClass = TClass::Find( pType->GetTPString8().GetString() );

		if ( !pClass )
		{
			TGUIManager::Error( TString8( TString8( "TGUIStyle: Unknown component class '" ) + pType->GetTPString8().GetString() ) + "'" );
			return TFALSE;
		}

		TGUIComponent* pComponent = TSTATICCAST( TGUIComponent, pClass->CreateObject() );
		pComponent->Deserialise( pComponentProperties, this );
		a_rComponents.InsertTail( pComponent );

		pProperty = a_pProperties->FindProperty( str_component, it );
	}

	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 0036c518
// $JPOG: FUNCTION TGuiInterface.dll+00018d40
TBOOL TGUIStyle::DeserialiseWidgetProps( const PProperties* a_pProperties )
{
	const PPropertyValue* pWidgetProps = GetOptionalProperty( a_pProperties, str_widgetprop, PPropertyValue::TYPE_PROPS );

	if ( !pWidgetProps )
		return TTRUE;

	const PProperties* pProperties = pWidgetProps->GetProperties();
	PropertySet*       pSet        = m_pWidgetInfo->GetProperties();

	for ( TINT i = 0; i < pSet->Size(); i++ )
	{
		const TGUIPropertySpec* pSpec = ( *pSet )[ i ];

		if ( pSpec->IsEnumerated() )
		{
			// Enumerated properties are written as the name of the value
			const PPropertyValue* pValue = GetOptionalProperty( pProperties, pSpec->GetName(), PPropertyValue::TYPE_TPSTRING8 );

			if ( pValue )
				SetFixedWidgetProperty( pSpec->GetName(), PPropertyValue( pSpec->GetEnumValue( pValue->GetTPString8() ) ) );
		}
		else
		{
			const PPropertyValue* pValue = GetOptionalProperty( pProperties, pSpec->GetName(), pSpec->GetType() );

			if ( pValue )
				SetFixedWidgetProperty( pSpec->GetName(), *pValue );
		}
	}

	return TTRUE;
}

// NOTE: JPOG gets the manager from the interface of the style
// $NicktoonsPS2: FUNCTION 0036c680
// $JPOG: FUNCTION TGuiInterface.dll+00017490
TBOOL TGUIStyle::Serialise( PProperties* a_pProperties )
{
	if ( !TGUIDeserialisable::Serialise( a_pProperties ) )
		return TFALSE;

	a_pProperties->PutProperty( str_name, PPropertyValue( m_strName ) );
	a_pProperties->PutProperty( str_widget, PPropertyValue( m_pWidgetInfo->GetWidgetName() ) );

	if ( m_uiNumParameters != 0 )
	{
		PPropertyValueArray* pArray = new PPropertyValueArray( m_uiNumParameters );

		for ( TINT i = 0; i < m_uiNumParameters; i++ )
			pArray->Push( PPropertyValue( m_pParameters[ i ].GetName() ) );

		a_pProperties->PutProperty( str_param, PPropertyValue( pArray ) );
	}

	return SerialiseRootComponents( a_pProperties ) && SerialiseWidgetProps( a_pProperties );
}

// $NicktoonsPS2: FUNCTION 0036ca58
// $JPOG: FUNCTION TGuiInterface.dll+00018e60
TBOOL TGUIStyle::SerialiseRootComponents( PProperties* a_pProperties )
{
	TINT iNumRootComponents = m_pWidgetInfo->GetComponentCount();

	for ( TINT i = 0; i < iNumRootComponents; i++ )
	{
		TGUIStyleRootComponent& rRootComponent = m_pRootComponents[ i ];

		if ( rRootComponent.GetStates().IsEmpty() )
			continue;

		PProperties* pRootProperties = new PProperties();
		a_pProperties->PutProperty( str_component, m_pWidgetInfo->GetComponentName( i ), PPropertyValue( pRootProperties ) );

		if ( rRootComponent.GetMarginLeft() != 0 )
			pRootProperties->PutProperty( str_marginleft, PPropertyValue( rRootComponent.GetMarginLeft() ) );

		if ( rRootComponent.GetMarginRight() != 0 )
			pRootProperties->PutProperty( str_marginright, PPropertyValue( rRootComponent.GetMarginRight() ) );

		if ( rRootComponent.GetMarginTop() != 0 )
			pRootProperties->PutProperty( str_margintop, PPropertyValue( rRootComponent.GetMarginTop() ) );

		if ( rRootComponent.GetMarginBottom() != 0 )
			pRootProperties->PutProperty( str_marginbottom, PPropertyValue( rRootComponent.GetMarginBottom() ) );

		SerialiseStates( pRootProperties, &rRootComponent );
	}

	return TTRUE;
}

// NOTE: PutProperty replaces the previous "state" property, only the last state with components is kept
// $NicktoonsPS2: FUNCTION 0036cc38
// $JPOG: FUNCTION TGuiInterface.dll+00019010
TBOOL TGUIStyle::SerialiseStates( PProperties* a_pProperties, TGUIStyleRootComponent* a_pRootComponent )
{
	TINT iNumStateVars = m_pWidgetInfo->GetStateVarCount();

	for ( auto it = a_pRootComponent->GetStates().Begin(); it != a_pRootComponent->GetStates().End(); it++ )
	{
		if ( it->GetComponents().IsEmpty() )
			continue;

		PProperties* pStateProperties = new PProperties();
		a_pProperties->PutProperty( str_state, PPropertyValue( pStateProperties ) );

		const TGUIStateRule& rcRule = it->GetStateRule();

		for ( TINT i = 0; i < iNumStateVars; i++ )
		{
			const TGUIStateVarInfo* pStateVar = m_pWidgetInfo->GetStateVarInfo( i );

			if ( pStateVar->uiMask & rcRule.uiMask )
				pStateProperties->PutProperty( pStateVar->strName, PPropertyValue( TBOOL( ( pStateVar->uiMask & rcRule.uiValue ) != 0 ) ) );
		}

		SerialiseComponents( pStateProperties, it->GetComponents() );
	}

	return TTRUE;
}

// NOTE: PutProperty replaces the previous "component" property, only the last component is kept
// $NicktoonsPS2: FUNCTION 0036ce00
// $JPOG: FUNCTION TGuiInterface.dll+000186d0
TBOOL TGUIStyle::SerialiseComponents( PProperties* a_pProperties, TNodeList<TGUIComponent>& a_rComponents )
{
	for ( auto it = a_rComponents.Begin(); it != a_rComponents.End(); it++ )
	{
		PProperties* pComponentProperties = new PProperties();
		a_pProperties->PutProperty( str_component, PPropertyValue( pComponentProperties ) );

		it->Serialise( pComponentProperties, this );
	}

	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 0036cef8
// $JPOG: FUNCTION TGuiInterface.dll+00019180
TBOOL TGUIStyle::SerialiseWidgetProps( PProperties* a_pProperties )
{
	if ( m_oFixedWidgetProperties.IsEmpty() )
		return TTRUE;

	PProperties* pWidgetProperties = new PProperties();
	a_pProperties->PutProperty( str_widgetprop, PPropertyValue( pWidgetProperties ) );

	for ( auto it = m_oFixedWidgetProperties.Begin(); it != m_oFixedWidgetProperties.End(); it++ )
	{
		const TGUIPropertySpec* pSpec = m_pWidgetInfo->GetPropertyInfo( it->oKey );

		if ( pSpec->IsEnumerated() )
			pWidgetProperties->PutProperty( it->oKey, PPropertyValue( pSpec->GetEnumName( it->oValue.GetInteger() ) ) );
		else
			pWidgetProperties->PutProperty( it->oKey, it->oValue );
	}

	return TTRUE;
}

// NOTE: the component class isn't checked to exist
// $JPOG: FUNCTION TGuiInterface.dll+00017920
TGUIComponent* TGUIStyle::CreateComponent( TINT a_iRootComponent, const TPString8& a_rcComponentClass, TGUIStateRule a_oRule )
{
	const TClass*  pClass     = TClass::Find( a_rcComponentClass.GetString(), &TGetClass( TGUIComponent ) );
	TGUIComponent* pComponent = TSTATICCAST( TGUIComponent, pClass->CreateObject() );

	m_pRootComponents[ a_iRootComponent ].CreateState( a_oRule )->AddComponent( pComponent );
	return pComponent;
}

// $JPOG: FUNCTION TGuiInterface.dll+000179a0
TGUIComponent* TGUIStyle::CreateComponent( const TPString8& a_rcRootComponent, const TPString8& a_rcComponentClass, TGUIStateRule a_oRule )
{
	return CreateComponent( m_pWidgetInfo->GetComponentIndex( a_rcRootComponent ), a_rcComponentClass, a_oRule );
}

// $NicktoonsPS2: FUNCTION 0036d080
// $JPOG: FUNCTION TGuiInterface.dll+000179e0
TGUIStyleState* TGUIStyle::GetState( TINT a_iRootComponent, TUINT a_uiState ) const
{
	return m_pRootComponents[ a_iRootComponent ].GetState( a_uiState );
}

// $JPOG: FUNCTION TGuiInterface.dll+00019970
TGUIStyleState* TGUIStyle::GetState( const TPString8& a_rcRootComponent, TUINT a_uiState ) const
{
	return GetState( m_pWidgetInfo->GetComponentIndex( a_rcRootComponent ), a_uiState );
}

// The components get no maximum size
// $JPOG: FUNCTION TGuiInterface.dll+00017a30
TBOOL TGUIStyle::GetComponentPreferredSize( TINT a_iRootComponent, PPropertyValue* a_pParameters, TINT& a_rWidth, TINT& a_rHeight )
{
	a_rWidth  = 0;
	a_rHeight = 0;

	TGUIStyleRootComponent& rRootComponent = m_pRootComponents[ a_iRootComponent ];

	for ( auto itState = rRootComponent.GetStates().Begin(); itState != rRootComponent.GetStates().End(); itState++ )
	{
		for ( auto it = itState->GetComponents().Begin(); it != itState->GetComponents().End(); it++ )
		{
			TINT iWidth;
			TINT iHeight;
			it->GetPreferredSize( a_pParameters, iWidth, iHeight, -1, -1 );

			a_rWidth  = TMath::Max( a_rWidth, iWidth );
			a_rHeight = TMath::Max( a_rHeight, iHeight );
		}
	}

	a_rWidth += rRootComponent.GetMarginLeft() + rRootComponent.GetMarginRight();
	a_rHeight += rRootComponent.GetMarginTop() + rRootComponent.GetMarginBottom();

	return TTRUE;
}

// a_rbAllKnown is cleared if a component can't give its size
// $JPOG: FUNCTION TGuiInterface.dll+00017b40
TBOOL TGUIStyle::GetComponentPreferredSize( TINT a_iRootComponent, PPropertyValue* a_pParameters, TINT& a_rWidth, TINT& a_rHeight, TINT16 a_iMaxWidth, TINT16 a_iMaxHeight, TBOOL& a_rbAllKnown )
{
	a_rWidth     = 0;
	a_rHeight    = 0;
	a_rbAllKnown = TTRUE;

	TGUIStyleRootComponent& rRootComponent = m_pRootComponents[ a_iRootComponent ];

	for ( auto itState = rRootComponent.GetStates().Begin(); itState != rRootComponent.GetStates().End(); itState++ )
	{
		for ( auto it = itState->GetComponents().Begin(); it != itState->GetComponents().End(); it++ )
		{
			TINT iWidth;
			TINT iHeight;
			a_rbAllKnown &= it->GetPreferredSize( a_pParameters, iWidth, iHeight, a_iMaxWidth, a_iMaxHeight );

			a_rWidth  = TMath::Max( a_rWidth, iWidth );
			a_rHeight = TMath::Max( a_rHeight, iHeight );
		}
	}

	a_rWidth += rRootComponent.GetMarginLeft() + rRootComponent.GetMarginRight();
	a_rHeight += rRootComponent.GetMarginTop() + rRootComponent.GetMarginBottom();

	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 0036d868
// $JPOG: FUNCTION TGuiInterface.dll+000183a0
void TGUIStyle::Paint( TGUIDisplayContext* a_pDisplayContext, TGUIWidget* a_pWidget, TUINT a_uiState, PPropertyValue* a_pParameters, TGUIComponentCache* a_pCaches )
{
	TINT iNumRootComponents = m_pWidgetInfo->GetComponentCount();

	for ( TINT i = 0; i < iNumRootComponents; i++ )
	{
		TGUIStyleState* pState = GetState( i, a_uiState );

		if ( !pState )
			continue;

		TGUIStyleRootComponent& rRootComponent = m_pRootComponents[ i ];
		const TGUIRectangle&    rcBBox         = a_pCaches[ i ].GetBBox();

		TGUIRectangle oRect(
		    rcBBox.m_iX + rRootComponent.GetMarginLeft(),
		    rcBBox.m_iY + rRootComponent.GetMarginTop(),
		    rcBBox.m_iWidth - ( rRootComponent.GetMarginLeft() + rRootComponent.GetMarginRight() ),
		    rcBBox.m_iHeight - ( rRootComponent.GetMarginTop() + rRootComponent.GetMarginBottom() )
		);

		for ( auto it = pState->GetComponents().Begin(); it != pState->GetComponents().End(); it++ )
			it->Paint( a_pDisplayContext, a_pParameters, oRect, &a_pCaches[ i ] );
	}
}

// $NicktoonsPS2: FUNCTION 0036d718
// $JPOG: FUNCTION TGuiInterface.dll+00018280
TBOOL TGUIStyle::HitTest( TGUIWidget* a_pWidget, TINT a_iX, TINT a_iY, TUINT a_uiState, PPropertyValue* a_pParameters, TGUIComponentCache* a_pCaches )
{
	TINT iNumRootComponents = m_pWidgetInfo->GetComponentCount();

	for ( TINT i = 0; i < iNumRootComponents; i++ )
	{
		if ( !GetState( i, a_uiState ) )
			continue;

		TGUIStyleRootComponent& rRootComponent = m_pRootComponents[ i ];
		const TGUIRectangle&    rcBBox         = a_pCaches[ i ].GetBBox();

		TINT iLeft = rcBBox.m_iX + rRootComponent.GetMarginLeft();
		TINT iTop  = rcBBox.m_iY + rRootComponent.GetMarginTop();

		if ( a_iX >= iLeft && a_iY >= iTop &&
		     a_iX < iLeft + rcBBox.m_iWidth - ( rRootComponent.GetMarginLeft() + rRootComponent.GetMarginRight() ) &&
		     a_iY < iTop + rcBBox.m_iHeight - ( rRootComponent.GetMarginTop() + rRootComponent.GetMarginBottom() ) )
		{
			return TTRUE;
		}
	}

	return TFALSE;
}

// $NicktoonsPS2: FUNCTION 0036d118
// $JPOG: FUNCTION TGuiInterface.dll+00017c60
TINT TGUIStyle::DeclareParameter( const TPString8& a_rcName, const TClass* a_pType )
{
	TINT iIndex = FindParameter( a_rcName );

	if ( iIndex == -1 )
	{
		if ( !SetParameterCount( m_uiNumParameters + 1 ) )
			return -1;

		iIndex                            = m_uiNumParameters - 1;
		m_pParameters[ iIndex ].m_strName = a_rcName;
		m_pParameters[ iIndex ].m_pSpec->SetName( a_rcName );
	}

	m_pParameters[ iIndex ].m_pType = a_pType;
	m_pParameters[ iIndex ].m_pSpec->SetType( a_pType );

	UpdateDependentWidgets();
	return iIndex;
}

// $NicktoonsPS2: FUNCTION 0036d268
// $JPOG: FUNCTION TGuiInterface.dll+00017d90
TINT TGUIStyle::FindParameter( const TPString8& a_rcName ) const
{
	for ( TINT i = 0; i < m_uiNumParameters; i++ )
	{
		if ( m_pParameters[ i ].m_strName == a_rcName )
			return i;
	}

	return -1;
}

// The parameters are copied to a new array, there can be MAX_PARAMETERS of them
// $NicktoonsPS2: FUNCTION 0036d330
// $JPOG: FUNCTION TGuiInterface.dll+00017df0
TBOOL TGUIStyle::SetParameterCount( TINT a_iCount )
{
	if ( a_iCount == m_uiNumParameters )
		return TTRUE;

	if ( TUINT( a_iCount ) > MAX_PARAMETERS )
		return TFALSE;

	TGUIStyleParameterInfo* pOldParameters = m_pParameters;

	if ( a_iCount != 0 )
	{
		m_pParameters = new TGUIStyleParameterInfo[ a_iCount ];

		TINT iNumCopied = TMath::Min( TINT( m_uiNumParameters ), a_iCount );
		TINT i          = 0;

		for ( ; i < iNumCopied; i++ )
		{
			m_pParameters[ i ]          = pOldParameters[ i ];
			m_pParameters[ i ].m_iIndex = i;
		}

		for ( ; i < a_iCount; i++ )
			m_pParameters[ i ].m_iIndex = i;
	}
	else
	{
		m_pParameters = TNULL;
	}

	if ( pOldParameters )
		delete[] pOldParameters;

	m_uiNumParameters = TUINT8( a_iCount );
	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 0036d2c0
// $JPOG: FUNCTION TGuiInterface.dll+00019950
const TGUIStyleParameterInfo* TGUIStyle::GetParameterInfo( TINT a_iIndex ) const
{
	return &m_pParameters[ a_iIndex ];
}

// $NicktoonsPS2: FUNCTION 0036d2d8
// $JPOG: FUNCTION TGuiInterface.dll+00019920
const TGUIStyleParameterInfo* TGUIStyle::GetParameterInfo( const TPString8& a_rcName ) const
{
	TINT iIndex = FindParameter( a_rcName );
	return ( iIndex == -1 ) ? TNULL : &m_pParameters[ iIndex ];
}

// Same widget class and same parameters (the types must match where both are known)
// $JPOG: FUNCTION TGuiInterface.dll+000177d0
TBOOL TGUIStyle::IsCompatibleWith( TGUIStyle* a_pStyle )
{
	if ( m_pWidgetInfo != a_pStyle->m_pWidgetInfo )
		return TFALSE;

	if ( m_uiNumParameters != a_pStyle->m_uiNumParameters )
		return TFALSE;

	for ( TINT i = 0; i < m_uiNumParameters; i++ )
	{
		const TGUIStyleParameterInfo* pParameter      = GetParameterInfo( i );
		const TGUIStyleParameterInfo* pOtherParameter = a_pStyle->GetParameterInfo( i );

		if ( pParameter->m_strName != pOtherParameter->m_strName )
			return TFALSE;

		if ( pParameter->m_pType != PPropertyValue::TYPE_UNDEF && pOtherParameter->m_pType != PPropertyValue::TYPE_UNDEF && pParameter->m_pType != pOtherParameter->m_pType )
			return TFALSE;
	}

	return TTRUE;
}

// $JPOG: FUNCTION TGuiInterface.dll+00017f60
TBOOL TGUIStyle::IsReferencingTextureSection( const TGUITextureSection* a_pTextureSection ) const
{
	for ( TINT i = 0; i < m_pWidgetInfo->GetComponentCount(); i++ )
	{
		TGUIStyleRootComponent& rRootComponent = m_pRootComponents[ i ];

		for ( auto itState = rRootComponent.GetStates().Begin(); itState != rRootComponent.GetStates().End(); itState++ )
		{
			for ( auto it = itState->GetComponents().Begin(); it != itState->GetComponents().End(); it++ )
			{
				if ( it->IsReferencingTextureSection( a_pTextureSection ) )
					return TTRUE;
			}
		}
	}

	return TFALSE;
}

// $JPOG: FUNCTION TGuiInterface.dll+00018020
TBOOL TGUIStyle::IsReferencingFont( const TGUIFont* a_pFont ) const
{
	for ( TINT i = 0; i < m_pWidgetInfo->GetComponentCount(); i++ )
	{
		TGUIStyleRootComponent& rRootComponent = m_pRootComponents[ i ];

		for ( auto itState = rRootComponent.GetStates().Begin(); itState != rRootComponent.GetStates().End(); itState++ )
		{
			for ( auto it = itState->GetComponents().Begin(); it != itState->GetComponents().End(); it++ )
			{
				if ( it->IsReferencingFont( a_pFont ) )
					return TTRUE;
			}
		}
	}

	return TFALSE;
}

// $NicktoonsPS2: FUNCTION 0036d510
// $JPOG: FUNCTION TGuiInterface.dll+000180e0
void TGUIStyle::SetFixedWidgetProperty( const TPString8& a_rcName, const PPropertyValue& a_rcValue )
{
	FixedWidgetProperty* pProperty = FindFixedWidgetPropertyPair( a_rcName );

	if ( pProperty )
		pProperty->oValue = a_rcValue;
	else
		m_oFixedWidgetProperties.InsertTail( new FixedWidgetProperty( a_rcName, a_rcValue ) );

	UpdateDependentWidgets();
}

// $JPOG: FUNCTION TGuiInterface.dll+000181e0
PPropertyValue TGUIStyle::GetFixedWidgetProperty( const TPString8& a_rcName ) const
{
	FixedWidgetProperty* pProperty = FindFixedWidgetPropertyPair( a_rcName );
	return pProperty ? pProperty->oValue : PPropertyValue();
}

// NOTE: the widgets that use the style aren't updated
// $JPOG: FUNCTION TGuiInterface.dll+000198a0
void TGUIStyle::ReleaseFixedWidgetProperty( const TPString8& a_rcName )
{
	FixedWidgetProperty* pProperty = FindFixedWidgetPropertyPair( a_rcName );

	if ( pProperty )
		m_oFixedWidgetProperties.Delete( pProperty );
}

// $JPOG: FUNCTION TGuiInterface.dll+000198f0
TBOOL TGUIStyle::IsWidgetPropertyFixed( const TPString8& a_rcName ) const
{
	return FindFixedWidgetPropertyPair( a_rcName ) != TNULL;
}

// $NicktoonsPS2: FUNCTION 0036d6c0
// $JPOG: FUNCTION TGuiInterface.dll+000189b0
TGUIStyle::FixedWidgetProperty* TGUIStyle::FindFixedWidgetPropertyPair( const TPString8& a_rcName ) const
{
	for ( auto it = m_oFixedWidgetProperties.Begin(); it != m_oFixedWidgetProperties.End(); it++ )
	{
		if ( it->oKey == a_rcName )
			return it;
	}

	return TNULL;
}

// $JPOG: FUNCTION TGuiInterface.dll+000178d0
TGUIWidget* TGUIStyle::CreateWidget()
{
	TGUIWidget* pWidget = TSTATICCAST( TGUIWidget, m_pWidgetInfo->GetTClass()->CreateObject() );
	pWidget->Create( this, TPString8() );

	return pWidget;
}

// $NicktoonsPS2: FUNCTION 0036d650
// $JPOG: FUNCTION TGuiInterface.dll+00018220
void TGUIStyle::ApplyFixedWidgetProperties( TGUIDeserialisable* a_pWidget )
{
	for ( auto it = m_oFixedWidgetProperties.Begin(); it != m_oFixedWidgetProperties.End(); it++ )
		a_pWidget->SetProperty( it->oKey, it->oValue );
}

// $NicktoonsPS2: FUNCTION 0036d9f0
// $JPOG: FUNCTION TGuiInterface.dll+000189e0
void TGUIStyle::UpdateDependentWidgets()
{
	TNodeList<TGUIManager::TopLevelInfo>* pList = TGUIManager::GetSingleton()->FindStyleReferences( this );

	if ( !pList )
		return;

	for ( auto it = pList->Begin(); it != pList->End(); it++ )
	{
		it->pWidget->NotifyStyleChanged();
		ApplyFixedWidgetProperties( it->pWidget );
	}

	pList->DeleteAll();
	delete pList;
}

// $JPOG: FUNCTION TGuiInterface.dll+00006710
TBOOL TGUIStyle::IsWidgetStyle( const TPString8& a_rcWidgetName ) const
{
	return m_pWidgetInfo->GetWidgetName() == a_rcWidgetName;
}

// $NicktoonsPS2: FUNCTION 0036db08
// $JPOG: FUNCTION TGuiInterface.dll+00019af0
void TGUIStyle::Set_name_property( const PPropertyValue& a_rcValue )
{
	m_strName = a_rcValue.GetTPString8();
}

// $NicktoonsPS2: FUNCTION 0036db90
// $JPOG: FUNCTION TGuiInterface.dll+00019ad0
PPropertyValue TGUIStyle::Get_name_property() const
{
	return PPropertyValue( m_strName );
}

// $NicktoonsPS2: FUNCTION 0036dc90
// $JPOG: FUNCTION TGuiInterface.dll+00019a20
void TGUIStyle::Set_marginleft_property( const PPropertyValue& a_rcValue )
{
	m_iMarginLeft = TINT16( a_rcValue.GetInteger() );
}

// $NicktoonsPS2: FUNCTION 0036dcc0
// $JPOG: FUNCTION TGuiInterface.dll+00019a00
PPropertyValue TGUIStyle::Get_marginleft_property() const
{
	return PPropertyValue( TINT( m_iMarginLeft ) );
}

// $NicktoonsPS2: FUNCTION 0036dcf8
// $JPOG: FUNCTION TGuiInterface.dll+000199e0
void TGUIStyle::Set_marginright_property( const PPropertyValue& a_rcValue )
{
	m_iMarginRight = TINT16( a_rcValue.GetInteger() );
}

// $NicktoonsPS2: FUNCTION 0036dd28
// $JPOG: FUNCTION TGuiInterface.dll+000199c0
PPropertyValue TGUIStyle::Get_marginright_property() const
{
	return PPropertyValue( TINT( m_iMarginRight ) );
}

// $NicktoonsPS2: FUNCTION 0036dbc0
// $JPOG: FUNCTION TGuiInterface.dll+00019aa0
void TGUIStyle::Set_margintop_property( const PPropertyValue& a_rcValue )
{
	m_iMarginTop = TINT16( a_rcValue.GetInteger() );
}

// $NicktoonsPS2: FUNCTION 0036dbf0
// $JPOG: FUNCTION TGuiInterface.dll+00019a80
PPropertyValue TGUIStyle::Get_margintop_property() const
{
	return PPropertyValue( TINT( m_iMarginTop ) );
}

// $NicktoonsPS2: FUNCTION 0036dc28
// $JPOG: FUNCTION TGuiInterface.dll+00019a60
void TGUIStyle::Set_marginbottom_property( const PPropertyValue& a_rcValue )
{
	m_iMarginBottom = TINT16( a_rcValue.GetInteger() );
}

// $NicktoonsPS2: FUNCTION 0036dc58
// $JPOG: FUNCTION TGuiInterface.dll+00019a40
PPropertyValue TGUIStyle::Get_marginbottom_property() const
{
	return PPropertyValue( TINT( m_iMarginBottom ) );
}

TOSHI_NAMESPACE_END
