#include "ToshiPCH.h"
#include "T2CommandLine.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi {

	T2CommandLine::T2CommandLine( T2ConstString8 a_pchCommandLine )
	{
		Create( a_pchCommandLine );
	}

	T2CommandLine::T2CommandLine()
	{
		m_szCommandLine[ 0 ] = '\0';
		m_bInitialised = TFALSE;
	}

	void T2CommandLine::Create( T2ConstString8 a_pchCommandLine )
	{
		if ( a_pchCommandLine )
		{
			TSIZE uiCommandLineLength = a_pchCommandLine.Length();
			m_szCommandLine.Copy( a_pchCommandLine );
			m_bInitialised = TTRUE;

			m_Parser.SetBuffer( a_pchCommandLine, uiCommandLineLength );

			// Parse the parameters
			m_ParsedParams.Clear();

			char szToken[ 1024 ];
			while ( !m_Parser.IsOver() )
			{
				TBOOL bReadToken = m_Parser.ReadToken( szToken, sizeof( szToken ) );
				
				if ( bReadToken )
					m_ParsedParams.Push( szToken );
			}
		}
	}

	TBOOL T2CommandLine::HasParameter( const TCHAR* a_pszParameter, TINT* a_pIndex /*= TNULL */ )
	{
		TASSERT( m_bInitialised );

		if ( a_pIndex )
			*a_pIndex = -1;

		TARRAY_FOREACH( m_ParsedParams, it )
		{
			if ( it.Get() == a_pszParameter )
			{
				if ( a_pIndex )
					*a_pIndex = it.GetCurrentIndex();

				return TTRUE;
			}
		}

		return TFALSE;
	}

	TString8 T2CommandLine::GetParameterValue( const TCHAR* a_pszParameter, const TCHAR* a_pszDefaultValue )
	{
		TASSERT( m_bInitialised );

		const TCHAR* pszValue = GetNextParameter( a_pszParameter );

		if ( pszValue )
			return pszValue;

		return a_pszDefaultValue;
	}

	const TCHAR* T2CommandLine::GetNextParameter( const TCHAR* a_pszParameter )
	{
		TINT iParamIndex;
		TBOOL bHasParam = HasParameter( a_pszParameter, &iParamIndex );

		if ( bHasParam && iParamIndex + 1 < m_ParsedParams.Size() )
		{
			return m_ParsedParams[ iParamIndex + 1 ];
		}

		return TNULL;
	}

}
