#pragma once
#include "Toshi/Typedefs.h"

namespace Toshi
{

union TVersion {
	TUINT32 Value;

	struct {
		TUINT16 Minor;
		TUINT16 Major;
	} Parts;

	void operator=(TUINT32 a_uiVersion)
	{
		Value = a_uiVersion;
	}

	operator TUINT32& ()
	{
		return Value;
	}
};

}