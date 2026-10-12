#include "ToshiPCH.h"
#include "PPropertyValue.h"
#include "PPropertyName.h"
#include "PPropertyValueArray.h"

#include "T2Locale/T2Locale.h"
#include "Toshi/T2String16.h"

#include "Math/TVector2.h"
#include "Math/TVector3.h"
#include "Math/TVector4.h"
#include "Math/TQuaternion.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

// NOTE: retail TClass::IsA (006bbd60) tolerates a TNULL class, ours is a member function
static TBOOL IsTypeA( const TClass* a_pType, const TClass* a_pBase )
{
	return a_pType != TNULL && a_pType->IsA( a_pBase );
}

// $Barnyard: FUNCTION 006af0f0
PPropertyValue::PPropertyValue()
{
	m_pType         = TYPE_UNDEF;
	m_uValue.iValue = 0;
}

// $Barnyard: FUNCTION 006af110
PPropertyValue::PPropertyValue( TINT a_iValue )
{
	m_pType         = TYPE_INT;
	m_uValue.iValue = a_iValue;
}

// $Barnyard: FUNCTION 006af130
PPropertyValue::PPropertyValue( TUINT32 a_uiValue )
{
	m_pType          = TYPE_UINT32;
	m_uValue.uiValue = a_uiValue;
}

// $Barnyard: FUNCTION 006af190
PPropertyValue::PPropertyValue( TFLOAT a_fValue )
{
	m_pType         = TYPE_FLOAT;
	m_uValue.fValue = a_fValue;
}

// $Barnyard: FUNCTION 006af1b0
PPropertyValue::PPropertyValue( TBOOL a_bValue )
{
	m_pType         = TYPE_BOOL;
	m_uValue.bValue = a_bValue;
}

// $Barnyard: FUNCTION 006af1d0
PPropertyValue::PPropertyValue( TWCHAR* a_wszValue, TBOOL a_bOwned )
{
	// Owned strings are freed with the value
	m_pType           = a_bOwned ? TYPE_STRING16A : TYPE_STRING16;
	m_uValue.wszValue = a_wszValue;
}

// $Barnyard: FUNCTION 006afab0
PPropertyValue::PPropertyValue( PProperties* a_pProperties )
{
	m_pType           = TYPE_PROPS;
	m_uValue.pData    = TNULL;
	GetPropertiesMP() = a_pProperties;
}

// $Barnyard: FUNCTION 006af200
PPropertyValue::PPropertyValue( const PPropertyName& a_rcName )
{
	m_pType        = TYPE_PROPNAME;
	m_uValue.pName = new PPropertyName( a_rcName );
}

// $Barnyard: FUNCTION 006b06c0
PPropertyValue::PPropertyValue( const TPString8& a_rcValue )
{
	m_pType        = TYPE_TPSTRING8;
	m_uValue.pData = TNULL;
	GetTPString8() = a_rcValue;
}

// $Barnyard: FUNCTION 006b0350
PPropertyValue::PPropertyValue( PPropertyValueArray* a_pArray )
{
	m_pType          = TYPE_ARRAY;
	m_uValue.pData   = TNULL;
	GetPropArrayMP() = a_pArray;
}

// $Barnyard: FUNCTION 006af240
PPropertyValue::PPropertyValue( TObject* a_pObject )
{
	if ( a_pObject )
	{
		m_pType          = a_pObject->GetClass();
		m_uValue.pObject = a_pObject;
	}
	else
	{
		m_pType          = TYPE_UNDEF;
		m_uValue.pObject = TNULL;
	}
}

// $Barnyard: FUNCTION 006b1270
PPropertyValue::PPropertyValue( const PPropertyValue& a_rcOther )
{
	m_pType = TYPE_UNDEF;
	Assign( a_rcOther );
}

// $Barnyard: FUNCTION 006b1290
// $Barnyard: FUNCTION 004014f0
PPropertyValue::~PPropertyValue()
{
	if ( m_pType == TYPE_STRING16A )
	{
		TFree( m_uValue.pData );
		return;
	}

	ReleaseValue();
}

// $Barnyard: FUNCTION 006b13b0
PPropertyValue& PPropertyValue::operator=( const PPropertyValue& a_rcOther )
{
	Assign( a_rcOther );
	return *this;
}

// $Barnyard: FUNCTION 006b07c0
TBOOL PPropertyValue::operator==( const PPropertyValue& a_rcOther ) const
{
	if ( !a_rcOther.CanBeType( m_pType ) )
	{
		// Objects of different types are still equal if it's the same object
		return IsTypeA( m_pType, &TGetClass( TObject ) ) && IsTypeA( a_rcOther.m_pType, &TGetClass( TObject ) ) && m_uValue.pData == a_rcOther.m_uValue.pData;
	}

	const TClass* pType = m_pType;

	if ( pType == TYPE_UNDEF )
		return TTRUE;

	if ( pType == TYPE_TPSTRING8 )
		return GetTPString8() == a_rcOther.GetTPString8();

	if ( pType == TYPE_STRING16 || pType == TYPE_STRING16A || pType == TYPE_LOCALEID )
		return TStringManager::String16Compare( GetString16(), a_rcOther.GetString16(), TSIZE( -1 ) ) == 0;

	if ( pType == TYPE_INT || pType == TYPE_UINT32 )
		return m_uValue.iValue == a_rcOther.m_uValue.iValue;

	if ( pType == TYPE_FLOAT )
		return m_uValue.fValue == a_rcOther.m_uValue.fValue;

	if ( pType == TYPE_BOOL )
		return m_uValue.bValue == a_rcOther.m_uValue.bValue;

	if ( pType == TYPE_PROPS )
		return GetProperties() == a_rcOther.GetProperties();

	if ( pType == TYPE_PROPNAME )
		return *m_uValue.pName == *a_rcOther.m_uValue.pName;

	return m_uValue.pData == a_rcOther.m_uValue.pData;
}

// $Barnyard: FUNCTION 006b0930
TBOOL PPropertyValue::operator!=( const PPropertyValue& a_rcOther ) const
{
	return !( *this == a_rcOther );
}

// $Barnyard: FUNCTION 006b0950
TBOOL PPropertyValue::operator<=( const PPropertyValue& a_rcOther ) const
{
	if ( !a_rcOther.CanBeType( m_pType ) )
	{
		// Objects of different types are compared by their address
		return IsTypeA( m_pType, &TGetClass( TObject ) ) && IsTypeA( a_rcOther.m_pType, &TGetClass( TObject ) ) && m_uValue.pData <= a_rcOther.m_uValue.pData;
	}

	const TClass* pType = m_pType;

	if ( pType == TYPE_UNDEF )
		return TTRUE;

	// NOTE: retail compares the addresses of the pooled strings
	if ( pType == TYPE_TPSTRING8 )
		return GetTPString8().GetString() <= a_rcOther.GetTPString8().GetString();

	if ( pType == TYPE_STRING16 || pType == TYPE_STRING16A || pType == TYPE_LOCALEID )
		return TStringManager::String16Compare( GetString16(), a_rcOther.GetString16(), TSIZE( -1 ) ) <= 0;

	if ( pType == TYPE_INT )
		return m_uValue.iValue <= a_rcOther.GetInteger();

	if ( pType == TYPE_UINT32 )
		return m_uValue.uiValue <= a_rcOther.GetUINT32();

	if ( pType == TYPE_FLOAT )
		return m_uValue.fValue <= a_rcOther.GetFloat();

	if ( pType == TYPE_BOOL )
		return TUINT8( m_uValue.bValue ) <= TUINT8( a_rcOther.GetBoolean() );

	if ( pType == TYPE_PROPS )
		return GetProperties() <= a_rcOther.GetProperties();

	// NOTE: retail compares the addresses of the pooled names
	if ( pType == TYPE_PROPNAME )
		return m_uValue.pName->GetName().GetString() <= a_rcOther.m_uValue.pName->GetName().GetString();

	return m_uValue.pData <= a_rcOther.m_uValue.pData;
}

// $Barnyard: FUNCTION 006b0b60
TBOOL PPropertyValue::operator<( const PPropertyValue& a_rcOther ) const
{
	if ( !a_rcOther.CanBeType( m_pType ) )
	{
		// Objects of different types are compared by their address
		return IsTypeA( m_pType, &TGetClass( TObject ) ) && IsTypeA( a_rcOther.m_pType, &TGetClass( TObject ) ) && m_uValue.pData < a_rcOther.m_uValue.pData;
	}

	const TClass* pType = m_pType;

	if ( pType == TYPE_UNDEF )
		return TTRUE;

	// NOTE: retail compares the addresses of the pooled strings
	if ( pType == TYPE_TPSTRING8 )
		return GetTPString8().GetString() < a_rcOther.GetTPString8().GetString();

	if ( pType == TYPE_STRING16 || pType == TYPE_STRING16A || pType == TYPE_LOCALEID )
		return TStringManager::String16Compare( GetString16(), a_rcOther.GetString16(), TSIZE( -1 ) ) < 0;

	if ( pType == TYPE_INT )
		return m_uValue.iValue < a_rcOther.GetInteger();

	if ( pType == TYPE_UINT32 )
		return m_uValue.uiValue < a_rcOther.GetUINT32();

	if ( pType == TYPE_FLOAT )
		return m_uValue.fValue < a_rcOther.GetFloat();

	if ( pType == TYPE_BOOL )
		return TUINT8( m_uValue.bValue ) < TUINT8( a_rcOther.GetBoolean() );

	if ( pType == TYPE_PROPS )
		return GetProperties() < a_rcOther.GetProperties();

	// NOTE: retail compares the addresses of the pooled names
	if ( pType == TYPE_PROPNAME )
		return m_uValue.pName->GetName().GetString() < a_rcOther.m_uValue.pName->GetName().GetString();

	return m_uValue.pData < a_rcOther.m_uValue.pData;
}

// $Barnyard: FUNCTION 006b0d80
TBOOL PPropertyValue::operator>=( const PPropertyValue& a_rcOther ) const
{
	return !( *this < a_rcOther );
}

// $Barnyard: FUNCTION 006b0da0
TBOOL PPropertyValue::operator>( const PPropertyValue& a_rcOther ) const
{
	return !( *this <= a_rcOther );
}

// $Barnyard: FUNCTION 006af3f0
TBOOL PPropertyValue::operator==( const TObject* a_pObject ) const
{
	return m_pType == a_pObject->GetClass() && m_uValue.pObject == a_pObject;
}

// $Barnyard: FUNCTION 006af440
TBOOL PPropertyValue::CanBeType( const TClass* a_pType ) const
{
	if ( a_pType == TYPE_UNDEF )
		return TTRUE;

	const TClass* pType = m_pType;

	if ( pType == TYPE_INT || pType == TYPE_UINT32 )
	{
		// Integers can be floats as long as they don't lose precision
		return a_pType == TYPE_INT || a_pType == TYPE_UINT32 || ( a_pType == TYPE_FLOAT && TFLOAT( m_uValue.iValue ) < 1e+08f );
	}

	if ( pType == TYPE_FLOAT )
	{
		if ( a_pType == TYPE_FLOAT )
			return TTRUE;

		return a_pType == TYPE_INT && m_uValue.fValue == TFLOAT( TINT( m_uValue.fValue ) );
	}

	if ( pType == TYPE_BOOL )
		return a_pType == TYPE_BOOL || a_pType == TYPE_INT || a_pType == TYPE_FLOAT;

	if ( pType == TYPE_TPSTRING8 )
		return a_pType == TYPE_TPSTRING8;

	if ( pType == TYPE_STRING16 )
		return a_pType == TYPE_STRING16 || a_pType == TYPE_STRING16A || a_pType == TYPE_LOCALEID;

	if ( a_pType == pType || IsTypeA( pType, a_pType ) )
		return TTRUE;

	// Empty values can be of any type
	return m_uValue.pData == TNULL;
}

// $Barnyard: FUNCTION 006af7a0
TBOOL PPropertyValue::GetValue( TINT& a_rOut ) const
{
	if ( !CanBeType( TYPE_INT ) )
		return TFALSE;

	a_rOut = GetInteger();
	return TTRUE;
}

// $Barnyard: FUNCTION 006af810
TBOOL PPropertyValue::GetValue( TUINT32& a_rOut ) const
{
	if ( !CanBeType( TYPE_UINT32 ) )
		return TFALSE;

	a_rOut = GetUINT32();
	return TTRUE;
}

// $Barnyard: FUNCTION 006af960
TBOOL PPropertyValue::GetValue( TFLOAT& a_rOut ) const
{
	if ( !CanBeType( TYPE_FLOAT ) )
		return TFALSE;

	a_rOut = GetFloat();
	return TTRUE;
}

// $Barnyard: FUNCTION 006af9f0
TBOOL PPropertyValue::GetValue( TBOOL& a_rOut ) const
{
	if ( !CanBeType( TYPE_BOOL ) )
		return TFALSE;

	a_rOut = GetBoolean();
	return TTRUE;
}

// $Barnyard: FUNCTION 006afa20
TBOOL PPropertyValue::GetValue( const TWCHAR*& a_rOut ) const
{
	if ( !CanBeType( TYPE_STRING16 ) )
		return TFALSE;

	a_rOut = GetString16();
	return TTRUE;
}

// $Barnyard: FUNCTION 006b0450
TBOOL PPropertyValue::GetValue( TPString8& a_rOut ) const
{
	if ( !CanBeType( TYPE_TPSTRING8 ) )
		return TFALSE;

	a_rOut = GetTPString8();
	return TTRUE;
}

// $Barnyard: FUNCTION 006b04a0
TString8 PPropertyValue::ToString() const
{
	TString8      strResult;
	const TClass* pType = m_pType;

	if ( pType == TYPE_TPSTRING8 )
		strResult = GetTPString8().GetString8();
	else if ( pType == TYPE_STRING16 )
		strResult = m_uValue.wszValue;
	else if ( pType == TYPE_STRING16A || pType == TYPE_LOCALEID )
		strResult = GetString16();
	else if ( pType == TYPE_INT || pType == TYPE_UINT32 )
		strResult.Format( "%i", m_uValue.iValue );
	else if ( pType == TYPE_FLOAT )
		strResult.Format( "%f", m_uValue.fValue );
	else if ( pType == TYPE_BOOL )
		strResult = m_uValue.bValue ? "true" : "false";
	else if ( pType == TYPE_PROPS )
		strResult.Format( "(Properties)%x", GetProperties() );
	else if ( pType == TYPE_PROPNAME )
		strResult.Format( "(PropName)%x", m_uValue.pName );
	else if ( pType == TYPE_ARRAY )
		strResult.Format( "(PropArray size %i)", m_uValue.pArray->GetSize() );
	else if ( pType == TYPE_UNDEF )
		strResult = "null";
	else
		strResult.Format( "(%s)%x", m_uValue.pObject ? pType->GetName() : "TNULL", m_uValue.pObject );

	return strResult;
}

// $Barnyard: FUNCTION 006b10c0
void PPropertyValue::Assign( const PPropertyValue& a_rcOther )
{
	ChangeType( a_rcOther.m_pType );

	const TClass* pType = m_pType;

	if ( pType == TYPE_TPSTRING8 )
	{
		GetTPString8() = a_rcOther.GetTPString8();
		return;
	}

	if ( pType == TYPE_STRING16 )
	{
		const TClass* pOtherType = a_rcOther.m_pType;

		if ( pOtherType == TYPE_STRING16 || pOtherType == TYPE_STRING16A )
			m_uValue.wszValue = a_rcOther.m_uValue.wszValue;
		else if ( pOtherType == TYPE_LOCALEID )
			m_uValue.wszValue = T2Locale::GetSingleton()->GetString( a_rcOther.m_uValue.iValue );
		else
			m_uValue.wszValue = TNULL;

		return;
	}

	if ( pType == TYPE_STRING16A )
	{
		// NOTE: the copy doesn't own the string
		m_pType           = TYPE_STRING16;
		m_uValue.wszValue = const_cast<TWCHAR*>( a_rcOther.GetString16() );
		return;
	}

	if ( pType == TYPE_LOCALEID || pType == TYPE_INT || pType == TYPE_UINT32 || pType == TYPE_FLOAT )
	{
		m_uValue.iValue = a_rcOther.m_uValue.iValue;
		return;
	}

	if ( pType == TYPE_BOOL )
	{
		m_uValue.bValue = a_rcOther.m_uValue.bValue;
		return;
	}

	if ( pType == TYPE_PROPS )
	{
		GetPropertiesMP() = a_rcOther.GetProperties();
		return;
	}

	if ( pType == TYPE_PROPNAME )
	{
		m_uValue.pName = new PPropertyName( *a_rcOther.m_uValue.pName );
		return;
	}

	if ( pType == TYPE_ARRAY )
	{
		GetPropArrayMP() = a_rcOther.m_uValue.pArray;
		return;
	}

	if ( pType == TYPE_UNDEF )
		return;

	if ( IsManagedPtr( pType ) )
	{
		m_uValue.pManaged = a_rcOther.m_uValue.pManaged;

		if ( m_uValue.pManaged )
			m_uValue.pManaged->IncRefCount();

		return;
	}

	m_uValue.pData = a_rcOther.m_uValue.pData;
}

// NOTE: shared by ChangeType (006b0ef0) and the destructor (006b1290), not a separate function in retail.
// String16A values aren't released here, both callers handle them first.
void PPropertyValue::ReleaseValue()
{
	const TClass* pType = m_pType;

	if ( pType == TYPE_UNDEF || pType == TYPE_INT || pType == TYPE_UINT32 || pType == TYPE_FLOAT || pType == TYPE_BOOL || pType == TYPE_STRING16 ||
	     pType == TYPE_STRING16A || pType == TYPE_LOCALEID )
		return;

	if ( pType == TYPE_TPSTRING8 )
	{
		GetTPString8() = static_cast<TPooledString8*>( TNULL );
	}
	else if ( pType == TYPE_PROPS )
	{
		GetPropertiesMP() = static_cast<PProperties*>( TNULL );
	}
	else if ( pType == TYPE_ARRAY )
	{
		// Retail destroys the TRef in place (006b1c50)
		GetPropArrayMP().~TRef();
	}
	else if ( pType == TYPE_PROPNAME )
	{
		if ( m_uValue.pName )
			delete m_uValue.pName;
	}
	else if ( IsManagedPtr( pType ) )
	{
		if ( m_uValue.pManaged && m_uValue.pManaged->DecRefCount() == 1 )
			delete m_uValue.pManaged;
	}
}

// $Barnyard: FUNCTION 006b0ef0
TBOOL PPropertyValue::ChangeType( const TClass* a_pType )
{
	if ( m_pType == TYPE_STRING16A )
	{
		TFree( m_uValue.pData );
		m_uValue.pData = TNULL;
	}

	if ( m_pType == a_pType )
		return TFALSE;

	// Release the old value
	ReleaseValue();

	// Initialise storage for the new type
	if ( a_pType != TYPE_UNDEF && a_pType != TYPE_INT && a_pType != TYPE_UINT32 && a_pType != TYPE_FLOAT && a_pType != TYPE_BOOL &&
	     a_pType != TYPE_STRING16 && a_pType != TYPE_LOCALEID )
	{
		if ( a_pType != TYPE_STRING16A )
		{
			if ( a_pType == TYPE_TPSTRING8 || a_pType == TYPE_PROPS || a_pType == TYPE_ARRAY )
			{
				// In-place construction of the TPString8 / managed pointer
				m_uValue.pData = TNULL;
				m_pType        = a_pType;
				return TTRUE;
			}

			// NOTE: retail checks the *old* type here (m_pType hasn't been updated yet)
			if ( a_pType != TYPE_PROPNAME && !IsManagedPtr( m_pType ) )
			{
				m_pType = a_pType;
				return TTRUE;
			}
		}

		m_uValue.pData = TNULL;
	}

	m_pType = a_pType;
	return TTRUE;
}

// $Barnyard: FUNCTION 006af580
const TClass* PPropertyValue::GetType() const
{
	return m_pType;
}

// $Barnyard: FUNCTION 006af760
TObject* PPropertyValue::GetTObject() const
{
	if ( m_pType == TYPE_UNDEF )
		return TNULL;

	return m_uValue.pObject;
}

// $Barnyard: FUNCTION 006af630
TBOOL PPropertyValue::GetBoolean() const
{
	const TClass* pType = m_pType;

	if ( pType == TYPE_INT || pType == TYPE_UINT32 )
		return m_uValue.iValue != 0;

	if ( pType == TYPE_FLOAT )
		return m_uValue.fValue != 0.0f;

	if ( pType == TYPE_BOOL )
		return m_uValue.bValue;

	return TFALSE;
}

// $Barnyard: FUNCTION 006af590
TINT PPropertyValue::GetInteger() const
{
	const TClass* pType = m_pType;

	if ( pType == TYPE_INT || pType == TYPE_UINT32 )
		return m_uValue.iValue;

	if ( pType == TYPE_FLOAT )
		return TINT( m_uValue.fValue );

	return 0;
}

// $Barnyard: FUNCTION 006af5c0
TUINT32 PPropertyValue::GetUINT32() const
{
	const TClass* pType = m_pType;

	if ( pType == TYPE_INT || pType == TYPE_UINT32 )
		return m_uValue.uiValue;

	if ( pType == TYPE_FLOAT )
		return TUINT32( m_uValue.fValue );

	return 0;
}

// $Barnyard: FUNCTION 006af5f0
TFLOAT PPropertyValue::GetFloat() const
{
	const TClass* pType = m_pType;

	if ( pType == TYPE_INT )
		return TFLOAT( m_uValue.iValue );

	if ( pType == TYPE_UINT32 )
		return TFLOAT( m_uValue.uiValue );

	if ( pType == TYPE_FLOAT )
		return m_uValue.fValue;

	return 0.0f;
}

// $Barnyard: FUNCTION 006af710
const TWCHAR* PPropertyValue::GetString16() const
{
	const TClass* pType = m_pType;

	if ( pType == TYPE_STRING16 || pType == TYPE_STRING16A )
		return m_uValue.wszValue;

	if ( pType == TYPE_LOCALEID )
		return T2Locale::GetSingleton()->GetString( m_uValue.iValue );

	return TNULL;
}

// $Barnyard: FUNCTION 006afb00
PProperties* PPropertyValue::GetProperties() const
{
	if ( m_pType == TYPE_PROPS )
		return m_uValue.pProperties;

	return TNULL;
}

// $Barnyard: FUNCTION 006b03b0
TPString8& PPropertyValue::GetTPString8()
{
	static TPString8 s_strEmpty;

	if ( m_pType != TYPE_TPSTRING8 )
		return s_strEmpty;

	return *TREINTERPRETCAST( TPString8*, &m_uValue );
}

// $Barnyard: FUNCTION 006b0400
const TPString8& PPropertyValue::GetTPString8() const
{
	static TPString8 s_strEmpty;

	if ( m_pType != TYPE_TPSTRING8 )
		return s_strEmpty;

	return *TREINTERPRETCAST( const TPString8*, &m_uValue );
}

// $Barnyard: FUNCTION 006b13d0
void PPropertyValue::SetUndef()
{
	ChangeType( TYPE_UNDEF );
}

// $Barnyard: FUNCTION 006b1620
void PPropertyValue::SetTObject( TObject* a_pObject )
{
	if ( a_pObject == TNULL )
	{
		ChangeType( &TGetClass( TObject ) );
		m_uValue.pObject = a_pObject;
		return;
	}

	ChangeType( a_pObject->GetClass() );
	m_uValue.pObject = a_pObject;
}

// $Barnyard: FUNCTION 006b1440
void PPropertyValue::SetBoolean( TBOOL a_bValue )
{
	ChangeType( TYPE_BOOL );
	m_uValue.bValue = a_bValue;
}

// $Barnyard: FUNCTION 006b13e0
void PPropertyValue::SetInteger( TINT a_iValue )
{
	ChangeType( TYPE_INT );
	m_uValue.iValue = a_iValue;
}

// $Barnyard: FUNCTION 006b1400
void PPropertyValue::SetUINT32( TUINT32 a_uiValue )
{
	ChangeType( TYPE_UINT32 );
	m_uValue.uiValue = a_uiValue;
}

// $Barnyard: FUNCTION 006b1420
void PPropertyValue::SetFloat( TFLOAT a_fValue )
{
	ChangeType( TYPE_FLOAT );
	m_uValue.fValue = a_fValue;
}

// $Barnyard: FUNCTION 006b1460
void PPropertyValue::SetTPString8( const TPString8& a_rValue )
{
	ChangeType( TYPE_TPSTRING8 );
	GetTPString8() = a_rValue;
}

// $Barnyard: FUNCTION 006b1660
void PPropertyValue::SetTObject( const TClass* a_pType, TObject* a_pObject )
{
	ChangeType( a_pType );
	m_uValue.pObject = a_pObject;
}

// $Barnyard: FUNCTION 006b15c0
void PPropertyValue::SetArray( PPropertyValueArray* a_pArray )
{
	ChangeType( TYPE_ARRAY );
	GetPropArrayMP() = a_pArray;
}

// $Barnyard: FUNCTION 006b1730
PPropertyValue& PPropertyValue::operator=( TINT a_iValue )
{
	ChangeType( TYPE_INT );
	m_uValue.iValue = a_iValue;
	return *this;
}

// $Barnyard: FUNCTION 006b1750
PPropertyValue& PPropertyValue::operator=( TUINT32 a_uiValue )
{
	ChangeType( TYPE_UINT32 );
	m_uValue.uiValue = a_uiValue;
	return *this;
}

// $Barnyard: FUNCTION 006b17b0
PPropertyValue& PPropertyValue::operator=( TFLOAT a_fValue )
{
	ChangeType( TYPE_FLOAT );
	m_uValue.fValue = a_fValue;
	return *this;
}

// $Barnyard: FUNCTION 006b17d0
PPropertyValue& PPropertyValue::operator=( const TPString8& a_rcValue )
{
	SetTPString8( a_rcValue );
	return *this;
}

// $Barnyard: FUNCTION 006b18b0
PPropertyValue& PPropertyValue::operator=( const TVector2& a_rcValue )
{
	PPropertyValueArray* pArray = new PPropertyValueArray();
	pArray->SetSize( 2 );
	*pArray->GetValue( 0 ) = a_rcValue.x;
	*pArray->GetValue( 1 ) = a_rcValue.y;

	SetArray( pArray );
	return *this;
}

// $Barnyard: FUNCTION 006b1950
PPropertyValue& PPropertyValue::operator=( const TVector3& a_rcValue )
{
	PPropertyValueArray* pArray = new PPropertyValueArray();
	pArray->SetSize( 3 );
	*pArray->GetValue( 0 ) = a_rcValue.x;
	*pArray->GetValue( 1 ) = a_rcValue.y;
	*pArray->GetValue( 2 ) = a_rcValue.z;

	SetArray( pArray );
	return *this;
}

// $Barnyard: FUNCTION 006b1a10
PPropertyValue& PPropertyValue::operator=( const TVector4& a_rcValue )
{
	PPropertyValueArray* pArray = new PPropertyValueArray();
	pArray->SetSize( 4 );
	*pArray->GetValue( 0 ) = a_rcValue.x;
	*pArray->GetValue( 1 ) = a_rcValue.y;
	*pArray->GetValue( 2 ) = a_rcValue.z;
	*pArray->GetValue( 3 ) = a_rcValue.w;

	SetArray( pArray );
	return *this;
}

// $Barnyard: FUNCTION 006b02d0
TBOOL PPropertyValue::GetTVector2( TVector2& a_rOut ) const
{
	if ( m_pType == TYPE_ARRAY )
		return m_uValue.pArray->GetTVector2( a_rOut );

	return TFALSE;
}

// $Barnyard: FUNCTION 006b02f0
TBOOL PPropertyValue::GetTVector3( TVector3& a_rOut ) const
{
	if ( m_pType == TYPE_ARRAY )
		return m_uValue.pArray->GetTVector3( a_rOut );

	return TFALSE;
}

// $Barnyard: FUNCTION 006b0310
TBOOL PPropertyValue::GetTVector4( TVector4& a_rOut ) const
{
	if ( m_pType == TYPE_ARRAY )
		return m_uValue.pArray->GetTVector4( a_rOut );

	return TFALSE;
}

// $Barnyard: FUNCTION 006b0330
TBOOL PPropertyValue::GetTQuaternion( TQuaternion& a_rOut ) const
{
	if ( m_pType == TYPE_ARRAY )
		return m_uValue.pArray->GetTQuaternion( a_rOut );

	return TFALSE;
}

//-----------------------------------------------------------------------------
// Unreferenced in retail (see the note in PPropertyValue.h)
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 006af150
PPropertyValue::PPropertyValue( TINT8 a_iValue )
{
	m_pType         = TYPE_INT;
	m_uValue.iValue = a_iValue;
}

// $Barnyard: FUNCTION 006af170
PPropertyValue::PPropertyValue( TUINT8 a_uiValue )
{
	m_pType          = TYPE_UINT32;
	m_uValue.uiValue = a_uiValue;
}

// $Barnyard: FUNCTION 006b1af0
PPropertyValue::PPropertyValue( const TVector2& a_rcValue )
{
	m_pType         = TYPE_UNDEF;
	m_uValue.iValue = 0;
	*this           = a_rcValue;
}

// $Barnyard: FUNCTION 006b1b20
PPropertyValue::PPropertyValue( const TVector3& a_rcValue )
{
	m_pType         = TYPE_UNDEF;
	m_uValue.iValue = 0;
	*this           = a_rcValue;
}

// $Barnyard: FUNCTION 006b1b50
PPropertyValue::PPropertyValue( const TVector4& a_rcValue )
{
	m_pType         = TYPE_UNDEF;
	m_uValue.iValue = 0;
	*this           = a_rcValue;
}

// $Barnyard: FUNCTION 006b1770
PPropertyValue& PPropertyValue::operator=( TINT8 a_iValue )
{
	ChangeType( TYPE_INT );
	m_uValue.iValue = a_iValue;
	return *this;
}

// $Barnyard: FUNCTION 006b1790
PPropertyValue& PPropertyValue::operator=( TUINT8 a_uiValue )
{
	ChangeType( TYPE_UINT32 );
	m_uValue.uiValue = a_uiValue;
	return *this;
}

// $Barnyard: FUNCTION 006b17f0
PPropertyValue& PPropertyValue::operator=( const TWCHAR* a_wszValue )
{
	ChangeType( TYPE_STRING16 );
	m_uValue.wszValue = const_cast<TWCHAR*>( a_wszValue );
	return *this;
}

// $Barnyard: FUNCTION 006b1810
PPropertyValue& PPropertyValue::operator=( PProperties* a_pProperties )
{
	ChangeType( TYPE_PROPS );
	GetPropertiesMP() = a_pProperties;
	return *this;
}

// $Barnyard: FUNCTION 006b1850
PPropertyValue& PPropertyValue::operator=( const PPropertyName& a_rcName )
{
	ChangeType( TYPE_PROPNAME );
	m_uValue.pName = new PPropertyName( a_rcName );
	return *this;
}

// $Barnyard: FUNCTION 006b1890
PPropertyValue& PPropertyValue::operator=( PPropertyValueArray* a_pArray )
{
	SetArray( a_pArray );
	return *this;
}

// $Barnyard: FUNCTION 006af270
TBOOL PPropertyValue::operator==( TINT a_iValue ) const
{
	return m_pType == TYPE_INT && m_uValue.iValue == a_iValue;
}

// $Barnyard: FUNCTION 006af290
TBOOL PPropertyValue::operator==( TUINT32 a_uiValue ) const
{
	return m_pType == TYPE_UINT32 && m_uValue.uiValue == a_uiValue;
}

// $Barnyard: FUNCTION 006af2b0
TBOOL PPropertyValue::operator==( TINT8 a_iValue ) const
{
	return m_pType == TYPE_INT && m_uValue.iValue == a_iValue;
}

// $Barnyard: FUNCTION 006af2e0
TBOOL PPropertyValue::operator==( TUINT8 a_uiValue ) const
{
	return m_pType == TYPE_UINT32 && m_uValue.uiValue == a_uiValue;
}

// $Barnyard: FUNCTION 006af310
TBOOL PPropertyValue::operator==( TFLOAT a_fValue ) const
{
	return m_pType == TYPE_FLOAT && m_uValue.fValue == a_fValue;
}

// $Barnyard: FUNCTION 006af340
TBOOL PPropertyValue::operator==( TBOOL a_bValue ) const
{
	return m_pType == TYPE_BOOL && m_uValue.bValue == a_bValue;
}

// $Barnyard: FUNCTION 006af360
TBOOL PPropertyValue::operator==( const TWCHAR* a_wszValue ) const
{
	const TWCHAR* wszString;

	if ( m_pType == TYPE_STRING16 || m_pType == TYPE_STRING16A )
		wszString = m_uValue.wszValue;
	else if ( m_pType == TYPE_LOCALEID )
		wszString = T2Locale::GetSingleton()->GetString( m_uValue.iValue );
	else
		return TFALSE;

	return wszString != TNULL && T2String16::Compare( wszString, a_wszValue, -1 ) == 0;
}

// $Barnyard: FUNCTION 006af3c0
TBOOL PPropertyValue::operator==( const PPropertyName& a_rcName ) const
{
	return m_pType == TYPE_PROPNAME && *m_uValue.pName == a_rcName;
}

// $Barnyard: FUNCTION 006afb80
TBOOL PPropertyValue::operator==( const PProperties* a_pProperties ) const
{
	return m_pType == TYPE_PROPS && m_uValue.pProperties == a_pProperties;
}

// $Barnyard: FUNCTION 006afba0
TBOOL PPropertyValue::operator==( const PPropertyValueArray* a_pArray ) const
{
	return m_pType == TYPE_ARRAY && m_uValue.pArray == a_pArray;
}

// $Barnyard: FUNCTION 006b0750
TBOOL PPropertyValue::operator==( const TPString8& a_rcValue ) const
{
	return m_pType == TYPE_TPSTRING8 && GetTPString8() == a_rcValue;
}

// $Barnyard: FUNCTION 006b0780
TBOOL PPropertyValue::operator==( const TCHAR* a_szValue ) const
{
	return m_pType == TYPE_TPSTRING8 && GetTPString8().GetString8().Compare( a_szValue, -1 ) == 0;
}

// $Barnyard: FUNCTION 006af420
TBOOL PPropertyValue::IsDefined() const
{
	return m_pType != TYPE_UNDEF;
}

// $Barnyard: FUNCTION 006b0dc0
TBOOL PPropertyValue::IsEmpty() const
{
	const TClass* pType = m_pType;

	if ( pType == TYPE_TPSTRING8 )
		return GetTPString8().GetString8().Length() == 0;

	if ( pType == TYPE_STRING16 )
		return m_uValue.wszValue == TNULL || *GetString16() == L'\0';

	if ( pType == TYPE_STRING16A || pType == TYPE_LOCALEID )
		return GetString16() == TNULL || *GetString16() == L'\0';

	if ( pType == TYPE_INT || pType == TYPE_UINT32 || pType == TYPE_FLOAT || pType == TYPE_BOOL )
		return TFALSE;

	if ( pType == TYPE_PROPS )
		return GetProperties() == TNULL;

	if ( pType == TYPE_PROPNAME )
		return m_uValue.pName == TNULL || m_uValue.pName->GetName().GetString8().Length() == 0;

	if ( pType == TYPE_ARRAY )
		return m_uValue.pArray == TNULL || m_uValue.pArray->GetSize() == 0;

	return pType == TYPE_UNDEF || m_uValue.pData == TNULL;
}

// $Barnyard: FUNCTION 006af880
TBOOL PPropertyValue::GetValue( TINT8& a_rOut ) const
{
	if ( !CanBeType( TYPE_INT ) )
		return TFALSE;

	a_rOut = TINT8( GetInteger() );
	return TTRUE;
}

// $Barnyard: FUNCTION 006af8f0
TBOOL PPropertyValue::GetValue( TUINT8& a_rOut ) const
{
	if ( !CanBeType( TYPE_UINT32 ) )
		return TFALSE;

	a_rOut = TUINT8( GetInteger() );
	return TTRUE;
}

// $Barnyard: FUNCTION 006afa90
const TCHAR* PPropertyValue::ToString( const TClass* a_pType )
{
	if ( a_pType == TYPE_UNDEF )
		return "TNULL";

	return a_pType->GetName();
}

// $Barnyard: FUNCTION 006af6c0
TWCHAR* PPropertyValue::GetString16()
{
	const TClass* pType = m_pType;

	if ( pType == TYPE_STRING16 || pType == TYPE_STRING16A )
		return m_uValue.wszValue;

	if ( pType == TYPE_LOCALEID )
		return T2Locale::GetSingleton()->GetString( m_uValue.iValue );

	return TNULL;
}

// $Barnyard: FUNCTION 006af700
TINT PPropertyValue::GetLocaleID() const
{
	return m_uValue.iValue;
}

// $Barnyard: FUNCTION 006af750
const PPropertyName& PPropertyValue::GetPropertyName() const
{
	return *m_uValue.pName;
}

// $Barnyard: FUNCTION 006afb20
PPropertyValueArray* PPropertyValue::GetArray() const
{
	return m_uValue.pArray;
}

// $Barnyard: FUNCTION 006b14e0
void PPropertyValue::SetString16( TWCHAR* a_wszValue )
{
	ChangeType( TYPE_STRING16 );
	m_uValue.wszValue = a_wszValue;
}

// $Barnyard: FUNCTION 006b1500
void PPropertyValue::SetString16A( TWCHAR* a_wszValue )
{
	ChangeType( TYPE_STRING16A );
	m_uValue.wszValue = a_wszValue;
}

// $Barnyard: FUNCTION 006b1520
void PPropertyValue::SetLocaleID( TINT a_iLocaleID )
{
	ChangeType( TYPE_LOCALEID );
	m_uValue.iValue = a_iLocaleID;
}

// $Barnyard: FUNCTION 006b1540
void PPropertyValue::SetProperties( PProperties* a_pProperties )
{
	ChangeType( TYPE_PROPS );
	GetPropertiesMP() = a_pProperties;
}

// $Barnyard: FUNCTION 006b1580
void PPropertyValue::SetPropertyName( const PPropertyName& a_rcName )
{
	ChangeType( TYPE_PROPNAME );
	m_uValue.pName = new PPropertyName( a_rcName );
}

// $Barnyard: FUNCTION 006b1680
void PPropertyValue::SetManagedPtr( const TClass* a_pType, void* a_pData )
{
	ChangeType( a_pType );
	m_uValue.pData = a_pData;
}
