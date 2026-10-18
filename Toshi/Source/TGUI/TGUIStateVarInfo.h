#pragma once
#include <Toshi/TPString8.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: a state variable of a widget class ("Enabled", "Focused",
// "Pressed"...): its name and the bit of the widget state word it uses.
//-----------------------------------------------------------------------------
struct TGUIStateVarInfo
{
	// Orders the state variables by their bits
	struct Comparator
	{
		TINT operator()( const TGUIStateVarInfo& a_rcA, const TGUIStateVarInfo& a_rcB ) const
		{
			return TINT( a_rcA.uiMask - a_rcB.uiMask );
		}
	};

	TGUIStateVarInfo()
	    : uiMask( 0 )
	{}

	TGUIStateVarInfo( const TPString8& a_rcName, TUINT a_uiMask )
	    : strName( a_rcName ), uiMask( a_uiMask )
	{}

	TPString8 strName; // 0x0
	TUINT     uiMask;  // 0x4
};

TOSHI_NAMESPACE_END
