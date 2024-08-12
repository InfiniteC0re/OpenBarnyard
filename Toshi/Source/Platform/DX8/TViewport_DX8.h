#pragma once
#include "Render/TViewport.h"

namespace Toshi
{

/**
	 * !!!
	 * This class is completed and is completely the same as TViewport because
	 * all virtual methods of TViewport are for some reason implemented in TViewport_DX8.cpp
	 */
class TViewportHAL : public Toshi::TViewport
{
public:
    TViewportHAL( TBOOL a_bFlag ) :
        TViewport( a_bFlag )
    {}
};

} // namespace Toshi