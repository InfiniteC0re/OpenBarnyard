#pragma once

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: condition on the state variables of a widget (bits of its state
// word, see TGUIStateVarInfo): the mask selects the variables the rule checks,
// the value gives their expected values. A rule with an empty mask matches any
// state (the default state of a style component).
//-----------------------------------------------------------------------------
struct TGUIStateRule
{
	TGUIStateRule()
	    : uiMask( 0 ), uiValue( 0 )
	{}

	TGUIStateRule( TUINT a_uiMask, TUINT a_uiValue )
	    : uiMask( a_uiMask ), uiValue( a_uiValue )
	{}

	TBOOL Matches( TUINT a_uiState ) const { return ( a_uiState & uiMask ) == uiValue; }

	TBOOL operator==( const TGUIStateRule& a_rcOther ) const { return uiMask == a_rcOther.uiMask && uiValue == a_rcOther.uiValue; }

	TUINT uiMask;  // 0x0
	TUINT uiValue; // 0x4
};

TOSHI_NAMESPACE_END
