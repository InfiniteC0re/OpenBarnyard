#include "ToshiPCH.h"
#include "TMinterm.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

const TCHAR* const TMinterm::ms_aEnglishOpNames[ OpName_NUMOF ] = {
	"true",
	"false",
	" and ",
	" or ",
	"not ",
	"(",
	")"
};

const TCHAR* const TMinterm::ms_aMathOpNames[ OpName_NUMOF ] = {
	"1",
	"0",
	" & ",
	" + ",
	"~",
	"(",
	")"
};

// $NicktoonsPS2: FUNCTION 0039a480
// $JPOG: FUNCTION TKernelInterface.dll+0001edf0
void TMinterm::MakeFunctionString( TString8& a_rOutString, const TCHAR* const* a_ppOpNames, const TArray<TPString8>& a_rcVarNames ) const
{
	if ( m_uiMask == 0 )
	{
		a_rOutString += a_ppOpNames[ OpName_True ];
		return;
	}

	if ( m_uiMask == TUINT( -1 ) && m_uiValue == 0 )
	{
		a_rOutString += a_ppOpNames[ OpName_False ];
		return;
	}

	TBOOL bFirst = TTRUE;

	for ( TINT i = 0; i < a_rcVarNames.Size(); i++ )
	{
		TUINT uiBit = 1 << i;

		if ( m_uiMask & uiBit )
		{
			if ( !bFirst )
				a_rOutString += a_ppOpNames[ OpName_And ];

			if ( !( m_uiValue & uiBit ) )
				a_rOutString += a_ppOpNames[ OpName_Not ];

			a_rOutString += a_rcVarNames[ i ].GetString();
			bFirst = TFALSE;
		}
	}
}

// $JPOG: FUNCTION TKernelInterface.dll+0001eda0
TPString8 TMinterm::MakeFunctionString( const TCHAR* const* a_ppOpNames, const TArray<TPString8>& a_rcVarNames ) const
{
	TString8 strFunction;
	MakeFunctionString( strFunction, a_ppOpNames, a_rcVarNames );

	return TPString8( strFunction.GetString() );
}

TOSHI_NAMESPACE_END
