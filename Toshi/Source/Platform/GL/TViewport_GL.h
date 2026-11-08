#pragma once
#include "Render/TViewport.h"

TOSHI_NAMESPACE_START

class MODAPI TViewportHAL : public Toshi::TViewport
{
public:
	TViewportHAL( TBOOL a_bFlag )
	    : TViewport( a_bFlag )
	{}

	~TViewportHAL() {}
};

TOSHI_NAMESPACE_END
