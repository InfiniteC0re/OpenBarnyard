#include "ToshiPCH.h"
#include "TGUIWidget.h"
#include "TGUIWindow.h"
#include "TGUIScreen.h"
#include "TGUIManager.h"
#include "TGUIStyle.h"
#include "TGUIStyleParameterInfo.h"
#include "TGUIWidgetInfo.h"
#include "TGUIComponentCache.h"
#include "TGUILayoutManager.h"
#include "TGUIInputContext.h"
#include "TGUIDisplayContext.h"
#include "TGUIPropertySpecSimple.h"
#include "TGUIStateVarSpec.h"
#include "TGUITextureSection.h"

#include <Plugins/PPropertyParser/PProperties.h>
#include <Plugins/PPropertyParser/PProperty.h>
#include <Plugins/PPropertyParser/PPropertyValue.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: CreateObject returns TNULL in Nicktoons (0036e3d8), TGUIWidget is abstract
TDEFINE_CLASS_INIT_NORUNTIME( TGUIWidget );

TGUIDeserialisable::PropertySet* TGUIWidget::ms_oPropertySet        = TNULL;
TGUIDeserialisable::PropertySet* TGUIWidget::ms_oPropertySetOrdered = TNULL;
TGUIWidgetInfo*                  TGUIWidget::ms_pWidgetInfo         = TNULL;
TINT                             TGUIWidget::ms_iNumInstances       = 0;

TPString8 TGUIWidget::str_widget;
TPString8 TGUIWidget::str_name;
TPString8 TGUIWidget::str_style;
TPString8 TGUIWidget::str_type;
TPString8 TGUIWidget::str_layout;
TPString8 TGUIWidget::str_width;
TPString8 TGUIWidget::str_height;
TPString8 TGUIWidget::str_xpos;
TPString8 TGUIWidget::str_ypos;
TPString8 TGUIWidget::str_animsync;
TPString8 TGUIWidget::str_alpha;

TPString8 TGUIWidget::ms_saDirectionName[ NeighbourDirection_NUMOF ];

const TCHAR* const TGUIWidget::ms_csaSizeModeNameStr[ SizeMode_NUMOF ]            = { "fixed", "preferred", "free" };
const TCHAR* const TGUIWidget::ms_csaPlaceModeNameStr[ PlaceMode_NUMOF ]          = { "fixed", "managed" };
const TCHAR* const TGUIWidget::ms_csaDirectionNameStr[ NeighbourDirection_NUMOF ] = { "North", "East", "South", "West", "Previous", "Next" };
const TCHAR* const TGUIWidget::ms_csaOrientationNameStr[ Orientation_NUMOF ]       = { "horizontal", "vertical" };
const TCHAR* const TGUIWidget::ms_csaPlacementNameStr[ Placement_NUMOF ]           = { "before", "after" };

// The ancestors of the two widgets of FindCommonAncestor (used by WalkUpDownTree)
// NOTE: two stacks of 32 widgets in the original (JPOG: 0x10047f00 and 0x10047fa0)
static TGUIWidget* s_apAncestors1[ 32 ];
static TINT        s_iNumAncestors1 = 0;
static TGUIWidget* s_apAncestors2[ 32 ];
static TINT        s_iNumAncestors2 = 0;

// The parameters and the component caches are allocated with TGUINew
// NOTE: inlined array new/delete of the original (Nicktoons 003767a8, 00376880, 00376618 and 003766d0)
template <class T>
static T* NewGUIArray( TINT a_iCount )
{
	TINT* pMem = TSTATICCAST( TINT, TGUINew( sizeof( TINT ) * 2 + sizeof( T ) * a_iCount ) );
	pMem[ 0 ]  = a_iCount;

	T* pArray = TREINTERPRETCAST( T*, pMem + 2 );

	for ( TINT i = 0; i < a_iCount; i++ )
		new ( &pArray[ i ] ) T();

	return pArray;
}

template <class T>
static void DeleteGUIArray( T* a_pArray )
{
	if ( !a_pArray )
		return;

	TINT* pMem   = TREINTERPRETCAST( TINT*, a_pArray ) - 2;
	TINT  iCount = pMem[ 0 ];

	for ( TINT i = iCount - 1; i >= 0; i-- )
		a_pArray[ i ].~T();

	TGUIDelete( pMem );
}

// $NicktoonsPS2: FUNCTION 0036f788
// $JPOG: FUNCTION TGuiInterface.dll+0001b510
void TGUIWidget::InitialiseClass()
{
	str_widget   = TPString8( "widget" );
	str_name     = TPString8( "name" );
	str_style    = TPString8( "style" );
	str_type     = TPString8( "type" );
	str_layout   = TPString8( "layout" );
	str_width    = TPString8( "width" );
	str_height   = TPString8( "height" );
	str_xpos     = TPString8( "xpos" );
	str_ypos     = TPString8( "ypos" );
	str_animsync = TPString8( "animsync" );
	str_alpha    = TPString8( "alpha" );

	for ( TINT i = 0; i < NeighbourDirection_NUMOF; i++ )
		ms_saDirectionName[ i ] = TPString8( ms_csaDirectionNameStr[ i ] );

	ms_pWidgetInfo = new TGUIWidgetInfo();

	ms_oPropertySet        = new PropertySet( 4, 0 );
	ms_oPropertySetOrdered = new PropertySet( 4, 0 );

	using t_Setter = TGUIPropertySpecSimple::t_Setter;
	using t_Getter = TGUIPropertySpecSimple::t_Getter;

	TINT      iSortHint = 0;
	TPString8 strCategory( "Design" );

	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( str_name, static_cast<t_Setter>( &TGUIWidget::Set_name_property ), static_cast<t_Getter>( &TGUIWidget::Get_name_property ), PPropertyValue::TYPE_TPSTRING8, strCategory, TFALSE, &TGetClass( TGUIWidget ), iSortHint++ ) );

	strCategory = TPString8( "Layout" );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "sizemode_horiz" ), static_cast<t_Setter>( &TGUIWidget::Set_sizemode_horiz_property ), static_cast<t_Getter>( &TGUIWidget::Get_sizemode_horiz_property ), PPropertyValue::TYPE_INT, strCategory, TTRUE, &TGetClass( TGUIWidget ), iSortHint++, SizeMode_NUMOF, ms_csaSizeModeNameStr ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "sizemode_vert" ), static_cast<t_Setter>( &TGUIWidget::Set_sizemode_vert_property ), static_cast<t_Getter>( &TGUIWidget::Get_sizemode_vert_property ), PPropertyValue::TYPE_INT, strCategory, TTRUE, &TGetClass( TGUIWidget ), iSortHint++, SizeMode_NUMOF, ms_csaSizeModeNameStr ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "placemode" ), static_cast<t_Setter>( &TGUIWidget::Set_placemode_property ), static_cast<t_Getter>( &TGUIWidget::Get_placemode_property ), PPropertyValue::TYPE_INT, strCategory, TTRUE, &TGetClass( TGUIWidget ), iSortHint++, PlaceMode_NUMOF, ms_csaPlaceModeNameStr ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( str_xpos, static_cast<t_Setter>( &TGUIWidget::Set_xpos_property ), static_cast<t_Getter>( &TGUIWidget::Get_xpos_property ), PPropertyValue::TYPE_INT, strCategory, TFALSE, &TGetClass( TGUIWidget ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( str_ypos, static_cast<t_Setter>( &TGUIWidget::Set_ypos_property ), static_cast<t_Getter>( &TGUIWidget::Get_ypos_property ), PPropertyValue::TYPE_INT, strCategory, TFALSE, &TGetClass( TGUIWidget ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( str_width, static_cast<t_Setter>( &TGUIWidget::Set_width_property ), static_cast<t_Getter>( &TGUIWidget::Get_width_property ), PPropertyValue::TYPE_INT, strCategory, TFALSE, &TGetClass( TGUIWidget ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( str_height, static_cast<t_Setter>( &TGUIWidget::Set_height_property ), static_cast<t_Getter>( &TGUIWidget::Get_height_property ), PPropertyValue::TYPE_INT, strCategory, TFALSE, &TGetClass( TGUIWidget ), iSortHint++ ) );

	strCategory = TPString8( "Appearance" );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( str_animsync, static_cast<t_Setter>( &TGUIWidget::Set_animsync_property ), static_cast<t_Getter>( &TGUIWidget::Get_animsync_property ), PPropertyValue::TYPE_FLOAT, strCategory, TFALSE, &TGetClass( TGUIWidget ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( str_alpha, static_cast<t_Setter>( &TGUIWidget::Set_alpha_property ), static_cast<t_Getter>( &TGUIWidget::Get_alpha_property ), PPropertyValue::TYPE_FLOAT, strCategory, TFALSE, &TGetClass( TGUIWidget ), iSortHint++ ) );

	strCategory = TPString8( "State" );
	DeclareStateVar( &TGetClass( TGUIWidget ), "Enabled", State_Enabled, strCategory, iSortHint, ms_pWidgetInfo, ms_oPropertySet, ms_oPropertySetOrdered );
	DeclareStateVar( &TGetClass( TGUIWidget ), "Visible", State_Visible, strCategory, iSortHint, ms_pWidgetInfo, ms_oPropertySet, ms_oPropertySetOrdered );
	DeclareStateVar( &TGetClass( TGUIWidget ), "Focused", State_Focused, strCategory, iSortHint, ms_pWidgetInfo, ms_oPropertySet, ms_oPropertySetOrdered );
	DeclareStateVar( &TGetClass( TGUIWidget ), "Rollover", State_Rollover, strCategory, iSortHint, ms_pWidgetInfo, ms_oPropertySet, ms_oPropertySetOrdered );
	DeclareStateVar( &TGetClass( TGUIWidget ), "Selected", State_Selected, strCategory, iSortHint, ms_pWidgetInfo, ms_oPropertySet, ms_oPropertySetOrdered );

	ms_pWidgetInfo->SetWidgetName( TPString8( "TGUIWidget" ) );
	ms_pWidgetInfo->SetWidgetLabel( TPString8( "Widget" ) );
	ms_pWidgetInfo->SetProperties( ms_oPropertySet, ms_oPropertySetOrdered );
	ms_pWidgetInfo->SetConcrete( TFALSE );

	// NOTE: the original also shrinks the capacity of the component name array of the widget info to its size here

	TGUIManager::AddWidgetClass( ms_pWidgetInfo );
}

// NOTE: Nicktoons destroys the widget info in its static deinitialiser (003769e0)
// $NicktoonsPS2: FUNCTION 00372708
// $JPOG: FUNCTION TGuiInterface.dll+0001c970
void TGUIWidget::DeinitialiseClass()
{
	ms_pWidgetInfo->Clear();
	DestroyPropertySets( ms_oPropertySet, ms_oPropertySetOrdered );

	TGUIMemory::ms_oMemoryPile.Free();

	delete ms_pWidgetInfo;
	ms_pWidgetInfo = TNULL;

	for ( TINT i = NeighbourDirection_NUMOF - 1; i >= 0; i-- )
		ms_saDirectionName[ i ] = TPString8();

	str_alpha    = TPString8();
	str_animsync = TPString8();
	str_ypos     = TPString8();
	str_xpos     = TPString8();
	str_height   = TPString8();
	str_width    = TPString8();
	str_layout   = TPString8();
	str_type     = TPString8();
	str_style    = TPString8();
	str_name     = TPString8();
	str_widget   = TPString8();
}

// $NicktoonsPS2: FUNCTION 0036efd8
// $JPOG: FUNCTION TGuiInterface.dll+0001ca40
TGUIWidget::TGUIWidget()
    : m_oInputEmitter( this )
    , m_oPaintEmitter( this )
    , m_pStyle( TNULL )
    , m_uiNumParameters( 0 )
    , m_pParameters( TNULL )
    , m_pComponentCaches( TNULL )
    , m_pLayoutManager( TNULL )
    , m_pParent( TNULL )
    , m_iXPos( 0 )
    , m_iYPos( 0 )
    , m_iUnknown1( 0 )
    , m_iWidth( 0 )
    , m_iHeight( 0 )
    , m_eSizeModeHoriz( SizeMode_Free )
    , m_eSizeModeVert( SizeMode_Free )
    , m_ePlaceMode( PlaceMode_Managed )
    , m_fAnimSync( 0.0f )
    , m_fAlpha( 1.0f )
    , m_bValidated( TFALSE )
    , m_bLayoutValid( TFALSE )
    , m_bUnknown3( TFALSE )
    , m_uiState( State_Enabled | State_Visible )
{
	ms_iNumInstances++;
}

// The children are cloned, the state slaves keep the same widgets
// $NicktoonsPS2: FUNCTION 0036f0d8
// $JPOG: FUNCTION TGuiInterface.dll+0001f350
TGUIWidget::TGUIWidget( const TGUIWidget& a_rcOther )
    : TGUIDeserialisable( a_rcOther )
    , m_oInputEmitter( this )
    , m_oPaintEmitter( this )
    , m_pStyle( TNULL )
    , m_uiNumParameters( a_rcOther.m_uiNumParameters )
    , m_pParameters( TNULL )
    , m_pComponentCaches( TNULL )
    , m_pLayoutManager( TNULL )
    , m_pParent( TNULL )
    , m_strName( a_rcOther.m_strName )
    , m_iXPos( a_rcOther.m_iXPos )
    , m_iYPos( a_rcOther.m_iYPos )
    , m_iUnknown1( 0 )
    , m_iWidth( a_rcOther.m_iWidth )
    , m_iHeight( a_rcOther.m_iHeight )
    , m_eSizeModeHoriz( a_rcOther.m_eSizeModeHoriz )
    , m_eSizeModeVert( a_rcOther.m_eSizeModeVert )
    , m_ePlaceMode( a_rcOther.m_ePlaceMode )
    , m_fAnimSync( a_rcOther.m_fAnimSync )
    , m_fAlpha( a_rcOther.m_fAlpha )
    , m_bValidated( TFALSE )
    , m_bLayoutValid( TFALSE )
    , m_bUnknown3( TFALSE )
    , m_uiState( a_rcOther.m_uiState )
{
	ms_iNumInstances++;

	if ( a_rcOther.m_pStyle )
		SetStyle( a_rcOther.m_pStyle );

	if ( a_rcOther.m_pLayoutManager )
		SetLayoutManager( a_rcOther.m_pLayoutManager->Clone() );

	if ( a_rcOther.m_pParameters )
	{
		for ( TINT i = 0; i < m_uiNumParameters; i++ )
			m_pParameters[ i ] = a_rcOther.m_pParameters[ i ];
	}

	if ( a_rcOther.m_pComponentCaches )
	{
		for ( TINT i = 0; i < a_rcOther.GetWidgetInfo()->GetComponentCount(); i++ )
			m_pComponentCaches[ i ] = a_rcOther.m_pComponentCaches[ i ];
	}

	TGUIWidget* pOther = const_cast<TGUIWidget*>( &a_rcOther );

	for ( auto it = pOther->m_oStateSlaves.Begin(); it != pOther->m_oStateSlaves.End(); it++ )
		AddStateSlave( it->pSlave, it->uiMask );

	for ( auto it = pOther->m_oActions.Begin(); it != pOther->m_oActions.End(); it++ )
		AddAction( it->GetDoodadID(), it->GetLabel(), 0 );

	for ( auto it = a_rcOther.m_oChildren.Begin(); it != a_rcOther.m_oChildren.End(); it++ )
		AddWidgetPrivate( it->Clone() );
}

// $NicktoonsPS2: FUNCTION 0036f3d8
// $JPOG: FUNCTION TGuiInterface.dll+0001cb00
TGUIWidget::~TGUIWidget()
{
	ms_iNumInstances--;

	if ( m_pParent )
		m_pParent->RemoveWidget( this );

	while ( !m_oStateSlaves.IsEmpty() )
	{
		StateSlaveInfo* pSlave = m_oStateSlaves.Head();
		pSlave->Remove();
		delete pSlave;
	}

	while ( !m_oActions.IsEmpty() )
	{
		TGUIWidgetAction* pAction = m_oActions.Head();
		pAction->Remove();
		delete pAction;
	}

	DeleteAllWidgets();

	if ( m_pParameters )
	{
		if ( m_pStyle )
		{
			for ( TINT i = 0; i < m_pStyle->GetParameterCount(); i++ )
				m_pParameters[ i ].SetUndef();
		}

		DeleteGUIArray( m_pParameters );
	}

	InvalidateComponentCache( TFALSE );

	if ( m_pComponentCaches )
		DeleteGUIArray( m_pComponentCaches );

	if ( m_pLayoutManager )
		delete m_pLayoutManager;
}

//-----------------------------------------------------------------------------
// TGUIDeserialisable
//-----------------------------------------------------------------------------

// NOTE: the state variables are properties (read by TGUIDeserialisable::Deserialise), DeserialiseState isn't called
// $NicktoonsPS2: FUNCTION 00372880
// $JPOG: FUNCTION TGuiInterface.dll+0001cd40
TBOOL TGUIWidget::Deserialise( const PProperties* a_pProperties )
{
	TGUIManager* pManager = TGUIManager::GetSingleton();
	TGUIStyle*   pStyle   = TNULL;
	TPString8    strName;

	if ( const PPropertyValue* pValue = GetOptionalProperty( a_pProperties, str_width, PPropertyValue::TYPE_INT ) )
		m_iWidth = TINT16( pValue->GetInteger() );

	if ( const PPropertyValue* pValue = GetOptionalProperty( a_pProperties, str_height, PPropertyValue::TYPE_INT ) )
		m_iHeight = TINT16( pValue->GetInteger() );

	if ( const PPropertyValue* pValue = GetOptionalProperty( a_pProperties, str_xpos, PPropertyValue::TYPE_INT ) )
		m_iXPos = TINT16( pValue->GetInteger() );

	if ( const PPropertyValue* pValue = GetOptionalProperty( a_pProperties, str_ypos, PPropertyValue::TYPE_INT ) )
		m_iYPos = TINT16( pValue->GetInteger() );

	if ( const PPropertyValue* pValue = GetOptionalProperty( a_pProperties, str_animsync, PPropertyValue::TYPE_FLOAT ) )
		m_fAnimSync = pValue->GetFloat();

	if ( const PPropertyValue* pValue = GetOptionalProperty( a_pProperties, str_alpha, PPropertyValue::TYPE_FLOAT ) )
		m_fAlpha = pValue->GetFloat();

	if ( const PPropertyValue* pValue = a_pProperties->GetProperty( str_style ) )
	{
		if ( !pValue->IsType( PPropertyValue::TYPE_TPSTRING8 ) )
		{
			TGUIManager::Error( "TGUIWidget: Incorrect type of 'style' property" );
			return TFALSE;
		}

		pStyle = pManager->FindStyleInstance( pValue->GetTPString8() );
	}

	if ( const PPropertyValue* pValue = a_pProperties->GetProperty( str_name ) )
	{
		if ( !pValue->IsType( PPropertyValue::TYPE_TPSTRING8 ) )
		{
			TGUIManager::Error( "TGUIWidget: Incorrect type of 'name' property" );
			return TFALSE;
		}

		strName = pValue->GetTPString8();
	}

	if ( !TGUIDeserialisable::Deserialise( a_pProperties ) )
		return TFALSE;

	SetStyle( pStyle );
	m_strName = strName;

	if ( !DeserialiseParameters( a_pProperties ) )
		return TFALSE;

	if ( !DeserialiseChildren( a_pProperties ) )
		return TFALSE;

	if ( const PPropertyValue* pValue = a_pProperties->GetProperty( str_layout ) )
	{
		if ( !pValue->IsType( PPropertyValue::TYPE_PROPS ) )
		{
			TGUIManager::Error( "TGUIWidget: Incorrect type of 'layout' property" );
			return TFALSE;
		}

		// NOTE: the previous layout manager isn't deleted (as in the original)
		m_pLayoutManager = TSTATICCAST( TGUILayoutManager, TGUIDeserialisable::Create( pValue->GetProperties() ) );

		if ( !m_pLayoutManager )
			return TFALSE;

		m_pLayoutManager->Deserialise( pValue->GetProperties() );
	}
	else
	{
		m_pLayoutManager = TNULL;
	}

	return TTRUE;
}

// NOTE: the state variables are written by TGUIDeserialisable::Serialise, SerialiseState isn't called
// $NicktoonsPS2: FUNCTION 003732e0
// $JPOG: FUNCTION TGuiInterface.dll+0001d120
TBOOL TGUIWidget::Serialise( PProperties* a_pProperties )
{
	TGUIDeserialisable::Serialise( a_pProperties );

	a_pProperties->PutProperty( str_name, PPropertyValue( m_strName ) );

	if ( m_eSizeModeHoriz == SizeMode_Fixed )
		a_pProperties->PutProperty( str_width, PPropertyValue( TINT( m_iWidth ) ) );

	if ( m_eSizeModeVert == SizeMode_Fixed )
		a_pProperties->PutProperty( str_height, PPropertyValue( TINT( m_iHeight ) ) );

	// The position of the widgets placed by the layout manager of their parent isn't saved
	if ( m_ePlaceMode == PlaceMode_Fixed || !m_pParent || !m_pParent->m_pLayoutManager )
	{
		a_pProperties->PutProperty( str_xpos, PPropertyValue( TINT( m_iXPos ) ) );
		a_pProperties->PutProperty( str_ypos, PPropertyValue( TINT( m_iYPos ) ) );
	}

	if ( m_fAnimSync != 0.0f )
		a_pProperties->PutProperty( str_animsync, PPropertyValue( m_fAnimSync ) );

	if ( m_fAlpha != 1.0f )
		a_pProperties->PutProperty( str_alpha, PPropertyValue( m_fAlpha ) );

	if ( m_pStyle )
		a_pProperties->PutProperty( str_style, PPropertyValue( m_pStyle->GetName() ) );

	if ( m_pLayoutManager )
	{
		PProperties* pLayoutProperties = new PProperties();
		a_pProperties->PutProperty( str_layout, PPropertyValue( pLayoutProperties ) );
		m_pLayoutManager->Serialise( pLayoutProperties );
	}

	return SerialiseParameters( a_pProperties ) && SerialiseChildren( a_pProperties );
}

// $NicktoonsPS2: FUNCTION 00373fb0
// $JPOG: FUNCTION TGuiInterface.dll+00020e10
TINT TGUIWidget::GetPropertyCount() const
{
	if ( m_pStyle )
		return GetWidgetInfo()->GetProperties()->Size() + m_pStyle->GetParameterCount();

	return GetWidgetInfo()->GetProperties()->Size();
}

// The properties of the class, then the parameters of the style
// $NicktoonsPS2: FUNCTION 00373cf0
// $JPOG: FUNCTION TGuiInterface.dll+0001d3d0
const TGUIPropertySpec* TGUIWidget::FindPropertyInfo( const TPString8& a_rcName ) const
{
	const TGUIPropertySpec* pSpec = TGUIDeserialisable::FindPropertyInfo( a_rcName );

	if ( pSpec )
		return pSpec;

	if ( m_pStyle )
	{
		const TGUIStyleParameterInfo* pParameter = m_pStyle->GetParameterInfo( a_rcName );
		return pParameter ? pParameter->GetSpec() : TNULL;
	}

	return TNULL;
}

// $NicktoonsPS2: FUNCTION 00374018
// $JPOG: FUNCTION TGuiInterface.dll+0001d3a0
const TGUIPropertySpec* TGUIWidget::GetPropertyInfo( TINT a_iIndex ) const
{
	PropertySet* pProperties = GetWidgetInfo()->GetProperties();

	if ( a_iIndex < pProperties->Size() )
		return ( *pProperties )[ a_iIndex ];

	return m_pStyle->GetParameterInfo( a_iIndex - pProperties->Size() )->GetSpec();
}

// The properties of the classes of the widget and of the style parameters are valid
// $NicktoonsPS2: FUNCTION 00373d48
// $JPOG: FUNCTION TGuiInterface.dll+0001d420
TBOOL TGUIWidget::ValidatePropertyInfo( const TGUIPropertySpec* a_pSpec ) const
{
	if ( !a_pSpec )
		return TFALSE;

	return GetClass()->IsA( a_pSpec->GetSourceClass() ) || a_pSpec->GetSourceClass()->IsA( &TGetClass( TGUIStyle ) );
}

// $NicktoonsPS2: FUNCTION 0036e3e8
// $JPOG: FUNCTION TGuiInterface.dll+00020e40
TGUIDeserialisable::PropertySet* TGUIWidget::GetPropertyInfoSet() const
{
	return ms_oPropertySet;
}

// $NicktoonsPS2: FUNCTION 0036e3f0
// $JPOG: FUNCTION TGuiInterface.dll+00020e30
TGUIDeserialisable::PropertySet* TGUIWidget::GetPropertyInfoOrderedSet() const
{
	return ms_oPropertySetOrdered;
}

//-----------------------------------------------------------------------------
// TGUIWidget virtuals
//-----------------------------------------------------------------------------

// NOTE: not virtual in JPOG (the widget info is a field of the widget there)
// $NicktoonsPS2: FUNCTION 00376bb8
// $JPOG: FUNCTION TGuiInterface.dll+00007800
const TGUIWidgetInfo* TGUIWidget::GetWidgetInfo() const
{
	return ms_pWidgetInfo;
}

// $NicktoonsPS2: FUNCTION 0036f688
void TGUIWidget::ResetParameters()
{
	for ( auto it = m_oChildren.Begin(); it != m_oChildren.End(); it++ )
	{
		if ( it->m_pStyle )
		{
			for ( TINT i = 0; i < it->m_pStyle->GetParameterCount(); i++ )
				it->m_pParameters[ i ].SetUndef();
		}
	}

	if ( m_pStyle )
	{
		for ( TINT i = 0; i < m_pStyle->GetParameterCount(); i++ )
			m_pParameters[ i ].SetUndef();
	}

	InvalidateComponentCache( TTRUE );
}

// $NicktoonsPS2: FUNCTION 00372860
void TGUIWidget::Unknown_0x40()
{
	m_bUnknown3 = TTRUE;
	Unknown_0x40Children();
}

// $NicktoonsPS2: FUNCTION 00375320
void TGUIWidget::Unknown_0x40Children()
{
	for ( auto it = m_oChildren.Begin(); it != m_oChildren.End(); it++ )
		it->Unknown_0x40();
}

// $NicktoonsPS2: FUNCTION 00376bc8
// $JPOG: FUNCTION TGuiInterface.dll+00007410
TBOOL TGUIWidget::ManagesChildren() const
{
	return TFALSE;
}

// $NicktoonsPS2: FUNCTION 00376bd0
// $JPOG: FUNCTION TGuiInterface.dll+00007400
TBOOL TGUIWidget::AllowsChildren() const
{
	return TFALSE;
}

// $NicktoonsPS2: FUNCTION 00376bd8
// $JPOG: FUNCTION TGuiInterface.dll+000073f0
TBOOL TGUIWidget::AcceptsFocus() const
{
	return TFALSE;
}

// NOTE: Nicktoons checks if a_pWidget is enabled and visible, but asks the parent in both cases
// $NicktoonsPS2: FUNCTION 00374540
// $JPOG: FUNCTION TGuiInterface.dll+00020840
TGUIWidget* TGUIWidget::AllowsFocus( TGUIWidget* a_pWidget )
{
	if ( m_pParent )
		return m_pParent->AllowsFocus( a_pWidget );

	return TNULL;
}

// $NicktoonsPS2: FUNCTION 00374610
// $JPOG: FUNCTION TGuiInterface.dll+00020800
void TGUIWidget::NotifyDescendantAddition( TGUIWidget* a_pWidget )
{
	if ( m_pParent )
		m_pParent->NotifyDescendantAddition( a_pWidget );
}

// $NicktoonsPS2: FUNCTION 00374640
// $JPOG: FUNCTION TGuiInterface.dll+000207e0
void TGUIWidget::NotifyDescendantRemoval( TGUIWidget* a_pWidget )
{
	if ( m_pParent )
		m_pParent->NotifyDescendantRemoval( a_pWidget );
}

// $NicktoonsPS2: FUNCTION 00374670
// $JPOG: FUNCTION TGuiInterface.dll+000207c0
void TGUIWidget::NotifyDescendantRemoved( TGUIWidget* a_pWidget, TGUIWidget* a_pOldParent )
{
	if ( m_pParent )
		m_pParent->NotifyDescendantRemoved( a_pWidget, a_pOldParent );
}

// $NicktoonsPS2: FUNCTION 003746a0
// $JPOG: FUNCTION TGuiInterface.dll+0001e660
void TGUIWidget::NotifyFocusLost( TGUIWidget* a_pNewFocus, TGUIWidget* a_pOldFocus )
{
	GetScreen()->ReleaseFocus( this );
}

// $NicktoonsPS2: FUNCTION 003746d0
// $JPOG: FUNCTION TGuiInterface.dll+0001e680
void TGUIWidget::NotifyFocusGained( TGUIWidget* a_pNewFocus, TGUIWidget* a_pOldFocus )
{
}

// $NicktoonsPS2: FUNCTION 003748d0
// $JPOG: FUNCTION TGuiInterface.dll+000207b0
void TGUIWidget::NotifySlaveStateChanged( TGUIWidget* a_pMaster, TUINT a_uiChangedMask )
{
}

// The input emitter gets the event first, the action of the doodad is fired then
// NOTE: JPOG checks that the label of the action isn't empty, Nicktoons that it's set
// $NicktoonsPS2: FUNCTION 003748d8
// $JPOG: FUNCTION TGuiInterface.dll+0001e7a0
TBOOL TGUIWidget::OnButtonPress( TInputInterface::InputEvent* a_pEvent, TINT a_iActions )
{
	TGUIWidgetInputEvent oEvent;
	oEvent.pEvent   = a_pEvent;
	oEvent.bHandled = TFALSE;
	m_oInputEmitter.Throw( &oEvent );

	if ( oEvent.bHandled )
		return TTRUE;

	for ( auto it = m_oActions.Begin(); it != m_oActions.End(); it++ )
	{
		if ( it->GetDoodadID() != a_pEvent->GetDoodad() )
			continue;

		if ( it->IsEnabled() && it->GetLabel() )
		{
			GetScreen()->GetInputContext()->FireWidgetAction( this, it );
			it->Throw( this );
		}

		return TTRUE;
	}

	return TFALSE;
}

// $NicktoonsPS2: FUNCTION 003749b0
// $JPOG: FUNCTION TGuiInterface.dll+0001e8c0
TBOOL TGUIWidget::OnButtonRepeat( TInputInterface::InputEvent* a_pEvent, TINT a_iActions )
{
	TGUIWidgetInputEvent oEvent;
	oEvent.pEvent   = a_pEvent;
	oEvent.bHandled = TFALSE;
	m_oInputEmitter.Throw( &oEvent );

	return oEvent.bHandled;
}

// $NicktoonsPS2: FUNCTION 003749e0
// $JPOG: FUNCTION TGuiInterface.dll+0001e900
TBOOL TGUIWidget::OnButtonRelease( TInputInterface::InputEvent* a_pEvent, TINT a_iActions )
{
	TGUIWidgetInputEvent oEvent;
	oEvent.pEvent   = a_pEvent;
	oEvent.bHandled = TFALSE;
	m_oInputEmitter.Throw( &oEvent );

	return oEvent.bHandled;
}

// $NicktoonsPS2: FUNCTION 00374a10
// $JPOG: FUNCTION TGuiInterface.dll+0001e940
TBOOL TGUIWidget::OnButtonHit( TInputInterface::InputEvent* a_pEvent, TINT a_iActions )
{
	TGUIWidgetInputEvent oEvent;
	oEvent.pEvent   = a_pEvent;
	oEvent.bHandled = TFALSE;
	m_oInputEmitter.Throw( &oEvent );

	return oEvent.bHandled;
}

// $NicktoonsPS2: FUNCTION 00374a40
// $JPOG: FUNCTION TGuiInterface.dll+0001e980
TBOOL TGUIWidget::OnAxisMove( TInputInterface::InputEvent* a_pEvent, TINT a_iActions )
{
	TGUIWidgetInputEvent oEvent;
	oEvent.pEvent   = a_pEvent;
	oEvent.bHandled = TFALSE;
	m_oInputEmitter.Throw( &oEvent );

	return oEvent.bHandled;
}

// $NicktoonsPS2: FUNCTION 00374a70
// $JPOG: FUNCTION TGuiInterface.dll+000207a0
TBOOL TGUIWidget::OnPointerMove( TINT a_iX, TINT a_iY )
{
	return TFALSE;
}

// $NicktoonsPS2: FUNCTION 00374a78
// $JPOG: FUNCTION TGuiInterface.dll+00020780
TBOOL TGUIWidget::OnPointerEnter()
{
	SetState( State_Rollover, State_Rollover );
	return TFALSE;
}

// $NicktoonsPS2: FUNCTION 00374aa0
// $JPOG: FUNCTION TGuiInterface.dll+00020770
TBOOL TGUIWidget::OnPointerExit()
{
	SetState( State_Rollover, 0u );
	return TFALSE;
}

// $NicktoonsPS2: FUNCTION 00374d08
// $JPOG: FUNCTION TGuiInterface.dll+0001e9c0
TBOOL TGUIWidget::OnPaint( TGUIDisplayContext* a_pDisplayContext )
{
	if ( m_pStyle )
		m_pStyle->Paint( a_pDisplayContext, this, m_uiState, m_pParameters, m_pComponentCaches );

	TGUIWidgetPaintEvent oEvent;
	oEvent.pDisplayContext = a_pDisplayContext;
	m_oPaintEmitter.Throw( &oEvent );

	return TFALSE;
}

// $NicktoonsPS2: FUNCTION 00376be0
// $JPOG: FUNCTION TGuiInterface.dll+000073d0
void TGUIWidget::Pack()
{
}

// $NicktoonsPS2: FUNCTION 00372c80
// $JPOG: FUNCTION TGuiInterface.dll+0001f5b0
TBOOL TGUIWidget::DeserialiseState( const PProperties* a_pProperties )
{
	m_uiState = State_Enabled | State_Visible;

	for ( TINT i = 0; i < GetWidgetInfo()->GetStateVarCount(); i++ )
	{
		const TGUIStateVarInfo* pStateVar = GetWidgetInfo()->GetStateVarInfo( i );
		const PPropertyValue*   pValue    = a_pProperties->GetProperty( pStateVar->strName );

		if ( !pValue )
			continue;

		if ( !pValue->IsType( PPropertyValue::TYPE_BOOL ) )
		{
			TGUIManager::Error( TString8::VarArgs( "TGUIWidget: Incorrect type of state variable '%s'", pStateVar->strName.GetString() ) );
			return TFALSE;
		}

		if ( pValue->GetBoolean() == TTRUE )
			m_uiState |= pStateVar->uiMask;
		else
			m_uiState &= ~pStateVar->uiMask;
	}

	return TTRUE;
}

// Reads the parameters of the style ("NAME = value;"): the texture sections are given by name, the strings
// are copied to wide strings
// NOTE: JPOG makes temporary localised strings of the strings
// $NicktoonsPS2: FUNCTION 00372dc8
// $JPOG: FUNCTION TGuiInterface.dll+0001f6e0
TBOOL TGUIWidget::DeserialiseParameters( const PProperties* a_pProperties )
{
	if ( !m_pStyle )
		return TTRUE;

	TGUIManager* pManager = TGUIManager::GetSingleton();
	m_uiNumParameters     = TUINT8( m_pStyle->GetParameterCount() );

	for ( TINT i = 0; i < m_pStyle->GetParameterCount(); i++ )
	{
		const TGUIStyleParameterInfo* pParameter = m_pStyle->GetParameterInfo( i );
		const PPropertyValue*         pValue     = a_pProperties->GetProperty( pParameter->GetName() );

		if ( !pValue )
			continue;

		if ( pValue->IsType( PPropertyValue::TYPE_TPSTRING8 ) && pParameter->GetType() == &TGetClass( TGUITextureSection ) )
		{
			TGUITextureSection* pTextureSection = pManager->FindTextureSection( pValue->GetTPString8() );

			if ( !pTextureSection )
			{
				TGUIManager::Error( TString8( "TGUIWidget: Cannot find texture section " ) + pValue->GetTPString8().GetString() + " for style parameter '" + pParameter->GetName().GetString() + "'" );
				return TFALSE;
			}

			m_pParameters[ i ] = PPropertyValue( pTextureSection );
			continue;
		}

		if ( pValue->IsType( PPropertyValue::TYPE_TPSTRING8 ) && pParameter->GetType() == PPropertyValue::TYPE_STRING16 )
		{
			const TPString8& rcString = pValue->GetTPString8();
			TINT             iLength  = TINT( rcString.GetString8().Length() );

			TWCHAR* wszString = TSTATICCAST( TWCHAR, TMalloc( ( iLength + 1 ) * sizeof( TWCHAR ) ) );
			TStringManager::StringCharToUnicode( wszString, rcString.GetString(), -1 );

			m_pParameters[ i ].SetString16A( wszString );
			continue;
		}

		if ( !pValue->IsType( pParameter->GetType() ) )
		{
			TGUIManager::Error( TString8( "TGUIWidget: Incorrect type of style parameter '" ) + pParameter->GetName().GetString() + "'" );
			return TFALSE;
		}

		m_pParameters[ i ] = *pValue;
	}

	return TTRUE;
}

// Creates the children ("widget" blocks), the style of a child sets its type
// $NicktoonsPS2: FUNCTION 00373108
// $JPOG: FUNCTION TGuiInterface.dll+0001fa30
TBOOL TGUIWidget::DeserialiseChildren( const PProperties* a_pProperties )
{
	TGUIManager* pManager = TGUIManager::GetSingleton();
	auto         it       = const_cast<PProperties*>( a_pProperties )->GetProperties().Begin();

	while ( const PProperty* pProperty = a_pProperties->FindProperty( str_widget, it ) )
	{
		if ( !pProperty->GetValue().IsType( PPropertyValue::TYPE_PROPS ) )
		{
			TGUIManager::Error( "TGUIWidget: Incorrect type of 'widget' property" );
			return TFALSE;
		}

		PProperties* pChildProperties = pProperty->GetValue().GetProperties();

		if ( const PPropertyValue* pStyleName = pChildProperties->GetProperty( str_style ) )
		{
			TGUIStyle* pStyle = pManager->FindStyleInstance( pStyleName->GetTPString8() );

			if ( !pStyle )
			{
				TGUIManager::Error( TString8::VarArgs( "TGUIWidget: Unknown widget style '%s'", pStyleName->GetTPString8().GetString() ) );
				return TFALSE;
			}

			// The type of the widget is the widget class of its style
			const_cast<PPropertyValue*>( pChildProperties->GetProperty( str_type ) )->SetTPString8( pStyle->GetWidgetInfo()->GetWidgetName() );
		}

		TGUIWidget* pChild = TSTATICCAST( TGUIWidget, TGUIDeserialisable::Create( pChildProperties ) );

		if ( !pChild )
			return TFALSE;

		pChild->Deserialise( pChildProperties );
		AddWidgetPrivate( pChild );
		pChild->SetState( State_Focused, 0u );
	}

	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 00373568
// $JPOG: FUNCTION TGuiInterface.dll+0001fc00
TBOOL TGUIWidget::SerialiseState( PProperties* a_pProperties )
{
	for ( TINT i = 0; i < GetWidgetInfo()->GetStateVarCount(); i++ )
	{
		const TGUIStateVarInfo* pStateVar = GetWidgetInfo()->GetStateVarInfo( i );
		a_pProperties->PutProperty( pStateVar->strName, PPropertyValue( TBOOL( ( pStateVar->uiMask & m_uiState ) != 0 ) ) );
	}

	return TTRUE;
}

// The texture sections are written by name
// $NicktoonsPS2: FUNCTION 00373620
// $JPOG: FUNCTION TGuiInterface.dll+0001fc90
TBOOL TGUIWidget::SerialiseParameters( PProperties* a_pProperties )
{
	if ( m_pStyle )
	{
		for ( TINT i = 0; i < m_pStyle->GetParameterCount(); i++ )
		{
			PPropertyValue oValue( m_pParameters[ i ] );

			if ( !oValue.IsEmpty() )
			{
				if ( oValue.GetType() == &TGetClass( TGUITextureSection ) )
					oValue = PPropertyValue( TSTATICCAST( TGUITextureSection, oValue.GetTObject() )->GetName() );

				a_pProperties->PutProperty( m_pStyle->GetParameterInfo( i )->GetName(), oValue );
			}
		}
	}

	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 00373728
// $JPOG: FUNCTION TGuiInterface.dll+0001fd80
TBOOL TGUIWidget::SerialiseChildren( PProperties* a_pProperties )
{
	for ( auto it = m_oChildren.Begin(); it != m_oChildren.End(); it++ )
	{
		PProperties* pChildProperties = new PProperties();
		a_pProperties->PutProperty( str_widget, PPropertyValue( pChildProperties ) );
		it->Serialise( pChildProperties );
	}

	return TTRUE;
}

// The state slaves are removed when the widget changes parent
// $NicktoonsPS2: FUNCTION 00373908
// $JPOG: FUNCTION TGuiInterface.dll+0001fe20
void TGUIWidget::NotifyParentChange( TGUIWidget* a_pParent )
{
	while ( !m_oStateSlaves.IsEmpty() )
	{
		StateSlaveInfo* pSlave = m_oStateSlaves.Head();
		pSlave->Remove();
		delete pSlave;
	}

	if ( a_pParent )
		a_pParent->NotifyDescendantAddition( this );
}

//-----------------------------------------------------------------------------
// Style and parameters
//-----------------------------------------------------------------------------

// $JPOG: FUNCTION TGuiInterface.dll+00020dc0
void TGUIWidget::Create( TGUIStyle* a_pStyle, const TPString8& a_rcName )
{
	SetStyle( a_pStyle );
	m_strName = a_rcName;
}

// $NicktoonsPS2: FUNCTION 00373b20
// $JPOG: FUNCTION TGuiInterface.dll+0001d480
void TGUIWidget::SetStyle( TGUIStyle* a_pStyle )
{
	InvalidateComponentCache( TFALSE );

	if ( a_pStyle )
	{
		if ( m_pStyle )
		{
			// NOTE: the parameters and the caches are kept, NotifyStyleChanged updates them
			InvalidateComponentCache( TFALSE );
			m_pStyle = a_pStyle;
			a_pStyle->ApplyFixedWidgetProperties( this );
			return;
		}

		m_pStyle = a_pStyle;

		if ( m_pParameters )
			DeleteGUIArray( m_pParameters );

		m_uiNumParameters = TUINT8( m_pStyle->GetParameterCount() );

		if ( m_pStyle->GetParameterCount() != 0 )
		{
			m_pParameters = NewGUIArray<PPropertyValue>( m_pStyle->GetParameterCount() );
			InitParameters( 0, m_pStyle->GetParameterCount() );
		}
		else
		{
			m_pParameters = TNULL;
		}

		if ( m_pComponentCaches )
			DeleteGUIArray( m_pComponentCaches );

		m_pComponentCaches = NewGUIArray<TGUIComponentCache>( a_pStyle->GetWidgetInfo()->GetComponentCount() );
		m_pStyle->ApplyFixedWidgetProperties( this );
	}
	else
	{
		m_pStyle = TNULL;

		if ( m_pParameters )
		{
			DeleteGUIArray( m_pParameters );
			m_pParameters = TNULL;
		}

		if ( m_pComponentCaches )
		{
			DeleteGUIArray( m_pComponentCaches );
			m_pComponentCaches = TNULL;
		}
	}
}

// Recreates the parameters for the new parameters of the style, the values that still have the right type are kept
// $NicktoonsPS2: FUNCTION 003755f8
// $JPOG: FUNCTION TGuiInterface.dll+0001f180
void TGUIWidget::NotifyStyleChanged()
{
	TUINT8 uiNumParameters = TUINT8( m_pStyle->GetParameterCount() );

	if ( uiNumParameters != 0 )
	{
		if ( m_uiNumParameters != uiNumParameters )
		{
			PPropertyValue* pParameters = NewGUIArray<PPropertyValue>( uiNumParameters );
			TINT            i           = 0;

			for ( ; i < TMath::Min( uiNumParameters, m_uiNumParameters ); i++ )
			{
				if ( m_pParameters[ i ].CanBeType( m_pStyle->GetParameterInfo( i )->GetType() ) )
					pParameters[ i ] = m_pParameters[ i ];
				else
					InitParameters( i, 1 );
			}

			DeleteGUIArray( m_pParameters );
			m_pParameters     = pParameters;
			m_uiNumParameters = uiNumParameters;

			if ( i < uiNumParameters )
				InitParameters( i, uiNumParameters );
		}
	}
	else if ( m_pParameters )
	{
		// NOTE: the parameters are freed but the pointer and the count are kept (as in the original)
		DeleteGUIArray( m_pParameters );
	}

	InvalidateComponentCache( TFALSE );
}

// $NicktoonsPS2: FUNCTION 00373dc0
// $JPOG: FUNCTION TGuiInterface.dll+0001d5e0
void TGUIWidget::SetParameter( TINT a_iIndex, const TWCHAR* a_wszString )
{
	m_pStyle->GetParameterInfo( a_iIndex );
	m_pParameters[ a_iIndex ].SetString16( const_cast<TWCHAR*>( a_wszString ) );
	InvalidateComponentCache( TFALSE );
}

// $NicktoonsPS2: FUNCTION 00373e20
// $JPOG: FUNCTION TGuiInterface.dll+0001d690
void TGUIWidget::SetParameter( TINT a_iIndex, TGUITextureSection* a_pTextureSection )
{
	m_pStyle->GetParameterInfo( a_iIndex );
	m_pParameters[ a_iIndex ].SetTObject( a_pTextureSection );
	InvalidateComponentCache( TFALSE );
}

// $NicktoonsPS2: FUNCTION 00373e80
// $JPOG: FUNCTION TGuiInterface.dll+0001d6d0
void TGUIWidget::SetParameter( TINT a_iIndex, const PPropertyValue& a_rcValue )
{
	if ( m_pStyle->GetParameterInfo( a_iIndex )->GetType() == PPropertyValue::TYPE_UNDEF )
	{
		m_pParameters[ a_iIndex ] = a_rcValue;
	}
	else
	{
		m_pParameters[ a_iIndex ] = a_rcValue;
		InvalidateComponentCache( TFALSE );
	}
}

// $NicktoonsPS2: FUNCTION 00373f08
// $JPOG: FUNCTION TGuiInterface.dll+00020d90
PPropertyValue TGUIWidget::GetParameter( TINT a_iIndex ) const
{
	m_pStyle->GetParameterInfo( a_iIndex );
	return m_pParameters[ a_iIndex ];
}

// NOTE: JPOG sets an empty localised string to the strings, the wide strings are set to TNULL here
// $NicktoonsPS2: FUNCTION 00373c40
// $JPOG: FUNCTION TGuiInterface.dll+000201f0
void TGUIWidget::InitParameters( TINT a_iFirst, TINT a_iLast )
{
	for ( TINT i = a_iFirst; i < a_iLast; i++ )
	{
		const TClass* pType = m_pStyle->GetParameterInfo( i )->GetType();

		if ( pType == &TGetClass( TGUITextureSection ) )
			m_pParameters[ i ].SetTObject( TNULL );
		else if ( pType == PPropertyValue::TYPE_STRING16 )
			m_pParameters[ i ].SetString16( TNULL );
	}
}

// $JPOG: FUNCTION TGuiInterface.dll+0001f2c0
TBOOL TGUIWidget::IsReferencingTextureSection( const TGUITextureSection* a_pTextureSection ) const
{
	if ( !m_pStyle )
		return TFALSE;

	for ( TINT i = 0; i < m_pStyle->GetParameterCount(); i++ )
	{
		if ( m_pParameters[ i ].GetType() == &TGetClass( TGUITextureSection ) && m_pParameters[ i ].GetTObject() == a_pTextureSection )
			return TTRUE;
	}

	return TFALSE;
}

// $NicktoonsPS2: FUNCTION 00375248
// $JPOG: FUNCTION TGuiInterface.dll+0001ee10
void TGUIWidget::InvalidateComponentCache( TBOOL a_bRecurse )
{
	if ( m_pStyle )
	{
		for ( TINT i = 0; i < GetWidgetInfo()->GetComponentCount(); i++ )
			m_pComponentCaches[ i ].Invalidate();
	}

	if ( a_bRecurse == TTRUE )
	{
		for ( auto it = m_oChildren.Begin(); it != m_oChildren.End(); it++ )
			it->InvalidateComponentCache( TTRUE );
	}
}

// $JPOG: FUNCTION TGuiInterface.dll+0001fe80
void TGUIWidget::SetComponentBBox( TINT a_iComponent, TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight )
{
	TGUIComponentCache& rCache = m_pComponentCaches[ a_iComponent ];

	if ( a_iWidth != rCache.GetBBox().m_iWidth || a_iHeight != rCache.GetBBox().m_iHeight )
		rCache.Invalidate();

	rCache.ModifyBBox( TNULL ) = TGUIRectangle( a_iX, a_iY, a_iWidth, a_iHeight );
}

// $JPOG: FUNCTION TGuiInterface.dll+0001fee0
void TGUIWidget::GetComponentBBox( TINT a_iComponent, TINT& a_rX, TINT& a_rY, TINT& a_rWidth, TINT& a_rHeight )
{
	const TGUIRectangle& rcBBox = m_pComponentCaches[ a_iComponent ].GetBBox();

	a_rX      = rcBBox.m_iX;
	a_rY      = rcBBox.m_iY;
	a_rWidth  = rcBBox.m_iWidth;
	a_rHeight = rcBBox.m_iHeight;
}

// The bounding box without the margins of the root component
// $JPOG: FUNCTION TGuiInterface.dll+0001ff20
void TGUIWidget::GetComponentInteriorBBox( TINT a_iComponent, TINT& a_rX, TINT& a_rY, TINT& a_rWidth, TINT& a_rHeight )
{
	GetComponentBBox( a_iComponent, a_rX, a_rY, a_rWidth, a_rHeight );

	TGUIStyleRootComponent& rRootComponent = m_pStyle->GetRootComponent( a_iComponent );
	a_rX += rRootComponent.GetMarginLeft();
	a_rY += rRootComponent.GetMarginTop();
	a_rWidth -= rRootComponent.GetMarginRight() + rRootComponent.GetMarginLeft();
	a_rHeight -= rRootComponent.GetMarginBottom() + rRootComponent.GetMarginTop();
}

//-----------------------------------------------------------------------------
// Children
//-----------------------------------------------------------------------------

// $NicktoonsPS2: FUNCTION 00373800
// $JPOG: FUNCTION TGuiInterface.dll+0001d760
void TGUIWidget::AddWidget( TGUIWidget* a_pWidget, TBOOL a_bAtHead )
{
	if ( a_bAtHead == TTRUE )
		m_oChildren.InsertHead( a_pWidget );
	else
		m_oChildren.InsertTail( a_pWidget );

	a_pWidget->m_pParent = this;
	a_pWidget->NotifyParentChange( this );
	a_pWidget->RemoveFocus();
}

// $JPOG: FUNCTION TGuiInterface.dll+0001d7e0
void TGUIWidget::AddWidgetBefore( TGUIWidget* a_pWidget, TGUIWidget* a_pBefore )
{
	m_oChildren.InsertBefore( a_pBefore, a_pWidget );

	a_pWidget->m_pParent = this;
	a_pWidget->NotifyParentChange( this );
	a_pWidget->RemoveFocus();
}

// $NicktoonsPS2: FUNCTION 003738a0
// $JPOG: FUNCTION TGuiInterface.dll+000202b0
void TGUIWidget::AddWidgetPrivate( TGUIWidget* a_pWidget )
{
	m_oChildren.InsertTail( a_pWidget );

	a_pWidget->m_pParent = this;
	a_pWidget->NotifyParentChange( this );
}

// $NicktoonsPS2: FUNCTION 00373998
// $JPOG: FUNCTION TGuiInterface.dll+0001d840
void TGUIWidget::RemoveWidget( TGUIWidget* a_pWidget )
{
	NotifyDescendantRemoval( a_pWidget );

	TGUIWidget* pOldParent = a_pWidget->m_pParent;
	m_oChildren.Remove( a_pWidget );

	a_pWidget->m_pParent = TNULL;
	a_pWidget->NotifyParentChange( TNULL );

	NotifyDescendantRemoved( a_pWidget, pOldParent );
}

// $NicktoonsPS2: FUNCTION 00373a40
// $JPOG: FUNCTION TGuiInterface.dll+0001d8a0
void TGUIWidget::RemoveAllWidgets()
{
	while ( !m_oChildren.IsEmpty() )
		RemoveWidget( m_oChildren.Head() );
}

// $NicktoonsPS2: FUNCTION 00373aa0
// $JPOG: FUNCTION TGuiInterface.dll+0001d8f0
void TGUIWidget::DeleteAllWidgets()
{
	while ( !m_oChildren.IsEmpty() )
	{
		TGUIWidget* pChild = m_oChildren.Head();
		RemoveWidget( pChild );

		if ( pChild )
			delete pChild;
	}
}

// $JPOG: FUNCTION TGuiInterface.dll+0001e050
TGUIWidget* TGUIWidget::FindIndexedChild( TINT a_iIndex ) const
{
	TINT iIndex = 0;

	for ( auto it = m_oChildren.Begin(); it != m_oChildren.End(); it++ )
	{
		if ( iIndex++ == a_iIndex )
			return it;
	}

	return TNULL;
}

// Searches the descendants (depth first)
// $NicktoonsPS2: FUNCTION 003743b0
// $JPOG: FUNCTION TGuiInterface.dll+0001e090
TGUIWidget* TGUIWidget::FindNamedChild( const TPString8& a_rcName ) const
{
	for ( auto it = m_oChildren.Begin(); it != m_oChildren.End(); it++ )
	{
		if ( it->m_strName == a_rcName )
			return it;

		TGUIWidget* pChild = it->FindNamedChild( a_rcName );

		if ( pChild )
			return pChild;
	}

	return TNULL;
}

// $JPOG: FUNCTION TGuiInterface.dll+0001e0f0
TGUIWidget* TGUIWidget::FindNamedChild( const TCHAR* a_szName ) const
{
	return FindNamedChild( TPString8( a_szName ) );
}

// The first ancestor (or this widget) that manages its children, this widget if there's none
// $JPOG: FUNCTION TGuiInterface.dll+0001e170
TGUIWidget* TGUIWidget::FindChildManager()
{
	for ( TGUIWidget* pWidget = this; pWidget; pWidget = pWidget->m_pParent )
	{
		if ( pWidget->ManagesChildren() )
			return pWidget;
	}

	return this;
}

// $NicktoonsPS2: FUNCTION 00374468
// $JPOG: FUNCTION TGuiInterface.dll+0001e140
TBOOL TGUIWidget::IsDescendantOf( const TGUIWidget* a_pWidget ) const
{
	for ( TGUIWidget* pParent = m_pParent; pParent; pParent = pParent->m_pParent )
	{
		if ( pParent == a_pWidget )
			return TTRUE;
	}

	return TFALSE;
}

// $NicktoonsPS2: FUNCTION 00374438
// $JPOG: FUNCTION TGuiInterface.dll+00020900
TBOOL TGUIWidget::IsAncestorOf( const TGUIWidget* a_pWidget ) const
{
	return a_pWidget && a_pWidget->IsDescendantOf( this );
}

// $NicktoonsPS2: FUNCTION 003742d8
// $JPOG: FUNCTION TGuiInterface.dll+0001dfd0
TGUIWindow* TGUIWidget::GetWindow()
{
	for ( TGUIWidget* pWidget = this; pWidget; pWidget = pWidget->m_pParent )
	{
		if ( pWidget->IsA( &TGetClass( TGUIWindow ) ) )
			return TSTATICCAST( TGUIWindow, pWidget );
	}

	return TNULL;
}

// $NicktoonsPS2: FUNCTION 00374340
// $JPOG: FUNCTION TGuiInterface.dll+0001e010
TGUIScreen* TGUIWidget::GetScreen()
{
	for ( TGUIWidget* pWidget = this; pWidget; pWidget = pWidget->m_pParent )
	{
		if ( pWidget->IsA( &TGetClass( TGUIScreen ) ) )
			return TSTATICCAST( TGUIScreen, pWidget );
	}

	return TNULL;
}

// NOTE: Nicktoons only continues if a_fnFilter returns exactly TTRUE (1)
// $NicktoonsPS2: FUNCTION 00375410
// $JPOG: FUNCTION TGuiInterface.dll+0001d950
void TGUIWidget::RecurseTree( t_RecurseCallback a_fnPre, t_RecurseCallback a_fnPost, t_RecurseCallback a_fnFilter, void* a_pUserData )
{
	if ( !a_fnFilter || a_fnFilter( this, a_pUserData ) == TTRUE )
	{
		if ( a_fnPre )
			a_fnPre( this, a_pUserData );

		RecurseTree2( a_fnPre, a_fnPost, a_fnFilter, a_pUserData );

		if ( a_fnPost )
			a_fnPost( this, a_pUserData );
	}
}

// The next child is read before the callbacks, they can remove the current one
// $NicktoonsPS2: FUNCTION 003754b0
// $JPOG: FUNCTION TGuiInterface.dll+00020120
void TGUIWidget::RecurseTree2( t_RecurseCallback a_fnPre, t_RecurseCallback a_fnPost, t_RecurseCallback a_fnFilter, void* a_pUserData )
{
	if ( a_fnFilter && !a_fnFilter( this, a_pUserData ) )
		return;

	auto it = m_oChildren.Begin();

	while ( it != m_oChildren.End() )
	{
		TGUIWidget* pChild = it;
		it++;

		if ( a_fnPre )
			a_fnPre( pChild, a_pUserData );

		pChild->RecurseTree2( a_fnPre, a_fnPost, a_fnFilter, a_pUserData );

		if ( a_fnPost )
			a_fnPost( pChild, a_pUserData );
	}
}

// Moves a child a_iOffset positions towards the head (negative) or the tail (positive)
// $JPOG: FUNCTION TGuiInterface.dll+0001e1b0
void TGUIWidget::OrderChildMovePosition( TGUIWidget* a_pWidget, TINT a_iOffset )
{
	if ( a_iOffset < 0 )
	{
		if ( a_pWidget == m_oChildren.Head().Get() )
			return;

		TGUIWidget* pPrev = a_pWidget->Prev();
		m_oChildren.Remove( a_pWidget );

		for ( TINT i = a_iOffset + 1; pPrev != m_oChildren.Head().Get() && i < 0; i++ )
			pPrev = pPrev->Prev();

		m_oChildren.InsertBefore( pPrev, a_pWidget );
	}
	else if ( a_iOffset > 0 )
	{
		if ( a_pWidget == m_oChildren.Tail().Get() )
			return;

		TGUIWidget* pNext = a_pWidget->Next();
		m_oChildren.Remove( a_pWidget );

		for ( TINT i = a_iOffset - 1; pNext != m_oChildren.Tail().Get() && i > 0; i-- )
			pNext = pNext->Next();

		m_oChildren.InsertAfter( pNext, a_pWidget );
	}
}

// Moves a child to a position: 0 the head, -1 (or more than the count) the tail, -2 before the tail...
// $JPOG: FUNCTION TGuiInterface.dll+0001e2a0
void TGUIWidget::OrderChildSetPosition( TGUIWidget* a_pWidget, TINT a_iPosition )
{
	m_oChildren.Remove( a_pWidget );

	if ( a_iPosition == 0 )
	{
		m_oChildren.InsertHead( a_pWidget );
	}
	else if ( a_iPosition == -1 || a_iPosition > TINT( m_oChildren.Count() ) )
	{
		m_oChildren.InsertTail( a_pWidget );
	}
	else if ( a_iPosition > 0 )
	{
		TGUIWidget* pNode = m_oChildren.Head();

		for ( TINT i = 1; i < a_iPosition; i++ )
			pNode = pNode->Next();

		m_oChildren.InsertAfter( pNode, a_pWidget );
	}
	else
	{
		TGUIWidget* pNode = m_oChildren.End();

		for ( TINT i = -2; i > a_iPosition; i-- )
			pNode = pNode->Prev();

		m_oChildren.InsertBefore( pNode, a_pWidget );
	}
}

// $JPOG: FUNCTION TGuiInterface.dll+00020860
void TGUIWidget::OrderPosition( TINT a_iPosition )
{
	if ( m_pParent )
		m_pParent->OrderChildSetPosition( this, a_iPosition );
}

// $JPOG: FUNCTION TGuiInterface.dll+00020880
void TGUIWidget::OrderBackward()
{
	if ( m_pParent )
		m_pParent->OrderChildMovePosition( this, 1 );
}

// $JPOG: FUNCTION TGuiInterface.dll+000208a0
void TGUIWidget::OrderForward()
{
	if ( m_pParent )
		m_pParent->OrderChildMovePosition( this, -1 );
}

// $JPOG: FUNCTION TGuiInterface.dll+000208c0
void TGUIWidget::OrderToBack()
{
	if ( m_pParent )
		m_pParent->OrderChildSetPosition( this, 0 );
}

// $JPOG: FUNCTION TGuiInterface.dll+000208e0
void TGUIWidget::OrderToFront()
{
	if ( m_pParent )
		m_pParent->OrderChildSetPosition( this, -1 );
}

// Fills the stacks of the ancestors of the two widgets, the common ancestor is removed from both
// $NicktoonsPS2: FUNCTION 003750e0
// $JPOG: FUNCTION TGuiInterface.dll+0001ef10
TGUIWidget* TGUIWidget::FindCommonAncestor( TGUIWidget* a_pWidget1, TGUIWidget* a_pWidget2 )
{
	s_iNumAncestors1 = 0;
	s_iNumAncestors2 = 0;

	if ( a_pWidget1 == a_pWidget2 )
		return a_pWidget2;

	for ( TGUIWidget* pWidget = a_pWidget1; pWidget; pWidget = pWidget->m_pParent )
		s_apAncestors1[ s_iNumAncestors1++ ] = pWidget;

	for ( TGUIWidget* pWidget = a_pWidget2; pWidget; pWidget = pWidget->m_pParent )
		s_apAncestors2[ s_iNumAncestors2++ ] = pWidget;

	TGUIWidget* pCommonAncestor = TNULL;

	while ( s_iNumAncestors1 != 0 && s_iNumAncestors2 != 0 )
	{
		if ( s_apAncestors1[ s_iNumAncestors1 - 1 ] != s_apAncestors2[ s_iNumAncestors2 - 1 ] )
			return pCommonAncestor;

		pCommonAncestor = s_apAncestors1[ s_iNumAncestors1 - 1 ];
		s_iNumAncestors1--;
		s_iNumAncestors2--;
	}

	return pCommonAncestor;
}

// $NicktoonsPS2: FUNCTION 00375008
// $JPOG: FUNCTION TGuiInterface.dll+0001f090
void TGUIWidget::WalkUpDownTree( TGUIWidget* a_pFrom, TGUIWidget* a_pTo, t_WalkCallback a_fnUp, t_WalkCallback a_fnDown )
{
	if ( a_pFrom == a_pTo )
		return;

	FindCommonAncestor( a_pFrom, a_pTo );

	// Up from a_pFrom
	for ( TINT i = 0; i < s_iNumAncestors1; i++ )
		a_fnUp( s_apAncestors1[ i ] );

	// Down to a_pTo
	while ( s_iNumAncestors2 != 0 )
		a_fnDown( s_apAncestors2[ --s_iNumAncestors2 ] );

	s_iNumAncestors1 = 0;
}

//-----------------------------------------------------------------------------
// Layout
//-----------------------------------------------------------------------------

// $NicktoonsPS2: FUNCTION 00373f68
// $JPOG: FUNCTION TGuiInterface.dll+0001d730
void TGUIWidget::SetLayoutManager( TGUILayoutManager* a_pLayoutManager )
{
	if ( m_pLayoutManager )
		delete m_pLayoutManager;

	m_pLayoutManager = a_pLayoutManager;
}

// $NicktoonsPS2: FUNCTION 00375380
// $JPOG: FUNCTION TGuiInterface.dll+0001ffa0
void TGUIWidget::PackChildren()
{
	if ( m_pLayoutManager )
	{
		m_pLayoutManager->DoPack( this );
		return;
	}

	for ( auto it = m_oChildren.Begin(); it != m_oChildren.End(); it++ )
		it->Pack();
}

// Lays out the children, then sizes the widget for its layout manager (0 if it has none) on the axes that aren't fixed
// $JPOG: FUNCTION TGuiInterface.dll+0001fff0
void TGUIWidget::LayoutChildren( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight )
{
	for ( auto it = m_oChildren.Begin(); it != m_oChildren.End(); it++ )
	{
		it->Layout( a_iMaxWidth, a_iMaxHeight );

		if ( !it->m_bLayoutValid && ( m_eSizeModeHoriz != SizeMode_Fixed || m_eSizeModeVert != SizeMode_Fixed ) )
			m_bLayoutValid = TFALSE;
	}

	if ( m_eSizeModeHoriz == SizeMode_Fixed && m_eSizeModeVert == SizeMode_Fixed )
		return;

	if ( m_pLayoutManager )
	{
		TINT iWidth  = 0;
		TINT iHeight = 0;
		m_pLayoutManager->DoLayout( this, iWidth, iHeight );

		if ( m_eSizeModeHoriz != SizeMode_Fixed )
		{
			if ( m_iWidth != iWidth ) m_bValidated = TFALSE;
			m_iWidth = TINT16( iWidth );
		}

		if ( m_eSizeModeVert != SizeMode_Fixed )
		{
			if ( m_iHeight != iHeight ) m_bValidated = TFALSE;
			m_iHeight = TINT16( iHeight );
		}
	}
	else
	{
		if ( m_eSizeModeHoriz != SizeMode_Fixed )
		{
			if ( m_iWidth != 0 ) m_bValidated = TFALSE;
			m_iWidth = 0;
		}

		if ( m_eSizeModeVert != SizeMode_Fixed )
		{
			if ( m_iHeight != 0 ) m_bValidated = TFALSE;
			m_iHeight = 0;
		}
	}
}

// $NicktoonsPS2: FUNCTION 00374240
// $JPOG: FUNCTION TGuiInterface.dll+0001dd50
void TGUIWidget::SetPosition( TINT a_iX, TINT a_iY )
{
	if ( m_iXPos != a_iX || m_iYPos != a_iY )
	{
		m_iXPos = TINT16( a_iX );
		m_iYPos = TINT16( a_iY );
	}
}

// $JPOG: FUNCTION TGuiInterface.dll+0001ddc0
void TGUIWidget::SetPositionCenter()
{
	SetPosition( ( m_pParent->m_iWidth - m_iWidth ) / 2, ( m_pParent->m_iHeight - m_iHeight ) / 2 );
}

// Only changes the size of the widgets that aren't fixed on any axis
// $NicktoonsPS2: FUNCTION 00374268
// $JPOG: FUNCTION TGuiInterface.dll+0001dd80
void TGUIWidget::SetSize( TINT a_iWidth, TINT a_iHeight )
{
	if ( m_eSizeModeHoriz != SizeMode_Fixed && m_eSizeModeVert != SizeMode_Fixed )
	{
		if ( m_iWidth != a_iWidth || m_iHeight != a_iHeight )
			m_bValidated = TFALSE;

		m_iWidth  = TINT16( a_iWidth );
		m_iHeight = TINT16( a_iHeight );
	}
}

// $JPOG: FUNCTION TGuiInterface.dll+0001de10
void TGUIWidget::SetFixedSize( TINT a_iWidth, TINT a_iHeight )
{
	if ( m_iWidth != a_iWidth || m_iHeight != a_iHeight )
		m_bValidated = TFALSE;

	m_iWidth         = TINT16( a_iWidth );
	m_iHeight        = TINT16( a_iHeight );
	m_eSizeModeHoriz = SizeMode_Fixed;
	m_eSizeModeVert  = SizeMode_Fixed;
}

// $JPOG: FUNCTION TGuiInterface.dll+00020970
void TGUIWidget::SetFixedWidth( TINT a_iWidth )
{
	if ( m_iWidth != a_iWidth )
		m_bValidated = TFALSE;

	m_iWidth         = TINT16( a_iWidth );
	m_eSizeModeHoriz = SizeMode_Fixed;
}

// $JPOG: FUNCTION TGuiInterface.dll+00020940
void TGUIWidget::SetFixedHeight( TINT a_iHeight )
{
	if ( m_iHeight != a_iHeight )
		m_bValidated = TFALSE;

	m_iHeight       = TINT16( a_iHeight );
	m_eSizeModeVert = SizeMode_Fixed;
}

// $JPOG: FUNCTION TGuiInterface.dll+000209d0
void TGUIWidget::SetWidth( TINT a_iWidth )
{
	if ( m_eSizeModeHoriz != SizeMode_Fixed )
	{
		if ( m_iWidth != a_iWidth )
			m_bValidated = TFALSE;

		m_iWidth = TINT16( a_iWidth );
	}
}

// $JPOG: FUNCTION TGuiInterface.dll+000209a0
void TGUIWidget::SetHeight( TINT a_iHeight )
{
	if ( m_eSizeModeVert != SizeMode_Fixed )
	{
		if ( m_iHeight != a_iHeight )
			m_bValidated = TFALSE;

		m_iHeight = TINT16( a_iHeight );
	}
}

// $JPOG: FUNCTION TGuiInterface.dll+0001de50
TINT TGUIWidget::GetInteriorWidth() const
{
	if ( m_pStyle )
		return m_iWidth - m_pStyle->GetMarginLeft() - m_pStyle->GetMarginRight();

	return m_iWidth;
}

// $JPOG: FUNCTION TGuiInterface.dll+0001de70
TINT TGUIWidget::GetInteriorHeight() const
{
	if ( m_pStyle )
		return m_iHeight - m_pStyle->GetMarginTop() - m_pStyle->GetMarginBottom();

	return m_iHeight;
}

// $JPOG: FUNCTION TGuiInterface.dll+0001de90
void TGUIWidget::GetInteriorSize( TINT& a_rWidth, TINT& a_rHeight ) const
{
	if ( m_pStyle )
	{
		a_rWidth  = m_iWidth - m_pStyle->GetMarginLeft() - m_pStyle->GetMarginRight();
		a_rHeight = m_iHeight - m_pStyle->GetMarginTop() - m_pStyle->GetMarginBottom();
	}
	else
	{
		a_rWidth  = m_iWidth;
		a_rHeight = m_iHeight;
	}
}

// $JPOG: FUNCTION TGuiInterface.dll+0001def0
void TGUIWidget::GetMargins( TINT& a_rLeft, TINT& a_rRight, TINT& a_rTop, TINT& a_rBottom )
{
	if ( m_pStyle )
	{
		a_rLeft   = m_pStyle->GetMarginLeft();
		a_rRight  = m_pStyle->GetMarginRight();
		a_rTop    = m_pStyle->GetMarginTop();
		a_rBottom = m_pStyle->GetMarginBottom();
	}
	else
	{
		a_rLeft   = 0;
		a_rRight  = 0;
		a_rTop    = 0;
		a_rBottom = 0;
	}
}

// $JPOG: FUNCTION TGuiInterface.dll+0001df40
TBOOL TGUIWidget::CanResizeHoriz() const
{
	if ( m_eSizeModeHoriz == SizeMode_Fixed )
		return TTRUE;

	if ( m_eSizeModeHoriz != SizeMode_Free )
		return TFALSE;

	return !m_pParent || !m_pParent->m_pLayoutManager;
}

// $JPOG: FUNCTION TGuiInterface.dll+0001df70
TBOOL TGUIWidget::CanResizeVert() const
{
	if ( m_eSizeModeVert == SizeMode_Fixed )
		return TTRUE;

	if ( m_eSizeModeVert != SizeMode_Free )
		return TFALSE;

	return !m_pParent || !m_pParent->m_pLayoutManager;
}

// $JPOG: FUNCTION TGuiInterface.dll+0001dfa0
TBOOL TGUIWidget::CanMove() const
{
	if ( m_ePlaceMode == PlaceMode_Fixed )
		return TTRUE;

	if ( m_ePlaceMode != PlaceMode_Managed )
		return TFALSE;

	return !m_pParent || !m_pParent->m_pLayoutManager;
}

// NOTE: recursive in Nicktoons, a loop in JPOG
// $NicktoonsPS2: FUNCTION 003744d8
// $JPOG: FUNCTION TGuiInterface.dll+0001e3c0
void TGUIWidget::TransformDeviceToWidget( TINT a_iX, TINT a_iY, TINT& a_rX, TINT& a_rY ) const
{
	if ( m_pParent )
	{
		m_pParent->TransformDeviceToWidget( a_iX - m_iXPos, a_iY - m_iYPos, a_rX, a_rY );
		return;
	}

	a_rX = a_iX - m_iXPos;
	a_rY = a_iY - m_iYPos;
}

// $JPOG: FUNCTION TGuiInterface.dll+0001e410
void TGUIWidget::TransformWidgetToDevice( TINT a_iX, TINT a_iY, TINT& a_rX, TINT& a_rY ) const
{
	const TGUIWidget* pWidget = this;

	while ( pWidget->m_pParent )
	{
		a_iY += pWidget->m_iYPos;
		a_iX += pWidget->m_iXPos;
		pWidget = pWidget->m_pParent;
	}

	a_rX = a_iX + pWidget->m_iXPos;
	a_rY = a_iY + pWidget->m_iYPos;
}

//-----------------------------------------------------------------------------
// State, focus, actions
//-----------------------------------------------------------------------------

// The screen is notified when the widget can't be focused anymore (or can be focused again), the state
// slaves get the changed bits of their masks
// NOTE: the notification of a widget that can be focused again does nothing in Nicktoons (empty function 00368ff0)
// $NicktoonsPS2: FUNCTION 003746d8
// $JPOG: FUNCTION TGuiInterface.dll+0001e460
void TGUIWidget::SetState( TUINT a_uiMask, TUINT a_uiValue )
{
	TUINT uiOldState = m_uiState;
	TUINT uiNewState = ( uiOldState & ~a_uiMask ) | ( a_uiValue & a_uiMask );
	TUINT uiChanged  = uiOldState ^ uiNewState;
	m_uiState        = uiNewState;

	const TUINT uiCanFocus = State_Enabled | State_Visible;

	if ( ( uiOldState & uiCanFocus ) == uiCanFocus )
	{
		if ( ( uiNewState & uiCanFocus ) != uiCanFocus )
		{
			if ( TGUIScreen* pScreen = GetScreen() )
				pScreen->NotifyDescendantCantFocus( this );
		}
	}
	else if ( ( uiNewState & uiCanFocus ) == uiCanFocus && CanFocus() )
	{
		if ( TGUIScreen* pScreen = GetScreen() )
			pScreen->NotifyDescendantCanFocus( this );
	}

	if ( uiChanged == 0 )
		return;

	InvalidateComponentCache( TFALSE );

	for ( auto it = m_oStateSlaves.Begin(); it != m_oStateSlaves.End(); it++ )
	{
		TUINT uiSlaveChanged = it->uiMask & uiChanged & ( m_uiState ^ it->pSlave->m_uiState );

		if ( uiSlaveChanged != 0 )
		{
			it->pSlave->SetState( uiSlaveChanged, m_uiState & uiSlaveChanged );
			it->pSlave->NotifySlaveStateChanged( this, uiSlaveChanged );
		}
	}
}

// $NicktoonsPS2: FUNCTION 003745d0
// $JPOG: FUNCTION TGuiInterface.dll+0001e630
TBOOL TGUIWidget::CanFocus()
{
	for ( TGUIWidget* pWidget = this; pWidget; pWidget = pWidget->m_pParent )
	{
		if ( !( pWidget->m_uiState & State_Enabled ) || !( pWidget->m_uiState & State_Visible ) )
			return TFALSE;
	}

	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 00374598
// $JPOG: FUNCTION TGuiInterface.dll+00020820
void TGUIWidget::RequestFocus()
{
	GetScreen()->ChangeWidgetFocus( this, TFALSE );
}

// $NicktoonsPS2: FUNCTION 00375588
// $JPOG: FUNCTION TGuiInterface.dll+000201b0
void TGUIWidget::RemoveFocus()
{
	SetState( State_Focused, 0u );

	for ( auto it = m_oChildren.Begin(); it != m_oChildren.End(); it++ )
		it->RemoveFocus();
}

// $NicktoonsPS2: FUNCTION 00374818
// $JPOG: FUNCTION TGuiInterface.dll+0001e690
void TGUIWidget::AddStateSlave( TGUIWidget* a_pSlave, TUINT a_uiMask )
{
	for ( auto it = m_oStateSlaves.Begin(); it != m_oStateSlaves.End(); it++ )
	{
		if ( it->pSlave == a_pSlave )
		{
			it->uiMask |= a_uiMask;
			return;
		}
	}

	StateSlaveInfo* pSlave = new StateSlaveInfo();
	pSlave->pSlave         = a_pSlave;
	pSlave->uiMask         = a_uiMask;
	m_oStateSlaves.InsertTail( pSlave );
}

// $JPOG: FUNCTION TGuiInterface.dll+0001e710
void TGUIWidget::RemoveStateSlave( TGUIWidget* a_pSlave, TUINT a_uiMask )
{
	for ( auto it = m_oStateSlaves.Begin(); it != m_oStateSlaves.End(); it++ )
	{
		if ( it->pSlave == a_pSlave )
		{
			it->uiMask &= ~a_uiMask;

			if ( it->uiMask == 0 )
			{
				StateSlaveInfo* pSlave = it;
				pSlave->Remove();
				delete pSlave;
			}

			return;
		}
	}
}

// $JPOG: FUNCTION TGuiInterface.dll+0001e770
TUINT TGUIWidget::GetStateSlaveMask( TGUIWidget* a_pSlave )
{
	for ( auto it = m_oStateSlaves.Begin(); it != m_oStateSlaves.End(); it++ )
	{
		if ( it->pSlave == a_pSlave )
			return it->uiMask;
	}

	return 0;
}

// $NicktoonsPS2: FUNCTION 00374f20
// $JPOG: FUNCTION TGuiInterface.dll+0001e560
TGUIWidgetAction* TGUIWidget::AddAction( TINT a_iDoodadID, const TWCHAR* a_wszLabel, TINT a_iCommandID )
{
	for ( auto it = m_oActions.Begin(); it != m_oActions.End(); it++ )
	{
		if ( it->GetDoodadID() == a_iDoodadID )
		{
			it->SetLabel( a_wszLabel );
			return it;
		}
	}

	TGUIWidgetAction* pAction = new TGUIWidgetAction( a_iDoodadID, a_wszLabel, a_iCommandID, TTRUE );
	m_oActions.InsertTail( pAction );

	return pAction;
}

//-----------------------------------------------------------------------------
// Painting and hit tests
//-----------------------------------------------------------------------------

// Paints the visible descendants depth first (the clip rectangle is the rectangle of the widget, the time of the
// display context gets the animation offset of the widget). The painting stops after a widget that returns TTRUE in
// OnPaint, the next widget is returned to continue it later (TNULL if all were painted).
// NOTE: Nicktoons modulates the colour of the widgets with an alpha different from 1
// $NicktoonsPS2: FUNCTION 00374ac8
// $JPOG: FUNCTION TGuiInterface.dll+0001ecd0
TGUIWidget* TGUIWidget::Paint( TGUIDisplayContext* a_pDisplayContext, TGUIWidget* a_pFrom )
{
	if ( m_oChildren.IsEmpty() )
		return TNULL;

	TGUIWidget* pWidget = a_pFrom;

	if ( !pWidget )
	{
		pWidget = m_oChildren.Head();
		a_pDisplayContext->PushClip( m_iXPos, m_iYPos, m_iWidth, m_iHeight );
	}

	TBOOL      bStop = TFALSE;
	TGUIColour oColour( TUINT8( 255 ), TUINT8( 255 ), TUINT8( 255 ), TUINT8( 255 ) );

	while ( TTRUE )
	{
		if ( pWidget->m_uiState & State_Visible )
		{
			a_pDisplayContext->PushClip( pWidget->m_iXPos, pWidget->m_iYPos, pWidget->m_iWidth, pWidget->m_iHeight );
			a_pDisplayContext->SetTimeOffset( pWidget->m_fAnimSync );

			if ( pWidget->m_fAlpha != 1.0f )
			{
				oColour = TGUIColour( TUINT8( 255 ), TUINT8( 255 ), TUINT8( 255 ), TUINT8( TINT( pWidget->m_fAlpha * 255.0f ) ) );
				a_pDisplayContext->PushModulateColour( oColour );
			}

			bStop = pWidget->OnPaint( a_pDisplayContext );

			if ( !pWidget->m_oChildren.IsEmpty() )
			{
				// Paint the children of the widget
				pWidget = pWidget->m_oChildren.Head();
				continue;
			}

			a_pDisplayContext->PopClip();

			if ( pWidget->m_fAlpha != 1.0f )
				a_pDisplayContext->PopModulateColour();
		}

		// Go up while the widget is the last child
		while ( pWidget->Next() == pWidget->m_pParent->m_oChildren.End().Get() )
		{
			pWidget = pWidget->m_pParent;
			a_pDisplayContext->PopClip();

			// NOTE: the colour is popped for this widget too, though it wasn't pushed (as in the original)
			if ( pWidget->m_fAlpha != 1.0f )
				a_pDisplayContext->PopModulateColour();

			if ( pWidget == this )
				return TNULL;
		}

		pWidget = pWidget->Next();

		if ( bStop )
			return pWidget;
	}
}

// $NicktoonsPS2: FUNCTION 00374d70
// $JPOG: FUNCTION TGuiInterface.dll+0001eb70
TBOOL TGUIWidget::HitTest( TINT a_iX, TINT a_iY, TINT& a_rX, TINT& a_rY )
{
	TINT iX = a_iX - m_iXPos;
	TINT iY = a_iY - m_iYPos;

	if ( iX < 0 || iX >= m_iWidth || iY < 0 || iY >= m_iHeight )
		return TFALSE;

	a_rX = iX;
	a_rY = iY;
	return TTRUE;
}

// The deepest widget at a_iX, a_iY (in the widget) whose state matches, the children are tested from the last
// (the top most) one. A widget with root components is only hit if its style is hit.
// $NicktoonsPS2: FUNCTION 00374dc0
// $JPOG: FUNCTION TGuiInterface.dll+0001ebd0
TGUIWidget* TGUIWidget::HitTestDeepest( TINT a_iX, TINT a_iY, TINT& a_rX, TINT& a_rY, TUINT a_uiStateMask, TUINT a_uiStateValue )
{
	for ( auto it = m_oChildren.Tail(); it != m_oChildren.End(); it-- )
	{
		if ( ( it->m_uiState & a_uiStateMask ) == a_uiStateValue && it->HitTest( a_iX, a_iY, a_rX, a_rY ) )
		{
			TGUIWidget* pWidget = it->HitTestDeepest( a_rX, a_rY, a_rX, a_rY, a_uiStateMask, a_uiStateValue );

			if ( pWidget )
				return pWidget;
		}
	}

	if ( GetWidgetInfo()->GetComponentCount() != 0 )
	{
		if ( !m_pStyle )
			return TNULL;

		if ( !m_pStyle->HitTest( this, a_iX, a_iY, m_uiState, m_pParameters, m_pComponentCaches ) )
			return TNULL;
	}

	a_rX = a_iX;
	a_rY = a_iY;
	return this;
}

// $JPOG: FUNCTION TGuiInterface.dll+0001ea10
TGUIWidget* TGUIWidget::GetChildAtPoint( TINT a_iX, TINT a_iY, TUINT a_uiStateMask, TUINT a_uiStateValue )
{
	for ( auto it = m_oChildren.Tail(); it != m_oChildren.End(); it-- )
	{
		if ( ( it->m_uiState & a_uiStateMask ) == a_uiStateValue )
		{
			TINT iX = a_iX - it->m_iXPos;
			TINT iY = a_iY - it->m_iYPos;

			if ( iX >= 0 && iX < it->m_iWidth && iY >= 0 && iY < it->m_iHeight )
				return it;
		}
	}

	return TNULL;
}

// $JPOG: FUNCTION TGuiInterface.dll+0001ea90
TGUIWidget* TGUIWidget::GetChildAtPoint( TINT a_iX, TINT a_iY, TINT& a_rX, TINT& a_rY, TUINT a_uiStateMask, TUINT a_uiStateValue )
{
	for ( auto it = m_oChildren.Tail(); it != m_oChildren.End(); it-- )
	{
		if ( ( it->m_uiState & a_uiStateMask ) == a_uiStateValue )
		{
			TINT iX = a_iX - it->m_iXPos;
			TINT iY = a_iY - it->m_iYPos;

			if ( iX >= 0 && iX < it->m_iWidth && iY >= 0 && iY < it->m_iHeight )
			{
				a_rX = iX;
				a_rY = iY;
				return it;
			}
		}
	}

	return TNULL;
}

// $JPOG: FUNCTION TGuiInterface.dll+0001eb20
TGUIWidget* TGUIWidget::GetDeepestChildAtPoint( TINT a_iX, TINT a_iY, TINT& a_rX, TINT& a_rY, TUINT a_uiStateMask, TUINT a_uiStateValue )
{
	TGUIWidget* pWidget = this;

	while ( TGUIWidget* pChild = pWidget->GetChildAtPoint( a_iX, a_iY, a_rX, a_rY, a_uiStateMask, a_uiStateValue ) )
	{
		a_iX    = a_rX;
		a_iY    = a_rY;
		pWidget = pChild;
	}

	return pWidget;
}

//-----------------------------------------------------------------------------
// Properties
//-----------------------------------------------------------------------------

// $NicktoonsPS2: FUNCTION 00375778
// $JPOG: FUNCTION TGuiInterface.dll+00020d40
void TGUIWidget::Set_name_property( const PPropertyValue& a_rcValue )
{
	m_strName = a_rcValue.GetTPString8();
}

// $NicktoonsPS2: FUNCTION 00375800
// $JPOG: FUNCTION TGuiInterface.dll+00020d20
PPropertyValue TGUIWidget::Get_name_property() const
{
	return PPropertyValue( m_strName );
}

// $NicktoonsPS2: FUNCTION 00375a28
// $JPOG: FUNCTION TGuiInterface.dll+00020ba0
void TGUIWidget::Set_sizemode_horiz_property( const PPropertyValue& a_rcValue )
{
	m_eSizeModeHoriz = SizeMode( a_rcValue.GetInteger() );
}

// $NicktoonsPS2: FUNCTION 00375a58
// $JPOG: FUNCTION TGuiInterface.dll+00020b80
PPropertyValue TGUIWidget::Get_sizemode_horiz_property() const
{
	return PPropertyValue( TINT( m_eSizeModeHoriz ) );
}

// $NicktoonsPS2: FUNCTION 00375a88
// $JPOG: FUNCTION TGuiInterface.dll+00020b60
void TGUIWidget::Set_sizemode_vert_property( const PPropertyValue& a_rcValue )
{
	m_eSizeModeVert = SizeMode( a_rcValue.GetInteger() );
}

// $NicktoonsPS2: FUNCTION 00375ab8
// $JPOG: FUNCTION TGuiInterface.dll+00020b40
PPropertyValue TGUIWidget::Get_sizemode_vert_property() const
{
	return PPropertyValue( TINT( m_eSizeModeVert ) );
}

// $NicktoonsPS2: FUNCTION 00375ae8
// $JPOG: FUNCTION TGuiInterface.dll+00020b20
void TGUIWidget::Set_placemode_property( const PPropertyValue& a_rcValue )
{
	m_ePlaceMode = PlaceMode( a_rcValue.GetInteger() );
}

// $NicktoonsPS2: FUNCTION 00375b18
// $JPOG: FUNCTION TGuiInterface.dll+00020b00
PPropertyValue TGUIWidget::Get_placemode_property() const
{
	return PPropertyValue( TINT( m_ePlaceMode ) );
}

// $NicktoonsPS2: FUNCTION 00375838
// $JPOG: FUNCTION TGuiInterface.dll+00020ce0
void TGUIWidget::Set_xpos_property( const PPropertyValue& a_rcValue )
{
	m_iXPos = TINT16( a_rcValue.GetInteger() );
}

// $NicktoonsPS2: FUNCTION 00375868
// $JPOG: FUNCTION TGuiInterface.dll+00020cc0
PPropertyValue TGUIWidget::Get_xpos_property() const
{
	return PPropertyValue( TINT( m_iXPos ) );
}

// $NicktoonsPS2: FUNCTION 003758a0
// $JPOG: FUNCTION TGuiInterface.dll+00020ca0
void TGUIWidget::Set_ypos_property( const PPropertyValue& a_rcValue )
{
	m_iYPos = TINT16( a_rcValue.GetInteger() );
}

// $NicktoonsPS2: FUNCTION 003758d0
// $JPOG: FUNCTION TGuiInterface.dll+00020c80
PPropertyValue TGUIWidget::Get_ypos_property() const
{
	return PPropertyValue( TINT( m_iYPos ) );
}

// $NicktoonsPS2: FUNCTION 00375908
// $JPOG: FUNCTION TGuiInterface.dll+00020c40
void TGUIWidget::Set_width_property( const PPropertyValue& a_rcValue )
{
	if ( m_iWidth != a_rcValue.GetInteger() )
		m_bValidated = TFALSE;

	m_iWidth = TINT16( a_rcValue.GetInteger() );
}

// $NicktoonsPS2: FUNCTION 00375960
// $JPOG: FUNCTION TGuiInterface.dll+00020c20
PPropertyValue TGUIWidget::Get_width_property() const
{
	return PPropertyValue( TINT( m_iWidth ) );
}

// $NicktoonsPS2: FUNCTION 00375998
// $JPOG: FUNCTION TGuiInterface.dll+00020be0
void TGUIWidget::Set_height_property( const PPropertyValue& a_rcValue )
{
	if ( m_iHeight != a_rcValue.GetInteger() )
		m_bValidated = TFALSE;

	m_iHeight = TINT16( a_rcValue.GetInteger() );
}

// $NicktoonsPS2: FUNCTION 003759f0
// $JPOG: FUNCTION TGuiInterface.dll+00020bc0
PPropertyValue TGUIWidget::Get_height_property() const
{
	return PPropertyValue( TINT( m_iHeight ) );
}

// $NicktoonsPS2: FUNCTION 00375bb0
// $JPOG: FUNCTION TGuiInterface.dll+00020ae0
void TGUIWidget::Set_animsync_property( const PPropertyValue& a_rcValue )
{
	m_fAnimSync = a_rcValue.GetFloat();
}

// $NicktoonsPS2: FUNCTION 00375be0
// $JPOG: FUNCTION TGuiInterface.dll+00020ac0
PPropertyValue TGUIWidget::Get_animsync_property() const
{
	return PPropertyValue( m_fAnimSync );
}

// $NicktoonsPS2: FUNCTION 00375b50
void TGUIWidget::Set_alpha_property( const PPropertyValue& a_rcValue )
{
	m_fAlpha = a_rcValue.GetFloat();
}

// $NicktoonsPS2: FUNCTION 00375b80
PPropertyValue TGUIWidget::Get_alpha_property() const
{
	return PPropertyValue( m_fAlpha );
}

// NOTE: JPOG has versions taking the name and the index of the state variable
// $NicktoonsPS2: FUNCTION 003740b0
// $JPOG: FUNCTION TGuiInterface.dll+0001dc80
void TGUIWidget::Set_state_property( const PPropertyValue& a_rcValue, TUINT a_uiMask )
{
	if ( a_rcValue.GetBoolean() == TTRUE )
		m_uiState |= a_uiMask;
	else
		m_uiState &= ~a_uiMask;
}

// $NicktoonsPS2: FUNCTION 00374108
// $JPOG: FUNCTION TGuiInterface.dll+00020a40
PPropertyValue TGUIWidget::Get_state_property( TUINT a_uiMask ) const
{
	return PPropertyValue( TBOOL( ( m_uiState & a_uiMask ) != 0 ) );
}

// NOTE: other types of values are ignored
// $NicktoonsPS2: FUNCTION 00374140
// $JPOG: FUNCTION TGuiInterface.dll+0001dcc0
void TGUIWidget::Set_style_parameter( const PPropertyValue& a_rcValue, TGUIStyleParameterInfo* a_pParameter )
{
	if ( a_rcValue.GetType() == PPropertyValue::TYPE_STRING16 )
		SetParameter( a_pParameter->GetIndex(), a_rcValue.GetString16() );
	else if ( a_rcValue.GetType() == &TGetClass( TGUITextureSection ) )
		SetParameter( a_pParameter->GetIndex(), TSTATICCAST( TGUITextureSection, a_rcValue.GetTObject() ) );
	else if ( a_rcValue.GetType() == PPropertyValue::TYPE_UNDEF )
		SetParameter( a_pParameter->GetIndex(), a_rcValue );
}

// $NicktoonsPS2: FUNCTION 00374210
// $JPOG: FUNCTION TGuiInterface.dll+00020a10
PPropertyValue TGUIWidget::Get_style_parameter( TGUIStyleParameterInfo* a_pParameter ) const
{
	return GetParameter( a_pParameter->GetIndex() );
}

// $NicktoonsPS2: FUNCTION 00375c70
void TGUIWidget::DeclareStateVar( const TClass* a_pClass, const TCHAR* a_szName, TUINT a_uiMask, const TPString8& a_rcCategory, TINT& a_rSortHint, TGUIWidgetInfo* a_pWidgetInfo, PropertySet* a_pPropertySet, PropertySet* a_pOrderedSet )
{
	TPString8 strName( a_szName );
	a_pWidgetInfo->AddStateVar( TGUIStateVarInfo( strName, a_uiMask ) );

	TGUIStateVarSpec* pSpec = new TGUIStateVarSpec(
	    strName,
	    a_rcCategory,
	    a_pClass,
	    a_rSortHint++,
	    static_cast<TGUIStateVarSpec::t_Setter>( &TGUIWidget::Set_state_property ),
	    static_cast<TGUIStateVarSpec::t_Getter>( &TGUIWidget::Get_state_property ),
	    a_uiMask
	);

	AddPropertySpec( a_pPropertySet, a_pOrderedSet, pSpec );
}

// $NicktoonsPS2: FUNCTION 00375c18
// $JPOG: FUNCTION TGuiInterface.dll+00020760
const TPString8& TGUIWidget::GetDirectionName( NeighbourDirection a_eDirection )
{
	return ms_saDirectionName[ a_eDirection ];
}

// NeighbourDirection_NUMOF if the name is unknown
// $NicktoonsPS2: FUNCTION 00375c30
// $JPOG: FUNCTION TGuiInterface.dll+0001eec0
TGUIWidget::NeighbourDirection TGUIWidget::GetDirectionEnum( const TPString8& a_rcName )
{
	for ( TINT i = 0; i < NeighbourDirection_NUMOF; i++ )
	{
		if ( ms_saDirectionName[ i ] == a_rcName )
			return NeighbourDirection( i );
	}

	return NeighbourDirection_NUMOF;
}

TOSHI_NAMESPACE_END
