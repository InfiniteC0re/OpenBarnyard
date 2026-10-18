#include "ToshiPCH.h"
#include "TGUIComponentInfo.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $NicktoonsPS2: FUNCTION 00379cf0
// $JPOG: FUNCTION TGuiInterface.dll+00024b70
TGUIComponentInfo::TGUIComponentInfo()
    : m_pClass( TNULL )
    , m_aElementNames( 3, 0 )
{
}

// NOTE: JPOG also copies the list links of the node, the copy isn't linked here
// $JPOG: FUNCTION TGuiInterface.dll+00006ee0
TGUIComponentInfo::TGUIComponentInfo( const TGUIComponentInfo& a_rcOther )
    : m_strComponentName( a_rcOther.m_strComponentName )
    , m_strComponentLabel( a_rcOther.m_strComponentLabel )
    , m_pClass( a_rcOther.m_pClass )
    , m_aElementNames( a_rcOther.m_aElementNames.GetGrowSize(), 0 )
{
	for ( TINT i = 0; i < a_rcOther.m_aElementNames.Size(); i++ )
		m_aElementNames.Push( a_rcOther.m_aElementNames[ i ] );
}

// $NicktoonsPS2: FUNCTION 00379d28
// $JPOG: FUNCTION TGuiInterface.dll+00024ba0
TGUIComponentInfo::~TGUIComponentInfo()
{
	if ( IsLinked() )
		GetList()->Remove( this );

	FreeElementNames();
}

// NOTE: JPOG also copies the list links of the node, they're kept here
// $JPOG: FUNCTION TGuiInterface.dll+00006e40
TGUIComponentInfo& TGUIComponentInfo::operator=( const TGUIComponentInfo& a_rcOther )
{
	m_strComponentName  = a_rcOther.m_strComponentName;
	m_strComponentLabel = a_rcOther.m_strComponentLabel;
	m_pClass            = a_rcOther.m_pClass;

	FreeElementNames();
	m_aElementNames.SetGrowSize( a_rcOther.m_aElementNames.GetGrowSize() );

	for ( TINT i = 0; i < a_rcOther.m_aElementNames.Size(); i++ )
		m_aElementNames.Push( a_rcOther.m_aElementNames[ i ] );

	return *this;
}

// Called by the static deinitialisers of the component classes
// NOTE: the name is unknown
// $NicktoonsPS2: FUNCTION 00379e30
void TGUIComponentInfo::Clear()
{
	if ( IsLinked() )
		GetList()->Remove( this );

	m_strComponentName  = TPString8();
	m_strComponentLabel = TPString8();
	m_pClass            = TNULL;

	FreeElementNames();
}

// $NicktoonsPS2: FUNCTION 00379fd8
// $JPOG: FUNCTION TGuiInterface.dll+00024c30
void TGUIComponentInfo::SetComponentName( const TPString8& a_rcName )
{
	m_strComponentName = a_rcName;
	m_pClass           = TClass::Find( m_strComponentName.GetString() );
}

// $NicktoonsPS2: FUNCTION 0037a078
// $JPOG: FUNCTION TGuiInterface.dll+00026230
void TGUIComponentInfo::SetComponentLabel( const TPString8& a_rcLabel )
{
	m_strComponentLabel = a_rcLabel;
}

// $NicktoonsPS2: FUNCTION 0037a0e8
// $JPOG: FUNCTION TGuiInterface.dll+000261b0
void TGUIComponentInfo::AddElementName( const TPString8& a_rcName )
{
	m_aElementNames.Push( a_rcName );
}

// NOTE: TArray doesn't destroy its elements
void TGUIComponentInfo::FreeElementNames()
{
	for ( TINT i = 0; i < m_aElementNames.Size(); i++ )
		m_aElementNames[ i ].~TPString8();

	m_aElementNames.FreeData();
}

TOSHI_NAMESPACE_END
