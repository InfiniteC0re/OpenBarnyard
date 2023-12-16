#pragma once
#include "AHookedSingleton.h"

class ARootTask :
	public AHookedSingleton<ARootTask, 0x0077de78>
{
public:
	void SetPaused(TBOOL a_bPaused);
};
