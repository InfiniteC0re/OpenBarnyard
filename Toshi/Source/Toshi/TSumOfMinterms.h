#pragma once
#include "TMinterm.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: a boolean function of the bits of a state word written as a sum
// (OR) of product terms (TMinterm). An empty sum is always true. Used for the
// SKU conditions of the top-level GUI dialogs ("skufunction").
//-----------------------------------------------------------------------------
class TSumOfMinterms : public TArray<TMinterm>
{
public:
	TSumOfMinterms();
	TSumOfMinterms( TINT a_iSize );
	TSumOfMinterms( const TMinterm& a_rcMinterm );
	TSumOfMinterms( const TSumOfMinterms& a_rcOther );
	~TSumOfMinterms();

	TSumOfMinterms& operator=( const TSumOfMinterms& a_rcOther );

	// Adds a term to the sum
	TSumOfMinterms& operator+( const TMinterm& a_rcMinterm );

	// Checks the function against the state word a_uiState
	TBOOL GetTruth( TUINT a_uiState ) const;

	// Appends the function written with the operator names a_ppOpNames (see TMinterm::OpName) and the variable names a_rcVarNames
	void MakeFunctionString( TString8& a_rOutString, const TCHAR* const* a_ppOpNames, const TArray<TPString8>& a_rcVarNames ) const;

	// Returns the function written with the operator names a_ppOpNames and the variable names a_rcVarNames
	TPString8 MakeFunctionString( const TCHAR* const* a_ppOpNames, const TArray<TPString8>& a_rcVarNames ) const;

	// NOTE: not implemented in JPOG, always fails
	TBOOL ParseFunctionString( const TString8& a_rcString, const TCHAR* const* a_ppOpNames, const TArray<TPString8>& a_rcVarNames );

private:
	void Assign( const TSumOfMinterms& a_rcOther );
};

TOSHI_NAMESPACE_END
