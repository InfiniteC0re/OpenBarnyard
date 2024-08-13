#pragma once
#include "THookedSingleton.h"

class ARootTask :
	public THookedSingleton<ARootTask, 0x0077de78>
{
public:
	void SetPaused( TBOOL a_bPaused );
};
