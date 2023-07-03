#pragma once

#define CVAR_CREATE(NAME, DEFAULT_VALUE) \
::AConsoleVar g_cvar_##NAME(#NAME, DEFAULT_VALUE);

#define CVAR_EXTERN(NAME) \
extern ::AConsoleVar g_cvar_#NAME;

#define CVAR_GET_INT(NAME) \
g_cvar_##NAME.GetInt()

#define CVAR_GET_FLOAT(NAME) \
g_cvar_##NAME.GetFloat()

#define CVAR_GET_BOOL(NAME) \
g_cvar_##NAME.GetBool()

#define CVAR_GET_STRING(NAME) \
g_cvar_##NAME.GetString()

#define CVAR_SET_INT(NAME, VALUE) \
g_cvar_##NAME.SetInt(VALUE)

#define CVAR_SET_FLOAT(NAME, VALUE) \
g_cvar_##NAME.SetFloat(VALUE)

#define CVAR_SET_BOOL(NAME, VALUE) \
g_cvar_##NAME.SetBool(VALUE)

#define CVAR_SET_STRING(NAME, VALUE) \
g_cvar_##NAME.SetString(VALUE)

class AConsoleVar
{
public:
	using TYPE = uint32_t;
	enum TYPE_ : TYPE
	{
		TYPE_INT,
		TYPE_FLOAT,
		TYPE_BOOL,
		TYPE_STRING,
	};

	static constexpr size_t MAX_STRING_SIZE = 32;

public:
	AConsoleVar(const char* name, int value)
	{
		m_szName = name;
		m_eType = TYPE_INT;
		m_ValueInt = value;
	}

	AConsoleVar(const char* name, float value)
	{
		m_szName = name;
		m_eType = TYPE_FLOAT;
		m_ValueFloat = value;
	}

	AConsoleVar(const char* name, TBOOL value)
	{
		m_szName = name;
		m_eType = TYPE_BOOL;
		m_ValueBool = value;
	}

	AConsoleVar(const char* name, const char*& value)
	{
		m_szName = name;
		m_eType = TYPE_STRING;
		Toshi::TStringManager::String8Copy(m_ValueString, value, MAX_STRING_SIZE);
	}

	void SetInt(int value)
	{
		TASSERT(TYPE_INT == m_eType);
		m_ValueInt = value;
	}

	void SetFloat(float value)
	{
		TASSERT(TYPE_FLOAT == m_eType);
		m_ValueFloat = value;
	}

	void SetBool(TBOOL value)
	{
		TASSERT(TYPE_BOOL == m_eType);
		m_ValueBool = value;
	}

	void SetString(const char*& value)
	{
		TASSERT(TYPE_STRING == m_eType);
		Toshi::TStringManager::String8Copy(m_ValueString, value, MAX_STRING_SIZE);
	}

	const char* GetName() const
	{
		return m_szName;
	}

	TYPE GetType() const
	{
		return m_eType;
	}

	int GetInt()
	{
		TASSERT(TYPE_INT == m_eType);
		return m_ValueInt;
	}

	float GetFloat()
	{
		TASSERT(TYPE_FLOAT == m_eType);
		return m_ValueFloat;
	}

	TBOOL GetBool()
	{
		TASSERT(TYPE_BOOL == m_eType);
		return m_ValueBool;
	}

	const char* GetString()
	{
		TASSERT(TYPE_STRING == m_eType);
		return m_ValueString;
	}

private:
	TYPE m_eType;
	
	union
	{
		int   m_ValueInt;
		float m_ValueFloat;
		TBOOL m_ValueBool;
		char  m_ValueString[MAX_STRING_SIZE];
	};

	const char* m_szName;
};