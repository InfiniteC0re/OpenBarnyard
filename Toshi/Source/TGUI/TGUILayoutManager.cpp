#include "ToshiPCH.h"
#include "TGUILayoutManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: CreateObject returns TNULL in Nicktoons (00386d20)
TDEFINE_CLASS_INIT_NORUNTIME( TGUILayoutManager );

TGUIDeserialisable::PropertySet* TGUILayoutManager::ms_oPropertySet        = TNULL;
TGUIDeserialisable::PropertySet* TGUILayoutManager::ms_oPropertySetOrdered = TNULL;

// $NicktoonsPS2: FUNCTION 00386e38
// $JPOG: FUNCTION TGuiInterface.dll+00012520
void TGUILayoutManager::InitialiseClass()
{
	ms_oPropertySet        = new PropertySet( 4, 0 );
	ms_oPropertySetOrdered = new PropertySet( 4, 0 );
}

// $NicktoonsPS2: FUNCTION 00386ea8
// $JPOG: FUNCTION TGuiInterface.dll+00012580
void TGUILayoutManager::DeinitialiseClass()
{
	DestroyPropertySets( ms_oPropertySet, ms_oPropertySetOrdered );
}

// NOTE: JPOG also clears the interface (+0x4) and an unknown field (+0x8), Nicktoons' class has neither
// $JPOG: FUNCTION TGuiInterface.dll+00012740
TGUILayoutManager::TGUILayoutManager()
{
}

// $JPOG: FUNCTION TGuiInterface.dll+00012690
TGUILayoutManager::TGUILayoutManager( const TGUILayoutManager& a_rcOther )
    : TGUIDeserialisable( a_rcOther )
{
}

// $NicktoonsPS2: FUNCTION 00386e10
// $JPOG: FUNCTION TGuiInterface.dll+00012730
TGUILayoutManager::~TGUILayoutManager()
{
}

// $NicktoonsPS2: FUNCTION 00386fe0
// $JPOG: FUNCTION TGuiInterface.dll+00012700
TBOOL TGUILayoutManager::Deserialise( const PProperties* a_pProperties )
{
	return TGUIDeserialisable::Deserialise( a_pProperties );
}

// $NicktoonsPS2: FUNCTION 00387000
// $JPOG: FUNCTION TGuiInterface.dll+000126d0
TBOOL TGUILayoutManager::Serialise( PProperties* a_pProperties )
{
	return TGUIDeserialisable::Serialise( a_pProperties );
}

// $NicktoonsPS2: FUNCTION 00387020
// $JPOG: FUNCTION TGuiInterface.dll+00012770
TGUIDeserialisable::PropertySet* TGUILayoutManager::GetPropertyInfoSet() const
{
	return ms_oPropertySet;
}

// $NicktoonsPS2: FUNCTION 00387028
// $JPOG: FUNCTION TGuiInterface.dll+00012760
TGUIDeserialisable::PropertySet* TGUILayoutManager::GetPropertyInfoOrderedSet() const
{
	return ms_oPropertySetOrdered;
}

// $JPOG: FUNCTION TGuiInterface.dll+00012420
TGUILayoutInfo::TGUILayoutInfo( const TPString8& a_rcName, const TPString8& a_rcLabel, PropertySet* a_pProperties )
    : m_strName( a_rcName )
    , m_strLabel( a_rcLabel )
    , m_pProperties( a_pProperties )
{
	m_pClass = TClass::Find( m_strName.GetString() );
}

// NOTE: the links of the node are copied too (as in the original)
// $JPOG: FUNCTION TGuiInterface.dll+00006540
TGUILayoutInfo::TGUILayoutInfo( const TGUILayoutInfo& a_rcOther )
    : TNodeList<TGUILayoutInfo>::TNode( a_rcOther )
    , m_strName( a_rcOther.m_strName )
    , m_strLabel( a_rcOther.m_strLabel )
    , m_pClass( a_rcOther.m_pClass )
    , m_pProperties( a_rcOther.m_pProperties )
{
}

// $NicktoonsPS2: FUNCTION 00386d38
// $JPOG: FUNCTION TGuiInterface.dll+000124a0
TGUILayoutInfo::~TGUILayoutInfo()
{
	if ( IsLinked() )
		Remove();
}

// $JPOG: FUNCTION TGuiInterface.dll+000064b0
TGUILayoutInfo& TGUILayoutInfo::operator=( const TGUILayoutInfo& a_rcOther )
{
	m_pList       = a_rcOther.m_pList;
	m_pNext       = a_rcOther.m_pNext;
	m_pPrev       = a_rcOther.m_pPrev;
	m_strName     = a_rcOther.m_strName;
	m_strLabel    = a_rcOther.m_strLabel;
	m_pClass      = a_rcOther.m_pClass;
	m_pProperties = a_rcOther.m_pProperties;

	return *this;
}

TOSHI_NAMESPACE_END
