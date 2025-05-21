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

	PBPropertyLocaleString GetLocaleString() const
	{
		TASSERT( m_eType == Type::LocaleString );
		return m_uValue.UInt32;
	}

	const TCHAR* GetString() const
	{
		TASSERT( m_eType == Type::String );
		return m_uValue.String;
	}

#ifdef __TOSHI_TPSTRING8_H__
	// $Barnyard: FUNCTION 006b2450
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

	template <typename T>
	TBOOL GetOptionalPropertyValue( T& a_rOutValue, const TCHAR* a_szName ) const
	{
		const PBPropertyValue* pFoundProperty = GetProperty( a_szName );

		if ( pFoundProperty && pFoundProperty->GetType() == PBPropertiesTypeCast<T>::type )
		{
			if constexpr ( PBPropertiesTypeCast<T>::type == PBPropertyValue::Type::Float )
				a_rOutValue = pFoundProperty->GetFloat();
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
