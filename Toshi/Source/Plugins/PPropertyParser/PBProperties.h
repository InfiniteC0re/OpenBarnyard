#pragma once
//-----------------------------------------------------------------------------
// This plugin allows reading PProperties from it's binary format which is used
// in TRB files. Before bnary format was introduced in TOSHI 2.0, PProperties
// were stored in .ini files and parsed using TFileLexer.
// Use XML2PProperties to create TRB files from XML/JSON input and backwards.
//-----------------------------------------------------------------------------

#include "Toshi/Toshi.h"
#include "Toshi/TStringManager.h"
#include "Toshi/TString8.h"
#include "Math/TVector2.h"
#include "Math/TVector3.h"
#include "Math/TVector4.h"
#include "Math/TQuaternion.h"

struct PBPropertyLocaleString
{
	constexpr PBPropertyLocaleString()
	    : uiIndex( -1 )
	{}

	constexpr PBPropertyLocaleString( TUINT a_uiIndex )
	    : uiIndex( a_uiIndex )
	{}

	PBPropertyLocaleString& operator=( TUINT a_uiIndex )
	{
		uiIndex = a_uiIndex;
		return *this;
	}

	TUINT uiIndex;
};

class PBPropertyValue
{
public:
	enum class Type
	{
		Int,
		UInt32,
		Float,
		Bool,
		Properties,
		PropertyName,
		Array,
		String,
		LocaleString
	};

	friend class PPropertiesWriter;
	friend class PBPropertyReader;

public:
	PBPropertyValue()
	{
		m_eType          = Type::Int;
		m_uValue.Pointer = TNULL;
	}

	PBPropertyValue( TINT a_iValue )
	{
		m_eType      = Type::Int;
		m_uValue.Int = a_iValue;
	}

	PBPropertyValue( TBOOL a_bValue )
	{
		m_eType       = Type::Bool;
		m_uValue.Bool = a_bValue;
	}

	PBPropertyValue( TFLOAT a_fValue )
	{
		m_eType        = Type::Float;
		m_uValue.Float = a_fValue;
	}

	PBPropertyValue( TUINT32 a_uiValue )
	{
		m_eType         = Type::UInt32;
		m_uValue.UInt32 = a_uiValue;
	}

	PBPropertyValue( const TCHAR* a_pchValue )
	{
		m_eType         = Type::String;
		m_uValue.String = new TCHAR[ Toshi::TStringManager::String8Length( a_pchValue ) + 1 ];
		Toshi::TStringManager::String8Copy( m_uValue.String, a_pchValue );
	}

	PBPropertyValue( const Toshi::TString8& a_sValue )
	{
		m_eType         = Type::String;
		m_uValue.String = new TCHAR[ a_sValue.Length() + 1 ];
		Toshi::TStringManager::String8Copy( m_uValue.String, a_sValue );
	}

	PBPropertyValue( PBPropertyLocaleString a_strValue )
	{
		m_eType         = Type::LocaleString;
		m_uValue.UInt32 = a_strValue.uiIndex;
	}

	PBPropertyValue( class PBProperties* a_pProperties )
	{
		m_eType             = Type::Properties;
		m_uValue.Properties = a_pProperties;
	}

	PBPropertyValue( class PBPropertyValueArray* a_pArray )
	{
		m_eType        = Type::Array;
		m_uValue.Array = a_pArray;
	}

	PBPropertyValue( const PBPropertyValue& other );
	PBPropertyValue( PBPropertyValue&& other ) noexcept;

	~PBPropertyValue();

	Type GetType() const
	{
		return m_eType;
	}

	void* GetRaw() const
	{
		return m_uValue.Pointer;
	}

	TBOOL GetBoolean() const
	{
		TASSERT( m_eType == Type::Bool );
		return m_uValue.Bool;
	}

	TFLOAT GetFloat() const
	{
		TASSERT( m_eType == Type::Float || m_eType == Type::Int );
		return m_eType == Type::Float ? m_uValue.Float : TFLOAT( m_uValue.Int );
	}

	TINT GetInteger() const
	{
		TASSERT( m_eType == Type::Int );
		return m_uValue.Int;
	}

	TUINT32 GetUINT32() const
	{
		TASSERT( m_eType == Type::UInt32 );
		return m_uValue.UInt32;
	}

	// $Barnyard: FUNCTION 006b1cf0
	PBPropertyLocaleString GetLocaleString() const
	{
		TASSERT( m_eType == Type::LocaleString );
		return m_uValue.UInt32;
	}

	// Returns the localised string of the LocaleString value
	// NOTE: unreferenced in retail, the name is invented (defined in PBPropertyReader.cpp)
	const TWCHAR* GetLocalisedString() const;

	const TCHAR* GetString() const
	{
		TASSERT( m_eType == Type::String );
		return m_uValue.String;
	}

#ifdef __TOSHI_TPSTRING8_H__
	// $Barnyard: FUNCTION 006b2450
	// $Barnyard: FUNCTION 006b2420
	Toshi::TPString8 GetTPString8() const
	{
		TASSERT( m_eType == Type::String );
		return Toshi::TPString8( m_uValue.String );
	}
#endif

	const class PBProperties* GetProperties() const
	{
		TASSERT( m_eType == Type::Properties );
		return m_uValue.Properties;
	}

	const class PBPropertyValueArray* GetArray() const
	{
		TASSERT( m_eType == Type::Array );
		return m_uValue.Array;
	}

	// Reads an array of 2 numbers (floats or integers). Returns TFALSE if the array has another size
	TBOOL GetVector2( Toshi::TVector2& a_rOutVector ) const;

	// Reads an array of 3 numbers (floats or integers). Returns TFALSE if the array has another size
	TBOOL GetVector3( Toshi::TVector3& a_rOutVector ) const;

	// Reads an array of 3 or 4 numbers (floats or integers), w is 1.0f if there are 3. Returns TFALSE if the array has another size
	TBOOL GetVector4( Toshi::TVector4& a_rOutVector ) const;

	// Reads an array of 4 numbers (floats or integers). Returns TFALSE if the array has another size
	TBOOL GetQuaternion( Toshi::TQuaternion& a_rOutQuaternion ) const;

	class PBProperties* GetProperties()
	{
		TASSERT( m_eType == Type::Properties );
		return m_uValue.Properties;
	}

	class PBPropertyValueArray* GetArray()
	{
		TASSERT( m_eType == Type::Array );
		return m_uValue.Array;
	}

	TBOOL* GetBooleanPointer()
	{
		TASSERT( m_eType == Type::Bool );
		return &m_uValue.Bool;
	}

	TFLOAT* GetFloatPointer()
	{
		TASSERT( m_eType == Type::Float );
		return &m_uValue.Float;
	}

	TINT* GetIntegerPointer()
	{
		TASSERT( m_eType == Type::Int );
		return &m_uValue.Int;
	}

	TUINT32* GetUINT32Pointer()
	{
		TASSERT( m_eType == Type::LocaleString );
		return &m_uValue.UInt32;
	}

	void SetInt( TINT a_iValue )
	{
		this->operator=( a_iValue );
	}

	void SetBool( TBOOL a_bValue )
	{
		this->operator=( a_bValue );
	}

	void SetFloat( TFLOAT a_fValue )
	{
		this->operator=( a_fValue );
	}

	void SetUINT32( TUINT32 a_uiValue )
	{
		this->operator=( a_uiValue );
	}

	void SetString( const Toshi::TString8& a_sValue )
	{
		this->operator=( a_sValue );
	}

	PBPropertyValue& operator=( TINT a_iValue )
	{
		TASSERT( m_eType == Type::Int );
		m_uValue.Int = a_iValue;
		return *this;
	}

	PBPropertyValue& operator=( TBOOL a_bValue )
	{
		TASSERT( m_eType == Type::Bool );
		m_uValue.Bool = a_bValue;
		return *this;
	}

	PBPropertyValue& operator=( TFLOAT a_fValue )
	{
		TASSERT( m_eType == Type::Float );
		m_uValue.Float = a_fValue;
		return *this;
	}

	PBPropertyValue& operator=( TUINT32 a_uiValue )
	{
		TASSERT( m_eType == Type::LocaleString );
		m_uValue.UInt32 = a_uiValue;
		return *this;
	}

	PBPropertyValue& operator=( const Toshi::TString8& a_sValue )
	{
		TASSERT( m_eType == Type::String );

		Delete();

		m_uValue.String = new TCHAR[ a_sValue.Length() + 1 ];
		Toshi::TStringManager::String8Copy( m_uValue.String, a_sValue );
		return *this;
	}

	PBPropertyValue& operator=( const PBPropertyValue& other );
	PBPropertyValue& operator=( PBPropertyValue&& other ) noexcept;

private:
	void Delete();

private:
	Type m_eType;

	union
	{
		void*                       Pointer;
		TCHAR*                      String;
		TFLOAT                      Float;
		TBOOL                       Bool;
		TINT                        Int;
		TUINT32                     UInt32;
		class PBProperties*         Properties;
		class PBPropertyValueArray* Array;
	} m_uValue;
};

class PBProperties;
class PBPropertyValueArray;

template <typename T>
struct PBPropertiesTypeCast
{};

template <>
struct PBPropertiesTypeCast<TINT>
{
	static constexpr PBPropertyValue::Type type = PBPropertyValue::Type::Int;
};

template <>
struct PBPropertiesTypeCast<TINT8>
{
	static constexpr PBPropertyValue::Type type = PBPropertyValue::Type::Int;
};

template <>
struct PBPropertiesTypeCast<TINT16>
{
	static constexpr PBPropertyValue::Type type = PBPropertyValue::Type::Int;
};

template <>
struct PBPropertiesTypeCast<TUINT8>
{
	static constexpr PBPropertyValue::Type type = PBPropertyValue::Type::Int;
};

template <>
struct PBPropertiesTypeCast<TUINT16>
{
	static constexpr PBPropertyValue::Type type = PBPropertyValue::Type::Int;
};

template <>
struct PBPropertiesTypeCast<TUINT32>
{
	static constexpr PBPropertyValue::Type type = PBPropertyValue::Type::UInt32;
};

template <>
struct PBPropertiesTypeCast<TFLOAT>
{
	static constexpr PBPropertyValue::Type type = PBPropertyValue::Type::Float;
};

template <>
struct PBPropertiesTypeCast<TBOOL>
{
	static constexpr PBPropertyValue::Type type = PBPropertyValue::Type::Bool;
};

template <>
struct PBPropertiesTypeCast<PBProperties*>
{
	static constexpr PBPropertyValue::Type type = PBPropertyValue::Type::Properties;
};

template <>
struct PBPropertiesTypeCast<const PBProperties*>
{
	static constexpr PBPropertyValue::Type type = PBPropertyValue::Type::Properties;
};

template <>
struct PBPropertiesTypeCast<PBPropertyValueArray*>
{
	static constexpr PBPropertyValue::Type type = PBPropertyValue::Type::Array;
};

template <>
struct PBPropertiesTypeCast<const PBPropertyValueArray*>
{
	static constexpr PBPropertyValue::Type type = PBPropertyValue::Type::Array;
};

template <>
struct PBPropertiesTypeCast<TCHAR*>
{
	static constexpr PBPropertyValue::Type type = PBPropertyValue::Type::String;
};

template <>
struct PBPropertiesTypeCast<const TCHAR*>
{
	static constexpr PBPropertyValue::Type type = PBPropertyValue::Type::String;
};

template <>
struct PBPropertiesTypeCast<PBPropertyLocaleString>
{
	static constexpr PBPropertyValue::Type type = PBPropertyValue::Type::LocaleString;
};

template <>
struct PBPropertiesTypeCast<Toshi::TString8>
{
	static constexpr PBPropertyValue::Type type = PBPropertyValue::Type::String;
};

#ifdef __TOSHI_TPSTRING8_H__

template <>
struct PBPropertiesTypeCast<Toshi::TPString8>
{
	static constexpr PBPropertyValue::Type type = PBPropertyValue::Type::String;
};

#endif // __TOSHI_TPSTRING8_H__

class PBPropertyValueArray
{
public:
	friend class PPropertiesWriter;
	friend class PBPropertyReader;

public:
	PBPropertyValueArray()
	{
		m_pValues = TNULL;
		m_iSize   = 0;
	}

	PBPropertyValueArray( const PBPropertyValueArray& other )
	{
		if ( other.m_iSize != 0 )
		{
			m_pValues = new PBPropertyValue[ other.m_iSize ];

			for ( TSIZE i = 0; i < other.m_iSize; i++ )
				m_pValues[ i ] = other.m_pValues[ i ];
		}
		else
		{
			m_pValues = TNULL;
		}

		m_iSize = other.m_iSize;
	}

	PBPropertyValueArray( PBPropertyValueArray&& other ) noexcept
	{
		m_pValues       = other.m_pValues;
		m_iSize         = other.m_iSize;
		other.m_pValues = TNULL;
		other.m_iSize   = 0;
	}

	~PBPropertyValueArray()
	{
		Delete();
	}

	PBPropertyValue* GetValue( TSIZE a_iIndex )
	{
		TASSERT( m_iSize > a_iIndex );
		return &m_pValues[ a_iIndex ];
	}

	const PBPropertyValue* GetValue( TSIZE a_iIndex ) const
	{
		TASSERT( m_iSize > a_iIndex );
		return &m_pValues[ a_iIndex ];
	}

	TUINT32 GetSize() const
	{
		return m_iSize;
	}

	PBPropertyValue* Add( TINT a_iValue )
	{
		return AllocValue( PBPropertyValue( a_iValue ) );
	}

	PBPropertyValue* Add( TBOOL a_bValue )
	{
		return AllocValue( PBPropertyValue( a_bValue ) );
	}

	PBPropertyValue* Add( TFLOAT a_fValue )
	{
		return AllocValue( PBPropertyValue( a_fValue ) );
	}

	PBPropertyValue* Add( TUINT32 a_uiValue )
	{
		return AllocValue( PBPropertyValue( a_uiValue ) );
	}

	PBPropertyValue* Add( PBPropertyLocaleString a_strLocaleString )
	{
		return AllocValue( PBPropertyValue( a_strLocaleString ) );
	}

	PBPropertyValue* Add( const Toshi::TString8& a_sValue )
	{
		return AllocValue( PBPropertyValue( a_sValue ) );
	}

	PBPropertyValue* Add( const TCHAR* a_pchValue )
	{
		return AllocValue( PBPropertyValue( a_pchValue ) );
	}

	PBPropertyValue* operator[]( TSIZE a_iIndex )
	{
		return GetValue( a_iIndex );
	}

	const PBPropertyValue* operator[]( TSIZE a_iIndex ) const
	{
		return GetValue( a_iIndex );
	}

	PBPropertyValueArray& operator=( const PBPropertyValueArray& other )
	{
		if ( this != &other )
		{
			Delete();

			if ( other.m_iSize != 0 )
			{
				m_pValues = new PBPropertyValue[ other.m_iSize ];

				for ( TSIZE i = 0; i < other.m_iSize; i++ )
					m_pValues[ i ] = other.m_pValues[ i ];

				m_iSize = other.m_iSize;
			}
		}

		return *this;
	}

	PBPropertyValueArray& operator=( PBPropertyValueArray&& other ) noexcept
	{
		if ( this != &other )
		{
			Delete();

			m_pValues       = other.m_pValues;
			m_iSize         = other.m_iSize;
			other.m_pValues = TNULL;
			other.m_iSize   = 0;
		}

		return *this;
	}

private:
	PBPropertyValue* AllocValue( PBPropertyValue&& a_Value )
	{
		if ( m_pValues != TNULL )
		{
			auto newValues = new PBPropertyValue[ m_iSize + 1 ];

			for ( TSIZE i = 0; i < m_iSize; i++ )
				newValues[ i ] = std::move( m_pValues[ i ] );

			Delete();
			m_pValues = newValues;

			m_iSize += 1;
		}
		else
		{
			m_pValues = new PBPropertyValue[ 1 ];
			m_iSize   = 1;
		}

		m_pValues[ m_iSize - 1 ] = std::move( a_Value );
		return &m_pValues[ m_iSize - 1 ];
	}

	void Delete()
	{
		if ( m_pValues )
			delete[] m_pValues;
	}

private:
	PBPropertyValue* m_pValues;
	TUINT32          m_iSize;
};

// $Barnyard: FUNCTION 006b1d00
inline TBOOL PBPropertyValue::GetVector2( Toshi::TVector2& a_rOutVector ) const
{
	const PBPropertyValueArray* pArray = m_uValue.Array;

	if ( pArray->GetSize() != 2 )
		return TFALSE;

	// Anything that isn't a float is read as an integer
	auto fnGetFloat = []( const PBPropertyValue* a_pValue ) -> TFLOAT {
		return ( a_pValue->GetType() == PBPropertyValue::Type::Float ) ? a_pValue->GetFloat() : TFLOAT( TINT( TREINTERPRETCAST( TUINTPTR, a_pValue->GetRaw() ) ) );
	};

	a_rOutVector.x = fnGetFloat( pArray->GetValue( 0 ) );
	a_rOutVector.y = fnGetFloat( pArray->GetValue( 1 ) );

	return TTRUE;
}

// $Barnyard: FUNCTION 006b1dc0
inline TBOOL PBPropertyValue::GetVector4( Toshi::TVector4& a_rOutVector ) const
{
	const PBPropertyValueArray* pArray = m_uValue.Array;

	if ( pArray->GetSize() != 3 && pArray->GetSize() != 4 )
		return TFALSE;

	// Anything that isn't a float is read as an integer
	auto fnGetFloat = []( const PBPropertyValue* a_pValue ) -> TFLOAT {
		return ( a_pValue->GetType() == PBPropertyValue::Type::Float ) ? a_pValue->GetFloat() : TFLOAT( TINT( TREINTERPRETCAST( TUINTPTR, a_pValue->GetRaw() ) ) );
	};

	a_rOutVector.x = fnGetFloat( pArray->GetValue( 0 ) );
	a_rOutVector.y = fnGetFloat( pArray->GetValue( 1 ) );
	a_rOutVector.z = fnGetFloat( pArray->GetValue( 2 ) );
	a_rOutVector.w = 1.0f;

	if ( pArray->GetSize() == 4 )
		a_rOutVector.w = fnGetFloat( pArray->GetValue( 3 ) );

	return TTRUE;
}

// $Barnyard: FUNCTION 006b1d50
inline TBOOL PBPropertyValue::GetVector3( Toshi::TVector3& a_rOutVector ) const
{
	const PBPropertyValueArray* pArray = m_uValue.Array;

	if ( pArray->GetSize() != 3 )
		return TFALSE;

	a_rOutVector.x = pArray->GetValue( 0 )->GetFloat();
	a_rOutVector.y = pArray->GetValue( 1 )->GetFloat();
	a_rOutVector.z = pArray->GetValue( 2 )->GetFloat();

	return TTRUE;
}

// $Barnyard: FUNCTION 006b1e60
inline TBOOL PBPropertyValue::GetQuaternion( Toshi::TQuaternion& a_rOutQuaternion ) const
{
	const PBPropertyValueArray* pArray = m_uValue.Array;

	if ( pArray->GetSize() != 4 )
		return TFALSE;

	a_rOutQuaternion.x = pArray->GetValue( 0 )->GetFloat();
	a_rOutQuaternion.y = pArray->GetValue( 1 )->GetFloat();
	a_rOutQuaternion.z = pArray->GetValue( 2 )->GetFloat();
	a_rOutQuaternion.w = pArray->GetValue( 3 )->GetFloat();

	return TTRUE;
}

class PBPropertyName
{
public:
	friend class PPropertiesWriter;

public:
	PBPropertyName()
	{
		m_szName = TNULL;
	}

	PBPropertyName( const Toshi::TString8& a_sName )
	{
		m_szName = new TCHAR[ a_sName.Length() + 1 ];
		Toshi::TStringManager::String8Copy( m_szName, a_sName );
	}

	PBPropertyName( const PBPropertyName& other )
	{
		m_szName = new TCHAR[ Toshi::TStringManager::String8Length( other.m_szName ) + 1 ];
		Toshi::TStringManager::String8Copy( m_szName, other.m_szName );
	}

	PBPropertyName( PBPropertyName&& other ) noexcept
	{
		m_szName       = other.m_szName;
		other.m_szName = TNULL;
	}

	~PBPropertyName()
	{
		Delete();
	}

	void SetName( const Toshi::TString8& a_sName )
	{
		Delete();

		m_szName = new TCHAR[ a_sName.Length() + 1 ];
		Toshi::TStringManager::String8Copy( m_szName, a_sName );
	}

	const TCHAR* GetString() const
	{
		return m_szName;
	}

	TBOOL operator==( PBPropertyName* a_pName ) const
	{
		return Toshi::TStringManager::String8Compare( m_szName, a_pName->m_szName ) == 0;
	}

	TBOOL operator==( const TCHAR* a_szName ) const
	{
		return Toshi::TStringManager::String8Compare( m_szName, a_szName ) == 0;
	}

	PBPropertyName operator=( const Toshi::TString8& a_sName )
	{
		Delete();

		m_szName = new TCHAR[ a_sName.Length() + 1 ];
		Toshi::TStringManager::String8Copy( m_szName, a_sName );
	}

private:
	void Delete()
	{
		if ( m_szName )
			delete[] m_szName;
	}

private:
	TCHAR* m_szName;
};

class PBProperties
{
public:
	class PBProperty
	{
	public:
		friend class PPropertiesWriter;

	public:
		PBProperty()
		{
			m_pName  = TNULL;
			m_pValue = TNULL;
		}

		PBProperty( const Toshi::TString8& a_sName, TINT a_iValue )
		{
			m_pName  = new PBPropertyName( a_sName );
			m_pValue = new PBPropertyValue( a_iValue );
		}

		PBProperty( const Toshi::TString8& a_sName, TBOOL a_bValue )
		{
			m_pName  = new PBPropertyName( a_sName );
			m_pValue = new PBPropertyValue( a_bValue );
		}

		PBProperty( const Toshi::TString8& a_sName, TFLOAT a_fValue )
		{
			m_pName  = new PBPropertyName( a_sName );
			m_pValue = new PBPropertyValue( a_fValue );
		}

		PBProperty( const Toshi::TString8& a_sName, TUINT32 a_uiValue )
		{
			m_pName  = new PBPropertyName( a_sName );
			m_pValue = new PBPropertyValue( a_uiValue );
		}

		PBProperty( const Toshi::TString8& a_sName, const Toshi::TString8& a_sValue )
		{
			m_pName  = new PBPropertyName( a_sName );
			m_pValue = new PBPropertyValue( a_sValue );
		}

		PBProperty( const Toshi::TString8& a_sName, PBProperties* a_pProperties )
		{
			m_pName  = new PBPropertyName( a_sName );
			m_pValue = new PBPropertyValue( a_pProperties );
		}

		PBProperty( const Toshi::TString8& a_sName, PBPropertyValueArray* a_pArray )
		{
			m_pName  = new PBPropertyName( a_sName );
			m_pValue = new PBPropertyValue( a_pArray );
		}

		PBProperty( const Toshi::TString8& a_sName, PBPropertyLocaleString a_strLocaleString )
		{
			m_pName  = new PBPropertyName( a_sName );
			m_pValue = new PBPropertyValue( a_strLocaleString );
		}

		PBProperty( const PBProperty& other )
		{
			m_pName  = new PBPropertyName( *other.m_pName );
			m_pValue = new PBPropertyValue( *other.m_pValue );
		}

		PBProperty( PBProperty&& other ) noexcept
		{
			m_pName        = other.m_pName;
			m_pValue       = other.m_pValue;
			other.m_pName  = TNULL;
			other.m_pValue = TNULL;
		}

		~PBProperty()
		{
			Delete();
		}

		PBPropertyName& GetName()
		{
			return *m_pName;
		}

		PBPropertyValue* GetValue()
		{
			return m_pValue;
		}

		const PBPropertyName& GetName() const
		{
			return *m_pName;
		}

		const PBPropertyValue* GetValue() const
		{
			return m_pValue;
		}

		PBProperty& operator=( const PBProperty& other )
		{
			Delete();

			m_pName  = new PBPropertyName( *other.m_pName );
			m_pValue = new PBPropertyValue( *other.m_pValue );

			return *this;
		}

		PBProperty& operator=( PBProperty&& other ) noexcept
		{
			Delete();

			m_pName        = other.m_pName;
			m_pValue       = other.m_pValue;
			other.m_pName  = TNULL;
			other.m_pValue = TNULL;

			return *this;
		}

	private:
		void Delete()
		{
			if ( m_pName )
				delete m_pName;

			if ( m_pValue )
				delete m_pValue;
		}

	private:
		PBPropertyName*  m_pName;
		PBPropertyValue* m_pValue;
	};

	friend class PPropertiesWriter;

	static constexpr const TCHAR* TRB_SECTION_NAME = "Main";

public:
	PBProperties()
	{
		m_pParent     = TNULL;
		m_pProperties = TNULL;
		m_iCount      = 0;
	}

	PBProperties( const PBProperties& other )
	{
		m_pParent = other.m_pParent;

		if ( other.m_pProperties != TNULL )
		{
			m_pProperties = new PBProperty[ other.m_iCount ];

			for ( TSIZE i = 0; i < other.m_iCount; i++ )
			{
				m_pProperties[ i ] = other.m_pProperties[ i ];
				auto pValue        = m_pProperties[ i ].GetValue();

				if ( pValue->GetType() == PBPropertyValue::Type::Properties )
				{
					pValue->GetProperties()->m_pParent = this;
				}
			}
		}
		else
		{
			m_pProperties = other.m_pProperties;
		}

		m_iCount = other.m_iCount;
	}

	PBProperties( PBProperties&& other ) noexcept
	{
		m_pParent           = other.m_pParent;
		m_pProperties       = other.m_pProperties;
		m_iCount            = other.m_iCount;
		other.m_pParent     = TNULL;
		other.m_pProperties = TNULL;
		other.m_iCount      = 0;
	}

	~PBProperties()
	{
		Delete();
	}

	PBProperty* Begin()
	{
		return m_pProperties;
	}

	PBProperty* End()
	{
		return m_pProperties + m_iCount;
	}

	const PBProperty* Begin() const
	{
		return m_pProperties;
	}

	const PBProperty* End() const
	{
		return m_pProperties + m_iCount;
	}

	const PBProperty* GetPropertyByIndex( TSIZE a_iIndex ) const
	{
		TASSERT( m_iCount > a_iIndex );
		return &m_pProperties[ a_iIndex ];
	}

	const PBPropertyValue* GetPropertyValue( TSIZE a_iIndex ) const
	{
		return GetPropertyByIndex( a_iIndex )->GetValue();
	}

	// $Barnyard: FUNCTION 006b1ee0
	const PBPropertyValue* GetProperty( const TCHAR* a_szName ) const
	{
		for ( TSIZE i = 0; i < m_iCount; i++ )
		{
			if ( m_pProperties[ i ].GetName() == a_szName )
			{
				return m_pProperties[ i ].GetValue();
			}
		}

		return TNULL;
	}

	// Like GetOptionalPropertyValue for TFLOAT, but also accepts integer values
	// $Barnyard: FUNCTION 006b2270
	TBOOL GetOptionalFloatOrIntValue( TFLOAT& a_rOutValue, const TCHAR* a_szName ) const
	{
		const PBPropertyValue* pValue = GetProperty( a_szName );

		if ( pValue )
		{
			if ( pValue->GetType() == PBPropertyValue::Type::Float )
			{
				a_rOutValue = pValue->GetFloat();
				return TTRUE;
			}

			if ( pValue->GetType() == PBPropertyValue::Type::Int )
			{
				a_rOutValue = TFLOAT( pValue->GetInteger() );
				return TTRUE;
			}
		}

		return TFALSE;
	}

	// Reads a_szName as an array of 2 numbers
	// NOTE: unreferenced in retail. Like the other ones, only the type of the first element is checked
	// $Barnyard: FUNCTION 006b2020
	TBOOL GetOptionalVector2( Toshi::TVector2& a_rOutValue, const TCHAR* a_szName ) const
	{
		const PBPropertyValue* pValue = GetProperty( a_szName );

		if ( !pValue || pValue->GetType() != PBPropertyValue::Type::Array )
			return TFALSE;

		const PBPropertyValueArray* pArray = pValue->GetArray();

		if ( pArray->GetSize() != 2 )
			return TFALSE;

		for ( TINT i = 0; i < 2; i++ )
		{
			if ( pArray->GetValue( 0 )->GetType() != PBPropertyValue::Type::Float )
				return TFALSE;
		}

		return pValue->GetVector2( a_rOutValue );
	}

	// Reads the LocaleString a_szName as the localised string
	// NOTE: unreferenced in retail, the name is invented (defined in PBPropertyReader.cpp)
	TBOOL GetOptionalLocalisedString( const TWCHAR*& a_rOutValue, const TCHAR* a_szName ) const;

	// Reads a_szName as an array of 3 numbers
	// NOTE: like the original, only the type of the first element is checked
	// $Barnyard: FUNCTION 006b20b0
	TBOOL GetOptionalVector3( Toshi::TVector3& a_rOutValue, const TCHAR* a_szName ) const
	{
		const PBPropertyValue* pValue = GetProperty( a_szName );

		if ( !pValue || pValue->GetType() != PBPropertyValue::Type::Array )
			return TFALSE;

		const PBPropertyValueArray* pArray = pValue->GetArray();

		if ( pArray->GetSize() != 3 )
			return TFALSE;

		for ( TINT i = 0; i < 3; i++ )
		{
			if ( pArray->GetValue( 0 )->GetType() != PBPropertyValue::Type::Float )
				return TFALSE;
		}

		return pValue->GetVector3( a_rOutValue );
	}

	// Reads a_szName as an array of 3 or 4 numbers (see PBPropertyValue::GetVector4)
	// NOTE: like the original, only the type of the first element is checked
	// $Barnyard: FUNCTION 006b2110
	// $Barnyard: FUNCTION 006b2340
	TBOOL GetOptionalVector4( Toshi::TVector4& a_rOutValue, const TCHAR* a_szName ) const
	{
		const PBPropertyValue* pValue = GetProperty( a_szName );

		if ( !pValue || pValue->GetType() != PBPropertyValue::Type::Array )
			return TFALSE;

		const PBPropertyValueArray* pArray = pValue->GetArray();

		if ( pArray->GetSize() != 3 && pArray->GetSize() != 4 )
			return TFALSE;

		for ( TUINT32 i = 0; i < pArray->GetSize(); i++ )
		{
			if ( pArray->GetValue( 0 )->GetType() != PBPropertyValue::Type::Float )
				return TFALSE;
		}

		return pValue->GetVector4( a_rOutValue );
	}

	// Reads a_szName as an array of 4 numbers
	// NOTE: like the original, only the type of the first element is checked
	// $Barnyard: FUNCTION 006b2180
	// $Barnyard: FUNCTION 006b2350
	TBOOL GetOptionalQuaternion( Toshi::TQuaternion& a_rOutValue, const TCHAR* a_szName ) const
	{
		const PBPropertyValue* pValue = GetProperty( a_szName );

		if ( !pValue || pValue->GetType() != PBPropertyValue::Type::Array )
			return TFALSE;

		const PBPropertyValueArray* pArray = pValue->GetArray();

		if ( pArray->GetSize() != 4 )
			return TFALSE;

		for ( TINT i = 0; i < 4; i++ )
		{
			if ( pArray->GetValue( 0 )->GetType() != PBPropertyValue::Type::Float )
				return TFALSE;
		}

		return pValue->GetQuaternion( a_rOutValue );
	}

	// $Barnyard: FUNCTION 006b1f30
	const PBProperties* FindProperties( const TCHAR* a_szName ) const
	{
		const PBPropertyValue* pValue = GetProperty( a_szName );

		if ( pValue && pValue->GetType() == PBPropertyValue::Type::Properties )
			return pValue->GetProperties();

		return TNULL;
	}

	// NOTE: retail has a separate copy of this per type that takes the name as a const TPString8&:
	// 006b1f50 (TFLOAT, same code as 006b2270), 006b1fa0/006b22c0 (TINT), 006b1fe0/006b2300 (TBOOL),
	// 006b2230 (PBPropertyLocaleString), 006b2470 (TPString8)
	// $Barnyard: FUNCTION 006b1f50
	// $Barnyard: FUNCTION 006b1fa0
	// $Barnyard: FUNCTION 006b22c0
	// $Barnyard: FUNCTION 006b1fe0
	// $Barnyard: FUNCTION 006b2300
	// $Barnyard: FUNCTION 006b2230
	// $Barnyard: FUNCTION 006b2470
	template <typename T>
	TBOOL GetOptionalPropertyValue( T& a_rOutValue, const TCHAR* a_szName ) const
	{
		// Like the original, the TFLOAT version also accepts integers
		if constexpr ( PBPropertiesTypeCast<T>::type == PBPropertyValue::Type::Float )
			return GetOptionalFloatOrIntValue( a_rOutValue, a_szName );

		const PBPropertyValue* pFoundProperty = GetProperty( a_szName );

		if ( pFoundProperty && pFoundProperty->GetType() == PBPropertiesTypeCast<T>::type )
		{
			if constexpr ( PBPropertiesTypeCast<T>::type == PBPropertyValue::Type::Float )
				a_rOutValue = pFoundProperty->GetFloat();
			else if constexpr ( PBPropertiesTypeCast<T>::type == PBPropertyValue::Type::LocaleString )
				a_rOutValue = pFoundProperty->GetLocaleString();
			else if constexpr ( PBPropertiesTypeCast<T>::type == PBPropertyValue::Type::Int )
				a_rOutValue = pFoundProperty->GetInteger();
			else if constexpr ( PBPropertiesTypeCast<T>::type == PBPropertyValue::Type::Bool )
				a_rOutValue = pFoundProperty->GetBoolean();
			else if constexpr ( PBPropertiesTypeCast<T>::type == PBPropertyValue::Type::UInt32 )
				a_rOutValue = pFoundProperty->GetUINT32();
			else if constexpr ( PBPropertiesTypeCast<T>::type == PBPropertyValue::Type::String )
				a_rOutValue = pFoundProperty->GetString();
			else
			{
				void* rawValue = pFoundProperty->GetRaw();
				a_rOutValue    = *(T*)( &rawValue );
			}

			return TTRUE;
		}

		return TFALSE;
	}

	const PBProperties* GetParentProperties() const
	{
		return m_pParent;
	}

	TUINT32 GetPropertyCount() const
	{
		return m_iCount;
	}

	PBProperty* AddProperty( const Toshi::TString8& a_Name, TINT a_iValue )
	{
		return AllocProperty( PBProperty( a_Name, a_iValue ) );
	}

	PBProperty* AddProperty( const Toshi::TString8& a_Name, PBPropertyLocaleString a_strLocaleString )
	{
		return AllocProperty( PBProperty( a_Name, a_strLocaleString ) );
	}

	PBProperty* AddPropertyBool( const Toshi::TString8& a_Name, TBOOL a_bValue )
	{
		return AllocProperty( PBProperty( a_Name, a_bValue ) );
	}

	PBProperty* AddProperty( const Toshi::TString8& a_Name, TFLOAT a_fValue )
	{
		return AllocProperty( PBProperty( a_Name, a_fValue ) );
	}

	PBProperty* AddProperty( const Toshi::TString8& a_Name, TUINT32 a_uiValue )
	{
		return AllocProperty( PBProperty( a_Name, a_uiValue ) );
	}

	PBProperty* AddProperty( const Toshi::TString8& a_Name, const Toshi::TString8& a_sValue )
	{
		return AllocProperty( PBProperty( a_Name, a_sValue ) );
	}

	PBProperty* AddProperties( const Toshi::TString8& a_Name )
	{
		PBProperties* pProperties = new PBProperties;
		pProperties->m_pParent    = this;

		return AllocProperty( PBProperty( a_Name, pProperties ) );
	}

	PBProperty* AddPropertyArray( const Toshi::TString8& a_Name )
	{
		PBPropertyValueArray* pArray = new PBPropertyValueArray;

		return AllocProperty( PBProperty( a_Name, pArray ) );
	}

#ifdef __TOSHI_TTRB_H__
	TFORCEINLINE static const PBProperties* LoadFromTRB( Toshi::TTRB* a_pTRB )
	{
		return a_pTRB->CastSymbol<PBProperties>( TRB_SECTION_NAME );
	}

	TFORCEINLINE static const PBProperties* LoadFromTRB( Toshi::TTRB& a_TRB )
	{
		return a_TRB.CastSymbol<PBProperties>( TRB_SECTION_NAME );
	}

	TFORCEINLINE static const PBProperties* LoadFromAsset( Toshi::TTRB* a_pTRB, const TCHAR* a_szFileName )
	{
		Toshi::TString8 symbolName = a_szFileName;
		symbolName.MakeLower();
		symbolName += "_";
		symbolName += TRB_SECTION_NAME;

		return a_pTRB->CastSymbol<PBProperties>( symbolName );
	}

	TFORCEINLINE static const PBProperties* LoadFromAsset( Toshi::TTRB& a_TRB, const TCHAR* a_szFileName )
	{
		Toshi::TString8 symbolName = a_szFileName;
		symbolName.MakeLower();
		symbolName += "_";
		symbolName += TRB_SECTION_NAME;

		return a_TRB.CastSymbol<PBProperties>( symbolName );
	}
#endif

private:
	PBProperty* AllocProperty( PBProperty&& a_Property )
	{
		if ( m_pProperties != TNULL )
		{
			auto newProperties = new PBProperty[ m_iCount + 1 ];

			for ( TSIZE i = 0; i < m_iCount; i++ )
				newProperties[ i ] = std::move( m_pProperties[ i ] );

			Delete();
			m_pProperties = newProperties;

			m_iCount += 1;
		}
		else
		{
			m_pProperties = new PBProperty[ 1 ];
			m_iCount      = 1;
		}

		m_pProperties[ m_iCount - 1 ] = std::move( a_Property );
		return &m_pProperties[ m_iCount - 1 ];
	}

	void Delete()
	{
		if ( m_pProperties )
			delete[] m_pProperties;
	}

private:
	PBProperties* m_pParent;
	PBProperty*   m_pProperties;
	TUINT32       m_iCount;
};

inline void PBPropertyValue::Delete()
{
	if ( m_uValue.Pointer )
	{
		if ( m_eType == Type::Array )
			delete m_uValue.Array;
		else if ( m_eType == Type::Properties )
			delete m_uValue.Properties;
		else if ( m_eType == Type::String )
			delete[] m_uValue.String;
	}
}

inline PBPropertyValue::PBPropertyValue( const PBPropertyValue& other )
{
	m_eType          = other.m_eType;
	m_uValue.Pointer = TNULL;

	if ( other.m_eType == Type::Array )
		m_uValue.Array = new PBPropertyValueArray( *other.m_uValue.Array );
	else if ( other.m_eType == Type::Properties )
		m_uValue.Properties = new PBProperties( *other.m_uValue.Properties );
	else if ( other.m_eType == Type::String )
		SetString( other.m_uValue.String );
	else
		m_uValue = other.m_uValue;
}

class PPropertyValue;
class PPropertyValueArray;

//-----------------------------------------------------------------------------
// Converts the binary properties into PPropertyValue values.
// NOTE: defined in PBPropertyReader.cpp since it needs the whole PPropertyValue.
//-----------------------------------------------------------------------------
class PBPropertyReader
{
public:
	// Returns a new value, owned by the caller
	static PPropertyValue* ToValue( const PBPropertyValue* a_pValue );

	// Returns a new array, owned by the caller
	static PPropertyValueArray* ToValueArray( const PBPropertyValueArray* a_pArray );

	// Returns a new name (a_pName with an empty sub name), owned by the caller
	static class PPropertyName* CreatePropertyName( const PBPropertyName* a_pName );

	// Returns new properties with the converted properties of a_pBProperties
	static class PProperties* CreateProperties( const PBProperties* a_pBProperties );

	// Adds the converted properties of a_pBProperties to a_pProperties
	static TBOOL FillProperties( const PBProperties* a_pBProperties, class PProperties* a_pProperties );
};

inline PBPropertyValue::PBPropertyValue( PBPropertyValue&& other ) noexcept
{
	m_eType                = other.m_eType;
	m_uValue.Pointer       = other.m_uValue.Pointer;
	other.m_uValue.Pointer = TNULL;
}

inline PBPropertyValue& PBPropertyValue::operator=( const PBPropertyValue& other )
{
	Delete();

	m_eType = other.m_eType;

	if ( other.m_eType == Type::Array )
		m_uValue.Array = new PBPropertyValueArray( *other.m_uValue.Array );
	else if ( other.m_eType == Type::Properties )
		m_uValue.Properties = new PBProperties( *other.m_uValue.Properties );
	else if ( other.m_eType == Type::String )
		SetString( other.m_uValue.String );
	else
		m_uValue = other.m_uValue;

	return *this;
}

inline PBPropertyValue& PBPropertyValue::operator=( PBPropertyValue&& other ) noexcept
{
	Delete();

	m_eType                = other.m_eType;
	m_uValue.Pointer       = other.m_uValue.Pointer;
	other.m_uValue.Pointer = TNULL;

	return *this;
}

inline PBPropertyValue::~PBPropertyValue()
{
	Delete();
}
