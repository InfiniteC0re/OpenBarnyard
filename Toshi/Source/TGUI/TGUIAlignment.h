#pragma once

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: alignment of an element or a text in its rectangle. The names of
// the values in the definition files are "center", "left", "right" (x) and
// "center", "top", "bottom" (y).
//-----------------------------------------------------------------------------
enum TGUIAlignment : TINT
{
	TGUIAlignment_Center = 0,
	TGUIAlignment_Left   = 1,
	TGUIAlignment_Right  = 2,
	TGUIAlignment_Top    = TGUIAlignment_Left,
	TGUIAlignment_Bottom = TGUIAlignment_Right,
	TGUIAlignment_NUMOF  = 3
};

TOSHI_NAMESPACE_END
