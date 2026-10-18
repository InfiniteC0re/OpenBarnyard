#include "ToshiPCH.h"
#include "TGUIWidgetInfo.h"
#include "TGUIPropertySpecSimple.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $NicktoonsPS2: FUNCTION 0036e3f8
// $JPOG: FUNCTION TGuiInterface.dll+0001b080
TGUIWidgetInfo::TGUIWidgetInfo()
    : m_pClass( TNULL )
    , m_aComponentNames( 4, 0 )
    , m_aStateVars( 10, 0 )
    , m_pProperties( TNULL )
    , m_pOrderedProperties( TNULL )
    , m_uiInputActions( 0 )
    , m_bConcrete( TFALSE )
    , m_iInstanceCount( 0 )
{
}

// $NicktoonsPS2: FUNCTION 0036eab0
// $JPOG: FUNCTION TGuiInterface.dll+0001b0c0
TGUIWidgetInfo::~TGUIWidgetInfo()
{
	if ( IsLinked() )
		GetList()->Remove( this );

	// NOTE: TArray doesn't destroy its elements
	for ( TINT i = 0; i < m_aStateVars.Size(); i++ )
		m_aStateVars[ i ].~TGUIStateVarInfo();

	for ( TINT i = 0; i < m_aComponentNames.Size(); i++ )
		m_aComponentNames[ i ].~TPString8();
}

// Called by the static deinitialisers of the widget classes
// NOTE: the name is unknown
// $NicktoonsPS2: FUNCTION 0036ebc8
void TGUIWidgetInfo::Clear()
{
	if ( IsLinked() )
		GetList()->Remove( this );

	m_strWidgetName  = TPString8();
	m_strWidgetLabel = TPString8();
	m_pClass         = TNULL;

	for ( TINT i = 0; i < m_aComponentNames.Size(); i++ )
		m_aComponentNames[ i ].~TPString8();

	m_aComponentNames.FreeData();

	for ( TINT i = 0; i < m_aStateVars.Size(); i++ )
		m_aStateVars[ i ].~TGUIStateVarInfo();

	m_aStateVars.FreeData();

	m_pOrderedProperties = TNULL;
	m_pProperties        = TNULL;
}

// $NicktoonsPS2: FUNCTION 0036e998
// $JPOG: FUNCTION TGuiInterface.dll+0001b430
void TGUIWidgetInfo::SetWidgetName( const TPString8& a_rcName )
{
	m_strWidgetName = a_rcName;
	m_pClass        = TClass::Find( m_strWidgetName.GetString() );
}

// $NicktoonsPS2: FUNCTION 0036ea38
// $JPOG: FUNCTION TGuiInterface.dll+00020ea0
void TGUIWidgetInfo::SetWidgetLabel( const TPString8& a_rcLabel )
{
	m_strWidgetLabel = a_rcLabel;
}

// NOTE: JPOG only sets the ordered set
// $NicktoonsPS2: FUNCTION 0036e988
// $JPOG: FUNCTION TGuiInterface.dll+00020e90
void TGUIWidgetInfo::SetProperties( PropertySet* a_pSet, PropertySet* a_pOrderedSet )
{
	m_pProperties        = a_pSet;
	m_pOrderedProperties = a_pOrderedSet;
}

// $NicktoonsPS2: FUNCTION 0036eaa8
// $JPOG: FUNCTION TGuiInterface.dll+00020e80
void TGUIWidgetInfo::SetConcrete( TBOOL a_bConcrete )
{
	m_bConcrete = a_bConcrete;
}

// $NicktoonsPS2: FUNCTION 0036e458
// $JPOG: FUNCTION TGuiInterface.dll+0001b320
void TGUIWidgetInfo::AddStateVars( const StateVarSet& a_rcStateVars )
{
	for ( TINT i = 0; i < a_rcStateVars.Size(); i++ )
		AddStateVar( a_rcStateVars[ i ] );
}

// Inserts the state variable after the ones with the same bit
// NOTE: inlined in AddStateVars and in the static initialisers of the widget classes
void TGUIWidgetInfo::AddStateVar( const TGUIStateVarInfo& a_rcStateVar )
{
	TGUIStateVarInfo::Comparator oComparator;

	TINT iLow  = 0;
	TINT iHigh = m_aStateVars.Size() - 1;

	while ( iLow <= iHigh )
	{
		TINT iMiddle  = ( iLow + iHigh ) / 2;
		TINT iCompare = oComparator( a_rcStateVar, m_aStateVars[ iMiddle ] );

		if ( iCompare < 0 )
		{
			iHigh = iMiddle - 1;
		}
		else
		{
			iLow = iMiddle + 1;

			if ( iCompare == 0 )
			{
				while ( iLow <= iHigh && m_aStateVars[ iLow ].uiMask == a_rcStateVar.uiMask )
					iLow++;

				break;
			}
		}
	}

	m_aStateVars.InsertBefore( StateVarSet::Iterator( iLow, m_aStateVars ), a_rcStateVar );
}

// NOTE: inlined in the static initialisers of the widget classes
void TGUIWidgetInfo::AddComponentName( const TPString8& a_rcName )
{
	m_aComponentNames.Push( a_rcName );
}

// $NicktoonsPS2: FUNCTION 0036edf8
// $JPOG: FUNCTION TGuiInterface.dll+0001b160
const TGUIStateVarInfo* TGUIWidgetInfo::GetStateVarInfo( const TPString8& a_rcName ) const
{
	for ( TINT i = 0; i < m_aStateVars.Size(); i++ )
	{
		if ( m_aStateVars[ i ].strName == a_rcName )
			return &m_aStateVars[ i ];
	}

	return TNULL;
}

// $JPOG: FUNCTION TGuiInterface.dll+0001b2d0
TUINT TGUIWidgetInfo::GetStateVarMask( const TPString8& a_rcName ) const
{
	for ( TINT i = 0; i < m_aStateVars.Size(); i++ )
	{
		if ( m_aStateVars[ i ].strName == a_rcName )
			return m_aStateVars[ i ].uiMask;
	}

	return 0;
}

// $JPOG: FUNCTION TGuiInterface.dll+0001b2a0
TINT TGUIWidgetInfo::GetComponentIndex( const TPString8& a_rcName ) const
{
	for ( TINT i = 0; i < m_aComponentNames.Size(); i++ )
	{
		if ( m_aComponentNames[ i ] == a_rcName )
			return i;
	}

	return -1;
}

// $JPOG: FUNCTION TGuiInterface.dll+0001b1b0
TPString8 TGUIWidgetInfo::GetStateExpression( TUINT a_uiMask, TUINT a_uiValue ) const
{
	if ( a_uiMask == 0 )
		return TPString8( "default" );

	TString8 strExpression;
	TBOOL    bFirst = TTRUE;

	for ( TINT i = 0; i < m_aStateVars.Size(); i++ )
	{
		TUINT uiMask = m_aStateVars[ i ].uiMask;

		if ( uiMask & a_uiMask )
		{
			if ( !bFirst )
				strExpression += " and ";

			if ( !( uiMask & a_uiValue ) )
				strExpression += "not ";

			strExpression += m_aStateVars[ i ].strName.GetString();
			bFirst = TFALSE;
		}
	}

	return TPString8( strExpression.GetString() );
}

// $NicktoonsPS2: FUNCTION 0036ee38
// $JPOG: FUNCTION TGuiInterface.dll+0001b4a0
const TGUIPropertySpec* TGUIWidgetInfo::GetPropertyInfo( const TPString8& a_rcName ) const
{
	TGUIPropertySpecSimple oKey( a_rcName );

	PropertySet*                 pOrderedSet = m_pOrderedProperties;
	TGUIPropertySpec::Comparator oComparator;

	TINT iLow  = 0;
	TINT iHigh = pOrderedSet->Size() - 1;

	while ( iLow <= iHigh )
	{
		TINT iMiddle  = ( iLow + iHigh ) / 2;
		TINT iCompare = oComparator( ( *pOrderedSet )[ iMiddle ], &oKey );

		if ( iCompare > 0 )
		{
			iHigh = iMiddle - 1;
		}
		else if ( iCompare < 0 )
		{
			iLow = iMiddle + 1;
		}
		else
		{
			TINT iIndex = iMiddle - 1;

			while ( iIndex >= iLow && oComparator( ( *pOrderedSet )[ iIndex ], &oKey ) == 0 )
				iIndex--;

			return ( *pOrderedSet )[ iIndex + 1 ];
		}
	}

	return TNULL;
}

TOSHI_NAMESPACE_END
