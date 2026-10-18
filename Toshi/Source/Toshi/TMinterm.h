#pragma once
#include <Toshi/TArray.h>
#include <Toshi/TPString8.h>
#include <Toshi/TString8.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: a product term of boolean variables (bits of a state word): the
// mask selects the variables of the term, the value gives the expected value
// of each of them. A term with no variables is always true, a term with every
// variable masked and no value set is the "false" term (TMinterm( TFALSE )).
//-----------------------------------------------------------------------------
class TMinterm
{
public:
	// Indices of the operator names of MakeFunctionString
	enum OpName
	{
		OpName_True,
		OpName_False,
		OpName_And,
		OpName_Or,
		OpName_Not,
		OpName_Open,
		OpName_Close,
		OpName_NUMOF
	};

public:
	// $JPOG: FUNCTION TKernelInterface.dll+0001f530
	TMinterm()
	    : m_uiMask( 0 ), m_uiValue( 0 )
	{}

	// $JPOG: FUNCTION TKernelInterface.dll+0001f500
	TMinterm( TBOOL a_bValue )
	    : m_uiMask( a_bValue ? 0 : TUINT( -1 ) ), m_uiValue( 0 )
	{}

	// $JPOG: FUNCTION TKernelInterface.dll+0001f4c0
	TMinterm( TUINT a_uiMask, TUINT a_uiValue )
	    : m_uiMask( a_uiMask ), m_uiValue( a_uiValue )
	{}

	// $JPOG: FUNCTION TKernelInterface.dll+0001f4e0
	TMinterm( const TMinterm& a_rcOther )
	    : m_uiMask( a_rcOther.m_uiMask ), m_uiValue( a_rcOther.m_uiValue )
	{}

	// $JPOG: FUNCTION TKernelInterface.dll+0001f4a0
	TMinterm& operator=( const TMinterm& a_rcOther )
	{
		m_uiMask  = a_rcOther.m_uiMask;
		m_uiValue = a_rcOther.m_uiValue;
		return *this;
	}

	// $JPOG: FUNCTION TKernelInterface.dll+0001f460
	TBOOL operator==( const TMinterm& a_rcOther ) const { return m_uiMask == a_rcOther.m_uiMask && m_uiValue == a_rcOther.m_uiValue; }

	// Checks the term against the state word a_uiState
	// $JPOG: FUNCTION TKernelInterface.dll+0001f480
	TBOOL GetTruth( TUINT a_uiState ) const { return ( a_uiState & m_uiMask ) == m_uiValue; }

	// Appends the term written with the operator names a_ppOpNames (see OpName) and the variable names a_rcVarNames (one per bit)
	void MakeFunctionString( TString8& a_rOutString, const TCHAR* const* a_ppOpNames, const TArray<TPString8>& a_rcVarNames ) const;

	// Returns the term written with the operator names a_ppOpNames and the variable names a_rcVarNames
	TPString8 MakeFunctionString( const TCHAR* const* a_ppOpNames, const TArray<TPString8>& a_rcVarNames ) const;

	TUINT GetMask() const { return m_uiMask; }
	TUINT GetValue() const { return m_uiValue; }

public:
	// "true", "false", " and ", " or ", "not ", "(", ")"
	static const TCHAR* const ms_aEnglishOpNames[ OpName_NUMOF ];

	// "1", "0", " & ", " + ", "~", "(", ")"
	static const TCHAR* const ms_aMathOpNames[ OpName_NUMOF ];

public:
	TUINT m_uiMask;  // 0x0
	TUINT m_uiValue; // 0x4
};

TOSHI_NAMESPACE_END
