#pragma once
#include "Toshi/Strings/TStringManager.h"

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

public:
	Type GetType()
	{
		return m_eType;
	}

	bool GetBoolean() const
	{
		TASSERT(m_eType == Type::Bool);
		return m_uValue.Bool;
	}

	float GetFloat() const
	{
		TASSERT(m_eType == Type::Float);
		return m_uValue.Float;
	}

	int GetInteger() const
	{
		TASSERT(m_eType == Type::Int);
		return m_uValue.Int;
	}

	uint32_t GetUINT32() const
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

private:
	Type m_eType;

	union {
		void* Pointer;
		const char* String;
		float Float;
		TBOOL Bool;
		int Int;
		uint32_t UInt32;
		class PProperties* Props;
		class PPropertyValueArray* Array;
	} m_uValue;
};

class PPropertyValueArray
{
public:
	const PPropertyValue* GetValue(size_t a_iIndex) const
	{
		TASSERT(m_iSize > a_iIndex);
		return &m_pProperties[a_iIndex];
	}

	uint32_t GetSize() const
	{
		return m_iSize;
	}

	const PPropertyValue* operator[](size_t a_iIndex) const
	{
		return GetValue(a_iIndex);
	}

private:
	PPropertyValue* m_pProperties;
	uint32_t m_iSize;
};

class PPropertyName
{
public:
	const char* GetName() const
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

private:
	const char* m_szName;
};

class PProperties
{
public:
	class PProperty
	{
	public:
		const PPropertyName& GetName() const
		{
			return *m_pName;
		}

		const PPropertyValue* GetValue() const
		{
			return m_pValue;
		}

	private:
		PPropertyName* m_pName;
		PPropertyValue* m_pValue;
	};

public:
	PProperties() = default;

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

	uint32_t GetPropertyCount() const
	{
		return m_iCount;
	}

#ifdef __TOSHI_TTRB_H__
	__forceinline static PProperties* LoadFromTRB(Toshi::TTRB* a_pTRB)
	{
		return a_pTRB->CastSymbol<PProperties>("Main");
	}

	__forceinline static PProperties* LoadFromTRB(Toshi::TTRB& a_TRB)
	{
		return a_TRB.CastSymbol<PProperties>("Main");
	}
#endif

private:
	PProperties* m_pParent;
	PProperty* m_pProperties;
	uint32_t m_iCount;
};
