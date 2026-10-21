#pragma once
#include "Render/TViewport.h"

TOSHI_NAMESPACE_START

/**
	 * !!!
	 * This class is completed and is completely the same as TViewport because
	 * all virtual methods of TViewport are for some reason implemented in TViewport_DX8.cpp
	 */
class MODAPI TViewportHAL : public Toshi::TViewport
{
public:
	TViewportHAL( TBOOL a_bFlag )
	    : TViewport( a_bFlag )
	{}

	// $Barnyard: FUNCTION 0060c510
	// $Barnyard: FUNCTION 0060c4f0
	~TViewportHAL() {}
};

TOSHI_NAMESPACE_END
