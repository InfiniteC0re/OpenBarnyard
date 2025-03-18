#pragma once
#include "Toshi/TString8.h"
#include "Toshi/T2String.h"
#include "Toshi/TArray.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: Allows to easily operate with command line parameters
//-----------------------------------------------------------------------------
class T2CommandLine
{
public:
	T2CommandLine();
	T2CommandLine( T2StringView a_pchCommandLine );

	void Create( T2StringView a_pchCommandLine );

	TBOOL    HasParameter( const TCHAR* a_pszParameter, TINT* a_pIndex = TNULL ) const;
	TString8 GetParameterValue( const TCHAR* a_pszParameter, const TCHAR* a_pszDefaultValue = TNULL ) const;

	const TCHAR* GetNextParameter( const TCHAR* a_pszParameter ) const;

private:
	T2FormatString<4096>     m_szCommandLine;
	T2String8Parser          m_Parser;
	mutable TArray<TString8> m_ParsedParams;
	TBOOL                    m_bInitialised;
};

TOSHI_NAMESPACE_END
