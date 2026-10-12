#include "ToshiPCH.h"
#include "PProperty.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

// NOTE: GetClass 006b28d0, CreateObject 006b2e70, CreateObjectInPlace 006b2ec0
TDEFINE_CLASS_NORUNTIME( PProperties );
TDEFINE_FREELIST_ALLOCATOR( PProperties );

TDEFINE_CLASS_NO_PARENT( PProperties::UINT32, 0, 1 );
TDEFINE_CLASS_NO_PARENT( PProperties::INT, 0, 1 );
TDEFINE_CLASS_NO_PARENT( PProperties::FLOAT, 0, 1 );
TDEFINE_CLASS_NO_PARENT( PProperties::BOOL, 0, 1 );
TDEFINE_CLASS_NO_PARENT( PProperties::TPString8, 0, 1 );
TDEFINE_CLASS_NO_PARENT( PProperties::String16, 0, 1 );
TDEFINE_CLASS_NO_PARENT( PProperties::String16A, 0, 1 );
TDEFINE_CLASS_NO_PARENT( PProperties::LocaleID, 0, 1 );
TDEFINE_CLASS_NO_PARENT( PProperties::PPropertyName, 0, 1 );
TDEFINE_CLASS_NO_PARENT( PProperties::PPropertyValueArray, 0, 1 );
TDEFINE_CLASS_NO_PARENT( PProperties::TManagedPtr, 1, 0 );

TDEFINE_FREELIST_ALLOCATOR( PProperty );

// $Barnyard: FUNCTION 006b29f0
PProperties::PProperties()
    : m_pParent( TNULL )
    , m_iNumProperties( 0 )
{
}

// NOTE: 006b3790 is the unreferenced out of line copy, the deleting destructor inlines it
// $Barnyard: FUNCTION 006b3d80
// $Barnyard: FUNCTION 006b3790
PProperties::~PProperties()
{
	Clear();
}

// $Barnyard: FUNCTION 006b35b0
void PProperties::Clear()
{
	m_llProperties.DeleteAll();
	m_iNumProperties = 0;
}

void PProperties::AddPropertyNoCount( PProperty* a_pProperty )
{
	m_llProperties.InsertTail( a_pProperty );
}

// $Barnyard: FUNCTION 006b3010
void PProperties::AddProperty( const ::PPropertyName& a_rcName, const PPropertyValue& a_rcValue )
{
	m_llProperties.InsertTail( new PProperty( a_rcName, a_rcValue ) );
	m_iNumProperties++;
}

// $Barnyard: FUNCTION 006b35f0
void PProperties::AddProperty( const Toshi::TPString8& a_rcName, const PPropertyValue& a_rcValue )
{
	AddProperty( ::PPropertyName( a_rcName ), a_rcValue );
}

// $Barnyard: FUNCTION 006b3650
void PProperties::AddProperty( const Toshi::TPString8& a_rcName, const Toshi::TPString8& a_rcSubName, const PPropertyValue& a_rcValue )
{
	AddProperty( ::PPropertyName( a_rcName, a_rcSubName ), a_rcValue );
}

// $Barnyard: FUNCTION 006b3150
void PProperties::PutProperty( const ::PPropertyName& a_rcName, const PPropertyValue& a_rcValue )
{
	for ( auto it = m_llProperties.Begin(); it != m_llProperties.End(); it++ )
	{
		if ( it->GetName() == a_rcName )
		{
			it->SetValue( a_rcValue );
			return;
		}
	}

	m_llProperties.InsertTail( new PProperty( a_rcName, a_rcValue ) );
	m_iNumProperties++;
}

// $Barnyard: FUNCTION 006b36c0
void PProperties::PutProperty( const Toshi::TPString8& a_rcName, const PPropertyValue& a_rcValue )
{
	PutProperty( ::PPropertyName( a_rcName ), a_rcValue );
}

// $Barnyard: FUNCTION 006b3720
void PProperties::PutProperty( const Toshi::TPString8& a_rcName, const Toshi::TPString8& a_rcSubName, const PPropertyValue& a_rcValue )
{
	PutProperty( ::PPropertyName( a_rcName, a_rcSubName ), a_rcValue );
}

// $Barnyard: FUNCTION 006b2bf0
void PProperties::AddCommentedProperty( const ::PPropertyName& a_rcName, const PPropertyValue& a_rcValue, const Toshi::TPString8& a_rcComment, TINT a_iUnused )
{
	m_llProperties.InsertTail( new PProperty( a_rcName, a_rcValue, a_rcComment ) );
	m_iNumProperties++;
}

// $Barnyard: FUNCTION 006b2f30
void PProperties::AddCommentedProperty( const Toshi::TPString8& a_rcName, const PPropertyValue& a_rcValue, const Toshi::TPString8& a_rcComment )
{
	AddCommentedProperty( ::PPropertyName( a_rcName ), a_rcValue, a_rcComment, -1 );
}

// $Barnyard: FUNCTION 006b2fa0
void PProperties::AddCommentedProperty( const Toshi::TPString8& a_rcName, const Toshi::TPString8& a_rcSubName, const PPropertyValue& a_rcValue, const Toshi::TPString8& a_rcComment )
{
	AddCommentedProperty( ::PPropertyName( a_rcName, a_rcSubName ), a_rcValue, a_rcComment, -1 );
}

// $Barnyard: FUNCTION 006b2c60
void PProperties::AddProperty( const ::PPropertyName& a_rcName, const PPropertyValue& a_rcValue, const Toshi::TPString8& a_rcComment )
{
	m_llProperties.InsertTail( new PProperty( a_rcName, a_rcValue, a_rcComment ) );
	m_iNumProperties++;
}

// $Barnyard: FUNCTION 006b3070
void PProperties::AddProperty( const Toshi::TPString8& a_rcName, const PPropertyValue& a_rcValue, const Toshi::TPString8& a_rcComment )
{
	AddProperty( ::PPropertyName( a_rcName ), a_rcValue, a_rcComment );
}

// $Barnyard: FUNCTION 006b30e0
void PProperties::AddProperty( const Toshi::TPString8& a_rcName, const Toshi::TPString8& a_rcSubName, const PPropertyValue& a_rcValue, const Toshi::TPString8& a_rcComment )
{
	AddProperty( ::PPropertyName( a_rcName, a_rcSubName ), a_rcValue, a_rcComment );
}

// $Barnyard: FUNCTION 006b2900
const PPropertyValue* PProperties::GetProperty( const ::PPropertyName& a_rcName ) const
{
	TDList<PProperty>& rList = const_cast<TDList<PProperty>&>( m_llProperties );

	for ( auto it = rList.Begin(); it != rList.End(); it++ )
	{
		if ( it->GetName() == a_rcName )
			return &it->GetValue();
	}

	return TNULL;
}

// $Barnyard: FUNCTION 006b2940
const PProperty* PProperties::FindProperty( const Toshi::TPString8& a_rcName, Toshi::TDList<PProperty>::Iterator& a_rIt ) const
{
	TDList<PProperty>& rList = const_cast<TDList<PProperty>&>( m_llProperties );

	while ( a_rIt != rList.End() )
	{
		PProperty* pProperty = a_rIt.Get();
		a_rIt++;

		if ( pProperty->GetName().GetName() == a_rcName )
			return pProperty;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 006b2a50
const PPropertyValue* PProperties::GetProperty( const Toshi::TPString8& a_rcName ) const
{
	return GetProperty( ::PPropertyName( a_rcName ) );
}

// $Barnyard: FUNCTION 006b2ab0
const PPropertyValue* PProperties::GetProperty( const Toshi::TPString8& a_rcName, const Toshi::TPString8& a_rcSubName ) const
{
	return GetProperty( ::PPropertyName( a_rcName, a_rcSubName ) );
}

// $Barnyard: FUNCTION 006b2b20
const PPropertyValue* PProperties::GetProperty( const Toshi::TPString8& a_rcName, TINT a_iIndex ) const
{
	// The index is used as the sub name
	TString8 strIndex;
	strIndex.Format( "%i", a_iIndex );

	return GetProperty( ::PPropertyName( a_rcName, Toshi::TPString8( strIndex.GetString() ) ) );
}

//-----------------------------------------------------------------------------
// Unreferenced in retail (see the note in PProperties.h)
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 006b2a20
PProperties::PProperties( PProperties* a_pParent )
    : m_pParent( a_pParent )
    , m_iNumProperties( 0 )
{
}

// Returns a_pValue if it's of type a_pType
static const PPropertyValue* FilterType( const PPropertyValue* a_pValue, const TClass* a_pType )
{
	if ( a_pValue == TNULL )
		return TNULL;

	return ( a_pValue->GetType() == a_pType ) ? a_pValue : TNULL;
}

// $Barnyard: FUNCTION 006b2cd0
const PPropertyValue* PProperties::GetOptionalProperty( const Toshi::TPString8& a_rcName, const TClass* a_pType ) const
{
	return FilterType( GetProperty( a_rcName ), a_pType );
}

// $Barnyard: FUNCTION 006b2d00
const PPropertyValue* PProperties::GetOptionalProperty( const Toshi::TPString8& a_rcName, const Toshi::TPString8& a_rcSubName, const TClass* a_pType ) const
{
	return FilterType( GetProperty( a_rcName, a_rcSubName ), a_pType );
}

// $Barnyard: FUNCTION 006b2d40
const PPropertyValue* PProperties::GetOptionalProperty( const Toshi::TPString8& a_rcName, TINT a_iIndex, const TClass* a_pType ) const
{
	return FilterType( GetProperty( a_rcName, a_iIndex ), a_pType );
}

// $Barnyard: FUNCTION 006b2d80
const PPropertyValue* PProperties::GetRequiredProperty( const Toshi::TPString8& a_rcName, const TClass* a_pType ) const
{
	return FilterType( GetProperty( a_rcName ), a_pType );
}

// $Barnyard: FUNCTION 006b2db0
const PPropertyValue* PProperties::GetRequiredProperty( const Toshi::TPString8& a_rcName, const Toshi::TPString8& a_rcSubName, const TClass* a_pType ) const
{
	return FilterType( GetProperty( a_rcName, a_rcSubName ), a_pType );
}

// $Barnyard: FUNCTION 006b2df0
const PPropertyValue* PProperties::GetRequiredProperty( const Toshi::TPString8& a_rcName, TINT a_iIndex, const TClass* a_pType ) const
{
	return FilterType( GetProperty( a_rcName, a_iIndex ), a_pType );
}

// $Barnyard: FUNCTION 006b2980
PProperties* PProperties::FindProperties( const Toshi::TPString8& a_rcName, Toshi::TDList<PProperty>::Iterator& a_rIt ) const
{
	const PProperty* pProperty = FindProperty( a_rcName, a_rIt );

	if ( pProperty == TNULL )
		return TNULL;

	return pProperty->GetValue().GetProperties();
}

// $Barnyard: FUNCTION 006b2e30
PProperties* PProperties::GetOptionalProperties( const Toshi::TPString8& a_rcName ) const
{
	const PPropertyValue* pValue = GetProperty( a_rcName );

	if ( pValue && pValue->IsType( PPropertyValue::TYPE_PROPS ) )
		return pValue->GetProperties();

	return TNULL;
}

// NOTE: the GetOptionalPropertyValue/GetRequiredPropertyValue overloads all share this code
#define PPROPERTIES_GET_VALUE( FUNC, TYPE, GETTER )                                 \
	TBOOL PProperties::FUNC( TYPE& a_rOut, const Toshi::TPString8& a_rcName ) const \
	{                                                                               \
		const PPropertyValue* pValue = GetProperty( a_rcName );                     \
		return pValue && pValue->GETTER( a_rOut );                                  \
	}

// $Barnyard: FUNCTION 006b31f0
PPROPERTIES_GET_VALUE( GetOptionalPropertyValue, TBOOL, GetValue )
// $Barnyard: FUNCTION 006b3220
PPROPERTIES_GET_VALUE( GetOptionalPropertyValue, TINT, GetValue )
// $Barnyard: FUNCTION 006b3250
PPROPERTIES_GET_VALUE( GetOptionalPropertyValue, TUINT32, GetValue )
// $Barnyard: FUNCTION 006b3280
PPROPERTIES_GET_VALUE( GetOptionalPropertyValue, TFLOAT, GetValue )
// $Barnyard: FUNCTION 006b32b0
PPROPERTIES_GET_VALUE( GetOptionalPropertyValue, const TWCHAR*, GetValue )
// $Barnyard: FUNCTION 006b32e0
PPROPERTIES_GET_VALUE( GetOptionalPropertyValue, Toshi::TPString8, GetValue )
// $Barnyard: FUNCTION 006b3310
PPROPERTIES_GET_VALUE( GetOptionalPropertyValue, Toshi::TVector2, GetTVector2 )
// $Barnyard: FUNCTION 006b3340
PPROPERTIES_GET_VALUE( GetOptionalPropertyValue, Toshi::TVector3, GetTVector3 )
// $Barnyard: FUNCTION 006b3370
PPROPERTIES_GET_VALUE( GetOptionalPropertyValue, Toshi::TVector4, GetTVector4 )
// $Barnyard: FUNCTION 006b33a0
PPROPERTIES_GET_VALUE( GetOptionalPropertyValue, Toshi::TQuaternion, GetTQuaternion )
// $Barnyard: FUNCTION 006b33d0
PPROPERTIES_GET_VALUE( GetRequiredPropertyValue, TBOOL, GetValue )
// $Barnyard: FUNCTION 006b3400
PPROPERTIES_GET_VALUE( GetRequiredPropertyValue, TINT, GetValue )
// $Barnyard: FUNCTION 006b3430
PPROPERTIES_GET_VALUE( GetRequiredPropertyValue, TUINT32, GetValue )
// $Barnyard: FUNCTION 006b3460
PPROPERTIES_GET_VALUE( GetRequiredPropertyValue, TFLOAT, GetValue )
// $Barnyard: FUNCTION 006b3490
PPROPERTIES_GET_VALUE( GetRequiredPropertyValue, const TWCHAR*, GetValue )
// $Barnyard: FUNCTION 006b34c0
PPROPERTIES_GET_VALUE( GetRequiredPropertyValue, Toshi::TPString8, GetValue )
// $Barnyard: FUNCTION 006b34f0
PPROPERTIES_GET_VALUE( GetRequiredPropertyValue, Toshi::TVector2, GetTVector2 )
// $Barnyard: FUNCTION 006b3520
PPROPERTIES_GET_VALUE( GetRequiredPropertyValue, Toshi::TVector3, GetTVector3 )
// $Barnyard: FUNCTION 006b3550
PPROPERTIES_GET_VALUE( GetRequiredPropertyValue, Toshi::TVector4, GetTVector4 )
// $Barnyard: FUNCTION 006b3580
PPROPERTIES_GET_VALUE( GetRequiredPropertyValue, Toshi::TQuaternion, GetTQuaternion )

#undef PPROPERTIES_GET_VALUE

//-----------------------------------------------------------------------------
// NOTE: retail uses a TArray of name/property pairs kept sorted by name (the name
// of the container is unknown), only PProperties::Sort uses it.
//-----------------------------------------------------------------------------
struct PPropertySortEntry
{
	// $Barnyard: FUNCTION 006b3a60
	PPropertySortEntry() {}

	// $Barnyard: FUNCTION 006b3a70
	~PPropertySortEntry() {}

	::PPropertyName oName;     // 0x0
	PProperty*      pProperty; // 0x8
};

class PPropertySortedArray
{
public:
	PPropertySortedArray( TINT a_iGrowSize )
	    : m_iGrowSize( a_iGrowSize )
	    , m_iNumEntries( 0 )
	    , m_iNumAllocEntries( 0 )
	    , m_pEntries( TNULL )
	{}

	~PPropertySortedArray()
	{
		if ( m_pEntries )
			delete[] m_pEntries;
	}

	// Returns the index a_rcName should be inserted at (after the equal names)
	TINT FindInsertIndex( const ::PPropertyName& a_rcName, TBOOL& a_rbFound ) const;

	// Inserts a_iCount empty entries at a_iIndex
	void InsertEmpty( TINT a_iIndex, TINT a_iCount );

	void Insert( const ::PPropertyName& a_rcName, PProperty* a_pProperty );

	TINT       GetNumEntries() const { return m_iNumEntries; }
	PProperty* GetProperty( TINT a_iIndex ) const { return m_pEntries[ a_iIndex ].pProperty; }

private:
	// Compares the entry name with a_rcName, an empty sub name is less than any other
	static TINT Compare( const ::PPropertyName& a_rcEntryName, const ::PPropertyName& a_rcName )
	{
		TINT iResult = a_rcEntryName.GetName().GetString8().Compare( a_rcName.GetName().GetString() );

		if ( iResult == 0 )
		{
			if ( a_rcEntryName.GetSubName().GetString8().Length() == 0 )
				return -1;

			if ( a_rcName.GetSubName().GetString8().Length() == 0 )
				return 1;

			iResult = a_rcEntryName.GetSubName().GetString8().Compare( a_rcName.GetSubName().GetString() );
		}

		return iResult;
	}

private:
	TINT                m_iGrowSize;        // 0x0
	TINT                m_iNumEntries;      // 0x4
	TINT                m_iNumAllocEntries; // 0x8
	PPropertySortEntry* m_pEntries;         // 0xC
};

// $Barnyard: FUNCTION 006b3880
TINT PPropertySortedArray::FindInsertIndex( const ::PPropertyName& a_rcName, TBOOL& a_rbFound ) const
{
	TINT iLow  = 0;
	TINT iHigh = m_iNumEntries - 1;
	a_rbFound  = TFALSE;

	while ( iLow <= iHigh )
	{
		TINT iMiddle = ( iLow + iHigh ) / 2;
		TINT iResult = Compare( m_pEntries[ iMiddle ].oName, a_rcName );

		if ( iResult > 0 )
		{
			iHigh = iMiddle - 1;
		}
		else if ( iResult < 0 )
		{
			iLow = iMiddle + 1;
		}
		else
		{
			// Skip all the entries with the same name
			TINT iIndex = iMiddle + 1;

			while ( iIndex <= iHigh )
			{
				const ::PPropertyName& rcEntryName = m_pEntries[ iIndex ].oName;

				if ( rcEntryName.GetName().GetString8().Compare( a_rcName.GetName().GetString() ) != 0 ) break;
				if ( rcEntryName.GetSubName().GetString8().Length() == 0 ) break;
				if ( a_rcName.GetSubName().GetString8().Length() == 0 ) break;
				if ( rcEntryName.GetSubName().GetString8().Compare( a_rcName.GetSubName().GetString() ) != 0 ) break;

				iIndex++;
			}

			a_rbFound = TTRUE;
			return iIndex;
		}
	}

	return iLow;
}

// $Barnyard: FUNCTION 006b3b20
void PPropertySortedArray::InsertEmpty( TINT a_iIndex, TINT a_iCount )
{
	if ( m_iNumEntries + a_iCount <= m_iNumAllocEntries )
	{
		// Enough space, move the entries after a_iIndex
		for ( TINT i = m_iNumEntries - 1; i >= a_iIndex; i-- )
		{
			m_pEntries[ i + a_iCount ].oName     = m_pEntries[ i ].oName;
			m_pEntries[ i + a_iCount ].pProperty = m_pEntries[ i ].pProperty;
		}

		m_iNumEntries += a_iCount;
		return;
	}

	TINT                iNumAlloc   = m_iNumAllocEntries + ( ( a_iCount < m_iGrowSize ) ? m_iGrowSize : a_iCount );
	PPropertySortEntry* pNewEntries = new PPropertySortEntry[ iNumAlloc ];

	if ( m_pEntries )
	{
		for ( TINT i = 0; i < a_iIndex; i++ )
		{
			pNewEntries[ i ].oName     = m_pEntries[ i ].oName;
			pNewEntries[ i ].pProperty = m_pEntries[ i ].pProperty;
		}

		for ( TINT i = a_iIndex; i < m_iNumEntries; i++ )
		{
			pNewEntries[ i + a_iCount ].oName     = m_pEntries[ i ].oName;
			pNewEntries[ i + a_iCount ].pProperty = m_pEntries[ i ].pProperty;
		}

		delete[] m_pEntries;
	}

	m_pEntries         = pNewEntries;
	m_iNumAllocEntries = iNumAlloc;
	m_iNumEntries += a_iCount;
}

// $Barnyard: FUNCTION 006b3cd0
void PPropertySortedArray::Insert( const ::PPropertyName& a_rcName, PProperty* a_pProperty )
{
	::PPropertyName oName( a_rcName );

	TBOOL bFound;
	TINT  iIndex = FindInsertIndex( oName, bFound );
	InsertEmpty( iIndex, 1 );

	m_pEntries[ iIndex ].oName     = oName;
	m_pEntries[ iIndex ].pProperty = a_pProperty;
}

// $Barnyard: FUNCTION 006b37c0
void PProperties::Sort()
{
	PPropertySortedArray oSorted( m_iNumProperties );

	while ( !m_llProperties.IsEmpty() )
	{
		PProperty* pProperty = m_llProperties.Head();
		pProperty->Remove();

		oSorted.Insert( pProperty->GetName(), pProperty );
	}

	for ( TINT i = 0; i < oSorted.GetNumEntries(); i++ )
		m_llProperties.InsertTail( oSorted.GetProperty( i ) );
}

// $Barnyard: FUNCTION 006b2f00
PProperty::PProperty( const PPropertyName& a_rcName, const PPropertyValue& a_rcValue )
    : m_oName( a_rcName )
    , m_oValue( a_rcValue )
{
}

// $Barnyard: FUNCTION 006b39f0
PProperty::~PProperty()
{
	// NOTE: the node unlinks itself in its destructor
}
