#pragma once
#include "Toshi/Toshi.h"
#include "Toshi/TLog.h"
#include "Toshi/TStringManager.h"

class PPropertyValue
{
public:
	enum class Type
	{
		Int,
		Unknown1,
		Float,
		Bool,
		Props,
		Unknown2,
		Array,
		String,
		UInt32
	};

	friend class PPropertiesWriter;

public:
	PPropertyValue()
	{
		m_eType = Type::Int;
		m_uValue.Pointer = TNULL;
	}

	PPropertyValue(TINT a_iValue)
	{
		m_eType = Type::Int;
		m_uValue.Int = a_iValue;
	}

	PPropertyValue(TBOOL a_bValue)
	{
		m_eType = Type::Bool;
		m_uValue.Bool = a_bValue;
	}

	PPropertyValue(TFLOAT a_fValue)
	{
		m_eType = Type::Float;
		m_uValue.Float = a_fValue;
	}

	PPropertyValue(TUINT32 a_uiValue)
	{
		m_eType = Type::UInt32;
		m_uValue.UInt32 = a_uiValue;
	}

	PPropertyValue(const Toshi::TString8& a_sValue)
	{
		m_eType = Type::String;
		m_uValue.String = new char[a_sValue.Length() + 1];
		Toshi::TStringManager::String8Copy(m_uValue.String, a_sValue);
	}

	PPropertyValue(class PProperties* a_pProperties)
	{
		m_eType = Type::Props;
		m_uValue.Props = a_pProperties;
	}

	PPropertyValue(class PPropertyValueArray* a_pArray)
	{
		m_eType = Type::Array;
		m_uValue.Array = a_pArray;
	}

	PPropertyValue(const PPropertyValue& other);
	PPropertyValue(PPropertyValue&& other) noexcept;

	~PPropertyValue();

	Type GetType() const
	{
		return m_eType;
	}

	TBOOL GetBoolean() const
	{
		TASSERT(m_eType == Type::Bool);
		return m_uValue.Bool;
	}

	TFLOAT GetFloat() const
	{
		TASSERT(m_eType == Type::Float);
		return m_uValue.Float;
	}

	TINT GetInteger() const
	{
		TASSERT(m_eType == Type::Int);
		return m_uValue.Int;
	}

	TUINT32 GetUINT32() const
	{
		TASSERT(m_eType == Type::UInt32);
		return m_uValue.UInt32;
	}

	const char* GetString() const
	{
		TASSERT(m_eType == Type::String);
		return m_uValue.String;
	}

#ifdef __TOSHI_TPSTRING8_H__
	Toshi::TPString8 GetTPString8() const
	{
		TASSERT(m_eType == Type::String);
		return Toshi::TPString8(m_uValue.String);
	}
#endif

	const class PProperties* GetProperties() const
	{
		TASSERT(m_eType == Type::Props);
		return m_uValue.Props;
	}

	const class PPropertyValueArray* GetArray() const
	{
		TASSERT(m_eType == Type::Array);
		return m_uValue.Array;
	}

	class PProperties* GetProperties()
	{
		TASSERT(m_eType == Type::Props);
		return m_uValue.Props;
	}

	class PPropertyValueArray* GetArray()
	{
		TASSERT(m_eType == Type::Array);
		return m_uValue.Array;
	}

	TBOOL* GetBooleanPointer()
	{
		TASSERT(m_eType == Type::Bool);
		return &m_uValue.Bool;
	}

	TFLOAT* GetFloatPointer()
	{
		TASSERT(m_eType == Type::Float);
		return &m_uValue.Float;
	}

	TINT* GetIntegerPointer()
	{
		TASSERT(m_eType == Type::Int);
		return &m_uValue.Int;
	}

	TUINT32* GetUINT32Pointer()
	{
		TASSERT(m_eType == Type::UInt32);
		return &m_uValue.UInt32;
	}

	void SetInt(TINT a_iValue)
	{
		this->operator=(a_iValue);
	}

	void SetBool(TBOOL a_bValue)
	{
		this->operator=(a_bValue);
	}

	void SetFloat(TFLOAT a_fValue)
	{
		this->operator=(a_fValue);
	}

	void SetUINT32(TUINT32 a_uiValue)
	{
		this->operator=(a_uiValue);
	}

	void SetString(const Toshi::TString8& a_sValue)
	{
		this->operator=(a_sValue);
	}

	PPropertyValue& operator=(TINT a_iValue)
	{
		TASSERT(m_eType == Type::Int);
		m_uValue.Int = a_iValue;
		return *this;
	}

	PPropertyValue& operator=(TBOOL a_bValue)
	{
		TASSERT(m_eType == Type::Bool);
		m_uValue.Bool = a_bValue;
		return *this;
	}

	PPropertyValue& operator=(TFLOAT a_fValue)
	{
		TASSERT(m_eType == Type::Float);
		m_uValue.Float = a_fValue;
		return *this;
	}

	PPropertyValue& operator=(TUINT32 a_uiValue)
	{
		TASSERT(m_eType == Type::UInt32);
		m_uValue.UInt32 = a_uiValue;
		return *this;
	}

	PPropertyValue& operator=(const Toshi::TString8& a_sValue)
	{
		TASSERT(m_eType == Type::String);

		Delete();

		m_uValue.String = new char[a_sValue.Length() + 1];
		Toshi::TStringManager::String8Copy(m_uValue.String, a_sValue);
		return *this;
	}

	PPropertyValue& operator=(const PPropertyValue& other);
	PPropertyValue& operator=(PPropertyValue&& other) noexcept;

private:
	void Delete();

private:
	Type m_eType;

	union {
		void* Pointer;
		char* String;
		TFLOAT Float;
		TBOOL Bool;
		TINT Int;
		TUINT32 UInt32;
		class PProperties* Props;
		class PPropertyValueArray* Array;
	} m_uValue;
};

class PPropertyValueArray
{
public:
	friend class PPropertiesWriter;

public:
	PPropertyValueArray()
	{
		m_pValues = TNULL;
		m_iSize = 0;
	}

	PPropertyValueArray(const PPropertyValueArray& other)
	{
		if (other.m_iSize != 0)
		{
			m_pValues = new PPropertyValue[other.m_iSize];

			for (size_t i = 0; i < other.m_iSize; i++)
				m_pValues[i] = other.m_pValues[i];
		}
		else
		{
			m_pValues = TNULL;
		}

		m_iSize = other.m_iSize;
	}

	PPropertyValueArray(PPropertyValueArray&& other) noexcept
	{
		m_pValues = other.m_pValues;
		m_iSize = other.m_iSize;
		other.m_pValues = TNULL;
		other.m_iSize = 0;
	}

	~PPropertyValueArray()
	{
		Delete();
	}

	PPropertyValue* GetValue(size_t a_iIndex)
	{
		TASSERT(m_iSize > a_iIndex);
		return &m_pValues[a_iIndex];
	}

	const PPropertyValue* GetValue(size_t a_iIndex) const
	{
		TASSERT(m_iSize > a_iIndex);
		return &m_pValues[a_iIndex];
	}

	TUINT32 GetSize() const
	{
		return m_iSize;
	}

	PPropertyValue* Add(TINT a_iValue)
	{
		return AllocValue(PPropertyValue(a_iValue));
	}

	PPropertyValue* Add(TBOOL a_bValue)
	{
		return AllocValue(PPropertyValue(a_bValue));
	}

	PPropertyValue* Add(TFLOAT a_fValue)
	{
		return AllocValue(PPropertyValue(a_fValue));
	}

	PPropertyValue* Add(TUINT32 a_uiValue)
	{
		return AllocValue(PPropertyValue(a_uiValue));
	}

	PPropertyValue* Add(const Toshi::TString8& a_sValue)
	{
		return AllocValue(PPropertyValue(a_sValue));
	}

	PPropertyValue* operator[](size_t a_iIndex)
	{
		return GetValue(a_iIndex);
	}

	const PPropertyValue* operator[](size_t a_iIndex) const
	{
		return GetValue(a_iIndex);
	}

	PPropertyValueArray& operator=(const PPropertyValueArray& other)
	{
		Delete();

		if (other.m_iSize != 0)
		{
			m_pValues = new PPropertyValue[other.m_iSize];

			for (size_t i = 0; i < other.m_iSize; i++)
				m_pValues[i] = other.m_pValues[i];

			m_iSize = other.m_iSize;
		}
	}

	PPropertyValueArray& operator=(PPropertyValueArray&& other) noexcept
	{
		Delete();

		m_pValues = other.m_pValues;
		m_iSize = other.m_iSize;
		other.m_pValues = TNULL;
		other.m_iSize = 0;
	}

private:
	PPropertyValue* AllocValue(PPropertyValue&& a_Value)
	{
		if (m_pValues != TNULL)
		{
			auto newValues = new PPropertyValue[m_iSize + 1];

			for (size_t i = 0; i < m_iSize; i++)
				newValues[i] = std::move(m_pValues[i]);

			Delete();
			m_pValues = newValues;

			m_iSize += 1;
		}
		else
		{
			m_pValues = new PPropertyValue[1];
			m_iSize = 1;
		}

		m_pValues[m_iSize - 1] = std::move(a_Value);
		return &m_pValues[m_iSize - 1];
	}

	void Delete()
	{
		if (m_pValues)
			delete[] m_pValues;
	}

private:
	PPropertyValue* m_pValues;
	TUINT32 m_iSize;
};

class PPropertyName
{
public:
	friend class PPropertiesWriter;

public:
	PPropertyName()
	{
		m_szName = TNULL;
	}

	PPropertyName(const Toshi::TString8& a_sName)
	{
		m_szName = new char[a_sName.Length() + 1];
		Toshi::TStringManager::String8Copy(m_szName, a_sName);
	}

	PPropertyName(const PPropertyName& other)
	{
		m_szName = new char[Toshi::TStringManager::String8Length(other.m_szName) + 1];
		Toshi::TStringManager::String8Copy(m_szName, other.m_szName);
	}

	PPropertyName(PPropertyName&& other) noexcept
	{
		m_szName = other.m_szName;
		other.m_szName = TNULL;
	}

	~PPropertyName()
	{
		Delete();
	}

	void SetName(const Toshi::TString8& a_sName)
	{
		Delete();

		m_szName = new char[a_sName.Length() + 1];
		Toshi::TStringManager::String8Copy(m_szName, a_sName);
	}

	const char* GetString() const
	{
		return m_szName;
	}

	TBOOL operator==(PPropertyName* a_pName) const
	{
		return Toshi::TStringManager::String8Compare(m_szName, a_pName->m_szName) == 0;
	}

	TBOOL operator==(const char* a_szName) const
	{
		return Toshi::TStringManager::String8Compare(m_szName, a_szName) == 0;
	}

	PPropertyName operator=(const Toshi::TString8& a_sName)
	{
		Delete();

		m_szName = new char[a_sName.Length() + 1];
		Toshi::TStringManager::String8Copy(m_szName, a_sName);
	}

private:
	void Delete()
	{
		if (m_szName)
			delete[] m_szName;
	}

private:
	char* m_szName;
};

class PProperties
{
public:
	class PProperty
	{
	public:
		friend class PPropertiesWriter;

	public:
		PProperty()
		{
			m_pName = TNULL;
			m_pValue = TNULL;
		}

		PProperty(const Toshi::TString8& a_sName, TINT a_iValue)
		{
			m_pName = new PPropertyName(a_sName);
			m_pValue = new PPropertyValue(a_iValue);
		}

		PProperty(const Toshi::TString8& a_sName, TBOOL a_bValue)
		{
			m_pName = new PPropertyName(a_sName);
			m_pValue = new PPropertyValue(a_bValue);
		}

		PProperty(const Toshi::TString8& a_sName, TFLOAT a_fValue)
		{
			m_pName = new PPropertyName(a_sName);
			m_pValue = new PPropertyValue(a_fValue);
		}

		PProperty(const Toshi::TString8& a_sName, TUINT32 a_uiValue)
		{
			m_pName = new PPropertyName(a_sName);
			m_pValue = new PPropertyValue(a_uiValue);
		}

		PProperty(const Toshi::TString8& a_sName, const Toshi::TString8& a_sValue)
		{
			m_pName = new PPropertyName(a_sName);
			m_pValue = new PPropertyValue(a_sValue);
		}

		PProperty(const Toshi::TString8& a_sName, PProperties* a_pProperties)
		{
			m_pName = new PPropertyName(a_sName);
			m_pValue = new PPropertyValue(a_pProperties);
		}

		PProperty(const Toshi::TString8& a_sName, PPropertyValueArray* a_pArray)
		{
			m_pName = new PPropertyName(a_sName);
			m_pValue = new PPropertyValue(a_pArray);
		}

		PProperty(const PProperty& other)
		{
			m_pName = new PPropertyName(*other.m_pName);
			m_pValue = new PPropertyValue(*other.m_pValue);
		}

		PProperty(PProperty&& other) noexcept
		{
			m_pName = other.m_pName;
			m_pValue = other.m_pValue;
			other.m_pName = TNULL;
			other.m_pValue = TNULL;
		}

		~PProperty()
		{
			Delete();
		}

		PPropertyName& GetName()
		{
			return *m_pName;
		}

		PPropertyValue* GetValue()
		{
			return m_pValue;
		}

		const PPropertyName& GetName() const
		{
			return *m_pName;
		}

		const PPropertyValue* GetValue() const
		{
			return m_pValue;
		}

		PProperty& operator=(const PProperty& other)
		{
			Delete();

			m_pName = new PPropertyName(*other.m_pName);
			m_pValue = new PPropertyValue(*other.m_pValue);

			return *this;
		}

		PProperty& operator=(PProperty&& other) noexcept
		{
			Delete();

			m_pName = other.m_pName;
			m_pValue = other.m_pValue;
			other.m_pName = TNULL;
			other.m_pValue = TNULL;

			return *this;
		}

	private:
		void Delete()
		{
			if (m_pName)
				delete m_pName;

			if (m_pValue)
				delete m_pValue;
		}

	private:
		PPropertyName* m_pName;
		PPropertyValue* m_pValue;
	};

	friend class PPropertiesWriter;

public:
	PProperties()
	{
		m_pParent = TNULL;
		m_pProperties = TNULL;
		m_iCount = 0;
	}

	PProperties(const PProperties& other)
	{
		m_pParent = other.m_pParent;

		if (other.m_pProperties != TNULL)
		{
			m_pProperties = new PProperty[other.m_iCount];

			for (size_t i = 0; i < other.m_iCount; i++)
			{
				m_pProperties[i] = other.m_pProperties[i];
				auto pValue = m_pProperties[i].GetValue();

				if (pValue->GetType() == PPropertyValue::Type::Props)
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

	PProperties(PProperties&& other) noexcept
	{
		m_pParent = other.m_pParent;
		m_pProperties = other.m_pProperties;
		m_iCount = other.m_iCount;
		other.m_pParent = TNULL;
		other.m_pProperties = TNULL;
		other.m_iCount = 0;
	}

	~PProperties()
	{
		Delete();
	}

	PProperty* Begin()
	{
		return m_pProperties;
	}

	PProperty* End()
	{
		return m_pProperties + m_iCount;
	}

	const PProperty* Begin() const
	{
		return m_pProperties;
	}

	const PProperty* End() const
	{
		return m_pProperties + m_iCount;
	}

	const PPropertyValue* GetProperty(size_t a_iIndex) const
	{
		TASSERT(m_iCount > a_iIndex);
		return m_pProperties[a_iIndex].GetValue();
	}

	const PPropertyValue* GetOptionalProperty(const char* a_szName) const
	{
		for (size_t i = 0; i < m_iCount; i++)
		{
			if (m_pProperties[i].GetName() == a_szName)
			{
				return m_pProperties[i].GetValue();
			}
		}

		return TNULL;
	}

	const PProperties* GetParentProperties() const
	{
		return m_pParent;
	}

	TUINT32 GetPropertyCount() const
	{
		return m_iCount;
	}

	PProperty* AddProperty(const Toshi::TString8& a_Name, TINT a_iValue)
	{
		return AllocProperty(PProperty(a_Name, a_iValue));
	}

	PProperty* AddPropertyBool(const Toshi::TString8& a_Name, TBOOL a_bValue)
	{
		return AllocProperty(PProperty(a_Name, a_bValue));
	}

	PProperty* AddProperty(const Toshi::TString8& a_Name, TFLOAT a_fValue)
	{
		return AllocProperty(PProperty(a_Name, a_fValue));
	}

	PProperty* AddProperty(const Toshi::TString8& a_Name, TUINT32 a_uiValue)
	{
		return AllocProperty(PProperty(a_Name, a_uiValue));
	}

	PProperty* AddProperty(const Toshi::TString8& a_Name, const Toshi::TString8& a_sValue)
	{
		return AllocProperty(PProperty(a_Name, a_sValue));
	}

	PProperty* AddProperties(const Toshi::TString8& a_Name)
	{
		PProperties* pProperties = new PProperties;
		pProperties->m_pParent = this;

		return AllocProperty(PProperty(a_Name, pProperties));
	}

	PProperty* AddPropertyArray(const Toshi::TString8& a_Name)
	{
		PPropertyValueArray* pArray = new PPropertyValueArray;

		return AllocProperty(PProperty(a_Name, pArray));
	}

#ifdef __TOSHI_TTRB_H__
	__forceinline static const PProperties* LoadFromTRB(Toshi::TTRB* a_pTRB)
	{
		return a_pTRB->CastSymbol<PProperties>("Main");
	}

	__forceinline static const PProperties* LoadFromTRB(Toshi::TTRB& a_TRB)
	{
		return a_TRB.CastSymbol<PProperties>("Main");
	}

	__forceinline static const PProperties* LoadFromAsset(Toshi::TTRB* a_pTRB, const char* a_szFileName)
	{
		Toshi::TString8 symbolName = a_szFileName;
		symbolName.MakeLower();
		symbolName += "_Main";

		return a_pTRB->CastSymbol<PProperties>(symbolName);
	}

	__forceinline static const PProperties* LoadFromAsset(Toshi::TTRB& a_TRB, const char* a_szFileName)
	{
		Toshi::TString8 symbolName = a_szFileName;
		symbolName.MakeLower();
		symbolName += "_Main";

		return a_TRB.CastSymbol<PProperties>(symbolName);
	}
#endif

private:
	PProperty* AllocProperty(PProperty&& a_Property)
	{
		if (m_pProperties != TNULL)
		{
			auto newProperties = new PProperty[m_iCount + 1];

			for (size_t i = 0; i < m_iCount; i++)
				newProperties[i] = std::move(m_pProperties[i]);

			Delete();
			m_pProperties = newProperties;

			m_iCount += 1;
		}
		else
		{
			m_pProperties = new PProperty[1];
			m_iCount = 1;
		}

		m_pProperties[m_iCount - 1] = std::move(a_Property);
		return &m_pProperties[m_iCount - 1];
	}

	void Delete()
	{
		if (m_pProperties)
			delete[] m_pProperties;
	}

private:
	PProperties* m_pParent;
	PProperty* m_pProperties;
	TUINT32 m_iCount;
};

inline void PPropertyValue::Delete()
{
	if (m_uValue.Pointer)
	{
		if (m_eType == Type::Array)
			delete m_uValue.Array;
		else if (m_eType == Type::Props)
			delete m_uValue.Props;
		else if (m_eType == Type::String)
			delete[] m_uValue.String;
	}
}

inline PPropertyValue::PPropertyValue(const PPropertyValue& other)
{
	m_eType = other.m_eType;
	m_uValue.Pointer = TNULL;

	if (other.m_eType == Type::Array)
		m_uValue.Array = new PPropertyValueArray(*other.m_uValue.Array);
	else if (other.m_eType == Type::Props)
		m_uValue.Props = new PProperties(*other.m_uValue.Props);
	else if (other.m_eType == Type::String)
		SetString(other.m_uValue.String);
	else
		m_uValue = other.m_uValue;
}

inline PPropertyValue::PPropertyValue(PPropertyValue&& other) noexcept
{
	m_eType = other.m_eType;
	m_uValue.Pointer = other.m_uValue.Pointer;
	other.m_uValue.Pointer = TNULL;
}

inline PPropertyValue& PPropertyValue::operator=(const PPropertyValue& other)
{
	Delete();

	m_eType = other.m_eType;

	if (other.m_eType == Type::Array)
		m_uValue.Array = new PPropertyValueArray(*other.m_uValue.Array);
	else if (other.m_eType == Type::Props)
		m_uValue.Props = new PProperties(*other.m_uValue.Props);
	else if (other.m_eType == Type::String)
		SetString(other.m_uValue.String);
	else
		m_uValue = other.m_uValue;

	return *this;
}

inline PPropertyValue& PPropertyValue::operator=(PPropertyValue&& other) noexcept
{
	Delete();

	m_eType = other.m_eType;
	m_uValue.Pointer = other.m_uValue.Pointer;
	other.m_uValue.Pointer = TNULL;

	return *this;
}

inline PPropertyValue::~PPropertyValue()
{
	Delete();
}
