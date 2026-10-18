#include "ToshiPCH.h"
#include "TSumOfMinterms.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $JPOG: FUNCTION TKernelInterface.dll+0001f260
TSumOfMinterms::TSumOfMinterms()
    : TArray<TMinterm>( 4, 1 )
{
}

// $JPOG: FUNCTION TKernelInterface.dll+0001f1f0
TSumOfMinterms::TSumOfMinterms( TINT a_iSize )
    : TArray<TMinterm>( 4, TMath::Max( a_iSize, 0 ) )
{
}

// $JPOG: FUNCTION TKernelInterface.dll+0001f160
TSumOfMinterms::TSumOfMinterms( const TMinterm& a_rcMinterm )
    : TArray<TMinterm>( 4, 1 )
{
	Push( a_rcMinterm );
}

// $JPOG: FUNCTION TKernelInterface.dll+0001f140
TSumOfMinterms::TSumOfMinterms( const TSumOfMinterms& a_rcOther )
    : TArray<TMinterm>( 4, 0 )
{
	Assign( a_rcOther );
}

// $JPOG: FUNCTION TKernelInterface.dll+0001f050
TSumOfMinterms::~TSumOfMinterms()
{
}

// $JPOG: FUNCTION TKernelInterface.dll+0001f030
TSumOfMinterms& TSumOfMinterms::operator=( const TSumOfMinterms& a_rcOther )
{
	Assign( a_rcOther );
	return *this;
}

// $JPOG: FUNCTION TKernelInterface.dll+0001f0e0
TSumOfMinterms& TSumOfMinterms::operator+( const TMinterm& a_rcMinterm )
{
	Push( a_rcMinterm );
	return *this;
}

// $JPOG: FUNCTION TKernelInterface.dll+0001f070
TBOOL TSumOfMinterms::GetTruth( TUINT a_uiState ) const
{
	if ( Size() == 0 )
		return TTRUE;

	for ( TINT i = 0; i < Size(); i++ )
	{
		if ( ( *this )[ i ].GetTruth( a_uiState ) )
			return TTRUE;
	}

	return TFALSE;
}

// $NicktoonsPS2: FUNCTION 0039a650
// $JPOG: FUNCTION TKernelInterface.dll+0001ef30
void TSumOfMinterms::MakeFunctionString( TString8& a_rOutString, const TCHAR* const* a_ppOpNames, const TArray<TPString8>& a_rcVarNames ) const
{
	if ( Size() == 0 )
	{
		a_rOutString += a_ppOpNames[ TMinterm::OpName_True ];
		return;
	}

	if ( Size() == 1 )
	{
		( *this )[ 0 ].MakeFunctionString( a_rOutString, a_ppOpNames, a_rcVarNames );
		return;
	}

	// (a) or (b) or ...
	a_rOutString += a_ppOpNames[ TMinterm::OpName_Open ];
	( *this )[ 0 ].MakeFunctionString( a_rOutString, a_ppOpNames, a_rcVarNames );
	a_rOutString += a_ppOpNames[ TMinterm::OpName_Close ];

	for ( TINT i = 1; i < Size(); i++ )
	{
		a_rOutString += a_ppOpNames[ TMinterm::OpName_Or ];
		a_rOutString += a_ppOpNames[ TMinterm::OpName_Open ];
		( *this )[ i ].MakeFunctionString( a_rOutString, a_ppOpNames, a_rcVarNames );
		a_rOutString += a_ppOpNames[ TMinterm::OpName_Close ];
	}
}

// $NicktoonsPS2: FUNCTION 0039a5c0
// $JPOG: FUNCTION TKernelInterface.dll+0001eee0
TPString8 TSumOfMinterms::MakeFunctionString( const TCHAR* const* a_ppOpNames, const TArray<TPString8>& a_rcVarNames ) const
{
	TString8 strFunction;
	MakeFunctionString( strFunction, a_ppOpNames, a_rcVarNames );

	return TPString8( strFunction.GetString() );
}

// $JPOG: FUNCTION TKernelInterface.dll+0001f020
TBOOL TSumOfMinterms::ParseFunctionString( const TString8& a_rcString, const TCHAR* const* a_ppOpNames, const TArray<TPString8>& a_rcVarNames )
{
	return TFALSE;
}

// Replaces the terms with copies of the terms of a_rcOther (the capacity becomes its size)
// NOTE: the name is unknown, shared by the copy constructor and operator= in JPOG
// $JPOG: FUNCTION TKernelInterface.dll+0001f2a0
void TSumOfMinterms::Assign( const TSumOfMinterms& a_rcOther )
{
	FreeData();
	SetGrowSize( a_rcOther.GetGrowSize() );

	if ( a_rcOther.Size() > 0 )
		Resize( a_rcOther.Size() );

	for ( TINT i = 0; i < a_rcOther.Size(); i++ )
		Push( a_rcOther[ i ] );
}

TOSHI_NAMESPACE_END
