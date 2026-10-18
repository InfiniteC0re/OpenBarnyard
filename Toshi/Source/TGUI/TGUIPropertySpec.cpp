#include "ToshiPCH.h"
#include "TGUIPropertySpec.h"

#include <Plugins/PPropertyParser/PPropertyValue.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TINT TGUIPropertySpec::Comparator::operator()( const TGUIPropertySpec* a_pA, const TGUIPropertySpec* a_pB ) const
{
	return TStringManager::String8Compare( a_pA->m_strName.GetString(), a_pB->m_strName.GetString() );
}

// NOTE: the unknown string is set to a static string of Nicktoons that is never assigned (empty)
// $NicktoonsPS2: FUNCTION 003870f8
// $JPOG: FUNCTION TGuiInterface.dll+00012910
TGUIPropertySpec::TGUIPropertySpec( const TPString8& a_rcName )
    : m_pType( PPropertyValue::TYPE_UNDEF )
    , m_strName( a_rcName )
    , m_bAutoInstall( TFALSE )
    , m_pSourceClass( TNULL )
    , m_iSortHint( 0 )
    , m_eKind( Kind_Value )
{
}

// $NicktoonsPS2: FUNCTION 00387168
// $JPOG: FUNCTION TGuiInterface.dll+00012b00
TGUIPropertySpec::TGUIPropertySpec( const TPString8& a_rcName, const TClass* a_pType, const TPString8& a_rcUnknown, TBOOL a_bAutoInstall, const TClass* a_pSourceClass, TINT a_iSortHint )
    : m_pType( a_pType )
    , m_strName( a_rcName )
    , m_strUnknown( a_rcUnknown )
    , m_bAutoInstall( a_bAutoInstall )
    , m_pSourceClass( a_pSourceClass )
    , m_iSortHint( a_iSortHint )
    , m_eKind( Kind_Value )
{
}

// NOTE: stripped from Nicktoons
// $JPOG: FUNCTION TGuiInterface.dll+00012b50
TGUIPropertySpec::TGUIPropertySpec( const TPString8& a_rcName, const TClass* a_pType, const TPString8& a_rcUnknown, TBOOL a_bAutoInstall, const TClass* a_pSourceClass, TINT a_iSortHint, const TClass* a_pBaseClass )
    : m_pType( a_pType )
    , m_strName( a_rcName )
    , m_strUnknown( a_rcUnknown )
    , m_bAutoInstall( a_bAutoInstall )
    , m_pSourceClass( a_pSourceClass )
    , m_iSortHint( a_iSortHint )
    , m_eKind( Kind_ClassName )
    , m_pBaseClass( a_pBaseClass )
{
}

// $NicktoonsPS2: FUNCTION 003871d8
// $JPOG: FUNCTION TGuiInterface.dll+00012bb0
TGUIPropertySpec::TGUIPropertySpec( const TPString8& a_rcName, const TClass* a_pType, const TPString8& a_rcUnknown, TBOOL a_bAutoInstall, const TClass* a_pSourceClass, TINT a_iSortHint, TINT a_iEnumCount, const TCHAR* const* a_ppEnumNames )
    : m_pType( a_pType )
    , m_strName( a_rcName )
    , m_strUnknown( a_rcUnknown )
    , m_bAutoInstall( a_bAutoInstall )
    , m_pSourceClass( a_pSourceClass )
    , m_iSortHint( a_iSortHint )
    , m_eKind( Kind_Enumerated )
    , m_iEnumCount( a_iEnumCount )
{
	m_pEnumNames = new TPString8[ a_iEnumCount ];

	for ( TINT i = 0; i < m_iEnumCount; i++ )
		m_pEnumNames[ i ] = TPString8( a_ppEnumNames[ i ] );
}

// NOTE: the kind decides what is copied, the sort hint isn't copied
// $NicktoonsPS2: FUNCTION 00387398
// $JPOG: FUNCTION TGuiInterface.dll+00012cf0
TGUIPropertySpec::TGUIPropertySpec( const TGUIPropertySpec& a_rcOther )
    : m_pType( a_rcOther.m_pType )
    , m_strName( a_rcOther.m_strName )
    , m_strUnknown( a_rcOther.m_strUnknown )
    , m_bAutoInstall( a_rcOther.m_bAutoInstall )
    , m_pSourceClass( a_rcOther.m_pSourceClass )
#ifdef BARNYARD_COMMUNITY_PATCH
    // NOTE: the original leaves the sort hint uninitialised (so do the clones in the property sets)
    , m_iSortHint( a_rcOther.m_iSortHint )
#endif
    , m_eKind( a_rcOther.m_eKind )
{
	if ( m_eKind == Kind_Enumerated )
	{
		m_iEnumCount = a_rcOther.m_iEnumCount;
		m_pEnumNames = new TPString8[ m_iEnumCount ];
		CopyEnumNames( a_rcOther.m_pEnumNames );
	}
	else if ( m_eKind == Kind_ClassName )
	{
		m_pBaseClass = a_rcOther.m_pBaseClass;
	}
}

// $NicktoonsPS2: FUNCTION 00387550
// $JPOG: FUNCTION TGuiInterface.dll+00012950
TGUIPropertySpec::~TGUIPropertySpec()
{
	if ( m_eKind == Kind_Enumerated && m_pEnumNames )
		DeleteEnumNames();
}

// NOTE: the kind isn't copied, the enumeration names and the base class are copied if this spec is of that kind
// $NicktoonsPS2: FUNCTION 00387660
// $JPOG: FUNCTION TGuiInterface.dll+000129a0
TGUIPropertySpec& TGUIPropertySpec::operator=( const TGUIPropertySpec& a_rcOther )
{
	m_pType        = a_rcOther.m_pType;
	m_strName      = a_rcOther.m_strName;
	m_strUnknown   = a_rcOther.m_strUnknown;
	m_bAutoInstall = a_rcOther.m_bAutoInstall;
	m_pSourceClass = a_rcOther.m_pSourceClass;
	m_iSortHint    = a_rcOther.m_iSortHint;

	if ( m_eKind == Kind_Enumerated )
	{
		m_iEnumCount = a_rcOther.m_iEnumCount;

		if ( m_pEnumNames )
			DeleteEnumNames();

		if ( a_rcOther.m_pEnumNames )
		{
			m_pEnumNames = new TPString8[ m_iEnumCount ];
			CopyEnumNames( a_rcOther.m_pEnumNames );
		}
		else
		{
			m_pEnumNames = TNULL;
		}
	}
	else if ( m_eKind == Kind_ClassName )
	{
		m_pBaseClass = a_rcOther.m_pBaseClass;
	}

	return *this;
}

// $NicktoonsPS2: FUNCTION 003878e0
// $JPOG: FUNCTION TGuiInterface.dll+000128e0
TINT TGUIPropertySpec::GetEnumValue( const TPString8& a_rcName ) const
{
	for ( TINT i = 0; i < m_iEnumCount; i++ )
	{
		if ( m_pEnumNames[ i ] == a_rcName )
			return i;
	}

	return -1;
}

// NOTE: inlined in the copy constructor and operator=
void TGUIPropertySpec::CopyEnumNames( const TPString8* a_pEnumNames )
{
	for ( TINT i = 0; i < m_iEnumCount; i++ )
		m_pEnumNames[ i ] = a_pEnumNames[ i ];
}

// NOTE: inlined in the destructor and operator=
void TGUIPropertySpec::DeleteEnumNames()
{
	delete[] m_pEnumNames;
}

TOSHI_NAMESPACE_END
