#pragma once
#include "AObjectHash.h"

//-----------------------------------------------------------------------------
// Purpose: A wrapper for AObjectHash that makes the class a singleton.
//-----------------------------------------------------------------------------
class AObjectHashMain
    : public AObjectHash
    , public Toshi::TSingleton<AObjectHashMain>
{
public:
	AObjectHashMain( TINT a_iNumNodes );
	~AObjectHashMain();

};
