#pragma once
#include "AObjectHash.h"

class AObjectHashMain
    : public AObjectHash
    , public Toshi::TSingleton<AObjectHashMain>
{
public:
	AObjectHashMain( TINT a_iNumNodes );
	~AObjectHashMain();

	void Initialise( TFLOAT a_fMinX, TFLOAT a_fMinY, TFLOAT a_fMaxX, TFLOAT a_fMaxY, TINT a_iNumCellsX, TINT a_iNumCellsY );

};
