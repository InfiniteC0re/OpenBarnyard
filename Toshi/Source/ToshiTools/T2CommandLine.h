#pragma once
#include "Toshi/TString8.h"
#include "Toshi/T2String8.h"
#include "Toshi/T2FixedString.h"
#include "Toshi/T2String8Parser.h"
#include "Toshi/TArray.h"

namespace Toshi {

	//-----------------------------------------------------------------------------
	// Purpose: Allows to easily operate with command line parameters
	//-----------------------------------------------------------------------------
	class T2CommandLine
	{
	public:
		T2CommandLine();
		T2CommandLine( T2ConstString8 a_pchCommandLine );

		void Create( T2ConstString8 a_pchCommandLine );

		TBOOL HasParameter( const TCHAR* a_pszParameter, TINT* a_pIndex = TNULL );
		TString8 GetParameterValue( const TCHAR* a_pszParameter, const TCHAR* a_pszDefaultValue = TNULL );

	private:
		const TCHAR* GetNextParameter( const TCHAR* a_pszParameter );

	private:
		T2FixedString<4096> m_szCommandLine;
		T2String8Parser m_Parser;
		TArray<TString8> m_ParsedParams;
		TBOOL m_bInitialised;
	};

}